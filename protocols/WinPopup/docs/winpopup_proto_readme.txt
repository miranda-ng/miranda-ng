WinPopup Protocol plugin for Miranda IM

Copyright (C) 2004-2011 Nikolay Raspopov
mailto: ryo-oh-ki[at]narod[dot]ru
http://www.cherubicsoft.com/miranda/

Communicate with users over LAN, compatible with "net send", WinPopup,
VikPoup, RealPopup, Netter and Miranda`s net_send_protocol, mNetSend plugins.
Uses Mailslot, NetBIOS and Messenger service for sending and receiving messages.

System requirements:

	Network with NetBIOS over TCP/IP enabled

	for 0.0.0.17
		Miranda IM 0.5.2.x
		Windows 95,98,ME,NT.

	for 0.0.0.18
		Miranda IM 0.7.x.x
		Windows 2000,XP,2003,Vista,7.

Notes:

	Search supports wildcards (case insensitive):
		?      - any single symbol,
		*      - any number of symbols including zero,
		#      - any digital symbol,
		[abcd] - symbol set,
		[a-z]  - sumbol range,
		[!...] - NOT in symbol set or range.

	How to send group messages:
		1) Open plugin options page;
		2) Check "Use Mailslot when sending" option;
		3) Press "Add Contact..." button;
		4) Type domain/workgroup name or * for everybody and check "Group contact" option;
		5) Send messge using newly created contact.

	You got "Access Denied" error when switching to online:
		Try to check "Use Messenger service when sending and receiving" option
		in plugin options.
		
	"Legacy online status detection" option:
		Detection online status by <00> NetBIOS-name also,
		normally status detected by <03> NetBIOS-name (aka Messenger).

Installation:

	1) Close similar applications - WinPopup, VikPopup, RealPopup, HiBuddy, NetHail,
		NetSend, Netter, OfficePopup, LanTalk XP etc.
	2) Disable similar Miranda IM plugins - net_send_protocol, mNetSend.
	3) Unzip winpopup_proto.dll to ..Miranda IM\Plugin\ folder.
	4) Restart Miranda IM.

Files:

	winpopup_proto.dll             - WinPopup Protocol Plugin
	winpopup_proto_readme.txt      - This file
	winpopup_proto_translation.txt - Translation template for language pack authors only

History:

	Old versions available at: http://www.cherubicsoft.com/miranda/

	0.0.0.19
	- Tweaked NetBIOS time-outs
	- More NetBIOS debug messages
	- Fixed <CR><LF> sending over NetBIOS

	0.0.0.18
	- Headers updated up to Miranda IM 0.9
	- Added Unicode support
	- Added 64-bit support
	- Added avatar support
	- Added group contact support
	- Performance optimizations
	- Added support for NetLib log
	- Fixed crash at miranda exit when contact still scanning
	- Fixed resource leaks (icons)
	- Fixed small memory leaks by replacing internal ForkThread() with Miranda mir_forkthread()
	- Fixed contact status detection when "Always Online" option set
	- Fixed extra requests of contact status retrieving
	- Fixed contact away message requesting
	- Fixed potential security problems
	- Reduced miranda exit time
	- Changed system requirements (removed Win9x/NT support, added Vista/7 support)
	- Project upgraded up to VS 2008

	0.0.0.17
	- Headers updated up to Miranda IM 0.8
	- Added main menu
	- Added "Legacy online status detection" option (by Anatoliy Samara)

	0.0.0.16
	- Headers updated up to Miranda IM 0.6 and plugin's sources were commented
	- Added support for non-latin (OEM) user/computer names
	- Added "Always Online" checkbox to contact's options
	- Added mNetSend contact's status support
	- Added "Free for chat" and "Invisible" statuses
	- Now Messenger service can be started even from disabled state (Administrators only)
	- Contact status check delay reduced from 60 to 30 seconds
	- Fixed plugin hangup is some cases during Miranda shutdown on Win9x
	- Updated translation file

	0.0.0.15
	- Fixed duplicated NetBIOS-names of options page
	- Fixed non-Messenger dialogs stealing
	- Fixed false NetBIOS error 13 when computer and user names are equal
	- Fixed false Access Denied error when unprivileged user starts Miranda IM and
	Messenger service already stopped or started
	- Fixed incompatibility with WinNT 4.0 (since 0.0.0.14)
	- Fixed empty NetBIOS names registration (some rare plugins combinations)
	- Changed infinite message popups timeout to 10 second timeout
	- Added message boxes with timeouts (used when Popup plugin is absent)
	- Added item "Explore" of contacts menu
	- Added NetBIOS names registration separate selection
	- Added automatic applying (switching to offline/online) of changed options
	- Added remote computer comment field to user info
	- Updated translation file

	0.0.0.14
	- Added full implementation of "Messenger sevice" mode
	- Added enhanced network error messages
	- Added support for Popup plugin
	- Fixed unexpected long timeout of contact online status scanning
	- Fixed missed Translate() calls
	- Fixed worker threads handling (more robust way, like Miranda IM itself)
	- Updated translation file
	- Some minor fixes

	0.0.0.13
	- System requirements lowered to Win95
	- ICMP CODS (contact online detection system :) replaced by NetBIOS CODS
	- Fixed empty message appearance when unsupported type of SMB-packet recieved
	(by Andrew O. Shadoura)
	- Fixed "missed names" in NetBIOS reciever
	- Fixed "connecting 2" plugin status (now "offline")
	- Added NetBIOS method of sending
	- Added NetMessageBufferSend method of sending
	(WinNT + started Messenger service)
	- Added "patient" NetBIOS-names deregistration routines
	- Added search cancellation capabilities
	- Added page with NetBIOS information to User Info dialog
	- Added "NetBIOS names registration status" to Options dialog
	- Added support for Updater plugin
	(http://addons.miranda-im.org/details.php?action=viewfile&id=2254)
	- Updated translation file
	- Some minor fixes

	0.0.0.12
	- Version fix

	0.0.0.11
	- Fixed crash after Miranda exit (DestroyServiceFunction bug)
	- Persistent protocol status icons changed to customized

	0.0.0.10
	- Fixed crash if invalid computer name consists of upper ASCII table symbols
	(russian symbols, germany umlauts etc.) in CharUpper WinAPI32 function
	- Fixed incorrect IP resolving if invalid computer name begins with digital
	- Fixed forgotten "unpingable" contacts (by Andrey anl_public3[at]mail.ru)
	- Fixed 100% CPU usage if network connection was lost (now switching to offline)
	- Added simple NetBIOS-names lookup
	- Allowed contacts with unknown IP (user responsibility)
	- Updated translation file

	0.0.0.9
	- Now searching for computer names, comments or workgroups (using wildcards)
	- Added protection vs mailslot BO-attack
	- Added duplicate message filtering
	- Some small NetBIOS fixes
	- Removed 256 chars message limitation (doh!)
	- Fixed big message (> 424 bytes) handling error

	0.0.0.8
	- Fixed unexpected locking inside Win32 API function ReadFile when 
	reading from mailslot created with MAILSLOT_WAIT_FOREVER flag under Win98
	(MAILSLOT_WAIT_FOREVER replaced with 2000 ms timeout)
	- Added big message splitting (424 bytes parts)
	- Added send message throttle (1000 ms) to preventing receivers mailslot overloading
	- Added direct contact addition (inside Options page) by name or IP
	- Added "RealIP" (equal "IP") and "About" (computer comment) settings to contact
	- Changed translation file

	0.0.0.7
	- Added options page
	- Added "Connecting" status (more smoother startup and shutdown)
	- Added translation template file
	- Added self message recognition
	- Added message CR-LF-B6 normalization
	- Added uninstaller support (PluginUninstaller 1.1.2.1)
	- Some titles renamed from "WinPopup Protocol" to "WinPopup"
	- Extended error messages
	- Project updated up to Visual Studio .NET 2003
	- Project refactored (STL to ATL -> file size changed from 112Kb to 72Kb)

	0.0.0.6
	- Fixed "Unknown Contact" in some poor interface plugins
	- Added persistent protocol status icons

	0.0.0.5
	- Added "IP" setting to contacts (good for some plugins)
	- Optimized ping detection routine
	- Optimized LAN names search routine (now multithreaded, NetBIOS search not yet implemented)
	- Optimized message sender (now multithreaded)
	- Added NetBIOS message receiver
	- Fixed Stop/Restart Windows Messenger Service routine

	0.0.0.4
	- Added experimental support for NetBIOS name registration (computer, current user, nick)
	- Added restoring Microsoft Network Messenger after closing Miranda IM
	- Fixed false contact status detection (was bug in ping functions)

	0.0.0.3
	- Plugin file renamed to winpopup_proto.dll
	- Fixed tiny memory leak in ReceiveMessage function
	- Added online/offline status checker
	- Added auto-answer
	- Added support for extended statuses (away, na, dnd, etc.)
	- Added Miranda-aware safe threads handling
	- Fixed hidden contact of first message

	0.0.0.2
	- Fixed tiny memory leak in SendMsg function
	- Mailslot-receiver thread code refactored

	0.0.0.1
	- Released first version

License:

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
