<?php
/**
 * @file
 * totem_rating.form.inc
 */

/**
 * Implements hook_form_alter().
 */
function totem_rating_form_totem_common_form_settings_alter(&$form, &$form_state) {

  $instance_base = field_info_instance('node', 'field_rating', 'community');

  if (empty($instance_base)) {
    drupal_set_message(t('"field_rating" no longer exists in the database. Please !feature.', array(
      '!feature' => l(t('re-install the Totem Rating module'), 'admin/structure/features'),
    )), 'error');

    return;
  }


  $settings = variable_get('totem_rating_settings');
  $settings_points_total = 0;
  if (!empty($settings)) {
    foreach ($settings as $type => $meta) {
      $settings_points_total += $meta['points'];
    }
  }

  $form['#submit'][] = 'totem_rating_form_totem_common_form_settings_submit';

  $form['totem_rating_config'] = array(
    '#type' => 'fieldset',
    '#title' => t("Rating Settings"),
    '#description' => t('<p>Current point pool: @points</p>', array(
      '@points' => $settings_points_total,
    )),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#weight' => 100,
  );

  $rating_options
    = $rating_defaults
    = $rating_points_options
    = array();

  $types = _totem_common_types_info();
  $types['comment'] = (object) array(
    'name' => 'Comment',
  );
  unset($types['page']);
  unset($types['orphan']);
  unset($types['recent']);
  // TODO: 2013-07-17, natemow: field_rating applied to user bundle is
  // currently spiking PHP memory somehow on /user page.
  unset($types['user']);

  for ($i = 0; $i <= 10; $i++) {
    $rating_points_options[$i] = $i;
  }

  foreach ($types as $bundle => $meta) {

    $select_default = '';
    if (!empty($settings[$bundle]) && is_numeric($settings[$bundle]['points'])) {
      $select_default = $settings[$bundle]['points'];
    }

    $select = array(
      '#type' => 'select',
      '#name' => "totem_rating_settings_points[{$bundle}]",
      '#multiple' => FALSE,
      '#required' => FALSE,
      '#options' => $rating_points_options,
      '#required' => TRUE,
      '#value' => $select_default,
    );

    $select = form_process_select($select);
    $select = drupal_render($select);

    $weight = 0;
    if (!empty($settings_points_total)) {
      $weight = round((($select_default * 100) / $settings_points_total), 4);
    }

    $rating_options[$bundle] = array(
      'name' => $meta->name,
      'points' => $select,
      'weight' => $weight . '%',
    );

    if (!empty($settings[$bundle])) {
      $rating_defaults[$bundle] = $bundle;
    }
  }

  $form['totem_rating_config']['totem_rating_settings'] = array(
    '#type' => 'tableselect',
    '#header' => array(
      'name' => t('Enable rating for these types:'),
      'points' => t('Points'),
      'weight' => t('Relative weight'),
    ),
    '#options' => $rating_options,
    '#default_value' => $rating_defaults,
  );

}
/**
 * Extra submit handler for system settings form.
 */
function totem_rating_form_totem_common_form_settings_submit($form, &$form_state) {

  $field_rating = field_info_field('field_rating');
  $instance_base = field_info_instance('node', 'field_rating', 'community');

  $settings = array();
  foreach ($form_state['values']['totem_rating_settings'] as $bundle => $checked) {

    // Set entity_type per bundle.
    $entity_type = 'node';
    switch ($bundle) {
      case 'user':
        $entity_type = 'user';
        break;

      case 'comment':
        $entity_type = 'comment';
        break;
    }

    if (!empty($checked)) {

      // Enable rating for this bundle.
      $points = $form_state['input']['totem_rating_settings_points'][$bundle];
      $settings[$bundle] = array(
        'points' => (!empty($points) ? $points : 0),
      );

      if ($bundle !== 'comment') {
        if (@!in_array($bundle, $field_rating['bundles'][$entity_type])) {
          $instance = $instance_base;
          $instance['entity_type'] = $entity_type;
          $instance['bundle'] = $bundle;
          field_create_instance($instance);
        }
        if ($entity_type == 'node') {
          if (!empty($form_state['values']['totem_rating_settings']['comment'])) {
            if (@!in_array("comment_{$entity_type}_{$bundle}", $field_rating['bundles']['comment'])) {
              // Add field_rating to this bundle's comments.
              $instance = $instance_base;
              $instance['entity_type'] = 'comment';
              $instance['bundle'] = "comment_{$entity_type}_{$bundle}";
              field_create_instance($instance);
            }
          }
          else {
            // Remove field_rating from this bundle's comments.
            $instance = field_read_instance('comment', 'field_rating', "comment_{$entity_type}_{$bundle}");
            if (!empty($instance)) {
              field_delete_instance($instance);
            }
          }
        }
      }

    }
    else {

      // Remove field_rating from this bundle.
      $instance = field_read_instance($entity_type, 'field_rating', $bundle);
      if (!empty($instance)) {
        if ($bundle !== 'community') {
          field_delete_instance($instance);
        }
      }
      // Remove field_rating from this bundle's comments.
      if ($entity_type == 'node') {
        $instance = field_read_instance('comment', 'field_rating', "comment_{$entity_type}_{$bundle}");
        if (!empty($instance)) {
          field_delete_instance($instance);
        }
      }

    }
  }

  variable_set('totem_rating_settings', $settings);

}
