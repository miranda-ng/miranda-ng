*********
Tipper YM
*********

Description
===========
Modified Tipper plugin by Scott Ellis.


New features
************
- skin support with glyph transformation and alpha channel transparency (win 2000+)
- two-level extended tray tooltip replacing default one with many options
- SmileyAdd support (graphic smileys in tooltips)
- menu for copy item/s or avatar to the clipboard (CTRL+C when tooltip is showed)
- new icons (Extra Status, Gender, Country Flags, Fingerprint)
- some new system substitutions
- raw protocol substitutions are also searched in UserInfo module
- correct status message in status bar tooltip
- ICQ XStatus and Jabber mood/activity details in status bar tooltip
- smooth animation when tooltip is showing and hiding
- option for keep original avatar size and make avatar transparent


changelog
*********

--- 2.1.0.5 ---
! new preset item - ICQ logon time

--- 2.1.0.4 ---
! fixed buffer overrun

--- 2.1.0.3 ---
! fixed crash when getting NULL value from db

--- 2.1.0.2 ---
! fixed contact's status message retrieving
! fixed possible crash when getting contact's account

--- 2.1.0.1 ---
+ merged changes from original Tipper
! fixed crash when getting own status message

--- 2.1.0.0 ---
+ merged changes from original Tipper
+ ability to display contact time in tooltips

--- 2.0.4.17 ---
! fixed crash when getting own status message
! memory leaks and x64 portability fixes (patch by Merlin_de) 

--- 2.0.4.16 ---
* code cleanup, x64 portability
! fixed behavior of the Apply button in options

--- 2.0.4.15 ---
+ option for parse Tipper variables first
! fixed tray tooltip detection
! fixed some other minor bugs 

--- 2.0.4.14 ---
! compatibility fixes (required Miranda IM 0.8.9 or newer)
! layout fixes for aid translation

--- 2.0.4.13 ---
+ added url link to get more skins
! no-icon setting wasn't applied to tray tooltip
! variables button now handle focused field (label or value)
! removed translation of db key

--- 2.0.4.12 ---
+ added variables button to new item dialog

--- 2.0.4.11 ---
* couple of changes in tray tooltip options page
* Tipper YM now requires Miranda 0.8
! fixed missing protocols (XFire) in tray tooltip
! translation fixes

--- 2.0.4.10 ---
+ added system substitution %sys:account%
! fixed smileys parsing in title
! fixed memory corruption (thx Protogenes)

--- 2.0.4.9 ---
! fixed 'skin coloring' feature

--- 2.0.4.8 ---
* title is wrapped instead of truncated
* smileys are displayed in title
! some fixes related to getting status message

--- 2.0.4.7 ---
! fixed possible occurrance of glitches in sidebar part of skin
! fixed occasional crash 

--- 2.0.4.6 ---
! fixed converting the font size value from skin file
(I hope this is the last change related to skin definition, please don't beat me :)

--- 2.0.4.5 ---
+ new attribute in [Font] section for specify font name
* improved function for get own status message

--- 2.0.4.4 ---
* improved %sys:msg_count_xxx% substitutions
! fixed saving skin options to db

--- 2.0.4.3 ---
+ new skin element [Other]
+ new options for load fonts, colors and appearance provided by skin
! many fixes for options dialog

--- 2.0.4.2 ---
+ new skin attribute for enable/disable coloring feature
! fixed skin parsing
! fixed %sys:msg_count_xxx% substitutions (thx ASSami)
! fixed reversed spin control in appearance options

--- 2.0.4.1 ---
! fixed crash if metacontact plugin isn't present

--- 2.0.4.0 ---
+ new skin engine with coloring feature
* separator in traytip moved above protocol name
! disabled accounts are no more visible in tray tooltip
! many other fixes :)

--- 2.0.3.4 ---
! fixed crash on tray tooltip (thx Lastwebpage)

--- 2.0.3.3 ---
+ button for add a separator (better representation of separators in item list)
+ translation for convert BYTE to language (ICQ)
+ translations for convert WORD to name of a day and name of a month (e.g. Last seen plugin)
+ option for limit status message to adjusted count of characters
! some fixes and cleanup

--- 2.0.3.2 ---
+ option for replace only isolated smiles in text
+ option for show tooltip after all content (avatar & status message) is ready  
+ option for disable retrieving of status message in all statuses ("CList/StatusMsg" value is used)
! fixed icq xstatus/jabber mood in status bar and tray tooltips
! fixed transparency of avatar border
! disabled accounts are no more visible in tray tooltip

--- 2.0.3.1 ---
+ option for set avatar transparency (via db key 'AvatarOpacity', value: 0 - 100%)
! various fixes (thx all for help)

--- 2.0.3.0 ---
!!! MAKE BACKUP OF YOUR TIPPER SETTING BEFORE USE THIS VERSION !!!

+ reworked aero glass on windows vista
+ option for specify show effect speed
+ indication of locked protocols
+ support for SimpleAway plugin
* tooltip items relocated to new db module Tipper_Items
* removed option for Transparent background
! fixed linebreaks when copy all items from tooltip
! fixed some bugs on Win 98

--- 2.0.2.11 ---
+ support for account system in miranda 0.8
+ option for disable offline protocols in tray tooltip
! fix for copy function (CTRL + C) with clist nicer
! workaround for unicode text in xstatus

--- 2.0.2.10 ---
+ new set of predefined items
! some fixes for predefined items (labels are translatable now)

--- 2.0.2.9 ---
+ added several predefined items
! fixed some memory leaks and crash

