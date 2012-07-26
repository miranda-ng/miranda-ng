/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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
*/

#define __MAJOR_VERSION				0
#define __MINOR_VERSION				8
#define __RELEASE_NUM				0
#define __BUILD_NUM					1

// other stuff for Version resource
#define __STRINGIFY_IMPL(x)			#x
#define __STRINGIFY(x)				__STRINGIFY_IMPL(x)

#define __FILEVERSION_STRING       __MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
#define __FILEVERSION_STRING_DOTS  __MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM
#define __VERSION_STRING           __STRINGIFY(__FILEVERSION_STRING_DOTS)

#define __PLUGIN_DESC        "ToptoolBar adds buttons in top frame for fast access."
#define __PLUGIN_LONGDESC    __PLUGIN_DESC
#define __PLUGIN_AUTHOR      "Bethoven"
#define __PLUGIN_AUTHORWEB   "http://miranda-ng.org/"
#define __PLUGIN_EMAIL       "Bethoven@mailgate.ru"
#define __PLUGIN_RIGHTS      "© 2003-2008 Bethoven"
#define __PLUGIN_FILE        "TopToolbar.dll"
#define __PLUGIN_SHORTNAME   "TopToolBar"

#define __PLUGIN_NAME        __PLUGIN_SHORTNAME
