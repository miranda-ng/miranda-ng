/*
MetaContacts Plugin for Miranda IM.

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

/** @file meta_services.c
*
* Functions specific to the protocol part of the plugin.
* Centralizes all the functions called by Miranda to make
* the plugin work as a protocol.
*/

#include "metacontacts.h"

#define PREF_METANODB	0x2000	//!< Flag to indicate message should not be added to db by filter when sending

char *pendingACK = 0;		//!< Name of the protocol in which an ACK is about to come.

int previousMode,			//!< Previous status of the MetaContacts Protocol
	mcStatus;				//!< Current status of the MetaContacts Protocol

HGENMENU
	hMenuConvert,      //!< \c HANDLE to the convert menu item.
	hMenuAdd,          //!< \c HANDLE to the add to menu item.
	hMenuEdit,         //!< \c HANDLE to the edit menu item.
	hMenuDelete,       //!< \c HANDLE to the delete menu item.
	hMenuDefault,      //!< \c HANDLE to the delete menu item.
	hMenuForceDefault, //!< \c HANDLE to the delete menu item.
	hMenuOnOff;	       //!< \c HANDLE to the enable/disable menu item.

HANDLE
	hEventDefaultChanged, //!< \c HANDLE to the 'default changed' event
	hEventForceSend,		 //!< \c HANDLE to the 'force send' event
	hEventUnforceSend,    //!< \c HANDLE to the 'unforce send' event
	hSubcontactsChanged,  //!< \c HANDLE to the 'contacts changed' event
	hEventNudge;


DWORD nextMetaID;	//!< Global variable specifying the ID value the next MetaContact will have.

BOOL message_window_api_enabled = FALSE; //!< Global variable specifying whether the message window api ver 0.0.0.1+ is available

// stuff for mw_clist extra icon
HANDLE hExtraImage[MAX_PROTOCOLS * 2]; // online and offline icons
char proto_names[MAX_PROTOCOLS * 128];
HANDLE hProtoIcons[MAX_PROTOCOLS * 2]; // online and offline icons

UINT_PTR setStatusTimerId = 0;
BOOL firstSetOnline = TRUE; // see Meta_SetStatus function

/** Get the capabilities of the "MetaContacts" protocol.
*
* @param wParam :	equals to one of the following values :\n
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

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR) Translate("Meta ID");

	case PFLAG_MAXLENOFMESSAGE:
		return 2000;

	case PFLAG_UNIQUEIDSETTING:
		return (INT_PTR)META_ID;
	}
	return 0;
}

/** Copy the name of the protocole into lParam
* @param wParam :	max size of the name
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
* @param wParam :	one of the following values : \n
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

	return (INT_PTR) LoadImage(hInstance, MAKEINTRESOURCE(id), IMAGE_ICON,
		GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON),
		GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
}

void CALLBACK SetStatusThread(HWND hWnd, UINT msg, UINT_PTR id, DWORD dw)
{
	previousMode = mcStatus;

	mcStatus = (int)ID_STATUS_ONLINE;
	ProtoBroadcastAck(META_PROTO, NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS, (HANDLE)previousMode, mcStatus);

	KillTimer(0, setStatusTimerId);
}

/** Changes the status and notifies everybody
* @param wParam :	The new mode
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
	ProtoBroadcastAck(META_PROTO, tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)tParam->id, (WPARAM)tParam->msg );

	CloseHandle( tParam->hEvent );
	mir_free(tParam);
	return 0;
}

/** Filter messages sent by subcontacts
*
* When groups are disabled, add an event to the DB for the metacontact to maintain history
*
* @param wParam :	index of the protocol in the protocol chain.
* @param lParam :	\c CCSDATA structure holding all the information about the message.
*
* @return			0 on success, 1 otherwise.
*/

INT_PTR MetaFilter_SendMessage(WPARAM wParam,LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	MCONTACT hMeta;

	if ((hMeta = (MCONTACT)db_get_dw(ccs->hContact, META_PROTO, "Handle", 0)) == 0)
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam); // Can't find the MetaID of the metacontact linked to

	// if subcontact sending, add db event to keep metacontact history correct
	if (options.metahistory && !(ccs->wParam & PREF_METANODB)) {

		// reject "file As Message" messages
		if (strlen((char *)ccs->lParam) > 5 && strncmp((char *)ccs->lParam, "<%fAM", 5) == 0)
			return CallService(MS_PROTO_CHAINSEND, wParam, lParam);	// continue processing

		// reject "data As Message" messages
		if (strlen((char *)ccs->lParam) > 5 && strncmp((char *)ccs->lParam, "<%dAM", 5) == 0)
			return CallService(MS_PROTO_CHAINSEND, wParam, lParam);	// continue processing

		// reject "OTR" messages
		if (strlen((char *)ccs->lParam) > 5 && strncmp((char *)ccs->lParam, "?OTR", 4) == 0)
			return CallService(MS_PROTO_CHAINSEND, wParam, lParam);	// continue processing

		DBEVENTINFO dbei = { sizeof(dbei) };
		dbei.szModule = META_PROTO;
		dbei.flags = DBEF_SENT;
		dbei.timestamp = time(NULL);
		dbei.eventType = EVENTTYPE_MESSAGE;
		if (ccs->wParam & PREF_RTL) dbei.flags |= DBEF_RTL;
		if (ccs->wParam & PREF_UTF) dbei.flags |= DBEF_UTF;
		dbei.cbBlob = (DWORD)strlen((char *)ccs->lParam) + 1;
		if ( ccs->wParam & PREF_UNICODE )
			dbei.cbBlob *= ( sizeof( wchar_t )+1 );
		dbei.pBlob = (PBYTE)ccs->lParam;
		db_event_add(hMeta, &dbei);
	}

	return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
}

INT_PTR Meta_SendNudge(WPARAM wParam,LPARAM lParam)
{
	MCONTACT hSubContact = Meta_GetMostOnline(wParam);
	return ProtoCallService(GetContactProto(hSubContact), PS_SEND_NUDGE, (WPARAM)hSubContact, lParam);
}

/////////////////////////////////////////////////////////////////

