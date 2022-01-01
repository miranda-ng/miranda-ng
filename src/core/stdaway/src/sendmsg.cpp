/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

uint32_t protoModeMsgFlags;
static HWND hwndStatusMsg;

const wchar_t* GetDefaultMessage(int status)
{
	switch (status) {
		case ID_STATUS_AWAY:       return TranslateT("I've been away since %time%.");
		case ID_STATUS_NA:         return TranslateT("Give it up, I'm not in!");
		case ID_STATUS_OCCUPIED:   return TranslateT("Not right now.");
		case ID_STATUS_DND:        return TranslateT("Give a guy some peace, would ya?");
		case ID_STATUS_FREECHAT:   return TranslateT("I'm a chatbot!");
		case ID_STATUS_ONLINE:     return TranslateT("Yep, I'm here.");
		case ID_STATUS_OFFLINE:    return TranslateT("Nope, not here.");
		case ID_STATUS_INVISIBLE:  return TranslateT("I'm hiding from the mafia.");
		case ID_STATUS_IDLE:       return TranslateT("idleeeeeeee");
	}
	return nullptr;
}

const char* StatusModeToDbSetting(int status, const char *suffix)
{
	const char *prefix;
	static char str[64];

	switch (status) {
		case ID_STATUS_AWAY:       prefix = "Away";     break;
		case ID_STATUS_NA:         prefix = "Na";       break;
		case ID_STATUS_DND:        prefix = "Dnd";      break;
		case ID_STATUS_OCCUPIED:   prefix = "Occupied"; break;
		case ID_STATUS_FREECHAT:   prefix = "FreeChat"; break;
		case ID_STATUS_ONLINE:     prefix = "On";       break;
		case ID_STATUS_OFFLINE:    prefix = "Off";      break;
		case ID_STATUS_INVISIBLE:  prefix = "Inv";      break;
		case ID_STATUS_IDLE:       prefix = "Idl";      break;
		default: return nullptr;
	}
	mir_snprintf(str, "%s%s", prefix, suffix);
	return str;
}

static wchar_t* GetAwayMessage(int statusMode, const char *szProto)
{
	if (szProto && !(CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(statusMode)))
		return nullptr;

	if (g_plugin.GetStatusModeByte(statusMode, "Ignore"))
		return nullptr;

	DBVARIANT dbv;
	if (g_plugin.GetStatusModeByte(statusMode, "UsePrev")) {
		if (g_plugin.getWString(StatusModeToDbSetting(statusMode, "Msg"), &dbv))
			dbv.pwszVal = mir_wstrdup(GetDefaultMessage(statusMode));
	}
	else {
		if (g_plugin.getWString(StatusModeToDbSetting(statusMode, "Default"), &dbv))
			dbv.pwszVal = mir_wstrdup(GetDefaultMessage(statusMode));

		for (int i = 0; dbv.pwszVal[i]; i++) {
			if (dbv.pwszVal[i] != '%')
				continue;

			wchar_t substituteStr[128];
			if (!wcsnicmp(dbv.pwszVal + i, L"%time%", 6)) {
				MIRANDA_IDLE_INFO mii;
				Idle_GetInfo(mii);

				if (mii.idleType == 1) {
					int mm;
					SYSTEMTIME t;
					GetLocalTime(&t);
					mm = t.wMinute + t.wHour * 60 - mii.idleTime;
					if (mm < 0) mm += 60 * 24;
					t.wMinute = mm % 60;
					t.wHour = mm / 60;
					GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &t, NULL, substituteStr, _countof(substituteStr));
				}
				else GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, substituteStr, _countof(substituteStr));
			}
			else if (!wcsnicmp(dbv.pwszVal + i, L"%date%", 6))
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, substituteStr, _countof(substituteStr));
			else continue;

			if (mir_wstrlen(substituteStr) > 6)
				dbv.pwszVal = (wchar_t*)mir_realloc(dbv.pwszVal, (mir_wstrlen(dbv.pwszVal) + 1 + mir_wstrlen(substituteStr) - 6) * sizeof(wchar_t));
			memmove(dbv.pwszVal + i + mir_wstrlen(substituteStr), dbv.pwszVal + i + 6, (mir_wstrlen(dbv.pwszVal) - i - 5) * sizeof(wchar_t));
			memcpy(dbv.pwszVal + i, substituteStr, mir_wstrlen(substituteStr) * sizeof(wchar_t));
		}
	}
	return dbv.pwszVal;
}

static LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if (wParam == '\n' && GetKeyState(VK_CONTROL) & 0x8000) {
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			return 0;
		}
		if (wParam == 1 && GetKeyState(VK_CONTROL) & 0x8000) { //ctrl-a
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return 0;
		}
		if (wParam == 23 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w
			SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
			return 0;
		}
		if (wParam == 127 && GetKeyState(VK_CONTROL) & 0x8000) //ctrl-backspace
		{
			uint32_t start, end;
			SendMessage(hwnd, EM_GETSEL, (WPARAM)&end, 0);
			SendMessage(hwnd, WM_KEYDOWN, VK_LEFT, 0);
			SendMessage(hwnd, EM_GETSEL, (WPARAM)&start, 0);
			int textLen = GetWindowTextLength(hwnd);
			wchar_t *text = (wchar_t *)alloca(sizeof(wchar_t) * (textLen + 1));
			GetWindowText(hwnd, text, textLen + 1);
			memmove(text + start, text + end, sizeof(wchar_t) * (textLen + 1 - end));
			SetWindowText(hwnd, text);
			SendMessage(hwnd, EM_SETSEL, start, start);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM)hwnd);
			return 0;
		}
		break;
	}
	return mir_callNextSubclass(hwnd, MessageEditSubclassProc, msg, wParam, lParam);
}

void ChangeAllProtoMessages(const char *szProto, int statusMode, wchar_t *msg)
{
	if (szProto == nullptr) {
		for (auto &pa : Accounts()) {
			if (!pa->IsEnabled())
				continue;

			if ((CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) && !pa->IsLocked())
				CallProtoService(pa->szModuleName, PS_SETAWAYMSG, statusMode, (LPARAM)msg);
		}
	}
	else CallProtoService(szProto, PS_SETAWAYMSG, statusMode, (LPARAM)msg);
}

class CSetAwayMsgDlg : public CDlgBase
{
	int m_statusMode;
	int m_countdown = 6;
	wchar_t m_okButtonFormat[64];
	const char *m_szProto;

	CTimer m_timer;
	CCtrlEdit edtMsg;
	CCtrlButton m_btnOk;

public:
	CSetAwayMsgDlg(const char *szProto, int statusMode) :
		CDlgBase(g_plugin, IDD_SETAWAYMSG),
		m_timer(this, 1),
		m_btnOk(this, IDOK),
		m_szProto(szProto),
		m_statusMode(statusMode),
		edtMsg(this, IDC_MSG)
	{
		m_timer.OnEvent = Callback(this, &CSetAwayMsgDlg::OnTimer);

		edtMsg.OnChange = Callback(this, &CSetAwayMsgDlg::onChange_Msg);
	}

	bool OnInitDialog() override
	{
		hwndStatusMsg = m_hwnd;

		edtMsg.SendMsg(EM_LIMITTEXT, 1024, 0);
		mir_subclassWindow(edtMsg.GetHwnd(), MessageEditSubclassProc);

		wchar_t str[256], format[128];
		GetWindowText(m_hwnd, format, _countof(format));
		mir_snwprintf(str, format, Clist_GetStatusModeDescription(m_statusMode, 0));
		SetWindowText(m_hwnd, str);

		m_btnOk.GetText(m_okButtonFormat, _countof(m_okButtonFormat));
		edtMsg.SetText(ptrW(GetAwayMessage(m_statusMode, m_szProto)));

		m_timer.Start(1000);
		OnTimer(0);

		Window_SetProtoIcon_IcoLib(m_hwnd, m_szProto, m_statusMode);
		Utils_RestoreWindowPosition(m_hwnd, NULL, MODULENAME, "AwayMsgDlg");
		return true;
	}

