
-- OVERVIEW --

The Totem User module manages all aspects of community member accounts.  It
handles the critical task of permissions management, allowing for controlled
access to communities as well as individual items of content within them.  It
also manages the additional capabilities granted to community managers.

-- FEATURES --

* Modal login, registration, and reset-password forms.

* Any member can start a new community.  This member thus becomes the owner,
  with the highest level of permission control over the community. (?)

* The community owner sets one of three possible levels of access to it:
  - Open: all site users (not just members of this community) can see all
    content, but only members can post content.  Anyone can join without
    approval required.
  - Restricted: all site users can see content, but must be approved to join.
  - Closed: only members can view the community and its content, and must be
    approved to join.
  Note that all communities can appear even to logged-out site users in search
  results. (?)

* Community owners can grant Manager status to members of their community.
  Managers are granted many of the same permissions over fellow members and
  community content as owners have.  (See details below.)

* The community owner and managers can choose to block members from the
  community.  They may request to rejoin, but will only regain full membership
  access if the owner or or a manager later unblocks them.

* Members can invite friends to join their communities, via either email
  addresses or posts to Facebook. Note: the fboauth module is packaged with 
  Totem, but disabled by default. In order to use the Facebook integrations on
  your site, you will need to set up a stub Facebook app, then set the App ID 
  and App Secret variables. Further reading:
  - https://developers.facebook.com/apps
  - https://www.google.com/search?q=create+a+facebook+app+id
  - http://example.com/admin/config/people/fboauth
  - http://drupal.org/node/1950294


Following is a comparison of community owner, manager, and member permissions.

  Owner only:

  - grant and revoke Manager status to members

  Owner and Managers:

  - block and unblock members
  - approve members to join (if the community is restricted or closed)
  - remove members from the community

  - edit content posted by other members
  - remove other members' content from the community (but not delete it
    permanently)

  Members only:

    - permanently delete one's own community content
