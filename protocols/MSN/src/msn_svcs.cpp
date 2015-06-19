/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "msn_proto.h"

extern int avsPresent;

/////////////////////////////////////////////////////////////////////////////////////////
// GetMyAwayMsg - obtain the current away message

INT_PTR CMsnProto::GetMyAwayMsg(WPARAM wParam, LPARAM lParam)
{
	char** msgptr = GetStatusMsgLoc(wParam ? wParam : m_iStatus);
	if (msgptr == NULL)	return 0;

	return (lParam & SGMA_UNICODE) ? (INT_PTR)mir_utf8decodeW(*msgptr) : (INT_PTR)mir_utf8decodeA(*msgptr);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnGetAvatar - retrieves the file name of my own avatar

INT_PTR CMsnProto::GetAvatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR* buf = (TCHAR*)wParam;
	int  size = (int)lParam;

	if (buf == NULL || size <= 0)
		return -1;

	MSN_GetAvatarFileName(NULL, buf, size, NULL);
	return _taccess(buf, 0);
}


/////////////////////////////////////////////////////////////////////////////////////////
// MsnGetAvatarInfo - retrieve the avatar info

void CMsnProto::sttFakeAvatarAck(void* arg)
{
	Sleep(100);
	ProtoBroadcastAck(((PROTO_AVATAR_INFORMATION*)arg)->hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, arg, 0);
}

INT_PTR CMsnProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION*)lParam;
	TCHAR filename[MAX_PATH];
	MsnContact *cont = NULL;

	if (pai->hContact) {
		cont = Lists_Get(pai->hContact);
		if (cont == NULL) return GAIR_NOAVATAR;

		/*
		if ((cont->cap1 & 0xf0000000) == 0)
			return GAIR_NOAVATAR;
		*/
	}

	if (pai->hContact == NULL || _stricmp(cont->email, MyOptions.szEmail) == 0) {
		MSN_GetAvatarFileName(NULL, filename, _countof(filename), NULL);
		pai->format = ProtoGetAvatarFormat(filename);
		if (pai->format != PA_FORMAT_UNKNOWN)
			mir_tstrcpy(pai->filename, filename);
		return pai->format == PA_FORMAT_UNKNOWN ? GAIR_NOAVATAR : GAIR_SUCCESS;
	}

	char *szContext;
	DBVARIANT dbv;
	if (getString(pai->hContact, pai->hContact ? "PictContext" : "PictObject", &dbv) == 0) {
		szContext = (char*)NEWSTR_ALLOCA(dbv.pszVal);
		db_free(&dbv);
	}
	else return GAIR_NOAVATAR;

	MSN_GetAvatarFileName(pai->hContact, filename, _countof(filename), NULL);
	pai->format = ProtoGetAvatarFormat(filename);

	if (pai->format != PA_FORMAT_UNKNOWN) {
		bool needupdate = true;
		if (getString(pai->hContact, "PictSavedContext", &dbv) == 0) {
			needupdate = mir_strcmp(dbv.pszVal, szContext) != 0;
			db_free(&dbv);
		}

		if (needupdate) {
			setString(pai->hContact, "PictSavedContext", szContext);

			// Store also avatar hash
			char* szAvatarHash = MSN_GetAvatarHash(szContext);
			if (szAvatarHash != NULL) {
				setString(pai->hContact, "AvatarSavedHash", szAvatarHash);
				mir_free(szAvatarHash);
			}
		}
		mir_tstrcpy(pai->filename, filename);
		return GAIR_SUCCESS;
	}

	if ((wParam & GAIF_FORCE) != 0 && pai->hContact != NULL) {
		if (avsPresent < 0) avsPresent = ServiceExists(MS_AV_SETMYAVATAR) != 0;
		if (!avsPresent)
			return GAIR_NOAVATAR;

		WORD wStatus = getWord(pai->hContact, "Status", ID_STATUS_OFFLINE);
		if (wStatus == ID_STATUS_OFFLINE) {
			delSetting(pai->hContact, "AvatarHash");
			PROTO_AVATAR_INFORMATION *fakeAI = new PROTO_AVATAR_INFORMATION;
			*fakeAI = *pai;
			ForkThread(&CMsnProto::sttFakeAvatarAck, fakeAI);
		}
		else if (!getString(pai->hContact, "AvatarUrl", &dbv)) {
			pushAvatarRequest(pai->hContact, dbv.pszVal);
			db_free(&dbv);
		}
#ifdef OBSOLETE
		else if (p2p_getAvatarSession(pai->hContact) == NULL) {
			filetransfer* ft = new filetransfer(this);
			ft->std.hContact = pai->hContact;
			ft->p2p_object = mir_strdup(szContext);

			MSN_GetAvatarFileName(pai->hContact, filename, _countof(filename), _T("unk"));
			ft->std.tszCurrentFile = mir_tstrdup(filename);

			p2p_invite(MSN_APPID_AVATAR, ft, NULL);
		}
#endif

		return GAIR_WAITFOR;
	}
	return GAIR_NOAVATAR;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnGetAvatarCaps - retrieves avatar capabilities

