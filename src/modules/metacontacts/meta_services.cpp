/*
former MetaContacts Plugin for Miranda IM.

Copyright © 2014 Miranda NG Team
Copyright © 2004-07 Scott Ellis
Copyright © 2004 Universite Louis PASTEUR, STRASBOURG.

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

#include "..\..\core\commonheaders.h"

#include <m_nudge.h>

#include "metacontacts.h"

extern "C" MIR_CORE_DLL(void) db_mc_notifyDefChange(WPARAM wParam, LPARAM lParam);

char *pendingACK = 0;    // Name of the protocol in which an ACK is about to come.

int previousMode,        // Previous status of the MetaContacts Protocol
	mcStatus;             // Current status of the MetaContacts Protocol

HANDLE
	hSubcontactsChanged,  // HANDLE to the 'contacts changed' event
	hEventNudge;

UINT_PTR setStatusTimerId = 0;
BOOL firstSetOnline = TRUE; // see Meta_SetStatus function

OBJLIST<MetaSrmmData> arMetaWindows(1, NumericKeySortT);

/** Get the capabilities of the "MetaContacts" protocol.
*
* @param wParam : 	equals to one of the following values :\n
			<tt> PFLAGNUM_1 | PFLAGNUM_2 | PFLAGNUM_3 | PFLAGNUM_4 | PFLAG_UNIQUEIDTEXT | PFLAG_MAXLENOFMESSAGE | PFLAG_UNIQUEIDSETTING </tt>.
* @param lParam :	Allways set to 0.
*
* @return			Depending on the \c WPARAM.
*/
INT_PTR Meta_GetCaps(WPARAM wParam,LPARAM lParam)
{
	switch (wParam) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_CHAT | PF1_FILESEND | PF1_MODEMSGRECV | PF1_NUMERICUSERID;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;

	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;

	case PFLAGNUM_4:
		return PF4_SUPPORTTYPING | PF4_AVATARS | PF4_IMSENDUTF;

	case PFLAGNUM_5:
		return PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;

	case PFLAG_MAXLENOFMESSAGE:
		return 2000;
	}
	return 0;
}

/** Copy the name of the protocole into lParam
* @param wParam : 	max size of the name
* @param lParam :	reference to a char *, which will hold the name
*/

INT_PTR Meta_GetName(WPARAM wParam,LPARAM lParam)
{
	char *name = (char *)Translate(META_PROTO);
	size_t size = min(strlen(name),wParam-1);	// copy only the first size bytes.
	if (strncpy((char *)lParam,name,size)==NULL)
		return 1;
	((char *)lParam)[size]='\0';
	return 0;
}

/** Loads the icon corresponding to the status
* Called by the CList when the status changes.
* @param wParam : 	one of the following values : \n
					<tt>PLI_PROTOCOL | PLI_ONLINE | PLI_OFFLINE</tt>
* @return			an \c HICON in which the icon has been loaded.
*/

