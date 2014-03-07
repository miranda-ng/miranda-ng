#include "common.h"

CDropbox::CDropbox()
{
	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = MODULE;
	pd.type = PROTOTYPE_VIRTUAL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	HookEventObj(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown, this);
	HookEventObj(ME_SYSTEM_MODULESLOADED, OnModulesLoaded, this);
	HookEventObj(ME_DB_CONTACT_DELETED, OnContactDeleted, this);
	HookEventObj(ME_OPT_INITIALISE, OnOptionsInitialized, this);
	HookEventObj(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu, this);

	CreateProtoServiceFunction(MODULE, PS_GETCAPS, ProtoGetCaps);
	CreateProtoServiceFunctionObj(PSS_FILE, ProtoSendFile, this);
	CreateProtoServiceFunctionObj(PSS_MESSAGE, ProtoSendMessage, this);
	CreateProtoServiceFunctionObj(PSR_MESSAGE, ProtoReceiveMessage, this);

	InitializeIcons();
	InitializeMenus();

	commands["help"] = CDropbox::CommandHelp;
	commands["content"] = CDropbox::CommandContent;
	commands["share"] = CDropbox::CommandShare;
	commands["delete"] = CDropbox::CommandDelete;

	hFileProcess = hMessageProcess = 1;
	hDefaultContact = hTransferContact = 0;
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

void CDropbox::RequestAcceessToken()
{
	ShellExecuteA(NULL, "open", DROPBOX_WWW_URL DROPBOX_API_VER "/oauth2/authorize?response_type=code&client_id=" DROPBOX_API_KEY, NULL, NULL, SW_SHOWDEFAULT);

	char request_token[128] = { 0 };

	if (DialogBoxParam(
		g_hInstance,
		MAKEINTRESOURCE(IDD_TOKEN_REQUEST),
		NULL,
		CDropbox::TokenRequestProc,
		(LPARAM)&request_token) == IDOK)
	{
		char data[1024];
		mir_snprintf(
			data,
			SIZEOF(data),
			"grant_type=authorization_code&code=%s",
			request_token);

		HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/oauth2/token");
		request->pData = mir_strdup(data);
		request->dataLength = (int)strlen(data);
		request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
		request->AddBasicAuthHeader(DROPBOX_API_KEY, DROPBOX_API_SECRET);

		mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

		delete request;

		MCONTACT hContact = CDropbox::GetDefaultContact();

		if (response)
		{
			JSONNODE *root = json_parse(response->pData);
			if (root)
			{
				if (response->resultCode == HTTP_STATUS_OK)
				{
					JSONNODE *node = json_get(root, "access_token");
					ptrA access_token = ptrA(mir_u2a(json_as_string(node)));
					db_set_s(NULL, MODULE, "TokenSecret", access_token);

					if (hContact)
					{
						if (db_get_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
							db_set_w(hContact, MODULE, "Status", ID_STATUS_ONLINE);
					}

					CDropbox::ShowNotification(TranslateT("You have been authorized"), MB_ICONINFORMATION);
				}
				else
				{
					JSONNODE *node = json_get(root, "error_description");
					ptrW error_description(json_as_string(node));

					CDropbox::ShowNotification((wchar_t*)error_description, MB_ICONERROR);
				}
			}
		}
		else
			HandleFileTransferError(response, hContact);
	}
}

void CDropbox::DestroyAcceessToken()
{
	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/disable_access_token");
	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	MCONTACT hContact = CDropbox::GetDefaultContact();

	db_unset(NULL, MODULE, "TokenSecret");
	if (hContact)
	{
		if (db_get_w(hContact, MODULE, "Status", ID_STATUS_ONLINE) == ID_STATUS_ONLINE)
			db_set_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE);
	}
}

UINT CDropbox::RequestAcceessTokenAsync(void *owner, void* param)
{
	HWND hwndDlg = (HWND)param;
	CDropbox *instance = (CDropbox*)owner;

	EnableWindow(GetDlgItem(hwndDlg, IDC_AUTHORIZE), FALSE);
	SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("in process..."));

	if (instance->HasAccessToken())
		instance->DestroyAcceessToken();

	char requestToken[128];
	GetDlgItemTextA(hwndDlg, IDC_REQUEST_CODE, requestToken, SIZEOF(requestToken));

	char data[1024];
	mir_snprintf(
		data,
		SIZEOF(data),
		"grant_type=authorization_code&code=%s",
		requestToken);

	HttpRequest *request = new HttpRequest(instance->hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/oauth2/token");
	request->AddBasicAuthHeader(DROPBOX_API_KEY, DROPBOX_API_SECRET);
	request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request->pData = mir_strdup(data);
	request->dataLength = (int)strlen(data);

	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	MCONTACT hContact = instance->GetDefaultContact();

	if (response)
	{
		JSONNODE *root = json_parse(response->pData);
		if (root)
		{
			if (response->resultCode == HTTP_STATUS_OK)
			{
				JSONNODE *node = json_get(root, "access_token");
				ptrA access_token = ptrA(mir_u2a(json_as_string(node)));
				db_set_s(NULL, MODULE, "TokenSecret", access_token);

				if (hContact)
				{
					if (db_get_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
						db_set_w(hContact, MODULE, "Status", ID_STATUS_ONLINE);
				}

				if (hwndDlg)
					SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("you have been authorized"));
				else
					ShowNotification(TranslateT("you have been authorized"), MB_ICONINFORMATION);
			}
			else
			{
				JSONNODE *node = json_get(root, "error_description");
				ptrW error_description(json_as_string(node));

				if (hwndDlg)
					SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, error_description);
				else
					ShowNotification((wchar_t*)error_description, MB_ICONERROR);
			}
		}
	}
	else
	{
		if (hwndDlg)
			SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("unknown error"));
		else
		HandleFileTransferError(response, hContact);
	}

	SetDlgItemTextA(hwndDlg, IDC_REQUEST_CODE, "");

	return 0;
}