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
static HANDLE hShutdownHook;
static HANDLE hCSSetStatusExService;
static HANDLE hCSShowConfirmDlgExService;
static HANDLE hCSStatusChangedExEvent;
static HANDLE hCSGetProtoCountService;

OBJLIST<PROTOCOLSETTINGEX>* protoList;

// prototypes
char *StatusModeToDbSetting(int status,const char *suffix);
DWORD StatusModeToProtoFlag(int status);
INT_PTR SetStatusEx(WPARAM wParam, LPARAM lParam);
int InitCommonStatus();
static int ModulesLoaded(WPARAM wParam, LPARAM lParam);
static int CreateServices();
int GetProtoCount();
static int Exit(WPARAM wParam, LPARAM lParam);
// extern
extern INT_PTR ShowConfirmDialogEx(WPARAM wParam, LPARAM lParam);

// some helpers from awaymsg.c ================================================================
char *StatusModeToDbSetting(int status,const char *suffix)
{
	char *prefix;
	static char str[64];

	switch(status) {
		case ID_STATUS_AWAY: prefix="Away";	break;
		case ID_STATUS_NA: prefix="Na";	break;
		case ID_STATUS_DND: prefix="Dnd"; break;
		case ID_STATUS_OCCUPIED: prefix="Occupied"; break;
		case ID_STATUS_FREECHAT: prefix="FreeChat"; break;
		case ID_STATUS_ONLINE: prefix="On"; break;
		case ID_STATUS_OFFLINE: prefix="Off"; break;
		case ID_STATUS_INVISIBLE: prefix="Inv"; break;
		case ID_STATUS_ONTHEPHONE: prefix="Otp"; break;
		case ID_STATUS_OUTTOLUNCH: prefix="Otl"; break;
		default: return NULL;
	}
	lstrcpyA(str,prefix); lstrcatA(str,suffix);
	return str;
}

DWORD StatusModeToProtoFlag(int status)
{
	// *not* the same as in core, <offline>
	switch(status) {
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
		if ( (protoSetting->lastStatus < MIN_STATUS) || (protoSetting->lastStatus > MAX_STATUS) )
			return CallProtoService(protoSetting->szName, PS_GETSTATUS, 0, 0);
		return protoSetting->lastStatus;
	}
	if (protoSetting->status == ID_STATUS_CURRENT)
		return CallProtoService(protoSetting->szName, PS_GETSTATUS, 0, 0);

	if ( (protoSetting->status < ID_STATUS_OFFLINE) || (protoSetting->status > ID_STATUS_OUTTOLUNCH) ) {
		log_debugA("invalid status detected: %d", protoSetting->status);
		return 0;
	}
	return protoSetting->status;
}

// helper, from core
static TCHAR *GetDefaultMessage(int status)
{
	switch(status) {
		case ID_STATUS_AWAY:       return TranslateT("I've been away since %time%.");
		case ID_STATUS_NA:         return TranslateT("Give it up, I'm not in!");
		case ID_STATUS_OCCUPIED:   return TranslateT("Not right now.");
		case ID_STATUS_DND:        return TranslateT("Give a guy some peace, would ya?");
		case ID_STATUS_FREECHAT:   return TranslateT("I'm a chatbot!");
		case ID_STATUS_ONLINE:     return TranslateT("Yep, I'm here.");
		case ID_STATUS_OFFLINE:    return TranslateT("Nope, not here.");
		case ID_STATUS_INVISIBLE:  return TranslateT("I'm hiding from the mafia.");
		case ID_STATUS_ONTHEPHONE: return TranslateT("That'll be the phone.");
		case ID_STATUS_OUTTOLUNCH: return TranslateT("Mmm...food.");
		case ID_STATUS_IDLE:       return TranslateT("idleeeeeeee");
	}
	return NULL;
}

