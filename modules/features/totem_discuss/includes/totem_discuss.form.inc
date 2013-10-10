<?php
/**
 * @file
 * totem_discuss.form.inc
 */

// Hook implementations.
/**
 * Implements hook_form_FORM_ID_alter() for topic_node_form().
 */
function totem_discuss_form_topic_node_form_alter(&$form, &$form_state, $form_id) {
  // Remove "leave shadow copy" option.
  unset($form['shadow']);
}
/**
 * Implements hook_form_FORM_ID_alter() for comment_form().
 */
function totem_discuss_form_comment_form_alter(&$form, &$form_state, $form_id) {

  global $user;

  // Only allow commenting if user is a member of this community.
  // PERMISSION.
  $form['#access'] = (_totem_user_has_community($user, $form['#node']) || user_access('administer comments'));

  // Make sure comment submit action keeps current node community context.
  $form['#action'] .= '?destination=' . drupal_get_path_alias();
  $form['#submit'][] = 'totem_discuss_form_comment_form_submit';

  // Add flag for new comment Replies.
  $form['new_reply'] = array(
    '#type' => 'value',
    '#value' => empty($form_state['comment']->cid) && !empty($form_state['comment']->pid),
  );

  // Hide author stuff completely.
  hide($form['author']);


  // Attach #ajax properties to submit button.
  // Figure out which case of this form we're on.
  $args = arg();
  $cid = $form['cid']['#value'];
  $pid = $form['pid']['#value'];

  // Editing an existing comment.
  $is_edit = $form['cid'] && array_search('comment', $args) !== FALSE && array_search('edit', $args) !== FALSE;
  // Replying to comment.
  $is_reply = $form['cid'] && $form['pid'] && array_search('comment', $args) !== FALSE && array_search('reply', $args) !== FALSE;
  // Straight-up posting a new comment.
  $is_new = !$is_edit && !$is_reply;

  // Attach the community context node to the standard system/ajax path,
  // so we can still take advantage of core's ajax stuff, plus make sure the
  // community is defined for usage in contextual link paths and access checks.
  $community_node = _totem_common_get_community_context_node();

  // Store it in $form array for use in custom submit handler.
  // This is similar to how the node being commented on is stored -
  // @see comment_form()
  $form['#community_node'] = $community_node;
  
  if(!empty($community_node)) {
  // @see http://api.drupal.org/api/drupal/includes!ajax.inc/group/ajax/7
  if ($is_new) {
    $form['actions']['submit']['#ajax'] = array(
      'path' => 'system/ajax/' . $community_node->nid,
      'callback' => '_totem_discuss_ajax_comment_new_cb',
      // @see comment-wrapper--totem.tpl.php
      'wrapper' => 'comments-inner',
      'effect' => 'fade',
    );
  }
  elseif ($is_edit) {
    $form['actions']['submit']['#ajax'] = array(
      'path' => 'system/ajax/' . $community_node->nid,
      'callback' => '_totem_discuss_ajax_comment_edit_cb',
      'wrapper' => 'comment-form',
      'effect' => 'fade',
    );

    $form['comment_body'][LANGUAGE_NONE][0]['#title'] = t("Edit comment");
  }
  elseif ($is_reply) {
    $form['actions']['submit']['#ajax'] = array(
      'path' => 'system/ajax/' . $community_node->nid,
      'callback' => '_totem_discuss_ajax_comment_reply_cb',
      'effect' => 'fade',
    );

    // This has already been expanded or something? what is 'text_format'?
    $form['comment_body'][LANGUAGE_NONE][0]['#title'] = t("Reply");
  }

  $form['actions']['submit']['#value'] = t("Post");
  unset($form['actions']['preview']);

  // Insert a wrapper to hold form errors so AJAX callbacks can insert
  // them in a consistent place in the markup.
  $form['errors_wrapper'] = array(
    '#markup' => '<div id="ajax-errors-' . $form['#id'] . '" class="ajax-errors"></div>',
    // Place just before actions, which is weighted 100.
    // @see system_element_info()
    '#weight' => 99,
  );

  if ($is_reply || $is_edit) {
    // Attach a cancel link which will not actually link anywhere,
    // just jQuery click handler to remove the form.
    $form['actions']['cancel'] = array(
      '#type' => 'link',
      '#title' => t("Cancel"),
      '#href' => 'comment/' . (int) $cid . '/cancel/' . ($is_reply ? 'reply' : ($is_edit ? 'edit' : '')),
      '#attributes' => array(
        'class' => array('comment-cancel-action'),
      ),
    );
  }
  }

}
/**
 * Custom submit function for comment form.
 *
 * Notify authors of replies to their comments via private message.
 * Attach user picture to comment object in $form_state.
 *
 * @global type $user
 * @param type $form
 * @param type $form_state
 */
