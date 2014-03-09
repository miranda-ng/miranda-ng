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

#include <windows.h>

#include <newpluginapi.h>
#include <m_utils.h>
#include <m_string.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_options.h>

#include <m_msg_buttonsbar.h>
#include <m_variables.h>

#include "version.h"
#include "resource.h"

#define MODULE "QuickReplies"
#define IDC_MESSAGE 1002
#define IDC_CHATMESSAGE 1009

#define MS_QUICKREPLIES_SERVICE MODULE"/Service"

extern HINSTANCE hInstance;

extern BYTE iNumber;

extern HANDLE hOnOptInitialized;
extern HANDLE hOnButtonPressed;
extern HANDLE hQuickRepliesService;
extern HANDLE hOnModulesLoaded;
extern HANDLE hOnPreShutdown;

INT_PTR QuickRepliesService(WPARAM wParam, LPARAM lParam);

int OnModulesLoaded(WPARAM wParam, LPARAM lParam);
int OnOptInitialized(WPARAM wParam, LPARAM lParam);
int OnButtonPressed(WPARAM wParam, LPARAM lParam);
int OnPreShutdown(WPARAM wParam, LPARAM lParam);

#endif //__QUICK_REPLY_H__