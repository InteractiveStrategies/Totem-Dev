<?php
/**
 * @file
 * totem_media.form.inc
 */

// Hook implementations.
/**
 * Implements hook_form_alter().
 */
function totem_media_form_alter(&$form, &$form_state, $form_id) {
  switch ($form_id) {
    case 'totem_common_form_settings':
      $form['totem_media_config'] = array(
        '#type' => 'fieldset',
        '#title' => t("Media/Files Settings"),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
      );

      $form['totem_media_config']['totem_media_gallery_mode'] = array(
        '#type' => 'checkbox',
        '#title' => t("Enable gallery/lightbox mode for media content"),
        '#description' => t("Check here to enable displaying thumbnails of media items in grid layouts, and viewing and paging through them full-size in a modal overlay. This is ideal if your media is mainly visual, such as photos and videos."),
        '#default_value' => variable_get('totem_media_gallery_mode'),
      );
      break;

    case 'media_node_form':
    case 'node_delete_confirm':
      // Adjust cancel link if we came from viewing media node in modal:
      // add attributes for modal / reuse-modal links so we can return there.
      // @see totem_common_form_node_form_alter(), node_delete_confirm(), confirm_form()
      if ($form['#node']->type == 'media' && variable_get('totem_media_gallery_mode') == TRUE && isset($form['actions']['cancel'])) {
        $cancel = &$form['actions']['cancel'];
        if ($cancel['#type'] == 'link' && strpos($cancel['#href'], '/media-view/') !== FALSE) {
          // Do not use our custom totem-media-ctools-reuse-modal class here,
          // since these forms have different modal settings (mainly size)
          // than the media view modal - so we want to open it anew.
          // @see _totem_common_modal_link_attributes_ensure()
          $cancel['#attributes']['class'][] = 'ctools-use-modal';
          $cancel['#attributes']['class'][] = 'ctools-modal-media-lightbox';
        }
      }

      break;
  }
}

// Node + custom forms.
/**
 * An #element_validate function for a text field containing a YouTube URL.
 *
 * Checks for:
 * - properly formatted YouTube URL, i.e.
 *   http://www.youtube.com/watch?v=[video ID]
 * - valid video ID
 * - video is not set to private by the owning YT account
 *
 * @see totem_media_form_upload(), totem_media_form_media_node_form_alter()
 */
function _totem_media_form_youtube_url_validate($element, &$form_state) {

  $url_parts = drupal_parse_url(trim($element['#value']));

  // Check for minimum necessary format/parts of YouTube URL.
  if ($url_parts['path'] != "http://www.youtube.com/watch" || empty($url_parts['query']['v'])) {
    form_error($element, t("Please enter a properly formatted URL."));
  }
  else {
    // Check for invalid or private video ID.
    // (Note that unlisted videos' data *can* be retrieved.)
    $yt_api_url = "http://gdata.youtube.com/feeds/api/videos/" . $url_parts['query']['v'] . "?alt=json";

    $ch = curl_init($yt_api_url);
    // Don't let the output get written into the AJAX response!!
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
    curl_exec($ch);

    switch (curl_getinfo($ch, CURLINFO_HTTP_CODE)) {
      case 400:
        form_error($element, t("The YouTube video ID is invalid."));
        break;

      case 403:
        form_error($element, t("You do not have permission to access this video."));
        break;
    }

    curl_close($ch);
  }

}
/**
 * Implements hook_form_FORM_ID_alter() for 'media_node_form'.
 */