function totem_discuss_form_comment_form_submit($form, &$form_state) {
  
  if (!form_get_errors()) {
    if ($form_state['values']['new_reply']) {
      $node = $form['#node'];
      $node_parent = $form['#community_node'];

      // Send to pid author.
      $parent = comment_load($form_state['comment']->pid);
      $recipients[] = user_load($parent->uid);
      $subject = t('"@title" - new reply', array('@title' => $node->title));
      $body = t('<p>A reply to your comment on !title has been posted in the @title_parent @community.</p>', array(
        '!title' => l($node->title, 'node/' . $node->nid, array(
          'absolute' => TRUE,
          'entity_type' => 'node',
          'entity' => $node,
          'entity_context_community' => $node_parent,
        )),
        '@title_parent' => $node_parent->title,
        '@community' => t('Community'),
      ));

      $message = privatemsg_new_thread($recipients, $subject, $body);
      if ($message['message'] !== FALSE && $message['success'] == TRUE) {
        drupal_set_message(check_plain(t('@name has been notified.', array('@name' => $recipients[0]->realname))));
      }

      foreach ($message['messages'] as $type => $group) {
        foreach ($group as $m) {
          drupal_set_message(check_plain($m), $type);
        }
      }
    }

    // Many properties are attached to $comment during comment_submit() and
    // comment_save(), which are both called within comment_form_submit()
    // process. However, $comment->picture is only added when loading a comment
    // object by joining to the user table, via CommentController::buildQuery().
    // This never happens before the AJAX submits render new and reply comments,
    // resulting in default user picture even if the comment author has one.
    // Just like initial authorship properties are initially assigned,
    // we use logged-in user to attach picture.
    // @see comment_form()
    global $user;
    $form_state['comment']->picture = $user->picture;
  }
}

// AJAX callbacks for comment_form submit button.
/**
 * Common error-checking helper function for comment submit AJAX callbacks.
 *
 * Checks new, edit, and reply submits for form errors and adds AJAX command to
 * insert new error markup or clears any prior.
 * NOTE: This does not visibly mark the erroneous field (e.g. red border),
 * but since we only have one field, a message should be sufficient.
 *
 * @param array $form
 * The form structure.
 * @param array $form_state
 * The current form state.
 * @param array $commands
 * An array of AJAX commands to be added to.
 *
 * @return boolean
 * Whether any form errors exist.
 */
function _totem_discuss_ajax_comment_error_check($form, $form_state, &$commands) {
  if (form_get_errors()) {
    // If there are errors, we just want to display those.
    $commands[] = ajax_command_html('#ajax-errors-' . $form['#id'], theme('status_messages', array('display' => 'error')));
    return TRUE;
  }
  else {
    // Just clear any error message markup from previous submits.
    $commands[] = ajax_command_html('#ajax-errors-' . $form['#id'], "");
  }
  return FALSE;
}
/**
 * AJAX callback for new (non-reply) comment form submit.
 *
 * @param array $form
 * The form structure.
 * @param array $form_state
 * The current form state.
 *
 * @return array
 * An render array of AJAX commands to insert the new comment and any messages.
 */
