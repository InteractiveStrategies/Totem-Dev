<?php
/**
 * @file
 * totem_rating.features.user_permission.inc
 */

/**
 * Implements hook_user_default_permissions().
 */
function totem_rating_user_default_permissions() {
  $permissions = array();

  // Exported permission: rate content
  $permissions['rate content'] = array(
    'name' => 'rate content',
    'roles' => array(
      0 => 'administrator',
      1 => 'authenticated user',
    ),
    'module' => 'fivestar',
  );

  return $permissions;
}
