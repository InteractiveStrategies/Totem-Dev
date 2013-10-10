<?php
/**
 * @file
 * totem_user.form.inc
 */

// User forms.
/**
 * Shared menu callback for user login/password/register modal forms.
 */
function totem_user_form_account($form_id, $title) {

  // Now respond to standard non-logged-in form requests.
  $form_state = array(
    'title' => t($title),
    'ajax' => TRUE,
    'build_info' => array(
      'args' => array(
        array(
          'modal' => TRUE,
        ),
      ),
    ),
  );

  form_load_include($form_state, 'inc', 'user', 'user.pages');
  ctools_include('modal');
  ctools_include('ajax');

  $output = ctools_modal_form_wrapper($form_id, $form_state);

  if (!empty($form_state['executed'])) {
    ctools_add_js('ajax-responder');

    // Overwrite the form output.
    $output = array();
    if (!empty($form_state['redirect'])) {
      $output[] = ctools_ajax_command_redirect($form_state['redirect']);
    }
    else {
      $output[] = ctools_ajax_command_reload();
    }
  }

  // Return array of AJAX commands, as ajax_deliver() expects.
  return array(
    '#type' => 'ajax',
    '#commands' => $output,
  );
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function totem_user_form_user_login_alter(&$form, &$form_state, $form_id) {

  $args = array();
  if (!empty($form_state['build_info']['args'])) {
    $args = $form_state['build_info']['args'][0];
  }

  $form['#prefix'] = (array_key_exists('intro_login', $args) ? $args['intro_login'] : '');

  // Facebook OAuth stuff.
  $facebook_oauth = NULL;
  $facebook_oauth_id = variable_get('fboauth_id');
  if (module_exists('fboauth') && !empty($facebook_oauth_id)) {
    // Module exists and FB app is defined; get login block.
    // @see fboauth_block_view(), totem_ui_fboauth_action__connect()
    $facebook_oauth = _totem_common_embed_block('fboauth', 'login');
    $form['#prefix'] .= drupal_render($facebook_oauth);
  }

  $validators = user_login_default_validators();

  // Replace user module's user_login_final_validate function
  // with custom implementation so we can control messaging.
  $validators = array_flip($validators);
  unset($validators['user_login_final_validate']);
  $validators = array_flip($validators);
  $validators[] = 'totem_user_login_final_validate';


  $form['#validate'] = $validators;
  $form['#submit'][] = 'totem_user_form_user_login_submit';
  $form['name']['#title'] = t('Email address');
  $form['name']['#description'] = NULL;
  $form['pass']['#title'] = t('Password');
  $form['pass']['#description'] = NULL;
  $form['persistent_login']['#title'] = t('Keep me signed in');
  $form['actions']['submit']['#value'] = t('Sign in');

  // Append account links.
  $query = drupal_get_destination();
  $items = array();
  $items[] = l(t('Forgot your password?'), 'user/modal/password', array('query' => $query, 'attributes' => array('title' => t('Forgot your password?'), 'class' => array('ctools-use-modal'))));
  if (variable_get('user_register', USER_REGISTER_VISITORS)) {
    $items[] = 'Not a Member? ' . l(t('Register Today'), 'user/modal/register', array('query' => $query, 'attributes' => array('title' => t('Register'), 'class' => array('ctools-use-modal'))));
  }

  $form['links'] = array('#weight' => 1000, '#markup' => theme('item_list', array('items' => $items, 'attributes' => array('class' => 'clearfix'))));

}
/**
 * Custom implementation of core's user_login_final_validate function. Used to
 * control error messaging.
 */
function totem_user_login_final_validate($form, &$form_state) {
  if (empty($form_state['uid'])) {
    // Always register an IP-based failed login event.
    flood_register_event('failed_login_attempt_ip', variable_get('user_failed_login_ip_window', 3600));
    // Register a per-user failed login event.
    if (isset($form_state['flood_control_user_identifier'])) {
      flood_register_event('failed_login_attempt_user', variable_get('user_failed_login_user_window', 21600), $form_state['flood_control_user_identifier']);
    }

    if (isset($form_state['flood_control_triggered'])) {
      if ($form_state['flood_control_triggered'] == 'user') {
        form_set_error('name', format_plural(variable_get('user_failed_login_user_limit', 5), 'Sorry, there has been more than one failed login attempt for this account. It is temporarily blocked. Try again later or <a href="@url">request a new password</a>.', 'Sorry, there have been more than @count failed login attempts for this account. It is temporarily blocked. Try again later or <a href="@url">request a new password</a>.', array('@url' => url('user/password'))));
      }
      else {
        // We did not find a uid, so the limit is IP-based.
        form_set_error('name', t('Sorry, too many failed login attempts from your IP address. This IP address is temporarily blocked. Try again later or <a href="@url">request a new password</a>.', array('@url' => url('user/password'))));
      }
    }
    else {
      form_set_error('name', t('Sorry, unrecognized username or password.'));
      watchdog('user', 'Login attempt failed for %user.', array('%user' => $form_state['values']['name']));
    }
  }
  elseif (isset($form_state['flood_control_user_identifier'])) {
    // Clear past failures for this user so as not to block a user who might
    // log in and out more than once in an hour.
    flood_clear_event('failed_login_attempt_user', $form_state['flood_control_user_identifier']);
  }
}
/**
 * Submit callback for the user login forms.
 */
function totem_user_form_user_login_submit($form, &$form_state) {
  $form_state['redirect'] = (!empty($_GET['destination']) ? $_GET['destination'] : 'user');
}
/**
 * Implements hook_form_FORM_ID_alter().
 */
function totem_user_form_user_pass_alter(&$form, &$form_state, $form_id) {

  $args = array();
  if (!empty($form_state['build_info']['args'])) {
    $args = $form_state['build_info']['args'][0];
  }

  $form['#prefix'] = (array_key_exists('intro_password', $args) ? $args['intro_password'] : NULL);
  $form['name']['#title'] = t('Email address');
  $form['actions']['submit']['#value'] = t('Reset password');

  // Append account links.
  $query = drupal_get_destination();
  $items = array();
  $items[] = 'Existing member? ' . l(t('Sign in'), 'user/modal/login', array('query' => $query, 'attributes' => array('title' => t('Sign in'), 'class' => array('ctools-use-modal'))));
  if (variable_get('user_register', USER_REGISTER_VISITORS)) {
    $items[] = 'Not a Member? ' . l(t('Register Today'), 'user/modal/register', array('query' => $query, 'attributes' => array('title' => t('Register'), 'class' => array('ctools-use-modal'))));
  }

  $form['links'] = array('#weight' => 1000, '#markup' => theme('item_list', array('items' => $items, 'attributes' => array('class' => 'clearfix'))));

}
/**
 * Implements hook_form_FORM_ID_alter().
 */
function totem_user_form_user_register_form_alter(&$form, &$form_state, $form_id) {

  // Don't alter this form if user has "administer users" perm.
  if (user_access('administer users')) {
    return;
  }

  $args = array();
  if (!empty($form_state['build_info']['args'])) {
    $args = $form_state['build_info']['args'][0];
  }

  // Force username to hidden unique id
  // mail will be copied to name field in hook_user_presave.
  $form['account']['name'] = array(
    '#type' => 'hidden',
    '#value' => uniqid('', TRUE),
  );

  $form['#prefix'] = (array_key_exists('intro_register', $args) ? $args['intro_register'] : NULL);

  // Facebook OAuth stuff.
  $facebook_oauth = NULL;
  $facebook_oauth_id = variable_get('fboauth_id');
  if (module_exists('fboauth') && !empty($facebook_oauth_id)) {
    // Module exists and FB app is defined; get login block.
    $facebook_oauth = _totem_common_embed_block('fboauth', 'login');
    $form['#prefix'] .= drupal_render($facebook_oauth);
  }

  $form['#submit'][] = 'totem_user_form_user_register_form_submit';
  $form['account']['mail']['#title'] = t('Email address');
  $form['account']['mail']['#description'] = NULL;
  // Force standard password field (vs. pass w/confirm and strength).
  if (!empty($form['account']['pass'])) {
    $form['account']['pass']['#type'] = 'password';
    $form['account']['pass']['#title'] = t('Password');
  }
  $form['actions']['submit']['#value'] = t('Register');

  unset($form['account']['persistent_login']);

  // Alter terms_of_use mod's output.
  $form['terms_of_use']['terms_of_use']['#title'] = str_replace('<a ', '<a target="_blank" ', $form['terms_of_use']['terms_of_use']['#title']);

  // Append account links.
  $query = drupal_get_destination();
  $items = array();
  $items[] = 'Existing member? ' . l(t('Sign in'), 'user/modal/login', array('query' => $query, 'attributes' => array('title' => t('Sign in'), 'class' => array('ctools-use-modal'))));

  $form['links'] = array('#weight' => 1000, '#markup' => theme('item_list', array('items' => $items, 'attributes' => array('class' => 'clearfix'))));

}
/**
 * Submit callback for the user registration form.
 */
function totem_user_form_user_register_form_submit($form, &$form_state) {

  // If user registration is not completed immediately on submit, give the user
  // a customized message to explain.
  $user_email_verification = variable_get('user_email_verification', FALSE);
  $user_approval = (variable_get('user_register', USER_REGISTER_VISITORS) == USER_REGISTER_VISITORS_ADMINISTRATIVE_APPROVAL);

  $message = "";

  if ($user_email_verification && $user_approval) {
    $message = t("Please check your email to complete the next step of your registration.<br/>Your account will then be awaiting administrator approval.");
  }
  elseif ($user_email_verification) {
    $message = t('Please check your email to complete your registration.');
  }
  elseif ($user_approval) {
    $message = t("Your account has been created and is now awaiting administrator approval.");
  }

  if (!empty($message)) {
    // Clear default status messages and add our own.
    // @see user_register_submit()
    drupal_get_messages('status');
    drupal_set_message($message);
  }

  // TODO: no redirect if registration not yet completed?
  // i.e. if (!($user_email_verification || $user_approval))
  $form_state['redirect'] = (!empty($_GET['destination']) ? $_GET['destination'] : 'user');

}
/**
 * Implements hook_form_FORM_ID_alter().
 */
function totem_user_form_user_profile_form_alter(&$form, &$form_state, $form_id) {

  $account = $form_state['user'];

  $form['#submit'][] = 'totem_user_form_user_profile_form_submit';

  // Force username to hidden unique id
  // mail will be copied to name field in hook_user_presave
  $form['account']['name'] = array(
    '#type' => 'hidden',
    '#value' => $form['account']['mail']['#default_value'],
  );



  // BEGIN LAYOUT STUFF.
  $form['#prefix'] = '<h1>My Account</h1>';

  // Profile Settings group
  $form['picture']['#weight'] = -500;
  $form['picture']['#prefix'] = '<div class="settings-profile"><h2>Edit Profile</h2><div class="row clearfix">';
  $form['picture']['#suffix'] = '</div>';
  unset($form['picture']['#title']);
  $form['field_name_first']['#weight'] = -490;
  $form['field_name_first']['#prefix'] = '<div class="row clearfix">';
  $form['field_name_last']['#weight'] = -480;
  $form['field_name_last']['#suffix'] = '</div>';
  $form['field_bio']['#weight'] = -470;
  $form['field_bio']['#prefix'] = '<div class="row clearfix">';
  $form['field_bio']['#suffix'] = '</div>';
  $form['field_location']['#weight'] = -460;
  $form['field_location']['#suffix'] = '<div class="clearfix"></div></div>';

  // $form['timezone']['#weight'] = -850;
  // $form['timezone']['#suffix'] = '<div class="clearfix"></div></div>';
  // $form['timezone']['#collapsible'] = FALSE;
  unset($form['timezone']);

  // Account Settings group.
  $form['account']['#weight'] = -300;
  $form['account']['#prefix'] = '<div class="settings-account"><h2>Account Settings</h2>';
  $form['account']['#suffix'] = '<div class="clearfix"></div></div>';
  $form['account']['current_pass']['#weight'] = 210;
  $form['account']['current_pass']['#prefix'] = '<div class="row clearfix">';
  $form['account']['current_pass']['#description'] = t('Enter your current password to change the <em class="placeholder">Email address</em> or <em class="placeholder">Password</em>.');
  $form['account']['pass']['#weight'] = 220;
  $form['account']['pass']['#suffix'] = '</div>';
  $form['account']['pass']['#type'] = 'password'; // Force standard password field (vs. pass w/confirm and strength)
  $form['account']['pass']['#title'] = t('New Password');
  unset($form['account']['pass']['#description']);
  $form['account']['mail']['#weight'] = 230;
  $form['account']['mail']['#prefix'] = '<div class="row clearfix">';
  $form['account']['mail']['#suffix'] = '</div>';
  $form['account']['mail']['#title'] = t('Email address');
  $form['account']['mail']['#description'] = t('A valid email address. All emails from the system will be sent to this address.');
  if (!empty($form['account']['fboauth'])) {
    $form['account']['fboauth']['#weight'] = 240;
  }

  // Message Settings group.
  $form['mimemail']['#weight'] = -200;
  $form['mimemail']['#prefix'] = '<div class="settings-messaging"><h2>Email Preferences</h2>';
  $form['mimemail']['#collapsible'] = FALSE;
  unset($form['mimemail']['#title']);

  $form['privatemsg']['#weight'] = -190;
  $form['privatemsg']['#suffix'] = '<div class="clearfix"></div></div>';
  $form['privatemsg']['#collapsible'] = FALSE;
  unset($form['privatemsg']['#title']);

  // END LAYOUT STUFF.

}
/**
 * Submit callback for the user profile form.
 */
function totem_user_form_user_profile_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'user';
}
/**
 * Implements hook_TYPE_alter().
 */
