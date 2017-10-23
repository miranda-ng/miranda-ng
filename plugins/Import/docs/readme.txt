
                   Import plugin for Miranda NG
                   ____________________________


About
_____

This Miranda IM plugin allows you to import contacts and messages
from your Mirabilis ICQ database and from Miranda IM profiles. Please note
that this plugin does not import settings and preferences.



Installation
____________

Simply copy "import.dll" into your Miranda IM plugin directory (usually
"C:\Program Files\Miranda IM\Plugins\") and restart Miranda IM.

You start the import by selecting "Import..." in the main menu.



Changes
_______

0.9.2

Bugfixes:
	- Another crash with non-IM contacts.
	- Miranda IM visibility lists were imported wrong.
	- Some general stability improvements when dealing with corrupted
	  Miranda IM databases.


0.9.1

Bugfixes:
	- No longer crashes when importing Miranda contacts from a protocol
	  with no unique ID setting (Weather, RSS, etc).


0.9

Bugfixes:
	- Don't auto run wizard until at least one protocol is installed.
	- Don't enable Mirabilis import unless ICQ is installed.
	- Memory leaks.

New features:
	- Supports all installed protocols.
	- 20% smaller.


0.8.1

Bugfixes:
	- Fixed a crash while importing from Miranda IM databases.

New features:
	- Added AIM support for Miranda IM import.
	- Added YAHOO support for Miranda IM import.


0.8

Bugfixes:
	- Fixed two memory leaks and a crash while importing from Miranda IM databases.

New features:
	- Added ICQ Corp support for Miranda IM import (thanks Sergey).



Known Problems
______________

- It is not possible to import from Mirabilis ICQ 2003b. This feature is being
  worked on and will be available in a future version.

- Weather stations, RSS feeds, and other non-IM items are not imported when you
  import contacts from a Miranda IM database. It has not yet been decided if
  this feature will be added or not.
  


Support and Bug Reporting
_________________________

We cannot give support through e-mail or instant messages. Please visit the Miranda IM
help page at http://www.miranda-im.org/help/ if you need help with this plugin.

If the help page does not answer your question, please visit the Miranda IM support
forum at: http://forums.miranda-im.org and we will try to assist you. Remember that
everyone involved in this project are unpaid volounteers, so keep it friendly.

If you want to report a bug, do so in the official bug tracker at:
http://bugs.miranda-im.org/



Contact
_______

Current maintainer is Martin Öberg.
E-mail: strickz at miranda-im.org



License and Copyright
_____________________

Copyright (C) 2012 George Hazan

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
