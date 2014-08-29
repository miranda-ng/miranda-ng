#include "common.h"

int CToxProto::OnAccountLoaded(WPARAM, LPARAM)
{
	HookProtoEvent(ME_OPT_INITIALISE, &CToxProto::OnOptionsInit);
	HookProtoEvent(ME_PROTO_ACCLISTCHANGED, &CToxProto::OnAccountListChanged);
	HookProtoEvent(ME_DB_CONTACT_SETTINGCHANGED, &CToxProto::OnSettingsChanged);
	HookProtoEvent(ME_MSG_PRECREATEEVENT, &CToxProto::OnPreCreateMessage);

	InitNetlib();

	return 0;
}

int CToxProto::OnPreShutdown(WPARAM, LPARAM)
{
	UninitNetlib();

	return 0;
}

INT_PTR CToxProto::OnAccountManagerInit(WPARAM, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(
		g_hInstance,
		MAKEINTRESOURCE(IDD_ACCOUNT_MANAGER),
		(HWND)lParam,
		CToxProto::MainOptionsProc,
		(LPARAM)this);
}

int CToxProto::OnAccountListChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT* account = (PROTOACCOUNT*)lParam;

	if (wParam == PRAC_ADDED && !strcmp(account->szModuleName, m_szModuleName))
	{
		UninitToxCore();
		DialogBoxParam(
			g_hInstance,
			MAKEINTRESOURCE(IDD_PROFILE_MANAGER),
			account->hwndAccMgrUI,
			CToxProto::ToxProfileManagerProc,
			(LPARAM)this);
		InitToxCore();
	}

	return 0;
}

int CToxProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	char *title = mir_t2a(m_tszUserName);

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInstance;
	odp.pszTitle = title;
	odp.dwInitParam = (LPARAM)this;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszGroup = LPGEN("Network");

	odp.pszTab = LPGEN("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_MAIN);
	odp.pfnDlgProc = MainOptionsProc;
	Options_AddPage(wParam, &odp);

	mir_free(title);

	return 0;
}

int CToxProto::OnContactDeleted(MCONTACT hContact, LPARAM lParam)
{
	if (hContact)
	{
		std::string toxId(getStringA(hContact, TOX_SETTINGS_ID));
		std::vector<uint8_t> clientId = HexStringToData(toxId);

		uint32_t number = tox_get_friend_number(tox, clientId.data());
		if (tox_del_friend(tox, number) == 0)
		{
			SaveToxData();

			return 0;
		}
	}

	return 1;
}

int CToxProto::OnSettingsChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING* dbcws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact == NULL && !strcmp(dbcws->szModule, m_szModuleName))
	{
		if (!strcmp(dbcws->szSetting, "Nick") && dbcws->value.pszVal)
		{
			if (tox_set_name(tox, (uint8_t*)dbcws->value.pszVal, (uint16_t)strlen(dbcws->value.pszVal)))
			{
				SaveToxData();
			}
		}

		/*if (!strcmp(dbcws->szSetting, "StatusMsg") || !strcmp(dbcws->szSetting, "StatusNote"))
		{
		if (tox_set_status_message(tox, (uint8_t*)(char*)ptrA(mir_utf8encodeW(dbcws->value.ptszVal)), (uint16_t)_tcslen(dbcws->value.ptszVal)))
		{
		SaveToxData();
		}
		}*/
	}

	return 0;
}

int CToxProto::OnPreCreateMessage(WPARAM wParam, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent *)lParam;
	if (strcmp(GetContactProto(evt->hContact), m_szModuleName))
	{
		return 0;
	}

	char *message = (char*)evt->dbei->pBlob;
	if (strncmp(message, "/me ", 4) == 0)
	{
		BYTE *action = (BYTE*)mir_alloc(sizeof(BYTE)* (evt->dbei->cbBlob - 4));
		memcpy(action, (char*)&evt->dbei->pBlob[4], evt->dbei->cbBlob - 4);
		mir_free(evt->dbei->pBlob);
		evt->dbei->pBlob = action;
		evt->dbei->cbBlob -= 4;

		evt->dbei->eventType = TOX_DB_EVENT_TYPE_ACTION;
	}

	return 1;
}

void CToxProto::OnFriendRequest(Tox *tox, const uint8_t *address, const uint8_t *message, const uint16_t messageSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	// trim tox address to tox id
	std::vector<uint8_t> clientId(address, address + TOX_CLIENT_ID_SIZE);
	std::string id = proto->DataToHexString(clientId);

	proto->RaiseAuthRequestEvent(time(NULL), id.c_str(), (char*)message);

	proto->SaveToxData();
}

