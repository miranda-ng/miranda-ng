/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (ñ) 2000-03 Miranda ICQ/IM project,
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

#define _CRT_SECURE_NO_DEPRECATE


#include <tchar.h>
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <shlobj.h>
#include <commctrl.h>
#include <stdio.h>

#include <dwmapi.h>
#include <propkey.h>
#include <propvarutil.h>

#define MIRANDA_VER	0x0A00
#include <win2k.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_clist.h>
#include <m_clc.h>
#include <m_clui.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_userinfo.h>
#include <m_history.h>
#include <m_addcontact.h>
#include <m_message.h>
#include <m_file.h>
#include <m_icolib.h>
#include <m_idle.h>
#include <m_awaymsg.h>
#include <m_system_cpp.h>
#include <m_avatars.h>
#include <m_clistint.h>
#include <m_protoint.h>
#include "m_w7ui.h"

#include "win7api.h"

#include "jumplistarray.h"
#include "jumplistbuilder.h"
#include "jumplist.h"

#include "subclassmgr.h"
#include "dwmwindow.h"
#include "clistproxywindow.h"
#include "srmmproxywindow.h"

extern HINSTANCE g_hInst;
