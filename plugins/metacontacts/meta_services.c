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

#define NB_SERVICES		62	//!< Number of services registered in Miranda (see Meta_CloseHandles()).
#define NB_HOOKS		17	//!< Number of hooks set up (see Meta_CloseHandles()).

#define PREF_METANODB	0x2000	//!< Flag to indicate message should not be added to db by filter when sending

char *pendingACK = 0;		//!< Name of the protocol in which an ACK is about to come.

int previousMode,			//!< Previous status of the MetaContacts Protocol
	mcStatus;				//!< Current status of the MetaContacts Protocol

HANDLE	hServices[NB_SERVICES] = {0},	//!< list of all the services registered (see Meta_CloseHandles()).
hHooks[NB_HOOKS] = {0};		//!< list of all hooks set up (see Meta_CloseHandles()).

HANDLE *hNudgeEvents = 0;
int iNudgeProtos = 0;

HANDLE	hMenuConvert,		//!< \c HANDLE to the convert menu item.
		hMenuAdd,			//!< \c HANDLE to the add to menu item.
		hMenuEdit,			//!< \c HANDLE to the edit menu item.
		hMenuDelete,		//!< \c HANDLE to the delete menu item.
		hMenuDefault,		//!< \c HANDLE to the delete menu item.
		hMenuForceDefault;	//!< \c HANDLE to the delete menu item.

HANDLE	hMenuOnOff;		//!< \c HANDLE to the enable/disable menu item.

HANDLE	hEventDefaultChanged,	//!< \c HANDLE to the 'default changed' event
		hEventForceSend,		//!< \c HANDLE to the 'force send' event
		hEventUnforceSend,		//!< \c HANDLE to the 'unforce send' event
		hSubcontactsChanged,	//!< \c HANDLE to the 'contacts changed' event
		hEventNudge;


DWORD nextMetaID;	//!< Global variable specifying the ID value the next MetaContact will have.

BOOL message_window_api_enabled = FALSE; //!< Global variable specifying whether the message window api ver 0.0.0.1+ is available

// stuff for mw_clist extra icon
int proto_count = 0;
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
	int ret = 0;
    switch (wParam) {
        case PFLAGNUM_1:
			//ret = PF1_IM | PF1_URL | PF1_FILE | PF1_MODEMSG | PF1_AUTHREQ | PF1_ADDED;
			//ret = PF1_IMSEND | PF1_URLSEND | PF1_FILESEND | PF1_MODEMSGSEND;
			ret = PF1_IM | PF1_CHAT | PF1_FILESEND | PF1_MODEMSGRECV | PF1_NUMERICUSERID;
			break;
        case PFLAGNUM_2:
			if(!options.suppress_proto) {
	            ret =	PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND
						| PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;
			}
			//ret = PF2_ONLINE;
			break;
        case PFLAGNUM_3:
            //ret = PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND;
            ret =	PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND
					| PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;
            break;
        case PFLAGNUM_4:
            //ret = PF4_FORCEAUTH;
			ret = PF4_SUPPORTTYPING | PF4_AVATARS;
            break;
        case PFLAGNUM_5:
	            ret =	PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND
						| PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;
            break;
        case PFLAG_UNIQUEIDTEXT:
            ret = (INT_PTR) Translate("Meta ID");
            break;
        case PFLAG_MAXLENOFMESSAGE:
            ret = 2000;
            break;
        case PFLAG_UNIQUEIDSETTING:
            ret = (INT_PTR) META_ID;
            break;
    }
    return ret;
}

