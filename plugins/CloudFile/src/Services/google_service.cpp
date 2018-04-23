#include "..\stdafx.h"
#include "google_api.h"

CGDriveService::CGDriveService(const char *protoName, const wchar_t *userName)
	: CCloudService(protoName, userName)
{
	m_hProtoIcon = GetIconHandle(IDI_GDRIVE);
}

CGDriveService* CGDriveService::Init(const char *moduleName, const wchar_t *userName)
{
	CGDriveService *proto = new CGDriveService(moduleName, userName);
	Services.insert(proto);
	return proto;
}

int CGDriveService::UnInit(CGDriveService *proto)
{
	Services.remove(proto);
	delete proto;
	return 0;
}

const char* CGDriveService::GetModuleName() const
{
	return "/Google";
}

int CGDriveService::GetIconId() const
{
	return IDI_GDRIVE;
}

bool CGDriveService::IsLoggedIn()
{
	ptrA token(getStringA("TokenSecret"));
	if (!token || token[0] == 0)
		return false;
	time_t now = time(nullptr);
	time_t expiresIn = getDword("ExpiresIn");
	return now < expiresIn;
}

void CGDriveService::Login(HWND owner)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA refreshToken(getStringA("RefreshToken"));
	if (token && refreshToken && refreshToken[0]) {
		GDriveAPI::RefreshTokenRequest request(refreshToken);
		NLHR_PTR response(request.Send(m_hConnection));

		JSONNode root = GetJsonResponse(response);

		JSONNode node = root.at("access_token");
		setString("TokenSecret", node.as_string().c_str());

		node = root.at("expires_in");
		time_t expiresIn = time(nullptr) + node.as_int();
		setDword("ExpiresIn", expiresIn);

		return;
	}

	COAuthDlg dlg(this, GOOGLE_AUTH, RequestAccessTokenThread);
	dlg.SetParent(owner);
	dlg.DoModal();
}

void CGDriveService::Logout()
{
	mir_forkthreadex(RevokeAccessTokenThread, this);
}

unsigned CGDriveService::RequestAccessTokenThread(void *owner, void *param)
{
	HWND hwndDlg = (HWND)param;
	CGDriveService *service = (CGDriveService*)owner;

	if (service->IsLoggedIn())
		service->Logout();

	char requestToken[128];
	GetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, requestToken, _countof(requestToken));

	GDriveAPI::GetAccessTokenRequest request(requestToken);
	NLHR_PTR response(request.Send(service->m_hConnection));

	if (response == nullptr || response->resultCode != HTTP_CODE_OK) {
		const char *error = response && response->dataLength
			? response->pData
			: service->HttpStatusToError(response ? response->resultCode : 0);

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
	db_set_s(NULL, service->GetAccountName(), "TokenSecret", node.as_string().c_str());

	node = root.at("expires_in");
	time_t expiresIn = time(nullptr) + node.as_int();
	db_set_dw(NULL, service->GetAccountName(), "ExpiresIn", expiresIn);

	node = root.at("refresh_token");
	db_set_s(NULL, service->GetAccountName(), "RefreshToken", node.as_string().c_str());

	SetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, "");

	EndDialog(hwndDlg, 1);

	return 0;
}

unsigned CGDriveService::RevokeAccessTokenThread(void *param)
{
	CGDriveService *service = (CGDriveService*)param;

	ptrA token(db_get_sa(NULL, service->GetAccountName(), "TokenSecret"));
	GDriveAPI::RevokeAccessTokenRequest request(token);
	NLHR_PTR response(request.Send(service->m_hConnection));

	service->delSetting("ExpiresIn");
	service->delSetting("TokenSecret");
	service->delSetting("RefreshToken");

	return 0;
}

void CGDriveService::HandleJsonError(JSONNode &node)
{
	JSONNode error = node.at("error");
	if (!error.isnull()) {
		json_string tag = error.at(".tag").as_string();
		throw Exception(tag.c_str());
	}
}

auto CGDriveService::UploadFile(const std::string &parentId, const std::string &fileName, const char *data, size_t size)
{
	ptrA token(getStringA("TokenSecret"));
	GDriveAPI::UploadFileRequest request(token, parentId.c_str(), fileName.c_str(), data, size);
	NLHR_PTR response(request.Send(m_hConnection));
	JSONNode root = GetJsonResponse(response);
	return root["id"].as_string();
}

