		-===========================================-
		 Simple Status Message plugin for Miranda IM
		-===========================================-


Description
-----------
Simple Status Message provides a simple way to set status and away messages in Miranda IM.
The plugin was previously named SimpleAway, but has been renamed to better reflect its purpose.
The SimpleAway plugin was originally created by Harven.

Main features:
- Global and per-protocol status messages.
- Startup status and status message (per-protocol configurable).
- Per-contact status messages using contact-related variables for ICQ protocol (supported by older IM clients only).
- Unicode status messages support.
- Predefined status messages.
- Can remember up to 25 recent status messages which you can choose in the status message dialog.
- The status message dialog can be accessed through a hotkey, the status bar menu, Modern Contact List Toolbar,
  TopToolbar plugin frame or could pop up on status change.
- Built-in variables (see below) and support for Variables plugin.
- Can update variables in status messages every specified time.

Built-in variables list:
  %winampsong% - title of the song currently playing in Winamp (or another player with Winamp API Emulator).
  %date% - current date.
  %time% - current time or the time of becoming idle.
  %rand(x,y)% - random number in a specified range. y must be greater than x and both arguments must be integers.
  %randmsg% - random status message from the history.
  %randdefmsg% - random predefined status message.
  %fortunemsg% - BSD Fortune message (requires FortuneAwayMsg plugin).
  %protofortunemsg% - BSD Fortune message for a protocol (requires FortuneAwayMsg plugin).
  %statusfortunemsg% - BSD Fortune status message for a status (requires FortuneAwayMsg plugin).

Miranda IM 0.9 or later is required.

Latest development version, source code and older releases can be found here:
http://code.google.com/p/dezeath


Installation
------------
Copy simplestatusmsg.dll to your Miranda IM plugins directory.
If you are updating from SimpleAway, make sure you removed simpleaway.dll.


Changelog
---------
See simplestatusmsg-changelog.txt.


========================
Base Address: 0x3ab00000
========================


Copyright and License
---------------------

Copyright (C) 2006-2011 Bartosz 'Dezeath' Bia³ek
mailto: dezred(at)gmail(dot)com
http://code.google.com/p/dezeath

Copyright (C) 2005 Mateusz 'Harven' Kwaœniewski
mailto: harven(at)users(dot)berlios(dot)de
http://developer.berlios.de/projects/mgoodies/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

