<?php
/**
 * @file
 * template.php
 */

/**
 * Implements theme_html_head_alter().
 */
function totem_ui_html_head_alter(&$vars) {

  // Update meta generator tag content.
  $vars['system_meta_generator']['#attributes']['content'] .= ', ' . drupal_ucfirst(variable_get('install_profile')) . ' distribution';

  // Update the corresponding X-Generator response header.
  $vars['system_meta_generator']['#attached']['drupal_add_http_header'][0] = array('X-Generator', $vars['system_meta_generator']['#attributes']['content']);

}

// Theme form input elements.
/**
 * Implements theme_password().
 */
function totem_ui_password(&$vars) {
  $element = $vars['element'];
  $element['#attributes']['type'] = 'password';
  element_set_attributes($element, array('id', 'name', 'size', 'maxlength'));
  _form_set_class($element, array('form-text'));

  unset($element['#attributes']['size']);

  return '<input' . drupal_attributes($element['#attributes']) . ' />';
}
/**
 * Implements theme_select().
 */
function totem_ui_select(&$vars) {
  $element = $vars['element'];
  element_set_attributes($element, array('id', 'name', 'size'));
  _form_set_class($element, array('form-select'));

  unset($element['#attributes']['size']);

  return '<select' . drupal_attributes($element['#attributes']) . '>' . form_select_options($element) . '</select>';
}
/**
 * Implements theme_textarea().
 */
function totem_ui_textarea(&$vars) {
  $element = $vars['element'];
  element_set_attributes($element, array('id', 'name', 'cols', 'rows'));
  _form_set_class($element, array('form-textarea'));

  unset($element['#attributes']['cols']);
  unset($element['#attributes']['rows']);

  $wrapper_attributes = array(
    'class' => array('form-textarea-wrapper'),
  );

  // Add resizable behavior.
  if (!empty($element['#resizable'])) {
    drupal_add_library('system', 'drupal.textarea');
    $wrapper_attributes['class'][] = 'resizable';
  }

  $output = '<div' . drupal_attributes($wrapper_attributes) . '>';
  $output .= '<textarea' . drupal_attributes($element['#attributes']) . '>' . check_plain($element['#value']) . '</textarea>';
  $output .= '</div>';
  return $output;
}
/**
 * Implements theme_textfield().
 */
function totem_ui_textfield(&$vars) {
  $element = $vars['element'];
  $element['#attributes']['type'] = 'text';
  element_set_attributes($element, array('id', 'name', 'value', 'size', 'maxlength'));
  _form_set_class($element, array('form-text'));

  unset($element['#attributes']['size']);

  $extra = '';
  if ($element['#autocomplete_path'] && drupal_valid_path($element['#autocomplete_path'])) {
    drupal_add_library('system', 'drupal.autocomplete');
    $element['#attributes']['class'][] = 'form-autocomplete';

    $attributes = array();
    $attributes['type'] = 'hidden';
    $attributes['id'] = $element['#attributes']['id'] . '-autocomplete';
    $attributes['value'] = url($element['#autocomplete_path'], array('absolute' => TRUE));
    $attributes['disabled'] = 'disabled';
    $attributes['class'][] = 'autocomplete';
    $extra = '<input' . drupal_attributes($attributes) . ' />';
  }

  $output = '<input' . drupal_attributes($element['#attributes']) . ' />';

  return $output . $extra;
}

/**
 * Implements theme_field().
 */
function totem_ui_field(&$vars) {
  $output = '';

  foreach ($vars['items'] as $delta => $item) {
    $output .= drupal_render($item);
  }

  return $output;

  /*
  // Render the label, if it's not hidden.
  if (!$vars['label_hidden']) {
    $output .= '<span class="field-label"' . $vars['title_attributes'] . '>' . $vars['label'] . ':&nbsp;</span>';
  }

  // Render the items.
  $output .= '<span class="field-items"' . $vars['content_attributes'] . '>';
  foreach ($vars['items'] as $delta => $item) {
    $classes = 'field-item ' . ($delta % 2 ? 'odd' : 'even');
    $output .= '<span class="' . $classes . '"' . $vars['item_attributes'][$delta] . '>' . drupal_render($item) . '</span>';
  }
  $output .= '</span>';

  // Render the top-level DIV.
  $output = '<span class="' . $vars['classes'] . '"' . $vars['attributes'] . '>' . $output . '</span>';

  return $output;
  */
}
/**
 * Implements theme_link().
 */