/** Send a message to the protocol specific network.
*
* Call the function specific to the protocol that belongs
* to the contact chosen to send the message.
*
* @param wParam :	index of the protocol in the protocol chain.
* @param lParam :	\c CCSDATA structure holding all the information abour rhe message.
*
* @return			0 on success, 1 otherwise.
*/
INT_PTR Meta_SendMessage(WPARAM wParam,LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	char *proto = 0;
	DWORD default_contact_number;

	if ((default_contact_number = db_get_dw(ccs->hContact, META_PROTO, "Default",(DWORD)-1)) == (DWORD)-1) {
		// This is a simple contact, let through the stack of protocols
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	}

	MCONTACT most_online = Meta_GetMostOnline(ccs->hContact);

	if ( !most_online) {
		DWORD dwThreadId;
		HANDLE hEvent;
		TFakeAckParams *tfap;

		// send failure to notify user of reason
		hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		tfap = (TFakeAckParams *)mir_alloc(sizeof(TFakeAckParams));
		tfap->hContact = ccs->hContact;
		tfap->hEvent = hEvent;
		tfap->id = 10;
		strcpy(tfap->msg, Translate("No online contacts found."));

		CloseHandle( CreateThread( NULL, 0, sttFakeAckFail, tfap, 0, &dwThreadId ));
		SetEvent( hEvent );

		return 10;
	}

	Meta_CopyContactNick(ccs->hContact, most_online);

	ccs->hContact = most_online;
	proto = GetContactProto(most_online);
	Meta_SetNick(proto);	// (no matter what was there before)

	// don't bypass filters etc
	if (options.subhistory && !(ccs->wParam & PREF_METANODB)) {
		// add sent event to subcontact
		DBEVENTINFO dbei = { sizeof(dbei) };
		dbei.szModule = GetContactProto(ccs->hContact);
		if (dbei.szModule) {
			dbei.flags = DBEF_SENT;
			dbei.timestamp = time(NULL);
			dbei.eventType = EVENTTYPE_MESSAGE;
			if (ccs->wParam & PREF_RTL) dbei.flags |= DBEF_RTL;
			if (ccs->wParam & PREF_UTF) dbei.flags |= DBEF_UTF;
			dbei.cbBlob = (DWORD)strlen((char *)ccs->lParam) + 1;
			if ( ccs->wParam & PREF_UNICODE )
				dbei.cbBlob *= ( sizeof( wchar_t )+1 );
			dbei.pBlob = (PBYTE)ccs->lParam;
			db_event_add(ccs->hContact, &dbei);
		}
	}

	// prevent send filter from adding another copy of this send event to the db
	ccs->wParam |= PREF_METANODB;

	return CallContactService(ccs->hContact, PSS_MESSAGE, ccs->wParam, ccs->lParam);
}

/** Transmit a message received by a contact.
*
* Forward the message received by a contact linked to a MetaContact
* to that MetaContact and inhibit the further reception of this message
* by the standard protocol of the contact.
*
* @param wParam :	index of the protocol in the protocol chain.
* @param lParam :	\c CCSDATA structure holding all the information about the message.
*
* @return			0 on success, 1 otherwise.
*/

INT_PTR MetaFilter_RecvMessage(WPARAM wParam,LPARAM lParam)
{
	DBEVENTINFO dbei;
	CCSDATA *ccs = (CCSDATA*)lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT *) ccs->lParam;
	MCONTACT hMeta;

	// Can't find the MetaID of the metacontact linked to this contact, let through the protocol chain
	if ((hMeta = (MCONTACT)db_get_dw(ccs->hContact, META_PROTO, "Handle", 0)) == 0)
		return CallService(MS_PROTO_CHAINRECV, wParam, (LPARAM)ccs);

	if (options.set_default_on_recv) {
		if (options.temp_default && db_get_dw(hMeta, META_PROTO, "SavedDefault", (DWORD)-1) == (DWORD)-1)
			db_set_dw(hMeta, META_PROTO, "SavedDefault", db_get_dw(hMeta, META_PROTO, "Default", 0));
		db_set_dw(hMeta, META_PROTO, "Default", db_get_dw(ccs->hContact, META_PROTO, "ContactNumber", 0));
		NotifyEventHooks(hEventDefaultChanged, (WPARAM)hMeta, (LPARAM)ccs->hContact); // nick set in event handler
	}

	// if meta disabled (now message api) or window open (message api), or using subcontact windows,
	// let through but add db event for metacontact history
	if ( !Meta_IsEnabled() || db_get_b(ccs->hContact, META_PROTO, "WindowOpen", 0) == 1 || options.subcontact_windows) {

		// add a clist event, so that e.g. there is an icon flashing
		// (only add it when message api available, 'cause then we can remove the event when the message window is opened)
		if (message_window_api_enabled
			&& db_get_b(ccs->hContact, META_PROTO, "WindowOpen", 0) == 0
			&& db_get_b(hMeta, META_PROTO, "WindowOpen", 0) == 0
			&& options.flash_meta_message_icon)
		{
			TCHAR toolTip[256];

			CLISTEVENT cle = { sizeof(cle) };
			cle.hContact = hMeta;
			cle.flags = CLEF_TCHAR;
			cle.hDbEvent = (HANDLE)ccs->hContact;	// use subcontact handle as key - then we can remove all events if the subcontact window is opened
			cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
			cle.pszService = "MetaContacts/CListMessageEvent";
			mir_sntprintf(toolTip, SIZEOF(toolTip), TranslateT("Message from %s"), pcli->pfnGetContactDisplayName(hMeta, GCDNF_TCHAR));
			cle.ptszTooltip = toolTip;
			CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
		}

		if (options.metahistory) {
			BOOL added = FALSE;

			// should be able to do this, but some protos mess with the memory
			if (options.use_proto_recv) {
				// use the subcontact's protocol 'recv' service to add the meta's history (AIMOSCAR removes HTML here!) if possible
				char *proto = GetContactProto(ccs->hContact);
				if (proto) {
					MCONTACT hSub = ccs->hContact;
					DWORD flags = pre->flags;
					ccs->hContact = hMeta;
					pre->flags |= (db_get_b(hMeta, META_PROTO, "WindowOpen", 0) ? 0 : PREF_CREATEREAD);
					if (ProtoServiceExists(proto, PSR_MESSAGE) && !ProtoCallService(proto, PSR_MESSAGE, 0, (LPARAM)ccs))
						added = TRUE;
					ccs->hContact = hSub;
					pre->flags = flags;
				}
			}

			if ( !added) {
				// otherwise add raw db event
				ZeroMemory(&dbei, sizeof(dbei));
				dbei.cbSize = sizeof(dbei);
				dbei.szModule = META_PROTO;
				dbei.timestamp = pre->timestamp;
				dbei.flags = (db_get_b(hMeta, META_PROTO, "WindowOpen", 0) ? 0 : DBEF_READ);
				if (pre->flags & PREF_RTL) dbei.flags |= DBEF_RTL;
				if (pre->flags & PREF_UTF) dbei.flags |= DBEF_UTF;
				dbei.eventType = EVENTTYPE_MESSAGE;
				dbei.cbBlob = (DWORD)strlen(pre->szMessage) + 1;
				if ( pre->flags & PREF_UNICODE ) {
					dbei.cbBlob *= ( sizeof( wchar_t )+1 );
				}
				dbei.pBlob = (PBYTE) pre->szMessage;
				db_event_add(hMeta, &dbei);
			}
		}

		return CallService(MS_PROTO_CHAINRECV, wParam, (LPARAM)ccs);
	}

	MCONTACT hSub = ccs->hContact;
	ccs->hContact = hMeta;	// Forward to the associated MetaContact.
	CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)ccs);
	ccs->hContact = hSub;

	if (options.subhistory && !(ccs->wParam & PREF_METANODB)) {
		// allow event pass through and thereby be added to subcontact history
		pre->flags |= (db_get_b(ccs->hContact, META_PROTO, "WindowOpen", 0) ? 0 : PREF_CREATEREAD);
		return CallService(MS_PROTO_CHAINRECV, wParam, (LPARAM)ccs);		// pass through as normal
	}

	return 1;	// Stop further processing.
}

