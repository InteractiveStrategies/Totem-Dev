<?php
/**
 * @file
 * totem_discuss.features.inc
 */

/**
 * Implements hook_node_info().
 */
function totem_discuss_node_info() {
  $items = array(
    'topic' => array(
      'name' => t('Discussion'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Subject'),
      'help' => '',
    ),
  );
  return $items;
}
