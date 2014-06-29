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

#define _CRT_SECURE_NO_WARNINGS
#define OEMRESOURCE

#include <windows.h>
#include <richedit.h>
#include <tom.h>
#include <richole.h>
#include <commctrl.h>
#include <map>
#include <vector>
#include <string>
using namespace std;

#include <newpluginapi.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_message.h>
#include <m_icolib.h>
#include <m_hotkeys.h>
#include <win2k.h>
#include <m_system_cpp.h>

//own includes
#include <m_folders.h>
#include <m_metacontacts.h>
#include <m_variables.h>
#include <m_userinfoex.h>
#include <m_spellchecker.h>

#include <../utils/mir_options.h>
//#include <../utils/tstring.h>
#include <../utils/utf8_helpers.h>
#include <../utils/scope.h>

#include "hunspell/affentry.hxx"
#include "hunspell/config.h"
#include "hunspell/hunspell.hxx"
#include "hunspell/csutil.hxx"
#include "hunspell/affixmgr.hxx"
#include "hunspell/langnum.hxx"
#include "hunspell/atypes.hxx"
#include "hunspell/dictmgr.hxx"
#include "hunspell/filemgr.hxx"
#include "hunspell/hashmgr.hxx"
#include "hunspell/hunspell.h"
#include "hunspell/hunzip.hxx"
#include "hunspell/phonet.hxx"
#include "hunspell/replist.hxx"
#include "hunspell/suggestmgr.hxx"

#include "resource.h"
#include "Version.h"
#include "options.h"
#include "autoreplace.h"
#include "dictionary.h"
#include "ardialog.h"
#include "RichEdit.h"

#define MODULE_NAME		"SpellChecker"

#define FLAGS_DLL_FOLDER _T("%miranda_path%\\Icons")
#define CUSTOM_DICTIONARIES_FOLDER _T("%miranda_userdata%\\Dictionaries")
#define DICTIONARIES_FOLDER _T("%miranda_path%\\Plugins\\dictionaries")

// Global Variables
extern HINSTANCE hInst;
extern BOOL uinfoex_enabled;
extern BOOL variables_enabled;

#define FREE(_m_)		if (_m_ != NULL) { free(_m_); _m_ = NULL; }

#define ICON_SIZE 16

#define TIMER_ID 17982
#define WMU_DICT_CHANGED (WM_USER+100)
#define WMU_KBDL_CHANGED (WM_USER+101)

#define HOTKEY_ACTION_TOGGLE 1

extern LIST<Dictionary> languages;
extern BITMAP bmpChecked;
extern HBITMAP hCheckedBmp;

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
	MCONTACT hContact;
	char name[64];
	Dictionary *lang;
	TCHAR lang_name[32];
	BOOL enabled;
	BOOL srmm;

	RichEdit *re;

	HWND hwnd_menu_owner;

	BOOL changed;
	BOOL markedSomeWord;
	int old_text_len;

	// Popup data
	HMENU hLanguageSubMenu;
	HMENU hWrongWordsSubMenu;
	vector<WrongWordPopupMenuData> *wrong_words;
};

BOOL CenterParent(HWND hwnd);
TCHAR *lstrtrim(TCHAR *str);
BOOL lstreq(TCHAR *a, TCHAR *b, size_t len = -1);
BOOL IsNumber(TCHAR c);

int MsgWindowEvent(WPARAM wParam, LPARAM lParam);
int MsgWindowPopup(WPARAM wParam, LPARAM lParam);
int IconPressed(WPARAM wParam, LPARAM lParam);

int AddContactTextBox(MCONTACT hContact, HWND hwnd, char *name, BOOL srmm, HWND hwndOwner);
int RemoveContactTextBox(HWND hwnd);
int ShowPopupMenu(HWND hwnd, HMENU hMenu, POINT pt, HWND hwndOwner);

INT_PTR AddContactTextBoxService(WPARAM wParam, LPARAM lParam);
INT_PTR RemoveContactTextBoxService(WPARAM wParam, LPARAM lParam);
INT_PTR ShowPopupMenuService(WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MenuWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ModifyIcon(Dialog *dlg);
BOOL GetWordCharRange(Dialog *dlg, CHARRANGE &sel, TCHAR *text, size_t text_len, int &first_char);
TCHAR *GetWordUnderPoint(Dialog *dlg, POINT pt, CHARRANGE &sel);

int GetClosestLanguage(TCHAR *lang_name);

#endif // __COMMONS_H__
