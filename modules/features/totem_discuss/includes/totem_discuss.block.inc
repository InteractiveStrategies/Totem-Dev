<?php
/**
 * @file
 * totem_discuss.block.inc
 */

// Hook implementations.
/**
 * Implements hook_block_view().
 */
function totem_discuss_block_view($delta = '', $context = '') {

  // View  blocks.
  $callback = '_totem_discuss_block_view_' . $delta;
  if (function_exists($callback)) {
    return $callback($delta, $context);
  }

}

// Block view callbacks.
/**
 * Callback for list block on Community Overview screen.
 */
function _totem_discuss_block_view_overview_community($delta, $context) {

  $block = module_invoke('totem_common', 'block_view', 'overview_community_stub', array(
    'totem_discuss',
    'topic',
    array(),
  ));

  if (!empty($block)) {
    $types = _totem_common_node_types();

    $block['subject'] = l(t('Latest @topic', array('@topic' => t('Discussions'))), $block['content']['#path']);
    $block['content'] = array_merge_recursive($block['content'], array(
      '#classes_array' => array('topics'),
      '#header_block' => _totem_common_embed_block('totem_common', 'button_add_type', $types['topic'], 'Add'),
      '#title_link' => $block['subject'],
      '#more_link' => '<div class="clearfix"></div>' . l(t('View All @topic <span></span>', array('@topic' => t('Discussions'))), $block['content']['#path'], array(
        'html' => TRUE,
        'attributes' => array(
          'class' => array('btn', 'corners', 'view-all'),
        ),
      )),
      '#show_pager' => FALSE,
    ));

    return $block;
  }
}
/**
 * Callback for list block on Member Overview screen.
 */
function _totem_discuss_block_view_overview_user($delta, $context) {

  $block = module_invoke('totem_common', 'block_view', 'overview_user_stub', array(
    'totem_discuss',
    'topic',
    array(),
  ));

  if (!empty($block)) {
    $block['subject'] = l(t('Discussions'), $block['content']['#path']);
    $block['content'] = array_merge_recursive($block['content'], array(
      '#classes_array' => array('topics'),
      '#header_block' => NULL,
      '#title_link' => $block['subject'],
      '#more_link' => '<div class="clearfix"></div>' . l(t('View All @topic <span></span>', array('@topic' => t('Discussions'))), $block['content']['#path'], array(
        'html' => TRUE,
        'attributes' => array(
          'class' => array('btn', 'corners', 'view-all'),
        ),
      )),
      '#show_pager' => FALSE,
    ));

    return $block;
  }
}
