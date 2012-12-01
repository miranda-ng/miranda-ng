/* 
Copyright (C) 2006-2010 Ricardo Pescuma Domenecci

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

#define OEMRESOURCE
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>
#include <richedit.h>
#include <tom.h>
#include <richole.h>
#include <commctrl.h>

#include <map>
#include <vector>
#include <string>
using namespace std;


// Miranda headers
#define MIRANDA_VER 0x0A00
#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_popup.h>
#include <m_history.h>
#include <m_message.h>
#include <m_icolib.h>
#include <m_hotkeys.h>
#include <win2k.h>

//own includes
#include "m_folders.h"
#include "m_metacontacts.h"
#include "m_variables.h"
#include "m_userinfoex.h"

#include "../utils/mir_memory.h"
#include "../utils/mir_options.h"
#include "../utils/tstring.h"
#include "../utils/utf8_helpers.h"
#include "../utils/scope.h"

#include "resource.h"
#include "Version.h"
#include "m_spellchecker.h"
#include "options.h"
#include "autoreplace.h"
#include "dictionary.h"
#include "ardialog.h"
#include "RichEdit.h"


#define MODULE_NAME		"SpellChecker"

#define FLAGS_DLL_FOLDER _T("%miranda_path%\\Icons")
#define CUSTOM_DICTIONARIES_FOLDER _T("%miranda_userdata%\\Dictionaries")
#define DICTIONARIES_FOLDER _T("%miranda_path%\\Dictionaries")

// Global Variables
extern HINSTANCE hInst;
extern BOOL uinfoex_enabled;
extern BOOL variables_enabled;


#define FREE(_m_)		if (_m_ != NULL) { free(_m_); _m_ = NULL; }


#define ICON_SIZE 16


extern LIST<Dictionary> languages;

struct WrongWordPopupMenuData 
{
	Suggestions suggestions;
	TCHAR *word;
	CHARRANGE pos;
	HMENU hMeSubMenu;
	HMENU hCorrectSubMenu;
	HMENU hReplaceSubMenu;
};

struct Dialog 
{
	HWND hwnd;
	HWND hwnd_owner;
	HANDLE hContact;
	char name[64];
	Dictionary *lang;
	TCHAR lang_name[32];
	WNDPROC old_edit_proc;
	WNDPROC owner_old_edit_proc;
	BOOL enabled;
	BOOL srmm;

	RichEdit *re;

	HWND hwnd_menu_owner;
	WNDPROC old_menu_proc;

	BOOL changed;
	BOOL markedSomeWord;
	int old_text_len;

	// Popup data
	HMENU hLanguageSubMenu;
	HMENU hWrongWordsSubMenu;
	vector<WrongWordPopupMenuData> *wrong_words;
};

BOOL CenterParent(HWND hwnd);
BOOL CreatePath(const TCHAR *path);
TCHAR *lstrtrim(TCHAR *str);
BOOL lstreq(TCHAR *a, TCHAR *b, size_t len = -1);
BOOL IsNumber(TCHAR c);

#endif // __COMMONS_H__
