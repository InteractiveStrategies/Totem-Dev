<?php
/**
 * @file
 * totem_discuss.theme.inc
 */

/**
 * Implements template_preprocess_page().
 */
function totem_discuss_preprocess_page(&$vars) {

  global $user;

  // Add module UI assets.
  if (!$vars['is_admin']) {
    $path = drupal_get_path('module', 'totem_discuss');

    drupal_add_css($path . '/theme/totem_discuss.css');

    drupal_add_js($path . '/theme/totem_discuss.js', array(
      'scope' => 'footer',
    ));
  }

}
/**
 * Implements template_preprocess_comment_wrapper().
 */
function totem_discuss_preprocess_comment_wrapper(&$vars) {

  // Prep for adding first/last classes to a comment thread.
  // comment_view_multiple() keys this array by cid.
  if (isset($vars['content']['comments']['#sorted']) && $vars['content']['comments']['#sorted'] == TRUE) {
    $cids = array_values(array_filter(array_keys($vars['content']['comments']), 'is_numeric'));
    $vars['content']['comments'][$cids[0]]['#delta_class'][] = 'first';
    $vars['content']['comments'][$cids[count($cids) - 1]]['#delta_class'][] = 'last';
  }

  if (!$vars['content']['comments']) {
    $vars['classes_array'][] = 'no-comments';
  }

}
/**
 * Implements template_preprocess_comment().
 */
function totem_discuss_preprocess_comment(&$vars) {

  $comment = $vars['elements']['#comment'];

  // Overwrite picture var with user's sized thumbnail.
  // @see totem_discuss_comment_view_alter()
  if (!empty($comment->content)) {
    $vars['picture'] = $comment->content['images']['user_thumb'];
  }

  // Expose view_mode to template if defined.
  if (!empty($vars['elements']['#view_mode'])) {
    $vars['view_mode'] = $vars['elements']['#view_mode'];
    $vars['classes_array'][] = drupal_html_class($vars['view_mode']);
  }

  // Add class to support custom "View more" paging.
  $vars['classes_array'][] = 'pager-entity';

  // Flag to indicate to attach 'comment-$cid' ID to the wrapping div of the
  // comment, as the anchor that uses that ID has been removed from #prefix in
  // the render array.
  // @see totem_discuss_comment_view_alter()
  if (isset($vars['elements']['#comment_div_id'])) {
    if ($vars['elements']['#comment_div_id'] == TRUE) {
      $vars['attributes_array']['id'] = 'comment-' . $vars['comment']->cid;
    }
  }

  // Attach 'first' and 'last' classes for theming.
  if (isset($vars['elements']['#delta_class'])) {
    $vars['classes_array'] = array_merge($vars['classes_array'], $vars['elements']['#delta_class']);
  }
}
