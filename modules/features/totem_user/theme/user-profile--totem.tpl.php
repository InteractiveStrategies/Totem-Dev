<?php
/**
 * @file
 * user-profile--totem.tpl.php
 */
?>

<?php if ($view_mode == 'full'): ?>

<?php elseif ($view_mode == 'menu_local_task'): ?>

  <?php print render($user_profile['menu_local_task']); ?>

<?php else: ?>

  <div<?php print $attributes; ?>>
    <?php print $user_profile['images']['user_thumb']; ?>
    <?php print render($title_prefix); ?>
    <h3><?php print render($user_profile['name']); ?></h3>
    <?php print render($title_suffix); ?>

    <div class="clearfix"></div>
  </div>

<?php endif;