function totem_user_location_element_alter(&$element) {

  $element['#collapsible'] = FALSE;
  $element['#collapsed'] = FALSE;

  unset($element['#title']);
  unset($element['name']);

  $element['country']['#prefix'] = '<div class="row clearfix">';
  $element['country']['#suffix'] = '</div>';

  $element['street']['#title'] = 'Address 1';
  $element['street']['#prefix'] = '<div class="row clearfix">';
  $element['additional']['#title'] = 'Address 2';
  $element['additional']['#suffix'] = '</div>';

  $element['city']['#prefix'] = '<div class="row clearfix">';
  $element['province']['#suffix'] .= '</div>';

  $element['postal_code']['#prefix'] = '<div class="row clearfix">';
  $element['phone']['#suffix'] = '</div>';

}

// Node forms.
/**
 * Implements hook_form_FORM_ID_alter().
 */
function totem_user_form_node_form_alter(&$form, &$form_state, $form_id) {

  // Actions for all node form submits.
  $form['actions']['submit']['#submit'][] = 'totem_user_form_node_form_submit';

}
/**
 * Custom submit action for all node add/edit ops.
 * PERMISSION
 */
function totem_user_form_node_form_submit($form, &$form_state) {

  // Get the new node.
  $node = node_load($form_state['values']['nid']);

  // Auto-assign community nid to user's field_community LOV.
  global $user;

  $nid_community = NULL;
  if ($node->type == 'community') {
    $nid_community = $form_state['values']['nid'];
  }
  else {
    if (!empty($form_state['values']['field_community']) && !empty($form_state['values']['field_community'][LANGUAGE_NONE])) {
      $nid_community = $form_state['values']['field_community'][LANGUAGE_NONE][0]['target_id'];
    }
  }

  if ($nid_community) {
    _totem_common_set_field_entityreference_values('user', $user, 'field_community', array($nid_community));
  }

}

