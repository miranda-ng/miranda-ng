#include "common.h"

std::map<HWND, MCONTACT> CDropbox::dcftp;
HGENMENU CDropbox::ContactMenuItems[CMI_MAX];

void CDropbox::Init()
{
	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = MODULE;
	pd.type = PROTOTYPE_VIRTUAL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	HookEvent(ME_OPT_INITIALISE, OnOptionsInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, CDropbox::OnModulesLoaded);
	HookEvent(ME_DB_CONTACT_DELETED, CDropbox::OnContactDeleted);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, CDropbox::OnPrebuildContactMenu);

	CreateProtoServiceFunction(MODULE, PS_GETCAPS, CDropbox::ProtoGetCaps);
	CreateProtoServiceFunction(MODULE, PSS_FILE, CDropbox::ProtoSendFile);
	CreateProtoServiceFunction(MODULE, PSS_MESSAGE, CDropbox::ProtoSendMessage);

	InitIcons();
	InitMenus();

	INSTANCE->hContactTransfer = 0;
}

MCONTACT CDropbox::hContactDefault = 0;

MCONTACT CDropbox::GetDefaultContact()
{
	if (!hContactDefault)
		hContactDefault = db_find_first(MODULE);

	return hContactDefault;
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
				if (response->resultCode == HTTP_STATUS::OK)
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

void CDropbox::RequestApiAuthorizationAsync(void *arg)
{
	if (HasAccessToken() && MessageBox(
		NULL, 
		TranslateT("Are you sure you want to request athorization?"), 
		TranslateT("Request authorization"), 
		MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		INSTANCE->DestroyAcceessToken();
		INSTANCE->RequestAcceessToken();
	}
	else
		INSTANCE->RequestAcceessToken();
}

void CDropbox::RevokeApiAuthorizationAsync(void *arg)
{
	if (HasAccessToken() && MessageBox(
		NULL,
		TranslateT("Are you sure you want to revoke athorization?"),
		TranslateT("Revoke authorization"),
		MB_YESNO | MB_ICONQUESTION) == IDYES)
		INSTANCE->DestroyAcceessToken();
}