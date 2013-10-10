<?php
/**
 * @file
 * totem-user-block-user-profile.tpl.php
 */
?>

<div class="userheader">

<?php if ($is_own_profile): ?>

  <?php if (!$user_has_communities && drupal_get_path_alias() == ltrim(url('user/' . $user_profile['#account']->uid), '/')): ?>
    <div class="profile-image">
      <?php print $user_profile['#account']->content['images']['user_thumb']; ?>
    </div>
    <div class="profile-welcome">
      <h1><span>Welcome to </span><?php print variable_get('site_name'); ?></h1>
      <?php
      print l(t('update profile'), 'user/' . $user_profile['#account']->uid . '/edit', array(
        'attributes' => array(
          'class' => array('btn', 'small', 'corners'),
        ),
      ));
      ?>
    </div>
  <?php else: ?>
    <div class="profile-image return">
      <?php print $user_profile['#account']->content['images']['user_thumb']; ?>
    </div>
    <div class="profile-welcome return">
      <p>Welcome back <?php print render($user_profile['field_name_first']); ?></p>
      <?php
      print l(t('update profile'), 'user/' . $user_profile['#account']->uid . '/edit', array(
        'attributes' => array(
          'class' => array('btn', 'small', 'corners'),
        ),
      ));
      ?>
    </div>
  <?php endif; ?>

<?php else: ?>

  <div class="profile-image return">
    <?php print $user_profile['#account']->content['images']['user_thumb']; ?>
  </div>
  <div class="profile-welcome return">
    <p><?php print render($user_profile['#account']->content['name']); ?></p>
  </div>

<?php endif; ?>

  <div class="clearfix"></div>
</div>
