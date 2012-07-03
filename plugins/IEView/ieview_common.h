/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

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

#ifndef IEVIEW_COMMON_H
#define IEVIEW_COMMON_H

#define MIRANDA_VER 0x0A00

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <process.h>
#include <shlguid.h>
#include <oleauto.h>
#include <mshtml.h>
#include <io.h>
#include <fcntl.h>

#include <newpluginapi.h>
#include <m_utils.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_system.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <m_database.h>
#include <m_skin.h>
#include <m_message.h>
#include <m_contacts.h>
#include <win2k.h>
#include "m_avatars.h"

#include "m_smileyadd.h"
#include "m_ieview.h"

#include "IEView.h"
#include "Utils.h"

extern HINSTANCE hInstance;
extern IEView *debugView;
extern char *workingDirUtf8;
extern char *ieviewModuleName;
extern HANDLE hHookOptionsChanged;

#endif
