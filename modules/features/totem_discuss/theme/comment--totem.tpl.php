<?php
/**
 * @file
 * comment--totem.tpl.php
 */
?>

<div<?php print $attributes; ?>>
  <?php
  // Hide links and rely on contextual links in $title_prefix.
  hide($content['links']);
  ?>

  <?php if ($view_mode == 'recent_entity'): ?>

    <?php print theme('totem_activity_recent_entity', $variables); ?>

  <?php else: ?>

    <?php print render($title_prefix); ?>
    <?php print render($title_suffix); ?>
    <div class="photo"><?php print $picture; ?></div>
    <div class="user-comment">
      <?php print $author; ?>
      <?php print render($content); ?>
      <div class="submitted"><?php print $created; ?></div>
    </div>

  <?php endif; ?>
</div>
