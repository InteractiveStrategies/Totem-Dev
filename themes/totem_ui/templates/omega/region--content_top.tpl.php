<?php
/**
 * @file
 * region--content_top.tpl.php
 */
?>

<?php
hide($page['action_links']);
$show_title = TRUE;
?>


<div<?php print $attributes; ?>>
  <div<?php print $content_attributes; ?>>
    <a id="main-content"></a>

    <?php if ($breadcrumb): ?>
      <div id="breadcrumb"><?php print $breadcrumb; ?></div>
    <?php endif; ?>

    <div class="utility">
      <?php if (!empty($rss_link)): ?>
        <?php print $rss_link; ?>
      <?php endif; ?>
      <?php if ($page['action_links']): ?>
        <ul class="actions">
          <?php print render($page['action_links']); ?>
        </ul>
      <?php endif; ?>
    </div>
    <div class="clearfix"></div>

    <?php print $content; ?>

    <?php
    // Hide title + tabs on user profile pages.
    if (arg(0) == 'user'):
      $show_title = (drupal_get_path_alias() !== 'members/' . arg(1));
    endif;

    if (!empty($page['node'])):
      if ($page['node']->type == 'community'):

        // Print sharethis block to short+long headers.
        print render($sharethis);

        // Print memory album header stuff -- not in wwf? maybe?
        $show_title = FALSE;

        // Determine long or short header view.
        $menu_local_task = arg(2);
        $show_long_header = empty($menu_local_task);

        $node_entity = entity_metadata_wrapper('node', $page['node']);
        // Get renderable node.
        $node = node_view($page['node']);

        if ($show_long_header):
          // Long header view.
          $details_classes = (empty($details) ? ' empty' : '');
    ?>
          <div class="community-image">
          <?php
          if (!empty($node['field_image'])):
            print $node['field_image'][0]['sized_images']['community_image'];
          endif;
          ?>
          </div>
          <?php print render($title_prefix); ?>
          <?php if ($title): ?>
          <?php if ($title_hidden): ?><div class="element-invisible"><?php endif; ?>
          <h1 class="title" id="page-title"><?php print $title; ?></h1>
          <?php if ($title_hidden): ?></div><?php endif; ?>
          <?php endif; ?>
          <?php print render($title_suffix); ?>

          <div class="grid-4 alpha details<?php print $details_classes; ?>">
          </div>
          <div class="grid-6 omega">
          </div>
          <div class="clearfix"></div>

    <?php
        else:
          // Short header view.
    ?>
          <div class="community-image">
          <?php
          if (!empty($node['field_image'])):
            print $node['field_image'][0]['sized_images']['community_image'];
          endif;
          ?>
          </div>
          <?php print render($title_prefix); ?>
          <?php if ($title): ?>
          <?php if ($title_hidden): ?><div class="element-invisible"><?php endif; ?>
          <h1 class="title" id="page-title"><?php print $title; ?></h1>
          <?php if ($title_hidden): ?></div><?php endif; ?>
          <?php endif; ?>
          <?php print render($title_suffix); ?>

    <?php
        endif;
      endif;
    endif;
    ?>

    <?php
    // Non-community page.
    if ($show_title):
    ?>
      <?php print render($title_prefix); ?>
      <?php if ($title): ?>
      <?php if ($title_hidden): ?><div class="element-invisible"><?php endif; ?>
      <h1 class="title" id="page-title"><?php print $title; ?></h1>
      <?php if ($title_hidden): ?></div><?php endif; ?>
      <?php endif; ?>
      <?php print render($title_suffix); ?>
    <?php
    endif;
    ?>

    <?php
    if ($tabs):
      if (!empty($tabs['#primary'])):
    ?>
      <div id="tabs" class="tabs corners top clearfix">
        <?php print render($tabs); ?>
        <?php if (!empty($page['tab_utility'])): ?>
        <div class="utility">
          <ul>
            <?php foreach ($page['tab_utility'] as $key => &$item): ?>
            <li><?php print render($item); ?></li>
            <?php endforeach; ?>
          </ul>
        </div>
        <?php endif; ?>
      </div>
    <?php
      endif;
    endif;
    ?>

  </div>
</div>
