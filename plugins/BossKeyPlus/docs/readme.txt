                              BossKey+
--------------------------------------------------------------------------
                        -=Miranda Plug-In=-
Copyrights (C) 2002-2003 Goblineye Entertainment, (C) 2007-2008 Billy_Bons
--------------------------------------------------------------------------

Desc:
====

This plugin is a mod of Bosskey plugin and is intended to hide all window icons 
and windows of Miranda on hotkey press.

Whats new:
- Added ability to set a password
- Added Miranda 0.8 support
- Reworked options dialog
- Status management system was rewritten. Now statuses are changed perprotocol.
- Added main menu item
- Updater plugin support
- Various changes and bugfixes

This plug-in hides all of Miranda's windows and system tray icon/s when you press a specified hotkey (Ctrl+F12 is the default), mostly useful to hide Miranda from your boss :)
BossKey was very much inspired by AntiA, and adds a couple of features to the original AntiA (Like: Icon/s can be restored, you can set a status message to when you leave etc.)


Installation:
============
To install the plug-in you will need to copy BossKey.dll to your Miranda's plug-in directory (<MIRANDA DIR>\plugins).


Uninstallation:
==============
Delete the BossKey.dll from <MIRANDA DIR>\plugins.


How to use:
==========
Press the hotkey (F12, by default) from anywhere in the system, and see how Miranda "disappears".
You should know that it's still there, it's just hidden (A quick look using a task manager will reveal it), so keep that in mind :)
You should also check out the options (Events->BossKey), they are somewhat useful :)


Changelog:
=========
v. 0.5.0.3
- Fixed occurrence of the second password window, when double-clicking hotkey

v. 0.5.0.2
- Increased the space for translation in some controls
- Changed the archive's structure to make updater happy

v. 0.5.0.1
- Support unicode status messages
- Removed internal keyboard handling
- Some internal changes and cleanup
- ATTENTION: NOW PLUGIN REQUESTED MIRANDA IM 0.8 AND HIGHER! PLEASE, DO NOT INSTALL THIS VERSION OF THE PLUGIN, IF YOU USE AN EARLIER VERSION OF MIRANDA.

v. 0.4.1.3
- Hook keyboard only if it needed
- Memory leak fixed

v. 0.4.1.2
- Fixed status message restoring (thnx Dezeath & AL|EN)
- Fixed MHeaderBar drawing in aero mode

v. 0.4.1.1
- Password dialog with the MHeaderBar control (for Miranda 0.8.0.29 and newer)
- Icon in Alt+Tab dialog
- Support core's PS_GETMYAWAYMSG service
- Added "Hide Miranda when Miranda is started" and "Restore hiding on startup after failure"
- Added button on Tabsrmm button bar (disabled by default)
- Restoring settings of sound, popups, etc. on startup, if Miranda was incorrectly finished in hidden mode

v. 0.4.0.9
- New options: Hide Miranda when workstation / Miranda is inactive or screen saver is running.
- Fixed crash after account deleting
- Improved the password window
- Some fixes and reorganizations in the Options page
- Some possible crushes fixed
- Some optimisations and bugfixes
- msvcr71.dll now is not required

v. 0.4.0.8
- Rare crush fixed (by Vasilich)
- Minor internal changes
- "Disable popups" option removed

v. 0.4.0.7
- Showing the current hot key combination in menu item
- Added token %bosskeyname% for Variables plugin, returning the current hot key combination (can be used together with UseActions plugin by Vasilich)
- Minor fixes

v. 0.4.0.6
- Minor GUI fix
- Possible crush fixed
- Now you can set the custom text of the tray icon's tool tip.
Just using DBE++ create new item (Unicode)ToolTipText = "your text" in module "BossKey".

v. 0.4.0.5
- Attempt to fix keyboard processing (by Yasnovidyashii)
- Added Ersatz plugin support
- Fixed beta URL's
- Updated PluginInfo

v. 0.4.0.4
- "Show the Tray Icon" option added

v. 0.4.0.3
- Added unicode version
- Added support of the new hotkeys module (core version > 0.8.0.3)
- Added support of the clist_modern's toolbar 
- Automatic change of the keyboard layout in the input password window
- More correctly hiding of floating contacts
- Other minor fixes and changes

v. 0.4.0.0
- Added "Hide Miranda if the computer is locked" option
- Added "Use default status message" option
- Added variables plugin support
- Added storing and restoring of status messages
- Functions of hiding and restoring icons are changed: now it should be more correct, but does not work on old versions of core ( < 0.7) and clists
- Temporary workaround for crash under win98 with popup+
- Random crash fixed
- Memory leaks fixed
- Various bugfixes and changes
- The minimal supported version of a core now is 0.7
- Added langpack