function totem_ui_link(&$vars) {

  $href = url($vars['path'], $vars['options']);

  if (empty($vars['options']['attributes']['class'])) {
    $vars['options']['attributes']['class'] = array();
  }
  elseif (is_string($vars['options']['attributes']['class'])) {
    $vars['options']['attributes']['class'] = explode(' ', $vars['options']['attributes']['class']);
  }

  // Apply modal attributes as needed.
  _totem_common_modal_link_attributes_ensure($vars['path'], $vars['options']['attributes']);

  return '<a href="' . check_plain($href) . '"' . drupal_attributes($vars['options']['attributes']) . '>' . ($vars['options']['html'] ? $vars['text'] : check_plain($vars['text'])) . '</a>';
}
/**
 * Implements theme_menu_link().
 */
function totem_ui_menu_link(&$vars) {

  global $user;

  $element = &$vars['element'];

  // TODO: Hack for core bug in hook_menu::$item['options'].
  // See #comment-5517948.
  // @see http://drupal.org/node/1043906
  $query = db_select('menu_links', 'l')->fields('l', array('options'))->condition('l.router_path', $element['#href'])->execute();
  foreach ($query as $result) {
    $options = unserialize($result->options);
    if (!empty($options)) {
      $element['#localized_options'] = array_merge($element['#localized_options'], $options);
    }
  }

  // TODO: Hack for core bug with active-trail classes (plus maybe a
  // fix-for-a-fix per TODO #1).
  // @see http://drupal.org/node/1578832
  $path = ltrim(request_uri(), '/');
  if ($path == $element['#href']) {
    if (!in_array('active', $element['#attributes']['class'])) {
      $element['#attributes']['class'][] = 'active';
    }
  }

  // Allow HTML in links.
  $element['#localized_options']['html'] = TRUE;

  // Add unique class to each link wrapper.
  $element['#attributes']['class'][] = 'menu-' . $element['#original_link']['mlid'];

  $sub_menu = '';
  if ($element['#below']) {
    $sub_menu = drupal_render($element['#below']);
  }


  // Adjust logged-in menu items.
  if ($element['#original_link']['menu_name'] == 'menu-community-menu') {
    $path = drupal_get_path_alias();
    $add_active = FALSE;

    // Active Home item when  viewing own profile.
    if ($element['#original_link']['link_path'] == 'user') {
      $add_active = (stristr($path, "members/{$user->uid}") || stristr($path, "user/{$user->uid}"));
    }
    // Active Communities item when  viewing /community nodes.
    if ($element['#original_link']['link_path'] == 'communities') {
      $add_active = stristr(url($path), 'community');
    }
    // Active Members item when viewing other user profiles.
    if ($element['#original_link']['link_path'] == 'members') {
      if ($user->uid !== arg(1)) {
        $add_active = $add_active = (stristr($path, "members/") || stristr($path, "user/"));
      }
    }
    // TODO: Active Resources item when viewing node.
    if ($element['#original_link']['link_path'] == 'resources') {
    }

    if ($add_active) {
      if (!in_array('active-trail', $element['#attributes']['class'])) {
        $element['#attributes']['class'][] = 'active-trail';
      }
      if (!in_array('active', $element['#attributes']['class'])) {
        $element['#attributes']['class'][] = 'active';
      }
    }
  }


  $output = l($element['#title'], $element['#href'], $element['#localized_options']);
  return '<li' . drupal_attributes($element['#attributes']) . '>' . $output . $sub_menu . "</li>\n";
}
/**
 * Implements theme_menu_local_task().
 */
function totem_ui_menu_local_task(&$vars) {
  $link = $vars['element']['#link'];
  $link_text = $link['title'];

  $link['localized_options']['attributes']['class'] = array('corners', 'top', drupal_html_class($link['title']));
  $link['localized_options']['html'] = TRUE;

  if (!empty($vars['element']['#active'])) {
    // Add text to indicate active tab for non-visual users.
    $active = '<span class="element-invisible">' . t('(active tab)') . '</span>';

    // If the link does not contain HTML already, check_plain() it now.
    // After we set 'html'=TRUE the link will not be sanitized by l().
    if (empty($link['localized_options']['html'])) {
      $link['title'] = check_plain($link['title']);
    }
    $link_text = t('!local-task-title!active', array('!local-task-title' => $link['title'], '!active' => $active));
  }

  return '<li' . (!empty($vars['element']['#active']) ? ' class="active"' : '') . '>' . l($link_text, $link['href'], $link['localized_options']) . "</li>\n";
}
/**
 * Implements theme_comment_post_forbidden().
 */
