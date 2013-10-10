<?php
/**
 * @file
 * totem_user.theme.inc
 */

/**
 * Implements MODULE_preprocess_page().
 */
function totem_user_preprocess_page(&$vars) {

  global $user;

  if ($vars['is_admin']) {
    $vars['is_user_tab'] = FALSE;
    return;
  }

  // Add module UI assets.
  drupal_add_css(drupal_get_path('module', 'totem_user') . '/theme/totem_user.css');
  drupal_add_js(drupal_get_path('module', 'totem_user') . '/theme/totem_user.js', array(
    'scope' => 'footer',
  ));

  // Homepage = user profile if logged in.
  if ($vars['is_front']) {
    if (user_is_logged_in()) {
      drupal_goto('user');
    }
  }

  if (!user_is_logged_in()) {

    // Always, always init a session for anon users...as of D7, this is not
    // started by default. But without it, all kinds of system messages, etc.
    // never get printed.
    drupal_session_start();

    // If not logged in, force user to homepage account options; we don't want to
    // bother with theming all the core pages and creating a secondary UX for
    // the same functionality.
    if (in_array(drupal_get_path_alias(), array(
      'user',
      'user/login',
      'user/password',
      'user/register',
    ))) {
      $query = drupal_get_query_parameters();
      if (!empty($query['access']) && $query['access'] == variable_get('cron_key', 'drupal')) {
        // Allow access if cron_key var found...need a failover in case admins
        // really need to access core screens.
      }
      else {
        drupal_goto('');
      }
    }
  }


  $page = &$vars['page'];

  // Adjustments for page--user.tpl.php
  if (in_array('page__user', $vars['theme_hook_suggestions'])) {
    $local_task = arg(2);
    $vars['is_user_tab'] = !empty($local_task);
    if ($vars['is_user_tab']) {
      // Tab-page callback; use ninjutsu fighting style to rearrange renderable
      // arrays. (Note hardcoded duplication of standard Omega markup
      // in page--user.tpl.php.)
      $vars['wrapper_css_content'] = ' ' . $page['content']['content']['content']['#data']['wrapper_css'];
      $vars['wrapper_css_sidebar_first'] = NULL;
      if (!empty($page['content']['content']['sidebar_first'])) {
        $vars['wrapper_css_sidebar_first'] = ' ' . $page['content']['content']['sidebar_first']['#data']['wrapper_css'];
      }
    }
  }

  // Community tab page callback.
  if (!empty($vars['is_community_tab']) && $vars['is_community_tab']) {
    $type = &$page['content']['content']['content']['system_main']['#type'];
    if ($type == 'user') {

      // Set default intro text for community Members screen.
      $node = _totem_common_get_community_context_node();
      $intro = '';

      // Get specific manager uids associated to this community.
      $gid = _totem_user_get_managers($node);
      $gid = array_merge($gid, _totem_user_get_users_by_role('moderator'));

      // If manager, set intro text per current filter option.
      if (in_array($user->uid, $gid) || user_access('administer users')) {
        $filters = drupal_get_query_parameters();
        $filters_current = (!empty($filters['status']) ? $filters['status'] : 'member');

        switch ($filters_current) {
          case 'member':
            $intro = t("<p>This page lists all members of @title.</p><p>To manage a member's status in the @community, hover over their profile. Depending on your relationship to each member, you may or may not have permission to change their status in the @community.</p>", array(
              '@title' => $node->title,
              '@community' => t('community'),
            ));
            break;

          case 'manager':
            $intro = t("<p>This page lists all members that have manager privileges in @title.</p><p>To manage a member's status in the @community, hover over their profile. Depending on your relationship to each member, you may or may not have permission to change their status in the @community.</p>", array(
              '@title' => $node->title,
              '@community' => t('community'),
            ));
            break;

          case 'pending':
            $intro = t("<p>This page lists all members who have requested to join @title.</p><p>To manage a member's status in the @community, hover over their profile. Depending on your relationship to each member, you may or may not have permission to change their status in the @community.</p>", array(
              '@title' => $node->title,
              '@community' => t('community'),
            ));
            break;

          case 'blocked':
            $intro = t("<p>This page lists all members who have been blocked from @title.</p><p>To manage a member's status in the @community, hover over their profile. Depending on your relationship to each member, you may or may not have permission to change their status in the @community.</p>", array(
              '@title' => $node->title,
              '@community' => t('community'),
            ));
            break;
        }
      }

      // Add block for intro and reset sorting.
      $page['content']['content']['content']['#sorted'] = FALSE;
      $page['content']['content']['content']['system_main_intro'] = array(
        '#markup' => $intro,
        '#weight' => -1,
      );
    }
  }

}
/**
 * Implements MODULE_preprocess_node().
 */