/** Copy the name of the protocole into lParam
* @param wParam :	max size of the name
* @param lParam :	reference to a char *, which will hold the name
*/
INT_PTR Meta_GetName(WPARAM wParam,LPARAM lParam)
{
	char *name = (char *)Translate(META_PROTO);
	size_t size = min(strlen(name),wParam-1);	// copy only the first size bytes.
	if(strncpy((char *)lParam,name,size)==NULL)
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
    switch (wParam & 0xFFFF)
	{
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


//static DWORD CALLBACK SetStatusThread( LPVOID param )
void CALLBACK SetStatusThread(HWND hWnd, UINT msg, UINT_PTR id, DWORD dw)
{

	previousMode = mcStatus;

	//Sleep(options.set_status_from_offline_delay);

	mcStatus = (int)ID_STATUS_ONLINE;
	ProtoBroadcastAck(META_PROTO,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS, (HANDLE)previousMode, mcStatus);

	//return 0;
	KillTimer(0, setStatusTimerId);
}

/** Changes the status and notifies everybody
* @param wParam :	The new mode
* @param lParam :	Allways set to 0.
*/
INT_PTR Meta_SetStatus(WPARAM wParam,LPARAM lParam)
{
	// firstSetOnline starts out true - used to delay metacontact's 'onlineness' to prevent double status notifications on startup
	if(mcStatus == ID_STATUS_OFFLINE && firstSetOnline) {
		// causes crash on exit if miranda is closed in under options.set_status_from_offline milliseconds!
		//CloseHandle( CreateThread( NULL, 0, SetStatusThread, (void *)wParam, 0, 0 ));		
		setStatusTimerId = SetTimer(0, 0, options.set_status_from_offline_delay, SetStatusThread);
		firstSetOnline = FALSE;
	} else {
		previousMode = mcStatus;
		mcStatus = (int)wParam;
		ProtoBroadcastAck(META_PROTO,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS, (HANDLE)previousMode, mcStatus);
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
typedef struct tag_TFakeAckParams
{
	HANDLE	hEvent;
	HANDLE	hContact;
	LONG		id;
	char	msg[512];
} TFakeAckParams;

/*
static DWORD CALLBACK sttFakeAckSuccess( LPVOID param )
{
	TFakeAckParams *tParam = ( TFakeAckParams* )param;
	WaitForSingleObject( tParam->hEvent, INFINITE );

	Sleep( 100 );
	ProtoBroadcastAck(META_PROTO, tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, ( HANDLE )tParam->id, 0 );

	CloseHandle( tParam->hEvent );
	free(tParam);
	return 0;
}
*/

static DWORD CALLBACK sttFakeAckFail( LPVOID param )
{
	TFakeAckParams *tParam = ( TFakeAckParams* )param;
	WaitForSingleObject( tParam->hEvent, INFINITE );

	Sleep( 100 );
	ProtoBroadcastAck(META_PROTO, tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, ( HANDLE )tParam->id, (WPARAM)tParam->msg );

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
    DBEVENTINFO dbei;
    CCSDATA *ccs = (CCSDATA *) lParam;
	HANDLE hMeta;

	if((hMeta = (HANDLE)DBGetContactSettingDword(ccs->hContact,META_PROTO, "Handle", (DWORD)0)) == (DWORD)0) {
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);		// Can't find the MetaID of the metacontact linked to
	}

	// if subcontact sending, add db event to keep metacontact history correct
	if(options.metahistory && !(ccs->wParam & PREF_METANODB)) {

		// reject "file As Message" messages
		if(strlen((char *)ccs->lParam) > 5 && strncmp((char *)ccs->lParam, "<%fAM", 5) == 0)
			return CallService(MS_PROTO_CHAINSEND, wParam, lParam);	// continue processing

		// reject "data As Message" messages
		if(strlen((char *)ccs->lParam) > 5 && strncmp((char *)ccs->lParam, "<%dAM", 5) == 0)
			return CallService(MS_PROTO_CHAINSEND, wParam, lParam);	// continue processing

		// reject "OTR" messages
		if(strlen((char *)ccs->lParam) > 5 && strncmp((char *)ccs->lParam, "?OTR", 4) == 0)
			return CallService(MS_PROTO_CHAINSEND, wParam, lParam);	// continue processing

		ZeroMemory(&dbei, sizeof(dbei));
		dbei.cbSize = sizeof(dbei);
		dbei.szModule = META_PROTO;
		dbei.flags = DBEF_SENT;
		dbei.timestamp = time(NULL);
		dbei.eventType = EVENTTYPE_MESSAGE;
		if(ccs->wParam & PREF_RTL) dbei.flags |= DBEF_RTL;
		if(ccs->wParam & PREF_UTF) dbei.flags |= DBEF_UTF;
		dbei.cbBlob = (DWORD)strlen((char *)ccs->lParam) + 1;
		if ( ccs->wParam & PREF_UNICODE )
			dbei.cbBlob *= ( sizeof( wchar_t )+1 );
		dbei.pBlob = (PBYTE)ccs->lParam;

		CallService(MS_DB_EVENT_ADD, (WPARAM) hMeta, (LPARAM)&dbei);
	}

	return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
}

INT_PTR Meta_SendNudge(WPARAM wParam,LPARAM lParam)
{
	HANDLE hMeta = (HANDLE)wParam,
		hSubContact = Meta_GetMostOnline(hMeta);

	char servicefunction[ 100 ];
	char *protoName = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hSubContact, 0);
	sprintf(servicefunction, "%s/SendNudge", protoName);

	return CallService(servicefunction, (WPARAM)hSubContact, lParam);

	//return CallService("NUDGE/Send", (WPARAM)hSubContact, lParam);
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
    DBEVENTINFO dbei;
    CCSDATA *ccs = (CCSDATA *) lParam;
	char *proto = 0;
	DWORD default_contact_number;


	if((default_contact_number = DBGetContactSettingDword(ccs->hContact,META_PROTO,"Default",(DWORD)-1)) == (DWORD)-1)
	{
		// This is a simple contact, let through the stack of protocols
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
	}
	else
	{
		char szServiceName[100];  
		HANDLE most_online;
		
		most_online = Meta_GetMostOnline(ccs->hContact);
		//DBEVENTINFO dbei;

		if(!most_online) {
			DWORD dwThreadId;
			HANDLE hEvent;
			TFakeAckParams *tfap;

			// send failure to notify user of reason
			hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

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
		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);
		Meta_SetNick(proto);	// (no matter what was there before)

		// don't bypass filters etc
		strncpy(szServiceName, PSS_MESSAGE, sizeof(szServiceName)); 

		if(ccs->wParam & PREF_UNICODE) { 
			char szTemp[100]; 
			_snprintf(szTemp, sizeof(szTemp), "%s%sW", proto, PSS_MESSAGE); 
			if (ServiceExists(szTemp)) 
				strncpy(szServiceName, PSS_MESSAGE "W", sizeof(szServiceName)); 
		}

		if(options.subhistory && !(ccs->wParam & PREF_METANODB)) {
			// add sent event to subcontact
			ZeroMemory(&dbei, sizeof(dbei));
			dbei.cbSize = sizeof(dbei);
			dbei.szModule = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ccs->hContact, 0);
			dbei.flags = DBEF_SENT;
			dbei.timestamp = time(NULL);
			dbei.eventType = EVENTTYPE_MESSAGE;
			if(ccs->wParam & PREF_RTL) dbei.flags |= DBEF_RTL;
			if(ccs->wParam & PREF_UTF) dbei.flags |= DBEF_UTF;
			dbei.cbBlob = (DWORD)strlen((char *)ccs->lParam) + 1;
			if ( ccs->wParam & PREF_UNICODE )
				dbei.cbBlob *= ( sizeof( wchar_t )+1 );
			dbei.pBlob = (PBYTE)ccs->lParam;

			CallService(MS_DB_EVENT_ADD, (WPARAM) ccs->hContact, (LPARAM)&dbei);
		}

		// prevent send filter from adding another copy of this send event to the db
		ccs->wParam |= PREF_METANODB;

		return CallContactService(ccs->hContact, szServiceName, ccs->wParam, ccs->lParam);
	}
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
    CCSDATA *ccs = (CCSDATA *) lParam;
    PROTORECVEVENT *pre = (PROTORECVEVENT *) ccs->lParam;
	HANDLE hMeta;

	if((hMeta = (HANDLE)DBGetContactSettingDword(ccs->hContact,META_PROTO, "Handle", (DWORD)0)) == (DWORD)0) {
		CallService(MS_PROTO_CHAINRECV, wParam, (LPARAM)ccs);		// Can't find the MetaID of the metacontact linked to
																	// this contact, let through the protocol chain
		return 0;
	}

	if(options.set_default_on_recv) {
		if(options.temp_default && DBGetContactSettingDword(hMeta, META_PROTO, "SavedDefault", (DWORD)-1) == (DWORD)-1)
			DBWriteContactSettingDword(hMeta, META_PROTO, "SavedDefault", DBGetContactSettingDword(hMeta, META_PROTO, "Default", 0));
		DBWriteContactSettingDword(hMeta, META_PROTO, "Default", DBGetContactSettingDword(ccs->hContact, META_PROTO, "ContactNumber", 0));
		NotifyEventHooks(hEventDefaultChanged, (WPARAM)hMeta, (LPARAM)ccs->hContact); // nick set in event handler
	}

	// if meta disabled (now message api) or window open (message api), or using subcontact windows,
	// let through but add db event for metacontact history
	if(!Meta_IsEnabled()
		|| DBGetContactSettingByte(ccs->hContact, META_PROTO, "WindowOpen", 0) == 1
		|| options.subcontact_windows) 
	{

		// add a clist event, so that e.g. there is an icon flashing
		// (only add it when message api available, 'cause then we can remove the event when the message window is opened)
		if(message_window_api_enabled
			&& DBGetContactSettingByte(ccs->hContact, META_PROTO, "WindowOpen", 0) == 0
			&& DBGetContactSettingByte(hMeta, META_PROTO, "WindowOpen", 0) == 0
			&& options.flash_meta_message_icon) 
		{
			CLISTEVENT cle;
			char toolTip[256], *contactName;
			ZeroMemory(&cle, sizeof(cle));

			cle.cbSize = sizeof(cle);
			cle.hContact = hMeta;
			cle.hDbEvent = ccs->hContact;	// use subcontact handle as key - then we can remove all events if the subcontact window is opened
			cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
			cle.pszService = "MetaContacts/CListMessageEvent";
			contactName = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hMeta, 0);
			_snprintf(toolTip, sizeof(toolTip), Translate("Message from %s"), contactName);
			cle.pszTooltip = toolTip;
			CallService(MS_CLIST_ADDEVENT, 0, (LPARAM) & cle);
		}

		if(options.metahistory) {

			BOOL added = FALSE;
			
			// should be able to do this, but some protos mess with the memory
			if(options.use_proto_recv) 
			{
				// use the subcontact's protocol 'recv' service to add the meta's history (AIMOSCAR removes HTML here!) if possible
				char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ccs->hContact, 0);
				if(proto) {
					char service[256];
					HANDLE hSub = ccs->hContact;
					DWORD flags = pre->flags;
					mir_snprintf(service, 256, "%s%s", proto, PSR_MESSAGE);
					ccs->hContact = hMeta;
					pre->flags |= (DBGetContactSettingByte(hMeta, META_PROTO, "WindowOpen", 0) ? 0 : PREF_CREATEREAD);
					if(ServiceExists(service) && !CallService(service, 0, (LPARAM)ccs))
						added = TRUE;
					ccs->hContact = hSub;
					pre->flags = flags;
				}
			}
			
			if(!added) {
				// otherwise add raw db event
				ZeroMemory(&dbei, sizeof(dbei));
				dbei.cbSize = sizeof(dbei);
				dbei.szModule = META_PROTO;
				dbei.timestamp = pre->timestamp;
				dbei.flags = (DBGetContactSettingByte(hMeta, META_PROTO, "WindowOpen", 0) ? 0 : DBEF_READ);
				if(pre->flags & PREF_RTL) dbei.flags |= DBEF_RTL;
				if(pre->flags & PREF_UTF) dbei.flags |= DBEF_UTF;
				dbei.eventType = EVENTTYPE_MESSAGE;
				dbei.cbBlob = (DWORD)strlen(pre->szMessage) + 1;
				if ( pre->flags & PREF_UNICODE ) {
					dbei.cbBlob *= ( sizeof( wchar_t )+1 );
				}
				dbei.pBlob = (PBYTE) pre->szMessage;
	    
				CallService(MS_DB_EVENT_ADD, (WPARAM) hMeta, (LPARAM)&dbei);
			}
		}

		CallService(MS_PROTO_CHAINRECV, wParam, (LPARAM)ccs);
		return 0;
	} // else:

	/*
	// add event to subcontact history (would do it in meta_recvmessage, but here we have the hcontact)
	// should be able to use the method below, except some protos can mess with the memory
	if(options.subhistory) {
		ZeroMemory(&dbei, sizeof(dbei));
		dbei.cbSize = sizeof(dbei);
		dbei.szModule = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ccs->hContact, 0);
		dbei.timestamp = pre->timestamp;
		dbei.flags = (DBGetContactSettingByte(ccs->hContact, META_PROTO, "WindowOpen", 0) ? 0 : DBEF_READ);
		if(pre->flags & PREF_RTL) dbei.flags |= DBEF_RTL;
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.cbBlob = strlen(pre->szMessage) + 1;
		if ( pre->flags & PREF_UNICODE )
			dbei.cbBlob *= ( sizeof( wchar_t )+1 );
		dbei.pBlob = (PBYTE) pre->szMessage;

		CallService(MS_DB_EVENT_ADD, (WPARAM) ccs->hContact, (LPARAM)&dbei);
	}
	*/
	

	{
		HANDLE hSub = ccs->hContact;
		ccs->hContact = hMeta;	// Forward to the associated MetaContact.
		CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)ccs);
		ccs->hContact = hSub;
	}

	if(options.subhistory && !(ccs->wParam & PREF_METANODB)) {
		// allow event pass through and thereby be added to subcontact history
		pre->flags |= (DBGetContactSettingByte(ccs->hContact, META_PROTO, "WindowOpen", 0) ? 0 : PREF_CREATEREAD);
		CallService(MS_PROTO_CHAINRECV, wParam, (LPARAM)ccs);		// pass through as normal
		return 0;
	}

	return 1;	// Stop further processing.
}

