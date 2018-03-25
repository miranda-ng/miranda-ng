#include "..\stdafx.h"
#include "yandex_api.h"

CYandexService::CYandexService(const char *protoName, const wchar_t *userName)
	: CCloudService(protoName, userName)
{
	m_hProtoIcon = GetIconHandle(IDI_YADISK);
}

CYandexService* CYandexService::Init(const char *moduleName, const wchar_t *userName)
{
	CYandexService *proto = new CYandexService(moduleName, userName);
	Services.insert(proto);
	return proto;
}

int CYandexService::UnInit(CYandexService *proto)
{
	Services.remove(proto);
	delete proto;
	return 0;
}

const char* CYandexService::GetModuleName() const
{
	return "Yandex.Disk";
}

int CYandexService::GetIconId() const
{
	return IDI_YADISK;
}

bool CYandexService::IsLoggedIn()
{
	ptrA token(getStringA("TokenSecret"));
	if (!token || token[0] == 0)
		return false;
	time_t now = time(nullptr);
	time_t expiresIn = getDword("ExpiresIn");
	return now < expiresIn;
}

void CYandexService::Login()
{
	ptrA token(getStringA("TokenSecret"));
	ptrA refreshToken(getStringA("RefreshToken"));
	if (token && refreshToken && refreshToken[0]) {
		YandexAPI::RefreshTokenRequest request(refreshToken);
		NLHR_PTR response(request.Send(m_hConnection));

		JSONNode root = GetJsonResponse(response);

		JSONNode node = root.at("access_token");
		setString("TokenSecret", node.as_string().c_str());

		node = root.at("expires_in");
		time_t expiresIn = time(nullptr) + node.as_int();
		setDword("ExpiresIn", expiresIn);

		node = root.at("refresh_token");
		setString("RefreshToken", node.as_string().c_str());

		return;
	}

	COAuthDlg dlg(this, YANDEX_AUTH, RequestAccessTokenThread);
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
	NLHR_PTR response(request.Send(service->m_hConnection));

	if (response == nullptr || response->resultCode != HTTP_CODE_OK) {
		const char *error = response->dataLength
			? response->pData
			: service->HttpStatusToError(response->resultCode);

		Netlib_Logf(service->m_hConnection, "%s: %s", service->GetAccountName(), error);
		ShowNotification(TranslateT("Server does not respond"), MB_ICONERROR);
		EndDialog(hwndDlg, 0);
		return 0;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty()) {
		Netlib_Logf(service->m_hConnection, "%s: %s", service->GetAccountName(), service->HttpStatusToError(response->resultCode));
		ShowNotification(TranslateT("Server does not respond"), MB_ICONERROR);
		EndDialog(hwndDlg, 0);
		return 0;
	}

	JSONNode node = root.at("error_description");
	if (!node.isnull()) {
		CMStringW error_description = node.as_mstring();
		Netlib_Logf(service->m_hConnection, "%s: %s", service->GetAccountName(), service->HttpStatusToError(response->resultCode));
		ShowNotification(error_description, MB_ICONERROR);
		EndDialog(hwndDlg, 0);
		return 0;
	}

	node = root.at("access_token");
	service->setString("TokenSecret", node.as_string().c_str());

	node = root.at("expires_in");
	time_t expiresIn = time(nullptr) + node.as_int();
	service->setDword("ExpiresIn", expiresIn);

	node = root.at("refresh_token");
	service->setString("RefreshToken", node.as_string().c_str());

	SetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, "");

	EndDialog(hwndDlg, 1);
	return 0;
}

