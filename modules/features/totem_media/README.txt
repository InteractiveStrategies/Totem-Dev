
-- OVERVIEW --

The Totem Media module allows members to add files - referred to as "media" by
default - to their communities and group these files into "media collections".
Each uploaded file is stored within its own node, and thus can have its own
comment thread.  If your communities are geared towards showcasing rich media,
the module's optional "gallery mode" is ideal for sharing photos and videos.

-- FEATURES --

* Enable Gallery Mode to display each media node in its full view in a modal
  overlay, rather than on individual pages.
  - Users can page through the media nodes in a single collection, or all
    "loose" media nodes (those not in a collection), without leaving the modal.
  - The AJAX commenting implementation in Totem Discuss allows for all
    commenting actions to happen fully within the modal view.

* Streamlined, user-friendly upload of multiple files at once via Plupload
  (http://www.plupload.com/).

* Users can group their files into collections.  For photos and videos,
  collections are analogous to familiar album functionality.
  - Drag-and-drop files to easily reorder them within a collection.
  - Remove a file from a collection via a contextual link.

* Video and audio files are played using the highly cross-browser-compatible
  MediaElement.js library (http://mediaelementjs.com/).

* Share links to individual media nodes, even if gallery mode is enabled and
  they do not live on their own pages.