// Custom forms.
/**
 * Modal node permissions/join/invite/leave/remove/block forms.
 *
 * PERMISSION
 */
function totem_user_form_node($op, $node, $account = NULL) {

  global $user;
  if (empty($account)) {
    $account = user_load($user->uid);
  }

  $title =
  $form_id = NULL;
  $args = array(
    'op' => $op,
    'node' => $node,
    'account' => $account,
  );

  switch ($op) {
    case 'permissions':
      // 1-click action to grant/revoke "manager" permissions on community node per user.
      $realm = 'totem_user_node_manager';
      $grants_managers = _totem_user_get_managers($node);

      if (!in_array($account->uid, $grants_managers)) {
        // Grant "manager" perms to this $account
        $record = array(
          'nid' => $node->nid,
          'gid' => $account->uid,
          'realm' => $realm,
          'grant_view' => 1,
          'grant_update' => 1,
          'grant_delete' => 0,
        );

        drupal_write_record('node_access', $record);

        drupal_set_message(t('Manager privileges granted to %name.', array('%name' => $account->realname)));
      }
      else {
        // Revoke "manager" perms for this $account
        db_delete('node_access')
          ->condition('realm', $realm)
          ->condition('nid', $node->nid)
          ->condition('gid', $account->uid)
          ->execute();

        drupal_set_message(t('Manager privileges revoked for %name.', array('%name' => $account->realname)));
      }

      // Allow other mods to respond to this action.
      module_invoke_all('user_community_form_submit', $args);

      drupal_goto(drupal_get_path_alias());
      exit;
      break;

    case 'join':
      $node_entity = entity_metadata_wrapper('node', $node);
      if ($node_entity->field_community_status->value() == TOTEM_COMMUNITY_STATUS_OPEN) {
        // 0: "Open" community; this section defines the non-modal, 1-click
        // "Join Community" action.
        // (see totem_user_block_view_button_add_user) for triggering link
        // as well.
        //
        // d. Open albums can be joined by all members
        // (see totem_user_menu( for access control).
        //
        // Assign community nid to user's field_community LOV.
        _totem_common_set_field_entityreference_values('user', $account, 'field_community', array($node->nid));
        drupal_set_message(t('You are now a member of the "@title" @community.', array('@title' => $node->title, '@community' => t('Community'))));

        // Allow other mods to respond to this action.
        module_invoke_all('user_community_form_submit', $args);

        drupal_goto('node/' . $node->nid);
        exit;

        // 1: "Restricted" community.
        // 2: "Closed" community.
        // See totem_user_form_node_add_user_submit() for join request action.
        // See case "approve" in this switch for manager join approval action.
      }

      $title = t('Join @community', array('@community' => t('Community')));
      $form_id = 'totem_user_form_node_add_user';
      break;

    case 'approve':
      // Add community nid to user's field_community LOV.
      _totem_common_set_field_entityreference_values('user', $account, 'field_community', array($node->nid));
      // Remove community nid from user's field_community_pending LOV.
      _totem_common_set_field_entityreference_values('user', $account, 'field_community_pending', array($node->nid), TRUE);

      drupal_set_message(t('%name has been added to %title.', array('%title' => $node->title, '%name' => $account->realname)));

      // Allow other mods to respond to this action.
      module_invoke_all('user_community_form_submit', $args);

      drupal_goto('node/' . $node->nid);
      exit;
      break;

    case 'request_unblock':
      $title = t('Join @community', array('@community' => t('Community')));
      $form_id = 'totem_user_form_node_add_user';
      break;

    case 'invite':
      $title = t('Invite Friends');
      $form_id = 'totem_user_form_node_add_user';
      break;

    case 'leave':
      $title = t('Leave @community', array('@community' => t('Community')));
      $form_id = 'totem_user_form_node_remove_user';
      break;

    case 'remove':
      $title = t('Remove member');
      $form_id = 'totem_user_form_node_remove_user';
      break;

    case 'block':
      $title = t('Block member');
      $form_id = 'totem_user_form_node_remove_user';
      break;

    case 'unblock':
      // Add community nid to user's field_community LOV.
      _totem_common_set_field_entityreference_values('user', $account, 'field_community', array($node->nid));
      // Remove community nid from user's field_community_blocked LOV.
      _totem_common_set_field_entityreference_values('user', $account, 'field_community_blocked', array($node->nid), TRUE);

      drupal_set_message(t('%name has been unblocked and added to %title.', array('%title' => $node->title, '%name' => $account->realname)));

      // Allow other mods to respond to this action.
      module_invoke_all('user_community_form_submit', $args);

      drupal_goto('node/' . $node->nid);
      exit;
      break;

    default:
      break;
  }

  $form_state = array(
    'title' => $title,
    'ajax' => TRUE,
    'build_info' => array(
      'args' => $args,
    ),
  );

  ctools_include('modal');
  ctools_include('ajax');

  $output = ctools_modal_form_wrapper($form_id, $form_state);

  if (!empty($form_state['executed'])) {
    ctools_add_js('ajax-responder');

    // Allow other mods to respond to this action.
    $args['form_state'] = $form_state;
    module_invoke_all('user_community_form_submit', $args);

    $output = array();
    if (!empty($form_state['redirect'])) {
      $output[] = ctools_ajax_command_redirect($form_state['redirect']);
    }
    else {
      $output[] = ctools_ajax_command_reload();
    }
  }

  print ajax_render($output);
  exit;
}
/**
 * Custom form to Join Community or Invite Friends.
 */
