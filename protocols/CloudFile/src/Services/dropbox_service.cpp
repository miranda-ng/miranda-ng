#include "..\stdafx.h"
#include "dropbox_api.h"

struct CMPluginDropbox : public PLUGIN<CMPluginDropbox>
{
	CMPluginDropbox() :
		PLUGIN<CMPluginDropbox>(MODULENAME "/Dropbox", pluginInfoEx)
	{
		m_hInst = g_plugin.getInst();

		RegisterProtocol(PROTOTYPE_PROTOWITHACCS, CDropboxService::Init, CDropboxService::UnInit);
	}
}
	g_pluginDropbox;

/////////////////////////////////////////////////////////////////////////////////////////

CDropboxService::CDropboxService(const char *protoName, const wchar_t *userName) :
	CCloudService(protoName, userName, &g_pluginDropbox)
{
	m_hProtoIcon = g_plugin.getIconHandle(IDI_DROPBOX);
}

PROTO_INTERFACE* CDropboxService::Init(const char *moduleName, const wchar_t *userName)
{
	CDropboxService *proto = new CDropboxService(moduleName, userName);
	Services.insert(proto);
	return proto;
}

int CDropboxService::UnInit(PROTO_INTERFACE *proto)
{
	Services.remove((CDropboxService *)proto);
	delete proto;
	return 0;
}

const char* CDropboxService::GetModuleName() const
{
	return "Dropbox";
}

int CDropboxService::GetIconId() const
{
	return IDI_DROPBOX;
}

bool CDropboxService::IsLoggedIn()
{
	ptrA token(getStringA("TokenSecret"));
	if (!token || token[0] == 0)
		return false;
	return true;
}

void CDropboxService::Login(HWND owner)
{
	COAuthDlg dlg(this, DROPBOX_API_AUTH, (MyThreadFunc)&CDropboxService::RequestAccessTokenThread);
	dlg.SetParent(owner);
	dlg.DoModal();
}

void CDropboxService::Logout()
{
	ForkThread((MyThreadFunc)&CDropboxService::RevokeAccessTokenThread);
}

void CDropboxService::RequestAccessTokenThread(void *param)
{
	HWND hwndDlg = (HWND)param;

	if (IsLoggedIn())
		Logout();

	char requestToken[128];
	GetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, requestToken, _countof(requestToken));

	DropboxAPI::GetAccessTokenRequest request(requestToken);
	NLHR_PTR response(request.Send(m_hConnection));

	if (response == nullptr || response->resultCode != HTTP_CODE_OK) {
		Netlib_Logf(m_hConnection, "%s: %s", GetAccountName(), HttpStatusToError());
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

	SetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, "");

	EndDialog(hwndDlg, 1);
}

void CDropboxService::RevokeAccessTokenThread(void *)
{
	ptrA token(getStringA("TokenSecret"));
	DropboxAPI::RevokeAccessTokenRequest request(token);
	NLHR_PTR response(request.Send(m_hConnection));

	delSetting("ExpiresIn");
	delSetting("TokenSecret");
	delSetting("RefreshToken");
}

void CDropboxService::HandleJsonError(JSONNode &node)
{
	JSONNode error = node.at("error");
	if (!error.isnull()) {
		json_string tag = error.at(".tag").as_string();
		throw Exception(tag.c_str());
	}
}

auto CDropboxService::UploadFile(const char *data, size_t size, const std::string &path)
{
	ptrA token(getStringA("TokenSecret"));
	uint8_t strategy = g_plugin.getByte("ConflictStrategy", OnConflict::REPLACE);
	DropboxAPI::UploadFileRequest request(token, path.c_str(), data, size, (OnConflict)strategy);
	NLHR_PTR response(request.Send(m_hConnection));

	JSONNode root = GetJsonResponse(response);
	return root["path_lower"].as_string();
}

auto CDropboxService::CreateUploadSession(const char *chunk, size_t chunkSize)
{
	ptrA token(getStringA("TokenSecret"));
	DropboxAPI::CreateUploadSessionRequest request(token, chunk, chunkSize);
	NLHR_PTR response(request.Send(m_hConnection));

	JSONNode root = GetJsonResponse(response);
	return root["session_id"].as_string();
}

void CDropboxService::UploadFileChunk(const std::string &sessionId, const char *chunk, size_t chunkSize, size_t offset)
{
	ptrA token(getStringA("TokenSecret"));
	DropboxAPI::UploadFileChunkRequest request(token, sessionId.c_str(), offset, chunk, chunkSize);
	NLHR_PTR response(request.Send(m_hConnection));
	HandleHttpError(response);
}