/** Receive a message for a MetaContact
*
* @return			0
*/
INT_PTR Meta_RecvMessage(WPARAM wParam, LPARAM lParam)
{
    DBEVENTINFO dbei;
    CCSDATA *ccs = (CCSDATA *) lParam;
    PROTORECVEVENT *pre = (PROTORECVEVENT *) ccs->lParam;
	
	char *proto;

	proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ccs->hContact, 0);

	// contact is not a meta proto contact - just leave it
	if(!proto || strcmp(proto, META_PROTO)) {
		return 0;
	}

	if(options.use_proto_recv) 
	{
		// use the subcontact's protocol to add the db if possible (AIMOSCAR removes HTML here!) 
		HANDLE most_online = Meta_GetMostOnline(ccs->hContact);
		char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);
		if(proto) {
			char service[256];
			mir_snprintf(service, 256, "%s%s", proto, PSR_MESSAGE);
			if (CallService(service, wParam, lParam) != CALLSERVICE_NOTFOUND)
				return 0;
		}
	}


	// otherwise, add event to db directly
    ZeroMemory(&dbei, sizeof(dbei));
    dbei.cbSize = sizeof(dbei);
    dbei.szModule = META_PROTO;
    dbei.timestamp = pre->timestamp;
    dbei.flags = (pre->flags & PREF_CREATEREAD ? DBEF_READ : 0);
	if(pre->flags & PREF_RTL) dbei.flags |= DBEF_RTL;
	if(pre->flags & PREF_UTF) dbei.flags |= DBEF_UTF;
    dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (DWORD)strlen(pre->szMessage) + 1;
	if ( pre->flags & PREF_UNICODE )
		dbei.cbBlob *= ( sizeof( wchar_t )+1 );
    dbei.pBlob = (PBYTE) pre->szMessage;
    
	CallService(MS_DB_EVENT_ADD, (WPARAM) ccs->hContact, (LPARAM)&dbei);

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
	HANDLE hUser;

	if(ack->hContact == 0 || (hUser = (HANDLE)DBGetContactSettingDword(ack->hContact,META_PROTO,"Handle",0)) == 0)
		return 0;	// Can't find the MetaID, let through the protocol chain


	if(!strcmp(ack->szModule, META_PROTO)) {
		return 0; // don't rebroadcast our own acks
	}

	// if it's for something we don't support, ignore
	if(ack->type != ACKTYPE_MESSAGE && ack->type != ACKTYPE_CHAT && ack->type != ACKTYPE_FILE && ack->type != ACKTYPE_AWAYMSG
		&& ack->type != ACKTYPE_AVATAR && ack->type != ACKTYPE_GETINFO)
		
	{
		return 0;
	}

    // change the hContact in the avatar info struct, if it's the avatar we're using - else drop it
	if(ack->type == ACKTYPE_AVATAR) {
		if(ack->result == ACKRESULT_SUCCESS || ack->result == ACKRESULT_FAILED || ack->result == ACKRESULT_STATUS) {
			HANDLE most_online;
			DBVARIANT dbv;

			// change avatar if the most online supporting avatars changes, or if we don't have one
			most_online = Meta_GetMostOnlineSupporting(hUser, PFLAGNUM_4, PF4_AVATARS);
			//if(AI.hContact == 0 || AI.hContact != most_online) {
			if(ack->hContact == 0 || ack->hContact != most_online) {
				return 0;
			}

			//if(!DBGetContactSetting(AI.hContact, "ContactPhoto", "File", &dbv)) {
			if(!DBGetContactSetting(ack->hContact, "ContactPhoto", "File", &dbv)) {
				DBWriteContactSettingString(hUser, "ContactPhoto", "File", dbv.pszVal);
				DBFreeVariant(&dbv);
			}

			if(ack->hProcess) {
				PROTO_AVATAR_INFORMATION AI;
				memcpy(&AI, (PROTO_AVATAR_INFORMATION *)ack->hProcess, sizeof(PROTO_AVATAR_INFORMATION));
				if(AI.hContact)
					AI.hContact = hUser;
			
				return ProtoBroadcastAck(META_PROTO,hUser,ack->type,ack->result, (HANDLE)&AI, ack->lParam);
			} else
				return ProtoBroadcastAck(META_PROTO,hUser,ack->type,ack->result, 0, ack->lParam);
		}
	}

	return ProtoBroadcastAck(META_PROTO,hUser,ack->type,ack->result,ack->hProcess,ack->lParam);
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
	char buffer[512], buffer2[512];
	int contact_number;
	HANDLE hMeta, most_online;


	// hide metacontacts when groups disabled
	if(wParam == 0
		&& ((strcmp(dcws->szModule, "CList") == 0 && strcmp(dcws->szSetting, "UseGroups") == 0)
			|| (strcmp(dcws->szModule, META_PROTO) == 0 && strcmp(dcws->szSetting, "Enabled") == 0)))
	{
		sttHideContacts(!Meta_IsEnabled());
		return 0;
	}

	if(wParam == 0
		&& strcmp(dcws->szModule, "Import") == 0 && strcmp(dcws->szSetting, "Completed") == 0)
	{
		// import process has just been run...call startup routines...
		Meta_SetHandles();
		{
			HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
			int meta_id;
			while ( hContact != NULL ) {
				if((meta_id = DBGetContactSettingDword(hContact,META_PROTO,META_ID,(DWORD)-1))!=(DWORD)-1) {
					Meta_CopyData(hContact);
				}
				hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
			}
		}

		Meta_HideLinkedContacts();
		Meta_SuppressStatus(options.suppress_status);
	}

	if(wParam == 0
		&& strcmp(dcws->szModule, "CListGroups") == 0 && dcws->value.type != DBVT_DELETED && strcmp(dcws->value.pszVal, META_HIDDEN_GROUP) == 0)
	{
		// someone is creating our hidden group!!

	}

	// from here on, we're just interested in contact settings
	if(wParam == 0) return 0;

	if((hMeta=(HANDLE)DBGetContactSettingDword((HANDLE)wParam,META_PROTO,"Handle",0))!=0
		&& CallService(MS_DB_CONTACT_IS, (WPARAM)hMeta, 0)) // just to be safe
	
	{	// This contact is attached to a MetaContact.

		contact_number = Meta_GetContactNumber((HANDLE)wParam);
		if(contact_number == -1) return 0; // exit - db corruption

		if(!meta_group_hack_disabled && !strcmp(dcws->szModule, "CList") && !strcmp(dcws->szSetting, "Group") && 
            Meta_IsEnabled() && DBGetContactSettingByte((HANDLE)wParam, META_PROTO, "Hidden", 0) == 0 && !Miranda_Terminated()) {
			if((dcws->value.type == DBVT_ASCIIZ || dcws->value.type == DBVT_UTF8) && !Meta_IsHiddenGroup(dcws->value.pszVal)) {
				// subcontact group reassigned - copy to saved group
				MyDBWriteContactSetting((HANDLE)wParam, META_PROTO, "OldCListGroup", &dcws->value);
				DBWriteContactSettingString((HANDLE)wParam, "CList", "Group", META_HIDDEN_GROUP);
			} else if(dcws->value.type == DBVT_DELETED) {
				DBDeleteContactSetting((HANDLE)wParam, META_PROTO, "OldCListGroup");
				DBWriteContactSettingString((HANDLE)wParam, "CList", "Group", META_HIDDEN_GROUP);
			}
		} else		

		// copy IP 
		if(!strcmp(dcws->szSetting, "IP")) {
			if(dcws->value.type == DBVT_DWORD)
				DBWriteContactSettingDword(hMeta, META_PROTO, "IP", dcws->value.dVal);
			else
				DBDeleteContactSetting(hMeta, META_PROTO, "IP");
		} else

		// copy RealIP
		if(!strcmp(dcws->szSetting, "RealIP")) {
			if(dcws->value.type == DBVT_DWORD)
				DBWriteContactSettingDword(hMeta, META_PROTO, "RealIP", dcws->value.dVal);
			else
				DBDeleteContactSetting(hMeta, META_PROTO, "RealIP");

		} else
		// copy ListeningTo
		if(!strcmp(dcws->szSetting, "ListeningTo")) {
			switch(dcws->value.type) {
				case DBVT_ASCIIZ:
					DBWriteContactSettingString(hMeta, META_PROTO, "ListeningTo", dcws->value.pszVal);
					break;
				case DBVT_UTF8:
					DBWriteContactSettingStringUtf(hMeta, META_PROTO, "ListeningTo", dcws->value.pszVal);
					break;
				case DBVT_WCHAR:
					DBWriteContactSettingWString(hMeta, META_PROTO, "ListeningTo", dcws->value.pwszVal);
					break;
				case DBVT_DELETED:
					DBDeleteContactSetting(hMeta, META_PROTO, "ListeningTo");
					break;
			}
		} else

		if(!strcmp(dcws->szSetting, "Nick") && !dcws->value.type == DBVT_DELETED) {
			DBVARIANT dbv;
			HANDLE most_online;

			// subcontact nick has changed - update metacontact	
			strcpy(buffer, "Nick");
			strcat(buffer, _itoa(contact_number, buffer2, 10));
			MyDBWriteContactSetting(hMeta, META_PROTO, buffer, &dcws->value);

			if(MyDBGetContactSetting((HANDLE)wParam, "CList", "MyHandle", &dbv)) {
				strcpy(buffer, "CListName");
				strcat(buffer, _itoa(contact_number, buffer2, 10));
				MyDBWriteContactSetting(hMeta, META_PROTO, buffer, &dcws->value);
			} else {
				DBFreeVariant(&dbv);
			}

			// copy nick to metacontact, if it's the most online
			most_online = Meta_GetMostOnline(hMeta);
			Meta_CopyContactNick(hMeta, most_online);

			return 0;
		} else

		if(!strcmp(dcws->szSetting, "IdleTS")) {
			if(dcws->value.type == DBVT_DWORD)
				DBWriteContactSettingDword(hMeta, META_PROTO, "IdleTS", dcws->value.dVal);
			else if(dcws->value.type == DBVT_DELETED)
				DBWriteContactSettingDword(hMeta, META_PROTO, "IdleTS", 0);
		} else

		if(!strcmp(dcws->szSetting, "LogonTS")) {
			if(dcws->value.type == DBVT_DWORD)
				DBWriteContactSettingDword(hMeta, META_PROTO, "LogonTS", dcws->value.dVal);
			else if(dcws->value.type == DBVT_DELETED)
				DBWriteContactSettingDword(hMeta, META_PROTO, "LogonTS", 0);
		} else

		if(!strcmp(dcws->szModule, "CList") && !strcmp(dcws->szSetting, "MyHandle")) {
			HANDLE most_online;

			if(dcws->value.type == DBVT_DELETED) {
				DBVARIANT dbv;

				char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
				strcpy(buffer, "CListName");
				strcat(buffer, _itoa(contact_number, buffer2, 10));
				if(proto && !MyDBGetContactSetting((HANDLE)wParam, proto, "Nick", &dbv)) {
					MyDBWriteContactSetting(hMeta, META_PROTO, buffer, &dbv);
					DBFreeVariant(&dbv);
				} else {
					DBDeleteContactSetting(hMeta, META_PROTO, buffer);
				}
			} else {
				// subcontact clist displayname has changed - update metacontact	
				strcpy(buffer, "CListName");
				strcat(buffer, _itoa(contact_number, buffer2, 10));
				
				MyDBWriteContactSetting(hMeta, META_PROTO, buffer, &dcws->value);
			}

			// copy nick to metacontact, if it's the most online
			most_online = Meta_GetMostOnline(hMeta);
			Meta_CopyContactNick(hMeta, most_online);

			return 0;
		} else

		if(!strcmp(dcws->szSetting, "Status") && !dcws->value.type == DBVT_DELETED) {	
			// subcontact changing status

			// update subcontact status setting
			strcpy(buffer, "Status");
			strcat(buffer, _itoa(contact_number, buffer2, 10));
			DBWriteContactSettingWord(hMeta, META_PROTO, buffer, dcws->value.wVal);
			strcpy(buffer, "StatusString");
			strcat(buffer, _itoa(contact_number, buffer2, 10));
			Meta_GetStatusString(dcws->value.wVal, buffer2, 512);
			DBWriteContactSettingString(hMeta, META_PROTO, buffer, buffer2);

			// if the contact was forced, unforce it (which updates status)
			if((HANDLE)DBGetContactSettingDword(hMeta, META_PROTO, "ForceSend", 0) == (HANDLE)wParam) {
				MetaAPI_UnforceSendContact((WPARAM)hMeta, 0);
			} else {
				// set status to that of most online contact
				most_online = Meta_GetMostOnline(hMeta);
				Meta_CopyContactNick(hMeta, most_online);

				Meta_FixStatus(hMeta);

				Meta_CopyData(hMeta);
			}

			// most online contact with avatar support might have changed - update avatar
			most_online = Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_4, PF4_AVATARS);
			if(most_online) {
				PROTO_AVATAR_INFORMATION AI;

				AI.cbSize = sizeof(AI);
				AI.hContact = hMeta;
				AI.format = PA_FORMAT_UNKNOWN;
				strcpy(AI.filename, "X");

				if((int)CallProtoService(META_PROTO, PS_GETAVATARINFO, 0, (LPARAM)&AI) == GAIR_SUCCESS)
					DBWriteContactSettingString(hMeta, "ContactPhoto", "File",AI.filename);
			}
		} else 

		if(strcmp(dcws->szSetting, "XStatusId") == 0 || strcmp(dcws->szSetting, "XStatusMsg") == 0 || strcmp(dcws->szSetting, "XStatusName") == 0 || strcmp(dcws->szSetting, "StatusMsg") == 0) {	
			Meta_CopyData(hMeta);
		} else
			
		if(strcmp(dcws->szSetting, "MirVer") == 0) {	
			Meta_CopyData(hMeta);
		} else

		if(!meta_group_hack_disabled && !strcmp(dcws->szModule, "CList") && !strcmp(dcws->szSetting, "Hidden")) {
			if((dcws->value.type == DBVT_DELETED || DBGetContactSettingByte((HANDLE)wParam, "CList", "Hidden", 0) == 0)
				&& DBGetContactSettingByte((HANDLE)wParam, META_PROTO, "Hidden", 0) == 1)
			{
				// a subcontact we hid (e.g. jabber) has been unhidden - hide it again :(
				DBWriteContactSettingByte((HANDLE)wParam, "CList", "Hidden", 1);
			}
		}
	}

	return 0;
}

