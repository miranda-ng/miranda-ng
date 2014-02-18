#include "dropBox_proto.h"

HANDLE g_hNetlibUser;
ULONG  g_fileId = 1;

int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = MODULE;
	nlu.szSettingsModule = MODULE;
	nlu.ptszDescriptiveName = L"DropBox";

	g_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	DropBoxLogIn();

	return 0;
}

int OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 100000000;
	odp.hInstance = g_hInstance;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_MAIN);
	odp.pszGroup = LPGEN("Network");
	odp.pszTitle = LPGEN("DropBox");
	odp.pfnDlgProc = MainOptionsProc;

	//Options_AddPage(wParam, &odp);

	return 0;
}

INT_PTR DropBoxGetCaps(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILESEND;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR) "Dropbox ID";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"uid";
	}

	return 0;
}

HttpRequest *DropBoxCreateFileChunkedRequest(const char *data, int length)
{
	HttpRequest *request = new HttpRequest(g_hNetlibUser, REQUEST_PUT, DROPBOX_APICONTENT_URL "/chunked_upload");
	request->AddParameter("access_token", db_get_sa(NULL, MODULE, "TokenSecret"));
	if (length > 0)
	{
		//request->AddHeader("Content-Type", "application/octet-stream");
		request->dataLength = length;
		request->pData = (char*)mir_alloc(sizeof(char) * (length + 1));
		memcpy(request->pData, data, length);
		request->pData[length] = 0;
	}

	return request;
}

bool DropBoxSendFileChunkedStart(const char *data, int length, char *uploadId, int &offset)
{
	HttpRequest *request = DropBoxCreateFileChunkedRequest(data, length);
	NETLIBHTTPREQUEST *response = request->Send();

	delete request;

	if (response && response->resultCode == 200)
	{
		JSONNODE *root = json_parse(response->pData);
		if (root != NULL)
		{
			JSONNODE *node = json_get(root, "upload_id");
			strcpy(uploadId, mir_u2a(json_as_string(node)));

			node = json_get(root, "offset");
			offset = json_as_int(node);

			return true;
		}
	}

	return 0;
}

bool DropBoxSendFileChunkedNext(const char *data, int length, const char *uploadId, int &offset)
{
	HttpRequest *request = DropBoxCreateFileChunkedRequest(data, length);
	request->AddParameter("upload_id", uploadId);
	request->AddParameter("offset", offset);

	NETLIBHTTPREQUEST *response = request->Send();

	delete request;

	if (response && response->resultCode == 200)
	{
		JSONNODE *root = json_parse(response->pData);
		if (root != NULL)
		{
			JSONNODE *node = json_get(root, "offset");
			offset = json_as_int(node);

			return true;
		}
	}

	return false;
}

bool DropBoxSendFileChunkedEnd(const char *fileName, const char *uploadId, MCONTACT hContact)
{
	char url[MAX_PATH];
	mir_snprintf(
		url,
		SIZEOF(url),
		"%s/commit_chunked_upload/sandbox/%s",
		DROPBOX_APICONTENT_URL,
		fileName);

	HttpRequest *request = new HttpRequest(g_hNetlibUser, REQUEST_POST, url);
	request->AddParameter("upload_id", uploadId);
	request->AddParameter("access_token", db_get_sa(NULL, MODULE, "TokenSecret"));

	NETLIBHTTPREQUEST *response = request->Send();

	delete request;

	if (response && response->resultCode == 200)
	{
		//char message[MAX_PATH];
		//mir_snprintf(
		//	message,
		//	SIZEOF(message),
		//	"%s/files/sandbox/%s",
		//	DROPBOX_APICONTENT_URL,
		//	fileName);

		//PROTORECVEVENT recv = { 0 };
		////recv.flags = flags;
		////recv.lParam = (LPARAM)&param;
		//recv.timestamp = time(NULL);
		//recv.szMessage = ::mir_strdup(message);
		//::ProtoChainRecvMsg(hContact, &recv);

		return true;
	}

	return false;
}

void DropBoxAsyncFileSend(void *args)
{
}

INT_PTR DropBoxSendFile(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *pccsd=(CCSDATA*)lParam;

	char **files = (char**)pccsd->lParam;
	for (int i = 0; files[i]; i++)
	{
		FILE *file = fopen(files[i], "rb");
		if (file != NULL)
		{
			int offset = 0;
			bool isFirstChunk = true;
			char *uploadId = new char[32];

			const char *fileName = strrchr(files[i], '\\') + 1;

			while (!feof(file) && !ferror(file))
			{
				char *data = new char[DROPBOX_FILE_CHUNK_SIZE + 1];
				size_t count = fread(data, sizeof(char), DROPBOX_FILE_CHUNK_SIZE, file);

				if (isFirstChunk)
				{
					DropBoxSendFileChunkedStart(data, count, uploadId, offset);
					isFirstChunk = false;
				}
				else
				{
					DropBoxSendFileChunkedNext(data, count, uploadId, offset);
				}
			}

			fclose(file);

			return DropBoxSendFileChunkedEnd(fileName, uploadId, pccsd->hContact);

		}

		//ULONG fileId = ::InterlockedIncrement(&g_fileId);

		//return fileId;
	}

	return 0;
}

