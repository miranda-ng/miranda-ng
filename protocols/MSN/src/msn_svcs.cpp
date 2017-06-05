/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2017 Miranda NG Team
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
	wchar_t* buf = (wchar_t*)wParam;
	int  size = (int)lParam;

	if (buf == NULL || size <= 0)
		return -1;

	MSN_GetAvatarFileName(NULL, buf, size, NULL);
	return _waccess(buf, 0);
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
	wchar_t filename[MAX_PATH];
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
			mir_wstrcpy(pai->filename, filename);
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
		mir_wstrcpy(pai->filename, filename);
		return GAIR_SUCCESS;
	}

	if ((wParam & GAIF_FORCE) != 0 && pai->hContact != NULL) {
		if (avsPresent < 0) avsPresent = ServiceExists(MS_AV_SETMYAVATARW) != 0;
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
	wchar_t* szFileName = (wchar_t*)lParam;

	wchar_t tFileName[MAX_PATH];
	MSN_GetAvatarFileName(NULL, tFileName, _countof(tFileName), NULL);
	_wremove(tFileName);

	if (szFileName == NULL) {
		delSetting("PictObject");
		delSetting("AvatarHash");
		ForkThread(&CMsnProto::msn_storeAvatarThread, NULL);
	}
	else {
		int fileId = _wopen(szFileName, _O_RDONLY | _O_BINARY, _S_IREAD);
		if (fileId < 0) return 1;

		size_t dwPngSize = _filelengthi64(fileId);
		unsigned char* pData = (unsigned char*)mir_alloc(dwPngSize);
		if (pData == NULL) {
			_close(fileId);
			return 2;
		}

		_read(fileId, pData, (unsigned)dwPngSize);
		_close(fileId);

		wchar_t drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_wsplitpath(szFileName, drive, dir, fname, ext);

		MSN_SetMyAvatar(fname, pData, dwPngSize);

		StoreAvatarData* par = (StoreAvatarData*)mir_alloc(sizeof(StoreAvatarData));
		par->szName = mir_wstrdup(fname);
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
	msnNsThread->sendMessage('3', tEmail, netId, "", MSG_NUDGE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnContactDeleted - called when a contact is deleted from list

int CMsnProto::OnContactDeleted(WPARAM hContact, LPARAM)
{
	if (!msnLoggedIn)  //should never happen for MSN contacts
		return 0;

	if (isChatRoom(hContact)) {
		DBVARIANT dbv;
		if (!getWString(hContact, "ChatRoomID", &dbv)) {
			MSN_KillChatSession(dbv.ptszVal);
			db_free(&dbv);
		}
	}
	else {
		char szEmail[MSN_MAX_EMAIL_LEN];
		if (MSN_IsMeByContact(hContact, szEmail))
			pcli->pfnRemoveEvent(hContact, 1);

		if (szEmail[0]) {
			debugLogA("Deleted Handler Email");

			if (Lists_IsInList(LIST_FL, szEmail)) {
				DeleteParam param = { this, MCONTACT(hContact) };
				DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_DELETECONTACT), NULL, DlgDeleteContactUI, (LPARAM)&param);

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
		if (MyOptions.SlowSend && strcmp(cws->szSetting, "MessageTimeout") == 0 &&
			(strcmp(cws->szModule, "SRMM") == 0 || strcmp(cws->szModule, "SRMsg") == 0)) {
			if (cws->value.dVal < 60000)
				MessageBox(NULL, TranslateT("MSN requires message send timeout in your Message window plugin to be not less then 60 sec. Please correct the timeout value."),
				TranslateT("MSN Protocol"), MB_OK | MB_ICONINFORMATION);
		}
		return 0;
	}

	if (!strcmp(cws->szSetting, "ApparentMode")) {
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

	if (!strcmp(cws->szSetting, "MyHandle") && !strcmp(cws->szModule, "CList")) {
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
			DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, DlgInviteToChat, LPARAM(new InviteChatParam(NULL, mwpd->hContact, this)));
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
		if (getWString(hContact, "ChatRoomID", &dbv) == 0) {
			MSN_KillChatSession(dbv.ptszVal);
			db_free(&dbv);
		}
	}
	return 0;
}
