#ifndef _stopspam_headers_h
#define _stopspam_headers_h

// disable security warnings about "*_s" functions
#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <commctrl.h>
#include <sstream>
#include <list>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include <m_skin.h>
#include <m_clist.h>

#include <m_stopspam.h>
#include <m_variables.h>


typedef std::wstring tstring;
#define PREF_TCHAR2 PREF_UTF

#include "eventhooker.h"
#include "version.h"
#include "resource.h"
#include "settings.h"

#define pluginName LPGEN("StopSpam")

extern TCHAR * pluginDescription;
extern TCHAR const * infTalkProtPrefix;
extern char const * answeredSetting;
extern char const * questCountSetting;
extern HANDLE hLoadHook;
extern HINSTANCE hInst;

//options
INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MessagesDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProtoDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//utils
void SetDlgItemString(HWND hwndDlg, UINT idItem, std::string const &str);
void SetDlgItemString(HWND hwndDlg, UINT idItem, std::wstring const &str);
tstring &GetDlgItemString(HWND hwnd, int id);
bool IsExistMyMessage(MCONTACT hContact);
tstring variables_parse(tstring const &tstrFormat, MCONTACT hContact);
tstring trim(tstring const &tstr, tstring const &trimChars = _T(" \f\n\r\t\v"));

INT_PTR IsContactPassed(WPARAM wParam, LPARAM /*lParam*/);
INT_PTR RemoveTempContacts(WPARAM wParam,LPARAM lParam);
int OnSystemModulesLoaded(WPARAM wParam, LPARAM lParam);
#endif