INT_PTR CMsnProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	int res = 0;

	switch (wParam) {
	case AF_MAXSIZE:
		((POINT*)lParam)->x = 96;
		((POINT*)lParam)->y = 96;
		break;

	case AF_PROPORTION:
		res = PIP_NONE;
		break;

	case AF_FORMATSUPPORTED:
		res = lParam == PA_FORMAT_PNG || lParam == PA_FORMAT_GIF || lParam == PA_FORMAT_JPEG;
		break;

	case AF_ENABLED:
		res = 1;
		break;
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
//	MsnSetAvatar - sets an avatar without UI

INT_PTR CMsnProto::SetAvatar(WPARAM, LPARAM lParam)
{
	TCHAR* szFileName = (TCHAR*)lParam;

	TCHAR tFileName[MAX_PATH];
	MSN_GetAvatarFileName(NULL, tFileName, _countof(tFileName), NULL);
	_tremove(tFileName);

	if (szFileName == NULL) {
		delSetting("PictObject");
		delSetting("AvatarHash");
		ForkThread(&CMsnProto::msn_storeAvatarThread, NULL);
	}
	else {
		int fileId = _topen(szFileName, _O_RDONLY | _O_BINARY, _S_IREAD);
		if (fileId < 0) return 1;

		size_t dwPngSize = _filelengthi64(fileId);
		unsigned char* pData = (unsigned char*)mir_alloc(dwPngSize);
		if (pData == NULL) {
			_close(fileId);
			return 2;
		}

		_read(fileId, pData, (unsigned)dwPngSize);
		_close(fileId);

		TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_tsplitpath(szFileName, drive, dir, fname, ext);

		MSN_SetMyAvatar(fname, pData, dwPngSize);

		StoreAvatarData* par = (StoreAvatarData*)mir_alloc(sizeof(StoreAvatarData));
		par->szName = mir_tstrdup(fname);
		par->data = pData;
		par->dataSize = dwPngSize;
		par->szMimeType = "image/png";

		ForkThread(&CMsnProto::msn_storeAvatarThread, par);
	}

	MSN_SetServerStatus(m_iStatus);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//	SetNickname - sets a nick name without UI

INT_PTR CMsnProto::SetNickName(WPARAM wParam, LPARAM lParam)
{
	if (wParam & SMNN_UNICODE)
		MSN_SendNickname((wchar_t*)lParam);
	else
		MSN_SendNickname((char*)lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnSendNudge - Sending a nudge

INT_PTR CMsnProto::SendNudge(WPARAM hContact, LPARAM)
{
	if (!msnLoggedIn) return 0;

	char tEmail[MSN_MAX_EMAIL_LEN];
	if (MSN_IsMeByContact(hContact, tEmail)) return 0;

	int netId = Lists_GetNetId(tEmail);

#ifdef OBSOLETE
	static const char nudgemsg[] =
		"Content-Type: text/x-msnmsgr-datacast\r\n\r\n"
		"ID: 1\r\n\r\n";

	switch (netId) {
	case NETID_UNKNOWN:
		hContact = MSN_GetChatInernalHandle(hContact);

	case NETID_MSN:
	case NETID_LCS:
	{
		bool isOffline;
		ThreadData* thread = MSN_StartSB(tEmail, isOffline);
		if (thread == NULL) {
			if (isOffline) return 0;
			MsgQueue_Add(tEmail, 'N', nudgemsg, -1);
		}
		else {
			int tNnetId = netId == NETID_UNKNOWN ? NETID_MSN : netId;
			thread->sendMessage('N', tEmail, tNnetId, nudgemsg, MSG_DISABLE_HDR);
		}
	}
	break;

	case NETID_YAHOO:
		msnNsThread->sendMessage('3', tEmail, netId, nudgemsg, MSG_DISABLE_HDR);
		break;

	default:
		break;
	}
#else
	msnNsThread->sendMessage('3', tEmail, netId, "", MSG_NUDGE);
#endif
	return 0;
}

#ifdef OBSOLETE
/////////////////////////////////////////////////////////////////////////////////////////
//	GetCurrentMedia - get current media

INT_PTR CMsnProto::GetCurrentMedia(WPARAM, LPARAM lParam)
{
	LISTENINGTOINFO *cm = (LISTENINGTOINFO *)lParam;

	if (cm == NULL || cm->cbSize != sizeof(LISTENINGTOINFO))
		return -1;

	cm->ptszArtist = mir_tstrdup(msnCurrentMedia.ptszArtist);
	cm->ptszAlbum = mir_tstrdup(msnCurrentMedia.ptszAlbum);
	cm->ptszTitle = mir_tstrdup(msnCurrentMedia.ptszTitle);
	cm->ptszTrack = mir_tstrdup(msnCurrentMedia.ptszTrack);
	cm->ptszYear = mir_tstrdup(msnCurrentMedia.ptszYear);
	cm->ptszGenre = mir_tstrdup(msnCurrentMedia.ptszGenre);
	cm->ptszLength = mir_tstrdup(msnCurrentMedia.ptszLength);
	cm->ptszPlayer = mir_tstrdup(msnCurrentMedia.ptszPlayer);
	cm->ptszType = mir_tstrdup(msnCurrentMedia.ptszType);
	cm->dwFlags = msnCurrentMedia.dwFlags;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//	SetCurrentMedia - set current media

INT_PTR CMsnProto::SetCurrentMedia(WPARAM, LPARAM lParam)
{
	// Clear old info
	mir_free(msnCurrentMedia.ptszArtist);
	mir_free(msnCurrentMedia.ptszAlbum);
	mir_free(msnCurrentMedia.ptszTitle);
	mir_free(msnCurrentMedia.ptszTrack);
	mir_free(msnCurrentMedia.ptszYear);
	mir_free(msnCurrentMedia.ptszGenre);
	mir_free(msnCurrentMedia.ptszLength);
	mir_free(msnCurrentMedia.ptszPlayer);
	mir_free(msnCurrentMedia.ptszType);
	memset(&msnCurrentMedia, 0, sizeof(msnCurrentMedia));

	// Copy new info
	LISTENINGTOINFO *cm = (LISTENINGTOINFO *)lParam;
	if (cm != NULL && cm->cbSize == sizeof(LISTENINGTOINFO) && (cm->ptszArtist != NULL || cm->ptszTitle != NULL)) {
		bool unicode = (cm->dwFlags & LTI_UNICODE) != 0;

		msnCurrentMedia.cbSize = sizeof(msnCurrentMedia);	// Marks that there is info set
		msnCurrentMedia.dwFlags = LTI_TCHAR;

		overrideStr(msnCurrentMedia.ptszType, cm->ptszType, unicode, _T("Music"));
		overrideStr(msnCurrentMedia.ptszArtist, cm->ptszArtist, unicode);
		overrideStr(msnCurrentMedia.ptszAlbum, cm->ptszAlbum, unicode);
		overrideStr(msnCurrentMedia.ptszTitle, cm->ptszTitle, unicode, _T("No Title"));
		overrideStr(msnCurrentMedia.ptszTrack, cm->ptszTrack, unicode);
		overrideStr(msnCurrentMedia.ptszYear, cm->ptszYear, unicode);
		overrideStr(msnCurrentMedia.ptszGenre, cm->ptszGenre, unicode);
		overrideStr(msnCurrentMedia.ptszLength, cm->ptszLength, unicode);
		overrideStr(msnCurrentMedia.ptszPlayer, cm->ptszPlayer, unicode);
	}

	// Set user text
	if (msnCurrentMedia.cbSize == 0)
		delSetting("ListeningTo");
	else {
		TCHAR *text;
		if (ServiceExists(MS_LISTENINGTO_GETPARSEDTEXT))
			text = (TCHAR *)CallService(MS_LISTENINGTO_GETPARSEDTEXT, (WPARAM)_T("%title% - %artist%"), (LPARAM)&msnCurrentMedia);
		else {
			text = (TCHAR *)mir_alloc(128 * sizeof(TCHAR));
			mir_sntprintf(text, 128, _T("%s - %s"), (msnCurrentMedia.ptszTitle ? msnCurrentMedia.ptszTitle : _T("")),
				(msnCurrentMedia.ptszArtist ? msnCurrentMedia.ptszArtist : _T("")));
		}
		setTString("ListeningTo", text);
		mir_free(text);
	}

	// Send it
	char** msgptr = GetStatusMsgLoc(m_iDesiredStatus);
	MSN_SendStatusMessage(msgptr ? *msgptr : NULL);

	return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// MsnContactDeleted - called when a contact is deleted from list

int CMsnProto::OnContactDeleted(WPARAM hContact, LPARAM)
{
	if (!msnLoggedIn)  //should never happen for MSN contacts
		return 0;

	if (isChatRoom(hContact)) {
		DBVARIANT dbv;
		if (!getTString(hContact, "ChatRoomID", &dbv)) {
			MSN_KillChatSession(dbv.ptszVal);
			db_free(&dbv);
		}
	}
	else {
		char szEmail[MSN_MAX_EMAIL_LEN];
		if (MSN_IsMeByContact(hContact, szEmail))
			CallService(MS_CLIST_REMOVEEVENT, hContact, 1);

		if (szEmail[0]) {
			debugLogA("Deleted Handler Email");

			if (Lists_IsInList(LIST_FL, szEmail)) {
				DeleteParam param = { this, hContact };
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DELETECONTACT), NULL, DlgDeleteContactUI, (LPARAM)&param);

				MsnContact *msc = Lists_Get(szEmail);
				if (msc)
					msc->hContact = NULL;
			}

			if (Lists_IsInList(LIST_LL, szEmail))
				MSN_AddUser(hContact, szEmail, 0, LIST_LL | LIST_REMOVE);
		}
	}

	return 0;
}


int CMsnProto::OnGroupChange(WPARAM hContact, LPARAM lParam)
{
	if (!msnLoggedIn || !MyOptions.ManageServer) return 0;

	const CLISTGROUPCHANGE* grpchg = (CLISTGROUPCHANGE*)lParam;

	if (hContact == NULL) {
		if (grpchg->pszNewName == NULL && grpchg->pszOldName != NULL) {
			LPCSTR szId = MSN_GetGroupByName(UTF8(grpchg->pszOldName));
			if (szId != NULL)
				MSN_DeleteServerGroup(szId);
		}
		else if (grpchg->pszNewName != NULL && grpchg->pszOldName != NULL) {
			LPCSTR szId = MSN_GetGroupByName(UTF8(grpchg->pszOldName));
			if (szId != NULL)
				MSN_RenameServerGroup(szId, UTF8(grpchg->pszNewName));
		}
	}
	else {
		if (MSN_IsMyContact(hContact))
			MSN_MoveContactToGroup(hContact, T2Utf(grpchg->pszNewName));
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
// MsnDbSettingChanged - look for contact's settings changes

int CMsnProto::OnDbSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING* cws = (DBCONTACTWRITESETTING*)lParam;

	if (!msnLoggedIn || MyOptions.netId != NETID_MSN)
		return 0;

	if (hContact == NULL) {
		if (MyOptions.SlowSend && mir_strcmp(cws->szSetting, "MessageTimeout") == 0 &&
			(mir_strcmp(cws->szModule, "SRMM") == 0 || mir_strcmp(cws->szModule, "SRMsg") == 0)) {
			if (cws->value.dVal < 60000)
				MessageBox(NULL, TranslateT("MSN requires message send timeout in your Message window plugin to be not less then 60 sec. Please correct the timeout value."),
				TranslateT("MSN Protocol"), MB_OK | MB_ICONINFORMATION);
		}
		return 0;
	}

	if (!mir_strcmp(cws->szSetting, "ApparentMode")) {
		char tEmail[MSN_MAX_EMAIL_LEN];
		if (!db_get_static(hContact, m_szModuleName, "wlid", tEmail, sizeof(tEmail)) ||
			!db_get_static(hContact, m_szModuleName, "e-mail", tEmail, sizeof(tEmail))) {
			bool isBlocked = Lists_IsInList(LIST_BL, tEmail);

			if (isBlocked && (cws->value.type == DBVT_DELETED || cws->value.wVal == 0)) {
				MSN_AddUser(hContact, tEmail, 0, LIST_BL + LIST_REMOVE);
				MSN_AddUser(hContact, tEmail, 0, LIST_AL);
			}
			else if (!isBlocked && cws->value.wVal == ID_STATUS_OFFLINE) {
				MSN_AddUser(hContact, tEmail, 0, LIST_AL + LIST_REMOVE);
				MSN_AddUser(hContact, tEmail, 0, LIST_BL);
			}
		}
	}

	if (!mir_strcmp(cws->szSetting, "MyHandle") && !mir_strcmp(cws->szModule, "CList")) {
		bool isMe = MSN_IsMeByContact(hContact);
		if (!isMe || !nickChg) {
			char szContactID[100];
			if (!db_get_static(hContact, m_szModuleName, "ID", szContactID, sizeof(szContactID))) {
				if (cws->value.type != DBVT_DELETED) {
					if (cws->value.type == DBVT_UTF8)
						MSN_ABUpdateNick(cws->value.pszVal, szContactID);
					else
						MSN_ABUpdateNick(UTF8(cws->value.pszVal), szContactID);
				}
				else MSN_ABUpdateNick(NULL, szContactID);
			}

			if (isMe)
				displayEmailCount(hContact);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnIdleChanged - transitions to Idle

int CMsnProto::OnIdleChanged(WPARAM, LPARAM lParam)
{
	if (m_iStatus == ID_STATUS_INVISIBLE || m_iStatus <= ID_STATUS_OFFLINE)
		return 0;

	bool bIdle = (lParam & IDF_ISIDLE) != 0;
	bool bPrivacy = (lParam & IDF_PRIVACY) != 0;

	if (isIdle && !bIdle) {
		isIdle = false;
		MSN_SetServerStatus(m_iDesiredStatus);
	}
	else if (!isIdle && bIdle && !bPrivacy && m_iDesiredStatus != ID_STATUS_AWAY) {
		isIdle = true;
		MSN_SetServerStatus(ID_STATUS_IDLE);
	}

	return 0;
}

#ifdef OBSOLETE
/////////////////////////////////////////////////////////////////////////////////////////
// OnWindowEvent - creates session on window open

int CMsnProto::OnWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData* msgEvData = (MessageWindowEventData*)lParam;

	if (msgEvData->uType == MSG_WINDOW_EVT_OPENING) {
		if (m_iStatus == ID_STATUS_OFFLINE || m_iStatus == ID_STATUS_INVISIBLE)
			return 0;

		if (!MSN_IsMyContact(msgEvData->hContact)) return 0;

		char tEmail[MSN_MAX_EMAIL_LEN];
		if (MSN_IsMeByContact(msgEvData->hContact, tEmail)) return 0;

		int netId = Lists_GetNetId(tEmail);
		if (netId != NETID_MSN && netId != NETID_LCS) return 0;

		if (Lists_IsInList(LIST_BL, tEmail)) return 0;

		bool isOffline;
		ThreadData* thread = MSN_StartSB(tEmail, isOffline);

		if (thread == NULL && !isOffline)
			MsgQueue_Add(tEmail, 'X', NULL, 0);
	}
	return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// OnWindowEvent - creates session on window open

int CMsnProto::OnWindowPopup(WPARAM, LPARAM lParam)
{
	MessageWindowPopupData *mwpd = (MessageWindowPopupData *)lParam;
	if (!MSN_IsMyContact(mwpd->hContact) || isChatRoom(mwpd->hContact))
		return 0;

	switch (mwpd->uType) {
	case MSG_WINDOWPOPUP_SHOWING:
		AppendMenu(mwpd->hMenu, MF_STRING, 13465, TranslateT("Convert to Chat"));
		break;

	case MSG_WINDOWPOPUP_SELECTED:
		if (mwpd->selection == 13465)
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, DlgInviteToChat, LPARAM(new InviteChatParam(NULL, mwpd->hContact, this)));
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnGetUnread - returns the actual number of unread emails in the INBOX

INT_PTR CMsnProto::GetUnreadEmailCount(WPARAM, LPARAM)
{
	if (!msnLoggedIn)
		return 0;
	return mUnreadMessages;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnLeaveChat - closes MSN chat window

INT_PTR CMsnProto::OnLeaveChat(WPARAM hContact, LPARAM)
{
	if (isChatRoom(hContact) != 0) {
		DBVARIANT dbv;
		if (getTString(hContact, "ChatRoomID", &dbv) == 0) {
			MSN_KillChatSession(dbv.ptszVal);
			db_free(&dbv);
		}
	}
	return 0;
}
