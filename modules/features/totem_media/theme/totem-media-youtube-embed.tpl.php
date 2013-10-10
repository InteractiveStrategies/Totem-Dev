<?php
/**
 * @file
 * totem-media-youtube-embed.tpl.php
 */
?>
<?php if (!empty($yt_id)): ?>
<?php // 'rel=0' disables displayed related videos after playback. ?>
<iframe width="640" height="480" src="//www.youtube.com/embed/<?php print $yt_id; ?>?rel=0" frameborder="0" allowfullscreen></iframe>
<?php endif; ?>