INT_PTR Meta_LoadIcon(WPARAM wParam,LPARAM lParam)
{
	UINT id;
	switch (wParam & 0xFFFF) {
	case PLI_PROTOCOL:
		id = IDI_MCMENU;
		break;
	case PLI_ONLINE:
		id = IDI_MCMENU;
		break;
	case PLI_OFFLINE:
		id = IDI_MCMENU;
		break;
	default:
		return 0;
	}

	return (INT_PTR)LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON,
		GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON),
		GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR MetaFilter_RecvMessage(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT *)ccs->lParam;
	DBCachedContact *cc = currDb->m_cache->GetCachedContact(ccs->hContact);
	if (cc == NULL)
		return 0;
	if (cc->IsSub()) {
		MetaSrmmData tmp = { cc->parentID };
		if (MetaSrmmData *p = arMetaWindows.find(&tmp))
			p->m_hSub = cc->contactID;
	}
	CallService(MS_PROTO_CHAINRECV, wParam, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CALLBACK SetStatusThread(HWND hWnd, UINT msg, UINT_PTR id, DWORD dw)
{
	previousMode = mcStatus;

	mcStatus = ID_STATUS_ONLINE;
	ProtoBroadcastAck(META_PROTO, NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS, (HANDLE)previousMode, mcStatus);

	KillTimer(0, setStatusTimerId);
}

/** Changes the status and notifies everybody
* @param wParam : 	The new mode
* @param lParam :	Allways set to 0.
*/

INT_PTR Meta_SetStatus(WPARAM wParam,LPARAM lParam)
{
	// firstSetOnline starts out true - used to delay metacontact's 'onlineness' to prevent double status notifications on startup
	if (mcStatus == ID_STATUS_OFFLINE && firstSetOnline) {
		// causes crash on exit if miranda is closed in under options.set_status_from_offline milliseconds!
		//CloseHandle( CreateThread( NULL, 0, SetStatusThread, (void *)wParam, 0, 0 ));
		setStatusTimerId = SetTimer(0, 0, options.set_status_from_offline_delay, SetStatusThread);
		firstSetOnline = FALSE;
	}
	else {
		previousMode = mcStatus;
		mcStatus = (int)wParam;
		ProtoBroadcastAck(META_PROTO, NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS, (HANDLE)previousMode, mcStatus);
	}
	return 0;
}

/** Returns the current status
*/
INT_PTR Meta_GetStatus(WPARAM wParam,LPARAM lParam)
{
	return mcStatus;
}

//////////////////////////////////////////////////////////
/// Copied from MSN plugin - sent acks need to be from different thread :(
//////////////////////////////////////////////////////////

struct TFakeAckParams
{
	HANDLE hEvent;
	MCONTACT hContact;
	LONG id;
	char msg[512];
};

static DWORD CALLBACK sttFakeAckFail( LPVOID param )
{
	TFakeAckParams *tParam = ( TFakeAckParams* )param;
	WaitForSingleObject( tParam->hEvent, INFINITE );

	Sleep( 100 );
	ProtoBroadcastAck(META_PROTO, tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)tParam->id, (WPARAM)tParam->msg);

	CloseHandle( tParam->hEvent );
	mir_free(tParam);
	return 0;
}

INT_PTR Meta_SendNudge(WPARAM wParam, LPARAM lParam)
{
	DBCachedContact *cc = CheckMeta(wParam);
	if (cc == NULL)
		return 1;

	MCONTACT hSubContact = Meta_GetMostOnline(cc);
	return ProtoCallService(GetContactProto(hSubContact), PS_SEND_NUDGE, hSubContact, lParam);
}

/////////////////////////////////////////////////////////////////

/** Send a message to the protocol specific network.
*
* Call the function specific to the protocol that belongs
* to the contact chosen to send the message.
*
* @param wParam : index of the protocol in the protocol chain.
* @param lParam : CCSDATA structure holding all the information abour rhe message.
*
* @return 0 on success, 1 otherwise.
*/

INT_PTR Meta_SendMessage(WPARAM wParam,LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;

	DBCachedContact *cc = CheckMeta(ccs->hContact);
	if (cc == NULL || cc->nDefault == -1) {
		// This is a simple contact, let through the stack of protocols
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	}

	MCONTACT hMostOnline = db_mc_getSrmmSub(cc->contactID);
	if (!hMostOnline) {
		// send failure to notify user of reason
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		TFakeAckParams *tfap = (TFakeAckParams *)mir_alloc(sizeof(TFakeAckParams));
		tfap->hContact = ccs->hContact;
		tfap->hEvent = hEvent;
		tfap->id = 10;
		strcpy(tfap->msg, Translate("No online contacts found."));

		DWORD dwThreadId;
		CloseHandle(CreateThread(NULL, 0, sttFakeAckFail, tfap, 0, &dwThreadId));
		SetEvent(hEvent);
		return 10;
	}

	Meta_CopyContactNick(cc, hMostOnline);

	ccs->hContact = hMostOnline;
	char *proto = GetContactProto(hMostOnline);
	Meta_SetNick(proto);	// (no matter what was there before)

	return CallContactService(ccs->hContact, PSS_MESSAGE, ccs->wParam, ccs->lParam);
}

/** Called when an ACK is received.
*
* Retransmit the ACK sent by a simple contact so that it
* looks like it was the MetaContact that sends the ACK.
*
* @param wParam : Allways set to 0.
* @param lParam : Reference to a ACKDATA that contains information about the ACK.
* @returns 0 on success, 1 otherwise.
*/

