<?php
/**
 * @file
 * totem_user.block.inc
 */

// Hook implementations.
/**
 * Implements hook_block_info().
 */
function totem_user_block_info() {

  $site_name = check_plain(variable_get('site_name'));

  $pages_user_header = implode("\r\n", array(
    'members/*',
    'user/*',
    'messages/*',
    'node/add/*',
    'search/*',
  ));

  $blocks = array();

  $blocks['user_profile'] = array(
    'region' => 'content_top',
    'status' => 1,
    'cache' => DRUPAL_NO_CACHE,
    'weight' => 0,
    'title' => '<none>',
    'visibility' => BLOCK_VISIBILITY_LISTED,
    'pages' => $pages_user_header,
    'info' => t('@site_name - Member profile', array('@site_name' => $site_name)),
  );

  // Note block_custom INSERT in _totem_user_install_blocks as well.
  // Need numeric delta to support block_custom record.
  $blocks['2'] = array(
    'region' => 'content_top',
    'status' => 1,
    'cache' => DRUPAL_CACHE_GLOBAL,
    'weight' => 1,
    'title' => '<none>',
    'visibility' => BLOCK_VISIBILITY_LISTED,
    'pages' => 'user/*',
    'info' => t('@site_name - Member notification', array('@site_name' => $site_name)),
  );

  $blocks['user_filter_form'] = array(
    'region' => 'sidebar_first',
    'status' => 1,
    'cache' => DRUPAL_NO_CACHE,
    'weight' => 10,
    'title' => '<none>',
    'visibility' => BLOCK_VISIBILITY_LISTED,
    'pages' => 'node/*/members',
    'info' => t('@site_name - Members filter form', array('@site_name' => $site_name)),
  );

  return $blocks;
}
/**
 * Implements hook_block_view().
 */
function totem_user_block_view($delta = '', $context = '') {

  global $user;

  // View blocks.
  $callback = '_totem_user_block_view_' . $delta;
  if (function_exists($callback)) {
    return $callback($delta, $context);
  }
  else {

    switch ($delta) {
      case '2':
        // Only show delta:2 system notification message on user's own profile.
        $account = _totem_user_get_current_profile();
        if ($account->uid == $user->uid && drupal_get_path_alias() == "members/{$account->uid}") {
          $block = block_block_view($delta);
          $block['content'] = array(
            '#type' => 'markup',
            '#markup' => $block['content'],
            '#classes_array' => array('my-special-class'),
          );

          return $block;
        }
        break;
    }

  }
}
/**
 * Implements hook_block_configure().
 */
function totem_user_block_configure($delta = 0) {

  // Only present configure form for block_custom blocks.
  if (!in_array($delta, array('2'))) {
    return;
  }

  if ($delta) {
    $custom_block = block_custom_block_get($delta);
  }
  else {
    $custom_block = array();
  }

  $form = block_custom_block_form($custom_block);
  $form['info']['#type'] = 'hidden';
  $form['body_field']['body']['#required'] = FALSE;
  $form['#prefix'] = '<p>This block allows you to define a custom message that will be presented to authenticated users on their main profile page.<br />(Example messages might include notice of scheduled system maintenance, or announcement of a new feature.)</p>';

  return $form;
}
/**
 * Implements hook_form_FORM_ID_alter().
 */
function totem_user_form_block_admin_configure_alter(&$form, &$form_state, $form_id) {

  $module = $form_state['build_info']['args'][0];
  $delta = $form_state['build_info']['args'][1];

  if ($module !== 'totem_user') {
    return;
  }

  switch ($delta) {
    case '2':
      $form['visibility_title']['#title_display'] = 'invisible';
      $form['visibility']['path']['#type']
          = $form['visibility']['node_type']['#type']
          = $form['visibility']['role']['#type']
          = $form['visibility']['user']['#type']
          = 'hidden';

      // Auto-append current timestamp class to css_class values. Can be used
      // to create/expire cookies in the theme layer if needed.
      $css_class = $form['settings']['css_class']['#default_value'];
      $css_class = explode(' ', $css_class);
      $css_class = array_combine($css_class, $css_class);
      foreach ($css_class as $key) {
        if (drupal_substr($key, 0, drupal_strlen('profile-sys-notice-updated-')) == 'profile-sys-notice-updated-') {
          unset($css_class[$key]);
        }
      }
      $css_class = array_values($css_class);

      $form['settings']['css_class']['#default_value'] = implode(' ', $css_class) . ' profile-sys-notice-updated-' . time();
      $form['settings']['css_class']['#description'] .= ' ' . t('Note that the "profile-sys-notice-updated-TIMESTAMP" is automatically applied to the "Member notification" block and can be used by themers to control cookie expiration if needed.');
      break;
  }
}
/**
 * Implements hook_block_save().
 */
