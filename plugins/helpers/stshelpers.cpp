#include "commonheaders.h"
#include "gen_helpers.h"
#include <m_statusplugins.h>
#include "stshelpers.h"

static struct ProtoInfo {
	const char *szProto;
	UINT idEvent;
};

// note: accesses to pi are not synchronized... 'cause I'm a lazy SOB (and it's not necessary ATM)
static struct ProtoInfo *pi = NULL;
static int piCount = 0;
static UINT g_idEvent = 0;
static HWND hMessageWindow = NULL;
static HANDLE hProtoAckHook = NULL;

static VOID CALLBACK EnableProtoTimer(HWND hwnd, UINT message, UINT idEvent, DWORD dwTime) {

	int i;

	KillTimer(hwnd, idEvent);
	log_debugA("EnableProtoTimer");
	for (i=0;i<piCount;i++) {
		if (pi[i].idEvent == idEvent) {
			log_debugA("cleaning timer %d (%d) for %s (%x) (%d) (%d events left)", i, idEvent, pi[i].szProto, pi[i].szProto, pi[i].idEvent, piCount);
			if (i < piCount-1) {
				log_debugA("(1) pi[%d] (%s %x) pi[%d] (%s %x )", i, pi[i].szProto, pi[i].szProto, i+1, pi[i+1].szProto, pi[i+1].szProto);
				MoveMemory(&pi[i], &pi[i+1], (piCount-i-1)*sizeof(struct ProtoInfo));
				log_debugA("(2) pi[%d] (%s %x) pi[%d] (%s %x )", i, pi[i].szProto, pi[i].szProto, i+1, pi[i+1].szProto, pi[i+1].szProto);
			}
			if (piCount > 1) {
				pi = realloc(pi, (piCount-1)*sizeof(struct ProtoInfo));
			}
			else {
				free(pi);
				pi = NULL;
				log_debugA("(3) pi freed");
			}
			piCount -= 1;
		}
	}
}

static int ProtoAck(WPARAM wParam, LPARAM lParam) {

	int status;
	ACKDATA *ack;
	
	ack = (ACKDATA*)lParam;
	if (ack->type != ACKTYPE_STATUS) return 0;
	if (ack->result != ACKRESULT_SUCCESS) return 0;
	log_debugA("StatusHandling ProtoAck");
	status = CallProtoService(ack->szModule, PS_GETSTATUS, 0, 0);
	log_debugA("StatusHandling ProtoAck status=%d", status);
	if ( (status < MIN_STATUS) || (status > MAX_STATUS) ) {
		return 0;
	}	
	pi = realloc(pi, (piCount+1)*sizeof(struct ProtoInfo));
	g_idEvent += 1;
	pi[piCount].idEvent = g_idEvent;
	pi[piCount].szProto = ack->szModule;
	piCount += 1;
	log_debugA("added timer %d for pi[%d] %s (%x)", pi[piCount-1].idEvent, piCount-1, ack->szModule, ack->szModule);
	SetTimer(hMessageWindow, g_idEvent, CONNECTIONTIMEOUT, EnableProtoTimer);

	return 0;
}

int InitProtoStatusChangedHandling() {

	if (hMessageWindow == NULL) {
		int i, count;
		PROTOCOLDESCRIPTOR **protos;

		CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);
		for(i=0;i<count;i++) {
			if (protos[i]->type!=PROTOTYPE_PROTOCOL || CallProtoService(protos[i]->szName,PS_GETCAPS,PFLAGNUM_2,0)==0) continue;
			//pi = realloc(pi, (piCount + 1)*sizeof(struct ProtoInfo));
			//ZeroMemory(&pi[piCount], sizeof(struct ProtoInfo));
			//pi[piCount].szProto = protos[i].szName;
			log_debugA("InitProtoStatusChangedHandling: %s added", protos[i]->szName);
		}
		hMessageWindow = CreateWindowExA(0, "STATIC", NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
		hProtoAckHook = HookEvent(ME_PROTO_ACK, ProtoAck);

		return 0;
	}

	return 1;
}

