<?php
/**
 * @file
 * totem.profile
 */

// Hook implementations.
/**
 * Implements hook_form_FORM_ID_alter().
 */
function totem_form_install_configure_form_alter(&$form, &$form_state, $form_id) {

  $mail = variable_get('site_mail', 'admin@' . $_SERVER['HTTP_HOST']);

  $form['site_information']['#collapsible']
      = $form['admin_account']['#collapsible']
      = $form['server_settings']['#collapsible']
      = $form['update_notifications']['#collapsible']
      = TRUE;

  $form['site_information']['#title'] = st('Community information');

  $form['site_information']['site_name']['#title'] = st('Name');
  $form['site_information']['site_name']['#weight'] = 10;
  $form['site_information']['site_name']['#default_value'] = variable_get('site_name');

  $form['site_information']['site_slogan'] = array(
    '#type' => 'textfield',
    '#title' => st('Tagline'),
    '#description' => st(''),
    '#default_value' => variable_get('site_slogan'),
    '#required' => FALSE,
    '#weight' => 30,
  );

  $form['site_information']['access_community_node_view'] = array(
    '#type' => 'checkbox',
    '#title' => st('Make !open and !restricted communities visible to anonymous users?', array(
      '!open' => '<abbr title="Authenticated users can view and join Open communities.">Open</abbr>',
      '!restricted' => '<abbr title="Authenticated users can view and request to join Restricted communities; joining requires manager approval.">Restricted</abbr>',
    )),
    '#description' => st('This permission can be changed later if needed. !closed communities will always require manager approval for access.', array(
      '!closed' => '<abbr title="Authenticated users can not view Closed communities, but can request to join and gain access; joining requires manager approval.">Closed</abbr>',
    )),
    '#required' => FALSE,
    '#weight' => 40,
  );

  $form['site_information']['site_mail']['#title'] = st('Email address');
  $form['site_information']['site_mail']['#weight'] = 30;
  $form['site_information']['site_mail']['#default_value'] = $mail;

  $form['admin_account']['#title'] = st('Community administrator');
  $form['admin_account']['account']['name']['#default_value'] = $mail;
  $form['admin_account']['account']['mail']['#title'] = st('Email address');
  $form['admin_account']['account']['mail']['#default_value'] = $mail;
  // Force standard password field (vs. pass w/confirm and strength).
  $form['admin_account']['account']['pass']['#type'] = 'password';
  $form['admin_account']['account']['pass']['#title'] = st('Password');

  // Force username to hidden unique id mail will be copied to name field
  // in hook_user_presave.
  $form['admin_account']['account']['name'] = array(
    '#type' => 'hidden',
    '#value' => $form['admin_account']['account']['mail']['#default_value'],
  );

  $form['admin_account']['update_status_module'] = $form['update_notifications']['update_status_module'];

  unset($form['update_notifications']);

  // Add extra submit handlers.
  $form['#submit'][] = 'totem_form_install_configure_form_submit';
}
/**
 * Custom submit handler for settings form.
 */
function totem_form_install_configure_form_submit($form, &$form_state) {

  global $user;

  // Save new account info.
  $userinfo = array(
    'field_name_first' => array(LANGUAGE_NONE => array(array('value' => 'Administrator'))),
    'field_name_last' => array(LANGUAGE_NONE => array(array('value' => 'Administrator'))),
  );

  $user = user_save($user, $userinfo);

  // Update system variables.
  variable_set('site_slogan', filter_xss_admin($form_state['values']['site_slogan']));

  // Set default access to community content. Site builders can now toggle
  // anon access to their site's community nodes via the
  // "access community node view" perm.
  user_role_grant_permissions(2, array('access community node view'));
  user_role_grant_permissions(3, array('access community node view'));
  user_role_grant_permissions(4, array('access community node view'));

  // Enable/disable anonymous access to community nodes.
  if (!empty($form_state['values']['access_community_node_view'])) {
    user_role_grant_permissions(1, array('access community node view'));
  }
  else {
    user_role_revoke_permissions(1, array('access community node view'));
  }

}
/**
 * Implements hook_install_tasks().
 */
function totem_install_tasks($install_state) {

  $tasks = array(
    '_totem_install_start' => array(),
    '_totem_install_menus' => array(),
    '_totem_install_blocks' => array(),
    '_totem_install_rebuilds' => array(
      'display_name' => st('Build permissions'),
    ),
    '_totem_install_finish' => array(),
  );

  return $tasks;
}

// Installation tasks.
/**
 * TODO.
 */
function _totem_install_start() {

  variable_set('error_level', 0);

}
/**
 * TODO.
 */
function _totem_install_finish() {

  variable_set('error_level', 1);

  drupal_set_message(t('@site_name installed successfully!', array(
    '@site_name' => variable_get('site_name'),
  )));

  drupal_set_message(t('To integrate @site_name with Facebook you will need to !link_fboauth, then set the App ID and App Secret created for your domain. See the !link_readme for further details.', array(
    '@site_name' => variable_get('site_name'),
    '!link_fboauth' => l(t('enable the Facebook OAuth module'), 'admin/modules', array('fragment' => 'edit-modules-other-entity-token-enable',)),
    '!link_readme' => l(t('Totem User README'), 'profiles/totem/modules/features/totem_user/README.txt', array('attributes' => array('target' => '_blank',))),
  )));

}
/**
 * TODO.
 */
