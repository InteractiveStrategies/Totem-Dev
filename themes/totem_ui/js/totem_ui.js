(function ($) {

  // Default theme's responsive menu; not included in behaviors, as we only
  // want to attach this once per request, regardless of modal calls, etc.
  // (which will re-attach behaviors).
  $('body.totem-ui .brand .site-name a').click(function(evt) {
    var $menu = $('body.totem-ui #region-header-menu');
    if ($menu.length) {
      evt.preventDefault();

      var $self = $(this);
      var $body = $('body.totem-ui');

      $body.toggleClass('hide-menu');
      $self.toggleClass('collapsed');
      $self.toggleClass('expanded');
    }
  });

  Drupal.behaviors.totem_ui = {
    attach : function(context, settings) {

    },
    resizeMainNav : function(context, settings, args) {

      if (args.reset) {
        var mm_items = $('#section-header .block.nav ul.menu > li', context);
        mm_items.removeAttr('style');
      }
      else {
        var mm_items = $('#section-header .block.nav ul.menu > li > a', context);
        if (mm_items.length) {

          var mm_width_total = $('#section-header .block.nav', context).width();
          var mm_width_available = mm_width_total;

          // Get px width - first item.
          var mm_item_width_pixels = Math.floor(mm_width_available / mm_items.length);
          var mm_item_width_percent = ((mm_item_width_pixels * 100) / mm_width_available);

          mm_items.parents('li').css({
            'width' : mm_item_width_pixels + 'px'
          });
        }
      }

    },
    alterResponsiveState : function(context, settings, args) {

      var w = $(window, context);

      // Anonymous function to check that current viewport width
      // is in media query range, and if so run a callback function.
      var isStateActive = function () {
        var width_viewport = w.width();
        var active = (width_viewport >= args.widthMin && width_viewport <= args.widthMax);

        if (args.callback) {
          args.callback(active);
        }
      };

      // Determine if state active on load.
      isStateActive();
      // Determine if state active on resize.
      w.resize(isStateActive);

    }
  };
})(jQuery);