void CToxProto::OnFriendMessage(Tox *tox, const int number, const uint8_t *message, const uint16_t messageSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		PROTORECVEVENT recv = { 0 };
		recv.flags = PREF_UTF;
		recv.timestamp = time(NULL);
		recv.szMessage = (char*)message;

		ProtoChainRecvMsg(hContact, &recv);
	}
}

void CToxProto::OnFriendAction(Tox *tox, const int number, const uint8_t *action, const uint16_t actionSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		proto->AddDbEvent(
			hContact,
			TOX_DB_EVENT_TYPE_ACTION,
			time(NULL),
			DBEF_UTF,
			actionSize,
			(BYTE*)action);
	}
}

void CToxProto::OnTypingChanged(Tox *tox, const int number, uint8_t isTyping, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)isTyping);
	}
}

void CToxProto::OnFriendNameChange(Tox *tox, const int number, const uint8_t *name, const uint16_t nameSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		proto->setString(hContact, "Nick", (char*)name);
	}
}

void CToxProto::OnStatusMessageChanged(Tox *tox, const int number, const uint8_t* message, const uint16_t messageSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		ptrW statusMessage(mir_utf8decodeW((char*)message));
		db_set_ws(hContact, "CList", "StatusMsg", statusMessage);
	}
}

void CToxProto::OnUserStatusChanged(Tox *tox, int32_t number, uint8_t usertatus, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		TOX_USERSTATUS userstatus = (TOX_USERSTATUS)usertatus;
		int status = proto->ToxToMirandaStatus(userstatus);
		proto->SetContactStatus(hContact, status);
	}
}

void CToxProto::OnConnectionStatusChanged(Tox *tox, const int number, const uint8_t status, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		int newStatus = status ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
		proto->SetContactStatus(hContact, newStatus);
	}
}

void CToxProto::OnReadReceipt(Tox *tox, int32_t number, uint32_t receipt, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		proto->ProtoBroadcastAck(
			hContact,
			ACKTYPE_MESSAGE,
			ACKRESULT_SUCCESS,
			(HANDLE)receipt, 0);
	}
}

void CToxProto::OnFriendFile(Tox *tox, int32_t number, uint8_t fileNumber, uint64_t fileSize, const uint8_t *fileName, uint16_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		FileTransferParam *transfer = new FileTransferParam(fileNumber, ptrT(mir_utf8decodeT((const char*)fileName)), fileSize);
		transfer->pfts.hContact = hContact;
		transfer->pfts.flags |= PFTS_RECEIVING;
		proto->transfers[fileNumber] = transfer;

		PROTORECVFILET pre = { 0 };
		pre.flags = PREF_TCHAR;
		pre.fileCount = 1;
		pre.timestamp = time(NULL);
		pre.tszDescription = _T("");
		pre.ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*)* 2);
		pre.ptszFiles[0] = mir_utf8decodeT((char*)fileName);
		pre.ptszFiles[1] = NULL;
		pre.lParam = (LPARAM)fileNumber;
		ProtoChainRecvFile(hContact, &pre);
	}
}

void CToxProto::OnFileData(Tox *tox, int32_t number, uint8_t fileNumber, const uint8_t *data, uint16_t size, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		FileTransferParam *transfer = proto->transfers.at(fileNumber);

		TCHAR filePath[MAX_PATH];
		mir_sntprintf(filePath, SIZEOF(filePath), _T("%s%s"), transfer->pfts.tszWorkingDir, transfer->pfts.tszCurrentFile);

		FILE *hFile = NULL;
		if (transfer->pfts.currentFileProgress == 0)
		{
			hFile = _tfopen(filePath, _T("wb"));
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)fileNumber, 0);
		}
		else
		{
			hFile = _tfopen(filePath, _T("ab"));
		}
		if (hFile != NULL)
		{
			if (fwrite(data, sizeof(uint8_t), size, hFile) == size)
			{
				transfer->pfts.totalProgress = transfer->pfts.currentFileProgress += size;
				proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)fileNumber, (LPARAM)&transfer->pfts);
			}
			fclose(hFile);
		}
	}
}

void CToxProto::OnFileRequest(Tox *tox, int32_t number, uint8_t isSend, uint8_t fileNumber, uint8_t type, const uint8_t *data, uint16_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		FileTransferParam *transfer = proto->transfers.at(fileNumber);

		switch (type)
		{
		case TOX_FILECONTROL_ACCEPT:
			break;

		case TOX_FILECONTROL_FINISHED:
			tox_file_send_control(proto->tox, number, 1, fileNumber, TOX_FILECONTROL_FINISHED, NULL, 0);
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)fileNumber, 0);
			break;
		}
	}
}