/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2012 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

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

INT_PTR CAimProto::GetMyAwayMsg(WPARAM wParam,LPARAM lParam)
{
	char** msgptr = get_status_msg_loc(wParam ? wParam : m_iStatus);
	if (msgptr == NULL)	return 0;

	return (lParam & SGMA_UNICODE) ? (INT_PTR)mir_utf8decodeW(*msgptr) : (INT_PTR)mir_utf8decodeA(*msgptr);
}

int CAimProto::OnIdleChanged(WPARAM, LPARAM lParam)
{
	if (state != 1) 
	{
		idle=0;
		return 0;
	}

	if (instantidle) //ignore- we are instant idling at the moment
		return 0;

	bool bIdle = (lParam & IDF_ISIDLE) != 0;
	bool bPrivacy = (lParam & IDF_PRIVACY) != 0;

	if (bPrivacy && idle) 
	{
		aim_set_idle(hServerConn,seqno,0);
		return 0;
	}

	if (bPrivacy)
		return 0;

	if (bIdle)  //don't want to change idle time if we are already idle
	{
		MIRANDA_IDLE_INFO mii = { sizeof(mii) };
		CallService(MS_IDLE_GETIDLEINFO, 0, (LPARAM)&mii);

		idle = 1;
		aim_set_idle(hServerConn,seqno,mii.idleTime * 60);
	}
	else aim_set_idle(hServerConn,seqno,0);

	return 0;
}

int CAimProto::OnWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData* msgEvData  = (MessageWindowEventData*)lParam;

	if (msgEvData->uType == MSG_WINDOW_EVT_CLOSE) 
	{
		if (state != 1 || !is_my_contact(msgEvData->hContact)) 
			return 0;

		if (getWord(msgEvData->hContact, AIM_KEY_ST, ID_STATUS_OFFLINE) == ID_STATUS_ONTHEPHONE)
			return 0;

		DBVARIANT dbv;
		if (!getBool(msgEvData->hContact, AIM_KEY_BLS, false) && !getString(msgEvData->hContact, AIM_KEY_SN, &dbv)) 
		{
			if (_stricmp(dbv.pszVal, SYSTEM_BUDDY))
				aim_typing_notification(hServerConn, seqno, dbv.pszVal, 0x000f);
			db_free(&dbv);
		}
	}
	return 0;
}

INT_PTR CAimProto::GetProfile(WPARAM wParam, LPARAM)
{
	if (state != 1)
		return 0;

	DBVARIANT dbv;
	if (!getString(wParam, AIM_KEY_SN, &dbv)) {
		request_HTML_profile = 1;
		aim_query_profile(hServerConn, seqno, dbv.pszVal);
		db_free(&dbv);
	}
	return 0;
}

INT_PTR CAimProto::GetHTMLAwayMsg(WPARAM wParam, LPARAM)
{
	if (state != 1)
		return 0;

	DBVARIANT dbv;
	if (!getString(wParam, AIM_KEY_SN, &dbv))
	{
		request_away_message = 1;
		aim_query_away_message(hServerConn, seqno, dbv.pszVal);
	}
	return 0;
}

int CAimProto::OnDbSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;

	if (mir_strcmp(cws->szModule, MOD_KEY_CL) == 0 && state == 1 && hContact)
	{
		if (mir_strcmp(cws->szSetting, AIM_KEY_NL) == 0)
		{
			if (cws->value.type == DBVT_DELETED)
			{
				DBVARIANT dbv;
				if (!db_get_utf(hContact, MOD_KEY_CL, OTH_KEY_GP, &dbv) && dbv.pszVal[0])
				{
					add_contact_to_group(hContact, dbv.pszVal);
					db_free(&dbv);
				}
				else
					add_contact_to_group(hContact, AIM_DEFAULT_GROUP);
			}
		}
		else if (mir_strcmp(cws->szSetting, "MyHandle") == 0)
		{
			char* name;
			switch (cws->value.type)
			{
			case DBVT_DELETED:
				set_local_nick(hContact, NULL, NULL);
				break;

			case DBVT_ASCIIZ:
				name = mir_utf8encode(cws->value.pszVal);
				set_local_nick(hContact, name, NULL);
				mir_free(name);
				break;

			case DBVT_UTF8:
				set_local_nick(hContact, cws->value.pszVal, NULL);
				break;

			case DBVT_WCHAR:
				name = mir_utf8encodeW(cws->value.pwszVal);
				set_local_nick(hContact, name, NULL);
				mir_free(name);
				break;
			}
		}
	}

	return 0;
}

