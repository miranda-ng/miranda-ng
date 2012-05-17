Gender plugin for Miranda IM (http://miranda-im.org)
Homepage: http://thief.miranda.im
Version: 0.0.2.1 (c) 2006-2011 Thief
Idea by nile, icons by a0x

This plugin shows gender icon in contact list. Nothing more ;)

Changes:

v0.0.2.1
+ 64bit build
+ internal unicode support
- removed old PLUGININFO support

v0.0.2.0
+ added support for extraicons service plugin

v0.0.1.9
* gender key is now written to "UserInfo" module instead of protocol (the key doesn't get deleted on info update) (ticket #32)
+ option to draw an icon when no gender info found (ticket #33)
* look up for gender key in "UserInfo" module and then in protocol
* other small fixes

v0.0.1.8
- some changes to icons handling, now Miranda 0.7 is required
+ added possibility to disable contact list icon
* fixed service function

v0.0.1.7
- version bump (to allow update from 0.0.1.6 previously released as beta)

v0.0.1.6
+ metacontacts support (thanks sje for clearing things out)
+ added a service for retrieving gender icon for a contact

v0.0.1.5
* changed icons - now using cute icons by a0x (thanks!)

v0.0.1.4
+ added uuid (FB1C17E0-77FC-45A7-9C8B-E2BEF4F56B28) and MIID_GENDER interface for Miranda 0.8.x compatibility
* make menu subitem checked when gender key is found

v0.0.1.3
! addons release
* hide menu items for chatrooms and non-IM protocols (like weather, etc)
* icons changes in icolib reflect menuitems icons
+ option to disable menu items

v0.0.1.2
+ plugin now checks "UserInfo" module if gender key was not found in protocol module (FR by BraVo123)
- MinGW build

v0.0.1.1
+ menuitems to set gender (FR by Shaggoth)
+ langpack and readme
Happy New Year! :)

v0.0.1.0
- initial release

Plugin is released under GPL licence.
http://www.gnu.org/copyleft/gpl.html