function _totem_discuss_ajax_comment_new_cb($form, $form_state) {
  $commands = array();

  // Check for errors. Commands array will be modified accordingly.
  // If errors, the helper populates all necessary commands, and we don't
  // want to add the comment (or lack thereof) to the page markup.
  // So we only continue otherwise.
  if (!_totem_discuss_ajax_comment_error_check($form, $form_state, $commands)) {
    // In case this is the first comment on the node, make sure to remove this
    // class which hides the header and wrapper.
    $commands[] = ajax_command_invoke('#comments', 'removeClass', array('no-comments'));

    $comment_render_array = comment_view($form_state['comment'], $form['#node']);

    // Just above the comment form, print the new comment. Note that
    // #ajax['wrapper'] is 'comments-inner'.
    $commands[] = ajax_command_append(NULL, drupal_render($comment_render_array));
    // Then print any status messages. Wrap messages so that jQuery animations
    // can operate on single DOM element.
    $messages = '<div id="ajax-comment-messages">' . theme('status_messages') . '</div>';
    $commands[] = ajax_command_append(NULL, $messages);
    // Clear the comment form's textarea.
    $commands[] = ajax_command_invoke('#edit-comment-body-und-0-value', 'val', array(""));
  }

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}
/**
 * AJAX callback for comment-edit form submit.
 *
 * @param array $form
 * The form structure.
 * @param array $form_state
 * The current form state.
 *
 * @return array
 * An render array of AJAX commands to insert the new comment and any messages.
 */
function _totem_discuss_ajax_comment_edit_cb($form, $form_state) {
  $commands = array();

  // Check for errors. Commands array will be modified accordingly.
  // If errors, the helper populates all necessary commands, and we don't
  // want to add the comment (or lack thereof) to the page markup.
  // So we only continue otherwise.
  if (!_totem_discuss_ajax_comment_error_check($form, $form_state, $commands)) {

    $comment = $form_state['comment'];
    $comment_render_array = comment_view($comment, $form['#node']);

    // Remove the old version of the comment from the markup
    // (which is temporarily kept, hidden, in case of user clicking Cancel).
    $commands[] = ajax_command_remove('#comment-' . $comment->cid);
    // Replace the edit form with the newly saved comment.
    // This wrapper is added via $form['#prefix'] and $form['#suffix'].
    $commands[] = ajax_command_replace('#inline-comment-action-form', drupal_render($comment_render_array));
    // Show the messages after it. Wrap messages so that jQuery animations can
    // operate on single DOM element.
    $messages = '<div id="ajax-comment-messages">' . theme('status_messages') . '</div>';
    $commands[] = ajax_command_after('#comment-' . $comment->cid, $messages);
  }

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}
/**
 * AJAX callback for comment-reply form submit
 *
 * @param array $form
 * The form structure.
 * @param array $form_state
 * The current form state.
 *
 * @return array
 * An render array of AJAX commands to insert the new comment and any messages.
 */
