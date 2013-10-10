<?php

/**
 * @file
 * totem_media.block.inc
 */

// Hook implementations.
/**
 * Implements hook_block_info_alter().
 */
function totem_media_block_info_alter(&$blocks, $theme, $code_blocks) {

  // Force button_add_type and list_type block instances
  // for these types to use /media paths exclusively.
  // Also use this ordering to set weight for list_type blocks (see below).
  $types = array('media_collection', 'media');

  $page_path_delimiter = "\r\n";

  foreach ($blocks['totem_common'] as $delta => &$block) {
    if (!empty($code_blocks['totem_common'][$delta]['properties'][2])) {

      $block_node_type = $code_blocks['totem_common'][$delta]['properties'][2]->type;

      // Check if we should disable all media_collection type blocks.
      if (variable_get('totem_media_hide_collections') && $block_node_type == 'media_collection') {
        $block['status'] = 0;
      }

      if ($code_blocks['totem_common'][$delta]['properties'][1] == 'button_add_type') {
        if (in_array($block_node_type, $types)) {
          $pages = array('node/*/media', 'node/*/media/*');

          $block['weight'] = 14;
          $block['pages'] = implode($page_path_delimiter, $pages);
        }
      }
      elseif ($code_blocks['totem_common'][$delta]['properties'][1] == 'list_type') {
        $type_index = array_search($block_node_type, $types);
        if ($type_index !== FALSE) {
          $pages = array('node/*/media');
          if (variable_get('totem_media_gallery_mode') == TRUE) {
            // Include path that auto-opens a media node in modal.
            $pages[] = 'node/*/media/view/*';
          }
          else {
            $pages[] = 'node/*/media/*';
          }

          $block['title'] = '';
          $block['weight'] = 15 + $type_index;
          $block['pages'] = implode($page_path_delimiter, $pages);
        }
      }

    }
  }

}
/**
 * Implements hook_block_view().
 */
function totem_media_block_view($delta = '', $context = '') {

  $callback = '_totem_media_block_view_' . $delta;
  if (function_exists($callback)) {
    return $callback($delta, $context);
  }

}
/**
 * Implements hook_block_view_MODULE_DELTA_alter().
 */
function totem_media_block_view_totem_common_button_add_type_alter(&$data, $block) {

  if ($data['content']['#entity_type'] == 'media_collection') {

    $content = drupal_get_form('totem_media_form_add_to_collection');
    if (empty($content)) {
      // If nothing returned, it's probably because no media is available for
      // current user to add to a collection. Make sure block doesn't show.
      $data['content'] = NULL;
    }
    else {
      $data['content'] = array_merge($data['content'], $content);
    }

  }
  elseif ($data['content']['#entity_type'] == 'media') {

    // Preload plupload stuff so totem_media.js
    // doesn't choke during event binding.
    drupal_add_library('plupload', 'plupload');
    drupal_add_css(drupal_get_path('module', 'plupload') . '/plupload.css');
    drupal_add_js(drupal_get_path('module', 'plupload') . '/plupload.js', array(
      'scope' => 'header',
      'group' => JS_LIBRARY,
      'weight' => -5000,
    ));
    
    $text = t('@verb @label', array('@verb' => t('Add'), '@label' => t('Media')));
    // Note: menu link is only defined with the nid as 4th arg now.
    $path = 'community/add/files';
    if ($node = _totem_common_get_community_context_node()) {
      $path .= '/' . $node->nid;
    }

    $link = _totem_common_modal_link(array(
      'text' => $text,
      'path' => $path,
      'class' => array('btn', 'corners', 'add-media'),
    ));

    $data['content']['#markup'] = $link;
  }

}
/**
 * Implements hook_block_view_MODULE_DELTA_alter().
 */
function totem_media_block_view_totem_common_list_type_alter(&$data, $block) {

  // If list_type block has no #theme defined, assume no results and output
  // no resultset message.
  // @see _totem_common_block_view_list_type()
  if ($data['content']['#entity_type'] == 'media') {
    if (variable_get('totem_media_gallery_mode')) {
      if (empty($data['content']['#theme'])) {
        $types = _totem_common_types_info();
        $filters = _totem_common_session_get('filter');

        $data['content']['#markup'] = $types[$data['content']['#entity_type']]->no_results_text_community;

        // Alter message if "Private" filter is active.
        if (!empty($filters['node']['status'])) {
          $data['content']['#markup'] = $types[$data['content']['#entity_type']]->no_results_text_community_private_filter_active;
        }
      }
    }
  }

}

// Block view callbacks.
/**
 * Callback for list block on Community Overview screen.
 */
function _totem_media_block_view_overview_community($delta, $context) {

  $block = module_invoke('totem_common', 'block_view', 'overview_community_stub', array(
    'totem_media',
    'media',
    // Force skipping NOT_IN_COLLECTION query alter.
    // @see _totem_common_block_view_overview_community_stub()
    // @see totem_media_node_community_media()
    array(
      'tags' => array(),
    ),
  ));

  if (!empty($block)) {
    $types = _totem_common_node_types();

    $block['subject'] = l(t('Media'), $block['content']['#path']);
    $block['content'] = array_merge_recursive($block['content'], array(
      '#classes_array' => array('media'),
      '#header_block' => _totem_common_embed_block('totem_common', 'button_add_type', $types['media'], 'Add'),
      '#title_link' => $block['subject'],
      '#more_link' => '<div class="clearfix"></div>' . l(t('View All Media') . '<span></span>', $block['content']['#path'], array('html' => TRUE, 'attributes' => array('class' => array('btn', 'corners', 'view-all')))),
      '#show_pager' => FALSE,
    ));

    return $block;
  }
}
/**
 * Callback for list block on Member Overview screen.
 */
function _totem_media_block_view_overview_user($delta, $context) {

  $block = module_invoke('totem_common', 'block_view', 'overview_user_stub', array(
    'totem_media',
    'media',
    array(),
  ));

  if (!empty($block)) {
    $block['subject'] = l(t('Media'), $block['content']['#path']);
    $block['content'] = array_merge_recursive($block['content'], array(
      '#classes_array' => array('media'),
      '#header_block' => NULL,
      '#title_link' => $block['subject'],
      '#more_link' => '<div class="clearfix"></div>' . l(t('View All Media') . '<span></span>', $block['content']['#path'], array('html' => TRUE, 'attributes' => array('class' => array('btn', 'corners', 'view-all')))),
      '#show_pager' => FALSE,
    ));

    return $block;
  }
}
