#include "stdafx.h"

CDropbox::CDropbox()
{
	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = MODULE;
	pd.type = PROTOTYPE_VIRTUAL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	HookEventObj(ME_PROTO_ACK, OnProtoAck, this);
	HookEventObj(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown, this);
	HookEventObj(ME_SYSTEM_MODULESLOADED, OnModulesLoaded, this);

	hFileSentEventHook = CreateHookableEvent(ME_DROPBOX_SENT);

	CreateServiceFunctionObj(MS_DROPBOX_SEND_FILE, SendFileToDropbox, this);

	CreateProtoServiceFunction(MODULE, PS_GETCAPS, ProtoGetCaps);
	CreateProtoServiceFunction(MODULE, PS_GETNAME, ProtoGetName);
	CreateProtoServiceFunction(MODULE, PS_LOADICON, ProtoLoadIcon);
	CreateProtoServiceFunction(MODULE, PS_GETSTATUS, ProtoGetStatus);
	CreateProtoServiceFunctionObj(PS_SETSTATUS, ProtoSetStatus, this);
	CreateProtoServiceFunctionObj(PSS_FILEW, ProtoSendFile, this);
	CreateProtoServiceFunctionObj(PSS_MESSAGE, ProtoSendMessage, this);
	CreateProtoServiceFunctionObj(PSR_MESSAGE, ProtoReceiveMessage, this);

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

			int status = db_get_w(hDefaultContact, MODULE, "Status", ID_STATUS_OFFLINE);
			if (HasAccessToken() && status == ID_STATUS_OFFLINE)
				db_set_w(hDefaultContact, MODULE, "Status", ID_STATUS_ONLINE);
		}
	}

	return hDefaultContact;
}

bool CDropbox::HasAccessToken()
{
	return db_get_sa(NULL, MODULE, "TokenSecret") != NULL;
}

void CDropbox::RequestAccountInfo()
{
	HttpRequest *request = new HttpRequest(hNetlibConnection, REQUEST_GET, DROPBOX_API_URL "/account/info");
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	MCONTACT hContact = CDropbox::GetDefaultContact();

	HandleHttpResponseError(response);

	JSONROOT root(response->pData);
	if (root)
	{
		JSONNODE *node = json_get(root, "referral_link");
		if (node)
		{
			ptrW referral_link = ptrW(json_as_string(node));
			db_set_ws(hContact, MODULE, "Homepage", referral_link);
		}

		node = json_get(root, "display_name");
		if (node)
		{
			ptrW display_name = ptrW(json_as_string(node));
			TCHAR *sep = wcsrchr(display_name, L' ');
			if (sep)
			{
				db_set_ws(hContact, MODULE, "LastName", sep + 1);
				display_name[wcslen(display_name) - wcslen(sep)] = '\0';
				db_set_ws(hContact, MODULE, "FirstName", display_name);
			}
			else
			{
				db_set_ws(hContact, MODULE, "FirstName", display_name);
				db_unset(hContact, MODULE, "LastName");
			}
		}

		node = json_get(root, "country");
		if (node)
		{
			ptrW isocodeW(json_as_string(node));
			ptrA isocode(mir_u2a(isocodeW));

			if (!strlen(isocode))
				db_unset(hContact, MODULE, "Country");
			else
			{
				char *country = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)isocode, 0);
				db_set_s(hContact, MODULE, "Country", country);
			}
		}

		node = json_get(root, "quota_info");
		JSONNODE *nroot = json_as_node(node);
		if (nroot)
		{
			node = json_get(nroot, "shared");
			if (node)
				db_set_dw(hContact, MODULE, "SharedQuota", json_as_int(node));
			node = json_get(nroot, "normal");
			if (node)
				db_set_dw(hContact, MODULE, "NormalQuota", json_as_int(node));
			node = json_get(nroot, "quota");
			if (node)
				db_set_dw(hContact, MODULE, "TotalQuota", json_as_int(node));
		}
	}
}

void CDropbox::DestroyAccessToken()
{
	HttpRequest *request = new HttpRequest(hNetlibConnection, REQUEST_POST, DROPBOX_API_URL "/disable_access_token");
	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	MCONTACT hContact = CDropbox::GetDefaultContact();

	db_unset(NULL, MODULE, "TokenSecret");
	if (hContact)
		if (db_get_w(hContact, MODULE, "Status", ID_STATUS_ONLINE) == ID_STATUS_ONLINE)
			db_set_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE);
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

	char data[1024];
	mir_snprintf(
		data,
		SIZEOF(data),
		"grant_type=authorization_code&code=%s",
		requestToken);

	HttpRequest *request = new HttpRequest(instance->hNetlibConnection, REQUEST_POST, DROPBOX_API_URL "/oauth2/token");
	request->AddBasicAuthHeader(DROPBOX_API_KEY, DROPBOX_API_SECRET);
	request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request->pData = mir_strdup(data);
	request->dataLength = (int)strlen(data);

	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	MCONTACT hContact = instance->GetDefaultContact();

	if (response == NULL)
	{
		if (hwndDlg)
			SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("server does not respond"));
		/*else
			ShowNotification(TranslateT("server does not respond"), MB_ICONERROR);*/
	}
	JSONROOT root(response->pData);
	if (root)
	{
		if (response->resultCode == HTTP_STATUS_OK)
		{
			JSONNODE *node = json_get(root, "access_token");
			ptrA access_token = ptrA(mir_u2a(json_as_string(node)));
			db_set_s(NULL, MODULE, "TokenSecret", access_token);

			if (hContact) {
				if (db_get_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
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
		}
		else
		{
			JSONNODE *node = json_get(root, "error_description");
			ptrW error_description(json_as_string(node));

			if (hwndDlg)
				SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, error_description);
			/*else
				ShowNotification((TCHAR*)error_description, MB_ICONERROR);*/
		}
	}

	SetDlgItemTextA(hwndDlg, IDC_REQUEST_CODE, "");

	return 0;
}
