/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#pragma once

/***********************************************************************************************************
 * some compiler definitions
 ***********************************************************************************************************/

#define _WIN32_WINNT	0x0501
#define _WIN32_IE			0x0500
#define WIN32_LEAN_AND_MEAN

#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 0
#define _CRT_SECURE_NO_WARNINGS

/***********************************************************************************************************
 * standard windows includes
 ***********************************************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shlwapi.h>
#include <Richedit.h>
#include <dlgs.h>
#include <assert.h>
#include <UxTheme.h>
#include <string>

using namespace std;

/***********************************************************************************************************
 * Miranda NG SDK includes and macros
 ***********************************************************************************************************/

#include <newpluginapi.h>
#include <m_button.h>
#include <m_clui.h>
#include <m_clistint.h>
#include <m_database.h>
#include <m_hotkeys.h>
#include <m_langpack.h>
#include <m_protomod.h>
#include <m_options.h>
#include <m_xml.h>
#include <m_timezones.h>
#include <m_imgsrvc.h>
#include <m_message.h>
#include <m_userinfo.h>
#include <m_icq.h>
#include <m_email.h>
#include <win2k.h>
#include <msapi/vsstyle.h>
#include <msapi/vssym32.h>
#include <m_skin.h>
#include <m_extraicons.h>
#include <m_avatars.h>
#include <m_contacts.h>
#include <m_db3xSA.h>
#include <m_icolib.h>
#include <m_string.h>
#include <m_popup.h>

/***********************************************************************************************************
 * Used Plugins SDK includes and macros
 ***********************************************************************************************************/

#include <m_flags.h>
#include <m_metacontacts.h>
#include <m_toptoolbar.h>
#include <m_userinfoex.h>

/***********************************************************************************************************
 * UserInfoEx plugin includes and macros
 ***********************************************************************************************************/

#pragma intrinsic(memcmp, memcpy, memset, strcmp, strlen)

#ifndef MIR_OK
#define MIR_OK		0		// success value of a miranda service function
#define MIR_FAIL	1		// general failure value of a miranda service function
#endif

#define MIRSUCCEEDED(f)		((f)==MIR_OK)
#define MIRFAILED(f)		((f)!=MIR_OK)
#define MIREXISTS(f)		((INT_PTR)(f)!=CALLSERVICE_NOTFOUND)

#define PtrIsValid(p)		(((p)!=0)&&(((HANDLE)(p))!=INVALID_HANDLE_VALUE))
#define FREE(p)				{if (PtrIsValid(p)){free((void*)p);(p)=NULL;}}
#define MIR_DELETE(p)		{LPVOID ptr = (LPVOID)(p);if (PtrIsValid(ptr)){delete(ptr);(ptr)=NULL;}}
#define MIR_FREE(p)			{if (PtrIsValid(p)){mir_free((void*)p);(p)=NULL;}}

#define GetUserData(p)		GetWindowLongPtr((p), GWLP_USERDATA)
#define SetUserData(p, l)	SetWindowLongPtr((p), GWLP_USERDATA, (LONG_PTR) (l))

unsigned int hashSetting_M2(const wchar_t * key);	//new Murma2 hash
unsigned int hashSetting_M2(const char * key);		//new Murma2 hash
unsigned int hashSettingW_M2(const char * key);		//new Murma2 hash

#include "resource.h"
#include "version.h"
#include "../IconPacks/default/src/icons.h"
#include "../IconPacks/ice/src/icons.h"
#include "svc_constants.h"
#include "svc_avatar.h"
#include "svc_contactinfo.h"
#include "svc_email.h"
#include "svc_gender.h"
#include "svc_homepage.h"
#include "svc_phone.h"
#include "svc_refreshci.h"
#include "svc_reminder.h"
#include "svc_timezone.h"
#include "svc_timezone_old.h"
#include "mir_contactqueue.h"
#include "mir_db.h"
#include "mir_string.h"
#include "mir_icolib.h"
#include "mir_menuitems.h"
#include "classMTime.h"
#include "classMAnnivDate.h"
#include "ctrl_base.h"
#include "ctrl_button.h"
#include "ctrl_contact.h"
#include "ctrl_annivedit.h"
#include "ctrl_combo.h"
#include "ctrl_edit.h"
#include "ctrl_tzcombo.h"
#include "dlg_msgbox.h"
#include "dlg_propsheet.h"
#include "dlg_anniversarylist.h"
#include "psp_base.h"
#include "psp_options.h"
#include "ex_import\svc_ExImport.h"
#include "ex_import\tinystr.h"
#include "ex_import\tinyxml.h"
#include "ex_import\mir_rfcCodecs.h"
#include "ex_import\classExImContactBase.h"
#include "ex_import\dlg_ExImProgress.h"
#include "ex_import\svc_ExImXML.h"
#include "ex_import\classExImContactXML.h"
#include "ex_import\dlg_ExImModules.h"
#include "ex_import\dlg_ExImOpenSaveFile.h"
#include "ex_import\svc_ExImINI.h"
#include "ex_import\svc_ExImVCF.h"
#include "flags\svc_flags.h"
#include "Flags\svc_flagsicons.h"

