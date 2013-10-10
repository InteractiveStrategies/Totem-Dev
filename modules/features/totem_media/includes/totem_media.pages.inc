<?php
/**
 * @file
 * totem_media.pages.inc
 */

/**
 * Menu callback for node/%node/media-view/%node.
 *
 * @param $node - the media node to be viewed.
 */
function totem_media_page_lightbox_view($community_node, $node) {
  ctools_include('ajax');
  ctools_include('modal');

  drupal_set_title($node->title);

  // There's no function to generate a render array for status messages,
  // so we have to render the node first in order to capture any messages
  // from its render process here.
  $node_output = drupal_render(node_view($node));
  $output = theme('status_messages') . $node_output;

  // The 'grouping' query parameter specifies prev/next paging within modal.
  // @see totem_media_url_outbound_alter()

  // Unfortunately we're re-setting the modal's title every time, but there's
  // no way around that when using the AJAX command Drupal.CTools.Modal.modal_display().
  if (isset($_GET['grouping'])) {
    if (is_numeric($_GET['grouping'])) {
      // We've received a media_collection nid.
      $modal_title = node_load($_GET['grouping'])->title;
    }
    else {
      // We're viewing the Media tab on a community and want to page through the loose items.
      $modal_title = t("Media");
    }
  }

  // Instead of calling ctools_modal_render() which calls print ajax_render() directly,
  // we return an AJAX commands array so that the menu item can use ajax_deliver().
  return array(
    '#type' => 'ajax',
    '#commands' => array(
      ctools_modal_command_display($modal_title, $output),
    ),
  );
}
