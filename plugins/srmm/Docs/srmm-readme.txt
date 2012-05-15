
       SRMM Plugin for Miranda IM
       __________________________


About
-----
This Miranda IM plugin allows you to send and receive instant messages.


Installation
------------
Simply copy "srmm.dll" into your Miranda IM plugin directory (usually
"C:\Program Files\Miranda IM\Plugins\") and restart Miranda IM.


Changes
-------
2.6.0.0
 - Improved RTL support
 - Removed offline from autopopup options

2.5.0.0
 - RTL support added
 - Updated icons

2.4.0.0
 - Message API fix (MS_MSG_GETWINDOWDATA)

2.3.0.0
 - Drag/drop didn't always work from message control

2.2.0.0
 - Log window didn't scroll correctly when avatar was loaded
 - Added show seconds option to log window timestamp
 - Drag and drop file transfer support
 - Updated internal API to 0.0.0.3
 - Fix some possible buffer overflow issues
 - Hide typing notification when user goes offline
 - Auto popup option is now set per status

2.1.0.0
 - Show avatars for protocols that support them
 - Show user menu by clicking user name
 - Copy user name by shift clicking username
 - Log window is drawn using XP theme if possible

2.0.0.0
 - Remove send to multiple "feature"
 - Remove single message mode
 - Remove file and url events from message log
 - Cache database reads when showing messages in the log
 - Show separate incoming/outgoing icons for messages with "Show Icons" option
 - Show status changes in the message window
 - Message area supports Ctrl+Up/Down to scroll through past messages
 - Close message dialog with Ctrl+w
 - Fixed some focus/flash issues
 - Added option to delete temporary contacts on window close
 - Support window api v0.0.0.1
 - Retry dialog caused crash

1.0.4.2
 - Typing Notify options didn't enable/disable correctly

1.0.4.1
 - Really fix new line issue

1.0.4.0
 - Minor log changes and url detection cleanup

1.0.3.0
 - Added option to see user is typing notifications with no window open
 - Send to multiple list respects the contact list "Hide Empty Groups" setting
 - Send to multiple list in single mode was cut off
 - Minor new line issue

1.0.2.0
 - Possible memory corruption

1.0.1.0
 - Initial release

Todo:
 - Font size fixes (muldiv, http://msdn.microsoft.com/library/default.asp?url=/library/en-us/gdi/fontext_1wmq.asp)
 - Cache fonts
 - Add "old" event fonts


Thanks
------
Thanks to Matrix and Valkyre who designed and created the icons.


Support and bug reporting
-------------------------
We cannot give support on e-mail or ICQ. Please visit the Miranda IM help page at
http://www.miranda-im.org/help/ if you need help with this plugin.

If the help page does answer your question, visit the Miranda IM support forum at:
http://forums.miranda-im.org and we will try to assist you.

If you want to report a bug, please do so in the official bugtracker at:
http://bugs.miranda-im.org/



Contact
-------
Current maintainer is Robert Rainwater.
E-mail: rainwater at miranda-im.org



License and Copyright
---------------------
Copyright (C) 2001-2005 Robert Rainwater, Martin Öberg, Richard Hughes

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