auto CDropboxService::CommitUploadSession(const std::string &sessionId, const char *data, size_t size, size_t offset, const std::string &path)
{
	ptrA token(getStringA("TokenSecret"));
	uint8_t strategy = g_plugin.getByte("ConflictStrategy", OnConflict::REPLACE);
	DropboxAPI::CommitUploadSessionRequest request(token, sessionId.c_str(), offset, path.c_str(), data, size, (OnConflict)strategy);
	NLHR_PTR response(request.Send(m_hConnection));

	JSONNode root = GetJsonResponse(response);
	return root["path_lower"].as_string();
}

void CDropboxService::CreateFolder(const std::string &path)
{
	ptrA token(getStringA("TokenSecret"));
	DropboxAPI::CreateFolderRequest request(token, path.c_str());
	NLHR_PTR response(request.Send(m_hConnection));

	if (response == nullptr)
		throw Exception(HttpStatusToError());

	if (HTTP_CODE_SUCCESS(response->resultCode)) {
		GetJsonResponse(response);
		return;
	}

	// forder exists on server
	if (response->resultCode == HTTP_CODE_CONFLICT) {
		return;
	}

	HttpResponseToError(response);
}

auto CDropboxService::CreateSharedLink(const std::string &path)
{
	ptrA token(getStringA("TokenSecret"));
	DropboxAPI::CreateSharedLinkRequest shareRequest(token, path.c_str());
	NLHR_PTR response(shareRequest.Send(m_hConnection));

	if (response && HTTP_CODE_SUCCESS(response->resultCode)) {
		JSONNode root = GetJsonResponse(response);
		return root["url"].as_string();
	}

	if (!response || response->resultCode != HTTP_CODE_CONFLICT)
		HttpResponseToError(response);

	JSONNode root = JSONNode::parse(response->pData);
	if (root.isnull())
		throw Exception(HttpStatusToError());

	JSONNode error = root.at("error");
	if (error.isnull()) {
		JSONNode link = root.at("url");
		return link.as_string();
	}

	json_string tag = error.at(".tag").as_string();
	if (tag != "shared_link_already_exists")
		throw Exception(tag.c_str());

	DropboxAPI::GetSharedLinkRequest getRequest(token, path.c_str());
	response = getRequest.Send(m_hConnection);

	root = GetJsonResponse(response);

	JSONNode links = root.at("links").as_array();
	JSONNode link = links[(size_t)0].at("url");
	return link.as_string();
}

void CDropboxService::Upload(FileTransferParam *ftp)
{
	auto serverDictionary = ftp->GetServerDirectory();
	std::string serverFolder = serverDictionary ? T2Utf(serverDictionary) : "";
	if (!serverFolder.empty()) {
		auto path = PreparePath(serverFolder);
		CreateFolder(path);
		auto link = CreateSharedLink(path);
		ftp->AddSharedLink(link.c_str());
	}

	ftp->FirstFile();
	do
	{
		std::string fileName = T2Utf(ftp->GetCurrentRelativeFilePath());
		uint64_t fileSize = ftp->GetCurrentFileSize();

		size_t chunkSize = ftp->GetCurrentFileChunkSize();
		mir_ptr<char> chunk((char*)mir_calloc(chunkSize));

		std::string path;
		if (!serverFolder.empty())
			path = "/" + serverFolder + "/" + fileName;
		else
			path = PreparePath(fileName);

		if (chunkSize == fileSize) {
			ftp->CheckCurrentFile();
			size_t size = ftp->ReadCurrentFile(chunk, chunkSize);

			path = UploadFile(chunk, size, path);

			ftp->Progress(size);
		}
		else {
			ftp->CheckCurrentFile();
			size_t size = ftp->ReadCurrentFile(chunk, chunkSize);

			auto sessionId = CreateUploadSession(chunk, size);

			ftp->Progress(size);

			size_t offset = size;
			double chunkCount = ceil(double(fileSize) / chunkSize) - 2;
			for (size_t i = 0; i < chunkCount; i++) {
				ftp->CheckCurrentFile();

				size = ftp->ReadCurrentFile(chunk, chunkSize);
				UploadFileChunk(sessionId, chunk, size, offset);

				offset += size;
				ftp->Progress(size);
			}

			ftp->CheckCurrentFile();
			size = offset < fileSize
				? ftp->ReadCurrentFile(chunk, fileSize - offset)
				: 0;

			path = CommitUploadSession(sessionId, chunk, size, offset, path);

			ftp->Progress(size);
		}

		if (!ftp->IsCurrentFileInSubDirectory()) {
			auto link = CreateSharedLink(path);
			ftp->AddSharedLink(link.c_str());
		}
	} while (ftp->NextFile());
}