int Meta_HandleACK(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	DBCachedContact *cc = CheckMeta(ack->hContact);
	if (cc == NULL)
		return 0;

	if (!strcmp(ack->szModule, META_PROTO))
		return 0; // don't rebroadcast our own acks

	// if it's for something we don't support, ignore
	if (ack->type != ACKTYPE_MESSAGE && ack->type != ACKTYPE_CHAT && ack->type != ACKTYPE_FILE && ack->type != ACKTYPE_AWAYMSG && ack->type != ACKTYPE_AVATAR && ack->type != ACKTYPE_GETINFO)
		return 0;

	// change the hContact in the avatar info struct, if it's the avatar we're using - else drop it
	if (ack->type == ACKTYPE_AVATAR) {
		if (ack->result == ACKRESULT_SUCCESS || ack->result == ACKRESULT_FAILED || ack->result == ACKRESULT_STATUS) {
			DBVARIANT dbv;

			// change avatar if the most online supporting avatars changes, or if we don't have one
			MCONTACT hMostOnline = Meta_GetMostOnlineSupporting(cc, PFLAGNUM_4, PF4_AVATARS);
			//if (AI.hContact == 0 || AI.hContact != hMostOnline) {
			if (ack->hContact == 0 || ack->hContact != hMostOnline) {
				return 0;
			}

			if (!db_get(ack->hContact, "ContactPhoto", "File", &dbv)) {
				db_set_ts(cc->contactID, "ContactPhoto", "File", dbv.ptszVal);
				db_free(&dbv);
			}

			if (ack->hProcess) {
				PROTO_AVATAR_INFORMATIONT AI;
				memcpy(&AI, (PROTO_AVATAR_INFORMATIONT *)ack->hProcess, sizeof(PROTO_AVATAR_INFORMATIONT));
				if (AI.hContact)
					AI.hContact = cc->contactID;

				return ProtoBroadcastAck(META_PROTO, cc->contactID, ack->type, ack->result, (HANDLE)&AI, ack->lParam);
			}

			return ProtoBroadcastAck(META_PROTO, cc->contactID, ack->type, ack->result, 0, ack->lParam);
		}
	}

	return ProtoBroadcastAck(META_PROTO, cc->contactID, ack->type, ack->result, ack->hProcess, ack->lParam);
}

/** Call whenever a contact changes one of its settings (for example, the status)
**
* @param wParam HANDLE to the contact that has change of its setting.
* @param lParam Reference to a structure that contains the setting that has changed (not used)
*/

