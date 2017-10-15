/*
	AdvancedAutoAway Plugin for Miranda-IM (www.miranda-im.org)
	KeepStatus Plugin for Miranda-IM (www.miranda-im.org)
	StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
	Copyright 2003-2006 P. Boon

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

#include "stdafx.h"

// handles for hooks and other Miranda thingies
static HANDLE hCSStatusChangedExEvent;

OBJLIST<PROTOCOLSETTINGEX> *protoList;

// prototypes
char* StatusModeToDbSetting(int status, const char *suffix);
DWORD StatusModeToProtoFlag(int status);
INT_PTR SetStatusEx(WPARAM wParam, LPARAM lParam);
int InitCommonStatus();
int GetProtoCount();

// some helpers from awaymsg.c ================================================================
char *StatusModeToDbSetting(int status, const char *suffix)
{
	char *prefix;
	static char str[64];

	switch (status) {
	case ID_STATUS_AWAY: prefix = "Away";	break;
	case ID_STATUS_NA: prefix = "Na";	break;
	case ID_STATUS_DND: prefix = "Dnd"; break;
	case ID_STATUS_OCCUPIED: prefix = "Occupied"; break;
	case ID_STATUS_FREECHAT: prefix = "FreeChat"; break;
	case ID_STATUS_ONLINE: prefix = "On"; break;
	case ID_STATUS_OFFLINE: prefix = "Off"; break;
	case ID_STATUS_INVISIBLE: prefix = "Inv"; break;
	case ID_STATUS_ONTHEPHONE: prefix = "Otp"; break;
	case ID_STATUS_OUTTOLUNCH: prefix = "Otl"; break;
	default: return nullptr;
	}
	mir_strcpy(str, prefix); mir_strcat(str, suffix);
	return str;
}

DWORD StatusModeToProtoFlag(int status)
{
	// *not* the same as in core, <offline>
	switch (status) {
	case ID_STATUS_ONLINE: return PF2_ONLINE;
	case ID_STATUS_OFFLINE: return PF2_OFFLINE;
	case ID_STATUS_INVISIBLE: return PF2_INVISIBLE;
	case ID_STATUS_OUTTOLUNCH: return PF2_OUTTOLUNCH;
	case ID_STATUS_ONTHEPHONE: return PF2_ONTHEPHONE;
	case ID_STATUS_AWAY: return PF2_SHORTAWAY;
	case ID_STATUS_NA: return PF2_LONGAWAY;
	case ID_STATUS_OCCUPIED: return PF2_LIGHTDND;
	case ID_STATUS_DND: return PF2_HEAVYDND;
	case ID_STATUS_FREECHAT: return PF2_FREECHAT;
	}
	return 0;
}

int GetActualStatus(PROTOCOLSETTINGEX *protoSetting)
{
	if (protoSetting->m_status == ID_STATUS_LAST) {
		if ((protoSetting->m_lastStatus < MIN_STATUS) || (protoSetting->m_lastStatus > MAX_STATUS))
			return CallProtoService(protoSetting->m_szName, PS_GETSTATUS, 0, 0);
		return protoSetting->m_lastStatus;
	}
	if (protoSetting->m_status == ID_STATUS_CURRENT)
		return CallProtoService(protoSetting->m_szName, PS_GETSTATUS, 0, 0);

	if ((protoSetting->m_status < ID_STATUS_OFFLINE) || (protoSetting->m_status > ID_STATUS_OUTTOLUNCH)) {
		log_debugA("invalid status detected: %d", protoSetting->m_status);
		return 0;
	}
	return protoSetting->m_status;
}

wchar_t* GetDefaultStatusMessage(PROTOCOLSETTINGEX *ps, int newstatus)
{
	if (ps->m_szMsg != nullptr) {// custom message set
		log_infoA("CommonStatus: Status message set by calling plugin");
		return mir_wstrdup(ps->m_szMsg);
	}

	wchar_t *tMsg = (wchar_t*)CallService(MS_AWAYMSG_GETSTATUSMSGW, newstatus, (LPARAM)ps->m_szName);
	log_debugA("CommonStatus: Status message retrieved from general awaysys: %S", tMsg);
	return tMsg;
}

static int equalsGlobalStatus(PROTOCOLSETTINGEX **ps)
{
	int i, j, pstatus = 0, gstatus = 0;

	for (i = 0; i < protoList->getCount(); i++)
		if (ps[i]->m_szMsg != nullptr && GetActualStatus(ps[i]) != ID_STATUS_OFFLINE)
			return 0;

	int count;
	PROTOACCOUNT **protos;
	Proto_EnumAccounts(&count, &protos);

	for (i = 0; i < count; i++) {
		if (!IsSuitableProto(protos[i]))
			continue;

		pstatus = 0;
		for (j = 0; j < protoList->getCount(); j++)
			if (!mir_strcmp(protos[i]->szModuleName, ps[j]->m_szName))
				pstatus = GetActualStatus(ps[j]);

		if (pstatus == 0)
			pstatus = CallProtoService(protos[i]->szModuleName, PS_GETSTATUS, 0, 0);

		if (db_get_b(0, protos[i]->szModuleName, "LockMainStatus", 0)) {
			// if proto is locked, pstatus must be the current status
			if (pstatus != CallProtoService(protos[i]->szModuleName, PS_GETSTATUS, 0, 0))
				return 0;
		}
		else {
			if (gstatus == 0)
				gstatus = pstatus;

			if (pstatus != gstatus)
				return 0;
		}
	}

	return gstatus;
}

static void SetStatusMsg(PROTOCOLSETTINGEX *ps, int newstatus)
{
	wchar_t* tszMsg = GetDefaultStatusMessage(ps, newstatus);
	if (tszMsg) {
		/* replace the default vars in msg  (I believe this is from core) */
		for (int j = 0; tszMsg[j]; j++) {
			if (tszMsg[j] != '%')
				continue;

			wchar_t substituteStr[128];
			if (!wcsnicmp(tszMsg + j, L"%time%", 6))
				GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, 0, 0, substituteStr, _countof(substituteStr));
			else if (!wcsnicmp(tszMsg + j, L"%date%", 6))
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, 0, 0, substituteStr, _countof(substituteStr));
			else
				continue;

			if (mir_wstrlen(substituteStr) > 6)
				tszMsg = (wchar_t*)mir_realloc(tszMsg, sizeof(wchar_t)*(mir_wstrlen(tszMsg) + 1 + mir_wstrlen(substituteStr) - 6));
			memmove(tszMsg + j + mir_wstrlen(substituteStr), tszMsg + j + 6, sizeof(wchar_t)*(mir_wstrlen(tszMsg) - j - 5));
			memcpy(tszMsg + j, substituteStr, sizeof(wchar_t)*mir_wstrlen(substituteStr));
		}

		wchar_t *szFormattedMsg = variables_parsedup(tszMsg, ps->m_tszAccName, 0);
		if (szFormattedMsg != nullptr) {
			mir_free(tszMsg);
			tszMsg = szFormattedMsg;
		}
	}
	log_debugA("CommonStatus sets status message for %s directly", ps->m_szName);
	CallProtoService(ps->m_szName, PS_SETAWAYMSG, newstatus, (LPARAM)tszMsg);
	mir_free(tszMsg);
}

