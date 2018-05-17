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
#include <m_clistint.h>
#include <m_gui.h>

#include <m_stopspam.h>
#include <m_variables.h>

typedef std::wstring tstring;
#define PREF_TCHAR2 PREF_UTF

#include "version.h"
#include "resource.h"
#include "settings.h"

#define pluginName LPGEN("StopSpam")

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin() :
		PLUGIN<CMPlugin>(pluginName)
	{}
};

extern char const *answeredSetting;

// utils
tstring &GetDlgItemString(HWND hwnd, int id);
bool IsExistMyMessage(MCONTACT hContact);
tstring variables_parse(const wchar_t *tstrFormat, MCONTACT hContact);
tstring trim(tstring const &tstr, tstring const &trimChars = L" \f\n\r\t\v");

INT_PTR IsContactPassed(WPARAM wParam, LPARAM /*lParam*/);
INT_PTR RemoveTempContacts(WPARAM wParam,LPARAM lParam);
int OnSystemModulesLoaded(WPARAM wParam, LPARAM lParam);
int OnDbEventAdded(WPARAM wParam, LPARAM lParam);
int OnDbEventFilterAdd(WPARAM w, LPARAM l);
int OnOptInit(WPARAM w, LPARAM l);
int OnDbContactSettingchanged(WPARAM hContact, LPARAM l);
#endif