int Meta_SettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dcws = (DBCONTACTWRITESETTING *)lParam;
	char buffer[512];

	// the only global options we're interested in
	if (hContact == 0)
		return 0;

	DBCachedContact *cc = currDb->m_cache->GetCachedContact(hContact);
	if (cc == NULL || !cc->IsSub())
		return 0;

	DBCachedContact *ccMeta = currDb->m_cache->GetCachedContact(cc->parentID);
	if (ccMeta == NULL || !ccMeta->IsMeta())
		return 0;

	// This contact is attached to a MetaContact.
	int contact_number = Meta_GetContactNumber(ccMeta, hContact);
	if (contact_number == -1)
		return 0; // exit - db corruption

	if (!strcmp(dcws->szSetting, "IP")) {
		if (dcws->value.type == DBVT_DWORD)
			db_set_dw(ccMeta->contactID, META_PROTO, "IP", dcws->value.dVal);
		else
			db_unset(ccMeta->contactID, META_PROTO, "IP");
	}
	else if (!strcmp(dcws->szSetting, "RealIP")) {
		if (dcws->value.type == DBVT_DWORD)
			db_set_dw(ccMeta->contactID, META_PROTO, "RealIP", dcws->value.dVal);
		else
			db_unset(ccMeta->contactID, META_PROTO, "RealIP");
	}
	else if (!strcmp(dcws->szSetting, "ListeningTo")) {
		switch (dcws->value.type) {
		case DBVT_ASCIIZ:
			db_set_s(ccMeta->contactID, META_PROTO, "ListeningTo", dcws->value.pszVal);
			break;
		case DBVT_UTF8:
			db_set_utf(ccMeta->contactID, META_PROTO, "ListeningTo", dcws->value.pszVal);
			break;
		case DBVT_WCHAR:
			db_set_ws(ccMeta->contactID, META_PROTO, "ListeningTo", dcws->value.pwszVal);
			break;
		case DBVT_DELETED:
			db_unset(ccMeta->contactID, META_PROTO, "ListeningTo");
			break;
		}
	}
	else if (!strcmp(dcws->szSetting, "Nick") && dcws->value.type != DBVT_DELETED) {
		// subcontact nick has changed - update metacontact
		mir_snprintf(buffer, SIZEOF(buffer), "Nick%d", contact_number);
		db_set(ccMeta->contactID, META_PROTO, buffer, &dcws->value);

		DBVARIANT dbv;
		if (db_get_s(hContact, "CList", "MyHandle", &dbv, 0)) {
			mir_snprintf(buffer, SIZEOF(buffer), "CListName%d", contact_number);
			db_set(ccMeta->contactID, META_PROTO, buffer, &dcws->value);
		}
		else db_free(&dbv);

		// copy nick to metacontact, if it's the most online
		MCONTACT hMostOnline = Meta_GetMostOnline(ccMeta);
		Meta_CopyContactNick(ccMeta, hMostOnline);
	}
	else if (!strcmp(dcws->szSetting, "IdleTS")) {
		if (dcws->value.type == DBVT_DWORD)
			db_set_dw(ccMeta->contactID, META_PROTO, "IdleTS", dcws->value.dVal);
		else if (dcws->value.type == DBVT_DELETED)
			db_set_dw(ccMeta->contactID, META_PROTO, "IdleTS", 0);
	}
	else if (!strcmp(dcws->szSetting, "LogonTS")) {
		if (dcws->value.type == DBVT_DWORD)
			db_set_dw(ccMeta->contactID, META_PROTO, "LogonTS", dcws->value.dVal);
		else if (dcws->value.type == DBVT_DELETED)
			db_set_dw(ccMeta->contactID, META_PROTO, "LogonTS", 0);
	}
	else if (!strcmp(dcws->szModule, "CList") && !strcmp(dcws->szSetting, "MyHandle")) {
		if (dcws->value.type == DBVT_DELETED) {
			char *proto = GetContactProto(hContact);
			mir_snprintf(buffer, SIZEOF(buffer), "CListName%d", contact_number);

			DBVARIANT dbv;
			if (proto && !db_get_ts(hContact, proto, "Nick", &dbv)) {
				db_set_ts(ccMeta->contactID, META_PROTO, buffer, dbv.ptszVal);
				db_free(&dbv);
			}
			else db_unset(ccMeta->contactID, META_PROTO, buffer);
		}
		else {
			// subcontact clist displayname has changed - update metacontact
			mir_snprintf(buffer, SIZEOF(buffer), "CListName%d", contact_number);
			db_set(ccMeta->contactID, META_PROTO, buffer, &dcws->value);
		}

		// copy nick to metacontact, if it's the most online
		Meta_CopyContactNick(ccMeta, Meta_GetMostOnline(ccMeta));
	}
	// subcontact changing status
	else if (!strcmp(dcws->szSetting, "Status") && dcws->value.type != DBVT_DELETED) {
		// update subcontact status setting
		mir_snprintf(buffer, SIZEOF(buffer), "Status%d", contact_number);
		db_set_w(ccMeta->contactID, META_PROTO, buffer, dcws->value.wVal);
		
		mir_snprintf(buffer, SIZEOF(buffer), "StatusString%d", contact_number);
		db_set_ts(ccMeta->contactID, META_PROTO, buffer, cli.pfnGetStatusModeDescription(dcws->value.wVal, 0));

		// set status to that of most online contact
		MCONTACT hMostOnline = Meta_GetMostOnline(ccMeta);
		if (hMostOnline != db_mc_getDefault(ccMeta->contactID))
			db_mc_notifyDefChange(ccMeta->contactID, hMostOnline);

		Meta_CopyContactNick(ccMeta, hMostOnline);
		Meta_FixStatus(ccMeta);

		// most online contact with avatar support might have changed - update avatar
		hMostOnline = Meta_GetMostOnlineSupporting(ccMeta, PFLAGNUM_4, PF4_AVATARS);
		if (hMostOnline) {
			PROTO_AVATAR_INFORMATIONT AI = { sizeof(AI) };
			AI.hContact = ccMeta->contactID;
			AI.format = PA_FORMAT_UNKNOWN;
			_tcscpy(AI.filename, _T("X"));
			if (CallProtoService(META_PROTO, PS_GETAVATARINFOT, 0, (LPARAM)&AI) == GAIR_SUCCESS)
				db_set_ts(ccMeta->contactID, "ContactPhoto", "File", AI.filename);
		}
	}

	return 0;
}

