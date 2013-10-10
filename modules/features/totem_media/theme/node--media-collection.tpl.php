<?php
/**
 * @file
 * node--media-collection.tpl.php
 */
?>

<div<?php print $attributes; ?>>

  <?php if ($view_mode == 'teaser'): ?>

    <?php if (variable_get('totem_media_gallery_mode') == TRUE): ?>

      <div class="child-teaser-image">
        <?php if (!empty($content['first_child_node']) && !empty($content['first_child_node']['field_file'][0]['sized_images'])): ?>
          <a href="<?php print $node_url; ?>" <?php print drupal_attributes($node_url_attributes); ?>>
          <?php print $content['first_child_node']['field_file'][0]['sized_images']['media_thumb']; ?>
          </a>
        <?php endif; ?>
      </div>
      <?php if ($title): ?>
        <?php print render($title_prefix); ?>
        <h3<?php print $title_attributes; ?>><a href="<?php print $node_url; ?>" <?php print drupal_attributes($node_url_attributes); ?>><?php print $title; ?></a></h3>
        <?php print render($title_suffix); ?>
      <?php endif; ?>
      <span class="child-count"><?php print format_plural($num_items, "1 item", "@count items"); ?></span>

    <?php else: ?>
      <div class="header">
        <div class="user-picture">
          <?php print $user_picture; ?>
        </div>

        <dl>
          <?php print render($title_prefix); ?>
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

  <?php else: ?>

    <div class="header">
      <?php if ($title): ?>
        <?php print render($title_prefix); ?>
        <h2<?php print $title_attributes; ?>><?php print $title; ?></h2>
        <?php print render($title_suffix); ?>
      <?php endif; ?>

      <?php print render($sharethis); ?>

      <?php if ($display_submitted): ?>
        <div class="submitted clearfix">
          <div>by <?php print $name; ?><br /><?php print $date; ?></div>
        </div>
      <?php endif; ?>
    </div>

    <div<?php print $content_attributes; ?>>
      <?php
      hide($content['comments']);
      hide($content['links']);
      hide($content['media_nodes']);

      print render($content);
      ?>

      <?php if (!empty($content['media_nodes'])): ?>
        <div class="child-media-nodes clearfix">
          <?php print render($content['media_nodes']); ?>
        </div>
      <?php endif; ?>

      <?php print render($content['comments']); ?>
    </div>

  <?php endif; ?>

</div>