TCHAR *GetDefaultStatusMessage(PROTOCOLSETTINGEX *ps, int newstatus)
{
	if (ps->szMsg != NULL) {// custom message set
		log_infoA("CommonStatus: Status message set by calling plugin");
		return mir_a2t( ps->szMsg );
	}

	if ( ServiceExists( MS_AWAYMSG_GETSTATUSMSGT )) {
		TCHAR* tMsg = ( TCHAR* )CallService( MS_AWAYMSG_GETSTATUSMSGT, newstatus, (LPARAM)ps->szName );
		log_debugA("CommonStatus: Status message retrieved from general awaysys (TCHAR)");
		return tMsg;
	}

	if ( ServiceExists(MS_AWAYMSG_GETSTATUSMSG)) {
		char *tMsg;
		if ( ServiceExists(MS_SA_ISSARUNNING) && CallService(MS_SA_ISSARUNNING, 0, 0))
			tMsg = (char*)CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM)newstatus, (LPARAM)ps->szName);
		else
      	tMsg = (char*)CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM)newstatus, 0);

		log_debugA("CommonStatus: Status message retrieved from general awaysys");

		TCHAR* result = mir_a2t( tMsg );
		mir_free(tMsg);
		return result;
	}

	/* awaysys doesn't define the service above */
	TCHAR* tMsg = GetDefaultMessage(newstatus);
	if (tMsg != NULL) {
		log_debugA("CommonStatus: Status message retrieved from defaults");
		return mir_tstrdup(tMsg);
	}

	return NULL;
}