int Meta_ContactDeleted(WPARAM hContact, LPARAM lParam)
{
	DBCachedContact *cc = currDb->m_cache->GetCachedContact(hContact);
	if (cc == NULL)
		return 0;

	// is a subcontact - update meta contact
	if (cc->IsSub()) {
		DBCachedContact *ccMeta = CheckMeta(cc->parentID);
		if (ccMeta) {
			Meta_RemoveContactNumber(ccMeta, Meta_GetContactNumber(ccMeta, hContact), true);
			NotifyEventHooks(hSubcontactsChanged, ccMeta->contactID, 0);
		}
		return 0;
	}

	// not a subcontact - is it a metacontact?
	if (!cc->IsMeta())
		return 0;

	if (cc->nSubs > 0)
		NotifyEventHooks(hSubcontactsChanged, hContact, 0);

	// remove & restore all subcontacts
	for (int i = 0; i < cc->nSubs; i++) {
		currDb->MetaDetouchSub(cc, i);

		// stop ignoring, if we were
		if (options.bSuppressStatus)
			CallService(MS_IGNORE_UNIGNORE, cc->pSubs[i], IGNOREEVENT_USERONLINE);
	}
	return 0;
}

/** Call when we want to send a user is typing message
*
* @param wParam HANDLE to the contact that we are typing to
* @param lParam either PROTOTYPE_SELFTYPING_ON or PROTOTYPE_SELFTYPING_OFF
*/
static INT_PTR Meta_UserIsTyping(WPARAM hMeta, LPARAM lParam)
{
	DBCachedContact *cc = CheckMeta(hMeta);
	if (cc == NULL)
		return 0;

	// forward to sending protocol, if supported
	MCONTACT hMostOnline = Meta_GetMostOnline(cc);
	Meta_CopyContactNick(cc, hMostOnline);
	if (!hMostOnline)
		return 0;

	char *proto = GetContactProto(hMostOnline);
	if (proto)
		if (ProtoServiceExists(proto, PSS_USERISTYPING))
			ProtoCallService(proto, PSS_USERISTYPING, hMostOnline, lParam);

	return 0;
}

/** Call when we want to receive a user is typing message
*
* @param wParam HANDLE to the contact that is typing or not
* @param lParam either PROTOTYPE_SELFTYPING_ON or PROTOTYPE_SELFTYPING_OFF
*/

static int Meta_ContactIsTyping(WPARAM hContact, LPARAM lParam)
{
	if (!db_mc_isEnabled())
		return 0;

	DBCachedContact *cc = currDb->m_cache->GetCachedContact(hContact);
	if (cc != NULL && cc->IsSub()) {
		CallService(MS_PROTO_CONTACTISTYPING, cc->parentID, lParam);
		return 1;
	}

	return 0;
}

/** Called when user info is about to be shown
*
* Returns 1 to stop event processing and opens page for metacontact default contact (returning 1 to stop it doesn't work!)
*
*/

static int Meta_UserInfo(WPARAM wParam, LPARAM hMeta)
{
	DBCachedContact *cc = CheckMeta(hMeta);
	if (cc == NULL || cc->nDefault == -1)
		return 0;

	CallService(MS_USERINFO_SHOWDIALOG, Meta_GetContactHandle(cc, cc->nDefault), 0);
	return 1;
}

// record window open/close status for subs & metas
static int Meta_MessageWindowEvent(WPARAM wParam, LPARAM lParam)
{
	MessageWindowEventData *mwed = (MessageWindowEventData*)lParam;
	if (mwed->uType == MSG_WINDOW_EVT_OPEN) {
		DBCachedContact *cc = currDb->m_cache->GetCachedContact(mwed->hContact);
		if (cc != NULL) {
			Meta_UpdateSrmmIcon(cc, db_get_w(cc->contactID, META_PROTO, "Status", ID_STATUS_OFFLINE));
			if (cc->IsMeta()) {
				MetaSrmmData *p = new MetaSrmmData;
				p->m_hMeta = cc->contactID;
				p->m_hSub = db_mc_getMostOnline(cc->contactID);
				p->m_hWnd = mwed->hwndWindow;
				arMetaWindows.insert(p);
			}
		}
	}
	else if (mwed->uType == MSG_WINDOW_EVT_CLOSING) {
		for (int i = 0; i < arMetaWindows.getCount(); i++)
			if (arMetaWindows[i].m_hWnd == mwed->hwndWindow)
				arMetaWindows.remove(i);
	}
	return 0;
}

// returns manually chosen sub in the meta window
static INT_PTR Meta_SrmmCurrentSub(WPARAM hMeta, LPARAM lParam)
{
	MetaSrmmData tmp = { hMeta };
	if (MetaSrmmData *p = arMetaWindows.find(&tmp))
		return p->m_hSub;

	return db_mc_getMostOnline(hMeta);
}

