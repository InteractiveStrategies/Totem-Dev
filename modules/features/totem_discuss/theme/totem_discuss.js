/**
 * Attaches the Drupal behavior.
 */
(function($) {

  Drupal.behaviors.totem_discuss = {
    // Attach behaviors to Drupal
    attach : function(context, settings) {
      Drupal.behaviors.totem_discuss.bindCommentFormCancelLinks(context, settings);
      Drupal.behaviors.totem_discuss.updateCommentDeltaClasses(context, settings);
      Drupal.behaviors.totem_discuss.hideCommentMessages(context, settings);
      Drupal.behaviors.totem_discuss.ajaxCommentingScrollIntoView(context, settings);
      Drupal.behaviors.totem_discuss.commentButtonPosting(context, settings);
    },

    bindCommentFormCancelLinks: function(context, settings) {
      $('form a.comment-cancel-action').bind('click', function() {
        var $link = $(this);
        var args = $link.attr('href').substr(1).split('/');
         // edit, reply, delete.
        var action = args[3];

        var $formWrapper = $link.parents('#inline-comment-action-form').eq(0);

        if (action == 'edit') {
          $formWrapper.remove();

          var cid = args[1];
          $('#comment-' + cid).fadeIn();
        }
        else {
          // Pretty slide-up then hide.
          $formWrapper.slideUp(400, function() {
            var $formWrapper = $(this);
            $formWrapper.remove();
          });
        }

        return false;
      });
    },

    // We're adding first and last classes to threaded comment display via
    // preprocessing. Make sure they stay up to date upon AJAX commenting
    // actions.
    updateCommentDeltaClasses: function(context, settings) {
      // Comments deleted by AJAX are (currently) using jQuery slideUp to hide them,
      // which doesn't happen fast enough to exclude them from selection using :visible.
      // So _totem_discuss_ajax_comment_delete_cb() immediately attaches a class to denote deletion.
      var $comments = $('#comments-inner .comment').not('.comment-ajax-deleted');
      if ($comments.length) {
        var $first = $comments.eq(0);
        if (!$first.hasClass('first')) {
          $('#comments-inner .comment.first').removeClass('first');
          $first.addClass('first');
        }

        var $last = $comments.eq(-1);
        if (!$last.hasClass('last')) {
          $('#comments-inner .comment.last').removeClass('last');
          $last.addClass('last');
        }
      }
    },

    hideCommentMessages: function(context, settings) {
      // Set auto comment messages hide according to Drupal variable.
      // @see totem_totem_common_preprocess_page()
      // @see settings.php
      if (Drupal.settings.variables.totem_common.autofade_messages) {
        // This is a custom wrapper added around theme('status_messages')
        // calls in AJAX comment callbacks.
        // @see totem_discuss.form.inc
        var $messages = $('#ajax-comment-messages');
        if ($messages.length) {
          $messages.delay(3000).slideUp("slow", function() {
            // Remove element from DOM after hiding, because any additional
            // AJAX comment callbacks may insert element with this same ID.
            $(this).remove();
          });
        }
      }
    },

    // For AJAX-loaded comment stuff, make sure new elements are in view.
    ajaxCommentingScrollIntoView: function(context, settings) {

      var $elemFirst = null;
      var $elemLast = null;

      var $e = ('#inline-comment-action-form', context);
      if (!$e.length || settings.ajax_comment_reply) {
        $e = $('#ajax-comment-messages', context);

        if ($e.length) {
          $elemLast = $e;
          $elemFirst = $elemLast.prev('#comment-' + settings.ajaxed_comment_cid);
        }
      }
      else {
        $elemFirst = $e;
      }

      if ($elemFirst) {
        totem_common.util.scrollIntoView($elemFirst, $elemLast);
      }
    },

    commentButtonPosting: function(context, settings) {
      // Drupal AJAX system binds to mousedown and prevents click event,
      // so we also bind to mousedown.
      $('form.comment-form input.form-submit.ajax-processed', context).bind('mousedown', function () {
        var $btn = $(this);
        $btn.data('buttonText', $btn.attr('value'));
        $btn.attr('value', "Posting...");
      });

      $('form.comment-form input.form-submit').filter(function() {
        return !!$(this).data('buttonText');
      }).each(function() {
        var $btn = $(this);
        $btn.attr('value', $btn.data('buttonText'));
        $btn.data('buttonText', null);
      });
    }
  };

})(jQuery);