INT_PTR SetStatusEx(WPARAM wParam, LPARAM)
{
	PROTOCOLSETTINGEX** protoSettings = *(PROTOCOLSETTINGEX***)wParam;
	if (protoSettings == nullptr)
		return -1;

	int globStatus = equalsGlobalStatus(protoSettings);

	// issue with setting global status;
	// things get messy because SRAway hooks ME_CLIST_STATUSMODECHANGE, so the status messages of SRAway and
	// commonstatus will clash
	NotifyEventHooks(hCSStatusChangedExEvent, (WPARAM)&protoSettings, protoList->getCount());

	// set all status messages first
	for (int i = 0; i < protoList->getCount(); i++) {
		char *szProto = protoSettings[i]->m_szName;
		if (!Proto_GetAccount(szProto)) {
			log_debugA("CommonStatus: %s is not loaded", szProto);
			continue;
		}
		// some checks
		int newstatus = GetActualStatus(protoSettings[i]);
		if (newstatus == 0) {
			log_debugA("CommonStatus: incorrect status for %s (%d)", szProto, protoSettings[i]->m_status);
			continue;
		}
		int oldstatus = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
		// set last status
		protoSettings[i]->m_lastStatus = oldstatus;
		if (IsStatusConnecting(oldstatus)) {
			// ignore if connecting, but it didn't came this far if it did
			log_debugA("CommonStatus: %s is already connecting", szProto);
			continue;
		}

		// status checks
		long protoFlag = Proto_Status2Flag(newstatus);
		int b_Caps2 = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_2, 0) & protoFlag;
		int b_Caps5 = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_5, 0) & protoFlag;
		if (newstatus != ID_STATUS_OFFLINE && (!b_Caps2 || b_Caps5)) {
			// status and status message for this status not supported
			//log_debug("CommonStatus: status not supported %s", szProto);
			continue;
		}

		int b_Caps1 = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND & ~PF1_INDIVMODEMSG;
		int b_Caps3 = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & protoFlag;
		if (newstatus == oldstatus && (!b_Caps1 || !b_Caps3)) {
			// no status change and status messages are not supported
			//log_debug("CommonStatus: no change, %s (%d %d)", szProto, oldstatus, newstatus);
			continue;
		}

		// set status message first
		if (b_Caps1 && b_Caps3)
			SetStatusMsg(protoSettings[i], newstatus);

		// set the status
		if (newstatus != oldstatus /*&& !(b_Caps1 && b_Caps3 && ServiceExists(MS_NAS_SETSTATE))*/) {
			log_debugA("CommonStatus sets status for %s to %d", szProto, newstatus);
			CallProtoService(szProto, PS_SETSTATUS, newstatus, 0);
		}
	}

	if (globStatus != 0) {
		log_debugA("CommonStatus: setting global status %u", globStatus);
		Clist_SetStatusMode(globStatus);
	}

	return 0;
}

static INT_PTR GetProtocolCountService(WPARAM, LPARAM)
{
	return GetProtoCount();
}

bool IsSuitableProto(PROTOACCOUNT *pa)
{
	return (pa == nullptr) ? false : (pcli->pfnGetProtocolVisibility(pa->szModuleName) != 0);
}

int GetProtoCount()
{
	int pCount = 0, count;
	PROTOACCOUNT **accs;
	Proto_EnumAccounts(&count, &accs);

	for (int i = 0; i < count; i++)
		if (IsSuitableProto(accs[i]))
			pCount++;

	return pCount;
}

static int CreateServices()
{
	if (ServiceExists(MS_CS_SETSTATUSEX))
		return -1;

	hCSStatusChangedExEvent = CreateHookableEvent(ME_CS_STATUSCHANGEEX);

	CreateServiceFunction(MS_CS_SETSTATUSEX, SetStatusEx);
	CreateServiceFunction(MS_CS_GETPROTOCOUNT, GetProtocolCountService);
	return 0;
}

static int onShutdown(WPARAM, LPARAM)
{
	DestroyHookableEvent(hCSStatusChangedExEvent);
	return 0;
}

int InitCommonStatus()
{
	if (!CreateServices())
		HookEvent(ME_SYSTEM_PRESHUTDOWN, onShutdown);

	return 0;
}
