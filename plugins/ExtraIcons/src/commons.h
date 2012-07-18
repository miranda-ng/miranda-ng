/*
 Copyright (C) 2009 Ricardo Pescuma Domenecci

 This is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this file; see the file license.txt.  If
 not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.
 */

#ifndef __COMMONS_H__
# define __COMMONS_H__

#define _CRT_SECURE_NO_WARNINGS

#ifdef UNICODE
#error "Unicode not needed by this plugin"
#endif

#define _WIN32_IE 0x500
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>
#include <commctrl.h>

#include <map>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

// Miranda headers
#define MIRANDA_VER 0x0A00

#include <newpluginapi.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_clui.h>
#include <m_clist.h>
#include <m_cluiframes.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_metacontacts.h>
#include <m_icolib.h>
#include <m_skin.h>
#include <m_userinfo.h>

#include "..\utils\mir_icons.h"

#include "resource.h"
#include "Version.h"
#include "m_extraicons.h"

#include "ExtraIcon.h"
#include "ExtraIconGroup.h"
#include "CallbackExtraIcon.h"
#include "IcolibExtraIcon.h"
#include "usedIcons.h"
#include "DefaultExtraIcons.h"
#include "options.h"

#define MODULE_NAME		"ExtraIcons"

// Global Variables
extern HINSTANCE hInst;

#define MAX_REGS(_A_)	( sizeof(_A_) / sizeof(_A_[0]))
#define FREE(_m_)		if (_m_ != NULL) { free(_m_); _m_ = NULL; }

#define ICON_SIZE 16

extern vector<BaseExtraIcon*> registeredExtraIcons;
extern vector<ExtraIcon*> extraIconsByHandle;
extern vector<ExtraIcon*> extraIconsBySlot;
void RebuildListsBasedOnGroups(vector<ExtraIconGroup *> &groups);
ExtraIcon * GetExtraIconBySlot(int slot);

int GetNumberOfSlots();
int ConvertToClistSlot(int slot);

int Clist_SetExtraIcon(HANDLE hContact, int slot, HANDLE hImage);

static inline BOOL IsEmpty(const char *str)
{
	return str == NULL || str[0] == 0;
}

static inline int MIN(int a, int b)
{
	if (a <= b)
		return a;
	return b;
}

static inline int MAX(int a, int b)
{
	if (a >= b)
		return a;
	return b;
}

#endif // __COMMONS_H__