int CAimProto::OnContactDeleted(WPARAM hContact, LPARAM)
{
	if (state != 1) return 0;

	if (db_get_b(hContact, MOD_KEY_CL, AIM_KEY_NL, 0))
		return 0;

	DBVARIANT dbv;
	if (!getString(hContact, AIM_KEY_SN, &dbv)) 
	{
		for(int i=1;;++i)
		{
			unsigned short item_id = getBuddyId(hContact, i);
			if (item_id == 0) break; 

			unsigned short group_id = getGroupId(hContact, i);
			if (group_id)
			{
				bool is_not_in_list = getBool(hContact, AIM_KEY_NIL, false);
				aim_ssi_update(hServerConn, seqno, true);
				aim_delete_contact(hServerConn, seqno, dbv.pszVal, item_id, group_id, 0, is_not_in_list);
				char* group = group_list.find_name(group_id);
				update_server_group(group, group_id);
				aim_ssi_update(hServerConn, seqno, false);
			}
		}
		db_free(&dbv);
	}
	return 0;
}


int CAimProto::OnGroupChange(WPARAM hContact,LPARAM lParam)
{
	if (state != 1 || !getByte(AIM_KEY_MG, 1)) return 0;

	CLISTGROUPCHANGE* grpchg = (CLISTGROUPCHANGE*)lParam;

	if (hContact == NULL) {
		if (grpchg->pszNewName == NULL && grpchg->pszOldName != NULL) {
			T2Utf szOldName(grpchg->pszOldName);
			unsigned short group_id = group_list.find_id(szOldName);
			if (group_id) {
				aim_delete_contact(hServerConn, seqno, szOldName, 0, group_id, 1, false);
				group_list.remove_by_id(group_id);
				update_server_group("", 0);
			}
		}
		else if (grpchg->pszNewName != NULL && grpchg->pszOldName != NULL) {
			unsigned short group_id = group_list.find_id(T2Utf(grpchg->pszOldName));
			if (group_id)
				update_server_group(T2Utf(grpchg->pszNewName), group_id);
		}
	}
	else {
		if (is_my_contact(hContact) && getBuddyId(hContact, 1) && !db_get_b(hContact, MOD_KEY_CL, AIM_KEY_NL, 0)) {
			if (grpchg->pszNewName)
				add_contact_to_group(hContact, T2Utf(grpchg->pszNewName));
			else
				add_contact_to_group(hContact, AIM_DEFAULT_GROUP);
		}
	}
	return 0;
}

INT_PTR CAimProto::AddToServerList(WPARAM hContact, LPARAM)
{
	if (state != 1) return 0;

	DBVARIANT dbv;
	if (!db_get_utf(hContact, MOD_KEY_CL, OTH_KEY_GP, &dbv) && dbv.pszVal[0]) {
		add_contact_to_group(hContact, dbv.pszVal);
		db_free(&dbv);
	}
	else add_contact_to_group(hContact, AIM_DEFAULT_GROUP);
	return 0;
}

