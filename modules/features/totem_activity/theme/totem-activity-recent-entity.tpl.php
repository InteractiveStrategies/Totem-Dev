<?php
/**
 * @file
 * totem-activity-recent-entity.tpl.php
 */
?>

<div class="submitted clearfix">

  <?php if ($entity_type == 'node'): ?>

    <?php print render($title_prefix); ?>
    <?php print render($title_suffix); ?>
    <div class="user-picture">
      <?php print $user_picture; ?>
    </div>
    <?php if ($title): ?>
      <p class="message">
        <?php print $name; ?> added the
        <?php print $content_type; ?>
        <span <?php print $title_attributes; ?>><a href="<?php print $node_url; ?>" <?php print drupal_attributes($node_url_attributes); ?>><?php print $title; ?></a></span><?php
        if (!$community_context): ?> to the <?php print $community_names; ?><?php endif; ?>.
      </p>
      <p class="ago"><?php print $date_ago; ?></p>
    <?php endif; ?>

  <?php elseif ($entity_type == 'comment'): ?>

    <div class="user-picture">
      <?php print $picture; ?>
    </div>
    <p class="message">
      <?php print $author; ?>
      <a href="<?php print $comment_url; ?>">commented</a> on the
      <?php print $content_type; ?>
      <a href="<?php print $node_url; ?>" <?php print drupal_attributes($node_url_attributes); ?>><?php print $node->title; ?></a><?php
      if (!$community_context): ?> in the <?php print $community_names; ?><?php endif; ?>.
    </p>
    <p class="ago"><?php print $date_ago; ?></p>

  <?php endif; ?>

</div>