int Meta_ContactDeleted(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta;

	// is a subcontact - update meta contact
	hMeta = (HANDLE)DBGetContactSettingDword((HANDLE)wParam, META_PROTO, "Handle", 0);
	if(hMeta) {
		Meta_RemoveContactNumber(hMeta, DBGetContactSettingDword((HANDLE)wParam, META_PROTO, "ContactNumber", -1));
		NotifyEventHooks(hSubcontactsChanged, (WPARAM)hMeta, 0);
		return 0;
	} else {
		// not a subcontact - is it a metacontact?
		int num_contacts = DBGetContactSettingDword((HANDLE)wParam, META_PROTO, "NumContacts", 0);
		int i;
		HANDLE hContact;

		if(num_contacts) NotifyEventHooks(hSubcontactsChanged, (WPARAM)wParam, 0);
	
		// remove & restore all subcontacts
		for(i = 0; i < num_contacts; i++) {
			hContact = Meta_GetContactHandle((HANDLE)wParam, i);

			if(hContact && (HANDLE)DBGetContactSettingDword(hContact, META_PROTO, "Handle", 0) == (HANDLE)wParam) {
				if(DBGetContactSettingByte(hContact, META_PROTO, "IsSubcontact", 0) == 1) 
					DBDeleteContactSetting(hContact,META_PROTO,"IsSubcontact");
				DBDeleteContactSetting(hContact,META_PROTO,META_LINK);
				DBDeleteContactSetting(hContact,META_PROTO,"Handle");
				DBDeleteContactSetting(hContact,META_PROTO,"ContactNumber");
				Meta_RestoreGroup(hContact);
				DBDeleteContactSetting(hContact,META_PROTO,"OldCListGroup");
				
				CallService(MS_PROTO_REMOVEFROMCONTACT, (WPARAM)hContact, (LPARAM)META_FILTER);
				// stop ignoring, if we were
				if(options.suppress_status)
					CallService(MS_IGNORE_UNIGNORE, (WPARAM)hContact, (WPARAM)IGNOREEVENT_USERONLINE);
			}
		}
		return 0;
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
	char *proto;
	char buff[512];

	if(DBGetContactSettingDword((HANDLE)wParam,META_PROTO,META_ID,(DWORD)-1) == (DWORD)-1)
	{
		// This is a simple contact, let through the stack of protocols
		return 0;
	}
	else
	{
		// forward to sending protocol, if supported

		HANDLE most_online = Meta_GetMostOnline((HANDLE)wParam);
		Meta_CopyContactNick((HANDLE)wParam, most_online);

		if(!most_online) return 0;

		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);
		if(proto) {
			strncpy(buff, proto, 512);
			strncpy(buff + strlen(proto), PSS_USERISTYPING, 512 - strlen(proto));

			if(ServiceExists(buff)) {
				CallService(buff, (WPARAM)most_online, (LPARAM)lParam);
			}
		}
	}

	return 0;
}

/** Call when we want to receive a user is typing message
*
* @param wParam \c HANDLE to the contact that is typing or not
* @param lParam either PROTOTYPE_SELFTYPING_ON or PROTOTYPE_SELFTYPING_OFF
*/
int Meta_ContactIsTyping(WPARAM wParam, LPARAM lParam)
{
	HANDLE hMeta;

	if((hMeta = (HANDLE)DBGetContactSettingDword((HANDLE)wParam,META_PROTO,"Handle",(DWORD)0)) != 0
			// check metacontacts enabled
			&& Meta_IsEnabled()
		)
	{	// This contact is attached to a MetaContact.
		if(!options.subcontact_windows) { // we don't want clicking on the clist notification icon to open the metacontact message window

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
	DWORD default_contact_number = DBGetContactSettingDword((HANDLE)lParam, META_PROTO, "Default", (DWORD)-1);
	
	if(default_contact_number == -1) // not a meta contact
		return 0;

	CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)Meta_GetContactHandle((HANDLE)lParam, default_contact_number), 0);

	return 1;
}

