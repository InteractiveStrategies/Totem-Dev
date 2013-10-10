<?php
/**
 * @file
 * totem_events.features.inc
 */

/**
 * Implements hook_node_info().
 */
function totem_events_node_info() {
  $items = array(
    'event' => array(
      'name' => t('Event'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  return $items;
}
