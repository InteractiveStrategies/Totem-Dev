<?php
/**
 * @file
 * totem_media.theme.inc
 */

/**
 * Implements MODULE_preprocess_html().
 */
function totem_media_preprocess_html(&$vars) {
  // Add class to body so any css rules can be specific to gallery mode
  // or non-gallery mode as necessary.
  $vars['classes_array'][] = (variable_get('totem_media_gallery_mode') == TRUE) ? 'totem-media-gallery-on' : 'totem-media-gallery-off';
}

/**
 * Implements MODULE_preprocess_page().
 */
function totem_media_preprocess_page(&$vars) {

  // Add module UI assets
  if (!$vars['is_admin']) {
    drupal_add_css(drupal_get_path('module', 'totem_media') . '/theme/totem_media.css');

    drupal_add_js(drupal_get_path('module', 'totem_media') . '/theme/totem_media.js', array(
      'scope' => 'footer',
    ));
  }

  // Adjustments for page--community.tpl with local task.
  if (!empty($vars['node']) && $vars['node']->type == 'community' && $vars['is_community_tab']) {

    // Move the Add Collection button/form block to just above the Media list.
    // Note that sidebar_first region blocks have been moved to content region,
    // and that blocks with "action" class have been extracted into a separate
    // variable, $vars['blocks_action'].
    // @see totem_common_preprocess_page()
    if (variable_get('totem_media_gallery_mode') && !empty($vars['blocks_action'])) {
      $region_content = &$vars['page']['content']['content']['content'];

      // Figure out deltas.
      foreach ($vars['blocks_action'] as $delta => $render_block) {
        if (in_array('action-media-collection', $render_block['#classes_array'])) {
          $delta_mc_btn = $delta;
        }
        elseif (in_array('action-media', $render_block['#classes_array'])) {
          $delta_m_list = str_replace('button_add', 'list', $delta);
        }
      }

      if (isset($delta_mc_btn) && isset($delta_m_list) && isset($region_content[$delta_m_list])) {
        // Move button into content region.
        $region_content[$delta_mc_btn] = $vars['blocks_action'][$delta_mc_btn];
        unset($vars['blocks_action'][$delta_mc_btn]);

        // Fix weights - Add Collection button goes right before Media list.
        // (Media list appears last regardless, so no harm increasing its weight.)
        $region_content[$delta_m_list]['#weight']++;
        $region_content[$delta_mc_btn]['#weight'] = $region_content[$delta_m_list]['#weight'] - 1;

        // Make drupal_render() re-sort.
        $region_content['#sorted'] = FALSE;
      }
    }
  }
}
/**
 * Implements MODULE_preprocess_node().
 */