// handle message window api ver 0.0.0.1+ events - record window open/close status for subcontacts, so we know whether to 
// let received messages through and add db history to metacontact, or vice versa
int Meta_MessageWindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *mwed = (MessageWindowEventData *)lParam;
	HANDLE hMeta = 0;

	message_window_api_enabled = TRUE;

	if((hMeta = (HANDLE)DBGetContactSettingDword(mwed->hContact, META_PROTO, "Handle", 0)) != 0
		|| DBGetContactSettingDword(mwed->hContact, META_PROTO, META_ID, (DWORD)-1) != (DWORD)-1)
	{
		// contact is subcontact of metacontact, or an actual metacontact - record whether window is open or closed
		if(mwed->uType == MSG_WINDOW_EVT_OPEN || mwed->uType == MSG_WINDOW_EVT_OPENING) {
			DBWriteContactSettingByte(mwed->hContact, META_PROTO, "WindowOpen", 1);

			if(hMeta) { // subcontact window opened - remove clist events we added for metacontact
				while(!CallService(MS_CLIST_REMOVEEVENT, (WPARAM)hMeta, (LPARAM)mwed->hContact));
			}
		} else if(mwed->uType == MSG_WINDOW_EVT_CLOSE || mwed->uType == MSG_WINDOW_EVT_CLOSING) {
			DBWriteContactSettingByte(mwed->hContact, META_PROTO, "WindowOpen", 0);
			if(!hMeta) { // hMeta is 0 for metacontact (sorry)
				DWORD saved_def;

				MetaAPI_UnforceSendContact((WPARAM)mwed->hContact, 0);

				// restore saved default contact
				if(options.set_default_on_recv) {
					saved_def = DBGetContactSettingDword(mwed->hContact, META_PROTO, "SavedDefault", -1);
					if(options.temp_default && saved_def != (DWORD)-1) {
						DBWriteContactSettingDword(mwed->hContact, META_PROTO, "Default", saved_def);
						DBWriteContactSettingDword(mwed->hContact, META_PROTO, "SavedDefault", (DWORD)-1);
						NotifyEventHooks(hEventDefaultChanged, (WPARAM)mwed->hContact, (LPARAM)Meta_GetContactHandle(hMeta, saved_def)); // nick set in event handler
					}
				}
			}
		}

	}

	return 0;
}
/*
int Meta_LoadIcons(WPARAM wParam, LPARAM lParam) {
	PROTOCOLDESCRIPTOR **protos;

	//MessageBox(0, "LoadIcons", "Event", MB_OK);	

	if(ServiceExists(MS_CLIST_EXTRA_ADD_ICON)) {
		int index = 0, i;

		CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&proto_count,(LPARAM)&protos);
		for(i = 0; i < proto_count && i < MAX_PROTOCOLS; i++) {
			if(protos[i]->type!=PROTOTYPE_PROTOCOL || CallProtoService(protos[i]->szName,PS_GETCAPS,PFLAGNUM_2,0)==0) 
				continue;

			strncpy(proto_names + (index * 128), protos[i]->szName, 128);
			hProtoIcons[index * 2] = LoadSkinnedProtoIcon(protos[i]->szName,ID_STATUS_ONLINE);
			hProtoIcons[index * 2 + 1] = LoadSkinnedProtoIcon(protos[i]->szName,ID_STATUS_OFFLINE);
			hExtraImage[index * 2] = 0; 
			hExtraImage[index * 2 + 1] = 0; 
			
			//sprintf(buff, "Added icon (hIcon = %d, hImage = %d) for protocol %s.", hProtoIcons[index], hExtraImage[index], protos[i]->szName);
			//MessageBox(0, buff, "Added Extra Icon", MB_OK);

			index++;
		}
		proto_count = index;

		//Meta_CListMW_ExtraIconsRebuild(0, 0);

	}


	return 0;
}

int Meta_CListMW_ExtraIconsRebuild(WPARAM wParam, LPARAM lParam) {
	int i;

	//MessageBox(0, "IconsRebuild", "Event", MB_OK);
	Meta_LoadIcons(0, 0);

	if(ServiceExists(MS_CLIST_EXTRA_ADD_ICON)) {
		for(i = 0; i < proto_count; i++) {
			hExtraImage[i * 2] = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)hProtoIcons[i * 2], 0);		
			hExtraImage[i * 2 + 1] = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)hProtoIcons[i * 2 + 1], 0);		
		}
	}

	return 0;
}

int Meta_CListMW_ExtraIconsApply(WPARAM wParam, LPARAM lParam) {

	//MessageBox(0, "IconsApply", "Event", MB_OK);

	if(DBGetContactSettingDword((HANDLE)wParam, META_PROTO, META_ID, (DWORD)-1) != (DWORD)-1) {
		if(ServiceExists(MS_CLIST_EXTRA_SET_ICON)) {
			IconExtraColumn iec;
			HANDLE most_online_im = Meta_GetMostOnline((HANDLE)wParam);
			int i;

			if(most_online_im) {
				char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online_im, 0);	
				if(proto) {
					WORD status = DBGetContactSettingWord(most_online_im, proto, "Status", ID_STATUS_OFFLINE);
					iec.cbSize = sizeof(iec);
					for(i = 0; i < proto_count; i++) {
						if(!strcmp((proto_names + i * 128), proto)) {
							if(hExtraImage[i * 2 + (status == ID_STATUS_OFFLINE ? 1 : 0)]) {
								iec.hImage = hExtraImage[i * 2 + (status == ID_STATUS_OFFLINE ? 1 : 0)];
								iec.ColumnType = EXTRA_ICON_ADV2;
								CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM)wParam, (LPARAM)&iec);
								iec.ColumnType = EXTRA_ICON_PROTO;
								CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM)wParam, (LPARAM)&iec);
							}
							break;
						}
					}
				}
			}
		}
	}
	return 0;
}
*/
int Meta_ClistDoubleClicked(WPARAM wParam, LPARAM lParam) {

	if(DBGetContactSettingDword((HANDLE)wParam,META_PROTO,"Default",(WORD)-1) == (WORD)-1)
	{
		// This is a simple contact
		return 0;
	}
	else
	{
		// -1 indicates  no specific capability but respect 'ForceDefault'
		HANDLE most_online = Meta_GetMostOnlineSupporting((HANDLE)wParam, PFLAGNUM_1, -1);
        //DBEVENTINFO dbei;
		char *proto;
		char buffer[512];
		int caps;

		if(!most_online)
			return 0;

		if(options.subcontact_windows) {
			if(lParam) {
				// contact from incoming message in lParam via (at this point) clist message event
				CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)lParam, 0);
			} else {
				// simulate double click on most_online contact and stop event processing
				CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)most_online, 0);
			}
			return 1;
		} else {
			proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);

			if(proto) {
				strcpy(buffer, proto);
				strcat(buffer, PS_GETCAPS);

				// get the contacts messaging capabilities
				caps = CallService(buffer, (WPARAM)PFLAGNUM_1, 0);

				if((caps & PF1_IMSEND) || (caps & PF1_CHAT) || (proto && strcmp(proto, "IRC") == 0))
					// let event process normally
					return 0;
				else {
					// simulate double click on most_online contact and stop event processing
					CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)most_online, 0);
					return 1;
				}
			} else
				return 0;
		}
	}

	return 0;
}

INT_PTR Meta_ClistMessageEventClicked(WPARAM wParam, LPARAM lParam) {

	HANDLE hContact = ((CLISTEVENT *)lParam)->hContact;

	// hdbevent contains the id of the subcontact
	return Meta_ClistDoubleClicked((WPARAM)hContact, (LPARAM)((CLISTEVENT *)lParam)->hDbEvent);
}


int NudgeRecieved(WPARAM wParam, LPARAM lParam) {
	/*
	// already being forwarded by someone
	HANDLE hMeta = (HANDLE)DBGetContactSettingDword((HANDLE)wParam,META_PROTO, "Handle", (DWORD)0);
	if(hMeta)
		NotifyEventHooks(hEventNudge, (WPARAM)hMeta, 0);
	*/
	return 0;
}

/** Called when all the plugin are loaded into Miranda.
*
* Initializes the 4 menus present in the context-menu
* and the initial value of nextMetaID
*/
int Meta_ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM menu = {0};
	char buffer[512], buffer2[512], buffer3[512];
	int i;

	if(ServiceExists(MS_MSG_GETWINDOWAPI)) {
		message_window_api_enabled = TRUE;
	}

	if(ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = (int)strlen((char *)update.pbVersion);
		update.szBetaChangelogURL = "https://server.scottellis.com.au/wsvn/mim_plugs/metacontacts/?op=log&rev=0&sc=0&isdir=1";

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/MetaContacts.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_MetaContacts.html";
		update.pbBetaVersionPrefix = (BYTE *)"MetaContacts Plugin, version ";
		
		update.cpbBetaVersionPrefix = (int)strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}


	// disable group hack for older nicer versions without the fix
	if(ServiceExists(MS_CLUI_GETVERSION)) {
		char *version = (char *)CallService(MS_CLUI_GETVERSION, 0, 0);
		if(version && strlen(version) >= strlen("CList Nicer+") && strncmp(version, "CList Nicer+", strlen("CList Nicer+")) == 0)
			meta_group_hack_disabled = TRUE;
	}

	// for database editor++ ver 3+
	if(ServiceExists("DBEditorpp/RegisterSingleModule"))
		CallService("DBEditorpp/RegisterSingleModule",(WPARAM)META_PROTO,0);


	hHooks[11] = (HANDLE)HookEvent(ME_CLIST_PREBUILDCONTACTMENU, Meta_ModifyMenu);
	hHooks[12] = (HANDLE)HookEvent(ME_CLIST_DOUBLECLICKED, Meta_ClistDoubleClicked );
	//hHooks[13] = (HANDLE)HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, Meta_CListMW_ExtraIconsRebuild);
	//hHooks[14] = (HANDLE)HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, Meta_CListMW_ExtraIconsApply);

	// icons are erased on this event...
	// (BUT, the me_clist_extra_list_rebuild is send FIRST...so, we ignore this one...)
	hHooks[15] = 0;//(HANDLE)HookEvent(ME_SKIN_ICONSCHANGED, Meta_LoadIcons);

	menu.cbSize=sizeof(menu);
	menu.flags = CMIM_ALL;

	// main menu item
	menu.pszName = "Toggle MetaContacts Off";
	menu.pszService = "MetaContacts/OnOff";
	menu.position = 500010000;
	hMenuOnOff = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&menu);

	// contact menu items
	menu.position = -200010;
	menu.pszName = "Convert to MetaContact";
	menu.pszService = "MetaContacts/Convert";
	hMenuConvert = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);
	menu.position = -200009;
	menu.pszName = "Add to existing MetaContact...";
	menu.pszService = "MetaContacts/AddTo";
	hMenuAdd = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);

	menu.position = -200010;
	menu.pszName = "Edit MetaContact...";
	menu.pszService = "MetaContacts/Edit";
	hMenuEdit = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);
	menu.position = -200009;
	menu.pszName = "Set as MetaContact default";
	menu.pszService = "MetaContacts/Default";
	hMenuDefault = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);
	menu.position = -200008;
	menu.pszName = "Delete MetaContact";
	menu.pszService = "MetaContacts/Delete";
	hMenuDelete = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);
	//menu.pszName = "Force Default";
	//menu.pszService = "MetaContacts/ForceDefault";
	//hMenuForceDefault = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);

	menu.flags |= CMIF_HIDDEN;
	menu.pszContactOwner = META_PROTO;

	menu.position = -99000;
	for(i = 0; i < MAX_CONTACTS; i++) {
		menu.position--;
		strcpy(buffer3, (char *)Translate("Context"));
		strcat(buffer3, _itoa(i, buffer2, 10));
		menu.pszName = buffer3;

		strcpy(buffer, "MetaContacts/MenuFunc");
		strcat(buffer, _itoa(i, buffer2, 10));
		menu.pszService= buffer; 
		
		hMenuContact[i] = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);
	}

	nextMetaID = DBGetContactSettingDword(NULL,META_PROTO,"NextMetaID",(DWORD)0);

	// attemp to subsume userinfo...(returning 1 does not prevent dialog - so disabled)
	//hHooks[] = (HANDLE)HookEvent(ME_USERINFO_INITIALISE, Meta_UserInfo);

	// loop and copy data from subcontacts
	if(options.copydata) {
		HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
		int meta_id;
		while ( hContact != NULL ) {
			if((meta_id = DBGetContactSettingDword(hContact,META_PROTO,META_ID,(DWORD)-1))!=(DWORD)-1) {
				Meta_CopyData(hContact);
			}
			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
		}
	}

	Meta_HideLinkedContacts();
	
	InitIcons();

	if(!Meta_IsEnabled())
	{
		// modify main menu item
		menu.flags = CMIM_NAME;
		menu.pszName = "Toggle MetaContacts On";
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuOnOff, (LPARAM)&menu);

		Meta_HideMetaContacts(TRUE);
	} else {
		Meta_SuppressStatus(options.suppress_status);
	}

	// hook srmm window close/open events - message api ver 0.0.0.1+
	hHooks[16] = (HANDLE)HookEvent(ME_MSG_WINDOWEVENT, Meta_MessageWindowEvent);
	if(hHooks[16]) // message api available
		message_window_api_enabled = TRUE;

	// hook protocol nudge events to forward to subcontacts
	{
		int i, numberOfProtocols,ret;
		char str[MAXMODULELABELLENGTH + 10];
		HANDLE hNudgeEvent = NULL;
		PROTOCOLDESCRIPTOR ** ppProtocolDescriptors;
		ret = CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM) &numberOfProtocols,(LPARAM)&ppProtocolDescriptors);
		if(ret == 0)
		{
			for(i = 0; i < numberOfProtocols ; i++)
			{
				if(ppProtocolDescriptors[i]->type == PROTOTYPE_PROTOCOL)
				{	
					if(strcmp(ppProtocolDescriptors[i]->szName, META_PROTO)) {
						sprintf(str,"%s/Nudge",ppProtocolDescriptors[i]->szName);
						hNudgeEvent = HookEvent(str, NudgeRecieved);
						if(hNudgeEvent != NULL) {
							++iNudgeProtos;
							hNudgeEvents = realloc(hNudgeEvents, sizeof(HANDLE) * iNudgeProtos);
							hNudgeEvents[iNudgeProtos - 1] = hNudgeEvent;
						}						
					}
				}
			}
			
		}
	}
	return 0;
}