/***********************************************************************************************************
 * UserInfoEx global variables
 ***********************************************************************************************************/

typedef struct _MGLOBAL
{
	DWORD		mirandaVersion;					// mirandaVersion
	BYTE		CanChangeDetails : 1;			// is service to upload own contact information for icq present?
	BYTE		TzIndexExist : 1;				// Win Reg has Timzone Index Info
	BYTE		PopupActionsExist : 1;			// Popup++ or MS_POPUP_REGISTERACTIONS exist
	BYTE		ShowPropsheetColours : 1;		// cached SET_PROPSHEET_SHOWCOLOURS database value
	BYTE		WantAeroAdaption : 1;			// reserved for later use
	BYTE		UseDbxTree : 1;					// use dbx_tree ?
} MGLOBAL, *LPMGLOBAL;

extern HINSTANCE		ghInst;
extern MGLOBAL			myGlobals;
extern FI_INTERFACE* FIP;
extern int nCountriesCount;
extern struct CountryListEntry *countries;

/***********************************************************************************************************
 * MIRANDA_CPP_PLUGIN_API
 ***********************************************************************************************************/

/**
 * These macros provide an interface for classes to use member
 * function as services and event hooks.
 *
 * @note	This requires Miranda Core 0.8+!
 *
 **/
#define MIRANDA_CPP_PLUGIN_API(CCoreClass) \
	typedef int (__cdecl CCoreClass::*EVENTHOOK)(WPARAM, LPARAM);	\
	typedef int (__cdecl CCoreClass::*EVENTHOOKPARAM)(WPARAM, LPARAM, LPARAM); \
	typedef int (__cdecl CCoreClass::*SERVICEFUNC)(WPARAM, LPARAM); \
	typedef int (__cdecl CCoreClass::*SERVICEFUNCPARAM)(WPARAM, LPARAM, LPARAM); \
	\
	HANDLE ThisHookEvent(const char* szEvent, EVENTHOOK pfnEvent) \
	{	return (HANDLE) ::HookEventObj(szEvent, (MIRANDAHOOKOBJ) (*(PVOID*) &pfnEvent), (PVOID)this);} \
	HANDLE ThisHookEventParam(const char* szEvent, EVENTHOOKPARAM pfnEvent, LPARAM lParam) \
	{	return (HANDLE) ::HookEventObjParam(szEvent, (MIRANDAHOOKOBJPARAM) (*(PVOID*) &pfnEvent), (PVOID)this, lParam);	} \
	\
	HANDLE ThisCreateService(const char* szService, SERVICEFUNC pfnService) \
	{	return (HANDLE) ::CreateServiceFunctionObj(szService, (MIRANDASERVICEOBJ) (*(PVOID*) &pfnService), (PVOID)this);	} \
	HANDLE ThisCreateServiceParam(const char* szService, SERVICEFUNCPARAM pfnService, LPARAM lParam) \
	{	return (HANDLE) ::CreateServiceFunctionObjParam(szService, (MIRANDASERVICEOBJPARAM) (*(PVOID*) &pfnService), (PVOID)this, lParam); } \

/***********************************************************************************************************
 * UserInfoEx common used functions
 ***********************************************************************************************************/

DWORD	hashSetting(LPCSTR szStr);					//old miranda hash

INT_PTR	myDestroyServiceFunction(const char * key);

static FORCEINLINE BOOL IsProtoOnline(LPSTR pszProto)
{
	return pszProto && pszProto[0] && CallProtoService(pszProto, PS_GETSTATUS, NULL, NULL) >= ID_STATUS_ONLINE;
}
static FORCEINLINE BOOL IsProtoLoaded(LPSTR pszProto)
{
	return (CallService(MS_PROTO_ISPROTOCOLLOADED, NULL, (LPARAM)pszProto) != NULL);
}
static FORCEINLINE BOOL IsProtoAccountEnabled(PROTOACCOUNT *pAcc)
{
	return (pAcc->bIsEnabled && IsProtoLoaded(pAcc->szModuleName));
}

typedef HRESULT (STDAPICALLTYPE *pfnDwmIsCompositionEnabled)(BOOL *);
extern pfnDwmIsCompositionEnabled dwmIsCompositionEnabled;
static FORCEINLINE BYTE IsAeroMode()
{
	BOOL result;
	return myGlobals.WantAeroAdaption && dwmIsCompositionEnabled && (dwmIsCompositionEnabled(&result) == S_OK) && result;
}
