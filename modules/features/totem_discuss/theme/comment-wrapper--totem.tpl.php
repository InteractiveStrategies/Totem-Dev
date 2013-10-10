<?php
/**
 * @file
 * comment-wrapper--totem.tpl.php
 */
?>

<div id="comments" class="<?php print $classes; ?>"<?php print $attributes; ?>>
  <span id="comments-header">Comments</span>
  <div id="comments-inner">
    <?php print render($content['comments']); ?>
  </div>
  <div class="comment-form">
    <?php if ($content['comment_form']): ?>
      <?php print render($content['comment_form']); ?>
    <?php endif; ?>
  </div>
</div>