	bool OnApply() override
	{
		if (m_countdown < 0) {
			wchar_t str[1024];
			edtMsg.GetText(str, _countof(str));
			ChangeAllProtoMessages(m_szProto, m_statusMode, str);
			g_plugin.setWString(StatusModeToDbSetting(m_statusMode, "Msg"), str);
		}
		return true;
	}

	void OnDestroy() override
	{
		if (!m_bSucceeded)
			ChangeAllProtoMessages(m_szProto, m_statusMode, ptrW(GetAwayMessage(m_statusMode, m_szProto)));

		Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "AwayMsgDlg");
		KillTimer(m_hwnd, 1);
		Window_FreeIcon_IcoLib(m_hwnd);
		hwndStatusMsg = nullptr;
	}

	void OnTimer(CTimer *)
	{
		if (--m_countdown >= 0) {
			wchar_t str[64];
			mir_snwprintf(str, m_okButtonFormat, m_countdown);
			m_btnOk.SetText(str);
		}
		else {
			m_timer.Stop();
			Close();
		}
	}

	void onChange_Msg(CCtrlEdit *)
	{
		if (m_countdown >= 0) {
			m_timer.Stop();
			m_btnOk.SetText(TranslateT("OK"));
			m_countdown = -1;
		}
	}
};

static int StatusModeChange(WPARAM wParam, LPARAM lParam)
{
	int statusMode = (int)wParam;
	char *szProto = (char*)lParam;

	if (protoModeMsgFlags == 0)
		return 0;

	// If its a global change check the complete PFLAGNUM_3 flags to see if a popup might be needed
	if (!szProto) {
		if (!(protoModeMsgFlags & Proto_Status2Flag(statusMode)))
			return 0;
	}
	else {
		// If its a single protocol check the PFLAGNUM_3 for the single protocol
		if (!(CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) ||
			 !(CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(statusMode)))
			return 0;
	}

	BOOL bScreenSaverRunning = IsScreenSaverRunning();
	if (g_plugin.GetStatusModeByte(statusMode, "Ignore"))
		ChangeAllProtoMessages(szProto, statusMode, nullptr);

	else if (bScreenSaverRunning || g_plugin.GetStatusModeByte(statusMode, "NoDlg", true)) {
		ChangeAllProtoMessages(szProto, statusMode, ptrW(GetAwayMessage(statusMode, szProto)));
	}
	else {
		if (hwndStatusMsg)
			DestroyWindow(hwndStatusMsg);
		(new CSetAwayMsgDlg(szProto, statusMode))->Create();
	}
	return 0;
}

static int AwayMsgSendAccountsChanged(WPARAM, LPARAM)
{
	protoModeMsgFlags = 0;

	for (auto &pa : Accounts())
		if (pa->IsEnabled())
			protoModeMsgFlags |= CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0);

	return 0;
}

static int AwayMsgSendModulesLoaded(WPARAM, LPARAM)
{
	AwayMsgSendAccountsChanged(0, 0);
	return 0;
}

static int AwayMsgSendPreshutdown(WPARAM, LPARAM)
{
	if (hwndStatusMsg)
		DestroyWindow(hwndStatusMsg);
	return 0;
}

// remember to mir_free() the return value
static INT_PTR sttGetAwayMessageT(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)GetAwayMessage((int)wParam, (char*)lParam);
}

int LoadAwayMessageSending(void)
{
	HookEvent(ME_CLIST_STATUSMODECHANGE, StatusModeChange);
	HookEvent(ME_OPT_INITIALISE, AwayMsgOptInitialise);
	HookEvent(ME_PROTO_ACCLISTCHANGED, AwayMsgSendAccountsChanged);
	HookEvent(ME_SYSTEM_MODULESLOADED, AwayMsgSendModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, AwayMsgSendPreshutdown);

	CreateServiceFunction(MS_AWAYMSG_GETSTATUSMSGW, sttGetAwayMessageT);
	return 0;
}
