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
#include "commonstatus.h"

// handles for hooks and other Miranda thingies
static HANDLE hCSStatusChangedExEvent;

OBJLIST<PROTOCOLSETTINGEX> *protoList;

// prototypes
char* StatusModeToDbSetting(int status, const char *suffix);
DWORD StatusModeToProtoFlag(int status);
INT_PTR SetStatusEx(WPARAM wParam, LPARAM lParam);
int InitCommonStatus();
int GetProtoCount();

// extern
extern INT_PTR ShowConfirmDialogEx(WPARAM wParam, LPARAM lParam);

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
	default: return NULL;
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
	if (protoSetting->status == ID_STATUS_LAST) {
		if ((protoSetting->lastStatus < MIN_STATUS) || (protoSetting->lastStatus > MAX_STATUS))
			return CallProtoService(protoSetting->szName, PS_GETSTATUS, 0, 0);
		return protoSetting->lastStatus;
	}
	if (protoSetting->status == ID_STATUS_CURRENT)
		return CallProtoService(protoSetting->szName, PS_GETSTATUS, 0, 0);

	if ((protoSetting->status < ID_STATUS_OFFLINE) || (protoSetting->status > ID_STATUS_OUTTOLUNCH)) {
		log_debugA("invalid status detected: %d", protoSetting->status);
		return 0;
	}
	return protoSetting->status;
}

// helper, from core
static TCHAR* GetDefaultMessage(int status)
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
	case ID_STATUS_ONTHEPHONE: return TranslateT("That'll be the phone.");
	case ID_STATUS_OUTTOLUNCH: return TranslateT("Mmm... food.");
	case ID_STATUS_IDLE:       return TranslateT("idleeeeeeee");
	}
	return NULL;
}

TCHAR* GetDefaultStatusMessage(PROTOCOLSETTINGEX *ps, int newstatus)
{
	if (ps->szMsg != NULL) {// custom message set
		log_infoA("CommonStatus: Status message set by calling plugin");
		return mir_tstrdup(ps->szMsg);
	}

	TCHAR *tMsg = (TCHAR*)CallService(MS_AWAYMSG_GETSTATUSMSGT, newstatus, (LPARAM)ps->szName);
	log_debugA("CommonStatus: Status message retrieved from general awaysys: %S", tMsg);
	return tMsg;
}

static int equalsGlobalStatus(PROTOCOLSETTINGEX **ps)
{

	int i, j, pstatus = 0, gstatus = 0;

	for (i = 0; i < protoList->getCount(); i++)
		if (ps[i]->szMsg != NULL && GetActualStatus(ps[i]) != ID_STATUS_OFFLINE)
			return 0;

	int count;
	PROTOACCOUNT **protos;
	ProtoEnumAccounts(&count, &protos);

	for (i = 0; i < count; i++) {
		if (!IsSuitableProto(protos[i]))
			continue;

		pstatus = 0;
		for (j = 0; j < protoList->getCount(); j++)
			if (!strcmp(protos[i]->szModuleName, ps[j]->szName))
				pstatus = GetActualStatus(ps[j]);

		if (pstatus == 0)
			pstatus = CallProtoService(protos[i]->szModuleName, PS_GETSTATUS, 0, 0);

		if (db_get_b(NULL, protos[i]->szModuleName, "LockMainStatus", 0)) {
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
	TCHAR* tszMsg = GetDefaultStatusMessage(ps, newstatus);
	if (tszMsg) {
		/* replace the default vars in msg  (I believe this is from core) */
		for (int j = 0; tszMsg[j]; j++) {
			if (tszMsg[j] != '%')
				continue;

			TCHAR substituteStr[128];
			if (!_tcsnicmp(tszMsg + j, _T("%time%"), 6))
				GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, 0, 0, substituteStr, SIZEOF(substituteStr));
			else if (!_tcsnicmp(tszMsg + j, _T("%date%"), 6))
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, 0, 0, substituteStr, SIZEOF(substituteStr));
			else
				continue;

			if (mir_tstrlen(substituteStr) > 6)
				tszMsg = (TCHAR*)mir_realloc(tszMsg, sizeof(TCHAR)*(mir_tstrlen(tszMsg) + 1 + mir_tstrlen(substituteStr) - 6));
			memmove(tszMsg + j + mir_tstrlen(substituteStr), tszMsg + j + 6, sizeof(TCHAR)*(mir_tstrlen(tszMsg) - j - 5));
			memcpy(tszMsg + j, substituteStr, sizeof(TCHAR)*mir_tstrlen(substituteStr));
		}

		TCHAR *szFormattedMsg = variables_parsedup(tszMsg, ps->tszAccName, NULL);
		if (szFormattedMsg != NULL) {
			mir_free(tszMsg);
			tszMsg = szFormattedMsg;
		}
	}
	log_debugA("CommonStatus sets status message for %s directly", ps->szName);
	if (CALLSERVICE_NOTFOUND == CallProtoService(ps->szName, PS_SETAWAYMSGT, newstatus, (LPARAM)tszMsg)) {
		char* sMsg = mir_t2a(tszMsg);
		CallProtoService(ps->szName, PS_SETAWAYMSG, newstatus, (LPARAM)sMsg);
		mir_free(sMsg);
	}
	mir_free(tszMsg);
}

INT_PTR SetStatusEx(WPARAM wParam, LPARAM)
{
	PROTOCOLSETTINGEX** protoSettings = *(PROTOCOLSETTINGEX***)wParam;
	if (protoSettings == NULL)
		return -1;

	int globStatus = equalsGlobalStatus(protoSettings);

	// issue with setting global status;
	// things get messy because SRAway hooks ME_CLIST_STATUSMODECHANGE, so the status messages of SRAway and
	// commonstatus will clash
	NotifyEventHooks(hCSStatusChangedExEvent, (WPARAM)&protoSettings, protoList->getCount());

	// set all status messages first
	for (int i = 0; i < protoList->getCount(); i++) {
		char *szProto = protoSettings[i]->szName;
		if (!ProtoGetAccount(szProto)) {
			log_debugA("CommonStatus: %s is not loaded", szProto);
			continue;
		}
		// some checks
		int newstatus = GetActualStatus(protoSettings[i]);
		if (newstatus == 0) {
			log_debugA("CommonStatus: incorrect status for %s (%d)", szProto, protoSettings[i]->status);
			continue;
		}
		int oldstatus = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
		// set last status
		protoSettings[i]->lastStatus = oldstatus;
		if (oldstatus <= MAX_CONNECT_RETRIES) {// ignore if connecting, but it didn't came this far if it did
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
		if (!ServiceExists(MS_CLIST_SETSTATUSMODE)) {
			log_debugA("CommonStatus: MS_CLIST_SETSTATUSMODE not available!");
			return -1;
		}
		log_debugA("CommonStatus: setting global status %u", globStatus);
		CallService(MS_CLIST_SETSTATUSMODE, globStatus, 0);
	}

	return 0;
}

static INT_PTR GetProtocolCountService(WPARAM, LPARAM)
{
	return GetProtoCount();
}

bool IsSuitableProto(PROTOACCOUNT *pa)
{
	return (pa == NULL) ? false : (pcli->pfnGetProtocolVisibility(pa->szModuleName) != 0);
}

int GetProtoCount()
{
	int pCount = 0, count;
	PROTOACCOUNT **accs;
	ProtoEnumAccounts(&count, &accs);

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
	CreateServiceFunction(MS_CS_SHOWCONFIRMDLGEX, ShowConfirmDialogEx);
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