static int equalsGlobalStatus(PROTOCOLSETTINGEX **ps) {

	int i, j, pstatus = 0, gstatus = 0;

	for ( i=0; i < protoList->getCount(); i++ )
		if ( ps[i]->szMsg != NULL && GetActualStatus(ps[i]) != ID_STATUS_OFFLINE )
			return 0;

	int count;
	PROTOACCOUNT** protos;
	ProtoEnumAccounts( &count, &protos );

	for ( i=0; i < count; i++ ) {
		if ( !IsSuitableProto( protos[i] ))
			continue;

		pstatus = 0;
		for ( j=0; j < protoList->getCount(); j++ )
			if (!strcmp(protos[i]->szModuleName, ps[j]->szName))
				pstatus = GetActualStatus(ps[j]);

		if (pstatus == 0)
			pstatus = CallProtoService(protos[i]->szModuleName, PS_GETSTATUS, 0, 0);

		if (DBGetContactSettingByte(NULL, protos[i]->szModuleName, "LockMainStatus", 0)) {
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

static int nasSetStatus(PROTOCOLSETTINGEX **protoSettings, int newstatus)
{
	NAS_PROTOINFO npi;
	char **nasProtoMessages, *nasGlobalMsg;
	int i, j, msgCount, maxMsgCount;

	if (!ServiceExists(MS_NAS_SETSTATE))
		return -1;

	nasGlobalMsg = NULL;
	msgCount = maxMsgCount = 0;
	if (newstatus == 0) {
	/*
	The global status will not be changed. Still, we'd like to set the global status message for NAS.
	This should only be done if all protocols will change in this call. Otherwise, the chance exists
	that an already set status message will be overwritten. The global status message will be set for
	the status message which is to be set for most of the protocols. Protocols are only considered
	which have szMsg==NULL. After the global status message is set, the protocols for which another
	message and/or status have to be set will be overwritten.
		*/
		ZeroMemory(&npi, sizeof(NAS_PROTOINFO));
		npi.cbSize = sizeof(NAS_PROTOINFO);
		npi.status = 0;
		npi.szProto = NULL;
		CallService(MS_NAS_GETSTATEA, (WPARAM)&npi, (LPARAM)1);
		nasProtoMessages = ( char** )calloc(protoList->getCount(),sizeof(char *));
		if (nasProtoMessages == NULL) {
			return -1;
		}
		// fill the array of proto message for NAS, this will be used anyway
		for (i=0;i<protoList->getCount();i++) {
			if ( (!CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)protoSettings[i]->szName)) || (protoSettings[i]->szMsg != NULL) ) {
				continue;
			}
			ZeroMemory(&npi, sizeof(NAS_PROTOINFO));
			npi.cbSize = sizeof(NAS_PROTOINFO);
			npi.status = GetActualStatus(protoSettings[i]);
			npi.szProto = protoSettings[i]->szName;
			if (CallService(MS_NAS_GETSTATEA, (WPARAM)&npi, (LPARAM)1) == 0) {
				nasProtoMessages[i] = npi.szMsg;
			}
		}
		// if not all proto's are to be set here, we don't set the global status message
		for (i=0;i<protoList->getCount();i++) {
			if (!CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)protoSettings[i]->szName)) {
				break;
			}
		}
		if (i == protoList->getCount()) {
			for (i=0;i<protoList->getCount();i++) {
				msgCount = 0;
				for (j=i;j<protoList->getCount();j++) {
					if ( (nasProtoMessages[i] != NULL) && (nasProtoMessages[j] != NULL) && (!strcmp(nasProtoMessages[i], nasProtoMessages[j])) ) {
						msgCount += 1;
						//log_infoA("Adding %s (%u) to %s (%u)", protoSettings[j]->szName, protoSettings[j]->status, protoSettings[i]->szName, protoSettings[i]->status);
					}
				}
				if ( (msgCount > maxMsgCount) && ((protoList->getCount() == 1) || (msgCount > 1)) ) {
					maxMsgCount = msgCount;
					nasGlobalMsg = _strdup(nasProtoMessages[i]);
				}
			}
		}
		if (nasGlobalMsg != NULL) {
			// set global message
			ZeroMemory(&npi, sizeof(NAS_PROTOINFO));
			npi.cbSize = sizeof(NAS_PROTOINFO);
			npi.status = 0; // status is not important for global message
			npi.szMsg = mir_strdup(nasGlobalMsg);
			npi.Flags |= PIF_NO_CLIST_SETSTATUSMODE;
			log_infoA("CommonStatus sets global status message for NAS which is to be changed for %d protocols", maxMsgCount);
			CallService(MS_NAS_SETSTATEA, (WPARAM)&npi, (LPARAM)1);
		}
		for (i=0;i<protoList->getCount();i++) {
			if (!CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)protoSettings[i]->szName))
				continue;

			ZeroMemory(&npi, sizeof(NAS_PROTOINFO));
			npi.cbSize = sizeof(NAS_PROTOINFO);
			npi.status = GetActualStatus(protoSettings[i]);
			npi.szProto = protoSettings[i]->szName;
			if (protoSettings[i]->szMsg != NULL) {
				npi.szMsg = mir_strdup(protoSettings[i]->szMsg);
				log_infoA("CommonStatus will set status %u for %s and message specified by plugin using NAS (%x)", npi.status, npi.szProto, npi.szMsg);
			}
			else if ( (nasProtoMessages[i] != NULL) && (nasGlobalMsg != NULL) && (!strcmp(nasProtoMessages[i], nasGlobalMsg)) ) {
				npi.szMsg = NULL;
				log_infoA("CommonStatus will set status %u for %s and global message using NAS", npi.status, npi.szProto);
			}
			else {
				npi.szMsg = nasProtoMessages[i];
				log_infoA("CommonStatus will set status %u for %s and message from NAS using NAS", npi.status, npi.szProto);
			}
			CallService(MS_NAS_SETSTATEA, (WPARAM)&npi, (LPARAM)1);
		}
		if (nasGlobalMsg != NULL) {
			free(nasGlobalMsg);
		}
		free(nasProtoMessages);
	}
	else {
		ZeroMemory(&npi, sizeof(NAS_PROTOINFO));
		npi.cbSize = sizeof(NAS_PROTOINFO);
		npi.szProto = NULL; // global
		npi.szMsg = NULL; // global
		npi.status = newstatus;
		log_debugA("CommonStatus sets global status %u using NAS", newstatus);
		CallService(MS_NAS_SETSTATEA, (WPARAM)&npi, (LPARAM)1);
	}

	return 0;
}