function totem_ui_comment_post_forbidden(&$vars) {
  $node = $vars['node'];
  global $user;

  // Since this is expensive to compute, we cache it so that a page with many
  // comments only has to query the database once for all the links.
  $authenticated_post_comments = &drupal_static(__FUNCTION__, NULL);

  if (!$user->uid) {
    if (!isset($authenticated_post_comments)) {
      // We only output a link if we are certain that users will get permission
      // to post comments by logging in.
      $comment_roles = user_roles(TRUE, 'post comments');
      $authenticated_post_comments = isset($comment_roles[DRUPAL_AUTHENTICATED_RID]);
    }

    if ($authenticated_post_comments) {
      // We cannot use drupal_get_destination() because these links
      // sometimes appear on /node and taxonomy listing pages.
      if (variable_get('comment_form_location_' . $node->type, COMMENT_FORM_BELOW) == COMMENT_FORM_SEPARATE_PAGE) {
        $destination = array('destination' => "comment/reply/$node->nid#comment-form");
      }
      else {
        $destination = array('destination' => "node/$node->nid#comment-form");
      }

      if (variable_get('user_register', USER_REGISTER_VISITORS_ADMINISTRATIVE_APPROVAL)) {
        // Users can register themselves.
        return t('!login or !register to post comments', array('!login' => l(t('Log in'), 'user/login', array('query' => $destination)), '!register' => l(t('register'), 'user/login', array('query' => $destination)) ));
      }
      else {
        // Only admins can add new users, no public registration.
        return t('!login to post comments', array('!login' => l(t('Log in'), 'user/login', array('query' => $destination)) ));
      }
    }
  }
}
/**
 * Implements theme_image().
 */
function totem_ui_image(&$vars) {
  $attributes = $vars['attributes'];
  $attributes['src'] = file_create_url($vars['path']);

  // exclude 'width' and 'height' attrs to ease responsive images.
  // NOTE: this allows for these to be passed through via
  // $vars['attributes'], but we expect calls to come mainly from
  // theme_image_style() which does not do so, and leave the possibility in
  // place for now, just in case.
  foreach (array('alt', 'title') as $key) {
    if (isset($vars[$key])) {
      $attributes[$key] = $vars[$key];
    }
  }

  return '<img' . drupal_attributes($attributes) . ' />';
}
/**
 * Implements theme_fboauth_action__connect().
 */
function totem_ui_fboauth_action__connect(&$vars) {
  // Band-aid to work around fboauth_block_view's expectation of default login
  // path; since our path is user/modal/login, alter the redirect logic.
  // @see fboauth_block_view(), totem_user_form_user_login_alter()
  if (!empty($_GET['destination'])) {
    $vars['properties'] = fboauth_action_link_properties('connect', $_GET['destination']);
  }
  elseif (in_array(drupal_get_path_alias(), array('user/modal/login', 'user/modal/register'))) {
    $vars['properties'] = fboauth_action_link_properties('connect', 'user');
  }

  return theme_fboauth_action__connect($vars);
}

// Most template_preprocess functions should be placed in the appropriate
// Features module; any preprocessors defined here are used *exclusively*
// for overriding Omega parent theme output.
/**
 * Implements hook_preprocess_html().
 */
function totem_ui_preprocess_html(&$vars) {

  $vars['attributes_array']['class'][] = drupal_html_class(variable_get('theme_default'));
  $vars['attributes_array']['class'][] = 'hide-menu';

}
/**
 * Implements hook_preprocess_page().
 */
function totem_ui_preprocess_page(&$vars) {

  // If no sidebar blocks assigned, force full width of main content area.
  // (Super-lame that Omega doesn't handle this out of the box...).
  if (empty($vars['page']['content']['content']['sidebar_first'])) {
    $vars['page']['content']['content']['content']['#grid']['columns'] = $vars['page']['content']['content']['#grid_container'];
  }

  // Add theme UI assets.
  $path = drupal_get_path('theme', 'totem_ui');

  drupal_add_js($path . '/js/totem_ui.js', array(
    'scope' => 'footer',
    'group' => JS_THEME,
  ));

}
/**
 * Implements hook_preprocess_node().
 */
function totem_ui_preprocess_node(&$vars) {

  // Remove some classes set by Omega.
  $classes = &$vars['attributes_array']['class'];
  $classes = array_flip($classes);

  unset($classes[drupal_html_class('author-' . $vars['node']->name)]);
  unset($classes['node-not-sticky']);
  unset($classes['node-not-promoted']);
  unset($classes['node-published']);

  $classes = array_flip($classes);

}
