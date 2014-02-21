#include "common.h"

void CDropbox::Init()
{
	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = MODULE;
	pd.type = PROTOTYPE_VIRTUAL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	HookEvent(ME_OPT_INITIALISE, OnOptionsInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, CDropbox::OnModulesLoaded);

	CreateProtoServiceFunction(MODULE, PS_GETCAPS, CDropbox::ProtoGetCaps);
	CreateProtoServiceFunction(MODULE, PSS_FILE, CDropbox::ProtoSendFile);
	CreateProtoServiceFunction(MODULE, PSS_MESSAGE, CDropbox::ProtoSendMessage);

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

void CDropbox::RequestAcceessToken(MCONTACT hContact)
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
		HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/oauth2/token");
		request->AddParameter("grant_type", "authorization_code");
		request->AddParameter("code", request_token);
		request->AddBasicAuthHeader(DROPBOX_API_KEY, DROPBOX_API_SECRET);

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

void CDropbox::DestroyAcceessToken(MCONTACT hContact)
{
	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/disable_access_token");
	NETLIBHTTPREQUEST *response = request->Send();

	delete request;
	mir_free(response);

	db_unset(NULL, MODULE, "TokenSecret");
	if (hContact)
	{
		if (db_get_w(hContact, MODULE, "Status", ID_STATUS_ONLINE) == ID_STATUS_ONLINE)
			db_set_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE);
	}
}

void CDropbox::RequestApiAuthorizationAsync(void *arg)
{
	MCONTACT hContact = (MCONTACT)arg;

	if (HasAccessToken() && MessageBox(
		NULL, 
		TranslateT("Are you sure you want to request athorization?"), 
		TranslateT("Request athorization"), 
		MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		Singleton<CDropbox>::GetInstance()->DestroyAcceessToken(hContact);
		Singleton<CDropbox>::GetInstance()->RequestAcceessToken(hContact);
	}
	else
		Singleton<CDropbox>::GetInstance()->RequestAcceessToken(hContact);
}