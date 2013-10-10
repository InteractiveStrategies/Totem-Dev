<?php
/**
 * @file
 * search-result--totem-search.tpl.php
 */
?>

<tr class="<?php print $classes; ?>"<?php print $attributes; ?>>
  <td>
    <?php
    if (!empty($result['community'])):
      print render($result['community']);
    endif;
    ?>
  </td>
  <td>
    <?php print render($title_prefix); ?>
    <h3<?php print $title_attributes; ?>><?php print $result['link']; ?></h3>
    <?php print render($title_suffix); ?>
  </td>
  <td>
    <?php print $result['user']; ?>
  </td>
  <td>
    <?php print $result['date']; ?>
  </td>
</tr>

<?php
// TODO: Alter individual results per type.
switch ($result['type']):
  case 'user':
    break;

  case 'community':
    break;

  case 'media':
  case 'media_collection':
  case 'topic':
  case 'event':
  default:
    break;
endswitch;
