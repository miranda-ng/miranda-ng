Protocol for the Miranda IM for communicating with users of
the MSN Messenger protocol.

Copyright (C) 2003-5 George Hazan (ghazan@postman.ru)
Copyright (C) 2001-3 Richard Hughes (original version),

Miranda IM: the free icq client for MS Windows
Copyright (C) 2002-5  Martin Oberg, Robert Rainwater, Sam Kothari, Lyon Lim
Copyright (C) 2000-2  Richard Hughes, Roland Rabien & Tristan Van de Vreede

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

================================================================================
Useful plugins which can help you to work with the MSN protocol
================================================================================

Popup 1.0.1.9
(http://miranda-im.org/download/details.php?action=viewfile&id=299)
Popup Plus 2.0.3.8
(http://miranda-im.org/download/details.php?action=viewfile&id=1170)
----------------------
All notifications in the MSN plugin are made using the Popup
plugin. After the Popup plugin installation there will be some
options available for tuning, you will find them in
Options/Popups/MSN tab.

Chat 0.2.0.2
(http://miranda-im.org/download/details.php?action=viewfile&id=1309)
----------------------
This plugin allows you to support group chats with many MSN users. Without
that plugin you can't leave a chat been being invited, so if you plan to
use groupchats, this plugin is essential.

SRMM (Unicode) 1.0.4.3
(http://miranda-im.org/download/details.php?action=viewfile&id=1136)
tabSRMM (Unicode) 0.9.9.95
(http://hell.at.eu.org/forums)
----------------------
They both are true Unicode-aware message editors. Using them you can
send/receive Unicode messages, so you won't be dependent on the
codepages, encodings etc. They work only under NT4/Win2k/XP/2003.

Unicode History Viewer
(http://miranda-im.org/download/details.php?action=viewfile&id=1109)
----------------------
If you work under NT4/Win2k/XP/2003, and you can send/receive messages
in Unicode, the standard history viewer will show question-marks when
Unicode characters cannot be mapped to the current codepage. In this
case you can use this plugin to have no problems at all.

================================================================================
Frequently asked questions.
================================================================================

Q1. I want to use the OpenSSL libraries, but plugin can't find or load
them. Where can I find the valid DLLs for Win32?

A1. http://www.slproweb.com/products/Win32OpenSSL.html
Use the latest stable version.

----------------------------------------------------------------

Q2. I get strange errors with connection, proxies, file sending/
receiving. How can I find the reason of the problem?

A2. Some useful information is grouped in the networking-msn.txt.
If it does not help, try to create a network log. Go to
Options/Network and press a "Log Options" button (the only button
in the upper-right corner). Very often this log helps users to
find a hidden diagnostic messages, or simply understand what is
happening concretely. If a problem persists, you can contact me,
and upon request, send me (zipped!) a piece of this log.

----------------------------------------------------------------

Q3. My Miranda logged into the MSN network successfully, but I
cannot send a message, and a contact's menu item 'Message' is blocked.

A3. The problem is in the Conversation Style Messaging plugin.
CSM plugin is incompatible with the MSN plugin because of the
error in it. Disable it and/or delete, if you want to use the MSN
protocol

----------------------------------------------------------------

Q4. I want to compile your sources, but got a lot of errors.

A4. First, you should download new Miranda's SDK sources, and
place the MSN plugin's sources into Protocol\MSN subdirectory
(accordingly to the Miranda's directory tree). Notice that MSN is
not a plugin anymore, it's stored among another protocols. Look at
http://cvs.sourceforge.net/cgi-bin/viewcvs.cgi/miranda-icq to
view the complete directory structure.

Then download the Popup plugin, and unzip header file m_popup.h
to the SDK\headers_c directory. Then you should be able to compile
MSN plugin without problems.

Installation of Microsoft Platform SDK is not required to
compile MSN plugin sources

----------------------------------------------------------------

Q5. When I receive an authorization request, my own nickname is
shown in the contact list. Is it a bug?

A5. Yes, it is, but it cannot be solved now. Miranda uses UINs
to identify a contact, but because MSN protocol have no UINs,
your own nickname (as a default one) is shown.

----------------------------------------------------------------

WMBR, George Hazan (ghazan@postman.ru).