function totem_user_form_node_add_user($form, &$form_state) {

  global $user;

  $op = $form_state['build_info']['args']['op'];
  $node = $form_state['build_info']['args']['node'];
  $account = $form_state['build_info']['args']['account'];

  // Build form.
  $form = array(
    '#submit' => array('totem_user_form_node_add_user_submit'),
    '#op' => $op,
    '#node' => $node,
    '#account' => $account,
  );

  switch ($op) {
    case 'request_unblock':
      drupal_set_message(t('You have been blocked from this @community. A manager must first unblock you.', array('@community' => t('Community'))), 'warning');

      $form['message'] = array(
        '#title' => t('Message to @community managers', array('@community' => t('Community'))),
        '#type' => 'textarea',
      );

      $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit request'),
      );

      break;

    case 'join':
      $form['#prefix'] = t('<p>This @community requires authorization to join. You can submit a request to join the "@title" @community here.</p>', array('@title' => $node->title, '@community' => t('Community')));

      $form['message'] = array(
        '#title' => t('Message to @community managers', array('@community' => t('Community'))),
        '#type' => 'textarea',
      );

      $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit request'),
      );

      // Alter form a bit if user has already submitted a join request.
      $node_entity = entity_metadata_wrapper('node', $node);
      if (in_array($node_entity->field_community_status->value(), array(TOTEM_COMMUNITY_STATUS_RESTRICTED, TOTEM_COMMUNITY_STATUS_CLOSED))) {
        $nid_pending = _totem_common_get_field_entityreference_values('user', $account, 'field_community_pending');
        if (in_array($node->nid, $nid_pending)) {
          $form['#prefix'] = t('<p>Your request to join the "@title" @community is pending approval.</p>', array('@title' => $node->title, '@community' => t('Community')));
          $form['actions']['submit']['#value'] = t('Send message');
        }
      }

      break;

    case 'invite':
      $form['#prefix'] = '<p>';
      $form['#prefix'] .= t('Invite friends to join the "@title" @community here.', array('@title' => $node->title, '@community' => t('Community')));

      $form['mail'] = array(
        '#title' => t('Email addresses; enter 1 per line'),
        '#type' => 'textarea',
        '#required' => TRUE,
      );

      // "Invite Friends" via Facebook support.
      $facebook_oauth_id = variable_get('fboauth_id');
      if (module_exists('fboauth') && !empty($facebook_oauth_id)) {

        $facebook_oauth_access = _totem_common_session_get('totem_user_fboauth_invite_app_access');
        $facebook_oauth_friends = _totem_common_session_get('totem_user_fboauth_invite');

        $form['#fboauth_access'] = $facebook_oauth_access;

        // Check if FB Graph query has already been completed.
        if (empty($facebook_oauth_access) || empty($facebook_oauth_friends)) {
          // Append custom Facebook invite link.
          $form['#prefix'] .= '<p>' . fboauth_action_display('totem_user_fboauth_invite') . '</p>';
        }
        else {
          // If Facebook friend GET has returned, present friends list.
          $fbid = fboauth_fbid_load($user->uid);

          $options = array();
          $options_first = NULL;
          foreach ($facebook_oauth_friends as $key => &$meta) {
            if ($meta->uid == $fbid) {
              // Put user's own FB profile at top of options list.
              $options_first[$meta->uid] = $meta->name;
            }
            else {
              $options[$meta->uid] = $meta->name;
            }
          }

          $form['mail'] = array(
            '#title' => t("Post invitations on these Facebook friend's walls")
            . l(t('Select all'), '', array('attributes' => array('class' => array('toggle-checked')))),
            '#type' => 'checkboxes',
            '#options' => ($options_first + $options),
            '#required' => TRUE,
          );

          // Remove FB data from session; lot to be carrying around here.
          _totem_common_session_set('totem_user_fboauth_invite', NULL);
        }
      }

      $form['message'] = array(
        '#title' => t('Message'),
        '#type' => 'textarea',
        '#attributes' => array('rows' => '5'),
      );

      $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Send invitations'),
      );
      break;
  }

  return $form;
}
/**
 * Submit action for Join Community or Invite Friend form.
 */