function _totem_discuss_ajax_comment_reply_cb($form, $form_state) {
  $commands = array();

  // Check for errors. Commands array will be modified accordingly.
  // If errors, the helper populates all necessary commands, and we don't
  // want to add the comment (or lack thereof) to the page markup.
  // So we only continue otherwise.
  if (!_totem_discuss_ajax_comment_error_check($form, $form_state, $commands)) {

    $comment = $form_state['comment'];

    // Remove the comment form used for the reply.
    // This wrapper is added via $form['#prefix'] and $form['#suffix'].
    $commands[] = ajax_command_remove('#inline-comment-action-form');

    // Pass info to Drupal.attachBehaviors() settings arg, so attach functions
    // can find the new reply.
    // @see Drupal.behaviors.totem_discuss.ajaxCommentingScrollIntoView()
    $commands[] = ajax_command_settings(array(
      'ajaxed_comment_cid' => $comment->cid,
      'ajax_comment_reply' => TRUE,
    ));

    // Determine where in markup to insert new reply.
    // NOTE: This logic is based on ascending sort order -- i.e. most recent
    // comment last, hence needing to append a new reply after any existing
    // child comments of the same parent. Reliance on ascending order is fairly
    // stable, since descending (most recent first) was actually removed as an
    // option in D7 (and they're still arguing whether to reintroduce into D8).
    // @see http://drupal.org/node/191499
    // Does this parent have any other child comments yet?
    $result = db_query("
      SELECT cid
      FROM {comment}
      WHERE status = 1
        AND pid = :pid
        AND cid <> :this_cid
    ", array(
      ':pid' => $comment->pid,
      ':this_cid' => $comment->cid
    ));

    if ($result->rowCount() > 0) {
      // If there are already child comments, append it to the indented wrapper.
      $comment_render_array = comment_view($comment, $form['#node']);
      // This ID is added via totem_discuss_preprocess_comment().
      $commands[] = ajax_command_append('#sub-comment-' . $comment->pid, drupal_render($comment_render_array));
    }
    else {
      // Otherwise, it's the first child of this parent, so place it after
      // parent. Note that comment_prepare_thread() does not run here, so
      // manually specify divs property for proper indentation markup in
      // #prefix and #suffix.
      $comment->divs = 1;
      $comment->divs_final = 1;
      $comment_render_array = comment_view($comment, $form['#node']);
      $commands[] = ajax_command_after('#comment-' . $comment->pid, drupal_render($comment_render_array));
    }

    // Print the status messages. Wrap messages so that jQuery animations can
    // operate on single DOM element.
    $messages = '<div id="ajax-comment-messages">' . theme('status_messages') . '</div>';
    $commands[] = ajax_command_after('#comment-' . $comment->cid, $messages);
  }

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Implements hook_form_FORM_ID_alter() for comment_confirm_delete().
 */
function totem_discuss_form_comment_confirm_delete_alter(&$form, &$form_state, $form_id) {

  // Attach the community context node to the standard system/ajax path, so we
  // can still take advantage of core's ajax stuff, plus make sure the community
  // is defined for usage in contextual link paths and access checks.
  $community_node = _totem_common_get_community_context_node();

  // The menu callback calls module_load_include() for comment.admin.inc, but
  // that doesn't persist to form processing, and we need it because it includes
  // comment_confirm_delete_submit().
  form_load_include($form_state, 'inc', 'comment', 'comment.admin');

  $form['actions']['submit']['#ajax'] = array(
    // Utilize system's ajax path (which has functions set up to handle the
    // response), but attach community nid for context checks later (mainly
    // access).
    'path' => 'system/ajax/' . $community_node->nid,
    'callback' => '_totem_discuss_ajax_comment_delete_cb',
    'effect' => 'slide',
  );

  // $form['actions']['cancel'] exists as a simple link back to node/$nid.
  // Override it to match comment_form(); just jQuery click handler to remove
  // the form.
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t("Cancel"),
    '#href' => 'comment/' . $form['#comment']->cid . '/cancel/delete',
    '#attributes' => array(
      'class' => array('comment-cancel-action'),
    ),
  );

}
/**
 * AJAX callback for comment delete-confirm submit.
 *
 * @param array $form
 * The form structure.
 * @param array $form_state
 * The current form state.
 *
 * @return array
 * An render array of AJAX commands to insert the new comment and any messages.
 */
