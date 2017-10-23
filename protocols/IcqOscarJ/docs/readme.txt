
          ICQ protocol plugin for Miranda IM (Joe)
          ________________________________________


About
_____

This Miranda IM plugin makes it possible to connect to the ICQ
instant messenger network and communicate with other ICQ users.


Known Problems
______________

- In Invisible mode, when other side is not in visible list messages are sometimes
  received twice from some clients (it is a problem of that clients, they do send them
  twice)

- When synchronising server-list, users get added with await auth flag or receive added
  message. The way which ICQ servers handle authorizations, we cannot do it better - when
  uploading contacts to server account has to be a new one, then contacts can be added
  and uploaded without auth... If the account is an old one they cannot be added without
  auth if they require one. So they are added with flag awaiting auth. With the new
  Manage server list contacts added contacts should never receive auth requests, but
  if you have an old account they can receive added notification.

- Some wicked firewall software can cause HTML tags (e.g. <font> </font>) to be added to
  incoming messages.

- With HTTP Proxy Mode in Connection settings, neither Avatars nor File Transfer can work.
  This is partly a limitation of current Miranda's network module implementation. But
  File Transfer cannot work with HTTP Proxy, that is a protocol limitation.


TO-DO List
__________

0.6
  Multi-user chat
  Mid-NetLib module for HTTP gateway (for Avatars to work) [icq_net]
  
0.5
  Full support for sub-groups in Manage server-list dialog
  Simple sub-groups support (optional)



Changes
_______

0.5 Development

Bugfixes:
  Fixed receiving of multi-part messages (only first part was received)
  Fixed processing of UTF-8 encoded status messages from QIP
  Fixed AIM away message handling (caused online status to be seen as away in some clients)

New Features
  Account management (requires Miranda IM 0.8 and newer)
  New user details protocol support
  Added support for status notes & moods (introduced by ICQ6)

Improvements:
  URL events are now received as normal messages
  Use new offline messages protocol (supports unicode)
  Ability to send/receive contacts to/from ICQ6 and AIM
  Server-list support now groups requests - much more faster & reliable
  Improved ChangeInfo module compatibility with screen readers
  Client detection improvements
  Avatars are now retrieved even for offline contacts


0.3.9.5

Bugfixes:
  Fixed connection problems with many contacts in server-list
  Do not lose server-list contacts due to too long nicknames/comments

Improvements:
  Reinvented known spam bot blocking


0.3.9.4

Improvements:
  Added ability to handle malformed UCS-2 encoded offline messages
  Fixed possible local buffer overflow


0.3.9.3

Bugfixes:
  Fixed problem with utf-8 message API and some message plugins (weird chars)
  Fixed double Avatar pages in User Details for newer Miranda IM


0.3.9.2

Bugfixes:
  Fixed possible memory corruption caused by malformed langpack
  Added support for new utf-8 message API (when available)
  Fixed possible rare problems with Folders plug-in support
  Other minor fixes merged from trunk (0.3.10.x Development)


0.3.9.1

Bugfixes:
  Fixes several minor problems
  
Improvements:
  Oscar File Transfers now fully handle directory transfers
  

0.3.9

Bugfixes:
  Fixed peer-to-peer session cookie checking (was working only due to coincidence)
  Fixed SMS message handling, made message parsing more consistent
  Applied fix for re-requesting authorization (thx Bio)
  Fixed handling of message acks & status msg replies from ICQ 6
  Fixed handling of malformed messages acks
  Minor fixes for Popup plugin support
  Minor protocol related tweaks
  Fixed possible lags caused by typing notifications (on slow connections)
  Fixed message handling (fixes bug used for stealth invisibility detection)
  Several fixes to Rate Level 2 handling - more propper handling of message responses  

New Features:
  Oscar File Transfers - like icq5 (including: transfer thru server, resume)

Improvements:
  Client detection improvements
  Level 1 Rate management (global) - should solve problems with many contacts in list
  Support for Custom Profile Folders plug-in - configurable avatars cache folder
  Ability to send long offline messages (up to 4096 characters)
  Messsage Error handling is now complete (no more disconnections on long msgs) 
  Support for Aim contacts enhanced
  Better support for Miranda Ignore module


0.3.7.5

Bugfixes:
  Fixed possible crash in avatar queue
  Fixed special character conversion in Custom Status messages
  Minor changes to client detection


0.3.7.4

Bugfixes:
  Some small fixes to Custom Status API
  Fixed possible buffer overrun problem in status change packet handling
  Some avatar related fixes

Improvements:
  Added avatar validity checking


0.3.7.3

Bugfixes:
  Fixed HTTP/HTTPS proxy support (removed useless gateway option), long messages are now sent properly
  Improved status change handling (can handle broken packets)


0.3.7.2

