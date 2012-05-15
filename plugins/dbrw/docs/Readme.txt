**************************************************************
** dbRW
**
** Miranda IM database driver powered by SQLite
**************************************************************

About
-----
dbRW is a SQLite powered database driver plugin for Miranda IM.  It implements
the same database API as the standard Miranda database plugin.  Therefore, it
should be compatible with all current plugins.


Changes
-------
1.0:
 - Initial release

1.1
 - Safer event deletion notification
 - Minor performance improvements

1.2
 - Automatic compaction dialog is now threaded
 - Database file is now locked while in use
 - Performance improvements
 - Update SQLite library to v3.3.17

1.3
 - Event cache table initialized improperly
 - Performance improvements
 

Author
------
Robert rainwater <rainwater@miranda-im.org>


Copyright
---------
Copyright (c) 2005-2011 Robert Rainwater
Portions of dbRW are Copyright (c) 2005 Sam Kothari

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