INT_PTR DropBoxSendMessage(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

INT_PTR DropBoxReceiveMessage(WPARAM wParam, LPARAM lParam)
{
	/*CCSDATA *pccsd = (CCSDATA *)lParam;
	PROTORECVEVENT *ppre = (PROTORECVEVENT *)pccsd->lParam;
	return CallService(MS_PROTO_CHAINRECV, wParam, lParam);

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = MODULE;
	dbei.timestamp = ppre->timestamp;
	dbei.eventType = type;
	dbei.cbBlob = ppre->cbCustomDataSize;
	dbei.pBlob = ppre->szMessage;
	dbei.flags = flags;
	return ::db_event_add(pccsd->hContact, &dbei);*/

	return 0;
}

void SetContactStatus(MCONTACT hContact, int newStatus)
{
	if (db_get_w(hContact, MODULE, "Status", ID_STATUS_OFFLINE) != newStatus)
		db_set_w(hContact, MODULE, "Status", newStatus);
}

bool DropBoxLogIn()
{
	char *access_token = db_get_sa(NULL, MODULE, "TokenSecret");
	if (!access_token)
	{
		ShellExecuteA(NULL, "open", "https://www.dropbox.com/1/oauth2/authorize?response_type=code&client_id="DROPBOX_API_KEY, NULL, NULL, SW_SHOWDEFAULT);

		char request_token[128];
		request_token[0] = 0;

		if (DialogBoxParam(
			g_hInstance,
			MAKEINTRESOURCE(IDD_TOKEN_REQUEST),
			NULL, 
			TokenRequestProc,
			(LPARAM)&request_token) == IDOK)
		{
			HttpRequest *request = new HttpRequest(g_hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/oauth2/token");
			request->AddParameter("grant_type", "authorization_code");
			request->AddParameter("code", request_token);
			request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
			char data[64];
			mir_snprintf(
				data,
				SIZEOF(data),
				"client_id=%s&client_secret=%s",
				DROPBOX_API_KEY,
				DROPBOX_API_SECRET);
			request->pData = mir_strdup(data);
			request->dataLength = strlen(data);

			NETLIBHTTPREQUEST *response = request->Send();

			if (response && response->resultCode == 200)
			{
				JSONNODE *root = json_parse(response->pData);
				if (root != NULL)
				{
					JSONNODE *node = json_get(root, "access_token");
					access_token = mir_u2a(json_as_string(node));
					db_set_s(NULL, MODULE, "TokenSecret", access_token);

					MCONTACT hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
					if (!CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)MODULE))
					{
						node = json_get(root, "uid");
						wchar_t *uid = json_as_string(node);
						db_set_ws(hContact, MODULE, "uid", uid);

						db_set_s(hContact, MODULE, "Nick", MODULE);
						db_set_w(hContact, MODULE, "Status", ID_STATUS_ONLINE);
						//SetContactStatus(hContact, ID_STATUS_ONLINE);
					}

					delete node;
					delete root;
					delete request;
					//delete access_token;

					return true;
				}
			}

			delete request;
		}
	}
	/*else
	{
	for (MCONTACT hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE))
	SetContactStatus(hContact, ID_STATUS_ONLINE);
	}*/

	return false;
}

//void LogOut()
//{
//	HttpRequest<CDropBoxProto> *request = new HttpRequest<CDropBoxProto>(this, REQUEST_POST, DROPBOX_API_URL "/disable_access_token");
//	//request->SendAsync(&CDropBoxProto::AsyncFunc);
//	request->Send();
//
//	delete request;
//}

INT_PTR CALLBACK TokenRequestProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char *token = reinterpret_cast<char*>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		token = (char*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			//::SendMessage(hwndDlg, WM_SETICON, ICON_BIG,	(LPARAM)CSkypeProto::IconList[0].Handle);
			//::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL,	(LPARAM)CSkypeProto::IconList[0].Handle);

			/*wchar_t title[MAX_PATH];
			::mir_sntprintf(
			title, 
			MAX_PATH, 
			::TranslateT("Enter a password for %s:"), 
			param->login);*/
			//::SetDlgItemText(hwndDlg, IDC_INSTRUCTION, title);

			SendDlgItemMessage(hwndDlg, IDC_TOKEN, EM_LIMITTEXT, 128 - 1, 0);
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
					char data[128];
					GetDlgItemTextA(hwndDlg, IDC_TOKEN, data, SIZEOF(data));
					strcpy(token, data);

					EndDialog(hwndDlg, IDOK);
				}
				break;

			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK MainOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}