function totem_media_form_media_node_form_alter(&$form, &$form_state, $form_id) {

  // Block body field from use in gallery mode.
  $form['body']['#access'] = !variable_get('totem_media_gallery_mode');

  // Each media node should have one of field_file or field_media_url populated,
  // but not both. Thus, we can't set either field as required on the node type.
  // So if editing an existing node, show only the populated field, and set it
  // as required (on the form at least).
  // Also add validation in the case of a YouTube URL.
  if (!empty($form['#node']->field_file)) {
    $form['field_media_url']['#access'] = FALSE;

    // This seems to invoke both blocking submit and displaying the error
    // message when field is left blank.
    // However, the required marker only shows when the field is empty, not if
    // a file is already reference there. This is probably a core bug related
    // related to multi-value fields.
    // @see https://drupal.org/node/980144
    $form['field_file'][LANGUAGE_NONE][0]['#required'] = TRUE;
  }
  elseif (!empty($form['#node']->field_media_url)) {
    $form['field_file']['#access'] = FALSE;

    // This makes the appropriate error message appear when field is left blank.
    $form['field_media_url'][LANGUAGE_NONE][0]['#required'] = TRUE;
    // This makes the required marker appear and blocks the form submit when
    // field is left blank.
    $form['field_media_url'][LANGUAGE_NONE][0]['value']['#required'] = TRUE;

    // Since field can only hold 1 value, I think we're safe to set this on
    // the individual text field.
    $form['field_media_url'][LANGUAGE_NONE][0]['value']['#element_validate'][] = '_totem_media_form_youtube_url_validate';
  }


  if (variable_get('totem_media_gallery_mode') == TRUE) {

    $path = drupal_get_destination();
    $path = drupal_parse_url($path['destination']);
    $redirect = str_ireplace("/media-view/{$form['#node']->nid}", '/media', $path['path']);

    if (!empty($path['query']['grouping'])) {
      if (is_numeric($path['query']['grouping'])) {
        // If clicking edit/delete on media items in a collection, force
        // redirect to collection node view.
        $redirect .= "/{$path['query']['grouping']}";
      }
    }

    $form['actions']['submit']['#submit'][] = 'totem_media_form_media_node_form_submit';

    $form['redirect'] = array(
      '#type' => 'value',
      '#value' => $redirect,
    );
  }
}
/**
 * Custom submit handler for node add/edit/delete forms.
 */
function totem_media_form_media_node_form_submit($form, &$form_state) {

  if (variable_get('totem_media_gallery_mode') == TRUE) {
    $form_state['redirect'] = $form_state['values']['redirect'];
  }

}
/**
 * Implements hook_form_FORM_ID_alter().
 */
function totem_media_form_node_delete_confirm_alter(&$form, &$form_state, $form_id) {

  $node = $form_state['build_info']['args'][0];

  if ($node->type == 'media') {
    if (variable_get('totem_media_gallery_mode') == TRUE) {
      // Copy "redirect" value and submit from the edit form so we don't have
      // to dupe logic.
      $form_edit = drupal_get_form('media_node_form', $node);
      $form['#submit'][] = 'totem_media_form_media_node_form_submit';
      $form['redirect'] = $form_edit['redirect'];
    }
  }

}
/**
 * Implements hook_form_FORM_ID_alter().
 */
function totem_media_form_totem_common_form_node_remove_alter(&$form, &$form_state, $form_id) {
  $form['#submit'][] = 'totem_media_form_node_remove_submit';
}
/**
 * Extra handler for totem_common_form_node_remove().
 * If media node is also in a collection(s) in that community, remove that reference too.
 */
function totem_media_form_node_remove_submit($form, &$form_state) {
  $removed_nid = $form_state['values']['nid'];

  // Find all media_collection nodes, in this community, that contain this media node.
  $efq_params = array(
    'return' => 'entity_id',
    'entity_type' => 'node',
    'bundle' => 'media_collection',
    'field_conditions' => array(
      array('field' => 'field_community', 'column' => 'target_id', 'value' => $form_state['values']['field_community_context']),
      array('field' => 'field_media', 'column' => 'target_id', 'value' => $removed_nid),
    ),
    'page_limit' => NULL,
    // Workaround for lack of node table join, which is necessary for count queries.
    // @see totem_common_query_NODE_FILTER_alter()
    'property_conditions' => array(
      array('column' => 'status', 'value' => array(0, 1)),
    ),
  );

  $collection_nids = _totem_common_efq($efq_params)->results;
  foreach ($collection_nids as $collection_nid) {
    // Remove media node from media_collection node.
    _totem_common_set_field_entityreference_values('node', node_load($collection_nid), 'field_media', array($removed_nid), TRUE);
  }
}
/**
 * Modal node add/edit/delete/remove forms.
 */
