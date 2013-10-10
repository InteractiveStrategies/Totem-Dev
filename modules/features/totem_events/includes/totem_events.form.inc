<?php
/**
 * @file
 * totem_events.form.inc
 */

/**
 * TODO.
 */
function totem_events_filter_form($form, $form_state) {

  $node_community = $form_state['build_info']['args'][0]['node_community'];
  $filters = drupal_get_query_parameters();

  // Get the Months and years for any community event.
  $items = module_invoke('totem_events', 'node_community_event', array(
    'node' => $node_community,
    // Force hook to skip params alter.
    // Force node table to be joined.
    'view_mode' => 'full',
    'property_order_by' => array(
      array('column' => 'status', 'value' => 1),
    ),
    'field_order_by' => array(
      array('field' => 'field_dates', 'column' => 'value', 'direction' => 'ASC'),
    ),
  ));

  $results = $items['nodes']['query']->results;

  // Set distinct date options.
  $options = array();
  $default_value = NULL;
  foreach ($results as $event) {
    $start = date('F-Y', strtotime($event['field_dates']['#items'][0]['value']));
    if (!array_key_exists($start, $options)) {
      $options[$start] = date('F Y', strtotime($event['field_dates']['#items'][0]['value']));
    }
    if (!empty($filters) && $start == $filters['date']) {
      $default_value = $filters['date'];
    }
  }

  if (empty($options)) {
    return NULL;
  }

  $form['#submit'] = array('totem_events_filter_form_submit');

  $form['date'] = array(
    '#title' => t('Date filter'),
    '#required' => FALSE,
    '#empty_option' => t('- None -'),
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => $default_value,
    '#attributes' => array(
      'onchange' => "document.getElementById('totem-events-filter-form').submit();",
    ),
  );

  // Need the button for the submit to work.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
    '#attributes' => array(
      'style' => 'display:none;',
    ),
  );

  return $form;
}
/**
 * TODO.
 */
function totem_events_filter_form_submit($form, $form_state) {

  $args['date'] = $form_state['values']['date'];

  $form_state['redirect'] = array(drupal_get_path_alias(), array('query' => $args));
  drupal_redirect_form($form_state);
}
