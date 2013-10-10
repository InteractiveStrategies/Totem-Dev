<?php
/**
 * @file
 * totem_rating.block.inc
 */

// Hook implementations.
/**
 * Implements hook_block_info().
 */
function totem_rating_block_info() {

  $site_name = check_plain(variable_get('site_name'));
  $blocks = array();

  $blocks['user_score'] = array(
    'region' => 'content_top',
    'cache' => DRUPAL_NO_CACHE,
    'weight' => 15,
    'title' => '<none>',
    'info' => t('@site_name - User Score', array('@site_name' => $site_name)),
    'pages' => "user\r\nuser/*",
    'visibility' => BLOCK_VISIBILITY_LISTED,
    'status' => 1,
  );

  return $blocks;
}
/**
 * Implements hook_block_view().
 */
function totem_rating_block_view($delta = '', $context = '') {

  // View  blocks.
  $callback = '_totem_rating_block_view_' . $delta;
  if (function_exists($callback)) {
    return $callback($delta, $context);
  }

}

// Block view callbacks.
/**
 * Callback for user score block.
 */
function _totem_rating_block_view_user_score($delta, $context) {
  // Find the current user page's user info.
  $account = _totem_user_get_current_profile();
  if (empty($account)) {
    global $user;
    $account = $user;    
  }
  $settings = array(
    'rating_info' => array(),
    'type_info' => array(),
    'return_info' => array(
      'type' => 'markup',
    ),
  );
  $markup = _totem_rating_user_score($account, $settings);
  
  $block = array(
    'subject' => '<none>',
    'content' => array(
      '#type' => 'markup',
      '#markup' => $markup,
    ),
  );
  
  return $block;
}