function _totem_install_menus() {

  // Only install menus if no custom client-centric mod is enabled.
  $profile_custom_mod = variable_get('totem_custom');
  if (!empty($profile_custom_mod)) {
    return;
  }

  // Menus are added in totem.profile, after all Features mods have
  // been installed.
  $profile_intall_vars = variable_get('totem_install_temp_vars');

  $items_footer = array(
    array(
      'link_title' => st('Featured Communities'),
      'link_path' => 'communities/featured',
      'menu_name' => 'footer-menu',
      'weight' => 4,
    ),
    array(
      'link_title' => st('About'),
      'link_path' => 'node/' . $profile_intall_vars['nid_about'],
      'menu_name' => 'footer-menu',
      'weight' => 5,
    ),
    array(
      'link_title' => st('Privacy Policy'),
      'link_path' => 'node/' . $profile_intall_vars['nid_privacy'],
      'menu_name' => 'footer-menu',
      'weight' => 6,
    ),
    array(
      'link_title' => st('Terms of Use'),
      'link_path' => 'node/' . $profile_intall_vars['nid_terms'],
      'menu_name' => 'footer-menu',
      'weight' => 7,
    ),
  );

  // Delete baseline Footer menu links.
  menu_delete_links('footer-menu');

  // Add Footer menu items.
  foreach ($items_footer as $key => &$meta) {
    menu_link_save($meta);
  }

  $items_main = array(
    array(
      'link_title' => st('Home'),
      'link_path' => '<front>',
      'menu_name' => 'main-menu',
      'weight' => 3,
    ),
    array(
      'link_title' => st('Communities'),
      'link_path' => 'user/modal/login',
      'menu_name' => 'main-menu',
      'weight' => 4,
    ),
    array(
      'link_title' => st('Members'),
      'link_path' => 'user/modal/login',
      'menu_name' => 'main-menu',
      'weight' => 5,
    ),
    array(
      'link_title' => st('Discussions'),
      'link_path' => 'user/modal/login',
      'menu_name' => 'main-menu',
      'weight' => 6,
    ),
    array(
      'link_title' => st('Events'),
      'link_path' => 'user/modal/login',
      'menu_name' => 'main-menu',
      'weight' => 7,
    ),
    array(
      'link_title' => st('Media'),
      'link_path' => 'user/modal/login',
      'menu_name' => 'main-menu',
      'weight' => 8,
    ),
    array(
      'link_title' => st('Resources'),
      'link_path' => 'user/modal/login',
      'menu_name' => 'main-menu',
      'weight' => 9,
    ),
  );

  // Delete baseline Main menu links.
  menu_delete_links('main-menu');

  // Add Main menu items.
  foreach ($items_main as $key => &$meta) {
    menu_link_save($meta);
  }

  $items_community = array(
    array(
      'link_title' => st('Home'),
      'link_path' => 'user',
      'menu_name' => 'menu-community-menu',
      'weight' => 3,
    ),
    array(
      'link_title' => st('Communities'),
      'link_path' => 'communities',
      'menu_name' => 'menu-community-menu',
      'weight' => 4,
    ),
    array(
      'link_title' => st('Members'),
      'link_path' => 'members',
      'menu_name' => 'menu-community-menu',
      'weight' => 5,
    ),
    array(
      'link_title' => st('Discussions'),
      'link_path' => 'topics',
      'menu_name' => 'menu-community-menu',
      'weight' => 6,
    ),
    array(
      'link_title' => st('Events'),
      'link_path' => 'events',
      'menu_name' => 'menu-community-menu',
      'weight' => 7,
    ),
    array(
      'link_title' => st('Media'),
      'link_path' => 'media',
      'menu_name' => 'menu-community-menu',
      'weight' => 8,
    ),
    array(
      'link_title' => st('Resources'),
      'link_path' => 'resources',
      'menu_name' => 'menu-community-menu',
      'weight' => 9,
      'expanded' => TRUE,
    ),
  );

  // Delete baseline Community menu links.
  menu_delete_links('menu-community-menu');

  // Add Community menu items.
  foreach ($items_community as $key => &$meta) {
    menu_link_save($meta);
  }

  // Update the menu router information.
  menu_rebuild();
}
/**
 * TODO.
 */
function _totem_install_blocks() {

  // Get the current themes.
  $theme_default = variable_get('theme_default');
  $theme_admin = variable_get('admin_theme');

  // Update titles.
  db_update('block')
    ->fields(array('title' => '<none>'))
    ->condition('module', 'totem_common', '!=')
    ->execute();

  // Unassign blocks from admin theme.
  db_update('block')
    ->fields(array('region' => BLOCK_REGION_NONE))
    ->condition('module', 'system', '!=')
    ->condition('delta', 'main', '!=')
    ->condition('theme', $theme_admin)
    ->execute();


  // Alter Main and Community menus depending on user's initial
  // access_community_node_view config; main-menu is not really needed at all
  // in the case of granting anonymous access.
  // @see _totem_common_install_blocks()
  $anonymous = drupal_anonymous_user();
  if (user_access('access community node view', $anonymous)) {

    // Disable main-menu; can rely on full menu-community-menu instead.
    db_update('block')
      ->fields(array('status' => 0))
      ->condition('module', 'system', '=')
      ->condition('delta', 'main-menu', '=')
      ->execute();

    // Remove the authenticated user role restriction for menu-community-menu.
    db_delete('block_role')
      ->condition('module', 'menu')
      ->condition('delta', 'menu-community-menu')
      ->execute();
  }

}
/**
 * TODO.
 */
function _totem_install_rebuilds() {

  // Remove temp install var.
  variable_del('totem_install_temp_vars');

  // Grant new administrator role all permissions.
  user_role_grant_permissions(3, array_keys(module_invoke_all('permission')));

  // Rebuild the node access database.
  node_access_rebuild(FALSE);

}
