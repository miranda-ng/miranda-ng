New Away System plugin
  by Chervov Dmitry aka Deathdemon

You can always find the latest plugin version here: http://deathdemon.int.ru/projects.php
If you have some questions, you'll find answers on most of them on the Miranda forums: http://forums.miranda-im.org/showthread.php?t=2179

New Away System requires at least Miranda v0.6 to work properly. v0.7 or above is recommended.
If you use an older Miranda core, either update it or use New Away System v0.3.7.8, which is available here:
http://deathdemon.int.ru/projects/archive/NewAwaySysW_build2371.zip (Unicode; Windows XP/2000/NT)
http://deathdemon.int.ru/projects/archive/NewAwaySys_build2371.zip (ANSI; Windows 95/98/ME)

Overview
========

New Away System is a successor to AwaySysMod plugin by XF007 and Tornado.
It allows to set your status messages using dynamic variables and do some interesting tricks, like different message each time someone requests it, or different status message depending on a time of the day, and much more...

If you're confused by all these variables, it's not a problem :) NAS can also be used in a usual way, by entering usual text as a status message. the only thing you really need to remember is that if your message gets parsed incorrectly (and this happens in some rare cases, when you accidentally use some special characters in the message, like # character), try enclosing that part of your message in `quotes` (note, it's `, not ' !)

Main features:
- categorized predefined status messages (a message tree is used to store them)
- autoreply
- per-protocol status messages, and (ICQ only) per-contact status messages
- optional notification through a popup that someone read your status message (works only for ICQ, as it's the only protocol that stores status messages on the client side)
- logging ICQ status message requests to a file (requires LogService plugin)
- optional updating of your status messages on non-ICQ protocols every X seconds
- multiple ICQ accounts support

Known issues:
- currently there is no way to show UINs of contacts that are not on the contact list and requesting a status message - it requires some changes to Variables plugin
- there's still no proper unicode status message support in protocol plugins, so even with unicode NAS version much depends on protocol itself

To use dynamic variables in NAS, you need to install Variables plugin by Unregistered: http://www.pboon.nl/projects.htm (highly recommended, as without Variables NAS will have a limited functionality).
To log ICQ status message requests to a file, you need LogService plugin: http://deathdemon.int.ru/projects.php

some useful variables:
?cinfo(%subject%,display) - nick of the contact who is reading your status message
?cinfo(%subject%,first) - first name of the contact
?cinfo(%subject%,last) - last name of the contact
(for detailed description of other possible values, see the Variables help dialog and readme)

A brief description of the plugin options, just to make things more clear:
Default messages are used by default for each status, unless you set "by default... use the message from last time" in the miranda options -> Status -> Away System -> Main options tab.
Each status can have one and only one default message. that's why it's not possible to 'uncheck' "use this message as default for.." buttons, once they were checked. - just select another message in the tree and set it as default.
You can set any predefined message to be default for certain statuses.
Also you can add/delete/modify/move your messages/categories in the "Predefined messages" category of the message tree.
Most recent status messages are placed in the "Recent messages" category; it's not recommended to put your messages manually there, as they'll be replaced by your recent messages. However this category is editable too, just like "predefined messages" is.

FAQ
===

Q: when I click on the blue question mark button, nothing happens
A: make sure you have the latest build of Variables plugin installed

Q: all variables remain unparsed and other contacts read my status message, for example, as "Gone since %nas_awaysince_time%" instead of "Gone since 11:45"
A: Either a) see the previous answer Or b) you checked the "Disable parsing of status messages by Variables plugin" checkbox in the settings menu (one of the buttons on the bottom of the status message change dialog)

Q: NAS uses a wrong status message when miranda sets idle status automatically after several minutes of inactivity
A: it's a known incompatibility with the built-in autoaway module and it's impossible to solve it in a reasonable way without changes to the miranda core. you can use AdvancedAutoAway plugin if you really need a correct status message with autoaway

Q: NAS uses a wrong status message even when I set it manually
A: You cannot ever trust contact list's status bar tooltips that show your own status message (though in many cases they still show it correctly). The only way to make sure that your status message is correct is to ask your friend to read it.

Q: what about xstatus support?
A: it's planned

Q: why does NAS show ?cinfo variables unparsed ("?cinfo(%subject%,display)", "?cinfo(%subject%,id)" etc.) when the contact is not on the contact list?
A: it requires changes to Variables plugin to allow NAS to override the default behavior of ?cinfo variable. currently there's no any solution of this issue