function totem_media_form_node($op, $type, $node_parent, $node) {

  global $user;

  $title = NULL;
  $form_id = NULL;
  $args = array();

  switch ($type) {
    case 'files':
      $title = t('Add Media');
      $form_id = 'totem_media_form_upload';
      $args[] = $node_parent;
      break;

    case 'collection':
      module_load_include('inc', 'node', 'node.pages');
      $form_id = 'media_collection_node_form';
      $form = drupal_get_form($form_id, $node, $node_parent);
      // Bypass putting form into modal by returning here.
      return $form;
      break;

    case 'media_item':
      // node/%node/modal/remove-from-collection/%node
      // totem_media_menu() makes up this $type arg just for this form.
      if ($op == 'remove-from-collection') {
        $form_id = 'totem_media_confirm_remove_from_collection';
        $args[] = $node; // the media node
        $args[] = $node_parent; // the media_collection node
        $title = t("Remove from @collection", array('@collection' => t("collection")));
      }
      break;
  }

  // For non-AJAX debug:
  // $form = drupal_get_form($form_id, $node_parent); return $form;
  $form_state = array(
    'title' => $title,
    'ajax' => TRUE,
    'build_info' => array(
      'args' => $args,
      // Add a custom flag to indicate from where the form was called.
      // This allows form alters to depend on $form_state['build_info']['args'].
      'generator' => __FUNCTION__,
    ),
  );

  ctools_include('modal');
  ctools_include('ajax');

  $output = ctools_modal_form_wrapper($form_id, $form_state);

  if (!empty($form_state['executed'])) {
    ctools_add_js('ajax-responder');

    // Overwrite the form output.
    $output = array();

    // 10/4, tory: this structure has been refactored a bit
    // to match totem_common_form_node()...
    // Note we aren't checking $_GET['destination'] here though,
    // b/c not sure if doing so would break any existing redirection.
    if (!empty($form_state['redirect'])) {
      $redirect = $form_state['redirect'];
    }

    if (!empty($redirect)) {
      // The first and third args get passed to url().
      if (is_array($redirect)) {
        $output[] = ctools_ajax_command_redirect($redirect[0], 0, $redirect[1]);
      }
      else {
        $output[] = ctools_ajax_command_redirect($redirect);
      }
    }
    else {
      // Refresh the page we came from.
      $output[] = ctools_ajax_command_reload();
    }
  }

  print ajax_render($output);
  exit;
}

/**
 * Custom form to add media by uploading files or entering a YouTube URL.
 */
function totem_media_form_upload($form, &$form_state) {

  $form = array(
    '#validate' => array('totem_media_form_upload_validate'),
    '#submit' => array('totem_media_form_upload_submit'),
  );

  // Default to checked if session filter is active.
  $filters = _totem_common_session_get('filter');
  $status_override_default = 1;
  if (!empty($filters['node']['status']) && empty($form_state['node']->nid)) {
    $status_override_default = 0;
  }

  $form['status_override'] = array(
    '#access' => user_access('view own unpublished content'),
    '#type' => 'checkbox',
    '#title' => t('Make these items private? Only you will be able to view and manage private content.'),
    '#default_value' => !$status_override_default,
  );

  $form['collection'] = array(
    '#type' => 'fieldset',
    // "Remove" all collection stuff from form according to variable.
    '#access' => !variable_get('totem_media_hide_collections'),
  );

  $form['collection']['add_to_collection'] = array(
    '#type' => 'checkbox',
    '#title' => t("Add these items to @collection?", array('@collection' => t('a collection'))),
  );

  // Get any existing media collection records.
  global $user;
  $collection_hook_args = array(
    'return' => 'entity',
    'node' => $form_state['build_info']['args'][0],
    'property_conditions' => array(
      array('column' => 'uid', 'value' => $user->uid),
    ),
    'page_limit' => NULL,
  );
  $collection_list = module_invoke('totem_media', 'node_community_media_collection', $collection_hook_args);
  $entities = $collection_list['nodes']['query']->results;

  $options = array();
  if (!empty($entities)) {
    foreach ($entities as $ix => $meta) {
      $options[$meta->nid] = $meta->title;
    }
  }
  $form['collection']['collection_nid'] = array(
    '#type' => 'select',
    '#title' => t('Select a collection'),
    '#options' => $options,
    '#required' => FALSE,
    '#empty_value' => 0,
    '#empty_option' => t('- New @collection -', array('@collection' => t('collection'))),
    '#states' => array(
      'visible' => array(
        ':input[name=add_to_collection]' => array('checked' => TRUE),
      ),
    ),
  );

  // This field is only enabled when "New collection" (empty option) is selected
  // in the dropdown. This adds clarity for user and prevents submitting both
  // a new collection title and an existing collection nid.
  $form['collection']['collection_title'] = array(
    '#type' => 'textfield',
    '#title' => t('New @collection title', array('@collection' => t('collection'))),
    '#states' => array(
      'visible' => array(
        ':input[name=add_to_collection]' => array('checked' => TRUE),
      ),
      'enabled' => array(
        ':input[name=collection_nid]' => array('value' => '0'),
      ),
    ),
  );

  // Choose between uploading local files or embedding a video by URL (currently
  // only supporting YouTube).
  $form['source'] = array(
    '#type' => 'radios',
    '#title' => t("How do you want to add media?"),
    '#options' => array(
      'upload' => t("Upload files"),
      'url' => t("Add a video by URL (currently supported: YouTube)"),
    ),
    '#default_value' => 'upload',
  );

  // Use settings from media content type's field_file.
  $field_file = field_info_instance('node', 'field_file', 'media');
  // Strangely enough, the first arg, $field, is not used in this function,
  // so no need to bother loading it.
  $upload_validators = file_field_widget_upload_validators(array(), $field_file);

  $form['files'] = array(
    '#title' => 'Files',
    '#type' => 'plupload',
    '#required' => FALSE,
    '#upload_validators' => $upload_validators,
    // Toggling visibility only partially works due to .form-wrapper class
    // plupload element; workaround included.
    // @see totem_media_element_info_alter(),
    // _totem_media_plupload_elem_post_render()
    '#states' => array(
      'visible' => array(
        ':input[name="source"]' => array('value' => 'upload'),
      ),
    ),
    // Plupload only passes a default setting for this, pulled from the PHP
    // max upload size setting. Override with our field's setting (if any;
    // falls back to the PHP setting otherwise).
    // @see plupload_library(), plupload_element_pre_render()
    // Feature request for Plupload module to do this:
    // @see https://drupal.org/node/2105243
    // Note also: file_validate_size() bypasses all checks for uid 1. So no,
    // you're not crazy - if logged in as uid 1, uploads are only limited by
    // the PHP max upload setting.
    '#attached' => array(
      'js' => array(
        array(
          'type' => 'setting',
          'data' => array(
            'plupload' => array(
              'edit-files' => array(
                'max_file_size' => $upload_validators['file_validate_size'][0] . 'b',
              ),
            ),
          ),
        ),
      ),
    ),
    // Sigh; hardcode the id attr because we've also gotta pass it to the js
    // setting just above, and we want to avoid the incrementation on AJAX
    // requests via drupal_html_id().
    // @see form_builder()
    '#id' => 'edit-files',
  );

  // TODO: support multiple? (textarea + line-break delineated? way easier than
  // ajax add-more...which in fact could be a mess on this ajax form.)
  // TODO: let user enter node title too? For now, not bothering since you can't
  // enter titles alongside uploading files either.
  $form['urls'] = array(
    '#type' => 'textfield',
    '#title' => t("Video URL"),
    '#description' => t("Enter the full URL of the video's web page, i.e. <code>http://www.youtube.com/watch?v=[video id]</code>"),
    '#states' => array(
      'visible' => array(
        ':input[name="source"]' => array('value' => 'url'),
      ),
    ),
  );

  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
    // Only show if adding by URL. If uploading files, hide button because
    // JS auto-submits when uploads complete.
    // @see totem_media.js
    '#states' => array(
      'visible' => array(
        ':input[name="source"]' => array('value' => 'url'),
      ),
    ),
  );

  return $form;
}
/**
 * Form validation for adding media.
 *
 * If the source is intended to be a URL, check that it's not empty. Then
 * validate the YouTube URL.
 */
