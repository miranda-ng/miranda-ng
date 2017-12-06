///////////////////////////////////////////////////////////////////
// Gadu-Gadu Protocol Plugin for Miranda IM
// Based on unix ekg client functionlity and libgadu
//
// Author: Adam Strzelecki <ono+miranda@java.pl>
//
// Thanks: Santithorn Bunchua "KeH", Rober Rainwater,
//		   libgadu Authors <http://dev.null.pl/ekg/>,
//		   Liquid (Image send/recv), The Leech (Proxy fixes)
//		   MirandaIM Polish Forum
//		   Angeli-Ka (Gadu-Gadu plugin icons)
///////////////////////////////////////////////////////////////////

1. About
--------
The Gadu-Gadu plugin provides functionality of original Gadu-Gadu
client extended with extra features available only for Miranda IM
users. Gadu-Gadu is most popular (competitive to ICQ) instant
messaging system in Poland. Now supports encrypted secure SSL/TLS
connections.

2. Features
-----------
 - Send/Receive messages
 - Add/Delete users
 - Public catalog search
 - Public catalog info changing
 - Import/export/remove from server and text file
 - Status messages support (offline messages too)
 - Visibility/ignore lists
 - Password changing
 - Password remind
 - Extended public catalog search
 - HTTP proxy support (uses only Netlib settings)
 - Create/Remove Gadu-Gadu account
 - File transfer
 - Image receving / sending (by Liquid)
 - Multiple plugin instances/accounts:
   <name>.dll or <name>debug.dll becomes proto name and id
   Warning: <name>.dll or <name>debug.dll is treated as
			the same proto and name is case insensitive.
 - 1st user mail & phone used for import/export
 - SSL/TLS secure connections
 - Groupchat/conferencing (Miranda IM version 0.4 or higher)

Note: For secure connections this plugin requires OpenSSL DLL
	  libraries to be present in system folder or Miranda folder.
	  These libraries can be downloaded from:
	  - http://www.miranda-im.pl/download/misc/
	  - http://www.openssl.org/.
	  If DLL files are not present plugin works only with plain
	  unsecure connections.

4. TODO
-------
 - Animated GIFs support (image sending)
 - Full Netlib integration
 - Integration with Miranda Image services
 - Voice chat

5. History
----------
0.0.4.6: 2007-07-04
 - Fix: Message doubling (i.e. with MetaContacts), thanks to Scott Ellis
 - Fix: Default icon setting problem
 - Rewritten image & token send & reception using Miranda Image services
   (This requires latest Miranda version in order to run)
 - Fix: Window size for big images on sending & reception
0.0.4.5: 2007-06-20
 - Angeli-Ka icons are now default Gadu-Gadu icons
 - Protocol status icons are now built in the Plugin, so taken by default
 - IcoLib support, now you can redefine all GG icons including "blocked" icon
 - Gadu-Gadu libgadu update to support fully DCC7 transfers and GG7.7 features
   (has to be tested, if it really works)
 - Some safety internal fixes
0.0.4.2: 2007-02-22
 - Binary release: Recompiled to support new NETLIB settings structure.
   !!Warning!! Plugin requires now Miranda 0.6.1+ for proxy support.
0.0.4.1: 2006-12-20
 - Fix: 512 characters for settings such as custom server list
 - Fix: Detection of Gadu-Gadu versions upto 7.6
 - Plugin now writes MirVer, so all tooltip plugins can show contact client type
   and version string.
0.0.4.0: 2006-10-23
 - Fix: Messages always have NOW timestamps unless they are OFFLINE.
   Online messages won't be shifted in the order even user has clockskew.
0.0.3.9: 2006-07-29
 - Configuration was put into tabs
 - Fix: Some of the configuration items were not triggering [Apply] properly.
0.0.3.8: 2006-06-26
 - Fix: It seems that GG servers now properly handle initial status for GG.
   This should fix issue of being detected by Inwigilator when connecting with
   invisible status. (Spotted and fixed thanks to piopawlu)
 - Fix: New libgadu version 2006.06.21 with fixed some memory leaks
 - Fix: GG now handles PS_SETAWAYMSG without previously called PS_SETSTATUS.
   This should fix issue with Watrack that couldn't set the status message
   for GG protocol. (Spotted and fixed thanks to mruweq)
 - Fix: We were reading from freed variable after new account registration
 - Fix: Possible crash when closing debug version
0.0.3.7: 2005-12-19
 - Image sending and receiving now uses ImgDecoder if present for PNG images
 - Fix: String fix for error dialogs' titles
0.0.3.6: 2005-11-15
 - Fix: String overflows (injections) that may be caused for example by too long
   translations or too long incoming filenames
0.0.3.5: 2005-10-28
 - Fix: Descriptions beginning with new line were not shown
 - Fix: Groupchat fixed for database locks
 - Fix: Upgrade new chat API
 - Fix: Image reception on system tray was broken
 - Fix: Image windows are now closed on Miranda exit
 - Fix: Plugin was writing junk entries to other protocols contacts
 - Image reception can now be triggered
 - Images are now scalled to their real size if possible
 - Small code formatting fixes
 - SSL connections are disabled now by default
