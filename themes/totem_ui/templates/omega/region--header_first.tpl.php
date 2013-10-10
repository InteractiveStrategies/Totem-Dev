<?php
/**
 * @file
 * region--header_first.tpl.php
 */
?>

<div<?php print $attributes; ?>>
  <div<?php print $content_attributes; ?>>
    <?php if ($linked_logo_img || $site_name || $site_slogan): ?>
      <div class="brand clearfix">

        <?php if ($linked_logo_img): ?>
          <?php print $linked_logo_img; ?>
        <?php endif; ?>

        <?php if ($site_name || $site_slogan): ?>
          <?php $class = $site_name_hidden && $site_slogan_hidden ? ' element-invisible' : ''; ?>
          <div class="site-name-slogan<?php print $class; ?>">
            <?php if ($site_name): ?>
              <?php $class = $site_name_hidden ? ' element-invisible' : ''; ?>
              <?php if ($is_front): ?>
                <h1 class="site-name<?php print $class; ?>"><?php print $linked_site_name; ?></h1>
              <?php else: ?>
                <h2 class="site-name<?php print $class; ?>"><?php print $linked_site_name; ?></h2>
              <?php endif; ?>
            <?php endif; ?>
            <?php if ($site_slogan): ?>
              <?php $class = $site_slogan_hidden ? ' element-invisible' : ''; ?>
              <h6 class="site-slogan<?php print $class; ?>"><?php print $site_slogan; ?></h6>
            <?php endif; ?>
          </div>
        <?php endif; ?>

      </div>
    <?php endif; ?>
    <?php print $content; ?>
  </div>
</div>