function totem_media_form_upload_validate($form, &$form_state) {
  // Check for empty 'urls' field here instead of making the field required
  // so that it won't cause an error if uploading files instead. This is simpler
  // than using two submit buttons and #limit_validation_errors.
  if ($form_state['values']['source'] == 'url') {
    if (empty($form_state['values']['urls'])) {
      form_set_error('urls', t("Please enter a valid URL."));
    }
    // TODO: support multiple?
    // Validate the YT URL. We call this manually instead of assigning it via
    // #element_validate because it assumes the element is not empty, which
    // in this context (new node) we are checking first per above.
    _totem_media_form_youtube_url_validate($form['urls'], $form_state);
  }
}
/**
 * Save each uploaded file or YouTube URL to a new media node.
 * If indicated, add all to a new or the chosen existing media_collection node.
 */
function totem_media_form_upload_submit($form, &$form_state) {

  global $user;

  if (!form_get_errors()) {

    $node_parent = $form_state['build_info']['args'][0];

    $add_to_collection = ($form_state['values']['add_to_collection'] == TRUE);
    if ($add_to_collection) {
      // Set array to collect nids as media nodes are created.
      $media_nids = array();
    }

    // Set up common properties for new media nodes.
    $node_base = new stdClass();
    $node_base->type = 'media';
    $node_base->uid = $user->uid;
    // Checked (value: 1) means private, thus unpublished.
    // Unchecked (value: 0) means public, thus published.
    $node_base->status = !$form_state['values']['status_override'];
    $node_base->language = LANGUAGE_NONE;
    $node_base->comment = variable_get('comment_' . $node_base->type, COMMENT_NODE_OPEN);
    $node_base->field_community[$node_base->language][0]['target_id'] = $node_parent->nid;


    if ($form_state['values']['source'] == 'upload') {

      $scheme = variable_get('file_default_scheme', 'public') . '://';

      foreach ($form_state['values']['files'] as $uploaded_file) {
        if ($uploaded_file['status'] == 'done') {

          $source = $uploaded_file['tmppath'];
          $destination = file_stream_wrapper_uri_normalize($scheme . $uploaded_file['name']);
          $destination = file_unmanaged_move($source, $destination, FILE_EXISTS_RENAME);
          $file = plupload_file_uri_to_object($destination);
          file_save($file);


          // Generate semi-pretty node title.
          $replacements = array(
            '/\..*/' => '',                     // Remove first "." and everything after.
            '/[^a-zA-Z0-9]+/' => ' ',           // Replace non letters or numbers with a single space.
            '/([a-z])([A-Z])/' => '\1 \2',      // Insert a space between a lowercase letter and an uppercase letter.
            '/([a-zA-Z])([0-9])/' => '\1 \2',   // Insert a space between a letter and a number.
            '/([0-9])([a-zA-Z])/' => '\1 \2',   // Insert a space between a number and a letter.
          );
          // In addition to above replacements, also capitalize the first letter of
          // each word, and remove leading and trailing spaces.
          $title = trim(ucwords(preg_replace(array_keys($replacements), array_values($replacements), $file->filename)));

          $node = clone $node_base;
          $node->title = $title;
          $node->field_file[$node->language][0]['fid'] = $file->fid;
          $node->field_file[$node->language][0]['display'] = 1;

          // Suppress core's file field PHP error notice.
          // @see http://drupal.org/node/1714596
          // @see http://drupal.org/node/985642
          @node_save($node);

          if ($add_to_collection) {
            $media_nids[] = $node->nid;
          }

        }
      }
    }
    elseif ($form_state['values']['source'] == 'url') {
      // TODO: support multiple?
      $url = trim($form_state['values']['urls']);

      // Grab the video's title from YT.
      $yt_data = _totem_media_get_youtube_video_data($url);
      $yt_title = $yt_data->entry->title->{'$t'};

      // TODO
      // According to YT API terms, "Your API Client may employ session-based
      // caching solely of YouTube API results" - so I don't think we can store
      // a thumbnail image source in the node.
      // @see https://developers.google.com/youtube/terms
      // Use first thumbnail because it's larger - 480x360. (The other three
      // are all 120x90.)
      //$yt_thumb_src = $yt_data->entry->{'media$group'}->{'media$thumbnail'}[0]->url;

      $node = clone $node_base;
      $node->title = $yt_title;
      // This is actually just a text field.
      $node->field_media_url[$node->language][0]['value'] = $url;

      node_save($node);

      if ($add_to_collection) {
        $media_nids[] = $node->nid;
      }
    }


    if ($add_to_collection) {
      // Since what we want to do here is essentially the same as on submit
      // of the add-to-collection form, create minimal $form and $form_state
      // arrays and manually call that form's submit function.
      // Store the community node in $form per totem_media_form_add_to_collection().
      $collection_form = array(
        '#node_parent' => $node_parent
      );
      // Pass on the values. The submit handler takes care of checking values
      // to determine if creating a new collection node or adding to existing.
      $collection_state = array(
        'values' => array(
          'nid' => $form_state['values']['collection_nid'],
          'title' => $form_state['values']['collection_title'],
          'field_media' => implode(',', $media_nids),
          // This field is not on the add_to_collection form, but we want to accommodate it.
          'status_override' => $form_state['values']['status_override'],
        ),
      );
      totem_media_form_add_to_collection_submit($collection_form, $collection_state);

      // The $form_state arg was passed by reference to the above,
      // so we can extract the redirect value that was saved to it
      // and use it for this form.
      $form_state['redirect'] = $collection_state['redirect'];
    }
  }
}
/**
 * Custom form to add media nodes to a new or existing media_collection node.
 *
 * @see totem_media.js
 */