static void SetStatusMsg(PROTOCOLSETTINGEX *ps, int newstatus)
{
	TCHAR* tszMsg = GetDefaultStatusMessage( ps, newstatus );
	if ( tszMsg ) {
		if ( ServiceExists( MS_VSRAMM_SETAWAYMSG )) {
			PROTOMSGINFO pmi;

			pmi.statusMode = ps->status;
			pmi.szProto = ps->szName;
			pmi.msg = mir_t2a( tszMsg );
			log_debugA("CommonStatus sets status message for %s using VSRAMM", ps->szName);
			CallService( MS_VSRAMM_SETAWAYMSG, 0, (LPARAM)&pmi );
			mir_free( pmi.msg );
		}
		else {
			/* replace the default vars in msg  (I believe this is from core) */
			for ( int j=0; tszMsg[j]; j++ ) {
				TCHAR substituteStr[128];

				if ( tszMsg[j] != '%' )
					continue;

				if ( !_tcsnicmp( tszMsg+j, _T("%time%"), 6 ))
					GetTimeFormat( LOCALE_USER_DEFAULT, TIME_NOSECONDS, 0, 0, substituteStr, SIZEOF(substituteStr));
				else if ( !_tcsnicmp( tszMsg+j, _T("%date%"), 6 ))
					GetDateFormat( LOCALE_USER_DEFAULT, DATE_SHORTDATE, 0, 0, substituteStr, SIZEOF(substituteStr));
				else
					continue;

				if ( lstrlen( substituteStr ) > 6 )
					tszMsg = (TCHAR*)mir_realloc(tszMsg, sizeof(TCHAR)*(lstrlen(tszMsg)+1+lstrlen(substituteStr)-6));
				MoveMemory( tszMsg + j + lstrlen(substituteStr), tszMsg+j+6, sizeof(TCHAR)*(lstrlen(tszMsg)-j-5));
				CopyMemory( tszMsg + j, substituteStr, sizeof(TCHAR)*lstrlen( substituteStr ));
			}

			TCHAR* szFormattedMsg = variables_parsedup(tszMsg, ps->tszAccName, NULL);
			if (szFormattedMsg != NULL) {
				mir_free( tszMsg );
				tszMsg = mir_tstrdup( szFormattedMsg );
				free( szFormattedMsg );
			}
		}

	}
	log_debugA("CommonStatus sets status message for %s directly", ps->szName);
	if ( CALLSERVICE_NOTFOUND == CallProtoService(ps->szName, PS_SETAWAYMSGT, newstatus, (LPARAM)tszMsg )) {
		char* sMsg = mir_t2a( tszMsg );
		CallProtoService(ps->szName, PS_SETAWAYMSG, newstatus, (LPARAM)sMsg );
		mir_free( sMsg );
	}
	mir_free( tszMsg );
}