function totem_user_form_node_add_user_submit(&$form, &$form_state) {

  if (!form_get_errors()) {

    $op = $form['#op'];
    $node = $form['#node'];
    $account = $form['#account'];

    switch ($op) {
      case 'request_unblock':
        // See totem_discuss_user_community_form_submit for building and
        // sending private message to community managers.
        drupal_set_message(t('Unblock request submitted.'));
        break;

      case 'join':
        $node_entity = entity_metadata_wrapper('node', $node);
        switch ($node_entity->field_community_status->value()) {
          // Open.
          case TOTEM_COMMUNITY_STATUS_OPEN:
            // See totem_user_form_node():$op=join
            // Non-form "add to LOV" action already accounted for, making this
            // code redundant.
            break;

          // Restriced.
          case TOTEM_COMMUNITY_STATUS_RESTRICTED:
          // Closed.
          case TOTEM_COMMUNITY_STATUS_CLOSED:
            $nid_pending = _totem_common_get_field_entityreference_values('user', $account, 'field_community_pending');
            if (in_array($node->nid, $nid_pending)) {
              drupal_set_message(t('Your message has been sent.'));
            }
            else {
              // Add community nid to user's pending field_community LOV.
              _totem_common_set_field_entityreference_values('user', $account, 'field_community_pending', array($node->nid), FALSE);

              drupal_set_message(t('Your join request has been submitted.'));
            }
            break;
        }
        break;

      case 'invite':

        // TODO: Improve this handling. If mail is array, assume we're
        // processing a checkbox list of FB friends.
        if (is_array($form_state['values']['mail'])) {
          totem_user_form_node_add_user_submit_fboauth_invite($form, $form_state);
          return;
        }

        $mail = drupal_strtolower(trim($form_state['values']['mail']));
        $mail = preg_replace('/(\r\n?|\n)/', '|', $mail);
        $mail = explode('|', $mail);
        $recipients_member
            = $recipients_nonmember = array();

        foreach ($mail as $address) {
          if (!empty($mail)) {
            // See if this user already exists.
            $recipient = user_load_by_mail($address);

            if (!empty($recipient)) {
              $user_community_all = array();
              // Don't invite if user has community.
              $user_community_all = array_merge($user_community_all, _totem_common_get_field_entityreference_values('user', $recipient, 'field_community'));
              // Don't invite if user is blocked.
              $user_community_all = array_merge($user_community_all, _totem_common_get_field_entityreference_values('user', $recipient, 'field_community_blocked'));
              // Don't invite if user is pending.
              $user_community_all = array_merge($user_community_all, _totem_common_get_field_entityreference_values('user', $recipient, 'field_community_pending'));

              if (!in_array($node->nid, $user_community_all)) {
                $recipients_member[] = $recipient->mail;
              }
            }
            else {
              $recipients_nonmember[] = $address;
            }
          }
        }

        // Message subject+body finalized elsewhere.
        // @see totem_user_mail()
        drupal_mail(
          'totem_user',
          'invite_members',
          $recipients_member,
          language_default(),
          array(
            'node' => $node,
            'account' => $account,
            'invite_message' => $form_state['values']['message'],
          )
        );

        drupal_mail(
          'totem_user',
          'invite_nonmembers',
          $recipients_nonmember,
          language_default(),
          array(
            'node' => $node,
            'account' => $account,
            'invite_message' => $form_state['values']['message'],
          )
        );

        drupal_set_message(t('Invitations sent.'));
        break;
    }

  }
}
/**
 * Custom form to Leave or Remove from Community.
 */