function totem_media_form_add_to_collection($form, &$form_state) {

  global $user;

  $node_parent = _totem_common_get_community_context_node();

  if (empty($node_parent)) {
    return NULL;
  }

  // Is there any media added by this user and available to add to collections?
  // Note: we do not override includsion of the NOT_IN_COLLECTION tag, since on
  // the main Media tab, only "loose" media nodes can be selected with the
  // checkboxes to add to collection.
  // @see totem_media_node_community_media_collection()
  $media_hook_args = array(
    'return' => 'entity_id',
    'node' => $node_parent,
    'property_conditions' => array(
      array('column' => 'uid', 'value' => $user->uid),
    ),
    'page_limit' => NULL,
  );
  $media_list = module_invoke('totem_media', 'node_community_media', $media_hook_args);

  if (empty($media_list['nodes']['query']->results)) {
    // Early return nothing. Block containing this form checks for this.
    // @see totem_media_block_view_totem_common_button_add_type_alter()
    return NULL;
  }

  // Get any existing media collection records created by this user.
  $collection_hook_args = array(
    'return' => 'entity',
    'node' => $node_parent,
    'property_conditions' => array(
      array('column' => 'uid', 'value' => $user->uid),
    ),
    'page_limit' => NULL,
  );
  $collection_list = module_invoke('totem_media', 'node_community_media_collection', $collection_hook_args);
  $entities = $collection_list['nodes']['query']->results;

  $options = array();
  foreach ($entities as $ix => $meta) {
    $options[$meta->nid] = $meta->title;
  }

  // Build form.
  $form = array(
    '#node_parent' => $node_parent,
    '#submit' => array('totem_media_form_add_to_collection_submit'),
    '#attributes' => array(
      'class' => array('add-to-collection'),
    ),
  );

  // Step 1: "Add to collection" button.
  $form['step1'] = array(
    '#type' => 'fieldset',
    '#title_display' => 'invisible',
    '#title' => t('Step 1'),
    '#attributes' => array(
      'class' => array('step', 'step-1', 'visible'),
    ),
  );

  $form['step1']['init'] = array(
    '#markup' => l(
      t('Add to @collection', array('@collection' => t('Collection'))),
      '#',
      array(
        'attributes' => array(
          'class' => array('btn', 'corners'),
        ),
      )
    ),
  );

  // Step 2: checkboxes on teasers; select existing album or input new title.
  $form['step2'] = array(
    '#type' => 'fieldset',
    '#title_display' => 'invisible',
    '#title' => t('Step 2'),
    '#attributes' => array(
      'class' => array('step', 'step-2'),
    ),
  );

  $form['step2']['message'] = array(
    '#markup' => t('<div class="stepholder clearfix"><div class="one"><h3>Step 1</h3><p>Use the checkboxes next to the photos below to select the photos you want to move to an album.</p></div><div class="two"><h3>Step 2</h3><p>Once you have selected your photo(s), you can place them in a new or existing album.</p>', array('@collection' => t('Collection'))),
  );

  $form['step2']['nid'] = array(
    '#type' => 'select',
    '#title' => t('Select a collection'),
    '#options' => $options,
    '#required' => FALSE,
    '#empty_value' => 0,
    '#empty_option' => t('- New @collection -', array('@collection' => t('collection'))),
  );

  // This field is only enabled when "New collection" (empty option) is selected
  // in the dropdown. This adds clarity for user and prevents submitting both
  // a new collection title and an existing collection nid.
  $form['step2']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('New @collection title', array('@collection' => t('collection'))),
    '#suffix' => '</div>',
    '#states' => array(
      'enabled' => array(
        ':input[name=nid]' => array('value' => '0'),
      ),
    ),
  );


  // Populated via jquery click action.
  $form['step2']['field_media'] = array(
    '#type' => 'hidden',
    '#default_value' => '',
  );

  $form['step2']['actions']['submit'] = array(
    '#prefix' => '<div class="three">',
    '#type' => 'submit',
    '#value' => t('Continue'),
    '#attributes' => array(
      'class' => array('btn', 'corners'),
    ),
  );

  $form['step2']['actions']['cancel'] = array(
    '#prefix' => '<p class="cancel">',
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => drupal_get_path_alias(),
    '#attributes' => array(
      'class' => array('cancel'),
    ),
    '#suffix' => '</p></div>',
  );

  return $form;
}
/**
 * Submit handler for totem_media_form_add_to_collection().
 */