function totem_user_preprocess_node(&$vars) {

  $node = &$vars['node'];

  if (!empty($node->uid)) {
    // See @totem_user_user_view_alter for build of "images"
    // array and fix for use of default user picture.
    $account = user_load($node->uid);
    $account = user_view($account, 'teaser');
    $vars['user_picture'] = $account['#account']->content['images'][variable_get('user_picture_style')];
  }

  if (!empty($vars['content']['body'][0]['#markup'])) {

    // Make sure any login/register/password links embedded in node body have
    // the correct destination query param. This has to be done server-side so
    // that CTools modal processes the correct path strings. Note that these
    // links will be hidden completely for logged-out users.
    // @see totem_user.js
    if (!user_is_logged_in() && class_exists('DOMDocument')) {
      $dom = new DOMDocument();
      $dom->loadHTML($vars['content']['body'][0]['#markup']);

      $search = array();
      $replace = array();
      foreach ($dom->getElementsByTagName('a') as $link) {
        $href = $link->getAttribute('href');
        $url = drupal_parse_url($href);
        $url['path'] = trim($url['path'], '/');
        if (in_array($url['path'], array('user/modal/login', 'user/modal/register', 'user/modal/password'))) {
          $destination = request_path();
          if (empty($destination)) {
            $destination = 'user';
          }
          if (empty($url['query']['destination'])) {
            $url['query']['destination'] = $destination;
          }

          $search[] = $href;
          $replace[] = url($url['path'], $url);
        }
      }

      $vars['content']['body'][0]['#markup'] = str_ireplace($search, $replace, $vars['content']['body'][0]['#markup']);
    }

  }


}
/**
 * Implements MODULE_preprocess_username().
 */
function totem_user_preprocess_username(&$vars) {
  // Do all the default stuff.
  template_preprocess_username($vars);

  // Reset name to untrimmed string.
  $name = $vars['name_raw'] = format_username($vars['account']);
  $vars['name'] = check_plain($name);
}
/**
 * Implements MODULE_preprocess_user_profile().
 */
function totem_user_preprocess_user_profile(&$vars) {

  global $user;

  $account = $vars['elements']['#account'];

  // @see totem_user_user_view_alter()
  // for $account->content population.
  foreach ($account->content as $key => $meta) {
    $vars['elements'][$key] = $account->content[$key];
  }

  // Add Owner/Manager label to profile.
  $vars['title_prefix']['label_perms'] = $vars['elements']['label_perms'];

  // Helpful $user_profile variable for templates.
  foreach (element_children($vars['elements']) as $key) {

    // Set CSV for term fields.
    if (!empty($vars['elements'][$key]['#field_type'])) {
      if ($vars['elements'][$key]['#field_type'] == 'taxonomy_term_reference') {
        $items = array();
        foreach ($vars['elements'][$key]['#items'] as $meta) {
          $items[] = $meta['taxonomy_term']->name;
        }

        $vars['elements'][$key] = array(
          '#markup' => implode(', ', $items),
        );
      }
    }

    // Assign field to template's $user_profile var.
    $vars['user_profile'][$key] = $vars['elements'][$key];

    // Hide all fields by default.
    hide($vars['user_profile'][$key]);
  }

  // Preprocess fields.
  field_attach_preprocess('user', $account, $vars['elements'], $vars);

  // Set template vars.
  $vars['view_mode'] = $vars['elements']['#view_mode'];
  $vars['is_own_profile'] = ($account->uid == $user->uid);

  $local_task = arg(2);
  $vars['is_user_tab'] = !empty($local_task);

  if ($vars['is_own_profile'] && !$vars['is_user_tab']) {
    $vars['classes_array'][] = 'self';
  }

  // Add class to support custom "View more" paging.
  $vars['classes_array'][] = 'pager-entity';

  if ($vars['view_mode'] == 'teaser') {
    return;
  }

  // Everything below is only applicable to full profile view.
  $vars['classes_array'][] = 'profile';

  //$vars['title'] = $vars['elements']['menu_local_task']['#title'];

}