/** Receive a message for a MetaContact
*
* @return			0
*/
INT_PTR Meta_RecvMessage(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT *) ccs->lParam;

	// contact is not a meta proto contact - just leave it
	char *proto = GetContactProto(ccs->hContact);
	if ( !proto || strcmp(proto, META_PROTO))
		return 0;

	if (options.use_proto_recv) {
		// use the subcontact's protocol to add the db if possible (AIMOSCAR removes HTML here!)
		MCONTACT most_online = Meta_GetMostOnline(ccs->hContact);
		if (char *subProto = GetContactProto(most_online))
			if ( ProtoCallService(subProto, PSR_MESSAGE, wParam, lParam) != CALLSERVICE_NOTFOUND)
				return 0;
	}

	// otherwise, add event to db directly
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = META_PROTO;
	dbei.timestamp = pre->timestamp;
	dbei.flags = (pre->flags & PREF_CREATEREAD ? DBEF_READ : 0);
	if (pre->flags & PREF_RTL) dbei.flags |= DBEF_RTL;
	if (pre->flags & PREF_UTF) dbei.flags |= DBEF_UTF;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (DWORD)strlen(pre->szMessage) + 1;
	if ( pre->flags & PREF_UNICODE )
		dbei.cbBlob *= ( sizeof( wchar_t )+1 );
	dbei.pBlob = (PBYTE) pre->szMessage;
	db_event_add(ccs->hContact, &dbei);
	return 0;
}


/** Called when an ACK is received.
*
* Retransmit the ACK sent by a simple contact so that it
* looks like it was the MetaContact that sends the ACK.
*
* @param wParam :	Allways set to 0.
* @param lParam :	Reference to a ACKDATA that contains
information about the ACK.
* @return			0 on success, 1 otherwise.
*/
int Meta_HandleACK(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*) lParam;
	MCONTACT hUser;

	if (ack->hContact == 0 || (hUser = (MCONTACT)db_get_dw(ack->hContact, META_PROTO, "Handle",0)) == 0)
		return 0;	// Can't find the MetaID, let through the protocol chain


	if ( !strcmp(ack->szModule, META_PROTO)) {
		return 0; // don't rebroadcast our own acks
	}

	// if it's for something we don't support, ignore
	if (ack->type != ACKTYPE_MESSAGE && ack->type != ACKTYPE_CHAT && ack->type != ACKTYPE_FILE && ack->type != ACKTYPE_AWAYMSG
		&& ack->type != ACKTYPE_AVATAR && ack->type != ACKTYPE_GETINFO)

	{
		return 0;
	}

	// change the hContact in the avatar info struct, if it's the avatar we're using - else drop it
	if (ack->type == ACKTYPE_AVATAR) {
		if (ack->result == ACKRESULT_SUCCESS || ack->result == ACKRESULT_FAILED || ack->result == ACKRESULT_STATUS) {
			DBVARIANT dbv;

			// change avatar if the most online supporting avatars changes, or if we don't have one
			MCONTACT most_online = Meta_GetMostOnlineSupporting(hUser, PFLAGNUM_4, PF4_AVATARS);
			//if (AI.hContact == 0 || AI.hContact != most_online) {
			if (ack->hContact == 0 || ack->hContact != most_online) {
				return 0;
			}

			//if ( !db_get(AI.hContact, "ContactPhoto", "File", &dbv)) {
			if ( !db_get(ack->hContact, "ContactPhoto", "File", &dbv)) {
				db_set_ts(hUser, "ContactPhoto", "File", dbv.ptszVal);
				db_free(&dbv);
			}

			if (ack->hProcess) {
				PROTO_AVATAR_INFORMATIONT AI;
				memcpy(&AI, (PROTO_AVATAR_INFORMATIONT *)ack->hProcess, sizeof(PROTO_AVATAR_INFORMATIONT));
				if (AI.hContact)
					AI.hContact = hUser;

				return ProtoBroadcastAck(META_PROTO, hUser,ack->type,ack->result, (HANDLE)&AI, ack->lParam);
			} else
				return ProtoBroadcastAck(META_PROTO, hUser,ack->type,ack->result, 0, ack->lParam);
		}
	}

	return ProtoBroadcastAck(META_PROTO, hUser,ack->type,ack->result,ack->hProcess,ack->lParam);
}

// hiding contacts on "CList/UseGroups" setting changed can cause a crash - do it in a seperate thread during idle time
static DWORD sttHideContacts( BOOL param )
{
	Meta_HideMetaContacts((int)param);
	return 0;
}