// we assume that it could be called only for the metacontacts
static int Meta_SrmmIconClicked(WPARAM hMeta, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData*)lParam;
	if (lstrcmpA(sicd->szModule, META_PROTO))
		return 0;

	DBCachedContact *cc = CheckMeta(hMeta);
	if (cc == NULL)
		return 0;

	HMENU hMenu = CreatePopupMenu();
	int iDefault = Meta_GetContactNumber(cc, db_mc_getSrmmSub(cc->contactID));
	TCHAR tszItemName[200];

	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_ID | MIIM_STATE | MIIM_STRING;
	mii.dwTypeData = tszItemName;
	mii.cch = SIZEOF(tszItemName);
	for (int i = 0; i < cc->nSubs; i++)	{
		char *szProto = GetContactProto(cc->pSubs[i]);
		if (szProto == NULL) continue;

		PROTOACCOUNT *pa = ProtoGetAccount(szProto);
		if (pa == NULL)
			continue;

		mir_sntprintf(tszItemName, SIZEOF(tszItemName), _T("%s [%s]"),
			cli.pfnGetContactDisplayName(cc->pSubs[i], 0), pa->tszAccountName);

		mii.wID = i + 1;
		mii.fState = (i == iDefault) ? MFS_CHECKED : MFS_ENABLED;
		InsertMenuItem(hMenu, i, TRUE, &mii);
	}

	UINT res = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_BOTTOMALIGN | TPM_LEFTALIGN, sicd->clickLocation.x, sicd->clickLocation.y, 0, cli.hwndContactTree, NULL);
	if (res > 0) {
		MCONTACT hChosen = Meta_GetContactHandle(cc, res - 1);

		MetaSrmmData tmp = { cc->contactID };
		if (MetaSrmmData *p = arMetaWindows.find(&tmp))
			p->m_hSub = hChosen;

		db_mc_setDefault(cc->contactID, hChosen, true);
	}
	return 0;
}

int NudgeRecieved(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

/** Called when all the plugin are loaded into Miranda.
*
* Initializes the 4 menus present in the context-menu
*/

int Meta_ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, Meta_ModifyMenu);

	// hook srmm window close/open events
	HookEvent(ME_MSG_WINDOWEVENT, Meta_MessageWindowEvent);
	HookEvent(ME_MSG_ICONPRESSED, Meta_SrmmIconClicked);

	////////////////////////////////////////////////////////////////////////////
	InitMenus();

	// create srmm icon
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = META_PROTO;
	sid.hIcon = LoadSkinnedProtoIcon(META_PROTO, ID_STATUS_ONLINE);
	Srmm_AddIcon(&sid);

	// hook protocol nudge events to forward to subcontacts
	int numberOfProtocols;
	PROTOACCOUNT ** ppProtocolDescriptors;
	ProtoEnumAccounts(&numberOfProtocols, &ppProtocolDescriptors);

	for (int i = 0; i < numberOfProtocols; i++)
		if (strcmp(ppProtocolDescriptors[i]->szModuleName, META_PROTO)) {
			char str[MAXMODULELABELLENGTH + 10];
			mir_snprintf(str, SIZEOF(str), "%s/Nudge", ppProtocolDescriptors[i]->szModuleName);
			HookEvent(str, NudgeRecieved);
		}

	return 0;
}

static VOID CALLBACK sttMenuThread(PVOID param)
{
	HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)param, 0);

	TPMPARAMS tpmp = { 0 };
	tpmp.cbSize = sizeof(tpmp);
	BOOL menuRet = TrackPopupMenuEx(hMenu, TPM_RETURNCMD, menuMousePoint.x, menuMousePoint.y, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), &tpmp);

	CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(menuRet), MPCF_CONTACTMENU), (LPARAM)param);

	DestroyMenu(hMenu);
}