function totem_media_preprocess_node(&$vars) {

  $node = &$vars['node'];

  if ($node->type == 'media') {

    if (($vars['view_mode'] == 'teaser' || $vars['view_mode'] == 'teaser_sort') && variable_get('totem_media_gallery_mode') == FALSE) {
      // Generate the file icon to place by node title.
      $vars['file_icon'] = theme('file_icon', array(
        'file' => $vars['elements']['field_file'][0]['#file']
      ));
    }

    if ($vars['view_mode'] == 'teaser') {

      $checkbox = array(
        '#name' => 'media',
        '#type' => 'checkbox',
        '#title' => $node->title,
        '#return_value' => $node->nid,
        '#access' => node_access('update', $node),
        '#prefix' => '<div class="album-checkbox">',
        '#suffix' => '</div>',
      );

      $vars['title_prefix']['media'] = $checkbox;

      if (variable_get('totem_media_gallery_mode') == TRUE) {
        // Manually add modal class, because media nodes' URLs do not include
        // 'modal/' which triggers adding the class and processing in other cases.
        // @see totem_common_preprocess_node(), _totem_common_modal_link_attributes_ensure()
        if (stristr($vars['node_url'], 'media-view/')) {
          $vars['node_url_attributes']['class'][] = 'ctools-use-modal';
          // Re-run for sake of customized attributes.
          _totem_common_modal_link_attributes_ensure($vars['node_url'], $vars['node_url_attributes']);
        }

        // Add class to invoke custom CTools modal settings.
        /*
         * @see node--media.tpl.php
         */
        $vars['node_url_attributes']['class'][] = 'ctools-modal-media-lightbox';

        // Pass in some custom settings to override those used when opening modal.
        // The key 'media-lightbox' corresponds to class on link, 'ctools-modal-media-lightbox'.
        // Default settings are passed to js in _totem_common_modal_link().
        // NOTE: this must happen here, not in any PHP that runs upon the AJAX
        // call to load the modal's content, so the settings exist when the
        // link to open modal is initially clicked.
        $lightbox_modal_settings = array(
          'media-lightbox' => array(
            'modalSize' => array(
              'width' => .9,
              'height' => .9,
              // We have 0 padding on .modal-content, so remove parameter
              // that sizes .modal-content narrower than modal's width.
              // (This defaults to 25 in ctools/js/modal.js.)
              'contentRight' => 0,
            ),
            // This custom 'reuse' key indicates settings only to be
            // checked/used in our (custom) modal reuse system.
            'reuse' => array(
              'keepTitle' => TRUE,
            ),
          ),
        );
        drupal_add_js($lightbox_modal_settings, 'setting');
      }
    }
    elseif ($vars['view_mode'] == 'full') {
      // This seems to be in contextual links already?
      if (isset($vars['content']['links']['#links']['privatemsg_link'])) {
        unset($vars['content']['links']['#links']['privatemsg_link']);
      }

      if (variable_get('totem_media_gallery_mode') == TRUE) {
        // TODO: remove contextual links from full view? If user follows these
        // links to another modal form, the form submit will reload the page
        // and thus we lose viewing the node in modal. (This is particularly
        // annoying for node edit case.)
        // Also note: contextual links are wacky... you can unset them all,
        // but not individual ones here.
        // unset($vars['content']['contextual_links']);

        // Remove jump-link to the comment form, since it should always be visible
        // below the comment thread. Also the href is wrong anyway.
        unset($vars['content']['links']['comment']['#links']['comment-add']);

        // Fix for videos not playing (unless natively) when media nodes are viewed in modal.
        // See mediaelement-and-player.js, line 774:
        // mejs.MediaElementDefaults object calls mejs.Utility.getScriptPath()
        // to figure out where files within the library are,
        // and saves result to 'pluginPath' option which is used in
        // the path to the swf file in the <embed> element markup.
        // getScriptPath() looks at <script> DOM elements within document.
        // When the node markup is generated upon an AJAX request,
        // ajax_render() pulls in the necessary js files,
        // but apparently when getScriptPath() runs it doesn't find
        // these added mediaelement js files at all.
        // Thus 'pluginPath' is an empty string,
        // and the <embed> src attribute is incorrect.
        // So, we pass 'pluginPath' here into Drupal.settings
        // to override the default value.
        foreach (element_children($vars['content']['field_file']) as $i) {
          $file = &$vars['content']['field_file'][$i]['file'];
          if (isset($file['#attached']) && isset($file['#attached']['library'])) {
            // See mediaelement_field_formatter_view() regarding this structure.
            $uses_mediaelement = array_search(array('mediaelement', 'mediaelement'), $file['#attached']['library']) !== FALSE;
            if ($uses_mediaelement) {
              // Note: libraries_get_path() is not a core function;
              // it's defined by the Libraries module.
              // Since mediaelement module depends on it, we're safe to use it.
              $library_path = libraries_get_path('mediaelement');
              $class_key    = '.' . $file['#attributes']['class'];
              // We can safely hardcode 'build' into the path - see mediaelement_library().
              $file['#attached']['js'][0]['data']['mediaelement'][$class_key]['opts']['pluginPath'] = '/' . $library_path . '/build/';

              // NOTE: see mediaelement.js - the attach function checks 'controls'
              // (a js setting adjacent to the 'opts' we modify above),
              // and the 'opts' are only passed if 'controls' is TRUE.
              // This makes NO sense whatsoever, but it does mean that
              // 'controls' set to FALSE would break our fix here.
              // The 'controls' value is taken from the file display setting
              // at /admin/config/media/file-types/manage/video/file-display
              // and passed to js in mediaelement_field_formatter_view().
            }
          }
        }

        if (isset($_GET['grouping'])) {
          // Figure out previous and next links for paging through items in lightbox.

          $mnids = array();

          if (is_numeric($_GET['grouping'])) {
            $collection_nid                  = (int) $_GET['grouping'];
            $vars['grouping_collection_nid'] = $collection_nid;
            $mnids                           = _totem_common_get_field_entityreference_values('node', node_load($collection_nid), 'field_media');
          }
          elseif ($_GET['grouping'] == 'loose') {
            $community_node = _totem_common_get_community_context_node();
            if ($community_node) {
              $vars['grouping_loose_community_nid'] = $community_node->nid;

              $efq_params = array(
                'return' => 'entity_id',
                'entity_type' => 'node',
                'bundle' => array('media'),
                'field_conditions' => array(
                  array('field' => 'field_community', 'column' => 'target_id', 'value' => $community_node->nid),
                ),
                'property_order_by' => array(
                  array('column' => 'changed', 'direction' => 'DESC'),
                ),
                'tags' => array('NOT_IN_COLLECTION'),
              );

              $mnids = _totem_common_efq($efq_params)->results;
            }
          }


          if (!empty($mnids)) {
            // $delta should be either actual field delta (in case of a collection)
            // or ordering of loose media nodes by last-changed
            foreach ($mnids as $delta => $mnid) {
              if ($mnid == $node->nid) {
                if (isset($mnids[$delta - 1])) {
                  $prev = node_load($mnids[$delta - 1]);
                  if ($prev) {
                    $prev_uri = entity_uri('node', $prev);
                    // Explicitly remove anything else that may be in the query, since this link will be handled by AJAX.
                    $prev_uri['options']['query'] = array('grouping' => $_GET['grouping']);
                    // Pass entity type and entity object into $options, which is critical for our url_outbound_alters
                    $vars['prev_url'] = url($prev_uri['path'], $prev_uri['options']);
                  }
                }
                if (isset($mnids[$delta + 1])) {
                  $next = node_load($mnids[$delta + 1]);
                  if ($next) {
                    $next_uri = entity_uri('node', $next);
                    // Explicitly remove anything else that may be in the query, since this link will be handled by AJAX.
                    $next_uri['options']['query'] = array('grouping' => $_GET['grouping']);
                    // Pass entity type and entity object into $options, which is critical for our url_outbound_alters
                    $vars['next_url'] = url($next_uri['path'], $next_uri['options']);
                  }
                }

                break;
              }
            }

          }
        }

      }

    }
  }
  elseif ($node->type == 'media_collection') {

    if ($vars['view_mode'] == 'teaser') {
      // Get number of items included in this collection.
      // Since we clean up references upon deletion of an entity,
      // this number taken from the $node object should be accurate.
      // @see totem_common_entity_delete()
      $vars['num_items'] = empty($node->field_media) ? 0 : count($node->field_media[$node->language]);
    }
    elseif ($vars['view_mode'] == 'full') {

      if (!empty($node->field_media)) {
        // Simplify field_media for sake of themers in template.
        $media_nodes = array();
        // This field in the $content array is populated with
        // entityreference view settings of 'rendered entity' in teaser mode
        // Structure:
        // $vars['content']['field_media'][0]['node'][$nid] => (render array for node)
        foreach (element_children($vars['content']['field_media']) as $delta) {
          $child_array = $vars['content']['field_media'][$delta]['node'];
          unset($child_array['#sorted']);
          // Now $child_array is an array of nid => actual node render array.
          // Simplify one more level and make media_nodes easily loopable.
          $child_array = array_shift($child_array);
          // Preserve delta order - it's the user's chosen ordering.
          $media_nodes[$delta] = $child_array;
        }
        // And don't let render() resort them!
        $media_nodes['#sorted'] = TRUE;

        $vars['content']['media_nodes'] = $media_nodes;

        unset($vars['content']['field_media']);
      }

      // If submit info is set to be displayed, make sure that only happens
      // with gallery mode enabled, to keep consistent with other types
      // in non-gallery mode.
      // @see template_preprocess_node()
      $vars['display_submitted'] = $vars['display_submitted'] && variable_get('totem_media_gallery_mode');

      // Note: in non-gallery mode, media collection pages look basically like
      // every other node type page.
      if (variable_get('totem_media_gallery_mode') == TRUE) {
        // Move contextual links back to before title due to full-width layout.
        // @see totem_common_preprocess_node()
        $vars['title_prefix']['contextual_links'] = $vars['content']['contextual_links'];
        unset($vars['content']['contextual_links']);
      }

    }
  }
}
/**
 * Implements MODULE_preprocess_block().
 */