/** Call whenever a contact changes one of its settings (for example, the status)
**
* @param wParam \c HANDLE to the contact that has change of its setting.
* @param lParam Reference to a structure that contains the setting that has changed (not used)
*/
int Meta_SettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dcws = (DBCONTACTWRITESETTING *)lParam;
	char buffer[512], szId[40];
	int contact_number;
	MCONTACT hMeta, most_online;

	// hide metacontacts when groups disabled
	if (wParam == 0
		&& ((strcmp(dcws->szModule, "CList") == 0 && strcmp(dcws->szSetting, "UseGroups") == 0)
		|| (strcmp(dcws->szModule, META_PROTO) == 0 && strcmp(dcws->szSetting, "Enabled") == 0)))
	{
		sttHideContacts(!Meta_IsEnabled());
		return 0;
	}

	if (wParam == 0
		&& strcmp(dcws->szModule, "Import") == 0 && strcmp(dcws->szSetting, "Completed") == 0)
	{
		// import process has just been run...call startup routines...
		Meta_SetHandles();

		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			int meta_id;
			if ((meta_id = db_get_dw(hContact, META_PROTO, META_ID,(DWORD)-1)) != (DWORD)-1)
				Meta_CopyData(hContact);
		}

		Meta_HideLinkedContacts();
		Meta_SuppressStatus(options.suppress_status);
	}

	// from here on, we're just interested in contact settings
	if (wParam == 0) return 0;

	if ((hMeta=(MCONTACT)db_get_dw(wParam, META_PROTO, "Handle",0))!=0 && CallService(MS_DB_CONTACT_IS, (WPARAM)hMeta, 0)) // just to be safe
	{
		// This contact is attached to a MetaContact.

		contact_number = Meta_GetContactNumber(wParam);
		if (contact_number == -1) return 0; // exit - db corruption

		if ( !meta_group_hack_disabled && !strcmp(dcws->szModule, "CList") && !strcmp(dcws->szSetting, "Group") &&
			Meta_IsEnabled() && db_get_b(wParam, META_PROTO, "Hidden", 0) == 0 && !Miranda_Terminated()) {
				if ((dcws->value.type == DBVT_ASCIIZ || dcws->value.type == DBVT_UTF8) && !Meta_IsHiddenGroup(dcws->value.pszVal)) {
					// subcontact group reassigned - copy to saved group
					db_set(wParam, META_PROTO, "OldCListGroup", &dcws->value);
					db_set_s(wParam, "CList", "Group", META_HIDDEN_GROUP);
				} else if (dcws->value.type == DBVT_DELETED) {
					db_unset(wParam, META_PROTO, "OldCListGroup");
					db_set_s(wParam, "CList", "Group", META_HIDDEN_GROUP);
				}
		}
		else if ( !strcmp(dcws->szSetting, "IP")) {
			if (dcws->value.type == DBVT_DWORD)
				db_set_dw(hMeta, META_PROTO, "IP", dcws->value.dVal);
			else
				db_unset(hMeta, META_PROTO, "IP");
		}
		else if ( !strcmp(dcws->szSetting, "RealIP")) {
			if (dcws->value.type == DBVT_DWORD)
				db_set_dw(hMeta, META_PROTO, "RealIP", dcws->value.dVal);
			else
				db_unset(hMeta, META_PROTO, "RealIP");

		}
		else if ( !strcmp(dcws->szSetting, "ListeningTo")) {
			switch(dcws->value.type) {
			case DBVT_ASCIIZ:
				db_set_s(hMeta, META_PROTO, "ListeningTo", dcws->value.pszVal);
				break;
			case DBVT_UTF8:
				db_set_utf(hMeta, META_PROTO, "ListeningTo", dcws->value.pszVal);
				break;
			case DBVT_WCHAR:
				db_set_ws(hMeta, META_PROTO, "ListeningTo", dcws->value.pwszVal);
				break;
			case DBVT_DELETED:
				db_unset(hMeta, META_PROTO, "ListeningTo");
				break;
			}
		}
		else if ( !strcmp(dcws->szSetting, "Nick") && !dcws->value.type == DBVT_DELETED) {
			// subcontact nick has changed - update metacontact
			strcpy(buffer, "Nick");
			strcat(buffer, _itoa(contact_number, szId, 10));
			db_set(hMeta, META_PROTO, buffer, &dcws->value);

			DBVARIANT dbv;
			if (Mydb_get(wParam, "CList", "MyHandle", &dbv)) {
				strcpy(buffer, "CListName");
				strcat(buffer, _itoa(contact_number, szId, 10));
				db_set(hMeta, META_PROTO, buffer, &dcws->value);
			}
			else db_free(&dbv);

			// copy nick to metacontact, if it's the most online
			MCONTACT most_online = Meta_GetMostOnline(hMeta);
			Meta_CopyContactNick(hMeta, most_online);

			return 0;
		}
		else if ( !strcmp(dcws->szSetting, "IdleTS")) {
			if (dcws->value.type == DBVT_DWORD)
				db_set_dw(hMeta, META_PROTO, "IdleTS", dcws->value.dVal);
			else if (dcws->value.type == DBVT_DELETED)
				db_set_dw(hMeta, META_PROTO, "IdleTS", 0);
		}
		else if ( !strcmp(dcws->szSetting, "LogonTS")) {
			if (dcws->value.type == DBVT_DWORD)
				db_set_dw(hMeta, META_PROTO, "LogonTS", dcws->value.dVal);
			else if (dcws->value.type == DBVT_DELETED)
				db_set_dw(hMeta, META_PROTO, "LogonTS", 0);
		}
		else if ( !strcmp(dcws->szModule, "CList") && !strcmp(dcws->szSetting, "MyHandle")) {
			if (dcws->value.type == DBVT_DELETED) {
				char *proto = GetContactProto(wParam);
				strcpy(buffer, "CListName");
				strcat(buffer, _itoa(contact_number, szId, 10));

				DBVARIANT dbv;
				if (proto && !Mydb_get(wParam, proto, "Nick", &dbv)) {
					db_set(hMeta, META_PROTO, buffer, &dbv);
					db_free(&dbv);
				} else {
					db_unset(hMeta, META_PROTO, buffer);
				}
			} else {
				// subcontact clist displayname has changed - update metacontact
				strcpy(buffer, "CListName");
				strcat(buffer, _itoa(contact_number, szId, 10));

				db_set(hMeta, META_PROTO, buffer, &dcws->value);
			}

			// copy nick to metacontact, if it's the most online
			MCONTACT most_online = Meta_GetMostOnline(hMeta);
			Meta_CopyContactNick(hMeta, most_online);

			return 0;
		}
		else if ( !strcmp(dcws->szSetting, "Status") && !dcws->value.type == DBVT_DELETED) {
			// subcontact changing status

			// update subcontact status setting
			strcpy(buffer, "Status");
			strcat(buffer, _itoa(contact_number, szId, 10));
			db_set_w(hMeta, META_PROTO, buffer, dcws->value.wVal);

			strcpy(buffer, "StatusString");
			strcat(buffer, _itoa(contact_number, szId, 10));
			db_set_ts(hMeta, META_PROTO, buffer, pcli->pfnGetStatusModeDescription(dcws->value.wVal, 0));

			// if the contact was forced, unforce it (which updates status)
			if ((HANDLE)db_get_dw(hMeta, META_PROTO, "ForceSend", 0) == (HANDLE)wParam)
				MetaAPI_UnforceSendContact((WPARAM)hMeta, 0);
			else {
				// set status to that of most online contact
				most_online = Meta_GetMostOnline(hMeta);
				Meta_CopyContactNick(hMeta, most_online);

				Meta_FixStatus(hMeta);
				Meta_CopyData(hMeta);
			}

			// most online contact with avatar support might have changed - update avatar
			most_online = Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_4, PF4_AVATARS);
			if (most_online) {
				PROTO_AVATAR_INFORMATIONT AI;

				AI.cbSize = sizeof(AI);
				AI.hContact = hMeta;
				AI.format = PA_FORMAT_UNKNOWN;
				_tcscpy(AI.filename, _T("X"));

				if ((int)CallProtoService(META_PROTO, PS_GETAVATARINFOT, 0, (LPARAM)&AI) == GAIR_SUCCESS)
					db_set_ts(hMeta, "ContactPhoto", "File",AI.filename);
			}
		}
		else if (strcmp(dcws->szSetting, "XStatusId") == 0 || strcmp(dcws->szSetting, "XStatusMsg") == 0 || strcmp(dcws->szSetting, "XStatusName") == 0 || strcmp(dcws->szSetting, "StatusMsg") == 0) {
			Meta_CopyData(hMeta);
		}
		else if (strcmp(dcws->szSetting, "MirVer") == 0) {
			Meta_CopyData(hMeta);
		}
		else if ( !meta_group_hack_disabled && !strcmp(dcws->szModule, "CList") && !strcmp(dcws->szSetting, "Hidden")) {
			if ((dcws->value.type == DBVT_DELETED || db_get_b(wParam, "CList", "Hidden", 0) == 0)
				&& db_get_b(wParam, META_PROTO, "Hidden", 0) == 1)
			{
				// a subcontact we hid (e.g. jabber) has been unhidden - hide it again :(
				db_set_b(wParam, "CList", "Hidden", 1);
			}
		}
	}

	return 0;
}

