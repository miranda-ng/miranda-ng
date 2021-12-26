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

StatusModeMap statusModes[MAX_STATUS_COUNT] =
{
	{ ID_STATUS_OFFLINE,    PF2_OFFLINE },
	{ ID_STATUS_ONLINE,     PF2_ONLINE },
	{ ID_STATUS_AWAY,       PF2_SHORTAWAY },
	{ ID_STATUS_NA,         PF2_LONGAWAY },
	{ ID_STATUS_OCCUPIED,   PF2_LIGHTDND },
	{ ID_STATUS_DND,        PF2_HEAVYDND },
	{ ID_STATUS_FREECHAT,   PF2_FREECHAT },
	{ ID_STATUS_INVISIBLE,  PF2_INVISIBLE }
};

// handles for hooks and other Miranda thingies
static HANDLE hCSStatusChangedExEvent;

int CompareProtoSettings(const SMProto *p1, const SMProto *p2)
{
	return mir_strcmp(p1->m_szName, p2->m_szName);
}

TProtoSettings protoList;

/////////////////////////////////////////////////////////////////////////////////////////

SMProto::SMProto(PROTOACCOUNT *pa)
{
	m_szName = pa->szModuleName;
	m_tszAccName = pa->tszAccountName;
	m_status = m_lastStatus = pa->iRealStatus;
}

SMProto::SMProto(const SMProto &p)
{
	memcpy(this, &p, sizeof(SMProto));
	m_szMsg = mir_wstrdup(p.m_szMsg);
}

SMProto::~SMProto()
{
	mir_free(m_szMsg);
}

/////////////////////////////////////////////////////////////////////////////////////////

TProtoSettings::TProtoSettings()
	: OBJLIST<SMProto>(10, CompareProtoSettings)
{}

TProtoSettings::TProtoSettings(const TProtoSettings &p)
	: OBJLIST<SMProto>(p.getCount(), CompareProtoSettings)
{
	for (auto &it : p)
		insert(new SMProto(*it));
}

// some helpers from awaymsg.c ================================================================

char* StatusModeToDbSetting(int status, const char *suffix)
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
	default: return nullptr;
	}
	mir_strcpy(str, prefix); mir_strcat(str, suffix);
	return str;
}

uint32_t StatusModeToProtoFlag(int status)
{
	// *not* the same as in core, <offline>
	switch (status) {
	case ID_STATUS_ONLINE: return PF2_ONLINE;
	case ID_STATUS_OFFLINE: return PF2_OFFLINE;
	case ID_STATUS_INVISIBLE: return PF2_INVISIBLE;
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
			return Proto_GetStatus(protoSetting->m_szName);
		
		return protoSetting->m_lastStatus;
	}
	
	if (protoSetting->m_status == ID_STATUS_CURRENT)
		return Proto_GetStatus(protoSetting->m_szName);

	if ((protoSetting->m_status < ID_STATUS_OFFLINE) || (protoSetting->m_status > ID_STATUS_MAX)) {
		log_debug(0, "invalid status detected: %d", protoSetting->m_status);
		return 0;
	}
	return protoSetting->m_status;
}

wchar_t* GetDefaultStatusMessage(PROTOCOLSETTINGEX *ps, int newstatus)
{
	if (ps->m_szMsg != nullptr) {// custom message set
		log_info(0, "CommonStatus: Status message set by calling plugin");
		return mir_wstrdup(ps->m_szMsg);
	}

	wchar_t *tMsg = (wchar_t*)CallService(MS_AWAYMSG_GETSTATUSMSGW, newstatus, (LPARAM)ps->m_szName);
	log_debug(0, "CommonStatus: Status message retrieved from general awaysys: %S", tMsg);
	return tMsg;
}

static int equalsGlobalStatus(TProtoSettings &ps)
{
	int pstatus = 0, gstatus = 0;

	for (auto &it : protoList)
		if (it->m_szMsg != nullptr && GetActualStatus(it) != ID_STATUS_OFFLINE)
			return 0;

	for (auto &pa : Accounts()) {
		if (!IsSuitableProto(pa))
			continue;

		pstatus = 0;
		for (auto &it : ps)
			if (!mir_strcmp(pa->szModuleName, it->m_szName))
				pstatus = GetActualStatus(it);

		if (pstatus == 0)
			pstatus = pa->iRealStatus;

		if (pa->bIsLocked) {
			// if proto is locked, pstatus must be the current status
			if (pstatus != pa->iRealStatus)
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
				GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, nullptr, nullptr, substituteStr, _countof(substituteStr));
			else if (!wcsnicmp(tszMsg + j, L"%date%", 6))
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, nullptr, nullptr, substituteStr, _countof(substituteStr));
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
	log_debug(0, "CommonStatus sets status message for %s directly", ps->m_szName);
	CallProtoService(ps->m_szName, PS_SETAWAYMSG, newstatus, (LPARAM)tszMsg);
	mir_free(tszMsg);
}