INT_PTR CAimProto::BlockBuddy(WPARAM hContact, LPARAM)
{
	if (state != 1)
		return 0;

	unsigned short item_id;
	DBVARIANT dbv;
	if (getString(hContact, AIM_KEY_SN, &dbv))
		return 0;

	switch(pd_mode) {
	case 1:
		pd_mode = 4;
		aim_set_pd_info(hServerConn, seqno);

	case 4:
		item_id = block_list.find_id(dbv.pszVal);
		if (item_id != 0) {
			block_list.remove_by_id(item_id);
			aim_delete_contact(hServerConn, seqno, dbv.pszVal, item_id, 0, 3, false);
		}
		else {
			item_id = block_list.add(dbv.pszVal);
			aim_add_contact(hServerConn, seqno, dbv.pszVal, item_id, 0, 3, false);
		}
		break;

	case 2:
		pd_mode = 3;
		aim_set_pd_info(hServerConn, seqno);

	case 3:
		item_id = allow_list.find_id(dbv.pszVal);
		if (item_id != 0) {
			allow_list.remove_by_id(item_id);
			aim_delete_contact(hServerConn, seqno, dbv.pszVal, item_id, 0, 2, false);
		}
		else {
			item_id = allow_list.add(dbv.pszVal);
			aim_add_contact(hServerConn, seqno, dbv.pszVal, item_id, 0, 2);
		}
		break;
	}
	db_free(&dbv);

	return 0;
}

 INT_PTR CAimProto::JoinChatUI(WPARAM, LPARAM)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHAT), NULL, join_chat_dialog, LPARAM(this));
	return 0;
}

INT_PTR CAimProto::OnJoinChat(WPARAM hContact, LPARAM)
{
	if (state != 1)	return 0;

	DBVARIANT dbv;
	if (!getString(hContact, "ChatRoomID", &dbv))
	{
		chatnav_param* par = new chatnav_param(dbv.pszVal, getWord(hContact, "Exchange", 4));
		ForkThread(&CAimProto::chatnav_request_thread, par);
		db_free(&dbv);
	}
	return 0;
}

INT_PTR CAimProto::OnLeaveChat(WPARAM wParam, LPARAM)
{
	if (state != 1)	return 0;

	MCONTACT hContact = wParam;

	DBVARIANT dbv;
	if (!getString(hContact, "ChatRoomID", &dbv))
	{
		chat_leave(dbv.pszVal);
		db_free(&dbv);
	}
	return 0;
}

INT_PTR CAimProto::InstantIdle(WPARAM, LPARAM)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_IDLE), NULL, instant_idle_dialog, LPARAM(this));
	return 0;
}

INT_PTR CAimProto::ManageAccount(WPARAM, LPARAM)
{
	ShellExecuteA(NULL, "open", "https://my.screenname.aol.com", NULL, NULL, SW_SHOW);
	return 0;
}

INT_PTR CAimProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONT* AI = (PROTO_AVATAR_INFORMATIONT*)lParam;
	
	AI->filename[0] = 0;
	AI->format = PA_FORMAT_UNKNOWN;

	if (getByte(AIM_KEY_DA, 0)) return GAIR_NOAVATAR;

	switch (get_avatar_filename(AI->hContact, AI->filename, SIZEOF(AI->filename), NULL))
	{
	case GAIR_SUCCESS:
		if (!(wParam & GAIF_FORCE) || state != 1 ) 
			return GAIR_SUCCESS;

	case GAIR_WAITFOR:
		AI->format = ProtoGetAvatarFormat(AI->filename);
		break;

	default:
		return GAIR_NOAVATAR;
	}

	if (state == 1)
	{
		ForkThread(&CAimProto::avatar_request_thread, (void*)AI->hContact);
		return GAIR_WAITFOR;
	}

	return GAIR_NOAVATAR;
}

INT_PTR CAimProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	int res = 0;

	switch (wParam) {
	case AF_MAXSIZE:
		((POINT*)lParam)->x = 100;
		((POINT*)lParam)->y = 100;
		break;

	case AF_MAXFILESIZE:
		res = 11264;
		break;

	case AF_PROPORTION:
		res = PIP_SQUARE;
		break;

	case AF_FORMATSUPPORTED:
		res = (lParam == PA_FORMAT_JPEG || lParam == PA_FORMAT_GIF || lParam == PA_FORMAT_BMP);
		break;

	case AF_ENABLED:
	case AF_DONTNEEDDELAYS:
	case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		res = 1;
		break;
	}

	return res;
}