int Meta_ContactDeleted(WPARAM wParam, LPARAM lParam)
{
	// is a subcontact - update meta contact
	MCONTACT hMeta = (MCONTACT)db_get_dw(wParam, META_PROTO, "Handle", 0);
	if (hMeta) {
		Meta_RemoveContactNumber(hMeta, db_get_dw(wParam, META_PROTO, "ContactNumber", -1));
		NotifyEventHooks(hSubcontactsChanged, (WPARAM)hMeta, 0);
		return 0;
	}

	// not a subcontact - is it a metacontact?
	int num_contacts = db_get_dw(wParam, META_PROTO, "NumContacts", 0);
	if (num_contacts)
		NotifyEventHooks(hSubcontactsChanged, (WPARAM)wParam, 0);

	// remove & restore all subcontacts
	for (int i = 0; i < num_contacts; i++) {
		MCONTACT hContact = Meta_GetContactHandle(wParam, i);
		if (hContact && (HANDLE)db_get_dw(hContact, META_PROTO, "Handle", 0) == (HANDLE)wParam) {
			if (db_get_b(hContact, META_PROTO, "IsSubcontact", 0) == 1)
				db_unset(hContact, META_PROTO, "IsSubcontact");
			db_unset(hContact, META_PROTO, META_LINK);
			db_unset(hContact, META_PROTO, "Handle");
			db_unset(hContact, META_PROTO, "ContactNumber");
			Meta_RestoreGroup(hContact);
			db_unset(hContact, META_PROTO, "OldCListGroup");

			// stop ignoring, if we were
			if (options.suppress_status)
				CallService(MS_IGNORE_UNIGNORE, hContact, (WPARAM)IGNOREEVENT_USERONLINE);
		}
	}
	return 0;
}

/** Call when we want to send a user is typing message
*
* @param wParam \c HANDLE to the contact that we are typing to
* @param lParam either PROTOTYPE_SELFTYPING_ON or PROTOTYPE_SELFTYPING_OFF
*/
INT_PTR Meta_UserIsTyping(WPARAM wParam, LPARAM lParam)
{
	// This is a simple contact, let through the stack of protocols
	if (db_get_dw(wParam, META_PROTO, META_ID,(DWORD)-1) == (DWORD)-1)
		return 0;

	// forward to sending protocol, if supported

	MCONTACT most_online = Meta_GetMostOnline(wParam);
	Meta_CopyContactNick(wParam, most_online);
	if ( !most_online)
		return 0;

	char *proto = GetContactProto(most_online);
	if (proto)
		if ( ProtoServiceExists(proto, PSS_USERISTYPING))
			ProtoCallService(proto, PSS_USERISTYPING, (WPARAM)most_online, (LPARAM)lParam);

	return 0;
}

/** Call when we want to receive a user is typing message
*
* @param wParam \c HANDLE to the contact that is typing or not
* @param lParam either PROTOTYPE_SELFTYPING_ON or PROTOTYPE_SELFTYPING_OFF
*/
int Meta_ContactIsTyping(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hMeta;
	if ((hMeta = (MCONTACT)db_get_dw(wParam, META_PROTO, "Handle",0)) != 0 && Meta_IsEnabled()) {
		// This contact is attached to a MetaContact.
		if ( !options.subcontact_windows) { // we don't want clicking on the clist notification icon to open the metacontact message window
			// try to remove any clist events we added for subcontact
			CallServiceSync(MS_CLIST_REMOVEEVENT, wParam, (LPARAM) 1);
			CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hMeta, lParam);
			// stop processing of event
			return 1;
		}
	}

	return 0;
}

/** Called when user info is about to be shown
*
* Returns 1 to stop event processing and opens page for metacontact default contact (returning 1 to stop it doesn't work!)
*
*/

int Meta_UserInfo(WPARAM wParam, LPARAM lParam)
{
	DWORD default_contact_number = db_get_dw(lParam, META_PROTO, "Default", (DWORD)-1);

	if (default_contact_number == -1) // not a meta contact
		return 0;

	CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)Meta_GetContactHandle(lParam, default_contact_number), 0);
	return 1;
}

// handle message window api ver 0.0.0.1+ events - record window open/close status for subcontacts, so we know whether to
// let received messages through and add db history to metacontact, or vice versa
int Meta_MessageWindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *mwed = (MessageWindowEventData *)lParam;
	MCONTACT hMeta = 0;

	message_window_api_enabled = TRUE;

	if ((hMeta = (MCONTACT)db_get_dw(mwed->hContact, META_PROTO, "Handle", 0)) != 0
		|| db_get_dw(mwed->hContact, META_PROTO, META_ID, (DWORD)-1) != (DWORD)-1)
	{
		// contact is subcontact of metacontact, or an actual metacontact - record whether window is open or closed
		if (mwed->uType == MSG_WINDOW_EVT_OPEN || mwed->uType == MSG_WINDOW_EVT_OPENING) {
			db_set_b(mwed->hContact, META_PROTO, "WindowOpen", 1);

			if (hMeta) { // subcontact window opened - remove clist events we added for metacontact
				while(!CallService(MS_CLIST_REMOVEEVENT, (WPARAM)hMeta, (LPARAM)mwed->hContact));
			}
		} else if (mwed->uType == MSG_WINDOW_EVT_CLOSE || mwed->uType == MSG_WINDOW_EVT_CLOSING) {
			db_set_b(mwed->hContact, META_PROTO, "WindowOpen", 0);
			if ( !hMeta) { // hMeta is 0 for metacontact (sorry)
				DWORD saved_def;

				MetaAPI_UnforceSendContact((WPARAM)mwed->hContact, 0);

				// restore saved default contact
				if (options.set_default_on_recv) {
					saved_def = db_get_dw(mwed->hContact, META_PROTO, "SavedDefault", -1);
					if (options.temp_default && saved_def != (DWORD)-1) {
						db_set_dw(mwed->hContact, META_PROTO, "Default", saved_def);
						db_set_dw(mwed->hContact, META_PROTO, "SavedDefault", (DWORD)-1);
						NotifyEventHooks(hEventDefaultChanged, (WPARAM)mwed->hContact, (LPARAM)Meta_GetContactHandle(hMeta, saved_def)); // nick set in event handler
					}
				}
			}
		}
	}

	return 0;
}