INT_PTR Meta_ContactMenuFunc(WPARAM hMeta, LPARAM lParam)
{
	DBCachedContact *cc = CheckMeta(hMeta);
	if (cc == NULL)
		return 0;

	MCONTACT hContact = Meta_GetContactHandle(cc, (int)lParam);

	if (options.menu_function == FT_MSG) {
		// open message window if protocol supports message sending or chat, else simulate double click
		char *proto = GetContactProto(hContact);
		if (proto) {
			INT_PTR caps = CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0);
			if ((caps & PF1_IMSEND) || (caps & PF1_CHAT)) {
				// set default contact for sending/status and open message window
				db_mc_setDefaultNum(hMeta, lParam, false);
				CallService(MS_MSG_SENDMESSAGET, hMeta, 0);
			}
			else // protocol does not support messaging - simulate double click
				CallService(MS_CLIST_CONTACTDOUBLECLICKED, hContact, 0);
		}
		else // protocol does not support messaging - simulate double click
			CallService(MS_CLIST_CONTACTDOUBLECLICKED, hContact, 0);
	}
	else if (options.menu_function == FT_MENU) // show contact's context menu
		CallFunctionAsync(sttMenuThread, (void*)hContact);
	else if (options.menu_function == FT_INFO) // show user info for subcontact
		CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);

	return 0;
}

////////////////////
// file transfer support - mostly not required, since subcontacts do the receiving
////////////////////

INT_PTR Meta_FileSend(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	DBCachedContact *cc = CheckMeta(ccs->hContact);
	if (cc == NULL || cc->nDefault == -1)
		return 0;

	MCONTACT hMostOnline = Meta_GetMostOnlineSupporting(cc, PFLAGNUM_1, PF1_FILESEND);
	if (!hMostOnline)
		return 0;

	char *proto = GetContactProto(hMostOnline);
	if (proto)
		return CallContactService(hMostOnline, PSS_FILE, ccs->wParam, ccs->lParam);

	return 0; // fail
}

INT_PTR Meta_GetAwayMsg(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	DBCachedContact *cc = CheckMeta(ccs->hContact);
	if (cc == NULL || cc->nDefault == -1)
		return 0;

	MCONTACT hMostOnline = Meta_GetMostOnlineSupporting(cc, PFLAGNUM_1, PF1_MODEMSGRECV);
	if (!hMostOnline)
		return 0;

	char *proto = GetContactProto(hMostOnline);
	if (!proto)
		return 0;

	ccs->hContact = hMostOnline;
	return CallContactService(ccs->hContact, PSS_GETAWAYMSG, ccs->wParam, ccs->lParam);
}

INT_PTR Meta_GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONT *AI = (PROTO_AVATAR_INFORMATIONT*)lParam;
	DBCachedContact *cc = CheckMeta(AI->hContact);
	if (cc == NULL)
		return GAIR_NOAVATAR;

	if (cc->nDefault == -1)
		return 0;

	MCONTACT hSub = Meta_GetMostOnlineSupporting(cc, PFLAGNUM_4, PF4_AVATARS);
	if (!hSub)
		return GAIR_NOAVATAR;

	char *proto = GetContactProto(hSub);
	if (!proto)
		return GAIR_NOAVATAR;

	AI->hContact = hSub;
	INT_PTR result = ProtoCallService(proto, PS_GETAVATARINFOT, wParam, lParam);
	AI->hContact = cc->contactID;
	if (result != CALLSERVICE_NOTFOUND)
		return result;

	return GAIR_NOAVATAR; // fail
}

INT_PTR Meta_GetInfo(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;

	// This is a simple contact
	// (this should normally not happen, since linked contacts do not appear on the list.)
	DBCachedContact *cc = CheckMeta(ccs->hContact);
	if (cc == NULL || cc->nDefault == -1)
		return 0;

	MCONTACT hMostOnline = Meta_GetMostOnlineSupporting(cc, PFLAGNUM_4, PF4_AVATARS);
	if (!hMostOnline)
		return 0;

	char *proto = GetContactProto(hMostOnline);
	if (!proto)
		return 0;

	PROTO_AVATAR_INFORMATIONT AI;
	AI.cbSize = sizeof(AI);
	AI.hContact = ccs->hContact;
	AI.format = PA_FORMAT_UNKNOWN;
	_tcscpy(AI.filename, _T("X"));
	if (CallProtoService(META_PROTO, PS_GETAVATARINFOT, 0, (LPARAM)&AI) == GAIR_SUCCESS)
		db_set_ts(ccs->hContact, "ContactPhoto", "File", AI.filename);

	hMostOnline = Meta_GetMostOnline(cc);
	Meta_CopyContactNick(cc, hMostOnline);

	if (!hMostOnline)
		return 0;

	ccs->hContact = hMostOnline;
	if (!ProtoServiceExists(proto, PSS_GETINFO))
		return 0; // fail

	return CallContactService(ccs->hContact, PSS_GETINFO, ccs->wParam, ccs->lParam);
}