--- 2.0.2.8 ---
+ support for jabber moods and activities
+ new translation to get number of days to next birthday
+ better caching for background image
* some design changes in options dialog
! fixed crash on status bar tooltip

--- 2.0.2.7 ---
! fixed crash with old variables plugin

--- 2.0.2.6 ---
+ added fade out effect when window is closing
* disabled setting for Aero Glass blur (need some work)
! fixed parsing of text with smileys (again)
! fixed protocol smileys for MetaContacts
! fixed memory leak

--- 2.0.2.5 ---
+ added support of window blur at Vista Aero (patch by Protogenes)
+ added option for use status bar protocols setting (on/off & order) in tray tooltip
+ added option for use protocol smileys instead of tipper smileys
! fixed clist event in tray tooltip
! fixed parsing of text with smileys

--- 2.0.2.4 ---
! fixed double icons drawing in extended traytip
! fixed drawing of transparent icons 

--- 2.0.2.3 ---
+ added clist event info (message from xyz etc.) to extended tray tooltip
+ added listening to and number of unread emails to status bar and tray tooltip
+ added options for favourite contacts: hide offline and append protocol name
* larger height is applied only to lines with smiley, not to all lines in text with smileys
* max height is ignored for tray tooltip
! fixed wrong size of pluginInfoEx structure (thx eblis)
! fixed getting of own status message from NAS
! fixed width of tray tooltip if 'number of contacts' was enabled

--- 2.0.2.2 ---
! fixed drawing of smileys without trasparency
! fixed crash on tray tooltip if protocols are connecting

--- 2.0.2.1 ---
! fixed calculation of tooltip width 

--- 2.0.2.0 ---
+ implemented two-level extended tray tooltip replacing default one with many options 
+ avatar padding divided into outer and inner padding
+ added option for value indent
* increased animation speed
! fixed drawing of edges - now they are transformed like centre area (according to transform setting)
! fixed drawing of avatars with transparent background

--- 2.0.1.1 ---
! fixed large memory leak

--- 2.0.1.0 ---
+ improved skin engine - added support for pictures with alpha channel transparency (win 2000+)
+ new option for copy all tooltip items with labels
+ fade effect when showing tooltip
+ option for title indent
* gender info is searched also in UserInfo module (for gender icon)
! small fix for alternate text

--- 2.0.0.0 ---
+ 'system' substitutions to get number of (sent or received) messages for contact 
  (%sys:msg_count_out%, %sys:msg_count_in%, %sys:msg_count_all%)
+ first FL version

--- 1.5.0.5 ---
+ menu for copy item/s or avatar to the clipboard (CTRL+C when tooltip is showed)
* value for 'custom' substitutions is also searched in UserInfo modul (if 'contact protocol module' is checked)
* changes in status message retrieving
! item was displayed if alternate substitution fails
! some options page fixes

--- 1.5.0.4 ---
! fixed reading of unicode settings from db

--- 1.5.0.3 ---
+ buttons for export/import skin setting
! fixed reading of numbers from db
! fixed system substitution 'sys:uid'

--- 1.5.0.2 ---
* substitution can be (dis)allowed for more protocols (separated by commas) - e.g. %x^w,y,z%
* substitution can be used in alternate text - e.g. %raw:x/y|sys:proto%
* if failed 'raw protocol substitution' (%raw:/x%) is tried UserInfo module (%raw:UserInfo/x%)

--- 1.5.0.1 ---
! fixed crash with variables

--- 1.5.0.0 ---
+ simple skin support with glyph transformation (like in clist modern)
+ updater support
! fixed tooltip hiding when clist hides from timeout 
! option "show if list not focused" affects only contact tooltips in clist
! some memory bug fixes
* minor options dialog changes

--- 1.4.8.8 ---
+ added SmileyAdd support :-)
+ added option for use background image
! fixed display of fingerprint icon
! many other fixes and optimalizations
* code cleanup

--- 1.4.8.7 ---
! fixed height calculation of status bar tooltip 
! fixed display of extra icons if animation is enabled

--- 1.4.8.6 ---
! window height is also calculated from extra icons height
! fixed nasty bug that caused crash on status bar tooltip
+ option for draw avatar border
+ added translation for extra icons

--- 1.4.8.5 ---
+ show log on info in the status bar tooltip
! fixed crash if there is no contact in db (status bar tooltip)
! show correct status message if global status was changed (status bar tooltip)

--- 1.4.8.4 ---
! fixed another stupid bug (getting status message)

--- 1.4.8.3 ---
! fixed crash with contact of the unloaded protocol ('unknown contact')

--- 1.4.8.2 ---
* SVN-sync 214

--- 1.4.5.2 ---
* SVN-sync 205 (utf8 patch from george)
+ option to change extra icons order
+ variables parsing in status bar tooltip
! fixed XStatus icon for metacontacts

--- 1.4.3.1 ---
+ new settings (options for disable new icons and functions)
+ option for keep original avatar size
! support for Country Flags v.0.1.1.0

--- 0.4.0.3 ---
* SVN-sync 177 (added timezone translation)
+ new icons (Gender, Country Flags)

--- 0.4.0.2 ---
* SVN-sync 172 (multiline text tips (FYR request))
+ show more icons (XStatus icon, Fingerprint icon)

--- 0.4.0.1 ---
* SVN-sync 126-157 (new plugin API)
+ retrieve XStatus details when automatic retrieve is disabled
* code cleanup

Symbols used in changelog: + = new / * = changed / ! = fixed


Author
======
original plugin by sje (thx!)
mod by yaho -> YM 

email:	yaho@atlas.cz
ICQ:	356344824
www:	www.miranda-easy.net