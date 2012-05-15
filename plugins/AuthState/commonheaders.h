/*
   Authorization State plugin for Miranda-IM (www.miranda-im.org)
   (c) 2006-2009 by Thief

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   File name      : $URL: http://svn.miranda.im/mainrepo/authstate/trunk/commonheaders.h $
   Revision       : $Rev: 1686 $
   Last change on : $Date: 2010-10-06 08:26:58 +0200 (Ср, 06 окт 2010) $
   Last change by : $Author: ghazan $

*/

#ifndef COMMHEADERS_H
#define COMMHEADERS_H

#define MIRANDA_VER    0x0900
#define MIRANDA_CUSTOM_LP

#include <windows.h>
#include <stdio.h> // needed by m_utils.h
#include <stddef.h> // needed by m_icolib.h > r9256
#include <commctrl.h>

// Miranda API headers
#include <newpluginapi.h>
#include <m_database.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_utils.h>
#include <m_clist.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include <m_cluiframes.h>
#include "resource.h"
#include "Version.h"

// Third party SDK headers
#include "m_updater.h"
#include "m_extraicons.h"

static const int DefaultSlot = EXTRA_ICON_ADV2;
#define MODULENAME "AuthState"

extern int onOptInitialise(WPARAM wParam, LPARAM lParam);
extern int onExtraImageApplying(WPARAM wParam, LPARAM lParam);
extern int onExtraImageListRebuild(WPARAM wParam, LPARAM lParam);

extern HINSTANCE g_hInst;
extern IconExtraColumn g_IECAuth, g_IECGrant, g_IECAuthGrant, g_IECClear;
extern INT clistIcon;
extern byte bUseAuthIcon, bUseGrantIcon, bContactMenuItem, bIconsForRecentContacts;
extern HANDLE hExtraIcon;

#endif //COMMHEADERS_H
