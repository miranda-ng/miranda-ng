#include "..\stdafx.h"
#include "microsoft_api.h"

COneDriveService::COneDriveService(const char *protoName, const wchar_t *userName)
	: CCloudService(protoName, userName)
{
	m_hProtoIcon = GetIconHandle(IDI_ONEDRIVE);
}

COneDriveService* COneDriveService::Init(const char *moduleName, const wchar_t *userName)
{
	COneDriveService *proto = new COneDriveService(moduleName, userName);
	Services.insert(proto);
	return proto;
}

int COneDriveService::UnInit(COneDriveService *proto)
{
	Services.remove(proto);
	delete proto;
	return 0;
}

const char* COneDriveService::GetModuleName() const
{
	return "/OneDrive";
}

int COneDriveService::GetIconId() const
{
	return IDI_ONEDRIVE;
}

bool COneDriveService::IsLoggedIn()
{
	ptrA token(getStringA("TokenSecret"));
	if (!token || token[0] == 0)
		return false;
	time_t now = time(nullptr);
	time_t expiresIn = getDword("ExpiresIn");
	return now < expiresIn;
}

void COneDriveService::Login()
{
	ptrA refreshToken(getStringA("RefreshToken"));
	if (refreshToken && refreshToken[0]) {
		OneDriveAPI::RefreshTokenRequest request(refreshToken);
		NLHR_PTR response(request.Send(m_hConnection));

		JSONNode root = GetJsonResponse(response);

		JSONNode node = root.at("access_token");
		db_set_s(NULL, GetAccountName(), "TokenSecret", node.as_string().c_str());

		node = root.at("expires_in");
		time_t expiresIn = time(nullptr) + node.as_int();
		setDword("ExpiresIn", expiresIn);

		return;
	}

	COAuthDlg dlg(this, MICROSOFT_AUTH, RequestAccessTokenThread);
	dlg.DoModal();
}

void COneDriveService::Logout()
{
	delSetting("ExpiresIn");
	delSetting("TokenSecret");
	delSetting("RefreshToken");
}

unsigned COneDriveService::RequestAccessTokenThread(void *owner, void *param)
{
	HWND hwndDlg = (HWND)param;
	COneDriveService *service = (COneDriveService*)owner;

	if (service->IsLoggedIn())
		service->Logout();

	char requestToken[128];
	GetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, requestToken, _countof(requestToken));

	OneDriveAPI::GetAccessTokenRequest request(requestToken);
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

void COneDriveService::HandleJsonError(JSONNode &node)
{
	JSONNode error = node.at("error");
	if (!error.isnull()) {
		json_string tag = error.at("message").as_string();
		throw Exception(tag.c_str());
	}
}

auto COneDriveService::UploadFile(const std::string &parentId, const std::string &fileName, const char *data, size_t size)
{
	ptrA token(getStringA("TokenSecret"));
	BYTE strategy = db_get_b(NULL, MODULE, "ConflictStrategy", OnConflict::REPLACE);
	OneDriveAPI::UploadFileRequest *request = !parentId.empty()
		? new OneDriveAPI::UploadFileRequest(token, parentId.c_str(), fileName.c_str(), data, size, (OnConflict)strategy)
		: new OneDriveAPI::UploadFileRequest(token, fileName.c_str(), data, size, (OnConflict)strategy);
	NLHR_PTR response(request->Send(m_hConnection));
	delete request;

	JSONNode root = GetJsonResponse(response);
	return root["id"].as_string();
}

auto COneDriveService::CreateUploadSession(const std::string &parentId, const std::string &fileName)
{
	ptrA token(getStringA("TokenSecret"));
	BYTE strategy = db_get_b(NULL, MODULE, "ConflictStrategy", OnConflict::REPLACE);
	OneDriveAPI::CreateUploadSessionRequest *request = !parentId.empty()
		? new OneDriveAPI::CreateUploadSessionRequest(token, parentId.c_str(), fileName.c_str(), (OnConflict)strategy)
		: new OneDriveAPI::CreateUploadSessionRequest(token, fileName.c_str(), (OnConflict)strategy);
	NLHR_PTR response(request->Send(m_hConnection));
	delete request;

	JSONNode root = GetJsonResponse(response);
	return root["uploadUrl"].as_string();
}

