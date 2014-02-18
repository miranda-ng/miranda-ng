#include "dropBox_proto.h"

HANDLE g_hNetlibUser;
ULONG  g_fileId = 1;

bool HasAccessToken()
{
	return db_get_sa(NULL, MODULE, "TokenSecret") != NULL;
}

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
		request->AddHeader("Content-Type", "application/octet-stream");
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
		"%s/commit_chunked_upload/%s/%s",
		DROPBOX_APICONTENT_URL,
		DROPBOX_API_ROOT,
		fileName);

	HttpRequest *request = new HttpRequest(g_hNetlibUser, REQUEST_POST, url);
	request->AddParameter("upload_id", uploadId);
	request->AddParameter("access_token", db_get_sa(NULL, MODULE, "TokenSecret"));

	NETLIBHTTPREQUEST *response = request->Send();

	delete request;

	if (response && response->resultCode == 200)
	{
		mir_snprintf(
			url,
			SIZEOF(url),
			"%s/shares/%s/%s",
			DROPBOX_API_URL,
			DROPBOX_API_ROOT,
			fileName);

		request = new HttpRequest(g_hNetlibUser, REQUEST_POST, url);
		request->AddParameter("access_token", db_get_sa(NULL, MODULE, "TokenSecret"));

		response = request->Send();

		if (response && response->resultCode == 200)
		{
			JSONNODE *root = json_parse(response->pData);
			if (root != NULL)
			{
				JSONNODE *node = json_get(root, "url");
				char *message = mir_utf8encodeW(json_as_string(node));

				DBEVENTINFO dbei = { sizeof(dbei) };
				dbei.szModule = MODULE;
				dbei.timestamp = time(NULL);
				dbei.eventType = EVENTTYPE_MESSAGE;
				dbei.cbBlob = strlen(message);
				dbei.pBlob = (PBYTE)mir_strdup(message);
				dbei.flags = DBEF_UTF;
				::db_event_add(hContact, &dbei);
			}
		}

		return true;
	}

	return false;
}

void DropBoxAsyncFileSend(void *arg)
{
	FileTransferParam *ftp = (FileTransferParam *)arg;

	ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ftp->hProcess, 0);

	for (int i = 0; ftp->pfts.pszFiles[i]; i++)
	{
		FILE *file = fopen(ftp->pfts.pszFiles[i], "rb");
		if (file != NULL)
		{
			int offset = 0;
			char *uploadId = new char[32];

			const char *fileName = strrchr(ftp->pfts.pszFiles[i], '\\') + 1;

			fseek(file, 0, SEEK_END);
			DWORD fileSize = ftell(file);
			fseek(file, 0, SEEK_SET);

			ftp->pfts.currentFileNumber = i;
			ftp->pfts.currentFileSize = fileSize;
			ftp->pfts.currentFileProgress = 0;
			ftp->pfts.szCurrentFile = strrchr(ftp->pfts.pszFiles[i], '\\') + 1;

			ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftp->hProcess, (LPARAM)&ftp->pfts);

			while (!feof(file) && !ferror(file))
			{
				int chunkSize = DROPBOX_FILE_CHUNK_SIZE;
				if (fileSize < 1024*1024)
					chunkSize = DROPBOX_FILE_CHUNK_SIZE / 5;
				else if (fileSize > 20*1024*1024)
					chunkSize = DROPBOX_FILE_CHUNK_SIZE * 4;

				char *data = new char[chunkSize + 1];
				size_t count = fread(data, sizeof(char), chunkSize, file);

				if (!offset)
					DropBoxSendFileChunkedStart(data, count, uploadId, offset);
				else
					DropBoxSendFileChunkedNext(data, count, uploadId, offset);

				ftp->pfts.currentFileProgress += count;
				ftp->pfts.totalProgress += count;

				ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftp->hProcess, (LPARAM)&ftp->pfts);
			}

			fclose(file);

			DropBoxSendFileChunkedEnd(fileName, uploadId, ftp->pfts.hContact);
			ftp->pfts.currentFileProgress = ftp->pfts.currentFileSize;

			if (i < ftp->pfts.totalFiles - 1)
				ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ftp->hProcess, 0);
		}
	}

	ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ftp->hProcess, 0);

	delete ftp;
}

INT_PTR DropBoxSendFile(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	FileTransferParam *ftp = new FileTransferParam();
	ftp->pfts.flags = PFTS_SENDING | PFTS_UTF;
	ftp->pfts.hContact = pccsd->hContact;

	char **files = (char**)pccsd->lParam;
	
	for (ftp->pfts.totalFiles = 0; files[ftp->pfts.totalFiles]; ftp->pfts.totalFiles++);
	ftp->pfts.pszFiles = new char*[ftp->pfts.totalFiles + 1];
	ftp->pfts.pszFiles[ftp->pfts.totalFiles] = NULL;
	for (int i = 0; i < ftp->pfts.totalFiles; i++)
	{
		ftp->pfts.pszFiles[i] = mir_strdup(files[i]);

		FILE *file = fopen(files[i], "rb");
		if (file != NULL)
		{
			fseek(file, 0, SEEK_END);
			ftp->pfts.totalBytes += ftell(file);
			fseek(file, 0, SEEK_SET);
			fclose(file);
		}
	}
	ULONG fileId = ::InterlockedIncrement(&g_fileId);
	ftp->hProcess = (HANDLE)fileId;

	mir_forkthread(DropBoxAsyncFileSend, ftp);

	return fileId;
}

INT_PTR DropBoxSendMessage(WPARAM wParam, LPARAM lParam)
{
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