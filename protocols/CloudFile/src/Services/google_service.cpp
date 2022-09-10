#include "..\stdafx.h"
#include "google_api.h"


struct CMPluginGoogle : public CMPluginBase
{
	CMPluginGoogle() :
		CMPluginBase(MODULENAME "/GDrive", pluginInfoEx)
	{
		m_hInst = g_plugin.getInst();

		RegisterProtocol(PROTOTYPE_PROTOWITHACCS, CGDriveService::Init, CGDriveService::UnInit);
	}
}
g_pluginGoogle;

/////////////////////////////////////////////////////////////////////////////////////////

CGDriveService::CGDriveService(const char *protoName, const wchar_t *userName) :
	CCloudService(protoName, userName, &g_pluginGoogle)
{
	m_hProtoIcon = g_plugin.getIconHandle(IDI_GDRIVE);
}

PROTO_INTERFACE* CGDriveService::Init(const char *moduleName, const wchar_t *userName)
{
	CGDriveService *proto = new CGDriveService(moduleName, userName);
	Services.insert(proto);
	return proto;
}

int CGDriveService::UnInit(PROTO_INTERFACE *proto)
{
	Services.remove((CGDriveService*)proto);
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
	time_t now = time(0);
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
		time_t expiresIn = time(0) + node.as_int();
		setDword("ExpiresIn", expiresIn);

		return;
	}

	COAuthDlg dlg(this, GOOGLE_AUTH, (MyThreadFunc)&CGDriveService::RequestAccessTokenThread);
	dlg.SetParent(owner);
	dlg.DoModal();
}

void CGDriveService::Logout()
{
	ForkThread((MyThreadFunc)&CGDriveService::RevokeAccessTokenThread);
}

void CGDriveService::RequestAccessTokenThread(void *param)
{
	HWND hwndDlg = (HWND)param;

	if (IsLoggedIn())
		Logout();

	char requestToken[128];
	GetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, requestToken, _countof(requestToken));

	GDriveAPI::GetAccessTokenRequest request(requestToken);
	NLHR_PTR response(request.Send(m_hConnection));

	if (response == nullptr || response->resultCode != HTTP_CODE_OK) {
		const char *error = response && response->dataLength
			? response->pData
			: HttpStatusToError(response ? response->resultCode : 0);

		Netlib_Logf(m_hConnection, "%s: %s", GetAccountName(), error);
		ShowNotification(TranslateT("Server does not respond"), MB_ICONERROR);
		EndDialog(hwndDlg, 0);
		return;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty()) {
		Netlib_Logf(m_hConnection, "%s: %s", GetAccountName(), HttpStatusToError(response->resultCode));
		ShowNotification(TranslateT("Server does not respond"), MB_ICONERROR);
		EndDialog(hwndDlg, 0);
		return;
	}

	JSONNode node = root.at("error_description");
	if (!node.isnull()) {
		CMStringW error_description = node.as_mstring();
		Netlib_Logf(m_hConnection, "%s: %s", GetAccountName(), HttpStatusToError(response->resultCode));
		ShowNotification(error_description, MB_ICONERROR);
		EndDialog(hwndDlg, 0);
		return;
	}

	node = root.at("access_token");
	db_set_s(0, GetAccountName(), "TokenSecret", node.as_string().c_str());

	node = root.at("expires_in");
	time_t expiresIn = time(0) + node.as_int();
	db_set_dw(0, GetAccountName(), "ExpiresIn", expiresIn);

	node = root.at("refresh_token");
	db_set_s(0, GetAccountName(), "RefreshToken", node.as_string().c_str());

	SetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, "");

	EndDialog(hwndDlg, 1);
}

void CGDriveService::RevokeAccessTokenThread(void*)
{
	ptrA token(db_get_sa(0, GetAccountName(), "TokenSecret"));
	GDriveAPI::RevokeAccessTokenRequest request(token);
	NLHR_PTR response(request.Send(m_hConnection));

	delSetting("ExpiresIn");
	delSetting("TokenSecret");
	delSetting("RefreshToken");
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

	if (HTTP_CODE_SUCCESS(response->resultCode)) 
		if (auto *pszHdr = Netlib_GetHeader(response, "Location"))
			return std::string(pszHdr);

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
	std::string folderId = "root";
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
