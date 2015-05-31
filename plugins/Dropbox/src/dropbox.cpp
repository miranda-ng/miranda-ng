#include "stdafx.h"

CDropbox::CDropbox() : transfers(1, HandleKeySortT)
{
	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = MODULE;
	pd.type = PROTOTYPE_VIRTUAL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	HookEvent(ME_PROTO_ACK, OnProtoAck);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	HookEventObj(ME_SYSTEM_MODULESLOADED, GlobalEvent<&CDropbox::OnModulesLoaded>, this);

	hFileSentEventHook = CreateHookableEvent(ME_DROPBOX_SENT);

	CreateServiceFunctionObj(MS_DROPBOX_SEND_FILE, GlobalService<&CDropbox::SendFileToDropbox>, this);

	CreateProtoServiceFunction(MODULE, PS_GETCAPS, ProtoGetCaps);
	CreateProtoServiceFunction(MODULE, PS_GETNAME, ProtoGetName);
	CreateProtoServiceFunction(MODULE, PS_LOADICON, ProtoLoadIcon);
	CreateProtoServiceFunctionObj(PS_GETSTATUS, GlobalService<&CDropbox::ProtoGetStatus>, this);
	CreateProtoServiceFunctionObj(PSS_FILE, GlobalService<&CDropbox::ProtoSendFile>, this);
	CreateProtoServiceFunctionObj(PSS_FILECANCEL, GlobalService<&CDropbox::ProtoCancelFile>, this);
	CreateProtoServiceFunctionObj(PSS_MESSAGE, GlobalService<&CDropbox::ProtoSendMessage>, this);
	CreateProtoServiceFunction(MODULE, PSR_MESSAGE, ProtoReceiveMessage);

	InitializeMenus();

	hFileProcess = hMessageProcess = 1;
}

CDropbox::~CDropbox()
{
	DestroyHookableEvent(hFileSentEventHook);
}

MCONTACT CDropbox::GetDefaultContact()
{
	if (!hDefaultContact)
		hDefaultContact = db_find_first(MODULE);

	if (!hDefaultContact)
	{
		hDefaultContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		if (!CallService(MS_PROTO_ADDTOCONTACT, hDefaultContact, (LPARAM)MODULE))
		{
			db_set_s(NULL, MODULE, "Nick", MODULE);
			db_set_s(hDefaultContact, MODULE, "Nick", MODULE);
			db_set_ws(hDefaultContact, "CList", "MyHandle", L"Dropbox");
		}
		db_set_w(hDefaultContact, MODULE, "Status", HasAccessToken() ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);
	}

	return hDefaultContact;
}

bool CDropbox::HasAccessToken()
{
	return db_get_sa(NULL, MODULE, "TokenSecret") != NULL;
}

void CDropbox::RequestAccountInfo()
{
	MCONTACT hContact = CDropbox::GetDefaultContact();

	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	GetAccountInfoRequest request(token);
	NetlibPtr response(request.Send(hNetlibConnection));
	HandleHttpResponseError(response);

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty())
		return;

	JSONNode referral_link = root.at("referral_link");
	if (!referral_link.empty())
		db_set_s(hContact, MODULE, "Homepage", referral_link.as_string().c_str());

	JSONNode display_name = root.at("display_name");
	if (!display_name.empty())
	{
		ptrT display_name(mir_utf8decodeT(display_name.as_string().c_str()));
		TCHAR *sep = _tcsrchr(display_name, _T(' '));
		if (sep)
		{
			db_set_ts(hContact, MODULE, "LastName", sep + 1);
			display_name[mir_tstrlen(display_name) - mir_tstrlen(sep)] = '\0';
			db_set_ts(hContact, MODULE, "FirstName", display_name);
		}
		else
		{
			db_set_ts(hContact, MODULE, "FirstName", display_name);
			db_unset(hContact, MODULE, "LastName");
		}
	}

	JSONNode country = root.at("country");
	if (!country.empty())
	{
		std::string isocode = country.as_string();

		if (isocode.empty())
			db_unset(hContact, MODULE, "Country");
		else
		{
			char *country = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)isocode.c_str(), 0);
			db_set_s(hContact, MODULE, "Country", country);
		}
	}

	JSONNode quota_info = root.at("quota_info");
	if (!quota_info.empty())
	{
		db_set_dw(hContact, MODULE, "SharedQuota", quota_info.at("shared").as_int());
		db_set_dw(hContact, MODULE, "NormalQuota", quota_info.at("normal").as_int());
		db_set_dw(hContact, MODULE, "TotalQuota", quota_info.at("quota").as_int());
	}
}

