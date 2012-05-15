#ifndef _stopspam_headers_h
#define _stopspam_headers_h

// disable security warnings about "*_s" functions
#define _CRT_SECURE_NO_DEPRECATE

// disable warnings about underscore in stdc functions
#pragma warning(disable: 4996)

#define MIRANDA_VER    0x0900
#define MIRANDA_CUSTOM_LP

#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <time.h>
#include <string>
#include <sstream>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include <m_skin.h>
#include <m_clist.h>

#include "src/eventhooker.h"
#include "res/version.h"
#include "res/resource.h"
#include "m_stopspam.h"

#include "m_variables.h"

#define pluginName "StopSpam"

extern char * pluginDescription;
extern TCHAR const * infTalkProtPrefix;
extern char const * answeredSetting;
extern char const * questCountSetting;

extern HINSTANCE hInst;

#ifdef _UNICODE
	typedef std::wstring tstring;
	#define PREF_TCHAR2 PREF_UTF
#else
	typedef std::string tstring;
	#define PREF_TCHAR2 0
#endif //_UNICODE

#include "src/settings.h"

//options
INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MessagesDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProtoDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//utils
void SetDlgItemString(HWND hwndDlg, UINT idItem, std::string const &str);
void SetDlgItemString(HWND hwndDlg, UINT idItem, std::wstring const &str);
tstring &GetDlgItemString(HWND hwnd, int id);
bool IsExistMyMessage(HANDLE hContact);
tstring variables_parse(tstring const &tstrFormat, HANDLE hContact);
tstring trim(tstring const &tstr, tstring const &trimChars = _T(" \f\n\r\t\v"));

INT_PTR IsContactPassed(WPARAM wParam, LPARAM /*lParam*/);
INT_PTR RemoveTempContacts(WPARAM wParam,LPARAM lParam);
int OnSystemModulesLoaded(WPARAM wParam, LPARAM lParam);
#endif
