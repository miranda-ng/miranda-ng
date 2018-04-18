#include "..\stdafx.h"
#include "dropbox_api.h"

CDropboxService::CDropboxService(const char *protoName, const wchar_t *userName)
	: CCloudService(protoName, userName)
{
	m_hProtoIcon = GetIconHandle(IDI_DROPBOX);
}

CDropboxService* CDropboxService::Init(const char *moduleName, const wchar_t *userName)
{
	CDropboxService *proto = new CDropboxService(moduleName, userName);
	Services.insert(proto);
	return proto;
}

int CDropboxService::UnInit(CDropboxService *proto)
{
	Services.remove(proto);
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
	COAuthDlg dlg(this, DROPBOX_API_AUTH, RequestAccessTokenThread);
	dlg.SetParent(owner);
	dlg.DoModal();
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
	NLHR_PTR response(request.Send(service->m_hConnection));

	if (response == nullptr || response->resultCode != HTTP_CODE_OK) {
		Netlib_Logf(service->m_hConnection, "%s: %s", service->GetAccountName(), service->HttpStatusToError());
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

	SetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, "");

	EndDialog(hwndDlg, 1);
	return 0;
}

unsigned CDropboxService::RevokeAccessTokenThread(void *param)
{
	CDropboxService *service = (CDropboxService*)param;

	ptrA token(service->getStringA("TokenSecret"));
	DropboxAPI::RevokeAccessTokenRequest request(token);
	NLHR_PTR response(request.Send(service->m_hConnection));

	service->delSetting("ExpiresIn");
	service->delSetting("TokenSecret");
	service->delSetting("RefreshToken");

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

auto CDropboxService::UploadFile(const char *data, size_t size, const std::string &path)
{
	ptrA token(getStringA("TokenSecret"));
	BYTE strategy = db_get_b(NULL, MODULE, "ConflictStrategy", OnConflict::REPLACE);
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
	BYTE strategy = db_get_b(NULL, MODULE, "ConflictStrategy", OnConflict::REPLACE);
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

	HandleHttpError(response);

	// forder exists on server 
	if (response->resultCode == HTTP_CODE_FORBIDDEN)
		return;

	GetJsonResponse(response);
}

auto CDropboxService::CreateSharedLink(const std::string &path)
{
	ptrA token(getStringA("TokenSecret"));
	DropboxAPI::CreateSharedLinkRequest shareRequest(token, path.c_str());
	NLHR_PTR response(shareRequest.Send(m_hConnection));

	if (response == nullptr)
		throw Exception(HttpStatusToError());

	if (!HTTP_CODE_SUCCESS(response->resultCode) &&
		response->resultCode != HTTP_CODE_CONFLICT) {
		if (response->dataLength)
			throw Exception(response->pData);
		throw Exception(HttpStatusToError(response->resultCode));
	}

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

UINT CDropboxService::Upload(FileTransferParam *ftp)
{
	if (!IsLoggedIn())
		Login();

	try {
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

			size_t chunkSize = ftp->GetCurrentFileChunkSize();
			mir_ptr<char>chunk((char*)mir_calloc(chunkSize));

			std::string path;
			auto serverFolder = ftp->GetServerFolder();
			if (serverFolder) {
				char serverPath[MAX_PATH] = { 0 };
				mir_snprintf(serverPath, "%s\\%s", T2Utf(serverFolder), fileName);
				path = PreparePath(serverPath);
			}
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

			if (!ftp->IsFolder()) {
				auto link = CreateSharedLink(path);
				ftp->AddSharedLink(link.c_str());
			}
		} while (ftp->NextFile());
	}
	catch (Exception &ex) {
		debugLogA("%s: %s", GetModuleName(), ex.what());
		ftp->SetStatus(ACKRESULT_FAILED);
		return ACKRESULT_FAILED;
	}

	ftp->SetStatus(ACKRESULT_SUCCESS);
	return ACKRESULT_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct CMPluginDropbox : public PLUGIN<CMPluginDropbox>
{
	CMPluginDropbox() :
		PLUGIN<CMPluginDropbox>(MODULE "/Dropbox")
	{
		m_hInst = g_plugin.getInst();

		RegisterProtocol(PROTOTYPE_PROTOWITHACCS, (pfnInitProto)CDropboxService::Init, (pfnUninitProto)CDropboxService::UnInit);
	}
}
	g_pluginDropbox;