Bugfixes:
  Fixes possible buffer overflow with contact status change
  Fixed migration was not working at all (should not disconnect so often on server's upgrading)
  Fixed offline greeting cards were not processed properly
  Fixed small protocol glitch with reverse peer-to-peer connection


0.3.7.1

Bugfixes:
  Fixed possible hang on exit
  Fixed avatars were not working when server port set to 0
  Made message parser more strict (fixes bug allowing stealth invisible client detection)
  Fixed Popup's unicode support cannot be used in ANSI mode even if available
  
Improvements:
  Added new session password API - allowing plugins like Secure Login to work better


0.3.7

Bugfixes:
  Our own contact in list now correctly handles events
  Proxy Gateway mode is working again
  File receiving from Trillian clients work again
  Incorporated some bugfixes from ISee (thx Bio)
  Message errors are now really displayed
  Fixed memory leak in avatar code
  File transfers with GnomeICU works now
  Fixed changing password in ICQ Details will not save it to DB if not there
  Fixed PING channel handling (caused connection drop)
  Fixed bug in changeinfo with translated listbox values (thx Bio)
  Fixed error handling in avatar code for uploading avatar
  Fixed bug in Manage-Server list dialog, caused hang
  Fixed occasional losing of messages (message parsing was incomplete)
  Server-list management now stores all info - solves problems
  Fixed unknown errors during login were not reported sometimes

New Features:
  Messaging support enhanced (P2P messaging supported)
  Direct Connection support rewritten (with P2P messaging and reverse file-transfers)
  Temporary Visible List support
  New Features page in options to configure extra features
  Custom statuses just like icq5 - needs clist_mw derivative (thx Big Muscle)
  New custom status icons (thx Angeli-ka)
  Enhanced Unicode & Ansi charsets support (each contact can have its own codepage)
  AIM cross-compatability (add/remove, search by mail, status, messaging, away messages)
  Secure MD5 login authentification
  Revoke authorization (Advanced Feature - see further) - removes you from contact's server-list

Improvements:
  Advanced search now can use newer method (thx Bio)
  Better protocol error handling
  Slightly better avatar handling
  Detects some spam bots & ability to block known spam bots
  Merged with ChangeInfo (thx Bio)
  Added possibility to remember password for session
  Added possibility to retrieve away message from libgaim clients
  Added status to UserInfo page
  Small improvements to client detection
  Uses newer capabilities format (cuts incoming traffic)
  Display error messages using PopUp Plugin
  Support for Iconlib Plugin
  Support for unicode database module (clist data)
  Level 2 Rate management for Status & Xtraz replies, Xtraz requests (useful with many contacts in list)
  Full unicode/ansi support (2in1)
  Support for Updater Plugin
  Added support for Flash Avatars Service Plugin
  Added ability to upload own flash avatar - XML file (thx Big Muscle)


0.3.5.3

Bugfixes:
  Fixed losing of almost all messages (protocol change)


0.3.5.2

Bugfixes:
  Error checking on offline messages was not working
  Now correctly handles "import time" item (should solve some auth issues)


0.3.5.1

Bugfixes:
  Renaming of server groups was not working properly - caused errors
  Avatar could not be deleted (the delete button did not work)
  File Transfers were not working properly in some cases (wrong cookie handling)
  Avatar formats were not recognized correctly
  Method of determining target dir in file receive was not solid enough
  Manage server-list dialog leaked memory
  Synchronize visibility items was not working properly
  Our avatar image was not linked to our ContactPhoto
  Added workaround for DB Blob caching issues
  Fixed occasional crash on login (missing TLV validity checks)
  Fixed slow update of nicks when users imported from server-list
  Fixed auto info update mechanism, do not progress too fast, do not drop processing
  Fixed empty groups are always hidden in Manage Server List, cannot be used either
  Fixed occasional crash on avatar retrieval - limit size of image to the size of packet
  If our rate is high, ignore user requests for status msgs & user details (prevents disconnection)
  Added temporary solution for roughly translated ICQ 2003b russian
  Manage server-list dialog could display other contacts and could crash
  Basic search could search for bad uin - garbage can be in the string
  Added workaround for select() malfunction - caused high CPU load

Improvements:
  If Update details from server is on, user groups are also updated
  Changed System Uptime to Member since in my details
  Auth system recognizes & sends UTF-8 messages
  Miranda version signature improved (preparing for old signature removal in the future)
  Added better error detection for offline msgs receival process
  Made avatar handling more resilient to server errors


0.3.5

Bugfixes:
  Server-list operations is now scheduled correctly
  Newly added contact with privacy setting is not hidden anymore
  Fixed unicode message receiving from Icq2Go! (messages were corrupted)
  Fixed Grant authorization - not showed correctly and crashing randomly
  Move to group was not working properly
  On accept/decline filetransfer miranda was sometimes disconnected
  Group with subgroups was deleted if empty, that messed up subgroups
  Newly added contacts from server sometimes missed their group
  Offline messages are no longer received older than existing ones
  Now will not try to add contacts to server, which are not ours
  Divided server ids to groups - caused strange behaviour if id and group id were same
  Other small fixes

New features:
  Added avatar tab to user-details dialog to show avatar even without mToolTip
  Linking avatar to mToolTip is now optional
  My user-details now show more informations & added idle since to ICQ tab
  Added support for uploading your own avatar image

Improvements:
  Rewritten Manage Server List dialog - now works perfectly (without sub-groups support)
  Added partial support for subgroups (supported: rename, move, parial: add)
  Added optional linking to mToolTip, link only if no image set
  Added workaround for QNext client (it is not capable of type2 msgs)
  Added option to turn off HTTP gateway on HTTP proxy


0.3.4.2 (not published)

Bugfixes:
  Fixed authorization reply
  Fixed contact transfer ack
  Now parses URL send ack correctly, no more timeout
  Now sending ack on contacts receive
  Now correctly add contact without auth if they does not require it
  Fixed crash on receiving long message through P2P (very old bug)
  Many other fixes (see CVS changelog for details)

New features:
  Added full unicode message support (requires SRMM Unicode)
  Added support for sending and receiving Idle time.
  Added reliable client identification (if not identified, gives appropriate ICQ client)
  Added support for avatar images (downloading only).
  Added Grant authorization option (send & recognize)

Improvements:
  Server-side list support rewritten, now uses acking, partly supports groups (without sub-groups for now).
  Most cookies standardised to imitate icq5 behaviour
  Basic search now automatically removes garbage from text, e.g. it can search by 123-456-789


0.3.3.1

Bugfixes:
	Could crash when receiving unicode messages.


0.3.3

Bugfixes:
	Failed to send or receive files from ICQ 2003b.
	Fixed a number of smaller memory leaks.
	Contact e-mail info was not displayed correctly.
	Failed to retrieve user details during certain circumstances.
	URL messages could disappear when sent through a Direct Connection.
	Nick name was not deleted from server list when local nick name was deleted.
	Server side contacts could reappear after being deleted if they were on the
	visible/invisible lists.
	Changing status while connecting had no effect.
	A bunch of other fixes that are too boring to list here, have a look at
	the CVS changelog if you want the big list.

New features:
	Added support for sending and receiving Typing Notifications.
	Now accepts messages formatted in unicode (note: this wont solve the problem
	with displaying messages with multiple charsets).

Improvements:
	Uses plugin DLL name in various menus to make it easier to have several ICQ
	plugins loaded.
	More robust packet parsing reduces the risk of any future stability problems.


0.3.2

Bugfixes:
	Prevent your status messages being read when you are invisible.
	Small memory leak when sending an SMS.
	Fixed a dumb bug that caused random disconnections from the ICQ server.
	Cleaned up the code for searches and user info updates, should work better now.
	AIM users in your server contact list would get added locally with uin 0.

New features:
	Added "missed message" notification.

Improvements:
	Added better error messages for message send failures.
	Messages now default to the most reliable delivery method available for a given contact.


0.3.1

Bugfixes:
	Fixed crash when incoming file transfer was cancelled before the transfer started
	Failed to receive SMS messages sent while offline
	Fixed some problems with accepting file transfers from Mirabilis clients
	Increased thread safety to reduce some reported connection/disconnection problems
	Fixed compability problems with sending messages to some jabber clients
	Fixed some message compability problems with Trillian
	Corrected some ack sending
	Added a lot of safety checks to increase general stability
	Plugin didnt load unless winsocks2 was installed

New features:
	Removed restrictions on UIN length for better compability with iserverd
	The password can now be left empty in options, it will be asked for during login
	Server port can now be left empty in options, a random port will be selected during login
	Show logged on since and system uptime info in ICQ tab

Improvements:
	Reduced file size
	Message sending now uses a more reliable delivery method



Support and bug reporting
_________________________

We cannot give support on e-mail or ICQ. Please visit the Miranda IM help page at
http://www.miranda-im.org/help/ if you need help with this plugin.

If the help page does answer your question, visit the Miranda support forum at:
http://forums.miranda-im.org/ and we will try to assist you.

If you want to report a bug, please do so in the official bugtracker at:
http://bugs.miranda-im.org/



Contact
_______

Current maintainer is Joe @ Whale, jokusoftware at miranda-im.org



Advanced Features
_________________

There are some features that are not widely used so there is no UI for them, if
you want to use them you need to do manual setup using DBE++.

Database items (usually at Contact\ICQ):
(BYTE)UnicodeSend = 0 - to Disable unicode support (useful as a workaround for Trillian's bug)

(WORD)CodePage - codepage for plain text encoding/decoding to/from unicode

(WORD)InfoCodePage - codepage for interpreting user details

Database items for Main Contact (usually at Settings\ICQ):
(DWORD)KeepAliveInterval - Period in which Keep-Alive packets are send if enabled - default is one minute

(BYTE)PrivacyItems = 1 - adds more privacy tools to contact menu (e.g. Revoke Authorization)



Credits and Thanks
__________________

Thanks to Angeli-Ka for nice custom status icons.



License and Copyright
_____________________

Copyright (C) 2000-2010 Joe Kucera, Martin Öberg, Richard Hughes, Jon Keating

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