function _totem_discuss_ajax_comment_delete_cb($form, $form_state) {
  // $form_state['comment'] does not exist in this case.
  $comment = $form['#comment'];

  $commands = array();

  // Replace the form with status messages.
  // This wrapper is added via $form['#prefix'] and $form['#suffix'].
  // Wrap messages so that jQuery animations can operate on single DOM element.
  $messages = '<div id="ajax-comment-messages">' . theme('status_messages') . '</div>';
  $commands[] = ajax_command_replace('#inline-comment-action-form', $messages);

  // Mark the comment immediately with a 'deleted' class (to aid JS) and
  // "remove" it.
  $commands[] = ajax_command_invoke('#comment-' . $comment->cid, 'addClass', array('comment-ajax-deleted'));
  $commands[] = ajax_command_invoke('#comment-' . $comment->cid, 'slideUp');

  // Also remove any children of this comment since, according to the form's
  // warning, they're also deleted. (This element will not exist in DOM, but
  // calling the jquery func on it will not hurt.)
  $commands[] = ajax_command_remove('#sub-comment-' . $comment->cid);

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

// Custom forms.
/**
 * Modal node report forms.
 */
function totem_discuss_form_node($op, $node, $node_parent, $type) {

  global $user;
  $account = user_load($user->uid);
    
  $title = NULL;
  $form_id = NULL;
  $args = array(
    'type' => $type,
    'op' => $op,
    'node' => $node,
    'node_parent' => $node_parent,
    'account' => $account,
  );

  switch ($op) {
    case 'report':
      $title = t('Report abuse');
      $form_id = 'totem_discuss_form_node_report';
      break;
  }


  $form_state = array(
    'title' => $title,
    'ajax' => TRUE,
    'build_info' => array(
      'args' => $args,
    ),
  );

  ctools_include('modal');
  ctools_include('ajax');

  $output = ctools_modal_form_wrapper($form_id, $form_state);

  if (!empty($form_state['executed'])) {
    ctools_add_js('ajax-responder');

    // Overwrite the form output.
    $output = array();
    $output[] = ctools_ajax_command_reload();
  }

  print ajax_render($output);
  exit;
}
/**
 * Custom form to report abuse.
 */
function totem_discuss_form_node_report($form, &$form_state) {

  $op = $form_state['build_info']['args']['op'];
  $node = $form_state['build_info']['args']['node'];
  $node_parent = $form_state['build_info']['args']['node_parent'];
  $account = $form_state['build_info']['args']['account'];
  $type = $form_state['build_info']['args']['type'];

  // Build form.
  $form = array(
    '#submit' => array('totem_discuss_form_node_report_submit'),
    '#op' => $op,
    '#node' => $node,
    '#node_parent' => $node_parent,
    '#account' => $account,
    '#objtype' => $type,
  );

  $form['message'] = array(
    '#title' => t('Message to @community managers', array('@community' => t('Community'))),
    '#type' => 'textarea',
  );

  $form['actions'] = array(
    '#type' => 'actions',
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Report'),
  );

  $options = drupal_parse_url(urldecode($_GET['destination']));
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => $options['path'],
    '#options' => $options,
  );

  return $form;
}
/**
 * Submit action for totem_discuss_form_node_report.
 */
