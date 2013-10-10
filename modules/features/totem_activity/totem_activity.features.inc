<?php
/**
 * @file
 * totem_activity.features.inc
 */

/**
 * Implements hook_flag_default_flags().
 */
function totem_activity_flag_default_flags() {
  $flags = array();
  // Exported flag: "Subscribe to node".
  $flags['subscribe_node'] = array(
    'content_type' => 'node',
    'title' => 'Subscribe to node',
    'global' => '0',
    'types' => array(
      0 => 'topic',
      1 => 'event',
      2 => 'media',
      3 => 'media_collection',
    ),
    'flag_short' => 'Subscribe',
    'flag_long' => 'Subscribe to this content.',
    'flag_message' => 'You have successfully subscribed to "[node:title]".',
    'unflag_short' => 'Unsubscribe',
    'unflag_long' => 'Unsubscribe from this content.',
    'unflag_message' => 'You have successfully unsubscribed from "[node:title]".',
    'unflag_denied_text' => '',
    'link_type' => 'normal',
    'roles' => array(
      'flag' => array(
        0 => '2',
        1 => '3',
      ),
      'unflag' => array(
        0 => '2',
        1 => '3',
      ),
    ),
    'weight' => 0,
    'show_on_form' => 0,
    'access_author' => '',
    'show_on_page' => 1,
    'show_on_teaser' => 0,
    'show_contextual_link' => 1,
    'i18n' => 0,
    'api_version' => 2,
    'module' => 'totem_activity',
    'locked' => array(
      0 => 'name',
    ),
  );
  return $flags;

}