int ProtoChangedStatus(char *szProto) {

	int i;

	log_debugA("ProtoChangedStatus %s", szProto);
	if (szProto != NULL) {
		for (i=0;i<piCount;i++) {
			if ( (pi[i].szProto != NULL) && (szProto != NULL) && (!strcmp(pi[i].szProto, szProto)) ) {
				log_debugA("%s just changed status", szProto);
				
				return TRUE;
			}
		}
	}

	return FALSE;
}

int Flag2Status(unsigned long flag) {

	switch(flag) {
	case PF2_ONLINE: return ID_STATUS_ONLINE;
	case PF2_OFFLINE: 
	case 0: return ID_STATUS_OFFLINE;
	case PF2_INVISIBLE: return ID_STATUS_INVISIBLE;
	case PF2_OUTTOLUNCH: return ID_STATUS_OUTTOLUNCH;
	case PF2_ONTHEPHONE: return ID_STATUS_ONTHEPHONE;
	case PF2_SHORTAWAY: return ID_STATUS_AWAY;
	case PF2_LONGAWAY: return ID_STATUS_NA;
	case PF2_LIGHTDND: return ID_STATUS_OCCUPIED;
	case PF2_HEAVYDND: return ID_STATUS_DND;
	case PF2_FREECHAT: return ID_STATUS_FREECHAT;
	case PF2_IDLE: return ID_STATUS_IDLE;
	case PF2_CURRENT: return ID_STATUS_CURRENT;
	case PF2_FROMTRIGGER: return ID_STATUS_FROMTRIGGER;
	}
	return 0;
}

unsigned long Status2Flag(int status) {

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
		case ID_STATUS_IDLE: return PF2_IDLE;
		case ID_STATUS_CURRENT: return PF2_CURRENT;
		case ID_STATUS_FROMTRIGGER: return PF2_FROMTRIGGER;
	}
	return 0;
}

static int GetActualStatus(SETSTATUSINFO *ssi) {

	if ( (ssi->status == ID_STATUS_CURRENT) || (ssi->flags&SSIF_DONTSETSTATUS) ) {
		return CallProtoService(ssi->szProto, PS_GETSTATUS, 0, 0);
	}

	return ssi->status;
}

static int EqualsGlobalStatus(SETSTATUSINFO *ssi, int count) {

	PROTOCOLDESCRIPTOR **protos;
	int i, j, protoCount, pstatus, gstatus;

	if ( (count == 1) && (ssi[0].flags&SSIF_USEAWAYSYS) && (ssi[0].szProto == NULL) ) {
		return ssi[0].status;
	}
	pstatus = gstatus = 0;
	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&protoCount,(LPARAM)&protos);
	for (i=0;i<protoCount;i++) {
		if ( (protos[i]->type != PROTOTYPE_PROTOCOL) || (CallProtoService(ssi[i].szProto, PS_GETCAPS, PFLAGNUM_2, 0) == 0)) continue;
		pstatus = 0;
		for (j=0;j<count;j++) {
			if (!strcmp(protos[i]->szName, ssi[j].szProto)) {
				if (ssi[j].flags&SSIF_DONTSETSTATUS) {
					// not all proto's will be set
					return 0;
				}
				if (!(ssi[j].flags&SSIF_USEAWAYSYS)) {
					// not all proto's handled by awaysys
					return 0;
				}
				pstatus = GetActualStatus(&ssi[j]);
			}
			else {
				// not all proto's will be set
				return 0;
			}
		}
		if (pstatus == 0) {
			pstatus = CallProtoService(protos[i]->szName, PS_GETSTATUS, 0, 0);
		}			
		if (gstatus == 0) {
			gstatus = pstatus;
		}
		if (pstatus != gstatus) {
			return 0;
		}
	}

	return gstatus;
}

