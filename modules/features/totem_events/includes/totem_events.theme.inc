<?php
/**
 * @file
 * totem_discuss.theme.inc
 */

/**
 * Implements template_preprocess_page().
 */
function totem_events_preprocess_page(&$vars) {

  // Add module UI assets.
  if (!$vars['is_admin']) {
    drupal_add_css(drupal_get_path('module', 'totem_events') . '/theme/totem_events.css');
  }

}
