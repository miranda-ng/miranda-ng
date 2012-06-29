{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
}

{$IFNDEF STATUSMODES}
{$DEFINE STATUSMODES}

const
  // add 1 to the ID_STATUS_CONNECTING to mark retries (v0.1.0.1+)
  // e.g. ID_STATUS_CONNECTING+2 is the third connection attempt, or the second retry
  ID_STATUS_CONNECTING = 1;

  // max retries is just a marker, so that the clist knows what
  // numbers represent retries, it should set any kind of limit on the number
  // of retries you can and/or should do
  MAX_CONNECT_RETRIES = 10000;

  // and the modes!

  ID_STATUS_OFFLINE    = 40071;
  ID_STATUS_ONLINE     = 40072;
  ID_STATUS_AWAY       = 40073;
  ID_STATUS_DND        = 40074;
  ID_STATUS_NA         = 40075;
  ID_STATUS_OCCUPIED   = 40076;
  ID_STATUS_FREECHAT   = 40077;
  ID_STATUS_INVISIBLE  = 40078;
  ID_STATUS_ONTHEPHONE = 40079;
  ID_STATUS_OUTTOLUNCH = 40080;
  ID_STATUS_IDLE       = 40081; // do not use as a status

  MAX_STATUS_COUNT = (ID_STATUS_OUTTOLUNCH-ID_STATUS_OFFLINE+1);

{$ENDIF}
