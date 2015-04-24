/*

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

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include <commctrl.h>
#include <commdlg.h>
#include <winioctl.h>
#include <Uxtheme.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_options.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_protosvc.h>
#include <m_message.h>
#include <m_xstatus.h>
#include <m_clc.h>
#include <m_skin.h>
#include <win2k.h>
#include <m_extraicons.h>

#include <m_kbdnotify.h>
#include <m_metacontacts.h>

#include "flash.h"
#include "ignore.h"
#include "keyboard.h"
#include "constants.h"
#include "protolist.h"
#include "EnumProc.h"
#include "utils.h"
#include "keypresses.h"
#include "resource.h"
#include "Version.h"