function totem_media_preprocess_block(&$vars) {
  // If gallery mode, avoid sticking pretty scroll onto Media and
  // Media Collection list blocks, because of special Media tab layout.
  // NOTE: this method results in a bit of a lie on block admin screen,
  // since the block_class db table still has custom-scroll class
  // assigned to these blocks. But checking for the type class ('list-____')
  // is easy here.
  if (variable_get('totem_media_gallery_mode') == TRUE) {
    if (in_array('list-media-collection', $vars['classes_array'])
            || in_array('list-media', $vars['classes_array'])) {
      $scs_key = array_search('custom-scroll', $vars['classes_array']);
      if ($scs_key !== FALSE) {
        unset($vars['classes_array'][$scs_key]);
      }
    }
  }

}
/**
 * Implements MODULE_preprocess_totem_activity_recent_entity().
 */
function totem_media_preprocess_totem_activity_recent_entity(&$vars) {

  $node = $vars['node'];

  if ($node->type == 'media') {
    // Override the node type name with a more specific file type name.
    // Note: since we don't actually need to render the file/media URL field for
    // this view mode, we check the $node object instead of field render arrays.
    $type = NULL;
    if (!empty($node->field_file) && !empty($node->field_file[LANGUAGE_NONE][0]['type'])) {
      $type = $node->field_file[LANGUAGE_NONE][0]['type'];
    }
    elseif (!empty($node->field_media_url) && !empty($node->field_media_url[LANGUAGE_NONE][0]['value'])) {
      // We only support YouTube links currently with this field, so we can
      // safely hardcode the type.
      $type = 'video';
    }

    switch ($type) {
      case 'image':
        $type_name = t('photo');
        break;

      case 'video':
        $type_name = t('video');
        break;

      case 'audio':
        $type_name = t('audio file');
        break;

      case 'default':
      default:
        // pdf, txt, doc, docx, xls, xlsx, ppt, pptx
        $type_name = t('document');
        break;
    }

    $vars['content_type'] = $type_name;
  }
}
/**
 * Template preprocessor for theme('totem_media_youtube_embed') calls.
 */
function template_preprocess_totem_media_youtube_embed(&$vars) {
  $vars['yt_id'] = '';

  // TODO: validate url?
  // TODO: url parse duped in _totem_media_get_youtube_video_data()
  // ...maybe the field should just hold the ID piece?
  $url_parts = drupal_parse_url($vars['yt_url']);
  if (!empty($url_parts['query']['v'])) {
    $vars['yt_id'] = $url_parts['query']['v'];
  }
}