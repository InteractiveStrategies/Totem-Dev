/**
 * Attaches the Drupal behavior.
 */
(function($) {

  Drupal.behaviors.totem_media = {
    // Attach behaviors to Drupal
    attach : function(context, settings) {

      //var links = $('.node .header .node-title a[href*="media"]');

      Drupal.behaviors.totem_media.pluploadEvents(context, settings);
      Drupal.behaviors.totem_media.addToCollection(context, settings);
      Drupal.behaviors.totem_media.sortMediaCollection(context, settings);
      Drupal.behaviors.totem_media.bindReuseModalLinks(context, settings);

      // Only relevant for variable 'totem_media_gallery_mode' == TRUE.
      Drupal.behaviors.totem_media.bindLightboxArrowKeys(context, settings);
      Drupal.behaviors.totem_media.bindMediaViewScrollableResize(context, settings);
      Drupal.behaviors.totem_media.modalCloseRemoveAudio(context, settings);

      Drupal.behaviors.totem_media.bindMediaAutoView(context, settings);
      Drupal.behaviors.totem_media.modalAjaxHistory(context, settings);
    },
    pluploadEvents: function(context, settings) {

      // See http://www.plupload.com/example_events.php for other plupload events.
      var pluploadStateChangedHandler = function(up) {
        if (up.state == plupload.STARTED) {
          //console.log('start');
        }
        else {
          //console.log('stop');

          // Auto-submit this form once upload is complete.
          var wrapper = $('#' + up.settings.container + '.plupload-element');
          if (wrapper.length) {
            var form = wrapper.parents('form');
            form.submit();
          }
        }
      };

      // Bind plupload elements to custom event handlers.
      $('.plupload-element', context).each(function(ix) {
        var self = $(this);
        var uploader = self.pluploadQueue();
        uploader.bind('StateChanged', pluploadStateChangedHandler);
      });

    },
    addToCollection: function(context, settings) {
      // AJAX load-more paging adds new media teasers, but form is not within
      // context in this case. So don't limit finding form to context.
      var form = $('form.add-to-collection');
      if (form.length) {
        var checkboxes = $('.album-checkbox', context);

        // AJAX load-more paging case: if step 2 already visible,
        // make sure checkboxes are visible.
        if (form.find('.step-2').is(':visible')) {
          checkboxes.show();
        }

        // Toggle step 1 to 2.
        form.find('.step-1 a.btn', context).click(function(evt) {
          evt.preventDefault();
          form.find('.step-1').hide().removeClass('visible');
          form.find('.step-2').show().addClass('visible');
          checkboxes.show();
          form.parents('.block-embed-type-button-add.action-media-collection').addClass('expanded');
        });

        // Populate hidden input in modal form onclick of
        // Submit based on content area checkbox values.
        form.find('.step-2 input.btn[type="submit"]').click(function(evt) {
          var checked = checkboxes.find('input[name="media"]:checked');

          var values = checked.map(function() {
            return $(this).val();
          }).get().join();

          var hidden = form.find('input[name="field_media"]');

          hidden.val(values);
        });

        // Toggle step 2 to 1.
        form.find('.step-2 a.cancel').click(function(evt) {
          evt.preventDefault();
          form.find('.step-1').show().addClass('visible');
          form.find('.step-2').hide().removeClass('visible');
          checkboxes.hide();
          form.parents('.block-embed-type-button-add.action-media-collection').removeClass('expanded');
        });

      }

    },
    sortMediaCollection: function(context, settings) {
      if ($('.sort-wrapper').length) {

        // http://jqueryui.com/demos/sortable/#display-grid
        // see: totem_media_subform_collection_dnd()
        var $list = $('ul#media-collection-sort');

        $list.sortable({
          containment: 'parent', // parent element must have a width/height - be careful with floated items inside..
          cursor: 'move'
        });
        $list.disableSelection(); // apparently this is undocumented

        // On submit of node form, stick reordered deltas into hidden field
        var $btn = $list.parents('form.node-form').find('input[type="submit"]');
        $btn.click(function(event) {
          var deltas = [];
          var $items = $list.children();
          $items.each(function(ix, $item) {
            deltas.push($item.id.split('-')[1]);
          });

          var $hidden = $btn.parents('form.node-form').find('input[name="field_media_new_deltas"]');
          $hidden.val(deltas);
        });

      }
    },
    // see: ctools/js/modal.js - Drupal.behaviors.ZZCToolsModal.attach()
    bindReuseModalLinks: function(context, settings) {
      $('a.totem-media-ctools-reuse-modal', context).once('totem-media-ctools-reuse-modal', function () {
        var $this = $(this);
        // implement a very minimalized version of Drupal.CTools.Modal.show()
        $this.bind('click', function () {

          // Set defaults and look for overrides to any custom settings.
          // see: Drupal.CTools.Modal.show()
          var reuseDefaults = {
            reuse: {
              keepTitle: false
            }
          };

          if (!Drupal.CTools.Modal.currentSettings.reuse) {
            $.extend(true, Drupal.CTools.Modal.currentSettings, reuseDefaults);
          }

          // this looks for settings via a class starting with 'ctools-modal-'
          var settingsKey = Drupal.CTools.Modal.getSettings(this);
          if (settingsKey && typeof settingsKey == 'string' && Drupal.settings[settingsKey]) {
            // since modal is already open, start with the current settings
            $.extend(true, Drupal.CTools.Modal.currentSettings, Drupal.settings[settingsKey]);
          }

          // Insert the 'loading' title and graphic, but don't call .modalContent()
          // adapted from last few lines of .show()
          // (NOTE: Drupal.CTools.Modal.modal is somehow not referring to the correct object in the DOM.
          // In fact it appears to be referring to a created object never actually inserted in the DOM...
          // so don't use it as context for finding modal title.)

          if (Drupal.CTools.Modal.currentSettings.reuse.keepTitle == false) {
            $('span#modal-title').html(Drupal.CTools.Modal.currentSettings.loadingText);
          }
          $('#modalContent .modal-content').html(Drupal.theme(Drupal.CTools.Modal.currentSettings.throbberTheme));

          // Remove old class derived from rel and attach new one.
          // @see Drupal.behaviors.totem_common.modalAdjustments()
          $('#modalContent').removeClass().addClass($(this).attr('rel'));

          $(window).trigger('resize');
        });
        // create Drupal.ajax object - lifted directly from .attach()
        var element_settings = {};
        if ($this.attr('href')) {
          element_settings.url = $this.attr('href');
          element_settings.event = 'click';
          element_settings.progress = {type: 'throbber'};
        }
        var base = $this.attr('href');
        Drupal.ajax[base] = new Drupal.ajax(base, this, element_settings);
      })
    },

    // Only relevant for variable 'totem_media_gallery_mode' == TRUE.
    bindLightboxArrowKeys: function(context, settings) {
      var $prev = $('.modal-content .node-media a.media-view-prev').eq(0);
      var $next = $('.modal-content .node-media a.media-view-next').eq(0);
      if ($prev.length || $next.length) {
        // 'context' seems to always be Document, even on AJAX calls to open/change modal content...
        $(context).bind('keyup', function(event) {
          // check existence again, as modal may have been closed and that does NOT unbind the behaviors.
          if (event.which == 37) { // left arrow key
            var $prev = $('.modal-content .node-media a.media-view-prev').eq(0);
            if ($prev.length) {
              $prev.click();
            }
          }
          else if (event.which == 39) { // right arrow key
            var $next = $('.modal-content .node-media a.media-view-next').eq(0);
            if ($next.length) {
              $next.click();
            }
          }
        });
      }
    },
    bindMediaAutoView: function(context, settings) {
      // Force this function to only happen once per page load. This avoids
      // many unwanted attempts to auto-view during any additional calls to
      // Drupal.attachBehaviors() after the initial one:
      // - during loading of the modal
      // - paging through media nodes in modal (AJAX requests)
      // - loading media node edit or delete form in modal (AJAX request)
      // This is especially important if the history state is updated while
      // paging within the modal, since the URL will match the auto-view URL.
      // @see Drupal.behaviors.totem_media.modalAjaxHistory()
      if (!this.autoviewChecked) {
        // Remove leading slash from path and "explode".
        var args = window.location.pathname.substr(1).split('/');
        // Look for "community/[community-name]/media/view/[media-nid]" or
        // "community/[community-name]/media/[collection-nid]/view/[media-nid]".
        if ((args.length == 5 || args.length == 6) && args[2] == 'media') {
          var nid = parseInt(args.pop());
          if (args.pop() == 'view' && nid) {
            // Find link for this node within its teaser and execute click on it
            // to open full-view modal.
            var $a = $('#node-media-' + nid + ' a[href*="/media-view/"]').eq(0);
            if ($a.length) {
              $a.click();
            }
          }
        }

        // Never again! bwahaha.
        this.autoviewChecked = true;
      }
    },
    modalAjaxHistory: function(context, settings) {
      var History = window.History;
      if (!History.enabled) {
        return;
      }

      // When full media node is loaded into modal (by AJAX),
      // Drupal.attachBehaviors() is called. So we don't update URL/history
      // until we know we have a media node loaded into the modal.
      // @see modal.js - Drupal.CTools.Modal.modal_display()
      var $mediaNodeFull = $('.ctools-modal-target-node-media-view .node-full.node-media', context).eq(0);
      if ($mediaNodeFull.length) {
        var href = window.location.pathname;

        var matches = $mediaNodeFull.attr('id').match(/-[0-9]+/);
        var nid = parseInt(matches[0].substr(1));
        var piece = "/view/" + nid;

        if (href.indexOf("/view/") === -1) {
          // Modal first opened by teaser click. Attach "view/[nid]".
          href = href + piece + window.location.search;
          History.pushState({ ajaxed : true }, document.title, href);
        }
        else if (href.indexOf(piece) === -1) {
          // Modal already open.
          // If auto-opened, the URL is already up-to-date; no change needed.
          // If paging through, update to new current node.
          href = href.replace(/\/view\/[0-9]+/, piece) + window.location.search;
          History.replaceState({ ajaxed : true }, document.title, href);
        }
      }

      // Bind modal close link to remove "view/[nid]" from URL.
      $('.modal-header a.close', context).once('close-history', function () {
        $(this).click(function () {
          var href = window.location.pathname.replace(/\/view\/[0-9]+/, "") + window.location.search;
          History.replaceState(null, document.title, href);
        });
      });


      // Allow back button to close modal (i.e. reverse the pushState() above).
      var $w = $(window);
      if (!$w.data('mediaStatechangeBound')) {
        $w.bind('statechange', function() {
          var $close = $('.modal-header a.close');
          if ($close.length) {
            var current = window.location.pathname;
            // If on Media tab, but without 'view' in URL anymore...
            if (current.indexOf("/media") !== -1 && current.indexOf("/view/") === -1) {
              $close.click();
            }
          }

        });
        $w.data('mediaStatechangeBound', true);
      }
    },

    bindMediaViewScrollableResize: function(context, settings) {
      // If set to do so, the CTools modal gets resized upon window resize.
      // In the media full-view in modal, we want to get a scrollbar onto
      // just *part* of the right column, i.e. we can't just use height 100%.
      // So we calculate height of the comment thread, leaving space above
      // for node title/author, and below for comment form.
      /*
      *  @see Drupal.CTools.Modal.show()
      **/
      if (Drupal.CTools.Modal.currentSettings) {
        if (Drupal.CTools.Modal.currentSettings.modalSize.type == 'scale' && $('#modal-content').length > 0) {

          var commentThreadHeight = function () {
            var $modalContent = $('#modal-content').eq(0);
            var total = $modalContent.find('.node-media .column-media-data').eq(0).height();
            // Other elements taking their own vertical space.
            // IMPORTANT: for accurate height, these elements must NOT have
            // any overlapping margins!!!!
            var others = ['.column-top', '#comments-header', '.comment-form'];
            var usedHeight = 0;
            for (var i = 0; i < others.length; i++) {
              // The outerHeight function finds height + padding,
              // + margin when arg is TRUE.
              usedHeight += $modalContent.find(others[i]).eq(0).outerHeight(true);
            }

            // Any wrappers of the scrollable element, within the total-height element,
            // whose margin and padding are taking up vertical space.
            var wrappers = ['.column-media-data .column-inner', '#comments'];
            for (var i = 0; i < wrappers.length; i++) {
              var $elem = $modalContent.find(wrappers[i]).eq(0);
              usedHeight += ($elem.outerHeight(true) - $elem.height());
            }

            var $scrollable = $modalContent.find('#comments-inner').eq(0);
            // Make sure to count any top/bottom padding on the element as used height.
            usedHeight += ($scrollable.outerHeight(true) - $scrollable.height());

            // Finally, apply the new calculated height to the element.
            $scrollable.height(total - usedHeight);
          };

          // Resize upon initial opening of modal.
          commentThreadHeight();

          $(window).bind('resize', commentThreadHeight);
        }
      }
    },
    modalCloseRemoveAudio: function(context, settings) {
      // This is a fix for the fact that mediaelement.js prepends any
      // fallback media elements (Flash, etc) to document.body,
      // for media types other than video. So when user closes modal,
      // audio continues playing. (lolz)
      // @see mediaelement-and-player.js lines 1134-1140.
      var $meAudioWrap = $('#modal-content .mediaelement-audio');
      if ($meAudioWrap.length > 0) {
        $meAudioWrap = $meAudioWrap.eq(0);
        var $close = $('.modal-header a.close').eq(0);
        // Luckily, MEJS saves info of players on page to a global DOM var.
        // We need to match some piece of data from the MEJS stuff within
        // the modal to the fallback element it sticks elsewhere.
        // Caveats:
        // - At this point, MEJS processing has not run, so all MEJS player
        //   markup does not exist yet inside .mediaelement-audio.
        // - When user closes modal, all its markup is lost, so we have to
        //   find something *before* our click handler runs.
        var audioElemClass = $meAudioWrap.find('audio').attr('class');

        // Use the optional eventData arg to pass this value to the handler,
        // where it is accessible at event.data.
        // @see http://api.jquery.com/bind/
        $close.click({'audioElemClass': audioElemClass}, function(event) {
          $.each(window.mejs.players, function(i, obj) {
            // (If the native element works, there's no separate element to remove.)
            if (obj.media.pluginType != 'native' && obj.$media.hasClass(event.data.audioElemClass)) {
              // The <embed> or whatever has a wrapper div with hardcoded ID suffix.
              // @see mediaelement-and-player.js line 1134.
              $('#' + obj.media.id + '_container').remove();
              // Break loop.
              return false;
            }
          });
        });
      }
    }
  };

})(jQuery);
