<?php
/**
 * @file
 * node--topic.tpl.php
 */
?>

<div<?php print $attributes; ?>>

  <?php if ($view_mode == 'teaser'): ?>

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

  <?php elseif ($view_mode == 'recent_entity'): ?>

    <?php print theme('totem_activity_recent_entity', $variables); ?>

  <?php else: ?>

    <div<?php print $content_attributes; ?>>
      <?php print render($title_prefix); ?>
      <h2<?php print $title_attributes; ?>><?php print $title ?></h2>
      <?php print render($title_suffix); ?>
      <?php
      // We hide the comments and links now so that we can render them later.
      hide($content['comments']);
      hide($content['links']);
      print render($content);

      print render($sharethis);

      print render($content['comments']);
      ?>
    </div>

  <?php endif; ?>

</div>
