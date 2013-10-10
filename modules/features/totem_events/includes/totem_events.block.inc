<?php
/**
 * @file
 * totem_events.block.inc
 */

// Hook implementations.
/**
 * Implements hook_block_info().
 */
function totem_events_block_info() {

  $site_name = check_plain(variable_get('site_name'));
  $blocks = array();

  $blocks['event_filter_grid'] = array(
    'region' => 'sidebar_first',
    'status' => 1,
    'cache' => DRUPAL_NO_CACHE,
    'weight' => 15,
    'title' => '<none>',
    'visibility' => BLOCK_VISIBILITY_LISTED,
    'pages' => 'events',
    'info' => t('@site_name - Events filter grid', array('@site_name' => $site_name)),
  );

  $blocks['event_filter_form'] = array(
    'region' => 'sidebar_first',
    'status' => 1,
    'cache' => DRUPAL_NO_CACHE,
    'weight' => 15,
    'title' => '<none>',
    'visibility' => BLOCK_VISIBILITY_LISTED,
    'pages' => 'node/*/events
node/*/events/*',
    'info' => t('@site_name - Events filter form', array('@site_name' => $site_name)),
  );

  return $blocks;
}
/**
 * Implements hook_block_view().
 */
function totem_events_block_view($delta = '', $context = '') {

  // View blocks.
  $callback = '_totem_events_block_view_' . $delta;
  if (function_exists($callback)) {
    return $callback($delta, $context);
  }

}

// Block view callbacks.
/**
 * Callback for list block on Community Overview screen.
 */
function _totem_events_block_view_overview_community($delta, $context) {

  $block = module_invoke('totem_common', 'block_view', 'overview_community_stub', array(
    'totem_events',
    'event',
    array(),
  ));

  if (!empty($block)) {
    $types = _totem_common_node_types();

    $block['subject'] = l(t('Events'), $block['content']['#path']);
    $block['content'] = array_merge_recursive($block['content'], array(
      '#classes_array' => array('events'),
      '#header_block' => _totem_common_embed_block('totem_common', 'button_add_type', $types['event'], 'Add'),
      '#title_link' => $block['subject'],
      '#more_link' => '<div class="clearfix"></div>' . l(t('View All Events <span></span>'), $block['content']['#path'], array('html' => TRUE, 'attributes' => array('class' => array('btn', 'corners', 'view-all')))),
      '#show_pager' => FALSE,
    ));

    return $block;
  }
}
/**
 * Callback for list block on Member Overview screen.
 */
function _totem_events_block_view_overview_user($delta, $context) {

  $block = module_invoke('totem_common', 'block_view', 'overview_user_stub', array(
    'totem_events',
    'event',
    array(),
  ));

  if (!empty($block)) {
    $block['subject'] = l(t('Events'), $block['content']['#path']);
    $block['content'] = array_merge_recursive($block['content'], array(
      '#classes_array' => array('events'),
      '#header_block' => NULL,
      '#title_link' => $block['subject'],
      '#more_link' => '<div class="clearfix"></div>' . l(t('View All Events <span></span>'), $block['content']['#path'], array('html' => TRUE, 'attributes' => array('class' => array('btn', 'corners', 'view-all')))),
      '#show_pager' => FALSE,
    ));

    return $block;
  }
}
/**
 * Callback for Calendar block view.
 */
function _totem_events_block_view_event_filter_grid($delta, $context) {

  module_load_include('inc', 'totem_events', 'includes/totem_events.calbuilder');

  // Output 2 months worth of calendar.
  $block['content'] = '';
  for ($i = 0; $i < 2; $i++) {
    $block['content'] .= TotemEventsCalendar::buildCalendar(date('Y'), (date('n') + $i));
  }

  return $block;
}
/**
 * Callback for Filter Form block view.
 */
function _totem_events_block_view_event_filter_form($delta, $context) {

  $node = _totem_common_get_community_context_node();
  if (!empty($node)) {
    $block = array(
      'content' => drupal_get_form('totem_events_filter_form', array(
        'node_community' => $node,
      )),
    );

    return $block;
  }
}
