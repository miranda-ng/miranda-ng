*******************
NewXstatusNotify YM
*******************

Description
===========
This plugin is modified NewStatusNotify plugin by Vasilich with added extra status 
notification and many other features. Plugin shows a popup and logs to a message window 
when someone in your contact list changes status, ICQ XStatus or Jabber mood/activity.

Hidden option: add "EnableLastSeen" byte value to "NewStatusNotify" module and set it
to 1 to store information about the recent appearance of contacts. The information will
be stored in the "SeenModule" in contact and can be displayed using the Tipper plugin.


changelog
**********

--- 1.4.0.9 ---
! fixed crash on opening message log

--- 1.4.0.8 ---
! fixed individual sounds
! fixed xstatus notification when contact goes offline
! fixed logging to ML when contact has no extra status
! removed empty line before status message in popup
! fixed bunch of minor bugs

--- 1.4.0.7 ---
+ added clist modern toolbar support
! fixed stupid bug introduced in the last release

--- 1.4.0.6 ---
+ option for Read status message can be enabled independently
! fixed crash with dbx_tree
! fixed memory leaks

--- 1.4.0.5 ---
! fixed buffer overrun

--- 1.4.0.4 ---
! fixed bug when notification stop working after some time

--- 1.4.0.3 ---
! fixed duplicated protocols and disappeared contacts after meta contact changes status

--- 1.4.0.2 ---
! fixed status icon in popup for meta contacts
! workaround for double logged Jabber activity in tabsrmm
! fixed behavior of the Apply button in options
! fixed saving of "enable/disable popup notification" option to db

--- 1.4.0.1 ---
+ option for disable status description in popup
+ log jabber activity when opening ML
! fixed possible hang of Miranda

--- 1.4.0.0 ---
+ support for Jabber mood and activity
+ better integration of extra status notification
* major part of code was rewritten
! many fixes and improvements

--- 1.3.2.1 ---
+ option for disable notifications only for global status changes
! translation fixes

--- 1.3.2.0 ---
+ added support for Jabber
* automatic close popup after execute click action
* redesigned options pages
* resized individual sounds dialog to fit userinfo window
! fixed notification if 'Disable for XStatus Music' was checked 
! fixed disappearing of xstatus log from message log

--- 1.3.1.3 ---
+ added options for global popups and sounds disabling on defined statuses (Status - Notify -> Configure)
* automatic popups and sounds disabling affect xstatus notifications too
! fixed crash after new mradio 'contact' was added
! fixed wrong size of pluginInfoEx structure (thx eblis)

--- 1.3.1.2 ---
+ updater support
! some fixes in options

--- 1.3.1.1 ---
! fixed crash in options

--- 1.3.1.0 ---
+ support for templates (xstatus module)
+ options for disable notification for xstatus 'Music'
+ options for cut message length to specific number of characters
+ sounds for xstatus message change and xstatus remove
* separate xstatus options for popups and logging to message window
! individual xstatus notification setting available only for protocols that support it
! other fixes and optimalizations

--- 1.3.0.2 ---
* extended individual contact notification setting (separate status and xstatus notification)

--- 1.3.0.1 ---
+ option for disable saving to history
+ option for prevent log indentical events

--- 1.3.0.0 ---
* completely rewrited xstatus module (better integration into the NSN)
* new detection of the xstatus changes (prompt notificaton after change)
* options page redesigned
* many other changes
! fixed Unicode logging to message log
! attempt to fix automatic reading of the status message

--- 1.2.0.2 ---
! fixed individual sounds

--- 1.2.0.1 ---
* SVN-sync 677
 - fixed bug which can lead to MIM crash in Clist module (double free)
 - finally fixed rightclick on popup 
 - made unload with destroying created services ("clean exit") 

Symbols used in changelog: + = new / * = changed / ! = fixed


Author
======
mod by yaho -> YM

ICQ:	356344824
email:	yaho@miranda-easy.net
www:	www.miranda-easy.net
