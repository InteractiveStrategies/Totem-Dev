/**
 * Attaches the Drupal behavior.
 */
(function($) {

  // Attach behaviors to Drupal
  Drupal.behaviors.totem_user = {
    attach : function(context, settings) {

      // Show/hide own profile fields on Overview screen.
      Drupal.behaviors.totem_user.toggleOwnProfile(context, settings);

      // Kind of a usability fix here...if links to logged-out user modals are
      // embedded in content, we need to forcefully hide so logged-in user
      // does not click and arrive at logged-out modals. Example usage of such
      // embedded links might be the 403 node body.
      //
      // 2013-02-07, natemow - note new back-end handling in
      // totem_user_form_account as well.
      $('body.logged-in a[href*="user/modal/login"], body.logged-in a[href*="user/modal/password"], body.logged-in a[href*="user/modal/register"]', context).hide();
    },
    toggleOwnProfile : function(context, settings) {

      var link = $('body.page-user a.toggle-profile-fields', context);
      if (link.length) {
        link.click(function(evt) {
          evt.preventDefault();

          var fields = $('body.page-user .user-profile.self', context);
          if (!fields.is(':visible')) {
            fields.slideDown(400, function() {
              link.text('Hide profile');
            });
            totem_common.util.scrollToTop(false, fields);
          }
          else {
            fields.fadeOut(400, function() {
              link.text('View profile');
            });
          }
        });
      }

    }
  };

})(jQuery);