INT_PTR SetStatusEx(WPARAM wParam, LPARAM lParam)
{
	PROTOCOLSETTINGEX** protoSettings = *(PROTOCOLSETTINGEX***)wParam;
	if ( protoSettings == NULL )
		return -1;

	int globStatus = equalsGlobalStatus( protoSettings );

	/*
		issue with setting global status;
		things get messy because SRAway hooks ME_CLIST_STATUSMODECHANGE, so the status messages of SRAway and
		commonstatus will clash
	*/
	NotifyEventHooks( hCSStatusChangedExEvent, ( WPARAM )&protoSettings, 0 );

	// set all status messages first
	for ( int i=0; i < protoList->getCount(); i++ ) {
		char* szProto = protoSettings[i]->szName;
		if ( !CallService( MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)szProto )) {
			log_debugA( "CommonStatus: %s is not loaded", szProto );
			continue;
		}
		// some checks
		int newstatus = GetActualStatus( protoSettings[i] );
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
		long protoFlag = Proto_Status2Flag( newstatus );
		int b_Caps2 = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_2, 0) & protoFlag;
		int b_Caps5 = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_5, 0) & protoFlag;
		if ( newstatus != ID_STATUS_OFFLINE && ( !b_Caps2 || b_Caps5 )) {
			// status and status message for this status not supported
			//log_debug("CommonStatus: status not supported %s", szProto);
			continue;
		}

		int b_Caps1 = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND & ~PF1_INDIVMODEMSG;
		int b_Caps3 = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & protoFlag;
		if  ( newstatus == oldstatus && ( !b_Caps1 || !b_Caps3 )) {
			// no status change and status messages are not supported
			//log_debug("CommonStatus: no change, %s (%d %d)", szProto, oldstatus, newstatus);
			continue;
		}

		// set status message first
		if ( b_Caps1 && b_Caps3 )
			SetStatusMsg( protoSettings[i], newstatus );

		// set the status
		if ( newstatus != oldstatus && !(b_Caps1 && b_Caps3 && ServiceExists( MS_NAS_SETSTATE ))) {
			log_debugA("CommonStatus sets status for %s to %d", szProto, newstatus);
			CallProtoService(szProto, PS_SETSTATUS, (WPARAM)newstatus, 0);
	}	}

	// and finally set a status
	if ( !nasSetStatus( protoSettings, globStatus ))
		return 0;

	if ( globStatus != 0 ) {
		if ( !ServiceExists( MS_CLIST_SETSTATUSMODE )) {
			log_debugA("CommonStatus: MS_CLIST_SETSTATUSMODE not available!");
			return -1;
		}
		log_debugA("CommonStatus: setting global status %u", globStatus);
		CallService(MS_CLIST_SETSTATUSMODE, (WPARAM)globStatus, 0);
	}

	return 0;
}

static INT_PTR GetProtocolCountService(WPARAM wParam, LPARAM lParam)
{
	return GetProtoCount();
}

bool IsSuitableProto( PROTOACCOUNT* pa )
{
	if ( pa == NULL )
		return false;

	if ( pa->bDynDisabled || !pa->bIsEnabled )
		return false;

	if ( CallProtoService( pa->szProtoName, PS_GETCAPS, PFLAGNUM_2, 0 ) == 0 )
		return false;

	return true;
}

int GetProtoCount()
{
	int pCount = 0;

	int count;
	PROTOACCOUNT** protos;
	ProtoEnumAccounts( &count, &protos );

	for( int i=0; i < count; i++ )
		if ( IsSuitableProto( protos[i] ))
			pCount ++;

	return pCount;
}

int InitCommonStatus()
{
	if (!CreateServices())
	  	hShutdownHook = HookEvent(ME_SYSTEM_OKTOEXIT, Exit);

	return 0;
}

static int CreateServices()
{
	if (ServiceExists(MS_CS_SETSTATUSEX))
		return -1;

	hCSSetStatusExService = CreateServiceFunction(MS_CS_SETSTATUSEX, SetStatusEx);
	hCSShowConfirmDlgExService = CreateServiceFunction(MS_CS_SHOWCONFIRMDLGEX, ShowConfirmDialogEx);
	hCSStatusChangedExEvent = CreateHookableEvent(ME_CS_STATUSCHANGEEX);
	hCSGetProtoCountService = CreateServiceFunction(MS_CS_GETPROTOCOUNT, GetProtocolCountService);
	return 0;
}

static int Exit(WPARAM wParam, LPARAM lParam) {

	UnhookEvent(hShutdownHook);
	if (hCSSetStatusExService != 0) {
		DestroyHookableEvent(hCSStatusChangedExEvent);
		DestroyServiceFunction(hCSSetStatusExService);
		DestroyServiceFunction(hCSShowConfirmDlgExService);
		DestroyServiceFunction(hCSGetProtoCountService);
	}

	return 0;
}
