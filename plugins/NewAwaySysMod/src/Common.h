/*
	New Away System - plugin for Miranda IM
	Copyright (c) 2005-2007 Chervov Dmitry
	Copyright (c) 2004-2005 Iksaif Entertainment
	Copyright (c) 2002-2003 Goblineye Entertainment

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0500

#define MIRANDA_VER 0x0A00

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <CommCtrl.h>
#include <commdlg.h>
#include <Uxtheme.h>
#include <vsstyle.h>

#include <time.h>
#include <shellapi.h>
#include <crtdbg.h>
#include <tchar.h>
#include <stdarg.h>

#include "newpluginapi.h"
#include "m_clist.h"
#include "m_system.h"
#include "m_database.h"
#include "m_clistint.h"
#include "m_langpack.h"
#include "m_protosvc.h"
#include "m_options.h"
#include "m_netlib.h"
#include "..\..\protocols\IcqOscarJ\src\icq_constants.h"
#include "m_skin.h"
#include "m_awaymsg.h"
#include "m_utils.h"
#include "m_system_cpp.h"
#include "m_history.h"
#include "m_icolib.h"
#include "m_message.h"
#include "m_userinfo.h"
#include "m_icq.h"
#include "win2k.h"

#include "resource.h"

#include "m_variables.h"
#include "m_toptoolbar.h"
#include "m_popup.h"
#include "m_metacontacts.h"

#include "CString.h"
#include "Options.h"

#define CBSCHECK_UNCHECKED 1
#define CBSCHECK_CHECKED 5
#define CBSCHECK_MIXED 9
#define CBSSTATE_NORMAL 0
#define CBSSTATE_HOT 1
#define CBSSTATE_PRESSED 2
#define CBSSTATE_DISABLED 3

#define VAR_AWAYSINCE_TIME _T("nas_awaysince_time")
#define VAR_AWAYSINCE_DATE _T("nas_awaysince_date")
#define VAR_STATDESC _T("nas_statdesc")
#define VAR_MYNICK _T("nas_mynick")
#define VAR_REQUESTCOUNT _T("nas_requestcount")
#define VAR_MESSAGENUM _T("nas_messagecount")
#define VAR_TIMEPASSED _T("nas_timepassed")
#define VAR_PREDEFINEDMESSAGE _T("nas_predefinedmessage")
#define VAR_PROTOCOL _T("nas_protocol")

#define SENDSMSG_EVENT_MSG 0x1
#define SENDSMSG_EVENT_URL 0x2
#define SENDSMSG_EVENT_FILE 0x4

#define AWAY_MSGDATA_MAX 8000

// Flags for status database settings
#define SF_OFF 0x1
#define SF_ONL 0x2
#define SF_AWAY 0x4
#define SF_NA 0x8
#define SF_OCC 0x10
#define SF_DND 0x20
#define SF_FFC 0x40
#define SF_INV 0x80
#define SF_OTP 0x100
#define SF_OTL 0x200
#define SF_OTHER 0x80000000

#define MOREOPTDLG_DEF_DONTPOPDLG (SF_ONL | SF_INV)
#define MOREOPTDLG_DEF_USEBYDEFAULT 0

// Event flags (used for "reply on event" options)
#define EF_MSG 1
#define EF_URL 2
#define EF_FILE 4

#define AUTOREPLY_DEF_REPLY 0
#define AUTOREPLY_DEF_REPLYONEVENT (EF_MSG | EF_URL | EF_FILE)
#define AUTOREPLY_DEF_PREFIX TranslateT("Miranda NG autoreply >\r\n%extratext%")
#define AUTOREPLY_DEF_DISABLEREPLY (SF_ONL | SF_INV)

#define AUTOREPLY_IDLE_WINDOWS 0
#define AUTOREPLY_IDLE_MIRANDA 1
#define AUTOREPLY_DEF_IDLEREPLYVALUE AUTOREPLY_IDLE_WINDOWS

#define AUTOREPLY_MAXPREFIXLEN 8000

#define VAL_USEDEFAULT 2 // undefined value for ignore/autoreply/notification settings in the db; must be 2 for proper ContactSettings support

// Set Away Message dialog flags
#define DF_SAM_SHOWMSGTREE 1
#define DF_SAM_SHOWCONTACTTREE 2
#define DF_SAM_DEFDLGFLAGS DF_SAM_SHOWMSGTREE

// WriteAwayMsgInDB option flags:
#define WRITE_LMSG 1
#define WRITE_RMSG 2
#define WRITE_INTERPRET 4
#define WRITE_CMSG 8

#define TOGGLE_SOE_COMMAND LPGENT("Toggle autoreply on/off")
#define DISABLE_SOE_COMMAND LPGENT("Toggle autoreply off")
#define ENABLE_SOE_COMMAND LPGENT("Toggle autoreply on")

#define STR_XSTATUSDESC TranslateT("extended status")

#define MOD_NAME "NewAwaySys"
#define LOG_ID MOD_NAME // LogService log ID
#define LOG_PREFIX MOD_NAME ": " // netlib.log prefix for all NAS' messages

#define DB_SETTINGSVER "SettingsVer"

#ifndef SIZEOF
#define SIZEOF(s) (sizeof(s) / sizeof(*s))
#endif

#define UM_ICONSCHANGED (WM_USER + 121)

// IDD_READAWAYMSG user-defined message constants
#define UM_RAM_AWAYMSGACK (WM_USER + 10)

// IDD_SETAWAYMSG user-defined message constants
#define UM_SAM_SPLITTERMOVED (WM_USER + 1)
#define UM_SAM_SAVEDLGSETTINGS (WM_USER + 2)
#define UM_SAM_APPLYANDCLOSE (WM_USER + 3)
#define UM_SAM_KILLTIMER (WM_USER + 4)
#define UM_SAM_REPLYSETTINGCHANGED (WM_USER + 5)
#define UM_SAM_PROTOSTATUSCHANGED (WM_USER + 6) // wParam = (char*)szProto

#define UM_CLICK (WM_USER + 100)

#define SAM_DB_DLGPOSX "SAMDlgPosX"
#define SAM_DB_DLGPOSY "SAMDlgPosY"
#define SAM_DB_DLGSIZEX "SAMDlgSizeX"
#define SAM_DB_DLGSIZEY "SAMDlgSizeY"
#define SAM_DB_MSGSPLITTERPOS "SAMMsgSplitterPos"
#define SAM_DB_CONTACTSPLITTERPOS "SAMContactSplitterPos"

#define DB_MESSAGECOUNT "MessageCount"
#define DB_REQUESTCOUNT "RequestCount"
#define DB_SENDCOUNT "SendCount"
#define MESSAGES_DB_MSGTREEDEF "MsgTreeDef"

#define MSGTREE_RECENT_OTHERGROUP _T("Other")

// GetMsgFormat flags
#define GMF_PERSONAL 1 // is also used to get global status message, when hContact = NULL (szProto = NULL)
#define GMF_PROTOORGLOBAL 2
#define GMF_LASTORDEFAULT 4 // this flag doesn't require hContact or szProto
#define GMF_TEMPORARY 8 // doesn't require status
#define GMF_ANYCURRENT (GMF_TEMPORARY | GMF_PERSONAL | GMF_PROTOORGLOBAL)

// SetMsgFormat flags
#define SMF_PERSONAL 1 // is also used to set global status message, when hContact = NULL (szProto = NULL)
#define SMF_LAST 2
#define SMF_TEMPORARY 4 // doesn't require status

// VAR_PARSE_DATA flags
#define VPF_XSTATUS 1 // use "extended status" instead of the usual status description in %nas_statdesc%, and XStatus message in %nas_message%

// options dialog
#define OPT_TITLE LPGENT("Away System")
#define OPT_MAINGROUP LPGEN("Status")
#define OPT_POPUPGROUP LPGEN("Popups")

#define MRM_MAX_GENERATED_TITLE_LEN 35 // maximum length of automatically generated title for recent messages

int ICQStatusToGeneralStatus(int bICQStat); // TODO: get rid of these protocol-specific functions, if possible

#define MS_AWAYSYS_SETCONTACTSTATMSG "AwaySys/SetContactStatMsg"

#define MS_AWAYSYS_AUTOREPLY_TOGGLE "AwaySys/AutoreplyToggle"
#define MS_AWAYSYS_AUTOREPLY_ON "AwaySys/AutoreplyOn"
#define MS_AWAYSYS_AUTOREPLY_OFF "AwaySys/AutoreplyOff"
#define MS_AWAYSYS_AUTOREPLY_USEDEFAULT "AwaySys/AutoreplyUseDefault"

#define MS_AWAYSYS_VARIABLESHANDLER "AwaySys/VariablesHandler"
#define MS_AWAYSYS_FREEVARMEM "AwaySys/FreeVarMem"
// these are obsolete AwaySysMod services, though they're still here for compatibility with old plugins
#define MS_AWAYSYS_SETSTATUSMODE "AwaySys/SetStatusMode" // change the status mode. wParam is new mode, lParam is new status message (AwaySys will interpret variables out of it), may be NULL.
#define MS_AWAYSYS_IGNORENEXT "AwaySys/IgnoreNextStatusChange" //ignore nest status change

struct SetAwayMsgData
{
	CString szProtocol;
	MCONTACT hInitContact; // initial contact (filled by caller)
	TCString Message; // initial message, NULL means default
	bool IsModeless; // means the dialog was created with the CreateDialogParam function, not DialogBoxParam
	int ISW_Flags; // InvokeStatusWindow service flags
};

struct VAR_PARSE_DATA
{
	char *szProto;
	TCString Message;
	DWORD UIN;
	int Flags; // a combination of VPF_ flags
};

struct DYNAMIC_NOTIFY_DATA
{
	MCONTACT hContact;
	int iStatusMode;
	TCString Proto;
};

struct PLUGIN_DATA
{
	BYTE PopupLClickAction, PopupRClickAction;
	MCONTACT hContact;
	HICON hStatusIcon; // needed here to destroy its handle on UM_FREEPLUGINDATA
};

struct NAS_ISWINFOv1
{
	int cbSize;
	char *szProto;
	MCONTACT hContact;
	char *szMsg;
	WORD status;
};

#define MTYPE_AUTOONLINE 0xE7 // required to support ICQ Plus online status messages

// Beware of conflicts between two different windows trying to use the same page at a time!
// Other windows than the owner of the Page must copy the page to their own memory,
// or use GetDBValueCopy to retrieve values
extern COptPage g_MessagesOptPage;
extern COptPage g_AutoreplyOptPage;
//extern COptPage g_PopupOptPage;
extern COptPage g_MoreOptPage;
extern COptPage g_SetAwayMsgPage;

extern HINSTANCE g_hInstance;
extern int g_Messages_RecentRootID, g_Messages_PredefinedRootID;
extern VAR_PARSE_DATA VarParseData;
extern bool g_fNoProcessing;
extern int g_bIsIdle;

// AwaySys.cpp
TCString GetDynamicStatMsg(MCONTACT hContact, char *szProto = NULL, DWORD UIN = 0, int iStatus = 0);
int IsAnICQProto(char *szProto);

// Client.cpp
void InitUpdateMsgs();
void ChangeProtoMessages(char* szProto, int iMode, TCString &Msg);
int GetRecentGroupID(int iMode);
TCString VariablesEscape(TCString Str);

// SetAwayMsg.cpp
INT_PTR CALLBACK SetAwayMsgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ReadAwayMsg.cpp
extern HANDLE g_hReadWndList;
INT_PTR GetContactStatMsg(WPARAM wParam, LPARAM lParam);

// AwayOpt.cpp
int ModernOptInitialise(WPARAM wParam, LPARAM lParam);
int OptsDlgInit(WPARAM wParam, LPARAM); // called on opening of the options dialog
void InitOptions(); // called once when plugin is loaded

//int ShowPopupNotification(COptPage &PopupNotifyData, MCONTACT hContact, int iStatusMode);
void ShowLog(TCString &LogFilePath);
void ShowMsg(TCHAR *szFirstLine, TCHAR *szSecondLine = _T(""), bool IsErrorMsg = false, int Timeout = 0);

#define AWAYSYS_STATUSMSGREQUEST_SOUND "AwaySysStatusMsgRequest"
#define ME_AWAYSYS_WORKAROUND "AwaySys/_CallService"
int _Workaround_CallService(const char *name, WPARAM wParam, LPARAM lParam);

// MsgEventAdded.cpp
int MsgEventAdded(WPARAM wParam, LPARAM lParam);

// buttons
INT_PTR ToggleSendOnEvent(WPARAM wParam, LPARAM lParam);

static __inline int LogMessage(const char *Format, ...)
{
	va_list va;
	char szText[8096];
	strcpy(szText, LOG_PREFIX);
	va_start(va, Format);
	mir_vsnprintf(szText + (SIZEOF(LOG_PREFIX) - 1), sizeof(szText) - (SIZEOF(LOG_PREFIX) - 1), Format, va);
	va_end(va);
	return CallService(MS_NETLIB_LOG, NULL, (LPARAM)szText);
}

__inline int CallAllowedPS_SETAWAYMSG(const char *szProto, int iMode, const char *szMsg)
{ // we must use this function everywhere we want to call PS_SETAWAYMSG, otherwise NAS won't allow to change the message!
	LogMessage("PS_SETAWAYMSG called by NAS. szProto=%s, Status=%d, Msg:\n%s", szProto, iMode, szMsg ? szMsg : "NULL");
	char str[MAXMODULELABELLENGTH];
	strcpy(str, szProto);
	strcat(str, PS_SETAWAYMSG);
	return CallService(str, (WPARAM)iMode, (LPARAM)szMsg);
}

static __inline int my_variables_showhelp(HWND hwndDlg, UINT uIDEdit, int flags = 0, char *szSubjectDesc = NULL, char *szExtraDesc = NULL)
{
	if (ServiceExists(MS_VARS_SHOWHELPEX)) {
		return variables_showhelp(hwndDlg, uIDEdit, flags, szSubjectDesc,szExtraDesc);
	}
	else {
		ShowMsg(TranslateT("New Away System"), TranslateT("Variables plugin is not installed"), true);
		return -1;
	}
}