void CDropbox::DestroyAccessToken()
{
	DisableAccessTokenRequest request;
	NetlibPtr response(request.Send(hNetlibConnection));

	db_unset(NULL, MODULE, "TokenSecret");
	MCONTACT hContact = CDropbox::GetDefaultContact();
	if (hContact)
		if (db_get_w(hContact, MODULE, "Status", ID_STATUS_ONLINE) != ID_STATUS_OFFLINE)
			db_set_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE);

	ProtoBroadcastAck(MODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_ONLINE, (WPARAM)ID_STATUS_OFFLINE);
}

UINT CDropbox::RequestAccessTokenAsync(void *owner, void *param)
{
	HWND hwndDlg = (HWND)param;
	CDropbox *instance = (CDropbox*)owner;

	EnableWindow(GetDlgItem(hwndDlg, IDC_AUTHORIZE), FALSE);
	SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("in process..."));

	if (instance->HasAccessToken())
		instance->DestroyAccessToken();

	char requestToken[128];
	GetDlgItemTextA(hwndDlg, IDC_REQUEST_CODE, requestToken, SIZEOF(requestToken));

	GetAccessTokenRequest request(requestToken);
	NetlibPtr response(request.Send(instance->hNetlibConnection));

	if (response == NULL)
	{
		Netlib_Logf(instance->hNetlibConnection, "%s: %s", MODULE, HttpStatusToText(HTTP_STATUS_ERROR));
		if (hwndDlg)
			SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("server does not respond"));
		/*else
			ShowNotification(TranslateT("server does not respond"), MB_ICONERROR);*/
		return 0;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty())
	{
		Netlib_Logf(instance->hNetlibConnection, "%s: %s", MODULE, HttpStatusToText((HTTP_STATUS)response->resultCode));
		if (hwndDlg)
			SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("server does not respond"));
		/*else
			ShowNotification((TCHAR*)error_description, MB_ICONERROR);*/
		return 0;
	}

	JSONNode node = root.at("error_description");
	if (node != JSONNULL)
	{
		ptrT error_description(mir_a2t_cp(node.as_string().c_str(), CP_UTF8));
		Netlib_Logf(instance->hNetlibConnection, "%s: %s", MODULE, HttpStatusToText((HTTP_STATUS)response->resultCode));
		if (hwndDlg)
			SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, error_description);
		/*else
			ShowNotification((TCHAR*)error_description, MB_ICONERROR);*/
		return 0;
	}

	node = root.at("access_token");
	db_set_s(NULL, MODULE, "TokenSecret", node.as_string().c_str());
	ProtoBroadcastAck(MODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_OFFLINE, (WPARAM)ID_STATUS_ONLINE);

	MCONTACT hContact = instance->GetDefaultContact();
	if (hContact)
	{
		if (db_get_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE) != ID_STATUS_ONLINE)
			db_set_w(hContact, MODULE, "Status", ID_STATUS_ONLINE);
	}

	try
	{
		instance->RequestAccountInfo();
	}
	catch (TransferException &ex)
	{
		Netlib_Logf(instance->hNetlibConnection, "%s: %s", MODULE, ex.what());
		return 0;
	}

	if (hwndDlg)
		SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("you have been authorized"));
	/*else
		ShowNotification(TranslateT("you have been authorized"), MB_ICONINFORMATION);*/

	SetDlgItemTextA(hwndDlg, IDC_REQUEST_CODE, "");

	return 0;
}
