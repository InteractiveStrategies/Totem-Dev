<?php
/**
 * @file
 * search-results--totem-search.tpl.php
 */
?>

<div class="clearfix"></div>

<?php if ($search_results): ?>

  <table class="pager-group">
    <thead>
      <tr>
        <th><?php print t('Community'); ?></th>
        <th>Title</th>
        <th>Author</th>
        <th>Date</th>
      </tr>
    </thead>
    <tbody class="pager-data">
      <?php print $search_results; ?>
    </tbody>
    <tfoot>
      <tr>
        <td colspan="4">
          <?php print $pager; ?>
        </td>
      </tr>
    </tfoot>
  </table>

<?php else : ?>

<div class="no-results-container">
  <p class="no-results"><?php print t('Your search yielded no results');?></p>
  <?php print search_help('search#noresults', drupal_help_arg()); ?>
</div>

<?php endif;
