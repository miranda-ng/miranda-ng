/*

'AutoShutdown'-Plugin for Miranda IM

Copyright 2004-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Shutdown-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define NEEDED_MIRANDA_VERSION      PLUGIN_MAKE_VERSION(0,7,0,16)
#define NEEDED_MIRANDA_VERSION_STR  "0.7 alpha build #16"
#define PLUGIN_VERSION              PLUGIN_MAKE_VERSION(1,4,0,2)
#define FILE_VERSION                1,4,0,2

#if defined(_DEBUG)
 #define FILE_VERSION_STR           "1.4.0.3 alpha"
#else
 #define FILE_VERSION_STR           "1.4.0.2"
#endif

#define PLUGIN_EMAIL                "hrathh users.sourceforge.net"
#define PLUGIN_EMAIL_ATT_POS        7  /* position of the @-sign in the email adress above */

#if defined(_UNICODE)
 #define PLUGIN_WEBSITE             "http://addons.miranda-im.org/details.php?action=viewfile&id=3056"
#else
 #define PLUGIN_WEBSITE             "http://addons.miranda-im.org/details.php?action=viewfile&id=1086"
#endif