function totem_media_form_add_to_collection_submit($form, &$form_state) {

  global $user;

  if (!form_get_errors()) {
    $node = (!empty($form_state['values']['nid']) ? node_load($form_state['values']['nid']) : NULL);
    $node_parent = $form['#node_parent'];
    // Explode does not return an empty array if an empty string is passed, so handle that case explicitly.
    $field_media = !empty($form_state['values']['field_media']) ? explode(',', $form_state['values']['field_media']) : array();

    if (empty($node)) {
      $node = new stdClass();
      $node->type = 'media_collection';
      $node->uid = $user->uid;
      // When this function is directly called from totem_media_form_upload_submit(), the privacy value is passed.
      $node->status = (isset($form_state['values']['status_override']) ? !$form_state['values']['status_override'] : NODE_PUBLISHED);
      $node->language = LANGUAGE_NONE;
      $node->title = (!empty($form_state['values']['title']) ? $form_state['values']['title'] : 'Untitled');
      $node->comment = variable_get('comment_' . $node->type, COMMENT_NODE_OPEN);
      node_save($node);
    }

    // TODO: Double-check that $user->uid owns each of these ref'd nodes.
    // (For the case of Add Media modal with 'add to new collection' checked, this access check is not necessary,
    // since those are new media nodes created by that user at the same time.)
    _totem_common_set_field_entityreference_values('node', $node, 'field_media', $field_media);
    _totem_common_set_field_entityreference_values('node', $node, 'field_community', array($node_parent->nid));


    // Redirect to edit form for the new media_collection node -
    // primarily to offer chance to reorder media nodes.
    // Attach $node_view_path as destination so that node form's submit button
    // will take us back to view the new node in its community context.
    $node_view_path = ltrim(url('node/' . $node->nid, array(
        'entity_type' => 'node',
        'entity' => $node,
      )), '/');
    $form_state['redirect'] = array(
      'node/' . $node_parent->nid . '/edit-media-collection/' . $node->nid,
      array(
        'query' => array('destination' => $node_view_path),
      ),
    );

  }
}
/**
 * Implements hook_form_FORM_ID_alter().
 */