static VOID CALLBACK sttMenuThread( PVOID param )
{
	HMENU hMenu;
	TPMPARAMS tpmp;
	BOOL menuRet;

	hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)param, 0);

	ZeroMemory(&tpmp, sizeof(tpmp));
	tpmp.cbSize = sizeof(tpmp);

	menuRet = TrackPopupMenuEx(hMenu, TPM_RETURNCMD, menuMousePoint.x, menuMousePoint.y, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), &tpmp);

	CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(menuRet), MPCF_CONTACTMENU), (LPARAM)param);

	DestroyMenu(hMenu);		
}

INT_PTR Meta_ContactMenuFunc(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact;
	hContact = Meta_GetContactHandle((HANDLE)wParam, (int)lParam);

	if(options.menu_function == FT_MSG) {
		// open message window if protocol supports message sending or chat, else simulate double click

		int caps;
		char *proto;
		char buffer[512];
	
		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

		if(proto) {
			strcpy(buffer, proto);
			strcat(buffer, PS_GETCAPS);

			caps = CallService(buffer, (WPARAM)PFLAGNUM_1, 0);

			if((caps & PF1_IMSEND) || (caps & PF1_CHAT) || (proto && strcmp(proto, "IRC") == 0)) {
				// set default contact for sending/status and open message window
				DBWriteContactSettingDword((HANDLE)wParam, META_PROTO, "Default", (DWORD)(int)lParam);
				NotifyEventHooks(hEventDefaultChanged, wParam, (LPARAM)hContact);
				CallService(MS_MSG_SENDMESSAGE, wParam, 0);
			} else
				// protocol does not support messaging - simulate double click
				CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)hContact, 0);
		} else 
			// protocol does not support messaging - simulate double click
			CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)hContact, 0);

	} else if(options.menu_function == FT_MENU) {
		// show contact's context menu
		CallFunctionAsync(sttMenuThread, hContact);
	} else if(options.menu_function == FT_INFO) {
		// show user info for subcontact
		CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)hContact, 0);
	}

	return 0;
}

////////////////////
// file transfer support - mostly not required, since subcontacts do the receiving
////////////////////
/*
INT_PTR Meta_FileResume(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
    CCSDATA *ccs = (CCSDATA *) lParam;
	char *proto = 0;

	if(DBGetContactSetting(ccs->hContact,META_PROTO,"Default",&dbv))
	{
		// This is a simple contact
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return 1;
	}
	else
	{
		HANDLE most_online = Meta_GetMostOnlineSupporting(ccs->hContact, PFLAGNUM_1, PF1_FILERESUME);
        //DBEVENTINFO dbei;
		char szServiceName[100];  

		DBFreeVariant(&dbv);

		if(!most_online)
			return 0;

		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);

		ccs->hContact = most_online;
		Meta_SetNick(proto);

		_snprintf(szServiceName, sizeof(szServiceName), "%s%s", proto, PS_FILERESUME); 
		if (ServiceExists(szServiceName)) {
			strncpy(szServiceName, PS_FILERESUME, sizeof(szServiceName)); 
			return (int)(CallContactService(ccs->hContact, szServiceName, ccs->wParam, ccs->lParam));
		}
	}
	return 1; // fail
}

INT_PTR Meta_FileAllow(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
    CCSDATA *ccs = (CCSDATA *) lParam;
	char *proto = 0;

	if(DBGetContactSetting(ccs->hContact,META_PROTO,"Default",&dbv))
	{
		// This is a simple contact
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return 0;
	}
	else
	{
		HANDLE most_online = Meta_GetMostOnlineSupporting(ccs->hContact, PFLAGNUM_1, PF1_FILE);
		char szServiceName[100];  

		DBFreeVariant(&dbv);

		if(!most_online)
			return 0;

		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);

		ccs->hContact = most_online;
		Meta_SetNick(proto);

		_snprintf(szServiceName, sizeof(szServiceName), "%s%s", proto, PSS_FILEALLOW); 
		if (ServiceExists(szServiceName)) {
			strncpy(szServiceName, PSS_FILEALLOW, sizeof(szServiceName)); 
			return (int)(CallContactService(ccs->hContact, szServiceName, ccs->wParam, ccs->lParam));
		}
	}
	return 0; // fail
}

INT_PTR Meta_FileDeny(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
    CCSDATA *ccs = (CCSDATA *) lParam;
	char *proto = 0;

	if(DBGetContactSetting(ccs->hContact,META_PROTO,"Default",&dbv))
	{
		// This is a simple contact
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return 1;
	}
	else
	{
		HANDLE most_online = Meta_GetMostOnlineSupporting(ccs->hContact, PFLAGNUM_1, PF1_FILE);
        //DBEVENTINFO dbei;
		char szServiceName[100];  

		DBFreeVariant(&dbv);

		if(!most_online)
			return 1;

		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);

		ccs->hContact = most_online;
		Meta_SetNick(proto);

		_snprintf(szServiceName, sizeof(szServiceName), "%s%s", proto, PSS_FILEDENY); 
		if (ServiceExists(szServiceName)) {
			strncpy(szServiceName, PSS_FILEDENY, sizeof(szServiceName)); 
			return (int)(CallContactService(ccs->hContact, szServiceName, ccs->wParam, ccs->lParam));
		}
	}
	return 1; // fail
}

INT_PTR Meta_FileRecv(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
    CCSDATA *ccs = (CCSDATA *) lParam;
	char *proto = 0;

	if(DBGetContactSetting(ccs->hContact,META_PROTO,"Default",&dbv))
	{
		// This is a simple contact
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return 0;
	}
	else
	{
		HANDLE most_online = Meta_GetMostOnlineSupporting(ccs->hContact, PFLAGNUM_1, PF1_FILE);
        //DBEVENTINFO dbei;
		char szServiceName[100];  

		DBFreeVariant(&dbv);

		if(!most_online)
			return 0;

		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);

		ccs->hContact = most_online;
		Meta_SetNick(proto);

		_snprintf(szServiceName, sizeof(szServiceName), "%s%s", proto, PSR_FILE); 
		if (ServiceExists(szServiceName)) {
			strncpy(szServiceName, PSR_FILE, sizeof(szServiceName)); 
			return (int)(CallContactService(ccs->hContact, szServiceName, ccs->wParam, ccs->lParam));
		}
	}

	return 0;
}

int Meta_FileCancel(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
    CCSDATA *ccs = (CCSDATA *) lParam;
	char *proto = 0;

	if(DBGetContactSetting(ccs->hContact,META_PROTO,"Default",&dbv))
	{
		// This is a simple contact
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return 0;
	}
	else
	{
		HANDLE most_online = Meta_GetMostOnlineSupporting(ccs->hContact, PFLAGNUM_1, PF1_FILE);
        //DBEVENTINFO dbei;
		char szServiceName[100];  

		DBFreeVariant(&dbv);

		if(!most_online)
			return 0;

		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);

		ccs->hContact = most_online;
		Meta_SetNick(proto);

		_snprintf(szServiceName, sizeof(szServiceName), "%s%s", proto, PSS_FILECANCEL); 
		if (ServiceExists(szServiceName)) {
			strncpy(szServiceName, PSS_FILECANCEL, sizeof(szServiceName)); 
			return (int)(CallContactService(ccs->hContact, szServiceName, ccs->wParam, ccs->lParam));
		}
	}
	return 0;
}
*/

INT_PTR Meta_FileSend(WPARAM wParam, LPARAM lParam)
{
    CCSDATA *ccs = (CCSDATA *) lParam;
	char *proto = 0;
	DWORD default_contact_number;

	if((default_contact_number = DBGetContactSettingDword(ccs->hContact,META_PROTO,"Default",(DWORD)-1)) == (DWORD)-1)
	{
		// This is a simple contact
		// (this should normally not happen, since linked contacts do not appear on the list.)
		//PUShowMessage("meta has no default", SM_NOTIFY);
		return 0;
	}
	else
	{
		HANDLE most_online;
        //DBEVENTINFO dbei;
		//char szServiceName[100];  

		most_online = Meta_GetMostOnlineSupporting(ccs->hContact, PFLAGNUM_1, PF1_FILESEND);

		if(!most_online) {
			//PUShowMessage("no most online for ft", SM_NOTIFY);
			return 0;
		}

		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);
		//Meta_CopyContactNick(ccs->hContact, most_online, proto);

		if(proto) {
			//ccs->hContact = most_online;
			//Meta_SetNick(proto);

			// don't check for existence of service - 'accounts' based protos don't have them!
			//_snprintf(szServiceName, sizeof(szServiceName), "%s%s", proto, PSS_FILE); 
			//if (ServiceExists(szServiceName)) {
			//	PUShowMessage("sending to subcontact", SM_NOTIFY);
				return (int)(CallContactService(most_online, PSS_FILE, ccs->wParam, ccs->lParam));
			//} else
			//	PUShowMessage("no service", SM_NOTIFY);
		} //else
			//PUShowMessage("no proto for subcontact", SM_NOTIFY);
	}
	return 0; // fail
}