int Meta_ClistDoubleClicked(WPARAM wParam, LPARAM lParam)
{
	if (db_get_dw(wParam, META_PROTO, "Default",(WORD)-1) == (WORD)-1)
		return 0;

	// -1 indicates  no specific capability but respect 'ForceDefault'
	MCONTACT most_online = Meta_GetMostOnlineSupporting(wParam, PFLAGNUM_1, -1);
	if ( !most_online)
		return 0;

	if (options.subcontact_windows) {
		if (lParam) // contact from incoming message in lParam via (at this point) clist message event
			CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)lParam, 0);
		else // simulate double click on most_online contact and stop event processing
			CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)most_online, 0);
		return 1;
	}

	char *proto = GetContactProto(most_online);
	if (proto == NULL)
		return 0;

	char buffer[512];
	strcpy(buffer, proto);
	strcat(buffer, PS_GETCAPS);

	// get the contacts messaging capabilities
	int caps = CallService(buffer, (WPARAM)PFLAGNUM_1, 0);
	if ((caps & PF1_IMSEND) || (caps & PF1_CHAT) || (proto && strcmp(proto, "IRC") == 0))
		// let event process normally
		return 0;

	// simulate double click on most_online contact and stop event processing
	CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)most_online, 0);
	return 1;
}

INT_PTR Meta_ClistMessageEventClicked(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = ((CLISTEVENT *)lParam)->hContact;
	return Meta_ClistDoubleClicked(hContact, (LPARAM)((CLISTEVENT *)lParam)->hDbEvent);
}

int NudgeRecieved(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

/** Called when all the plugin are loaded into Miranda.
*
* Initializes the 4 menus present in the context-menu
* and the initial value of nextMetaID
*/
int Meta_ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	char buffer[512], buffer2[512], buffer3[512];
	int i;

	if (ServiceExists(MS_MSG_GETWINDOWAPI))
		message_window_api_enabled = TRUE;

	// disable group hack for older nicer versions without the fix
	if (ServiceExists(MS_CLUI_GETVERSION)) {
		char *version = (char *)CallService(MS_CLUI_GETVERSION, 0, 0);
		if (version && strlen(version) >= strlen("CList Nicer+") && strncmp(version, "CList Nicer+", strlen("CList Nicer+")) == 0)
			meta_group_hack_disabled = TRUE;
	}

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, Meta_ModifyMenu);
	HookEvent(ME_CLIST_DOUBLECLICKED, Meta_ClistDoubleClicked );

	InitIcons();

	////////////////////////////////////////////////////////////////////////////
	CLISTMENUITEM mi = { sizeof(mi) };

	// main menu item
	mi.icolibItem = GetIconHandle(I_MENUOFF);
	mi.pszName = LPGEN("Toggle MetaContacts Off");
	mi.pszService = "MetaContacts/OnOff";
	mi.position = 500010000;
	hMenuOnOff = Menu_AddMainMenuItem(&mi);

	// contact menu items
	mi.icolibItem = GetIconHandle(I_CONVERT);
	mi.position = -200010;
	mi.pszName = LPGEN("Convert to MetaContact");
	mi.pszService = "MetaContacts/Convert";
	hMenuConvert = Menu_AddContactMenuItem(&mi);

	mi.icolibItem = GetIconHandle(I_ADD);
	mi.position = -200009;
	mi.pszName = LPGEN("Add to existing MetaContact...");
	mi.pszService = "MetaContacts/AddTo";
	hMenuAdd = Menu_AddContactMenuItem(&mi);

	mi.icolibItem = GetIconHandle(I_EDIT);
	mi.position = -200010;
	mi.pszName = LPGEN("Edit MetaContact...");
	mi.pszService = "MetaContacts/Edit";
	hMenuEdit = Menu_AddContactMenuItem(&mi);

	mi.icolibItem = GetIconHandle(I_SETDEFAULT);
	mi.position = -200009;
	mi.pszName = LPGEN("Set as MetaContact default");
	mi.pszService = "MetaContacts/Default";
	hMenuDefault = Menu_AddContactMenuItem(&mi);

	mi.icolibItem = GetIconHandle(I_REMOVE);
	mi.position = -200008;
	mi.pszName = LPGEN("Delete MetaContact");
	mi.pszService = "MetaContacts/Delete";
	hMenuDelete = Menu_AddContactMenuItem(&mi);

	mi.flags |= CMIF_HIDDEN;
	mi.pszContactOwner = META_PROTO;

	mi.position = -99000;
	for (i = 0; i < MAX_CONTACTS; i++) {
		mi.position--;
		strcpy(buffer3, (char *)Translate("Context"));
		strcat(buffer3, _itoa(i, buffer2, 10));
		mi.pszName = buffer3;

		strcpy(buffer, "MetaContacts/MenuFunc");
		strcat(buffer, _itoa(i, buffer2, 10));
		mi.pszService= buffer;

		hMenuContact[i] = Menu_AddContactMenuItem(&mi);
	}

	nextMetaID = db_get_dw(NULL, META_PROTO, "NextMetaID",0);

	// loop and copy data from subcontacts
	if (options.copydata) {
		int meta_id;
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
			if ((meta_id = db_get_dw(hContact, META_PROTO, META_ID,(DWORD)-1))!=(DWORD)-1)
				Meta_CopyData(hContact);
	}

	Meta_HideLinkedContacts();

	if ( !Meta_IsEnabled()) {
		// modify main menu item
		mi.flags = CMIM_NAME | CMIM_ICON;
		mi.icolibItem = GetIconHandle(I_MENU);
		mi.pszName = LPGEN("Toggle MetaContacts On");
		Menu_ModifyItem(hMenuOnOff, &mi);

		Meta_HideMetaContacts(TRUE);
	}
	else Meta_SuppressStatus(options.suppress_status);

	// hook srmm window close/open events - message api ver 0.0.0.1+
	if (HookEvent(ME_MSG_WINDOWEVENT, Meta_MessageWindowEvent))
		message_window_api_enabled = TRUE;

	// hook protocol nudge events to forward to subcontacts
	int numberOfProtocols;
	PROTOACCOUNT ** ppProtocolDescriptors;
	ProtoEnumAccounts(&numberOfProtocols, &ppProtocolDescriptors);

	for (int i = 0; i < numberOfProtocols ; i++)
		if ( strcmp(ppProtocolDescriptors[i]->szModuleName, META_PROTO)) {
			char str[MAXMODULELABELLENGTH + 10];
			mir_snprintf(str, SIZEOF(str), "%s/Nudge", ppProtocolDescriptors[i]->szModuleName);
			HookEvent(str, NudgeRecieved);
		}

	return 0;
}

