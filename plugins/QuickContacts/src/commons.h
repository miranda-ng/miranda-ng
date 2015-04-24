/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci
Based on work (C) Heiko Schillinger

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

#include <windows.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_message.h>
#include <m_userinfo.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_button.h>
#include <m_file.h>
#include <m_url.h>
#include <m_history.h>
#include <m_icolib.h>
#include <m_hotkeys.h>
#include <win2k.h>

#include <m_metacontacts.h>
#include <m_quickcontacts.h>

#include "../utils/mir_options.h"

#include "resource.h"
#include "options.h"
#include "Version.h"

#define MODULE_NAME		"QuickContacts"


// Global Variables
extern HINSTANCE hInst;

// Copied from "../modernb/clc.h" ///////////////////////////////////////////////////////////////////

//add a new hotkey so it has a default and can be changed in the options dialog
//wParam=0
//lParam=(LPARAM)(SKINHOTKEYDESC*)ssd;
//returns 0 on success, nonzero otherwise

typedef struct {
	int cbSize;
	const char *pszName;		   //name to refer to sound when playing and in db
	const char *pszDescription;	   //description for options dialog
    const char *pszSection;        //section name used to group sounds (NULL is acceptable)
	const char *pszService;        //Service to call when HotKey Pressed

	int DefHotKey; //default hot key for action
} SKINHOTKEYDESCEX;

#define MS_SKIN_ADDHOTKEY      "Skin/HotKeys/AddNew"
#define MS_SKIN_PLAYHOTKEY		"Skin/HotKeys/Run"



#endif // __COMMONS_H__