function totem_discuss_form_node_report_submit($form, &$form_state) {

  if (!form_get_errors()) {
    $op = $form['#op'];
    $node = $form['#node'];
    $node_parent = $form['#node_parent'];
    $account = $form['#account'];
    $type = $form['#objtype'];
    
    
    // Because we're adding comments, we need to write the title link differently.
    // We're also going to be adding the comment body into the message.
    if ($type == 'comment' && $op == 'report') {
      $realnode = node_load($node->nid);
      $comment_body = field_view_field('comment',$node,'comment_body');
      $node_link = 'node/' . $realnode->nid;
      
      // We also need the community context. I could have changed the menu callback.
      // But since the node is basically the node parent -- I thought it made more sense to leave as is.
      // Also, we need to build the body different based on the context of the comment.
      if ($realnode->type == 'community') {   
        $parent_title = $realnode->title;
        $body = t('<p>!name has reported abuse on a comment in the !title @community.</p><p>The comment reads:</p>'.render($comment_body), array(
        '!name' => l($account->realname, 'user/' . $account->uid, array('absolute' => TRUE)),
        '!title' => l($realnode->title, $node_link, array(
          'absolute' => TRUE,
          'entity_type' => 'node',
          'entity' => $realnode,
          )),
        '@community' => t('Community'),
        ));
      }
      else {
        // At this point, I also need to grab the community title.
        // This also assumes that there's only one community reference per node.
        // That might need changing later, but for now I'm going to leave it.
        $comm = _totem_common_get_field_entityreference_values('node',$node_parent,'field_community');
        $community = node_load($comm[0]);
        $parent_title = $community->title;
        $body = t('<p>!name has reported abuse on a comment in !title in the @title_parent @community.</p><p>The comment reads:</p>'.render($comment_body), array(
        '!name' => l($account->realname, 'user/' . $account->uid, array('absolute' => TRUE)),
        '!title' => l($realnode->title, $node_link, array(
          'absolute' => TRUE,
          'entity_type' => 'node',
          'entity' => $realnode,
          )),
        '@title_parent' => $parent_title,
        '@community' => t('Community'),
        ));
        
        // At this point, we need to set the node parent to the community.
        // So the get managers function works.
        $node_parent = $community;
      }
    }
    else {
      $parent_title = $node_parent->title;
      $body = t('<p>!name has reported abuse on !title in the @title_parent @community.</p>', array(
      '!name' => l($account->realname, 'user/' . $account->uid, array('absolute' => TRUE)),
      '!title' => l($node->title, 'node/' . $node->nid, array(
        'absolute' => TRUE,
        'entity_type' => 'node',
        'entity' => $node,
        )),
      '@title_parent' => $parent_title,
      '@community' => t('Community'),
      ));
    }
    
    
    // Send to managers.
    $managers = _totem_user_get_managers($node_parent);
    $recipients = user_load_multiple($managers);
    $subject = t('"@title" - abuse report', array('@title' => $parent_title));
    
    $body .= t('<p>@name wrote:</p>', array('@name' => $account->realname));
    $body .= $form_state['values']['message'];

    $message = privatemsg_new_thread($recipients, $subject, $body);
    if ($message['message'] !== FALSE && $message['success'] == TRUE) {
      drupal_set_message(check_plain(t('Managers have been notified.')));
    }

    foreach ($message['messages'] as $type => $group) {
      foreach ($group as $m) {
        drupal_set_message(check_plain($m), $type);
      }
    }

  }
}

/**
 * Modal privatemsg forms.
 */
function totem_discuss_form_message($sender, $recipient, $subject) {

  $title = NULL;
  $form_id = NULL;
  $args = array(
    $recipient,
    $subject
  );

  $title = t('Write new message');
  $form_id = 'privatemsg_new';

  $form_state = array(
    'title' => $title,
    'ajax' => TRUE,
    'build_info' => array(
      'args' => $args,
    ),
  );


  module_load_include('inc', 'privatemsg', 'privatemsg.pages');
  ctools_include('modal');
  ctools_include('ajax');

  $output = ctools_modal_form_wrapper($form_id, $form_state);

  if (!empty($form_state['executed'])) {
    ctools_add_js('ajax-responder');

    // Overwrite the form output.
    $output = array();
    $output[] = ctools_ajax_command_reload();
  }

  print ajax_render($output);
  exit;
}
/**
 * Implements hook_form_FORM_ID_alter().
 */
function totem_discuss_form_privatemsg_new_alter(&$form, &$form_state, $form_id) {

  global $user;

  $url = 'messages';
  $title = t('Cancel');
  if (isset($_REQUEST['destination'])) {
    $url = $_REQUEST['destination'];
  }
  else {
    $url = $_GET['q'];

    if (stripos($form['#action'], 'messages/view')) {
      // Reply form; force Cancel button to return to user's main inbox.
      $url = "user/{$user->uid}/messages";
    }
  }

  $form['actions']['cancel']['#markup'] = l($title, $url, array(
    'attributes' => array(
      'id' => 'edit-cancel',
      'class' => array('cancel'),
    ),
  ));

}
/**
 * Implements hook_form_FORM_ID_alter().
 */