static VOID CALLBACK sttMenuThread( PVOID param )
{
	HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)param, 0);

	TPMPARAMS tpmp = { 0 };
	tpmp.cbSize = sizeof(tpmp);
	BOOL menuRet = TrackPopupMenuEx(hMenu, TPM_RETURNCMD, menuMousePoint.x, menuMousePoint.y, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), &tpmp);

	CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(menuRet), MPCF_CONTACTMENU), (LPARAM)param);

	DestroyMenu(hMenu);
}

INT_PTR Meta_ContactMenuFunc(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = Meta_GetContactHandle(wParam, (int)lParam);

	if (options.menu_function == FT_MSG) {
		// open message window if protocol supports message sending or chat, else simulate double click

		int caps;
		char *proto;
		char buffer[512];

		proto = GetContactProto(hContact);

		if (proto) {
			strcpy(buffer, proto);
			strcat(buffer, PS_GETCAPS);

			caps = CallService(buffer, (WPARAM)PFLAGNUM_1, 0);

			if ((caps & PF1_IMSEND) || (caps & PF1_CHAT) || (proto && strcmp(proto, "IRC") == 0)) {
				// set default contact for sending/status and open message window
				db_set_dw(wParam, META_PROTO, "Default", (DWORD)(int)lParam);
				NotifyEventHooks(hEventDefaultChanged, wParam, (LPARAM)hContact);
				CallService(MS_MSG_SENDMESSAGE, wParam, 0);
			} else
				// protocol does not support messaging - simulate double click
				CallService(MS_CLIST_CONTACTDOUBLECLICKED, hContact, 0);
		} else
			// protocol does not support messaging - simulate double click
			CallService(MS_CLIST_CONTACTDOUBLECLICKED, hContact, 0);

	} else if (options.menu_function == FT_MENU) {
		// show contact's context menu
		CallFunctionAsync(sttMenuThread, (void*)hContact);
	} else if (options.menu_function == FT_INFO) {
		// show user info for subcontact
		CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
	}

	return 0;
}

////////////////////
// file transfer support - mostly not required, since subcontacts do the receiving
////////////////////

INT_PTR Meta_FileSend(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	char *proto = 0;
	DWORD default_contact_number;

	if ((default_contact_number = db_get_dw(ccs->hContact, META_PROTO, "Default",(DWORD)-1)) == (DWORD)-1)
	{
		// This is a simple contact
		// (this should normally not happen, since linked contacts do not appear on the list.)
		//PUShowMessage("meta has no default", SM_NOTIFY);
		return 0;
	}
	else
	{
		MCONTACT most_online = Meta_GetMostOnlineSupporting(ccs->hContact, PFLAGNUM_1, PF1_FILESEND);
		if ( !most_online) {
			//PUShowMessage("no most online for ft", SM_NOTIFY);
			return 0;
		}

		proto = GetContactProto(most_online);
		if (proto)
			return (int)(CallContactService(most_online, PSS_FILE, ccs->wParam, ccs->lParam));
	}
	return 0; // fail
}

INT_PTR Meta_GetAwayMsg(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	char *proto = 0;
	DWORD default_contact_number;

	if ((default_contact_number = db_get_dw(ccs->hContact, META_PROTO, "Default",(DWORD)-1)) == (DWORD)-1)
	{
		// This is a simple contact
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return 0;
	}
	else
	{
		MCONTACT most_online = Meta_GetMostOnlineSupporting(ccs->hContact, PFLAGNUM_1, PF1_MODEMSGRECV);
		if ( !most_online)
			return 0;

		proto = GetContactProto(most_online);
		if ( !proto) return 0;

		//Meta_CopyContactNick(ccs->hContact, most_online, proto);

		ccs->hContact = most_online;
		//Meta_SetNick(proto);

		return (int)(CallContactService(ccs->hContact, PSS_GETAWAYMSG, ccs->wParam, ccs->lParam));
	}
	return 0; // fail
}

INT_PTR Meta_GetAvatarInfo(WPARAM wParam, LPARAM lParam) {
	PROTO_AVATAR_INFORMATIONT *AI = (PROTO_AVATAR_INFORMATIONT *) lParam;
	DWORD default_contact_number;

	if ((default_contact_number = db_get_dw(AI->hContact, META_PROTO, "Default",(DWORD)-1)) == (DWORD)-1)
	{
		// This is a simple contact
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return 0;
	}
	else
	{
		MCONTACT hMeta = AI->hContact;
		MCONTACT hSub = Meta_GetMostOnlineSupporting(AI->hContact, PFLAGNUM_4, PF4_AVATARS);
		if ( !hSub)
			return GAIR_NOAVATAR;

		char *proto = GetContactProto(hSub);
		if ( !proto) return GAIR_NOAVATAR;

		AI->hContact = hSub;

		INT_PTR result = ProtoCallService(proto, PS_GETAVATARINFOT, wParam, lParam);
		AI->hContact = hMeta;
		if (result != CALLSERVICE_NOTFOUND)
			return result;
	}
	return GAIR_NOAVATAR; // fail
}

INT_PTR Meta_GetInfo(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	DWORD default_contact_number;

	// This is a simple contact
	// (this should normally not happen, since linked contacts do not appear on the list.)
	if ((default_contact_number = db_get_dw(ccs->hContact, META_PROTO, "Default",(DWORD)-1)) == (DWORD)-1)
		return 0;

	MCONTACT most_online = Meta_GetMostOnlineSupporting(ccs->hContact, PFLAGNUM_4, PF4_AVATARS);
	if ( !most_online)
		return 0;

	char *proto = GetContactProto(most_online);
	if ( !proto) return 0;

	PROTO_AVATAR_INFORMATIONT AI;
	AI.cbSize = sizeof(AI);
	AI.hContact = ccs->hContact;
	AI.format = PA_FORMAT_UNKNOWN;
	_tcscpy(AI.filename, _T("X"));
	if ((int)CallProtoService(META_PROTO, PS_GETAVATARINFOT, 0, (LPARAM)&AI) == GAIR_SUCCESS)
		db_set_ts(ccs->hContact, "ContactPhoto", "File",AI.filename);

	most_online = Meta_GetMostOnline(ccs->hContact);
	Meta_CopyContactNick(ccs->hContact, most_online);

	if ( !most_online)
		return 0;

	ccs->hContact = most_online;
	if ( !ProtoServiceExists(proto, PSS_GETINFO))
		return 0; // fail

	return CallContactService(ccs->hContact, PSS_GETINFO, ccs->wParam, ccs->lParam);
}

