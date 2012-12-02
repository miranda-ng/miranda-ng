/*

Simple Status Message plugin for Miranda IM
Copyright (C) 2006-2011 Bartosz 'Dezeath' Bia³ek, (C) 2005 Harven

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
*/

// to enable all 0.9.0 core functions
#define MIRANDA_VER 0x0A00
// to enable custom langpacks
#define MIRANDA_CUSTOM_LP

#define _WIN32_IE 0x0501

#define _CRT_SECURE_NO_WARNINGS

#include <m_stdhdr.h>

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_button.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_skin.h>
#include <m_system_cpp.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_database.h>
#include <m_awaymsg.h>
#include <m_idle.h>
#include <m_icolib.h>
#include <m_protoint.h>
#include <m_hotkeys.h>
#include <m_icq.h>
#include <win2k.h>

#include "../../protocols/IcqOscarJ/src/icq_constants.h"
#include "m_fortunemsg.h"
#include "m_statusplugins.h"
#include "m_toptoolbar.h"
#include "m_variables.h"
#include "m_simpleaway.h"
#include "m_simplestatusmsg.h"

#include "resource.h"
