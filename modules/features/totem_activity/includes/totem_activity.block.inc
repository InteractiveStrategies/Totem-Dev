<?php
/**
 * @file
 * totem_activity.block.inc
 */

// Hook implementations.
/**
 * Implements hook_block_view().
 */
function totem_activity_block_view($delta = '', $context = '') {

  // View  blocks.
  $callback = '_totem_activity_block_view_' . $delta;
  if (function_exists($callback)) {
    return $callback($delta, $context);
  }

}

// Block view callbacks.
/**
 * Callback for list block on Community Overview screen.
 */
function _totem_activity_block_view_overview_community($delta, $context) {

  $block = module_invoke('totem_common', 'block_view', 'overview_community_stub', array(
    'totem_activity',
    'recent',
    array(
      'page_limit' => PAGE_SIZE_LISTS_PAGE,
    ),
  ));

  if (!empty($block)) {
    $block['subject'] = t('Recent Activity');
    $block['content'] = array_merge_recursive($block['content'], array(
      '#classes_array' => array('activity'),
      '#header_block' => NULL,
      '#title_link' => $block['subject'],
      '#more_link' => '<div class="clearfix"></div>' . l(t('View All Activity <span></span>'), $block['content']['#path'], array(
        'html' => TRUE,
        'attributes' => array(
          'class' => array('btn', 'corners', 'view-all'),
        ),
      )),
      '#show_pager' => TRUE,
    ));

    return $block;
  }
}
/**
 * Callback for list block on Member Overview screen.
 */
function _totem_activity_block_view_overview_user($delta, $context) {

  $block = module_invoke('totem_common', 'block_view', 'overview_user_stub', array(
    'totem_activity',
    'recent',
    array(
      'page_limit' => PAGE_SIZE_LISTS_PAGE,
    ),
  ));

  if (!empty($block)) {
    // Force this block to always show.
    unset($block['content']['#classes_array']);

    $block['subject'] = t('Recent Activity');
    $block['content'] = array_merge_recursive($block['content'], array(
      '#classes_array' => array('box', 'box-recent', 'clearfix', 'activity'),
      '#header_block' => _totem_common_embed_block('totem_activity', 'user_recent_activity_filter', $block['content']['#account']),
      '#title_link' => $block['subject'],
      '#more_link' => '<div class="clearfix"></div>' . l(t('View All Activity <span></span>'), $block['content']['#path'], array(
        'html' => TRUE,
        'attributes' => array(
          'class' => array('btn', 'corners', 'view-all'),
        ),
      )),
      '#show_pager' => TRUE,
    ));

    return $block;
  }
}
/**
 * Callback for list subscription block on Member tab screens.
 */
function _totem_activity_block_view_subscribed_type($delta, $context) {

  global $user;

  // Only return block if user is viewing own profile.
  if (user_is_logged_in() && arg(1) == $user->uid) {
    $type = $context[0];
    $query = NULL;
    $hook = "user_subscribed_node";
    $items = module_invoke_all($hook, array(
      'account' => $user,
      'bundle' => $type->type,
      'page_limit' => NULL,
    ));

    $query = _totem_common_efq_extract_query($items);

    if (!empty($query) && !empty($query->results)) {
      $block = array(
        'subject' => t('My @label Subscriptions', array('@label' => $type->name)),
        'content' => array(
          '#theme' => 'totem_common_paged_entities',
          '#query' => $query,
          '#classes_array' => array(
            'subscribed-type',
            'subscribed-' . drupal_html_class($type->type),
          ),
        ),
      );

      return $block;
    }
  }
}
/**
 * Callback for user profile Recent Activity filter form block view.
 */
function _totem_activity_block_view_user_recent_activity_filter($delta, $context) {

  $block = array(
    'content' => drupal_get_form('totem_activity_user_recent_activity_filter_form', array(
      'account' => $context[0],
    )),
  );

  return $block;
}
