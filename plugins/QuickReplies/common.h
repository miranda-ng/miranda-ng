/* 
Copyright (C) 2010 Unsane

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


#ifndef __QUICK_REPLY_H__
#define __QUICK_REPLY_H__

#define MIRANDA_VER 0x0900
#define MIRANDA_CUSTOM_LP

#include <windows.h>
#include <vector>
#include <string>

#include "tchar.h"

#include <newpluginapi.h>
#include <m_database.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_options.h>

#include "m_msg_buttonsbar.h"
#include "m_variables.h"

#include "resource.h"
#include "version.h"

#define MODULE_NAME				__INTERNAL_NAME
#define TEXT_LIMIT				2048
#define IDC_MESSAGE				1002
#define IDC_CHATMESSAGE			1009

#define MS_QUICKREPLIES_SERVICE	MODULE_NAME"/Service"

extern HINSTANCE hInstance;

extern BYTE iNumber;

extern HANDLE hOnOptInitialized;
extern HANDLE hOnButtonPressed;
extern HANDLE hQuickRepliesService;
extern HANDLE hOnModulesLoaded;
extern HANDLE hOnPreShutdown;

#ifdef _UNICODE
	typedef std::wstring tString;
#else
	typedef std::string tString;
#endif //_UNICODE

INT_PTR QuickRepliesService(WPARAM wParam, LPARAM lParam);

INT_PTR OnModulesLoaded(WPARAM wParam, LPARAM lParam);
INT_PTR OnOptInitialized(WPARAM wParam, LPARAM lParam);
INT_PTR OnButtonPressed(WPARAM wParam, LPARAM lParam);
INT_PTR OnPreShutdown(WPARAM wParam, LPARAM lParam);

#endif //__QUICK_REPLY_H__