Q: I can't read or set a status message in unicode
A: there's still no proper support of unicode status messages by protocol plugins. So, currently even the unicode build of NAS is unable to show unicode status messages of your contacts. I'll fix it as soon as protocol plugins allow me to do that.

Q: I updated NAS and an autoreply doesn't contain my status message now. with an old version it appended the status message at the end.
A: check that %extratext% is present at the end of Autoreply format (options ->Status->Away System->Autoreply tab). If it's not there, just add it at the end. for example, the default format is:
Miranda IM autoreply >
%extratext%


Thanks to
=========

- Faith Healer for icons and useful suggestions
- Nullbie for his ideas
- Bio for a nice idea of msvcr70->msvcrt patch
- UnregistereD for Variables plugin
- ...and to many other people who reported bugs and gave me ideas


Contact
=======
The preferable way for questions, bug reports or some suggestions is using Miranda forums:
http://forums.miranda-im.org/showthread.php?t=2179
Otherwise, if you have something extremely important to say, you can contact me on ICQ or e-mail :) ICQ: 310927; e-mail: dchervov@yahoo.com


Changelog
=========

v0.3.8.3 (build 26xx; xxx xx, 2007)
---------
 - fixed some GDI leaks

v0.3.8.2 (build 2628; Oct 13, 2007)
---------
 - fixed a bug with default messages and per-contact notification settings, introduced in the previous build

v0.3.8.1 (build 2626; Oct 13, 2007)
---------
 - added "Not-on-list contacts" setting to the NAS' contact list in the options
 - added back an option to log to a file only when popup notification for a contact is enabled
 - fix for %nas_timepassed% to return correct value
 - minor interface improvements
 - ContactSettings api change
 - some minor fixes

v0.3.8.0 (build 2601; Oct 4, 2007)
---------
 - added Miranda 0.8 support
 - significantly improved autoreply, added more options and fixed bugs
 - added support for ICQ Plus online status messages
 - removed %nas_message% variable. use %extratext% instead
 - fixed xstatus request notifications
 - moved logging features to a separate plugin - LogService
 - added support for ContactSettings plugin
 - MS_VARS_GETICON service is used now to get an icon for Variables help buttons
 - some more minor fixes and improvements

v0.3.7.8 (build 2371; Jul 2, 2006)
---------
 - fixed a bug with disappeared notification icons under the Contacts tab
 - other minor fixes

v0.3.7.7 (build 2368; Jul 2, 2006)
---------
 - support for locked protocols
 - implemented unicode logging correctly (thanks to Unregistered)
 - fix for MS_AWAYMSG_GETSTATUSMSG to return parsed status message
 - made xstatus notification popups show current xstatus icon
 - added possibility to change per-contact autoreply setting from the contact menu
 - fix for wrong extra icons in the contact list of the status message change window
 - other minor fixes and improvements

v0.3.7.6 (build 2348; May 12, 2006)
---------
 - changed NAS behavior so that it accepts status message changes through PS_SETAWAYMSG service now
 - removed some obsolete implementations of NAS services (update your MyDetails/StartupStatus/AdvancedAutoAway plugins if they stopped working with this release of NAS)
 - fixed a minor bug - empty status message after setting it through a service, in some specific cases

v0.3.7.5 (build 2346; May 8, 2006)
---------
 - fixed crash on startup on some systems

v0.3.7.4 (build 2344; May 5, 2006)
---------
 - fix due to database API change in nightly build #53 that made NAS reset its message tree to the defaults

v0.3.7.3 (build 2341; May 4, 2006)
---------
 - fix for a wrong message in some cases, when using NAS' services
 - some other minor fixes

v0.3.7.2 (build 2338; April 30, 2006)
---------
 - various improvements and fixes to the NAS services (finally managed to get SS and NAS working together ok (hopefully); thanks to Unregistered)
 - improved compatibility with plugins that don't support NAS (implemented a method to detect status changes by such plugins)
 - disabled autoreply on protocols that don't support outgoing messages
 - fixed a bug with usual status in popups on extended status message request when logging is disabled
 - fixed some other bugs

v0.3.7.1 (build 2311; April 22, 2006)
---------
 - fix for contact menu items in unicode build (only the first letter was displayed)
 - some other minor changes