function totem_user_form_node_remove_user($form, &$form_state) {

  $op = $form_state['build_info']['args']['op'];
  $node = $form_state['build_info']['args']['node'];
  $account = $form_state['build_info']['args']['account'];
  $description
      = $submit_text
      = NULL;

  switch ($op) {
    case 'leave':
      $description = t('Are you sure you want to leave %title?', array('%title' => $node->title));
      $submit_text = t('Leave @community', array('@community' => t('Community')));
      break;

    case 'remove':
      $description = t('Are you sure you want to remove %name from %title?', array('%title' => $node->title, '%name' => $account->realname));
      $submit_text = t('Remove Member');
      break;

    case 'block':
      $description = t('Are you sure you want to block %name from %title? This member will be removed from the @community until a manager unblocks and adds the member again. The member\'s content will also be removed from the @community.', array('%title' => $node->title, '%name' => $account->realname, '@community' => t('Community')));
      $submit_text = t('Block Member');
      break;
  }

  // Build form
  $form = array(
    '#submit' => array('totem_user_form_node_remove_user_submit'),
    '#op' => $op,
    '#node' => $node,
    '#account' => $account,
  );

  return confirm_form(
    $form,
    $description,
    'node/' . $node->nid,
    $description,
    $submit_text,
    t('Cancel')
  );
}
/**
 * Submit action for Leave Community form.
 *
 * PERMISSION
 */
