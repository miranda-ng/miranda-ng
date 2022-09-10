#include "..\stdafx.h"
#include "microsoft_api.h"

struct CMPluginOnedrive : public CMPluginBase
{
	CMPluginOnedrive() :
		CMPluginBase(MODULENAME "/OneDrive", pluginInfoEx)
	{
		m_hInst = g_plugin.getInst();

		RegisterProtocol(PROTOTYPE_PROTOWITHACCS, COneDriveService::Init, COneDriveService::UnInit);
	}
}
g_pluginOnedrive;

/////////////////////////////////////////////////////////////////////////////////////////

COneDriveService::COneDriveService(const char *protoName, const wchar_t *userName) :
	CCloudService(protoName, userName, &g_pluginOnedrive)
{
	m_hProtoIcon = g_plugin.getIconHandle(IDI_ONEDRIVE);
}

PROTO_INTERFACE* COneDriveService::Init(const char *moduleName, const wchar_t *userName)
{
	COneDriveService *proto = new COneDriveService(moduleName, userName);
	Services.insert(proto);
	return proto;
}

int COneDriveService::UnInit(PROTO_INTERFACE *proto)
{
	Services.remove((COneDriveService *)proto);
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
	time_t now = time(0);
	time_t expiresIn = getDword("ExpiresIn");
	return now < expiresIn;
}

void COneDriveService::Login(HWND owner)
{
	ptrA refreshToken(getStringA("RefreshToken"));
	if (refreshToken && refreshToken[0]) {
		OneDriveAPI::RefreshTokenRequest request(refreshToken);
		NLHR_PTR response(request.Send(m_hConnection));

		JSONNode root = GetJsonResponse(response);

		JSONNode node = root.at("access_token");
		db_set_s(0, GetAccountName(), "TokenSecret", node.as_string().c_str());

		node = root.at("expires_in");
		time_t expiresIn = time(0) + node.as_int();
		setDword("ExpiresIn", expiresIn);

		return;
	}

	COAuthDlg dlg(this, MICROSOFT_AUTH, (MyThreadFunc)&COneDriveService::RequestAccessTokenThread);
	dlg.SetParent(owner);
	dlg.DoModal();
}

void COneDriveService::Logout()
{
	delSetting("ExpiresIn");
	delSetting("TokenSecret");
	delSetting("RefreshToken");
}

void COneDriveService::RequestAccessTokenThread(void *param)
{
	HWND hwndDlg = (HWND)param;

	if (IsLoggedIn())
		Logout();

	char requestToken[128];
	GetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, requestToken, _countof(requestToken));

	OneDriveAPI::GetAccessTokenRequest request(requestToken);
	NLHR_PTR response(request.Send(m_hConnection));

	if (response == nullptr || response->resultCode != HTTP_CODE_OK) {
		const char *error = response->dataLength
			? response->pData
			: HttpStatusToError(response->resultCode);

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
	setString("TokenSecret", node.as_string().c_str());

	node = root.at("expires_in");
	time_t expiresIn = time(0) + node.as_int();
	setDword("ExpiresIn", expiresIn);

	node = root.at("refresh_token");
	setString("RefreshToken", node.as_string().c_str());

	SetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, "");

	EndDialog(hwndDlg, 1);
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
	uint8_t strategy = g_plugin.getByte("ConflictStrategy", OnConflict::REPLACE);
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
	uint8_t strategy = g_plugin.getByte("ConflictStrategy", OnConflict::REPLACE);
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

	return std::string();
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

void COneDriveService::Upload(FileTransferParam *ftp)
{
	std::string folderId;
	auto serverDictionary = ftp->GetServerDirectory();
	std::string serverFolder = serverDictionary ? T2Utf(serverDictionary) : "";
	if (!serverFolder.empty()) {
		folderId = CreateFolder(serverFolder);
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
