/*

'Language Pack Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (LangMan-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define _CRT_SECURE_NO_WARNINGS

#define __RPCASYNC_H__        /* header shows warnings in VS6 */
#include <windows.h>
#pragma warning(disable:4201) /* nonstandard extension used : nameless struct/union */
#include <commctrl.h>
#pragma warning(default:4201) /* nonstandard extension used : nameless struct/union */
#include <win2k.h>

#include <tchar.h>
#include <stdio.h>            /* for mir_snprintf() */
#include <time.h>             /* for time() */

#define MIRANDA_VER  0x0600
#include <newpluginapi.h>
#include <m_system.h>
#include <m_utils.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_flags.h>
#include <m_netlib.h>
#include <m_clist.h>

#include "langpack.h"
#include "options.h"
//#include "update.h"
//#include "unzip.h"
#include "utils.h"
#include "resource.h"
#include "m_langman.h"
