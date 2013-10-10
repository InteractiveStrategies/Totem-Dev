<?php
/**
 * @file
 * node--media.tpl.php
 */
?>

<div<?php print $attributes; ?>>

  <?php if ($view_mode == 'teaser'): // contains contextual links ?>

    <?php if (variable_get('totem_media_gallery_mode') == TRUE): ?>

      <?php print render($title_prefix); ?>
      <?php if (!empty($content['field_file']) && !empty($content['field_file'][0]['sized_images'])): ?>
        <div class="media-image-thumb">
          <a href="<?php print $node_url; ?>" <?php print drupal_attributes($node_url_attributes); ?>>
            <?php print $content['field_file'][0]['sized_images']['media_thumb']; ?>
          </a>
        </div>
      <?php endif; ?>
      <h3<?php print $title_attributes; ?>><a href="<?php print $node_url; ?>" <?php print drupal_attributes($node_url_attributes); ?>><?php print $title; ?></a></h3>
      <?php print render($title_suffix); ?>

    <?php else: ?>
      <div class="header">
        <div class="user-picture">
          <?php print $user_picture; ?>
        </div>

        <dl>
          <?php print render($title_prefix); ?>
          <?php print $file_icon; ?>
          <dd<?php print $title_attributes; ?>><a href="<?php print $node_url; ?>" <?php print drupal_attributes($node_url_attributes); ?>><?php print $title; ?></a></dd>
          <?php print render($title_suffix); ?>

          <?php if ($display_submitted): ?>
            <dd>by <?php print $name; ?></dd>
            <dd class="date"><?php print $date; ?></dd>
          <?php endif; ?>
        </dl>
      </div>

    <?php endif; ?>


  <?php elseif ($view_mode == 'recent_entity'): ?>

    <?php print theme('totem_activity_recent_entity', $variables); ?>

  <?php
  // A slimmed version of teaser for drag-and-drop sorting.
  // @see totem_media_subform_collection_dnd()
  elseif ($view_mode == 'teaser_sort'):  ?>

    <?php print render($title_prefix); // contains contextual links ?>

    <?php if (variable_get('totem_media_gallery_mode') == TRUE): ?>
      <?php if (!empty($content['field_file']) && !empty($content['field_file'][0]['sized_images'])): ?>
        <div class="media-image-thumb">
          <?php print $content['field_file'][0]['sized_images']['media_thumb']; ?>
        </div>
      <?php endif; ?>
    <?php else: ?>
      <?php print $file_icon; ?>
    <?php endif; ?>

    <h3<?php print $title_attributes; ?>><?php print $title; ?></h3>
    <?php print render($title_suffix); ?>

  <?php else: ?>

    <?php if (variable_get('totem_media_gallery_mode') == TRUE): ?>

      <?php
      // We hide the comments and links now so that we can render them later.
      hide($content['comments']);
      hide($content['links']);
      hide($content['contextual_links']);

      hide($content['field_file']);
      ?>

      <div class="column column-media-view">
        <div class="column-inner">

          <div<?php print $content_attributes; ?>>
            <?php
            // The latter piece of the 'ctools-modal-media-lightbox' class
            // (after 'ctools-modal-') corresponds to key in Drupal.settings.
            // Note manual addition of 'rel' attribute;
            // @see Drupal.CTools.Modal.getSettings(), _totem_common_modal_link_attributes_ensure()
            ?>
            <?php if (isset($prev_url)): ?>
            <a class="media-view-prev totem-media-ctools-reuse-modal ctools-modal-media-lightbox" href="<?php print $prev_url; ?>" rel="ctools-modal-target-node-media-view">Previous</a>
            <?php endif; ?>
            <?php if (isset($next_url)): ?>
            <a class="media-view-next totem-media-ctools-reuse-modal ctools-modal-media-lightbox" href="<?php print $next_url; ?>" rel="ctools-modal-target-node-media-view">Next</a>
            <?php endif; ?>

            <?php
            print render($content);
            ?>

            <div class="media-file">
              <?php
              // This field is limited to hold one file.
              $field_file = $content['field_file'][0];
              if ($field_file['#file']->type == 'image') {
                // Keyed array of image sizes is added in totem_common_node_view().
                // Do NOT call render(), just print the size we want.
                print $field_file['sized_images']['media_large'];
              }
              else {
                // Render field normally.
                // Note: mediaelement.js handles audio and video.
                print render($field_file);
              }
              ?>
            </div>
          </div>

          <div class="bottom-bar">
            <div class="bottom-bar-inner clearfix">
              <?php print render($content['contextual_links']); ?>
              <?php print render($sharethis); ?>
            </div>
          </div>

        </div>
      </div>

      <div class="column column-media-data">
        <div class="column-inner">
          <div class="column-top header">
            <?php print render($title_prefix); ?>
            <h2<?php print $title_attributes; ?>><?php print $title ?></h2>
            <?php print render($title_suffix); ?>

            <div class="submitted clearfix">
              <div class="user-picture">
                <?php print $user_picture; ?>
              </div>
              <dl>
                <dd>Posted by <?php print $name; ?></dd>
                <dd class="date">on <?php print $date; ?></dd>
              </dl>
            </div>

            <?php
            if (!empty($content['links'])):
              print render($content['links']);
            endif;
            ?>

          </div>

          <?php
          print render($content['comments']);
          ?>
        </div>
      </div>

    <?php else: // (non-gallery mode) ?>

      <div<?php print $content_attributes; ?>>
        <?php print render($title_prefix); ?>
        <h2<?php print $title_attributes; ?>><?php print $title ?></h2>
        <?php print render($title_suffix); ?>

        <?php
        // Comments rendered below.
        hide($content['comments']);
        // Links are unneeded; contextual links used instead.
        hide($content['links']);

        hide($content['field_file']);
        // This field is limited to hold one file.
        $field_file = $content['field_file'][0];
        // Keyed array of image sizes is added in totem_common_node_view().
        if ($field_file['#file']->type == 'image') {
          // Do NOT call render(), just print the size we want.
          // Add same wrapper that the general file renderer adds.
          print '<span class="file">' . $field_file['sized_images']['media_large'] . '</span>';
        }
        else {
          // Render field normally.
          // Note: mediaelement.js handles audio and video.
          print render($field_file);
        }

        print render($content);

        print render($sharethis);

        print render($content['comments']);
        ?>

      </div>

    <?php endif; // gallery mode? ?>

  <?php endif; // (view modes) ?>

</div>
