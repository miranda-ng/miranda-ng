#include "dropbox.h"

CDropbox::CDropbox()
{
	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = MODULE;
	pd.type = PROTOTYPE_VIRTUAL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	HookEvent(ME_OPT_INITIALISE, OnOptionsInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, CDropbox::OnModulesLoaded);

	CreateProtoServiceFunction(MODULE, PS_GETCAPS, CDropbox::GetCaps);
	CreateProtoServiceFunction(MODULE, PSS_FILE, CDropbox::SendFile);
	CreateProtoServiceFunction(MODULE, PSS_MESSAGE, CDropbox::SendMessage);

	InitIcons();
	InitMenus();
}

MCONTACT CDropbox::GetDefaultContact()
{
	return db_find_first(MODULE);
}

bool CDropbox::HasAccessToken()
{
	return db_get_sa(NULL, MODULE, "TokenSecret") != NULL;
}

void CDropbox::RequestAcceessToken()
{
	ShellExecuteA(NULL, "open", DROPBOX_WWW_URL "/" DROPBOX_API_VER "/oauth2/authorize?response_type=code&client_id=" DROPBOX_API_KEY, NULL, NULL, SW_SHOWDEFAULT);

	char request_token[128] = { 0 };
	//request_token[0] = 0;

	if (DialogBoxParam(
		g_hInstance,
		MAKEINTRESOURCE(IDD_TOKEN_REQUEST),
		NULL, 
		CDropbox::TokenRequestProc,
		(LPARAM)&request_token) == IDOK)
	{
		char data[64];
		mir_snprintf(
			data,
			SIZEOF(data),
			"client_id=%s&client_secret=%s",
			DROPBOX_API_KEY,
			DROPBOX_API_SECRET);

		HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/oauth2/token");
		request->AddParameter("grant_type", "authorization_code");
		request->AddParameter("code", request_token);
		request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
		request->pData = mir_strdup(data);
		request->dataLength = strlen(data);

		NETLIBHTTPREQUEST *response = request->Send();

		delete request;

		if (response)
		{
			if (response->resultCode == HttpStatus::OK)
			{
				JSONNODE *root = json_parse(response->pData);
				if (root != NULL)
				{
					JSONNODE *node = json_get(root, "access_token");
					ptrA access_token = ptrA(mir_u2a(json_as_string(node)));
					db_set_s(NULL, MODULE, "TokenSecret", access_token);

					MCONTACT hContact = GetDefaultContact();
					if (hContact)
					{
						node = json_get(root, "uid");
						wchar_t *uid = json_as_string(node);
						db_set_ws(hContact, MODULE, "uid", uid);
						if (db_get_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
							db_set_w(hContact, MODULE, "Status", ID_STATUS_ONLINE);
					}

					CDropbox::ShowNotification(TranslateT("Access request"), TranslateT("Access granted"), MB_ICONINFORMATION);

					delete node;
					delete root;
				}
			}
			else
			{
				JSONNODE *root = json_parse(response->pData);
				if (root != NULL)
				{
					JSONNODE *node = json_get(root, "error_description");
					wchar_t *error_description = json_as_string(node);

					CDropbox::ShowNotification(TranslateT("Access request"), error_description, MB_ICONERROR);

					delete node;
					delete root;
				}
			}

			mir_free(response);
		}
	}
}

void CDropbox::DestroyAcceessToken()
{

	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/disable_access_token");
	//request->SendAsync(&CDropboxProto::AsyncFunc);
	NETLIBHTTPREQUEST *response = request->Send();

	delete request;
	mir_free(response);

	db_unset(NULL, MODULE, "TokenSecret");
	MCONTACT hContact = GetDefaultContact();
	if (hContact)
	{
		if (db_get_w(hContact, MODULE, "Status", ID_STATUS_ONLINE) == ID_STATUS_ONLINE)
			db_set_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE);
	}
}