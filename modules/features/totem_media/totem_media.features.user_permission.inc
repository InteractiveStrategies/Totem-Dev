<?php
/**
 * @file
 * totem_media.features.user_permission.inc
 */

/**
 * Implements hook_user_default_permissions().
 */
function totem_media_user_default_permissions() {
  $permissions = array();

  // Exported permission: access media node view.
  $permissions['access media node view'] = array(
    'name' => 'access media node view',
    'roles' => array(
      0 => 'administrator',
    ),
    'module' => 'internal_nodes',
  );

  // Exported permission: add media from remote sources.
  $permissions['add media from remote sources'] = array(
    'name' => 'add media from remote sources',
    'roles' => array(
      0 => 'administrator',
      1 => 'authenticated user',
    ),
    'module' => 'media_internet',
  );

  // Exported permission: create media content.
  $permissions['create media content'] = array(
    'name' => 'create media content',
    'roles' => array(
      0 => 'administrator',
    ),
    'module' => 'node',
  );

  // Exported permission: create media_collection content.
  $permissions['create media_collection content'] = array(
    'name' => 'create media_collection content',
    'roles' => array(
      0 => 'administrator',
    ),
    'module' => 'node',
  );

  // Exported permission: delete own media content.
  $permissions['delete own media content'] = array(
    'name' => 'delete own media content',
    'roles' => array(
      0 => 'administrator',
      1 => 'authenticated user',
    ),
    'module' => 'node',
  );

  // Exported permission: delete own media_collection content.
  $permissions['delete own media_collection content'] = array(
    'name' => 'delete own media_collection content',
    'roles' => array(
      0 => 'administrator',
      1 => 'authenticated user',
    ),
    'module' => 'node',
  );

  // Exported permission: edit any media content.
  $permissions['edit any media content'] = array(
    'name' => 'edit any media content',
    'roles' => array(
      0 => 'administrator',
      1 => 'moderator',
    ),
    'module' => 'node',
  );

  // Exported permission: edit any media_collection content.
  $permissions['edit any media_collection content'] = array(
    'name' => 'edit any media_collection content',
    'roles' => array(
      0 => 'administrator',
      1 => 'moderator',
    ),
    'module' => 'node',
  );

  // Exported permission: edit own media content.
  $permissions['edit own media content'] = array(
    'name' => 'edit own media content',
    'roles' => array(
      0 => 'administrator',
      1 => 'authenticated user',
    ),
    'module' => 'node',
  );

  // Exported permission: edit own media_collection content.
  $permissions['edit own media_collection content'] = array(
    'name' => 'edit own media_collection content',
    'roles' => array(
      0 => 'administrator',
      1 => 'authenticated user',
    ),
    'module' => 'node',
  );

  // Exported permission: view media.
  $permissions['view media'] = array(
    'name' => 'view media',
    'roles' => array(
      0 => 'administrator',
      1 => 'authenticated user',
    ),
    'module' => 'media',
  );

  return $permissions;
}