function totem_discuss_form_privatemsg_list_alter(&$form, &$form_state, $form_id) {

  // Prepend empty list with some helper text.
  $types = _totem_common_types_info();

  $form['updated']['list']['#empty'] = t('<p>Stay connected with members and @communities on @site_name. Here you can send and receive messages from other members of @site_name. You also receive notifications for activity on your @topic and @media here. Try it out by clicking "New message".</p>', array(
    '@communities' => t($types['community']->name_plural),
    '@site_name' => variable_get('site_name'),
    '@topic' => t($types['topic']->name_plural),
    '@media' => t($types['media']->name_plural),
  ));

}

// Commenting - menu callbacks for AJAX requests.
/**
 * Inline comment-reply form.
 *
 * Menu callback for comment/%comment/inline/reply/%node/%totem_common_context_community
 *
 * @return array
 * An render array of AJAX commands to insert the new comment and any messages.
 */
function totem_discuss_put_inline_comment_reply_form($node, $parent_comment) {
  $commands = array();

  // If another inline comment form is already open, trigger canceling it.
  $commands[] = ajax_command_invoke('a.comment-cancel-action', 'trigger', array('click'));

  $pid = $parent_comment->cid;

  // @see comment_reply()
  $form = drupal_get_form("comment_node_{$node->type}_form",
          (object) array('nid' => $node->nid, 'pid' => $pid));

  // Attach a wrapper to entire form.
  // Includes the hidden clientside validation div, simplifying for cancel.
  $form['#prefix'] = '<div id="inline-comment-action-form" class="indented">';
  $form['#suffix'] = '</div>';

  $commands[] = ajax_command_after('#comment-' . $pid, drupal_render($form));

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}
/**
 * Inline comment editing form.
 *
 * Menu callback for comment/%comment/inline/edit/%node/%totem_common_context_community
 *
 * @return array
 * An render array of AJAX commands to insert the new comment and any messages.
 */
function totem_discuss_put_inline_comment_edit_form($comment, $node) {
  $commands = array();

  // If another inline comment form is already open, trigger canceling it.
  $commands[] = ajax_command_invoke('a.comment-cancel-action', 'trigger', array('click'));

  // @see comment_edit_page()
  $form = drupal_get_form('comment_node_' . $node->type . '_form', $comment);

  // Attach a wrapper to entire form.
  // Includes the hidden clientside validation div, simplifying for cancel.
  $form['#prefix'] = '<div id="inline-comment-action-form">';
  $form['#suffix'] = '</div>';

  // Hide the existing comment so clicking Cancel can re-show it by jQuery.
  $commands[] = ajax_command_invoke('#comment-' . $comment->cid, 'hide');
  // Place the edit form right after it.
  $commands[] = ajax_command_after('#comment-' . $comment->cid, drupal_render($form));

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}
/**
 * Inline comment confirm-delete form.
 *
 * Menu callback for comment/%comment/inline/delete/%node/%totem_common_context_community
 *
 * @return array
 * An render array of AJAX commands to insert the new comment and any messages.
 *
 * @see comment_confirm_delete_page()
 */
function totem_discuss_put_inline_comment_delete_form($comment) {

  $commands = array();

  // If another inline comment form is already open, trigger canceling it.
  $commands[] = ajax_command_invoke('a.comment-cancel-action', 'trigger', array('click'));

  // This form function is defined in comment.admin.inc. Ideally we might prefer
  // to use form_load_include() here, but we don't have access to $form_state.
  // @see totem_media_form_comment_confirm_delete_alter()
  module_load_include('inc', 'comment', 'comment.admin');
  $form = drupal_get_form('comment_confirm_delete', $comment);

  // Attach a wrapper to entire form.
  // Includes the hidden clientside validation div, simplifying for cancel.
  $form['#prefix'] = '<div id="inline-comment-action-form">';
  $form['#suffix'] = '</div>';

  $commands[] = ajax_command_after('#comment-' . $comment->cid, drupal_render($form));
  $commands[] = ajax_command_after('#comment-' . $comment->cid, theme('status_messages'));

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}