unsigned CYandexService::RevokeAccessTokenThread(void *param)
{
	CYandexService *service = (CYandexService*)param;

	ptrA token(db_get_sa(NULL, service->GetAccountName(), "TokenSecret"));
	YandexAPI::RevokeAccessTokenRequest request(token);
	NLHR_PTR response(request.Send(service->m_hConnection));

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

auto CYandexService::CreateUploadSession(const std::string &path)
{
	ptrA token(getStringA("TokenSecret"));
	BYTE strategy = db_get_b(NULL, MODULE, "ConflictStrategy", OnConflict::REPLACE);
	YandexAPI::GetUploadUrlRequest request(token, path.c_str(), (OnConflict)strategy);
	NLHR_PTR response(request.Send(m_hConnection));

	JSONNode root = GetJsonResponse(response);
	return root["href"].as_string();
}

void CYandexService::UploadFile(const std::string &uploadUri, const char *data, size_t size)
{
	YandexAPI::UploadFileRequest request(uploadUri.c_str(), data, size);
	NLHR_PTR response(request.Send(m_hConnection));

	HandleHttpError(response);

	if (response->resultCode == HTTP_CODE_CREATED)
		return;

	HttpResponseToError(response);
}

void CYandexService::UploadFileChunk(const std::string &uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize)
{
	YandexAPI::UploadFileChunkRequest request(uploadUri.c_str(), chunk, chunkSize, offset, fileSize);
	NLHR_PTR response(request.Send(m_hConnection));

	HandleHttpError(response);

	if (response->resultCode == HTTP_CODE_ACCEPTED ||
		response->resultCode == HTTP_CODE_CREATED)
		return;

	HttpResponseToError(response);
}

void CYandexService::CreateFolder(const std::string &path)
{
	ptrA token(getStringA("TokenSecret"));
	YandexAPI::CreateFolderRequest request(token, path.c_str());
	NLHR_PTR response(request.Send(m_hConnection));

	GetJsonResponse(response);
}

auto CYandexService::CreateSharedLink(const std::string &path)
{
	ptrA token(getStringA("TokenSecret"));
	YandexAPI::PublishRequest publishRequest(token, path.c_str());
	NLHR_PTR response(publishRequest.Send(m_hConnection));

	GetJsonResponse(response);

	YandexAPI::GetResourcesRequest resourcesRequest(token, path.c_str());
	response = resourcesRequest.Send(m_hConnection);

	JSONNode root = GetJsonResponse(response);
	return root["public_url"].as_string();
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

		if (ftp->IsFolder()) {
			T2Utf folderName(ftp->GetFolderName());

			auto path = PreparePath(folderName);
			CreateFolder(path);

			auto link = CreateSharedLink(path);
			ftp->AddSharedLink(link.c_str());
		}

		ftp->FirstFile();
		do
		{
			T2Utf fileName(ftp->GetCurrentRelativeFilePath());
			uint64_t fileSize = ftp->GetCurrentFileSize();

			auto path = PreparePath(fileName);
			auto uploadUri = CreateUploadSession(path);

			size_t chunkSize = ftp->GetCurrentFileChunkSize();
			mir_ptr<char>chunk((char*)mir_calloc(chunkSize));

			if (chunkSize == fileSize) {
				ftp->CheckCurrentFile();
				size_t size = ftp->ReadCurrentFile(chunk, chunkSize);
				UploadFile(uploadUri, chunk, size);
				ftp->Progress(size);
			}
			else {
				uint64_t offset = 0;
				double chunkCount = ceil(double(fileSize) / chunkSize);
				for (size_t i = 0; i < chunkCount; i++) {
					ftp->CheckCurrentFile();
					size_t size = ftp->ReadCurrentFile(chunk, chunkSize);
					UploadFileChunk(uploadUri, chunk, size, offset, fileSize);
					offset += size;
					ftp->Progress(size);
				}
			}

			if (!ftp->IsFolder()) {
				auto link = CreateSharedLink(path);
				ftp->AddSharedLink(link.c_str());
			}
		} while (ftp->NextFile());
	}
	catch (Exception &ex) {
		debugLogA("%s: %s", GetAccountName(), ex.what());
		ftp->SetStatus(ACKRESULT_FAILED);
		return ACKRESULT_FAILED;
	}

	ftp->SetStatus(ACKRESULT_SUCCESS);
	return ACKRESULT_SUCCESS;
}
