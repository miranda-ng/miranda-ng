Authorization State plugin for Miranda IM (http://miranda-im.org)
Homepage: http://thief.miranda.im
Version: 0.0.2.0 (c) 2006-2009 Thief
Idea by snwbrdr, icons by Faith Healer, Angeli-Ka

This small plugin intended to show an icon in front of contacts which haven't gived us an authorization.
Another icon is displayed if there is Grant Authorization item in contact's menu.
Options to disable/enable icons for particular contact are available in contact's menu.

I would like to say thanks to:
snwbrdr for initial idea
Faith Healer for cute icons
Kildor for several creative ideas
mirandaim.ru forum members for valuable feedback and suggestions.
eblis and baloo for WhenWasIt and SecureIM plugins (I found many useful hints on implementation in sources of these plugins)

Changes:

v0.0.2.0
+ added support for extraicons service plugin

v0.0.1.6
! fixed wrong auth-grant icon in some cases
- minor adjustments to menu item logic

v0.0.1.5
* icons by Angeli-Ka
+ auth-grant icon is shown if contact wasn't added to server-side contact list

v0.0.1.4
! MinGW build
- small corrections and fixes

v0.0.1.3
+ added Miranda 0.8 support (PluginInfoEx and Interfaces)
+ optional ability to move contacts without permited authorization to a separate group (like icq official) - WORK IN PROGRESS
* fixed crash on exit (thx Joe)
* small bugfixes

v0.0.1.2
! fix: when changing icon slot clist was getting multiply icons
! fix: dropdown items in options weren't visible in some cases

v0.0.1.1
* fixes to translation

v0.0.1.0
- initial FL release

v0.0.0.x
- test versions released on mirandaim.ru forum only.

Plugin is released under GPL licence.
http://www.gnu.org/copyleft/gpl.html