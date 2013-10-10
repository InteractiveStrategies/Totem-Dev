/**
 * Attaches the Drupal behavior.
 */
(function($) {

  Drupal.behaviors.totem_search = {
    // Attach behaviors to Drupal
    attach : function(context, settings) {

      // Focus/blur form input values, defaulting to label.
      var focusBlurContainers = ['#totem-search-form-search-type'];
      $.each(focusBlurContainers, function(ix, selector) {
        Drupal.behaviors.totem_common.applyTxtFocusBlur.init($(selector));
      });

      // Print search result count.
      Drupal.behaviors.totem_search.printResultCount(context, settings);

    },
    printResultCount : function(context, settings) {

      var count_results = $('#search-form .meta-count-results, #totem-search-form-search-type .meta-count-results', context);
      if (count_results.length && settings.totem_search) {
        count_results.html(settings.totem_search.count_results);
      }

    }
  };

})(jQuery);