// helper, from core
static char *GetDefaultMessage(int status)
{
	switch(status) {
		case ID_STATUS_AWAY: return Translate("I've been away since %time%.");
		case ID_STATUS_NA: return Translate("Give it up, I'm not in!");
		case ID_STATUS_OCCUPIED: return Translate("Not right now.");
		case ID_STATUS_DND: return Translate("Give a guy some peace, would ya?");
		case ID_STATUS_FREECHAT: return Translate("I'm a chatbot!");
		case ID_STATUS_ONLINE: return Translate("Yep, I'm here.");
		case ID_STATUS_OFFLINE: return Translate("Nope, not here.");
		case ID_STATUS_INVISIBLE: return Translate("I'm hiding from the mafia.");
		case ID_STATUS_ONTHEPHONE: return Translate("That'll be the phone.");
		case ID_STATUS_OUTTOLUNCH: return Translate("Mmm...food.");
		case ID_STATUS_IDLE: return Translate("idleeeeeeee");
	}
	return NULL;
}

static char *GetDefaultStatusMessage(int newstatus) {

	char *sMsg, *tMsg;

	sMsg = NULL;
	if (ServiceExists(MS_AWAYMSG_GETSTATUSMSG)) {
		tMsg = (char*)CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM)newstatus, 0);
		if (tMsg != NULL) {
			sMsg = _strdup(tMsg);
			mir_free(tMsg);
		}
	}
	else {
		tMsg = GetDefaultMessage(newstatus); /* awaysys doesn't define the service above */
		if (tMsg != NULL) {
			sMsg = _strdup(tMsg);
		}
	}
	
	return sMsg;
}

int Hlp_SetStatus(SETSTATUSINFO *ssi, int count) {

	int i, status;
	char *szMsg;

	status = EqualsGlobalStatus(ssi, count);
	if (status != 0) {
		log_debugA("Hlp_SetStatus: Setting global status");
		CallService(MS_CLIST_SETSTATUSMODE, (WPARAM)status, 0);
		return 0;
	}
	for (i=0;i<count;i++) {
		if (!CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)ssi[i].szProto)) {
			log_debugA("Hlp_SetStatus: %s not loaded", ssi[i].szProto);
			continue;
		}
		status = GetActualStatus(&ssi[i]);
		if (!(ssi[i].flags&SSIF_DONTSETSTATUS)) {
			if (ServiceExists(MS_KS_ANNOUNCESTATUSCHANGE)) {
				log_debugA("Hlp_SetStatus: Announcing status change to KeepStatus");
				announce_status_change(ssi[i].szProto, status, ssi[i].flags&SSIF_DONTSETMESSAGE?NULL:ssi[i].szMsg);
			}
			log_debugA("Hlp_SetStatus: Setting status %d for %s", status, ssi[i].szProto);
			CallProtoService(ssi[i].szProto, PS_SETSTATUS, (WPARAM)status, 0);
		}
		if (!(ssi[i].flags&SSIF_DONTSETMESSAGE)) {
			if (ssi[i].flags&SSIF_USEAWAYSYS) {
				szMsg = GetDefaultStatusMessage(status);
			}
			else {
				if (ssi[i].szMsg != NULL) {
					szMsg = _strdup(ssi[i].szMsg);
				}
				else {
					szMsg = NULL;
				}
			}
			log_debugA("Hlp_SetStatus: Setting statusmessage for %d for %s", status, ssi[i].szProto);
			CallProtoService(ssi[i].szProto, PS_SETAWAYMSG, (WPARAM)status, (LPARAM)szMsg);
			if (szMsg != NULL) {
				free(szMsg);
			}
		}
	}

	return 0;
}

void Hlp_FreeStatusInfo(SETSTATUSINFO *ssi, int count) {

	int i;

	for (i=0;i<count;i++) {
		if (ssi[i].szMsg != NULL) {
			free(ssi[i].szMsg);
		}
	}
}