function totem_media_form_media_collection_node_form_alter(&$form, &$form_state, $form_id) {

  $node = $form_state['node'];

  // Limit access to this field to admins.
  $form['field_media']['#access'] = user_access('administer nodes');

  if (!empty($node->field_media)) {

    $form['media_dnd'] = array(
      '#type' => 'fieldset',
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      '#title' => t("Media Ordering"),
      '#description' => t("Here you may reorder the @media in this @collection via drag-and-drop.",
        array(
          '@media' => t('media items'),
          '@collection' => t('collection')
        )
      ),
      // This happens to work to place it after field_media.
      '#weight' => 10,
    );

    $form['media_dnd'] = array_merge($form['media_dnd'], totem_media_form_media_collection_node_form_sort($node, $form_state));
  }
}
/**
 * Custom sub-form containing markup for sorting a collection's child media nodes.
 * This is merged into the node form for media_collection nodes.
 */
function totem_media_form_media_collection_node_form_sort($collection_node, &$form_state) {
  // Preload jquery.ui.sortable from core.
  // Note that ui.draggable and ui.droppable libraries are not needed.
  drupal_add_library('system', 'ui.sortable');

  $subform = array();

  $media_nids = _totem_common_get_field_entityreference_values('node', $collection_node, 'field_media');
  if (!empty($media_nids)) {
    // TODO: this ultimately calls http://api.drupal.org/api/drupal/includes!entity.inc/function/DrupalDefaultEntityController%3A%3Aload/7
    // which is a confusing function but I think it will exclude or skip ones that fail to load?
    // need to test it.
    $media_nodes = node_load_multiple($media_nids);

    // Store the ids for delta comparison later.
    $form_state['field_media_orig_order'] = array_keys($media_nodes);

    // See: http://jqueryui.com/demos/sortable/#display-grid
    // We could do this with #prefix and #suffix, to be as Drupaly as possible, but is it worth it?
    $sort_markup[] = '<div class="sort-wrapper">';
    // Per jQuery UI docs, to use the "containment: 'parent'" option,
    // the parent element must have a (calculated) width and height.
    // Since the <li> elements are floated, put clearfix on parent <ul>.
    $sort_markup[] = '<ul id="media-collection-sort" class="sortable clearfix">';

    foreach (array_values($media_nodes) as $delta => $mnode) {
      $sort_markup[] = '<li id="d-' . $delta . '">';
      $sort_markup[] = drupal_render(node_view($mnode, 'teaser_sort'));
      $sort_markup[] = '</li>';
    }

    $sort_markup[] = '</ul>';
    $sort_markup[] = '</div>';

    $subform['sort_area'] = array(
      '#markup' => implode("\n", $sort_markup),
    );

    // Before submit, this will be populated with the order of the deltas,
    // based on the id attribute of the <li> elements containing the media node teasers.
    $subform['field_media_new_deltas'] = array(
      '#type' => 'hidden',
      '#default_value' => '',
    );

  }

  return $subform;
}
/**
 * Implements hook_field_attach_submit().
 *
 * Since we need to make an adjustment to field values before save,
 * use this hook, rather than other possible ways
 * e.g. a custom submit function for node form or hook_node_submit().
 * Also note that core uses field_attach_submit() for its own drag-and-drop reordering.
 *
 * @see node_form_submit_build_node()
 */