v0.3.7.0 (build 2305; April 21, 2006)
---------
 - unicode support
 - some fixes to the NAS services for compatibility with StartupStatus and AdvancedAutoAway plugins
 - fixed an unnecessary 0x13 character at the end of the popup header

v0.3.6.9 (build 2264; March 15, 2006)
--------
 - now it's possible to get default messages for any status mode using NAS services

v0.3.6.8 (build 2262; March 13, 2006)
--------
 - %nas_requestcount% and %nas_messagecount% fix
 - status message is automatically selected now when the status selection window opens
 - now NAS doesn't add status message to an autoreply automatically. use %extratext% variable in the Autoreply format for that

v0.3.6.7 (build 2257; February 12, 2006)
--------
 - some minor changes to improve MyDetails plugin support

v0.3.6.6 (build 2256; November 2, 2005)
--------
 - status message change dialog didn't ever pop up for Online and Invisible modes, regardless of settings. fixed.

v0.3.6.5 (build 2253; October 30, 2005)
--------
 - updated all the links (also for Updater plugin) to point to deathdemon.int.ru

v0.3.6.4 (build 2250; October 16, 2005)
--------
 - now NAS shares contact status messages with other plugins when requesting them through contact's menu.
 - improved accuracy of status message updates
 - Read status message window is resizeable now
 - Updater plugin support
 - implemented MS_NAS_INVOKESTATUSWINDOW service

v0.3.6.3 (build 2216; September 14, 2005)
--------
 - correct ICQ XStatus logging and popup notification.
 - fixed a bug with status message refresh on startup when setting status through StartupStatus.

v0.3.6.3 (build 2210; September 12, 2005)
--------
 - added a possibility to use %nas_message% variable in the autoreply format, to specify where exactly NAS should place usual status message.
 - added "Log requests only if popup notification for a contact is enabled" checkbox in the Log tab
 - some strings were untranslatable. fixed.

v0.3.6.3 (build 2200; September 11, 2005)
--------
 - workaround for contact list's CLGN_NEXT bug.

v0.3.6.3 (build 2193; September 10, 2005)
--------
 - now NAS will open the status message change window much faster when "Show contacts" is disabled
 - optimized tabs in the options - now NAS' options will load faster
 - previous NAS build didn't load on systems with an old msvcrt.dll version. fixed. (thanks to scHacklaban)

v0.3.6.3 (build 2186; September 8, 2005)
--------
 - fixed a crash on status change when there are no groups in a contact list (thanks to Ladiko)
 - added "Contacts" options tab - it allows to set per-contact settings, and also there is a new setting - per-contact popup notification
 - added %nas_protocol% variable that returns current protocol name
 - fixed a bug with message tree scrolling when per-status recent messages are enabled
 - NAS stored per-protocol recent messages in wrong categories with per-status recent messages enabled
 - some other minor bugfixes

v0.3.6.2 (build 2136; August 28, 2005)
--------
 - fixed a bug with wrong status messages that was introduced in the previous build
 - implemented services that will allow other plugins to get/set status messages in a way compatible with NAS

v0.3.6.2 (build 2129; August 27, 2005)
--------
 - added support for StartupStatus and AdvancedAutoAway, and also hopefully for some other plugins that change status
 - added nas_predefinedmessage variable
 - some minor bugfixes

v0.3.6.2 (build 2110; August 26, 2005)
--------
bugfixes:
 - hopefully fixed problems with autoreply
 - previous builds set an empty message on statuses for which "don't show status message dialog.." was enabled

v0.3.6.2 (build 2108; August 26, 2005)
--------
 - multiple ICQ accounts support
 - optional updating of your status messages on non-ICQ protocols
 - now NAS works pretty well even without Variables plugin
 - some bugfixes
 - added translation.txt

v0.3.6.1 (build 2083; August 25, 2005)
--------
Added a lot of new features:
 - categorized predefined status messages
 - separate default messages for different statuses
 - (bugfix) NAS stores status messages separately for different protocols now. for example, if you'll change your MSN status message, your ICQ message won't be overwritten now.
 - many improvements in user interface
 - some bugfixes

v0.3.6.0 (build 733; August 13, 2005)
--------
 - Just a quick fix of an old build to stop notification of custom status message requests (newer builds are still very unstable).

v0.3.6.0 (build 732; May 16, 2005)
--------
 - First public release.