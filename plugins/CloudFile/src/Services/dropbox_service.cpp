#include "..\stdafx.h"
#include "dropbox_api.h"

CDropboxService::CDropboxService(HNETLIBUSER hConnection)
	: CCloudService(hConnection)
{
}

const char* CDropboxService::GetModule() const
{
	return "Dropbox";
}

const wchar_t* CDropboxService::GetText() const
{
	return L"Dropbox";
}

HANDLE CDropboxService::GetIcon() const
{
	return GetIconHandle(IDI_DROPBOX);
}

bool CDropboxService::IsLoggedIn()
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	if (!token || token[0] == 0)
		return false;
	return true;
}

void CDropboxService::Login()
{
	COAuthDlg(this, DROPBOX_WWW_URL "/oauth2/authorize?response_type=code&client_id=" DROPBOX_APP_KEY, RequestAccessTokenThread).DoModal();
}

void CDropboxService::Logout()
{
	mir_forkthreadex(RevokeAccessTokenThread, this);
}

unsigned CDropboxService::RequestAccessTokenThread(void *owner, void *param)
{
	HWND hwndDlg = (HWND)param;
	CDropboxService *service = (CDropboxService*)owner;

	if (service->IsLoggedIn())
		service->Logout();

	char requestToken[128];
	GetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, requestToken, _countof(requestToken));

	DropboxAPI::GetAccessTokenRequest request(requestToken);
	NLHR_PTR response(request.Send(service->hConnection));

	if (response == NULL || response->resultCode != HTTP_CODE_OK) {
		Netlib_Logf(service->hConnection, "%s: %s", service->GetModule(), service->HttpStatusToError());
		//ShowNotification(TranslateT("server does not respond"), MB_ICONERROR);
		return 0;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty()) {
		Netlib_Logf(service->hConnection, "%s: %s", service->GetModule(), service->HttpStatusToError(response->resultCode));
		//ShowNotification((wchar_t*)error_description, MB_ICONERROR);
		return 0;
	}

	JSONNode node = root.at("error_description");
	if (!node.isnull()) {
		ptrW error_description(mir_a2u_cp(node.as_string().c_str(), CP_UTF8));
		Netlib_Logf(service->hConnection, "%s: %s", service->GetModule(), service->HttpStatusToError(response->resultCode));
		//ShowNotification((wchar_t*)error_description, MB_ICONERROR);
		return 0;
	}

	node = root.at("access_token");
	db_set_s(NULL, service->GetModule(), "TokenSecret", node.as_string().c_str());
	ProtoBroadcastAck(MODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_OFFLINE, (WPARAM)ID_STATUS_ONLINE);

	SetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, "");

	EndDialog(hwndDlg, 1);

	return 0;
}

unsigned CDropboxService::RevokeAccessTokenThread(void *param)
{
	CDropboxService *service = (CDropboxService*)param;

	ptrA token(db_get_sa(NULL, service->GetModule(), "TokenSecret"));
	DropboxAPI::RevokeAccessTokenRequest request(token);
	NLHR_PTR response(request.Send(service->hConnection));

	return 0;
}

void CDropboxService::HandleJsonError(JSONNode &node)
{
	JSONNode error = node.at("error");
	if (!error.isnull()) {
		json_string tag = error.at(".tag").as_string();
		throw Exception(tag.c_str());
	}
}

char* CDropboxService::UploadFile(const char *data, size_t size, char *path)
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	ptrA encodedPath(mir_utf8encode(path));
	DropboxAPI::UploadFileRequest request(token, encodedPath, data, size);
	NLHR_PTR response(request.Send(hConnection));

	JSONNode root = GetJsonResponse(response);
	JSONNode node = root.at("path_lower");
	mir_strcpy(path, node.as_string().c_str());

	return path;
}

void CDropboxService::StartUploadSession(const char *data, size_t size, char *sessionId)
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	DropboxAPI::StartUploadSessionRequest request(token, data, size);
	NLHR_PTR response(request.Send(hConnection));

	JSONNode root = GetJsonResponse(response);
	JSONNode node = root.at("session_id");
	mir_strcpy(sessionId, node.as_string().c_str());
}

void CDropboxService::AppendToUploadSession(const char *data, size_t size, const char *sessionId, size_t offset)
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	DropboxAPI::AppendToUploadSessionRequest request(token, sessionId, offset, data, size);
	NLHR_PTR response(request.Send(hConnection));

	GetJsonResponse(response);
}

