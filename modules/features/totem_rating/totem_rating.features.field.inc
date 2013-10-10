<?php
/**
 * @file
 * totem_rating.features.field.inc
 */

/**
 * Implements hook_field_default_fields().
 */
function totem_rating_field_default_fields() {
  $fields = array();

  // Exported field: 'node-community-field_rating'.
  $fields['node-community-field_rating'] = array(
    'field_config' => array(
      'active' => '1',
      'cardinality' => '1',
      'deleted' => '0',
      'entity_types' => array(),
      'field_name' => 'field_rating',
      'foreign keys' => array(),
      'indexes' => array(),
      'locked' => '0',
      'module' => 'fivestar',
      'settings' => array(
        'axis' => 'vote',
      ),
      'translatable' => '0',
      'type' => 'fivestar',
    ),
    'field_instance' => array(
      'bundle' => 'community',
      'default_value' => NULL,
      'deleted' => '0',
      'description' => '',
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'module' => 'fivestar',
          'points' => 15,
          'settings' => array(
            'expose' => TRUE,
            'style' => 'average',
            'text' => 'average',
            'widget' => array(
              'fivestar_widget' => NULL,
            ),
          ),
          'type' => 'fivestar_formatter_default',
          'weight' => 16,
        ),
        'teaser' => array(
          'label' => 'hidden',
          'points' => 0,
          'settings' => array(),
          'type' => 'hidden',
          'weight' => 2,
        ),
      ),
      'entity_type' => 'node',
      'exclude_cv' => 0,
      'field_name' => 'field_rating',
      'label' => 'Rating',
      'required' => 0,
      'settings' => array(
        'allow_clear' => 1,
        'exclude_cv' => FALSE,
        'stars' => '5',
        'target' => 'none',
        'user_register_form' => FALSE,
      ),
      'widget' => array(
        'active' => 1,
        'module' => 'fivestar',
        'points' => '31',
        'settings' => array(),
        'type' => 'exposed',
        'weight' => 32,
      ),
    ),
  );

  // Translatables
  // Included for use with string extractors like potx.
  t('Rating');

  return $fields;
}
