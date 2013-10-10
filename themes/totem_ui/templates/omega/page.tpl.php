<?php
/**
 * @file
 * page.tpl.php
 */
?>

<?php if ($messages): ?>
  <div id="messages"><?php print $messages; ?></div>
<?php endif; ?>

<div<?php print $attributes; ?>>
  <?php if (isset($page['header'])) : ?>
    <?php print render($page['header']); ?>
  <?php endif; ?>

  <?php if (isset($page['content'])) : ?>
    <?php print render($page['content']); ?>
  <?php endif; ?>

  <?php if (isset($page['footer'])) : ?>
    <?php print render($page['footer']); ?>
  <?php endif; ?>
</div>