function totem_user_form_node_remove_user_submit(&$form, &$form_state) {

  if ($form_state['values']['confirm']) {

    $op = $form['#op'];
    $node = $form['#node'];
    $account = $form['#account'];
    $message =
    $redirect = NULL;

    switch ($op) {
      case 'leave':
        $message = t('You have left %title.', array('%title' => $node->title));
        $redirect = 'user';

        // Remove community nid from user's field_community LOV.
        _totem_common_set_field_entityreference_values('user', $account, 'field_community', array($node->nid), TRUE);
        break;

      case 'remove':
        $message = t('%name has been removed from %title.', array('%title' => $node->title, '%name' => $account->realname));

        // Remove community nid from user's field_community LOV.
        _totem_common_set_field_entityreference_values('user', $account, 'field_community', array($node->nid), TRUE);
        break;

      case 'block':
        // Get a list of all nodes associated with this community.
        // Force node table to be joined.
        $efq_params = array(
          'return' => 'entity_id',
          'entity_type' => 'node',
          'property_conditions' => array(
            array('column' => 'status', 'value' => 1),
          ),
          'field_conditions' => array(
            array('field' => 'field_community', 'column' => 'target_id', 'value' => $node->nid),
          ),
        );

        $all_nids = _totem_common_efq($efq_params)->results;

        // Remove community nid from user's field_community LOV.
        _totem_common_set_field_entityreference_values('user', $account, 'field_community', array($node->nid), TRUE);
        // Add community nid to user's field_community_blocked LOV.
        _totem_common_set_field_entityreference_values('user', $account, 'field_community_blocked', array($node->nid), FALSE);

        // Now remove all user content from this community.
        $efq_params = array(
          'return' => 'entity',
          'entity_type' => 'node',
          'property_conditions' => array(
            array('column' => 'uid', 'value' => $account->uid),
          ),
          'field_conditions' => array(
            array('field' => 'field_community', 'column' => 'target_id', 'value' => $node->nid),
          ),
        );

        $obj = _totem_common_efq($efq_params);
        $entities = $obj->results;
        foreach ($entities as $key => $entity) {
          _totem_common_set_field_entityreference_values('node', $entity, 'field_community', array($node->nid), TRUE);
        }

        // Delete all user comments across all nodes in this community.
        if (module_exists('comment')) {
          $cids = db_select('comment', 'c')
            ->fields('c', array('cid'))
            ->condition('nid', $all_nids, 'IN')
            ->condition('uid', $account->uid, '=')
            ->execute()
            ->fetchCol();

          comment_delete_multiple($cids);
        }

        $message = t("%name has been removed from %title and blocked from re-joining. The member's content has also been removed from the @community.", array('%title' => $node->title, '%name' => $account->realname, '@community' => t('Community')));
        break;
    }

    // Revoke all user privileges for this realm.
    db_delete('node_access')
      ->condition('realm', 'totem_user_node_manager')
      ->condition('nid', $node->nid)
      ->condition('gid', $account->uid)
      ->execute();

    drupal_set_message($message);

    if (!empty($redirect)) {
      $form_state['redirect'] = $redirect;
    }

  }
}

// "Invite Facebook Friends" support.
/**
 * Submit action for Invite Friends form Facebook variant.
 */
