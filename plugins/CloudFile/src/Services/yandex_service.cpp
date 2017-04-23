#include "..\stdafx.h"
#include "yandex_api.h"

CYandexService::CYandexService(HNETLIBUSER hConnection)
	: CCloudService(hConnection)
{
}

const char* CYandexService::GetModule() const
{
	return "Yandex";
}

const wchar_t* CYandexService::GetText() const
{
	return L"Яндекс.Диск";
}

HANDLE CYandexService::GetIcon() const
{
	return NULL;
}

bool CYandexService::IsLoggedIn()
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	if (!token || token[0] == 0)
		return false;
	time_t now = time(NULL);
	time_t expiresIn = db_get_dw(NULL, GetModule(), "ExpiresIn");
	return now < expiresIn;
}

void CYandexService::Login()
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	ptrA refreshToken(db_get_sa(NULL, GetModule(), "RefreshToken"));
	if (token && refreshToken && refreshToken[0]) {
		YandexAPI::RefreshTokenRequest request(refreshToken);
		NLHR_PTR response(request.Send(hConnection));

		JSONNode root = GetJsonResponse(response);

		JSONNode node = root.at("access_token");
		db_set_s(NULL, GetModule(), "TokenSecret", node.as_string().c_str());

		node = root.at("expires_in");
		time_t expiresIn = time(NULL) + node.as_int();
		db_set_dw(NULL, GetModule(), "ExpiresIn", expiresIn);

		node = root.at("refresh_token");
		db_set_s(NULL, GetModule(), "RefreshToken", node.as_string().c_str());

		return;
	}

	COAuthDlg dlg(this, YANDEX_OAUTH "/authorize?response_type=code&client_id=" YANDEX_APP_ID, RequestAccessTokenThread);
	dlg.DoModal();
}

void CYandexService::Logout()
{
	mir_forkthreadex(RevokeAccessTokenThread, this);
}

unsigned CYandexService::RequestAccessTokenThread(void *owner, void *param)
{
	HWND hwndDlg = (HWND)param;
	CYandexService *service = (CYandexService*)owner;

	if (service->IsLoggedIn())
		service->Logout();

	char requestToken[128];
	GetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, requestToken, _countof(requestToken));

	YandexAPI::GetAccessTokenRequest request(requestToken);
	NLHR_PTR response(request.Send(service->hConnection));

	if (response == NULL || response->resultCode != HTTP_CODE_OK) {
		const char *error = response->dataLength
			? response->pData
			: service->HttpStatusToError(response->resultCode);

		Netlib_Logf(service->hConnection, "%s: %s", service->GetModule(), error);
		ShowNotification(TranslateT("server does not respond"), MB_ICONERROR);
		return 0;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty()) {
		Netlib_Logf(service->hConnection, "%s: %s", service->GetModule(), service->HttpStatusToError(response->resultCode));
		ShowNotification(TranslateT("server does not respond"), MB_ICONERROR);
		return 0;
	}

	JSONNode node = root.at("error_description");
	if (!node.isnull()) {
		ptrW error_description(mir_a2u_cp(node.as_string().c_str(), CP_UTF8));
		Netlib_Logf(service->hConnection, "%s: %s", service->GetModule(), service->HttpStatusToError(response->resultCode));
		ShowNotification((wchar_t*)error_description, MB_ICONERROR);
		return 0;
	}

	node = root.at("access_token");
	db_set_s(NULL, service->GetModule(), "TokenSecret", node.as_string().c_str());

	node = root.at("expires_in");
	time_t expiresIn = time(NULL) + node.as_int();
	db_set_dw(NULL, service->GetModule(), "ExpiresIn", expiresIn);

	node = root.at("refresh_token");
	db_set_s(NULL, service->GetModule(), "RefreshToken", node.as_string().c_str());

	SetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, "");

	EndDialog(hwndDlg, 1);

	return 0;
}

unsigned CYandexService::RevokeAccessTokenThread(void *param)
{
	CYandexService *service = (CYandexService*)param;

	ptrA token(db_get_sa(NULL, service->GetModule(), "TokenSecret"));
	YandexAPI::RevokeAccessTokenRequest request(token);
	NLHR_PTR response(request.Send(service->hConnection));

	return 0;
}

void CYandexService::HandleJsonError(JSONNode &node)
{
	JSONNode error = node.at("error");
	if (!error.isnull()) {
		json_string tag = error.at(".tag").as_string();
		throw Exception(tag.c_str());
	}
}

void CYandexService::GetUploadUrl(char *path, char *url)
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	YandexAPI::GetUploadUrlRequest request(token, path);
	NLHR_PTR response(request.Send(hConnection));

	JSONNode root = GetJsonResponse(response);
	JSONNode node = root.at("href");
	mir_strcpy(url, node.as_string().c_str());
}

void CYandexService::UploadFile(const char *url, const char *data, size_t size)
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	YandexAPI::UploadFileRequest request(token, url, data, size);
	NLHR_PTR response(request.Send(hConnection));

	if (response == NULL)
		throw Exception(HttpStatusToError());

	if (response->resultCode >= HTTP_CODE_OK &&
		response->resultCode <= HTTP_CODE_MULTIPLE_CHOICES) {
		return;
	}

	if (response->dataLength)
		throw Exception(response->pData);
	throw Exception(HttpStatusToError(response->resultCode));
}

void CYandexService::CreateFolder(const char *path)
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	YandexAPI::CreateFolderRequest request(token, path);
	NLHR_PTR response(request.Send(hConnection));

	GetJsonResponse(response);
}

void CYandexService::CreateSharedLink(const char *path, char *url)
{
	ptrA token(db_get_sa(NULL, GetModule(), "TokenSecret"));
	YandexAPI::PublishRequest publishRequest(token, path);
	NLHR_PTR response(publishRequest.Send(hConnection));

	GetJsonResponse(response);

	YandexAPI::GetResourcesRequest resourcesRequest(token, path);
	response = resourcesRequest.Send(hConnection);

	JSONNode root = GetJsonResponse(response);
	JSONNode link = root.at("public_url");

	mir_strcpy(url, link.as_string().c_str());
}

UINT CYandexService::Upload(FileTransferParam *ftp)
{
	try {
		if (!IsLoggedIn())
			Login();

		if (!IsLoggedIn()) {
			ftp->SetStatus(ACKRESULT_FAILED);
			return ACKRESULT_FAILED;
		}

		const wchar_t *folderName = ftp->GetFolderName();
		if (folderName) {
			char path[MAX_PATH], link[MAX_PATH];
			PreparePath(folderName, path);
			CreateFolder(path);
			CreateSharedLink(path, link);
			ftp->AppendFormatData(L"%s\r\n", ptrW(mir_utf8decodeW(link)));
		}

		ftp->FirstFile();
		do
		{
			const wchar_t *fileName = ftp->GetCurrentRelativeFilePath();
			uint64_t fileSize = ftp->GetCurrentFileSize();

			char path[MAX_PATH];
			const wchar_t *serverFolder = ftp->GetServerFolder();
			if (serverFolder) {
				wchar_t serverPath[MAX_PATH] = { 0 };
				mir_snwprintf(serverPath, L"%s\\%s", serverFolder, fileName);
				PreparePath(serverPath, path);
			}
			else
				PreparePath(fileName, path);
			char url[MAX_PATH];
			GetUploadUrl(path, url);

			mir_ptr<char>data((char*)mir_calloc(fileSize));
			size_t size = ftp->ReadCurrentFile(data, fileSize);
			UploadFile(url, data, size);

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