INT_PTR Meta_GetAwayMsg(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	char *proto = 0;
	DWORD default_contact_number;

	if((default_contact_number = DBGetContactSettingDword(ccs->hContact,META_PROTO,"Default",(DWORD)-1)) == (DWORD)-1)
	{
		// This is a simple contact
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return 0;
	}
	else
	{
		HANDLE most_online;

		most_online = Meta_GetMostOnlineSupporting(ccs->hContact, PFLAGNUM_1, PF1_MODEMSGRECV);

		if(!most_online)
			return 0;

		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);
		if(!proto) return 0;

		//Meta_CopyContactNick(ccs->hContact, most_online, proto);

		ccs->hContact = most_online;
		//Meta_SetNick(proto);

		return (int)(CallContactService(ccs->hContact, PSS_GETAWAYMSG, ccs->wParam, ccs->lParam));
	}
	return 0; // fail
}

INT_PTR Meta_GetAvatarInfo(WPARAM wParam, LPARAM lParam) {
    PROTO_AVATAR_INFORMATION *AI = (PROTO_AVATAR_INFORMATION *) lParam;
	char *proto = 0;
	DWORD default_contact_number;

	if((default_contact_number = DBGetContactSettingDword(AI->hContact,META_PROTO,"Default",(DWORD)-1)) == (DWORD)-1)
	{
		// This is a simple contact
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return 0;
	}
	else
	{
		HANDLE hSub, hMeta;
		char szServiceName[100];  
		int result;

		hMeta = AI->hContact;
		hSub = Meta_GetMostOnlineSupporting(AI->hContact, PFLAGNUM_4, PF4_AVATARS);

		if(!hSub)
			return GAIR_NOAVATAR;

		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hSub, 0);
		if(!proto) return GAIR_NOAVATAR;

		AI->hContact = hSub;

		mir_snprintf(szServiceName, sizeof(szServiceName), "%s%s", proto, PS_GETAVATARINFO); 
		result = CallService(szServiceName, wParam, lParam);
		AI->hContact = hMeta;
		if (result != CALLSERVICE_NOTFOUND) return result;
	}
	return GAIR_NOAVATAR; // fail
}

INT_PTR Meta_GetInfo(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	char *proto = 0;
	DWORD default_contact_number;

	if((default_contact_number = DBGetContactSettingDword(ccs->hContact,META_PROTO,"Default",(DWORD)-1)) == (DWORD)-1)
	{
		// This is a simple contact
		// (this should normally not happen, since linked contacts do not appear on the list.)
		return 0;
	}
	else
	{
		HANDLE most_online;
		PROTO_AVATAR_INFORMATION AI;
		char szServiceName[100];  

		most_online = Meta_GetMostOnlineSupporting(ccs->hContact, PFLAGNUM_4, PF4_AVATARS);

		if(!most_online)
			return 0;

		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);
		if(!proto) return 0;

		AI.cbSize = sizeof(AI);
		AI.hContact = ccs->hContact;
		AI.format = PA_FORMAT_UNKNOWN;
		strcpy(AI.filename, "X");
		if((int)CallProtoService(META_PROTO, PS_GETAVATARINFO, 0, (LPARAM)&AI) == GAIR_SUCCESS)
	        DBWriteContactSettingString(ccs->hContact, "ContactPhoto", "File",AI.filename);

		most_online = Meta_GetMostOnline(ccs->hContact);
		Meta_CopyContactNick(ccs->hContact, most_online);

		if(!most_online)
			return 0;

		//Meta_CopyContactNick(ccs->hContact, most_online, proto);

		ccs->hContact = most_online;
		//Meta_SetNick(proto);

		_snprintf(szServiceName, sizeof(szServiceName), "%s%s", proto, PSS_GETINFO); 
		if (ServiceExists(szServiceName)) {
			strncpy(szServiceName, PSS_GETINFO, sizeof(szServiceName)); 
			return (int)(CallContactService(ccs->hContact, szServiceName, ccs->wParam, ccs->lParam));
		}
	}
	return 0; // fail
}

int Meta_OptInit(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp;
	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize						= sizeof(odp);
	odp.position					= -790000000;
	odp.hInstance					= hInstance;
	odp.flags						= ODPF_BOLDGROUPS;

	odp.pszTemplate					= MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszTitle					= "MetaContacts";
	odp.pszGroup					= "Contact List";
	odp.pszTab						= "General";
	odp.pfnDlgProc					= DlgProcOpts;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );	

	odp.pszTemplate					= MAKEINTRESOURCE(IDD_PRIORITIES);
	odp.pszTitle					= "MetaContacts";
	odp.pszGroup					= "Contact List";
	odp.pszTab						= "Priorities";
	odp.pfnDlgProc					= DlgProcOptsPriorities;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );	

	odp.pszTemplate					= MAKEINTRESOURCE(IDD_HISTORY);
	odp.pszTitle					= "MetaContacts";
	odp.pszGroup					= "Contact List";
	odp.pszTab						= "History";
	odp.pfnDlgProc					= DlgProcOpts;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );	
	return 0;
}

int Meta_CallMostOnline(WPARAM wParam, LPARAM lParam) {
	HANDLE most_online_im = Meta_GetMostOnline((HANDLE)wParam);

	// fix nick
	Meta_CopyContactNick((HANDLE)wParam, most_online_im);

	// fix status
	Meta_FixStatus((HANDLE)wParam);

	// copy all other data
	Meta_CopyData((HANDLE) wParam);

	return 0;
}


INT_PTR Meta_OnOff(WPARAM wParam, LPARAM lParam) {
	CLISTMENUITEM menu;
	menu.cbSize = sizeof(CLISTMENUITEM);
	// just write to db - the rest is handled in the Meta_SettingChanged function
	if(DBGetContactSettingByte(0, META_PROTO, "Enabled", 1)) {
		DBWriteContactSettingByte(0, META_PROTO, "Enabled", 0);
		// modify main menu item
		menu.flags = CMIM_NAME | CMIM_ICON;
		menu.hIcon = LoadIconEx(I_MENU);
		menu.pszName = "Toggle MetaContacts On";
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuOnOff, (LPARAM)&menu);
	} else {
		DBWriteContactSettingByte(0, META_PROTO, "Enabled", 1);
		// modify main menu item
		menu.flags = CMIM_NAME | CMIM_ICON;
		menu.hIcon = LoadIconEx(I_MENUOFF);
		menu.pszName = "Toggle MetaContacts Off";
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuOnOff, (LPARAM)&menu);
	}
	ReleaseIconEx(menu.hIcon);

	return 0;
}


int Meta_PreShutdown(WPARAM wParam, LPARAM lParam) {
	//MessageBox(0, "Preshutdown called", "MC", MB_OK);
	Meta_SetStatus((WPARAM)ID_STATUS_OFFLINE, 0);
	Meta_UnhideLinkedContacts();
	Meta_SuppressStatus(FALSE);
	//MessageBox(0, "Status is OFFLINE", "MC", MB_OK);
	//MessageBox(0, "Preshutdown complete", "MC", MB_OK);

	if(setStatusTimerId) KillTimer(0, setStatusTimerId);

	return 0;
}

int Meta_OkToExit(WPARAM wParam, LPARAM lParam) {
	Meta_SetStatus((WPARAM)ID_STATUS_OFFLINE, 0);
	return 0;
}

int Meta_OnIdleChanged(WPARAM wParam, LPARAM lParam) {
	return 0;
}

