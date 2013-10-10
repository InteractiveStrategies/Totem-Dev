
-- OVERVIEW --

The Totem Discuss module implements communication between community members.
This includes discussion threads, comment functionality on all community
content, automated notification messages, and private messaging between users.

-- FEATURES --

* Creates the Discussion node type - a basic type with a text body and comments
  enabled - to function as discussion threads between community members.

* Includes a custom implementation of AJAX commenting, built with Drupal core's
  comment module and AJAX framework
  (http://api.drupal.org/api/drupal/includes!ajax.inc/group/ajax/7).
  AJAX functionality includes:
  - posting new comments
  - posting replies within a comment thread
  - inline editing of existing comments
  - deletion of comments

* Adds a contextual link to all community content nodes and comments for
  members to easily send a message to the node or comment author, via the
  Privatemsg module (http://drupal.org/project/privatemsg).

* Automatically sends notifications (as private messages) to community managers
  and members when certain actions relevant to them occur on the site.
  Examples:
  - Managers are notified when users request to join a restricted or closed
    community.
  - Users are notified when such a request is approved.
  - Comment authors are notified when someone replies to their comment.

* Adds a contextual link to all community content nodes for members to report
  inappropriate content to the community managers.
