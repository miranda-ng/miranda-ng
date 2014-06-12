/*

Simple Status Message plugin for Miranda IM
Copyright (C) 2006-2011 Bartosz 'Dezeath' Bia³ek, (C) 2005 Harven

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef SIMPLESTATUSMSG_H__
#define SIMPLESTATUSMSG_H__ 1

typedef struct tagACCOUNTS
{
	PROTOACCOUNT	**pa;
	int				count;
	int				statusCount;
	DWORD			statusFlags;
	int				statusMsgCount;
	DWORD			statusMsgFlags;
}
PROTOACCOUNTS;

extern PROTOACCOUNTS *accounts;

// Represents status that a protocol(s) is/are currently in
#define	ID_STATUS_CURRENT	40082

#define DLG_SHOW_STATUS				1
#define DLG_SHOW_STATUS_ICONS		2
#define DLG_SHOW_LIST_ICONS			4
#define DLG_SHOW_BUTTONS			8
#define DLG_SHOW_BUTTONS_INLIST		16
#define DLG_SHOW_BUTTONS_FLAT		32
#define	DLG_SHOW_STATUS_PROFILES	64
//NOTE: MAX 128
#define DLG_SHOW_DEFAULT			DLG_SHOW_STATUS | DLG_SHOW_STATUS_ICONS | DLG_SHOW_LIST_ICONS | DLG_SHOW_BUTTONS

#define STATUS_SHOW_DLG			1
#define STATUS_EMPTY_MSG		2
#define STATUS_DEFAULT_MSG		4
#define STATUS_LAST_MSG			8
#define STATUS_THIS_MSG			16
#define STATUS_LAST_STATUS_MSG	32
#define STATUS_DEFAULT			STATUS_SHOW_DLG | STATUS_LAST_MSG

#define PROTO_NO_MSG	1
#define PROTO_THIS_MSG	2
#define PROTO_POPUPDLG	4
#define PROTO_NOCHANGE	8
#define PROTO_DEFAULT	PROTO_NOCHANGE

struct MsgBoxInitData
{
	char	*m_szProto;
	int		m_iStatus;
	int		m_iStatusModes;
	int		m_iStatusMsgModes;
	BOOL	m_bOnEvent;
	BOOL	m_bOnStartup;
};

extern HINSTANCE g_hInst;
extern HWND hwndSAMsgDialog;

#define MS_SIMPLESTATUSMSG_SHOWDIALOGINT "SimpleStatusMsg/ShowDialogInternal" // internal use ONLY

/* awaymsg.cpp */
int LoadAwayMsgModule(void);
int AwayMsgPreShutdown(void);

/* main.cpp */
void SetStatusMessage(const char *szProto, int initial_status_mode, int status_mode, TCHAR *message, BOOL on_startup);

/* msgbox.cpp */
INT_PTR CALLBACK AwayMsgBoxDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* options.cpp */
int InitOptions(WPARAM wParam, LPARAM lParam);

/* utils.cpp */
void IconsInit(void);
HICON LoadIconEx(const char* name);
HANDLE GetIconHandle(int iconId);
void ReleaseIconEx(const char* name);
HANDLE HookProtoEvent(const char *szModule, const char *szEvent, MIRANDAHOOKPARAM hookProc);
void UnhookProtoEvents(void);
int GetRandom(int from, int to);
const TCHAR *GetDefaultMessage(int status);
const char *StatusModeToDbSetting(int status, const char *suffix);
int GetCurrentStatus(const char *szProto);
int GetStartupStatus(const char *szProto);

#endif // SIMPLESTATUSMSG_H__
