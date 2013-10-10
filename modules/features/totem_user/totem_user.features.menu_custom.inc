<?php
/**
 * @file
 * totem_user.features.menu_custom.inc
 */

/**
 * Implements hook_menu_default_menu_custom().
 */
function totem_user_menu_default_menu_custom() {
  $menus = array();

  // Exported menu: user-menu.
  $menus['user-menu'] = array(
    'menu_name' => 'user-menu',
    'title' => 'User menu',
    'description' => 'The <em>User</em> menu contains links related to the user\'s account.',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('The <em>User</em> menu contains links related to the user\'s account.');
  t('User menu');


  return $menus;
}