auto COneDriveService::UploadFileChunk(const std::string &uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize)
{
	OneDriveAPI::UploadFileChunkRequest request(uploadUri.c_str(), chunk, chunkSize, offset, fileSize);
	NLHR_PTR response(request.Send(m_hConnection));

	HandleHttpError(response);

	if (response->resultCode == HTTP_CODE_ACCEPTED)
		return std::string();

	if (HTTP_CODE_SUCCESS(response->resultCode)) {
		JSONNode root = GetJsonResponse(response);
		return root["id"].as_string();
	}

	HttpResponseToError(response);
}

auto COneDriveService::CreateFolder(const std::string &path)
{
	ptrA token(getStringA("TokenSecret"));
	OneDriveAPI::CreateFolderRequest request(token, path.c_str());
	NLHR_PTR response(request.Send(m_hConnection));

	JSONNode root = GetJsonResponse(response);
	return root["id"].as_string();
}

auto COneDriveService::CreateSharedLink(const std::string &itemId)
{
	ptrA token(getStringA("TokenSecret"));
	OneDriveAPI::CreateSharedLinkRequest request(token, itemId.c_str());
	NLHR_PTR response(request.Send(m_hConnection));

	JSONNode root = GetJsonResponse(response);
	return root["link"]["webUrl"].as_string();
}

UINT COneDriveService::Upload(FileTransferParam *ftp)
{
	try {
		if (!IsLoggedIn())
			Login();

		if (!IsLoggedIn()) {
			ftp->SetStatus(ACKRESULT_FAILED);
			return ACKRESULT_FAILED;
		}

		std::string folderId;
		if (ftp->IsFolder()) {
			T2Utf folderName(ftp->GetFolderName());

			auto path = PreparePath(folderName);
			folderId = CreateFolder(path);

			auto link = CreateSharedLink(path);
			ftp->AddSharedLink(link.c_str());
		}

		ftp->FirstFile();
		do {
			std::string fileName = T2Utf(ftp->GetCurrentRelativeFilePath());
			uint64_t fileSize = ftp->GetCurrentFileSize();

			size_t chunkSize = ftp->GetCurrentFileChunkSize();
			mir_ptr<char>chunk((char*)mir_calloc(chunkSize));

			std::string fileId;
			if (chunkSize == fileSize) {
				ftp->CheckCurrentFile();
				size_t size = ftp->ReadCurrentFile(chunk, chunkSize);

				fileId = UploadFile(folderId, fileName, chunk, size);

				ftp->Progress(size);
			}
			else {
				auto uploadUri = CreateUploadSession(folderId, fileName);

				uint64_t offset = 0;
				double chunkCount = ceil(double(fileSize) / chunkSize);
				for (size_t i = 0; i < chunkCount; i++) {
					ftp->CheckCurrentFile();
					size_t size = ftp->ReadCurrentFile(chunk, chunkSize);
					fileId = UploadFileChunk(uploadUri, chunk, size, offset, fileSize);
					offset += size;
					ftp->Progress(size);
				}
			}

			if (!ftp->IsFolder()) {
				auto link = CreateSharedLink(fileId);
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

/////////////////////////////////////////////////////////////////////////////////////////

struct CMPluginOnedrive : public CMPluginBase
{
	CMPluginOnedrive() :
		CMPluginBase(MODULE "/OneDrive")
	{
		RegisterProtocol(PROTOTYPE_PROTOCOL, (pfnInitProto)COneDriveService::Init, (pfnUninitProto)COneDriveService::UnInit);
	}
}
	g_pluginOnedrive;