function totem_user_form_node_add_user_submit_fboauth_invite(&$form, &$form_state) {

  if (!form_get_errors()) {

    $node = $form['#node'];
    $facebook_oauth_access = $form['#fboauth_access'];

    $recipients = array_flip($form_state['values']['mail']);
    unset($recipients[0]);
    $recipients = array_keys($recipients);
    $link = url("node/{$node->nid}", array('absolute' => TRUE));
    $message = trim($form_state['values']['message']);

    // Post links to friends' feeds.
    // @see http://developers.facebook.com/docs/reference/api/user/#links
    foreach ($recipients as $uid) {
      $query = fboauth_graph_query("{$uid}/feed", $facebook_oauth_access['access_token'], array(
          'link' => $link,
          'message' => $message,
        ),
        'POST'
      );
    }

    // Remove app access args from session.
    _totem_common_session_set('totem_user_fboauth_invite_app_access', NULL);

    drupal_set_message(t('Invitations sent.'));

    $form_state['redirect'] = "node/{$node->nid}";
  }

}
/**
 * Implements hook_fboauth_actions().
 */
function totem_user_fboauth_actions() {

  $perms = array(
    'email',
    'publish_stream',
  );

  $items['totem_user_fboauth_invite'] = array(
    'title' => t('Post invitations to your friends on Facebook'),
    'file' => 'includes/totem_user.form.inc',
    'callback' => 'totem_user_fboauth_actions_invite',
    'theme' => 'totem_user_fboauth_invite',
    // @see http://developers.facebook.com/docs/authentication/permissions/
    'permissions' => $perms,
  );

  return $items;
}
/**
 * Callback for totem_user_fboauth_invite action. This callback is responsible
 * for retrieving user + friends data.
 */
function totem_user_fboauth_actions_invite($app_id, $access_token) {

  global $user;

  $fbid = fboauth_fbid_load($user->uid);
  if (empty($fbid)) {
    fboauth_action_connect($app_id, $access_token);
  }

  // Get me() + all my friends.
  // @see http://developers.facebook.com/docs/technical-guides/fql/
  $query = fboauth_graph_query('fql', $access_token, array(
      'q' => "SELECT uid,name FROM user WHERE uid = me() OR uid IN (SELECT uid2 FROM friend WHERE uid1 = me()) ORDER BY last_name",
    )
  );

  // Add args to session for later reuse in form submit.
  _totem_common_session_set('totem_user_fboauth_invite_app_access', array(
    'app_id' => $app_id,
    'access_token' => $access_token,
  ));

  // Shove me and all my friends in to session. fboauth_action_page takes over
  // from here and redirects to ?destination=X.
  // @see theme_totem_user_fboauth_invite()
  _totem_common_session_set('totem_user_fboauth_invite', $query->data);

}
/**
 * Link theme function for totem_user_fboauth_invite fboauth_action.
 * @see totem_user_theme()
 */
function theme_totem_user_fboauth_invite(&$vars) {

  global $user;

  $action = $vars['action'];
  $link = $vars['properties'];

  $title = isset($link['title']) ? $link['title'] : $action['title'];
  $attributes = isset($link['attributes']) ? $link['attributes'] : array();
  $attributes['class'][] = 'facebook-invite-friends';

  // Add a "destination" query param to fboauth's redirect_uri param.
  // @see fboauth_action_page()
  $node_community = _totem_common_get_community_context_node();
  $link['query']['redirect_uri'] .= '?destination='
  . trim(url("node/{$node_community->nid}", array(
      'query' => array(
        // Append autoload_modal param.
        // @see totem_common_preprocess_page()
        'modal' => url("user/{$user->uid}/modal/invite/{$node_community->nid}"),
      )
    )), '/');

  return '<a ' . drupal_attributes($attributes) . ' href="' . url($link['href'], array('query' => $link['query'])) . '">' . check_plain($title) . '</a>';
}

/**
 * Custom form to filter community members.
 */
function totem_user_form_user_filter($form, &$form_state) {

  global $user;

  $node = $form_state['build_info']['args'][0];
  $filters = drupal_get_query_parameters();

  // Get specific manager uids associated to this community.
  $gid = _totem_user_get_managers($node);
  $gid = array_merge($gid, _totem_user_get_users_by_role('moderator'));

  if (in_array($user->uid, $gid) || user_access('administer users')) {

    $options = array(
      'member' => 'Members',
      'manager' => 'Managers',
      'pending' => 'Pending Members',
      'blocked' => 'Blocked Members',
    );

    $default_value = (!empty($filters['status']) ? $filters['status'] : 'member');

    $form['#submit'] = array('totem_user_form_user_filter_submit');

    $form['status'] = array(
      '#type' => 'select',
      '#title' => t('Currently showing'),
      '#options' => $options,
      '#default_value' => $default_value,
      '#attributes' => array(
        'onchange' => "document.getElementById('totem-user-form-user-filter').submit();",
      ),
    );

    // Need the button for the submit to work.
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Search'),
      '#attributes' => array(
        'class' => array('element-invisible'),
      ),
    );

    return $form;
  }

  return NULL;
}
/**
 * TODO.
 */
function totem_user_form_user_filter_submit($form, $form_state) {

  $args['status'] = $form_state['values']['status'];

  $form_state['redirect'] = array(drupal_get_path_alias(), array('query' => $args));
  drupal_redirect_form($form_state);
}