int Meta_CallMostOnline(WPARAM hContact, LPARAM lParam)
{
	DBCachedContact *cc = CheckMeta(hContact);
	if (cc == NULL)
		return 0;

	Meta_CopyContactNick(cc, db_mc_getSrmmSub(cc->contactID));
	Meta_FixStatus(cc);
	return 0;
}

int Meta_PreShutdown(WPARAM wParam, LPARAM lParam)
{
	Meta_SetStatus(ID_STATUS_OFFLINE, 0);
	Meta_SuppressStatus(FALSE);
	if (setStatusTimerId)
		KillTimer(0, setStatusTimerId);
	return 0;
}

INT_PTR MenuFunc(WPARAM wParam, LPARAM lParam, LPARAM param)
{
	return Meta_ContactMenuFunc(wParam, param);
}

/** Initializes all services provided by the plugin
*
* Creates every function and hooks the event desired.
*/

void Meta_InitServices()
{
	previousMode = mcStatus = ID_STATUS_OFFLINE;

	CreateServiceFunction("MetaContacts/Convert", Meta_Convert);
	CreateServiceFunction("MetaContacts/AddTo", Meta_AddTo);
	CreateServiceFunction("MetaContacts/Edit", Meta_Edit);
	CreateServiceFunction("MetaContacts/Delete", Meta_Delete);
	CreateServiceFunction("MetaContacts/Default", Meta_Default);

	// hidden contact menu items...ho hum
	for (int i = 0; i < MAX_CONTACTS; i++) {
		char szServiceName[100];
		mir_snprintf(szServiceName, SIZEOF(szServiceName), "MetaContacts/MenuFunc%d", i);
		CreateServiceFunctionParam(szServiceName, MenuFunc, i);
	}

	CreateProtoServiceFunction(META_PROTO, PS_GETCAPS, Meta_GetCaps);
	CreateProtoServiceFunction(META_PROTO, PS_GETNAME, Meta_GetName);
	CreateProtoServiceFunction(META_PROTO, PS_LOADICON, Meta_LoadIcon);

	CreateProtoServiceFunction(META_PROTO, PS_SETSTATUS, Meta_SetStatus);

	CreateProtoServiceFunction(META_PROTO, PS_GETSTATUS, Meta_GetStatus);
	CreateProtoServiceFunction(META_PROTO, PSS_MESSAGE, Meta_SendMessage);

	CreateProtoServiceFunction(META_PROTO, PSS_USERISTYPING, Meta_UserIsTyping);

	// file recv is done by subcontacts
	CreateProtoServiceFunction(META_PROTO, PSS_FILE, Meta_FileSend);
	CreateProtoServiceFunction(META_PROTO, PSS_GETAWAYMSG, Meta_GetAwayMsg);
	CreateProtoServiceFunction(META_PROTO, PS_GETAVATARINFOT, Meta_GetAvatarInfo);
	CreateProtoServiceFunction(META_PROTO, PSS_GETINFO, Meta_GetInfo);

	// receive filter
	CreateProtoServiceFunction(META_FILTER, PSR_MESSAGE, MetaFilter_RecvMessage);

	// API services and events
	CreateApiServices();

	CreateServiceFunction("MetaContacts/OnOff", Meta_OnOff);
	CreateServiceFunction(MS_MC_GETSRMMSUB, Meta_SrmmCurrentSub);

	CreateProtoServiceFunction(META_PROTO, PS_SEND_NUDGE, Meta_SendNudge);

	// create our hookable events
	hSubcontactsChanged = CreateHookableEvent(ME_MC_SUBCONTACTSCHANGED);

	// hook other module events we need
	HookEvent(ME_PROTO_ACK, Meta_HandleACK);
	HookEvent(ME_PROTO_CONTACTISTYPING, Meta_ContactIsTyping);
	HookEvent(ME_DB_CONTACT_DELETED, Meta_ContactDeleted);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, Meta_SettingChanged);
	HookEvent(ME_OPT_INITIALISE, Meta_OptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, Meta_ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, Meta_PreShutdown);

	// hook our own events, used to call Meta_GetMostOnline which sets nick for metacontact
	HookEvent(ME_MC_DEFAULTTCHANGED, Meta_CallMostOnline);

	// redirect nudge events
	hEventNudge = CreateHookableEvent(META_PROTO "/Nudge");
}

// Unregister all hooks and services from Miranda
void Meta_CloseHandles()
{
	DestroyHookableEvent(hSubcontactsChanged);
	DestroyHookableEvent(hEventNudge);
}
