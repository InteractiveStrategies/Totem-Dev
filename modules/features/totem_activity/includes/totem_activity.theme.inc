<?php
/**
 * @file
 * totem_activity.theme.inc
 */

/**
 * Implements MODULE_preprocess_node().
 */
function totem_activity_preprocess_node(&$vars) {

  // "Recent Activity" view mode stuff.
  if ($vars['view_mode'] == 'recent_entity') {
    // Outermost wrapper is printed before view mode routes the output
    // to theme('totem_activity_recent_entity'), so add a common class.
    $vars['classes_array'][] = 'recent-entity';

    // Remove modal processing and force absolute URL path.
    $uri = entity_uri('node', $vars['node']);
    $uri['options']['absolute'] = TRUE;
    $vars['node_url']  = url($uri['path'], $uri['options']);

    $attributes = array(
      'title' => $vars['title'],
    );
    $vars['node_url_attributes'] = $attributes;
  }

}

/**
 * Template preprocessor for theme('totem_activity_recent_entity') calls.
 */
function template_preprocess_totem_activity_recent_entity(&$vars) {

  $vars['entity_type'] = (is_object($vars['comment']) ? 'comment' : 'node');
  $node = $vars['node'];
  $entity = $node;
  if ($vars['entity_type'] == 'comment') {
    $entity = $vars['comment'];
  }

  // Comment $vars won't have node_url_attributes; make sure we add it here.
  if (empty($vars['node_url_attributes'])) {
    $node_url_options = array(
      'entity_type' => 'node',
      'entity' => $node,
      'absolute' => TRUE,
    );
    $vars['node_url'] = url('node/' . $node->nid, $node_url_options);

    // For the node's url, apply modal attributes as needed.
    $attributes = array(
      'title' => $node->title,
    );
    _totem_common_modal_link_attributes_ensure($vars['node_url'], $attributes);
    $vars['node_url_attributes'] = $attributes;

    // Add anchor link to new comment_url var.
    $node_url_options['fragment'] = 'comment-' . $entity->cid;
    $vars['comment_url'] = url('node/' . $node->nid, $node_url_options);
  }

  // Set an "ago" date string.
  $vars['date_ago'] = t("@interval ago", array('@interval' => format_interval(time() - $entity->created, 2)));

  // Specify what type of new content this is.
  $types = _totem_common_node_types();
  $vars['content_type'] = drupal_strtolower(t($types[$node->type]->name));

  // Are we displaying in the context of a single community?
  $community_context = _totem_common_get_community_context_node();
  $vars['community_context'] = ($community_context !== FALSE);

  // If not viewing Recent Activity in a single community's context,
  // list all communities (accessible to this user) the node is in.
  if (!$vars['community_context']) {
    // Specify which community(ies) the new content is in.
    $nid_community = _totem_common_get_field_entityreference_values('node', $node, 'field_community');

    $community_links = array();

    foreach ($nid_community as $nc) {
      $node_community = node_load($nc);
      if (node_access('view', $node_community)) {
        $community_links[] = l($node_community->title, 'node/' . $node_community->nid);
      }
    }

    // Prepare a nicely-worded string of the community links.
    if (count($community_links) == 1) {
      $vars['community_names'] = $community_links[0] . " " . t("community");
    }
    else {
      $last = count($community_links) - 1;
      $last_link = $community_links[$last];
      unset($community_links[$last]);
      $vars['community_names'] = implode(", ", $community_links) . " and " . $last_link . " " . t("communities");
    }
  }


}

/**
 * Implements MODULE_preprocess_totem_common_node_community_box().
 */
function totem_activity_preprocess_totem_common_node_community_box(&$vars) {
  if ($vars['hook'] == 'user_community_recent') {
    // Denote "subscribed activity" filter for use in theming (mainly the
    // empty-results case). This duplicates the check in
    // totem_activity_totem_common_EFQ_params_alter() - but is simpler than
    // trying to pass the result of that check through the EFQ, stub block,
    // and block render array.
    $args = drupal_get_query_parameters();
    $vars['filtered_subscribed'] = (!empty($args['recent']) && $args['recent'] == 'subscribed');
  }
}