function totem_user_block_save($delta = 0, $edit = array()) {

  // Only present configure form for block_custom blocks.
  if (!in_array($delta, array('2'))) {
    return;
  }

  $edit['css_class'] .= ' test';

  block_custom_block_save($edit, $delta);
}

// Block view callbacks.
/**
 * Callback for "Add type" block view.
 */
function _totem_user_block_view_button_add_user($delta, $context) {

  global $user;

  if (!empty($context)) {
    $node_community_context = $context[0];
  }
  else {
    $node_community_context = _totem_common_get_community_context_node();
  }
  $text = NULL;
  $content = NULL;
  $is_modal = TRUE;

  // 2013-06-05, natemow. Per new use of "access community node view" perm,
  // user may or may not be anonymous.
  if (user_is_logged_in()) {

    // Add "Join community" link.
    $path = "user/{$user->uid}/modal/join/{$node_community_context->nid}";
    $item = menu_get_item($path);

    if ($item['access']) {
      $text = t(str_ireplace('community', '@community', $item['title']), array('@community' => t('Community')));
      $content = _totem_common_modal_link(array(
        'text' => $text,
        'path' => $path,
        'class' => array('btn', 'corners', 'join-invite'),
      ));

      $node_entity = entity_metadata_wrapper('node', $node_community_context);
      if ($node_entity->field_community_status->value() == TOTEM_COMMUNITY_STATUS_OPEN) {
        /*
         * "Open" community; don't return modal link
         * @see totem_user_form_node()
         * $op=add for link action as well.
         */
        $is_modal = FALSE;
        $path = str_ireplace('modal/', 'open/', $path);
        $content = l($text, $path, array('attributes' => array('class' => array('btn', 'corners', 'join-invite'))));
      }

    }
    else {
      // Add "Invite friends" link
      $path = "user/{$user->uid}/modal/invite/{$node_community_context->nid}";
      $item = menu_get_item($path);

      if ($item['access']) {
        $text = $item['title'];
        $content = _totem_common_modal_link(array(
          'text' => $text,
          'path' => $path,
          'class' => array('btn', 'corners', 'join-invite'),
        ));
      }
    }
  }

  if (!empty($content)) {
    $block = array(
      'subject' => NULL,
      'content' => array(
        '#type' => 'markup',
        '#text' => $text,
        '#path' => $path,
        '#is_modal' => $is_modal,
        '#markup' => $content,
      ),
    );

    return $block;
  }

}
/**
 * Callback for list block on Community Overview screen.
 */
function _totem_user_block_view_overview_community($delta, $context) {

  $block = module_invoke('totem_common', 'block_view', 'overview_community_stub', array(
    'totem_user',
    'user',
    array(),
  ));

  if (!empty($block)) {
    $block['subject'] = l(t('Recent Members'), $block['content']['#path']);
    $block['content'] = array_merge_recursive($block['content'], array(
      '#classes_array' => array('members'),
      '#header_block' => _totem_common_embed_block('totem_user', 'button_add_user'),
      '#title_link' => $block['subject'],
      '#more_link' => '<div class="clearfix"></div>' . l(t('View All Members <span></span>'), $block['content']['#path'], array('html' => TRUE, 'attributes' => array('class' => array('btn', 'corners', 'view-all')))),
      '#show_pager' => FALSE,
    ));

    return $block;
  }
}
/**
 * Callback for Profile block view.
 */
function _totem_user_block_view_user_profile($delta, $context) {

  global $user;
  $account = _totem_user_get_current_profile();

  if (!empty($account)) {
    $user_profile = user_view($account, 'full');

    if (!empty($user_profile['#account']->content)) {

      $nid_community = _totem_common_get_field_entityreference_values('user', $account, 'field_community');

      $block = array(
        'subject' => t(''),
        'content' => array(
          '#theme' => 'totem_user_block_user_profile',
          '#user_profile' => $user_profile,
          '#is_own_profile' => ($account->uid == $user->uid),
          '#user_has_communities' => !empty($nid_community),
        ),
      );

      return $block;
    }
  }
}
/**
 * Callback for Filter Form block view.
 */
function _totem_user_block_view_user_filter_form($delta, $context) {

  $node = _totem_common_get_community_context_node();
  if (!empty($node)) {
    $content = drupal_get_form('totem_user_form_user_filter', $node);
    $content['#classes_array'] = array('action', 'member-filter');

    $block = array(
      'content' => $content,
    );

    return $block;
  }
}