function totem_media_field_attach_submit($entity_type, $entity, $form, &$form_state) {

  if ($entity_type == 'node' && $entity->type == 'media_collection') {

    // It's possible to create an empty collection, which won't even include
    // the form elements for sorting.
    if (!empty($form_state['field_media_orig_order']) && !empty($form_state['values']['field_media_new_deltas'])) {

      $orig = $form_state['field_media_orig_order'];
      $new_deltas = explode(',', $form_state['values']['field_media_new_deltas']);

      // Try to sure that the POSTed field_media_new_deltas makes sense (wasn't hacked);
      // then perform the reorder and assign new array as field value.

      if (count($new_deltas) == count($orig)) {
        // Are all the keys (deltas) in the original array accounted for in $new_deltas?
        if (count(array_intersect_key($orig, array_flip($new_deltas))) == count($orig)) {
          // NOTE: by overwriting whatever got saved to $entity from the node_form field
          // with the nids in our custom $orig array, we effectively prevent
          // editing the node field values via the input on node_form.
          $new_field = array();
          foreach ($new_deltas as $delta) {
            $new_field[] = array('target_id' => $orig[$delta]);
          }
          $entity->field_media[$entity->language] = $new_field;
        }
      }

    }
  }

}

/**
 * Custom form to confirm removing a media node from a collection.
 *
 * @see totem_common_form_node_remove()
 * @see node_delete_confirm()
 */
function totem_media_confirm_remove_from_collection($form, &$form_state) {
  $media_node = $form_state['build_info']['args'][0];
  $collection_node = $form_state['build_info']['args'][1];

  $form['#node'] = $media_node;
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $media_node->nid
  );

  // Use standard Drupal confirmation form.
  // Cancellation path *should* be grabbed from 'destination' query param on the removal link,
  // but we don't know whether we were viewing or editing the collection node.
  // TODO: make it just close the modal
  return confirm_form(
    $form,
    t("Confirm removal from @collection", array('@collection' => t("collection"))),
    'node/' . $collection_node->nid,
    t("Are you sure you want to remove %title from the @collection %collection? This action cannot be undone.",
      array(
        '%title' => $media_node->title,
        '@collection' => t('collection'),
        '%collection' => $collection_node->title
      )
    ),
    t("Remove"),
    t("Cancel")
  );
}
/**
 * Submit handler for totem_media_confirm_remove_from_collection().
 */
function totem_media_confirm_remove_from_collection_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $nid_to_remove = $form_state['values']['nid'];
    $collection_node = $form_state['build_info']['args'][1];
    // TRUE parameter indicates to remove the specified nids.
    _totem_common_set_field_entityreference_values('node', $collection_node, 'field_media', array($nid_to_remove), TRUE);
    drupal_set_message(t("%title has been removed from the @collection %collection.",
      array(
        '%title' => $form['#node']->title,
        '@collection' => t('collection'),
        '%collection' => $collection_node->title,
      )
    ));
  }
}
/**
 * Implements hook_form_FORM_ID_alter() for 'search_block_form'.
 *
 * @see totem_search_form_search_block_form_alter()
 */
function totem_media_form_search_block_form_alter(&$form, &$form_state) {
  if (module_exists('totem_search') && variable_get('totem_media_hide_collections', FALSE)) {
    // Use an #after_build function so we can alter an element created by
    // totem_search's implementation of this same form alter hook.
    $form['#after_build'][] = '_totem_media_search_form_after_build_hide_collections';
  }
}
/**
 * Implements hook_form_FORM_ID_alter() for 'search_form'.
 *
 * @see totem_search_form_search_form_alter()
 */
function totem_media_form_search_form_alter(&$form, &$form_state) {
  if (module_exists('totem_search') && variable_get('totem_media_hide_collections', FALSE)) {
    // Use an #after_build function so we can alter an element created by
    // totem_search's implementation of this same form alter hook.
    $form['#after_build'][] = '_totem_media_search_form_after_build_hide_collections';
  }
}
/**
 * An #after_build function for both versions of the search form.
 */
function _totem_media_search_form_after_build_hide_collections($form, $form_state) {
  if (!empty($form['advanced'])) {
    $elem = &$form['advanced']['type'];
  }
  else {
    $elem = &$form['type'];
  }

  unset($elem['#options']['media_collection']);

  if ($elem['#type'] == 'checkboxes') {
    // These have already been #processed into separate form elements.
    unset($elem['media_collection']);
  }

  return $form;
}