INT_PTR CAimProto::GetAvatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR* buf = (TCHAR*)wParam;
	int  size = (int)lParam;

	if (buf == NULL || size <= 0)
		return -1;

	PROTO_AVATAR_INFORMATIONT ai = { sizeof(ai) };
	if (GetAvatarInfo(0, (LPARAM)&ai) == GAIR_SUCCESS)
	{
		mir_tstrncpy(buf, ai.filename, size);
		buf[size-1] = 0;
		return 0;
	}

	return -1;
}

INT_PTR CAimProto::SetAvatar(WPARAM, LPARAM lParam)
{
	TCHAR* szFileName = (TCHAR*)lParam;

	if (state != 1) return 1;

	if (szFileName == NULL)
	{
		aim_ssi_update(hServerConn, seqno, true);
		aim_delete_avatar_hash(hServerConn, seqno, 1, 1, avatar_id_sm);
		aim_delete_avatar_hash(hServerConn, seqno, 1, 12, avatar_id_lg);
		aim_ssi_update(hServerConn, seqno, false);

		avatar_request_handler(NULL, NULL, 0);
	}
	else
	{
		char hash[16], hash1[16], *data, *data1 = NULL;
		unsigned short size, size1 = 0;

		if (!get_avatar_hash(szFileName, hash, &data, size))
		{
			mir_free(hash);
			return 1;
		}

		rescale_image(data, size, data1, size1);

		if (size1)
		{
			mir_md5_state_t state;
			mir_md5_init(&state);
			mir_md5_append(&state, (unsigned char*)data1, size1);
			mir_md5_finish(&state, (unsigned char*)hash1);

			mir_free(hash_lg); 	hash_lg = bytes_to_string(hash, sizeof(hash));
			mir_free(hash_sm); 	hash_sm = bytes_to_string(hash1, sizeof(hash1));

			aim_ssi_update(hServerConn, seqno, true);
			aim_set_avatar_hash(hServerConn, seqno, 1, 1, avatar_id_sm, 16, hash1);
			aim_set_avatar_hash(hServerConn, seqno, 1, 12, avatar_id_lg, 16, hash);
			aim_ssi_update(hServerConn, seqno, false);
		}
		else
		{
			mir_free(hash_lg); 	hash_lg = NULL;
			mir_free(hash_sm); 	hash_sm = bytes_to_string(hash, sizeof(hash1));

			aim_ssi_update(hServerConn, seqno, true);
			aim_set_avatar_hash(hServerConn, seqno, 1, 1, avatar_id_sm, 16, hash);
			aim_delete_avatar_hash(hServerConn, seqno, 1, 12, avatar_id_lg);
			aim_ssi_update(hServerConn, seqno, false);
		}

		avatar_request_handler(NULL, NULL, 0);

		avatar_up_req *req = new avatar_up_req(data, size, data1, size1);
		ForkThread(&CAimProto::avatar_upload_thread, req);

		TCHAR tFileName[MAX_PATH];
		TCHAR *ext = _tcsrchr(szFileName, '.');
		get_avatar_filename(NULL, tFileName, SIZEOF(tFileName), ext);
		int fileId = _topen(tFileName, _O_CREAT | _O_TRUNC | _O_WRONLY | O_BINARY, _S_IREAD | _S_IWRITE);
		if (fileId < 0)
		{
			char errmsg[512];
			mir_snprintf(errmsg, SIZEOF(errmsg), "Cannot store avatar. File '%s' could not be created/overwritten", tFileName);
			ShowPopup(errmsg, ERROR_POPUP);
			return 1; 
		}
		_write(fileId, data, size);
		_close(fileId);
	}
	return 0;
}