0.0.3.4: 2005-07-23
 - Fix: Connection server config was broken
 - Fix: Password retrieval was broken
 - Fix: Maximum image size 512KB was not accepted original GG client.
   Miranda now can receive images upto 255KB from original GG clients.
 - Fix: Yet another image receive fix, libgadu fixes introduced ghost image
   messages
 - Small code formatting fixes
0.0.3.3: 2005-07-19
 - Fix: Serious multiple vulnerabilities of libgadu upto ekg version 1.6rc3
0.0.3.2: 2005-07-14
 - Fix: libgadu event.c commit version 1.83 by szalik introduced bug that was
   crashing Miranda on image receive, since it was designed specially for Kadu
0.0.3.1: 2005-07-12
 - Hidden db setting "EraOmnix" for being visible as EraOmnix client
 - Fix: Temporary contacts cannot see our status when having "Friends only"
   (Features thanks to JacekFH)
 - Fix: GG thread hanging, not going offline when cannot reconnect
   after disconnection
0.0.3.0: 2005-06-27
 - Status descriptions compatible with MWClist
 - GG.dll now compiles on Visual C++ 6.0 and Visual C++ 2005
 - libgadu upgraded to version 20050615
 - Binaries are now compiled with VC++ 6.0
 - Fix: Windows 95 & NT problems with GetLongPathName function missing
0.0.2.9: 2005-04-29
 - Fix: Status descriptions were not set properly when Miranda wasn't asking
   for new status. This change may cause incompatibility with status routines
   different than built-in. Note for misc plugins (mBot, etc.):
	 For avoiding sending gg_setstatus(status, description) twice on
	 miranda_setstatus and miranda_setawaymsg, GG sets status only on
	 miranda_setawaymsg, also this order must be fulfilled to make GG working
	 right with Miranda. (this is how internal module sraway works actually)
 - When reconnect flag is set and manual server host cycling is on the end of
   the list, reconnect will go back to the begin
   (no longer taking address from web at the end)
 - Fix: Stupid GetModuleFileName returning all lowercase characters from
   DLL filename > WinNT, now we call GetLongPathName to retrieve valid case
0.0.2.8: 2005-04-14
 - Server connection (multi)thread controller (Experimental!)
   Miranda won't hang anymore when GG is trying to connect & status change is
   requested (However sometimes on exit it required killing threads (dirty))
 - Multiple server hostnames (cycling while connecting)
 - Conference policy setting
0.0.2.7: 2004-12-26
 - Gadu-Gadu conferencing support trough chat.dll groupchat plugin
   (requires > Miranda 0.4)
0.0.2.6: 2004-12-13
 - Fix: Notification list should be sent in one piece gg_notify_ex(*uinlist),
   not one by one gg_add_notify_ex(uin)
 - Fix: GG.dll uses now internal Miranda safe thread registering
0.0.2.5: 2004-12-01
 - Fix: Plugin crashed when reporting token download connection related errors
 - Fix: Max image size limit changed to 512KB
 - Option to turn on/off showing contacts that are not on my list but having me
   on their contact list.
0.0.2.4: 2004-08-03
 - Fix: Code of image routines was simplyfied, however needs more care (unstable)
 - Server host and external forwarding host could be specified as a hostname or
   IP number (before only IP number was valid). This could be useful for dynamic
   IPs and DNS mapping services such as dyndns.org.
0.0.2.3: 2004-07-15
 - Fix: Proxy authentication fixed (by The Leech)
 - Images receiving / sending (by Liquid)
0.0.2.2: 2003-12-20
 - Fix: Autoreconnect failure, no "Connecting..." message
 - Fix: Latest M-IM build sends twice SET_OFFLINE when disconnect
   which caused GG plugin to hang
0.0.2.1: 2003-11-28
 - Source: GCC >= 3.2 compiler support
 - Fix: File transfer crashes, 100% CPU utilization
 - Fix: Proxy working again (HTTP w/auth)
   Note: GG works only with HTTP proxies (also with auth) due to limitation of libgadu.
   Please turn off "Manually specify connection host" before running trough proxy.
 - Fix: Filesize 135KB - deleted unused icons (thanks to Jacek_FH)
 - Filetransfer resume support
 - Ignore incoming conference messages option
0.0.2.0: 2003-10-27
 - Gadu-Gadu 6.0 support extended
 - Token support: Working pass changing, user registration, deletion
 - Warning: Registration e-mail changing not working (no libgadu implementation)
 - Server userlist manipulation working again
0.0.1.9: 2003-10-16
 - Gadu-Gadu 6.0 (libgadu-CVS) support
 - Warning: Register, unregister, password change/remind, userlist server manipulation
   not available for the moment. Need to implement new TOKEN routines introduced in GG6
 - Warning: SSL still having problems with connection (to be fixed)
 - Client version info in contact details