/** Initializes all services provided by the plugin
*
* Creates every function and hooks the event desired.
*/
void Meta_InitServices()
{
	int i;

	previousMode = mcStatus = ID_STATUS_OFFLINE;

	// set hooks pointers and services pointers to zero - in case we do not initialize them all correctly
	for(i=0;i<NB_HOOKS;i++)
		hHooks[i] = 0;
	for(i=0;i<NB_SERVICES;i++)
		hServices[i] = 0;
	
	hServices[0] = CreateServiceFunction("MetaContacts/Convert",Meta_Convert);
	hServices[1] = CreateServiceFunction("MetaContacts/AddTo",Meta_AddTo);
	hServices[2] = CreateServiceFunction("MetaContacts/Edit",Meta_Edit);
	hServices[3] = CreateServiceFunction("MetaContacts/Delete",Meta_Delete);
	hServices[4] = CreateServiceFunction("MetaContacts/Default",Meta_Default);
	hServices[5] = CreateServiceFunction("MetaContacts/ForceDefault",Meta_ForceDefault);

	// hidden contact menu items...ho hum
	hServices[6 + 0] = CreateServiceFunction("MetaContacts/MenuFunc0",MenuFunc0);
	hServices[6 + 1] = CreateServiceFunction("MetaContacts/MenuFunc1",MenuFunc1);
	hServices[6 + 2] = CreateServiceFunction("MetaContacts/MenuFunc2",MenuFunc2);
	hServices[6 + 3] = CreateServiceFunction("MetaContacts/MenuFunc3",MenuFunc3);
	hServices[6 + 4] = CreateServiceFunction("MetaContacts/MenuFunc4",MenuFunc4);
	hServices[6 + 5] = CreateServiceFunction("MetaContacts/MenuFunc5",MenuFunc5);
	hServices[6 + 6] = CreateServiceFunction("MetaContacts/MenuFunc6",MenuFunc6);
	hServices[6 + 7] = CreateServiceFunction("MetaContacts/MenuFunc7",MenuFunc7);
	hServices[6 + 8] = CreateServiceFunction("MetaContacts/MenuFunc8",MenuFunc8);
	hServices[6 + 9] = CreateServiceFunction("MetaContacts/MenuFunc9",MenuFunc9);
	hServices[6 + 10] = CreateServiceFunction("MetaContacts/MenuFunc10",MenuFunc10);
	hServices[6 + 11] = CreateServiceFunction("MetaContacts/MenuFunc11",MenuFunc11);
	hServices[6 + 12] = CreateServiceFunction("MetaContacts/MenuFunc12",MenuFunc12);
	hServices[6 + 13] = CreateServiceFunction("MetaContacts/MenuFunc13",MenuFunc13);
	hServices[6 + 14] = CreateServiceFunction("MetaContacts/MenuFunc14",MenuFunc14);
	hServices[6 + 15] = CreateServiceFunction("MetaContacts/MenuFunc15",MenuFunc15);
	hServices[6 + 16] = CreateServiceFunction("MetaContacts/MenuFunc16",MenuFunc16);
	hServices[6 + 17] = CreateServiceFunction("MetaContacts/MenuFunc17",MenuFunc17);
	hServices[6 + 18] = CreateServiceFunction("MetaContacts/MenuFunc18",MenuFunc18);
	hServices[6 + 19] = CreateServiceFunction("MetaContacts/MenuFunc19",MenuFunc19);

	hServices[26] = CreateProtoServiceFunction(META_PROTO,PS_GETCAPS,Meta_GetCaps);
	hServices[27] = CreateProtoServiceFunction(META_PROTO,PS_GETNAME,Meta_GetName);
	hServices[28] = CreateProtoServiceFunction(META_PROTO,PS_LOADICON,Meta_LoadIcon);

	hServices[29] = CreateProtoServiceFunction(META_PROTO,PS_SETSTATUS,Meta_SetStatus);

	hServices[30] = CreateProtoServiceFunction(META_PROTO,PS_GETSTATUS,Meta_GetStatus);
	hServices[31] = CreateProtoServiceFunction(META_PROTO,PSS_MESSAGE,Meta_SendMessage);
	hServices[32] = CreateProtoServiceFunction(META_PROTO,PSS_MESSAGE"W",Meta_SendMessage); // unicode send (same send func as above line, checks for PREF_UNICODE)

	hServices[33] = CreateProtoServiceFunction(META_PROTO,PSS_USERISTYPING,Meta_UserIsTyping );

	hServices[34] = CreateProtoServiceFunction(META_PROTO,PSR_MESSAGE,Meta_RecvMessage);

	// file recv is done by subcontacts
	//hServices[] = CreateProtoServiceFunction(META_PROTO,PS_FILERESUME,Meta_FileResume);
	//hServices[] = CreateProtoServiceFunction(META_PROTO,PSS_FILEALLOW,Meta_FileAllow);
	//hServices[] = CreateProtoServiceFunction(META_PROTO,PSS_FILEDENY,Meta_FileDeny);
	//hServices[] = CreateProtoServiceFunction(META_PROTO,PSS_FILECANCEL,Meta_FileCancel);
	//hServices[] = CreateProtoServiceFunction(META_PROTO,PSR_FILE,Meta_FileRecv);
	hServices[35] = CreateProtoServiceFunction(META_PROTO,PSS_FILE,Meta_FileSend);

	hServices[36] = CreateProtoServiceFunction(META_PROTO,PSS_GETAWAYMSG,Meta_GetAwayMsg);

	hServices[37] = CreateProtoServiceFunction(META_PROTO,PS_GETAVATARINFO,Meta_GetAvatarInfo);

	hServices[38] = CreateProtoServiceFunction(META_PROTO,PSS_GETINFO,Meta_GetInfo);

	hServices[39] = CreateProtoServiceFunction(META_FILTER,PSR_MESSAGE,MetaFilter_RecvMessage);
	hServices[40] = CreateProtoServiceFunction(META_FILTER,PSS_MESSAGE,MetaFilter_SendMessage);
	hServices[41] = CreateProtoServiceFunction(META_FILTER,PSS_MESSAGE"W",MetaFilter_SendMessage);

	// API services and events

	hServices[42] = CreateServiceFunction(MS_MC_GETMETACONTACT, MetaAPI_GetMeta);
	hServices[43] = CreateServiceFunction(MS_MC_GETDEFAULTCONTACT, MetaAPI_GetDefault);
	hServices[44] = CreateServiceFunction(MS_MC_GETDEFAULTCONTACTNUM, MetaAPI_GetDefaultNum);
	hServices[45] = CreateServiceFunction(MS_MC_GETMOSTONLINECONTACT, MetaAPI_GetMostOnline);
	hServices[46] = CreateServiceFunction(MS_MC_GETNUMCONTACTS, MetaAPI_GetNumContacts);
	hServices[47] = CreateServiceFunction(MS_MC_GETSUBCONTACT, MetaAPI_GetContact);
	hServices[48] = CreateServiceFunction(MS_MC_SETDEFAULTCONTACTNUM, MetaAPI_SetDefaultContactNum);
	hServices[49] = CreateServiceFunction(MS_MC_SETDEFAULTCONTACT, MetaAPI_SetDefaultContact);
	hServices[50] = CreateServiceFunction(MS_MC_FORCESENDCONTACTNUM, MetaAPI_ForceSendContactNum);
	hServices[51] = CreateServiceFunction(MS_MC_FORCESENDCONTACT, MetaAPI_ForceSendContact);
	hServices[52] = CreateServiceFunction(MS_MC_UNFORCESENDCONTACT, MetaAPI_UnforceSendContact);
	hServices[53] = CreateServiceFunction(MS_MC_GETPROTOCOLNAME, MetaAPI_GetProtoName);
	hServices[54] = CreateServiceFunction(MS_MC_GETFORCESTATE, MetaAPI_GetForceState);

	hServices[55] = CreateServiceFunction(MS_MC_CONVERTTOMETA, MetaAPI_ConvertToMeta);
	hServices[56] = CreateServiceFunction(MS_MC_ADDTOMETA, MetaAPI_AddToMeta);
	hServices[57] = CreateServiceFunction(MS_MC_REMOVEFROMMETA, MetaAPI_RemoveFromMeta);

	hServices[58] = CreateServiceFunction(MS_MC_DISABLEHIDDENGROUP, MetaAPI_DisableHiddenGroup);

	hServices[59] = CreateServiceFunction("MetaContacts/OnOff", Meta_OnOff);
	hServices[60] = CreateServiceFunction("MetaContacts/CListMessageEvent", Meta_ClistMessageEventClicked);

	hServices[61] = CreateProtoServiceFunction(META_PROTO, "/SendNudge", Meta_SendNudge);

	// create our hookable events
	hEventDefaultChanged = CreateHookableEvent(ME_MC_DEFAULTTCHANGED);
	hEventForceSend = CreateHookableEvent(ME_MC_FORCESEND);
	hEventUnforceSend = CreateHookableEvent(ME_MC_UNFORCESEND);
	hSubcontactsChanged = CreateHookableEvent(ME_MC_SUBCONTACTSCHANGED);

	// hook other module events we need
	hHooks[0] = (HANDLE)HookEvent(ME_PROTO_ACK, Meta_HandleACK);
	hHooks[1] = (HANDLE)HookEvent(ME_DB_CONTACT_SETTINGCHANGED, Meta_SettingChanged);
	hHooks[2] = (HANDLE)HookEvent(ME_SYSTEM_MODULESLOADED, Meta_ModulesLoaded);
	hHooks[3] = (HANDLE)HookEvent(ME_PROTO_CONTACTISTYPING, Meta_ContactIsTyping);
	hHooks[4] = (HANDLE)HookEvent(ME_DB_CONTACT_DELETED, Meta_ContactDeleted);
	hHooks[5] = (HANDLE)HookEvent(ME_OPT_INITIALISE, Meta_OptInit );

	hHooks[6] = (HANDLE)HookEvent(ME_SYSTEM_PRESHUTDOWN, Meta_PreShutdown);
	hHooks[7] = (HANDLE)HookEvent(ME_SYSTEM_OKTOEXIT, Meta_OkToExit);

	// hook our own events, used to call Meta_GetMostOnline which sets nick for metacontact
	hHooks[8] = (HANDLE)HookEvent(ME_MC_DEFAULTTCHANGED, Meta_CallMostOnline );
	hHooks[9] = (HANDLE)HookEvent(ME_MC_FORCESEND, Meta_CallMostOnline );
	hHooks[10] = (HANDLE)HookEvent(ME_MC_UNFORCESEND, Meta_CallMostOnline );

	/// more hooks in modules loaded event handler - for services that are not created

	//hHooks[] = (HANDLE)HookEvent(ME_IDLE_CHANGED, Meta_OnIdleChanged); // what can we do with idle?

	// redirect nudge events
	hEventNudge = CreateHookableEvent(META_PROTO "/Nudge");
}

//! Unregister all hooks and services from Miranda
void Meta_CloseHandles()
{
	int i;

	for(i=0;i<iNudgeProtos;i++)		// Idem for the hooks.
	{
		UnhookEvent(hNudgeEvents[i]);
	}
	free(hNudgeEvents);
	iNudgeProtos = 0;

	for(i=0;i<NB_HOOKS;i++)		// Idem for the hooks.
		if(hHooks[i]) UnhookEvent(hHooks[i]);

	if(ServiceExists(MS_CLIST_EXTRA_ADD_ICON)) {
		proto_count = 0;
	}

	// destroy our hookable events
	DestroyHookableEvent(hEventDefaultChanged);
	DestroyHookableEvent(hEventForceSend);
	DestroyHookableEvent(hEventUnforceSend);
	DestroyHookableEvent(hSubcontactsChanged);
	DestroyHookableEvent(hEventNudge);

	// lets leave them, hey? (why?)
	for(i=0;i<NB_SERVICES;i++)	// Scan each 'HANDLE' and Destroy the service attached to it.
		if(hServices[i]) DestroyServiceFunction(hServices[i]);

	DeinitIcons();
}