char* CDropboxService::FinishUploadSession(const char *data, size_t size, const char *sessionId, size_t offset, char *path)
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	DropboxAPI::FinishUploadSessionRequest request(token, sessionId, offset, path, data, size);
	NLHR_PTR response(request.Send(hConnection));

	JSONNode root = GetJsonResponse(response);
	JSONNode node = root.at("path_lower");
	mir_strcpy(path, node.as_string().c_str());

	return path;
}

void CDropboxService::CreateFolder(const char *path)
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	DropboxAPI::CreateFolderRequest request(token, path);
	NLHR_PTR response(request.Send(hConnection));

	HandleHttpError(response);

	// forder exists on server 
	if (response->resultCode == HTTP_CODE_FORBIDDEN)
		return;

	GetJsonResponse(response);
}

void CDropboxService::CreateSharedLink(const char *path, char *url)
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	DropboxAPI::CreateSharedLinkRequest shareRequest(token, path);
	NLHR_PTR response(shareRequest.Send(hConnection));

	HandleHttpError(response);

	JSONNode root = JSONNode::parse(response->pData);
	if (root.isnull())
		throw Exception(HttpStatusToError());

	JSONNode error = root.at("error");
	if (error.isnull()) {
		JSONNode link = root.at("link");
		mir_strcpy(url, link.as_string().c_str());
		return;
	}

	json_string tag = error.at(".tag").as_string();
	if (tag != "shared_link_already_exists")
		throw Exception(tag.c_str());

	DropboxAPI::GetSharedLinkRequest getRequest(token, path);
	response = getRequest.Send(hConnection);

	root = GetJsonResponse(response);

	JSONNode links = root.at("links").as_array();
	JSONNode link = links[0u].at("url");
	mir_strcpy(url, link.as_string().c_str());
}

UINT CDropboxService::Upload(FileTransferParam *ftp)
{
	if (!IsLoggedIn())
		Login();

	try {
		const wchar_t *folderName = ftp->GetFolderName();
		if (folderName) {
			char path[MAX_PATH], url[MAX_PATH];
			PreparePath(folderName, path);
			CreateFolder(path);
			CreateSharedLink(path, url);
			ftp->AppendFormatData(L"%s\r\n", ptrW(mir_utf8decodeW(url)));
		}

		ftp->FirstFile();
		do
		{
			const wchar_t *fileName = ftp->GetCurrentRelativeFilePath();
			uint64_t fileSize = ftp->GetCurrentFileSize();

			int chunkSize = ftp->GetCurrentFileChunkSize();
			mir_ptr<char>data((char*)mir_calloc(chunkSize));
			size_t size = ftp->ReadCurrentFile(data, chunkSize);

			size_t offset = 0;
			char sessionId[64];
			StartUploadSession(data, size, sessionId);

			offset += size;
			ftp->Progress(size);

			for (size_t chunk = 0; chunk < (fileSize / chunkSize) - 1; chunk++)
			{
				ftp->CheckCurrentFile();

				size = ftp->ReadCurrentFile(data, chunkSize);
				AppendToUploadSession(data, size, sessionId, offset);

				offset += size;
				ftp->Progress(size);
			}

			if (offset < fileSize)
				size = ftp->ReadCurrentFile(data, fileSize - offset);
			else
				size = 0;

			char path[MAX_PATH];
			const wchar_t *serverFolder = ftp->GetServerFolder();
			if (serverFolder) {
				wchar_t serverPath[MAX_PATH] = { 0 };
				mir_snwprintf(serverPath, L"%s\\%s", serverFolder, fileName);
				PreparePath(serverPath, path);
			}
			else
				PreparePath(fileName, path);
			FinishUploadSession(data, size, sessionId, offset, path);

			ftp->Progress(size);

			if (!wcschr(fileName, L'\\')) {
				char url[MAX_PATH];
				CreateSharedLink(path, url);
				ftp->AppendFormatData(L"%s\r\n", ptrW(mir_utf8decodeW(url)));
			}
		} while (ftp->NextFile());
	}
	catch (Exception &ex) {
		Netlib_Logf(hConnection, "%s: %s", MODULE, ex.what());
		ftp->SetStatus(ACKRESULT_FAILED);
		return ACKRESULT_FAILED;
	}

	ftp->SetStatus(ACKRESULT_SUCCESS);
	return ACKRESULT_SUCCESS;
}