0.0.1.7: 2003-09-xx (Unreleased, tests and fixes)
0.0.1.7: 2003-07-06
 - SSL/TLS secure connection to server support (trough OpenSSL)
 - Option to turn on/off message acknowledgement
 - Fix: Reconnects when connection is broken on startup only if
   auto-reconnect option is turned on
 - Fix: Owner's nickname properly displayed in chat/msg dialogs
   Note: Use View/Change my details... to load-up nickname.
 - Fix: Plugin doesn't link to any ssl library but loads it dynamically
   when OpenSSL dll files exist in system or Miranda folder.
0.0.1.6: 2003-06-04
 - Fix: Auto-reconnect do not get 100% cpu anymore when no network (1 sec per trial)
 - Fix: Blocked icon works again (upx shouldn't compress icons)
 - Fix: Contact description is now cleared when contact makes it empty
 - Debug: logged connection errors are more specific now
0.0.1.5: 2003-04-09
 - Fix: Multiple instances menu items worked just for first instance
 - Fix: Auto-reconnect fix, again working
 - Time deviation support for timestamps (default 5 min)
   "TimeDeviation" db setting if your clock differs more than 5min from actual time
 - Better error descriptions on logon failure
0.0.1.4: 2003-04-04
 - Fix: User IP numbers in user info shown better (port < 10 means behind firewall)
 - Fix: %ip% compatible now with tooltip plugin
 - Fix: Import/Export now uses first user (editable) e-mail and phone number
 - Fix: Remove all old db uneditable e-mail & phone settings from previous plugins
 - Debug: Advanced libgadu debug log information displayed in NetLib dump
 - Filetransfer support with firewall forwarding support (except multiple files)
 - Multiple instances of plugin supported (see feature list)
0.0.1.3: 2003-03-31
 - Fix: Send chat type message instead of normal message
 - Fix: Next 20 results displayed after searching again
 - Fix: Public catalog change always writes Male as gender
 - Extended public catalog search
 - HTTP proxy support (uses only Netlib settings)
 - Create/Remove Gadu-Gadu account
 - Change Gadu-Gadu registration e-mail (requires valid e-mail in configuration)
0.0.1.2: 2003-03-19
 - Select status for description after disconnected
 - Fix: Finally working away msgs based on KeH's Jabber plugin code
 - Fix: Info page fix for empty "Gender" field
 - Fix: Import code for groups and subgroups
 - Removed: "Change status safely" no longer needed
 - "StartInvisible" db only option to start fully invisible
0.0.1.1: 2003-03-16
 - Fix: Male/female fix in public catalog
0.0.1.0: 2003-03-16
 - Fix: Public info changing now works
 - Fix: Status changing bugs
0.0.0.9: 2003-03-16
 - Fix: Import bugs
 - Fix: Status changing bugs (still fighting with)
0.0.0.8: 2003-03-15
 - Fix: Multiple main connection thread runned
 - Fix: Status hangup on Connecting...
 - Password & e-mail changing
 - Password remind to e-mail
 - Owner info changing [NOT WORKING DUE PROTOCOL CHANGES]
 - Autoreconnect when disconnected unintentionally
 - Turn on/off connection errors
 - Leave last status after disconnected
 - Fix: Double status change on login
0.0.0.7: 2003-03-14
 - Fix: Blocked icon fixed
 - Fix: Safe status changing
 - Fix: Contacts status set offline when disconnected
0.0.0.6: 2003-03-13
 - Fix: Import/export fixes
 - Name unknown contact with nickname from public catalog
   if available
 - Remove from server
 - Visible/Ignore (blocked) list
0.0.0.5: 2003-03-13
 - Contacts info tab
0.0.0.4: 2003-03-12
 - Fix: Import from files
 - Fix: Search and away msg retreival
0.0.0.3: 2003-03-12
 - Import/export from server/text file
 - Fix: Improved stability
0.0.0.2: 2003-03-11
 - Improved stability
 - Status descriptions
 - Show invisible users
 - Friends only
0.0.0.1: (UNRELEASED)
 - Initial Release
 - Basic messaging

6. Latest development version:
------------------------------
Latest development version is always available at:
	http://www.miranda-im.org/development/

	- GG.dll - release version
	- GGdebug.dll - debug version, debug infos in NetLib log
	- gg-readme.txt - this file with modified changelog
	- gg-translation-sample.txt - latest partial langpack

7. Bugs
-------
 - Before sending reports check if bugs are still present in latest
   development version
 - [Polish] Send bug reports to Polish Miranda IM Forum at:
		http://www.miranda-im.pl/viewforum.php?f=3
 - [English] Send bug reports to Miranda IM bugtracker at:
		http://sourceforge.net/tracker/?atid=102179&group_id=2179

8. Discussion & Feature Requests
--------------------------------
 - Request features, discuss plugin at Polish Miranda IM Forum:
		http://www.miranda-im.pl/viewforum.php?f=3

9. Source code
--------------
Source code of this plugin is available at Miranda IM SVN repository.
Consult for details:
		http://sourceforge.net/svn/?group_id=94142

Copyright (C) 2003-2007 Adam Strzelecki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