int Meta_OptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = -790000000;
	odp.hInstance = hInstance;
	odp.flags = ODPF_BOLDGROUPS;

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszTitle = LPGEN("MetaContacts");
	odp.pszGroup = LPGEN("Contacts");
	odp.pszTab = LPGEN("General");
	odp.pfnDlgProc = DlgProcOpts;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PRIORITIES);
	odp.pszTab = LPGEN("Priorities");
	odp.pfnDlgProc = DlgProcOptsPriorities;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_HISTORY);
	odp.pszTab = LPGEN("History");
	odp.pfnDlgProc = DlgProcOpts;
	Options_AddPage(wParam, &odp);
	return 0;
}

int Meta_CallMostOnline(WPARAM wParam, LPARAM lParam)
{
	MCONTACT most_online_im = Meta_GetMostOnline(wParam);

	// fix nick
	Meta_CopyContactNick(wParam, most_online_im);

	// fix status
	Meta_FixStatus(wParam);

	// copy all other data
	Meta_CopyData((MCONTACT) wParam);
	return 0;
}

INT_PTR Meta_OnOff(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_NAME | CMIM_ICON;
	// just write to db - the rest is handled in the Meta_SettingChanged function
	if (db_get_b(0, META_PROTO, "Enabled", 1)) {
		db_set_b(0, META_PROTO, "Enabled", 0);
		// modify main mi item
		mi.icolibItem = GetIconHandle(I_MENU);
		mi.pszName = LPGEN("Toggle MetaContacts On");
	} else {
		db_set_b(0, META_PROTO, "Enabled", 1);
		// modify main mi item
		mi.icolibItem = GetIconHandle(I_MENUOFF);
		mi.pszName = LPGEN("Toggle MetaContacts Off");
	}
	Menu_ModifyItem(hMenuOnOff, &mi);
	return 0;
}


int Meta_PreShutdown(WPARAM wParam, LPARAM lParam) {
	Meta_SetStatus((WPARAM)ID_STATUS_OFFLINE, 0);
	Meta_UnhideLinkedContacts();
	Meta_SuppressStatus(FALSE);
	//MessageBox(0, "Status is OFFLINE", "MC", MB_OK);
	//MessageBox(0, "Preshutdown complete", "MC", MB_OK);

	if (setStatusTimerId) KillTimer(0, setStatusTimerId);

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
	CreateServiceFunction("MetaContacts/ForceDefault", Meta_ForceDefault);

	// hidden contact menu items...ho hum
	for (int i=0; i < MAX_CONTACTS; i++) {
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

	CreateProtoServiceFunction(META_PROTO, PSS_USERISTYPING, Meta_UserIsTyping );

	CreateProtoServiceFunction(META_PROTO, PSR_MESSAGE, Meta_RecvMessage);

	// file recv is done by subcontacts
	CreateProtoServiceFunction(META_PROTO, PSS_FILE, Meta_FileSend);

	CreateProtoServiceFunction(META_PROTO, PSS_GETAWAYMSG, Meta_GetAwayMsg);

	CreateProtoServiceFunction(META_PROTO, PS_GETAVATARINFOT, Meta_GetAvatarInfo);

	CreateProtoServiceFunction(META_PROTO, PSS_GETINFO, Meta_GetInfo);

	CreateProtoServiceFunction(META_FILTER, PSR_MESSAGE, MetaFilter_RecvMessage);
	CreateProtoServiceFunction(META_FILTER, PSS_MESSAGE, MetaFilter_SendMessage);

	// API services and events
	CreateServiceFunction(MS_MC_GETMETACONTACT, MetaAPI_GetMeta);
	CreateServiceFunction(MS_MC_GETDEFAULTCONTACT, MetaAPI_GetDefault);
	CreateServiceFunction(MS_MC_GETDEFAULTCONTACTNUM, MetaAPI_GetDefaultNum);
	CreateServiceFunction(MS_MC_GETMOSTONLINECONTACT, MetaAPI_GetMostOnline);
	CreateServiceFunction(MS_MC_GETNUMCONTACTS, MetaAPI_GetNumContacts);
	CreateServiceFunction(MS_MC_GETSUBCONTACT, MetaAPI_GetContact);
	CreateServiceFunction(MS_MC_SETDEFAULTCONTACTNUM, MetaAPI_SetDefaultContactNum);
	CreateServiceFunction(MS_MC_SETDEFAULTCONTACT, MetaAPI_SetDefaultContact);
	CreateServiceFunction(MS_MC_FORCESENDCONTACTNUM, MetaAPI_ForceSendContactNum);
	CreateServiceFunction(MS_MC_FORCESENDCONTACT, MetaAPI_ForceSendContact);
	CreateServiceFunction(MS_MC_UNFORCESENDCONTACT, MetaAPI_UnforceSendContact);
	CreateServiceFunction(MS_MC_GETPROTOCOLNAME, MetaAPI_GetProtoName);
	CreateServiceFunction(MS_MC_GETFORCESTATE, MetaAPI_GetForceState);

	CreateServiceFunction(MS_MC_CONVERTTOMETA, MetaAPI_ConvertToMeta);
	CreateServiceFunction(MS_MC_ADDTOMETA, MetaAPI_AddToMeta);
	CreateServiceFunction(MS_MC_REMOVEFROMMETA, MetaAPI_RemoveFromMeta);

	CreateServiceFunction(MS_MC_DISABLEHIDDENGROUP, MetaAPI_DisableHiddenGroup);

	CreateServiceFunction("MetaContacts/OnOff", Meta_OnOff);
	CreateServiceFunction("MetaContacts/CListMessageEvent", Meta_ClistMessageEventClicked);

	CreateProtoServiceFunction(META_PROTO, PS_SEND_NUDGE, Meta_SendNudge);

	// create our hookable events
	hEventDefaultChanged = CreateHookableEvent(ME_MC_DEFAULTTCHANGED);
	hEventForceSend = CreateHookableEvent(ME_MC_FORCESEND);
	hEventUnforceSend = CreateHookableEvent(ME_MC_UNFORCESEND);
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
	HookEvent(ME_MC_FORCESEND, Meta_CallMostOnline);
	HookEvent(ME_MC_UNFORCESEND, Meta_CallMostOnline);

	// redirect nudge events
	hEventNudge = CreateHookableEvent(META_PROTO "/Nudge");
}

//! Unregister all hooks and services from Miranda
void Meta_CloseHandles()
{
	// destroy our hookable events
	DestroyHookableEvent(hEventDefaultChanged);
	DestroyHookableEvent(hEventForceSend);
	DestroyHookableEvent(hEventUnforceSend);
	DestroyHookableEvent(hSubcontactsChanged);
	DestroyHookableEvent(hEventNudge);
}
