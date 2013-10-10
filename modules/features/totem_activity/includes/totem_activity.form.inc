<?php
/**
 * @file
 * totem_activity.form.inc
 */

/**
 * Filter form for Recent Activity on user profile.
 */
function totem_activity_user_recent_activity_filter_form($form, &$form_state) {

  global $user;

  $account = $form_state['build_info']['args'][0]['account'];
  $nid_community = _totem_common_get_field_entityreference_values('user', $account, 'field_community');

  $options = array();

  // Only allow subscribed filter if own profile.
  if ($account->uid == $user->uid) {
    $options['subscribed'] = 'All Subscribed Activity';
  }
  foreach ($nid_community as $nc) {
    $tn = node_load($nc);
    if (is_object($tn)) {
      $options[$nc] = $tn->title;
    }
  }

  $qp = drupal_get_query_parameters();
  $default_value = 0;
  if (isset($qp['recent'])) {
    if (in_array((int) $qp['recent'], $nid_community)) {
      $default_value = (int) $qp['recent'];
    }
    elseif ($qp['recent'] == 'subscribed') {
      $default_value = 'subscribed';
    }
  }

  $form['ra_filter'] = array(
    '#type' => 'select',
    '#title' => '',
    '#empty_option' => t('All @community Activity', array('@community' => t('Community'))),
    '#options' => $options,
    '#default_value' => $default_value,
  );

  // Hidden submit button - needed for Drupal form API functions to work.
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
    '#attributes' => array(
      // Core system.base.css uses this for display: none.
      'class' => array('element-invisible'),
    ),
  );

  return $form;
}
/**
 * Submit handler for totem_activity_user_recent_activity_filter_form().
 */
function totem_activity_user_recent_activity_filter_form_submit($form, &$form_state) {
  $query = drupal_get_query_parameters();
  if (!empty($form_state['values']['ra_filter'])) {
    $query['recent'] = $form_state['values']['ra_filter'];
  }
  elseif (isset($query['recent'])) {
    unset($query['recent']);
  }

  $form_state['redirect'] = array(
    current_path(),
    array(
      'query' => $query,
    ),
  );
}
