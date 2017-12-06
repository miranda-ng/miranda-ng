ClientChangeNotify plugin for Miranda IM.
(c) Chervov Dmitry aka Deathdemon.

This plugin shows a notification when someone in your contact list changes his client.
It requires Popup or YAPP plugin.

"Ignore these clients" setting is a list of substrings separated by semicolons; if ClientChangeNotify finds one of these substrings in a new client name, it doesn't show the notification.
Also it's possible to use regular expressions, but it requires pcre.dll or pcre3.dll in your miranda directory.
PCRE is available from http://www.pcre.org/ (Binaries for Windows are here: http://gnuwin32.sourceforge.net/downlinks/pcre-bin-zip.php )
When PCRE library is installed successfully, ClientChangeNotify will show "you can use regular expressions here" under the "Ignore these clients" field in the options.

Regular expressions are detected by '/' symbol at the start of the substring. The same symbol must be at the end of the substring, possibly followed by pattern modifier characters. I.e. "gmail;/[0-9A-F]{8}(\W|$)/i;skype" has two usual substrings and one regexp substring with an "i" modifier (caseless).

There are several solutions to filter jgmail client changes:
1) Disable notification about client version changes
(Miranda options -> Popups -> ClientChangeNotify -> "Notify also when just client version changes" checkbox, turn it off).
This solution is somewhat easier, but ClientChangeNotify won't notify you of any client version changes.
2) Disable client change notifications for separate contacts
3) Install PCRE library (see the download link above), and make sure that substring "/[0-9A-F]{8}(\W|$)/" is present in the "Ignore these clients" field of the ClientChangeNotify options. If the substring is not there, add it.
This solution is the most correct.

ClientChangeNotify supports per-contact notification setting. There are two ways to use it: either by using ContactSettings plugin that adds a GUI to easily modify this setting, or by modifying the database directly:
Contact/ClientChangeNotify/Notify (BYTE) =
 0 - completely disable notifications for this contact
 1 - show notifications always, except when ClientChangeNotify popups are turned off
 2 - show notifications always, even when ClientChangeNotify popups are turned off
 3 (default) - use global settings (the plugin will examine the following two checkboxes before deciding whether to show the notification: "Notify also when just client version changes" and "Don't notify for contacts with disabled status notification")

Changelog
=========

v0.1.1.2 (build 61; Jan 12, 2008)
--------
 - workaround for contact lists bug with multiple submenu items when using a localization file

v0.1.1.1 (build 60; Jan 2, 2008)
--------
 - added an item to the Popups submenu of the main menu to toggle client change notifications
 - fixed regexp to ignore jgmail clients
 - improved pcre library detection
 - changed notification text formatting
 - changes in ContactSettings API

v0.1.1.0 (build 54; Sep 17, 2007)
--------
 - added support for Miranda 0.8. CCN interface is {E9D1F0D4-D65D-4840-87BD-59D7B4702C47}
 - moved logging features to a separate plugin - LogService
 - added possibility to ignore certain clients, also using regular expressions
 - added per-contact setting to turn notifications on/off, and support for ContactSettings plugin
 - some other minor improvements

v0.1.0.9 (build 29; Jul 3, 2006)
--------
 - fixed non-working preview on some profiles
 - fix to log unique ID of every protocol, not only ICQ UIN

v0.1.0.8 (build 27; Jul 2, 2006)
--------
 - disabled notification for new contacts
 - implemented unicode logging correctly

v0.1.0.7 (build 24; May 2, 2006)
--------
 - unicode support
 - some minor fixes

v0.1.0.6 (build 22; Apr 15, 2006)
--------
 - fixed disappearing client icons in popups

v0.1.0.5 (build 18; Apr 15, 2006)
--------
 - fixed a crash with some "unusual" clients

v0.1.0.4 (build 17; Apr 14, 2006)
--------
 - Added a check whether Fingerprint plugin is installed or not. Now ClientChangeNotify can work without it, but with a limited functionality. It's still highly recommended to install Fingerprint.

v0.1.0.2 (build 15; Apr 14, 2006)
--------
 - First public release.