auto CGDriveService::CreateUploadSession(const std::string &parentId, const std::string &fileName)
{
	ptrA token(getStringA("TokenSecret"));
	GDriveAPI::CreateUploadSessionRequest request(token, parentId.c_str(), fileName.c_str());
	NLHR_PTR response(request.Send(m_hConnection));

	HandleHttpError(response);

	if (HTTP_CODE_SUCCESS(response->resultCode)) {
		for (int i = 0; i < response->headersCount; i++) {
			if (mir_strcmpi(response->headers[i].szName, "Location"))
				continue;
			return std::string(response->headers[i].szValue);
		}
	}

	HttpResponseToError(response);

	return std::string();
}

auto CGDriveService::UploadFileChunk(const std::string &uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize)
{
	GDriveAPI::UploadFileChunkRequest request(uploadUri.c_str(), chunk, chunkSize, offset, fileSize);
	NLHR_PTR response(request.Send(m_hConnection));

	if (response->resultCode == HTTP_CODE_PERMANENT_REDIRECT)
		return std::string();

	HandleHttpError(response);

	if (HTTP_CODE_SUCCESS(response->resultCode)) {
		JSONNode root = GetJsonResponse(response);
		return root["id"].as_string();
	}

	HttpResponseToError(response);

	return std::string();
}

auto CGDriveService::CreateFolder(const std::string &parentId, const std::string &name)
{
	ptrA token(getStringA("TokenSecret"));
	GDriveAPI::GetFolderRequest getFolderRequest(token, parentId.c_str(), name.c_str());
	NLHR_PTR response(getFolderRequest.Send(m_hConnection));

	JSONNode root = GetJsonResponse(response);
	JSONNode files = root["files"].as_array();
	if (files.size() > 0)
		return files[(size_t)0]["id"].as_string();

	GDriveAPI::CreateFolderRequest createFolderRequest(token, parentId.c_str(), name.c_str());
	response = createFolderRequest.Send(m_hConnection);

	root = GetJsonResponse(response);
	return root["id"].as_string();
}

auto CGDriveService::CreateSharedLink(const std::string &itemId)
{
	ptrA token(getStringA("TokenSecret"));
	GDriveAPI::GrantPermissionsRequest request(token, itemId.c_str());
	NLHR_PTR response(request.Send(m_hConnection));

	HandleHttpError(response);

	if (HTTP_CODE_SUCCESS(response->resultCode)) {
		std::string url = GDRIVE_SHARE;
		url += itemId;
		return url;
	}

	HttpResponseToError(response);

	return std::string();
}

void CGDriveService::Upload(FileTransferParam *ftp)
{
	std::string folderId;
	auto serverDictionary = ftp->GetServerDirectory();
	std::string serverFolder = serverDictionary ? T2Utf(serverDictionary) : "";
	if (!serverFolder.empty()) {
		folderId = CreateFolder(folderId, serverFolder);
		auto link = CreateSharedLink(folderId);
		ftp->AddSharedLink(link.c_str());
	}

	ftp->FirstFile();
	do {
		std::string fileName = T2Utf(ftp->GetCurrentRelativeFilePath());
		uint64_t fileSize = ftp->GetCurrentFileSize();

		size_t chunkSize = ftp->GetCurrentFileChunkSize();
		mir_ptr<char> chunk((char*)mir_calloc(chunkSize));

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

		if (!ftp->IsCurrentFileInSubDirectory()) {
			auto link = CreateSharedLink(fileId);
			ftp->AddSharedLink(link.c_str());
		}
	} while (ftp->NextFile());
}

/////////////////////////////////////////////////////////////////////////////////////////

struct CMPluginGoogle : public CMPluginBase
{
	CMPluginGoogle() :
		CMPluginBase(MODULE "/GDrive")
	{
		m_hInst = g_plugin.getInst();

		RegisterProtocol(PROTOTYPE_PROTOWITHACCS, (pfnInitProto)CGDriveService::Init, (pfnUninitProto)CGDriveService::UnInit);
	}
}
g_pluginGoogle;