int SetStatusEx(TProtoSettings &ps)
{
	int globStatus = equalsGlobalStatus(ps);

	// issue with setting global status;
	// things get messy because SRAway hooks ME_CLIST_STATUSMODECHANGE, so the status messages of SRAway and
	// commonstatus will clash
	NotifyEventHooks(hCSStatusChangedExEvent, (WPARAM)&ps, ps.getCount());

	// set all status messages first
	for (auto &p : ps) {
		if (p->m_status == ID_STATUS_DISABLED)
			continue;

		if (!Proto_GetAccount(p->m_szName)) {
			log_debug(0, "CommonStatus: %s is not loaded", p->m_szName);
			continue;
		}
		// some checks
		int newstatus = GetActualStatus(p);
		if (newstatus == 0) {
			log_debug(0, "CommonStatus: incorrect status for %s (%d)", p->m_szName, p->m_status);
			continue;
		}
		int oldstatus = Proto_GetStatus(p->m_szName);
		// set last status
		p->m_lastStatus = oldstatus;
		if (IsStatusConnecting(oldstatus)) {
			// ignore if connecting, but it didn't came this far if it did
			log_debug(0, "CommonStatus: %s is already connecting", p->m_szName);
			continue;
		}

		// status checks
		long protoFlag = Proto_Status2Flag(newstatus);
		int b_Caps2 = CallProtoService(p->m_szName, PS_GETCAPS, PFLAGNUM_2, 0) & protoFlag;
		int b_Caps5 = CallProtoService(p->m_szName, PS_GETCAPS, PFLAGNUM_5, 0) & protoFlag;
		if (newstatus != ID_STATUS_OFFLINE && (!b_Caps2 || b_Caps5)) {
			// status and status message for this status not supported
			log_debug(0, "CommonStatus: status not supported %s", p->m_szName);
			continue;
		}

		int b_Caps1 = CallProtoService(p->m_szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND & ~PF1_INDIVMODEMSG;
		int b_Caps3 = CallProtoService(p->m_szName, PS_GETCAPS, PFLAGNUM_3, 0) & protoFlag;
		if (newstatus == oldstatus && (!b_Caps1 || !b_Caps3)) {
			// no status change and status messages are not supported
			log_debug(0, "CommonStatus: no change, %s (%d %d)", p->m_szName, oldstatus, newstatus);
			continue;
		}

		// set status message first
		if (b_Caps1 && b_Caps3)
			SetStatusMsg(p, newstatus);

		// set the status
		if (newstatus != oldstatus /*&& !(b_Caps1 && b_Caps3 && ServiceExists(MS_NAS_SETSTATE))*/) {
			log_debug(0, "CommonStatus sets status for %s to %d", p->m_szName, newstatus);
			CallProtoService(p->m_szName, PS_SETSTATUS, newstatus, 0);
		}
	}

	if (globStatus != 0) {
		log_debug(0, "CommonStatus: setting global status %u", globStatus);
		Clist_SetStatusMode(globStatus);
	}

	return 0;
}

static INT_PTR GetProtocolCountService(WPARAM, LPARAM)
{
	int pCount = 0;

	for (auto &pa : Accounts())
		if (IsSuitableProto(pa))
			pCount++;

	return pCount;
}

bool IsSuitableProto(PROTOACCOUNT *pa)
{
	return (pa == nullptr) ? false : pa->IsVisible();
}

static int onShutdown(WPARAM, LPARAM)
{
	g_plugin.bMirandaLoaded = false;
	DestroyHookableEvent(hCSStatusChangedExEvent);
	return 0;
}

static INT_PTR SetStatusEx(WPARAM wParam, LPARAM pCount)
{
	PROTOCOLSETTINGEX **protoSettings = *(PROTOCOLSETTINGEX***)wParam;
	if (protoSettings == nullptr)
		return -1;

	TProtoSettings ps;
	for (int i = 0; i < pCount; i++)
		ps.insert((SMProto*)protoSettings[i]);
	return SetStatusEx(ps);
}

int InitCommonStatus()
{
	hCSStatusChangedExEvent = CreateHookableEvent(ME_CS_STATUSCHANGEEX);

	CreateServiceFunction(MS_CS_SETSTATUSEX, SetStatusEx);
	CreateServiceFunction(MS_CS_GETPROTOCOUNT, GetProtocolCountService);
	
	HookEvent(ME_SYSTEM_PRESHUTDOWN, onShutdown);
	return 0;
}
