#include "common.h"

int CToxProto::OnAccountLoaded(WPARAM, LPARAM)
{
	HookEventObj(ME_OPT_INITIALISE, OnOptionsInit, this);
	HookEventObj(ME_PROTO_ACCLISTCHANGED, OnAccountListChanged, this);

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

int CToxProto::OnAccountListChanged(void *obj, WPARAM wParam, LPARAM lParam)
{
	CToxProto *proto = (CToxProto*)obj;
	PROTOACCOUNT* account = (PROTOACCOUNT*)lParam;

	if (wParam == PRAC_ADDED && !strcmp(account->szModuleName, proto->m_szModuleName))
	{
		proto->UninitToxCore();
		DialogBoxParam(
			g_hInstance,
			MAKEINTRESOURCE(IDD_PROFILE_MANAGER),
			account->hwndAccMgrUI,
			CToxProto::ToxProfileManagerProc,
			(LPARAM)proto);
		proto->InitToxCore();
	}

	return 0;
}

int CToxProto::OnOptionsInit(void *obj, WPARAM wParam, LPARAM)
{
	CToxProto *proto = (CToxProto*)obj;

	char *title = mir_t2a(proto->m_tszUserName);

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInstance;
	odp.pszTitle = title;
	odp.dwInitParam = LPARAM(obj);
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

int CToxProto::OnSettingsChanged(MCONTACT hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING* dbcws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact == NULL && !strcmp(dbcws->szModule, m_szModuleName) && !strcmp(dbcws->szSetting, "Nick"))
	{
		if (tox_set_name(tox, (uint8_t*)(char*)ptrA(mir_utf8encodeW(dbcws->value.ptszVal)), _tcslen(dbcws->value.ptszVal)))
		{
			SaveToxData();
		}
	}

	return 0;
}

void CToxProto::OnFriendRequest(Tox *tox, const uint8_t *userId, const uint8_t *message, const uint16_t messageSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	std::vector<uint8_t> clientId(userId, userId + TOX_CLIENT_ID_SIZE);
	std::string toxId = proto->DataToHexString(clientId);

	proto->RaiseAuthRequestEvent(time(NULL), toxId.c_str(), (char*)message);
	
	proto->SaveToxData();
}

void CToxProto::OnFriendMessage(Tox *tox, const int friendnumber, const uint8_t *message, const uint16_t messageSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	std::vector<uint8_t> clientId(TOX_CLIENT_ID_SIZE);
	tox_get_client_id(tox, friendnumber, &clientId[0]);
	std::string toxId = proto->DataToHexString(clientId);

	MCONTACT hContact = proto->FindContact(toxId.c_str());
	if (hContact)
	{
		PROTORECVEVENT recv = { 0 };
		recv.flags = PREF_UTF;
		recv.timestamp = time(NULL);
		recv.szMessage = (char*)message;

		ProtoChainRecvMsg(hContact, &recv);
	}
}

void CToxProto::OnFriendNameChange(Tox *tox, const int friendnumber, const uint8_t *name, const uint16_t nameSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	std::vector<uint8_t> clientId(TOX_CLIENT_ID_SIZE);
	tox_get_client_id(tox, friendnumber, &clientId[0]);
	std::string toxId = proto->DataToHexString(clientId);

	MCONTACT hContact = proto->FindContact(toxId.c_str());
	if (hContact)
	{
		proto->setString(hContact, "Nick", (char*)name);
	}
}

void CToxProto::OnStatusMessageChanged(Tox *tox, const int friendnumber, const uint8_t* message, const uint16_t messageSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	std::vector<uint8_t> clientId(TOX_CLIENT_ID_SIZE);
	tox_get_client_id(tox, friendnumber, &clientId[0]);
	std::string toxId = proto->DataToHexString(clientId);

	MCONTACT hContact = proto->FindContact(toxId.c_str());
	if (hContact)
	{
		db_set_s(hContact, "CList", "StatusMsg", (char*)message);
	}
}

void CToxProto::OnUserStatusChanged(Tox *tox, int32_t friendnumber, uint8_t usertatus, void *arg)
{
	TOX_USERSTATUS userstatus = (TOX_USERSTATUS)usertatus;
	if (userstatus == TOX_USERSTATUS::TOX_USERSTATUS_NONE)
	{
		return;
	}

	CToxProto *proto = (CToxProto*)arg;

	std::vector<uint8_t> clientId(TOX_CLIENT_ID_SIZE);
	tox_get_client_id(tox, friendnumber, &clientId[0]);
	std::string toxId = proto->DataToHexString(clientId);

	MCONTACT hContact = proto->FindContact(toxId.c_str());
	if (hContact)
	{
		int status = proto->ToxToMirandaStatus(userstatus);
		proto->SetContactStatus(hContact, status);
	}
}

void CToxProto::OnConnectionStatusChanged(Tox *tox, const int friendnumber, const uint8_t status, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	std::vector<uint8_t> clientId(TOX_CLIENT_ID_SIZE);
	tox_get_client_id(tox, friendnumber, &clientId[0]);
	std::string toxId = proto->DataToHexString(clientId);

	MCONTACT hContact = proto->FindContact(toxId.c_str());
	if (hContact)
	{
		int newStatus = status ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
		proto->SetContactStatus(hContact, newStatus);
	}
}

void CToxProto::OnAction(Tox *tox, const int friendnumber, const uint8_t *message, const uint16_t messageSize, void *arg)
{

}

void CToxProto::OnReadReceipt(Tox *tox, int32_t friendnumber, uint32_t receipt, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	std::vector<uint8_t> clientId(TOX_CLIENT_ID_SIZE);
	tox_get_client_id(tox, friendnumber, &clientId[0]);
	std::string toxId = proto->DataToHexString(clientId);

	MCONTACT hContact = proto->FindContact(toxId.c_str());

	proto->ProtoBroadcastAck(
		hContact,
		ACKTYPE_MESSAGE,
		ACKRESULT_SUCCESS,
		(HANDLE)receipt, 0);
}