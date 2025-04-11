#include "stdafx.h"

CCloudService::CCloudService(const char *protoName, const wchar_t *userName, HPLUGIN pPlugin) :
	PROTO<CCloudService>(protoName, userName),
	m_pPlugin(pPlugin)
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = (char*)protoName;
	nlu.szDescriptiveName.w = (wchar_t*)userName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	CreateProtoService(PS_UPLOAD, &CCloudService::UploadMenuCommand);

	g_arServices.insert(this);
}

CCloudService::~CCloudService()
{
	if (!Miranda_IsTerminated())
		InitializeMenus();

	Netlib_CloseHandle(m_hNetlibUser);
	m_hNetlibUser = nullptr;
}

void CCloudService::OnModulesLoaded()
{
	InitializeMenus();
}

HPLUGIN CCloudService::GetId() const
{
	return m_pPlugin;
}

INT_PTR CCloudService::GetCaps(int type, MCONTACT)
{
	if (type == PFLAGNUM_1)
		return PF1_FILESEND;

	return 0;
}

int CCloudService::FileCancel(MCONTACT, HANDLE hTransfer)
{
	FileTransferParam *ftp = Transfers.find((FileTransferParam*)&hTransfer);
	if (ftp)
		ftp->Terminate();

	return 0;
}

HANDLE CCloudService::SendFile(MCONTACT hContact, const wchar_t *description, wchar_t **paths)
{
	FileTransferParam *ftp = new FileTransferParam(hContact);
	ftp->SetDescription(description);
	ftp->SetWorkingDirectory(paths[0]);
	for (int i = 0; paths[i]; i++) {
		if (PathIsDirectory(paths[i]))
			continue;
		ftp->AddFile(paths[i]);
	}
	Transfers.insert(ftp);
	mir_forkthreadowner(UploadAndReportProgressThread, this, ftp);
	return (HANDLE)ftp->GetId();
}

void CCloudService::OpenUploadDialog(MCONTACT hContact)
{
	char *proto = Proto_GetBaseAccountName(hContact);
	if (!mir_strcmpi(proto, META_PROTO))
		hContact = db_mc_getMostOnline(hContact);

	auto it = InterceptedContacts.find(hContact);
	if (it == InterceptedContacts.end()) {
		HWND hwnd = File::Send(hContact);
		InterceptedContacts[hContact] = hwnd;
	}
	else SetActiveWindow(it->second);
}

MWindow CCloudService::OnCreateAccMgrUI(MWindow hwndParent)
{
	CAccountManagerDlg *page = new CAccountManagerDlg(this);
	page->SetParent(hwndParent);
	page->Show();
	return page->GetHwnd();
}

std::string CCloudService::PreparePath(const std::string &path) const
{
	std::string newPath = path;
	if (newPath[0] != '/')
		newPath.insert(0, "/");
	std::replace(newPath.begin(), newPath.end(), '\\', '/');
	size_t pos = newPath.find("//");
	while (pos != std::string::npos) {
		newPath.replace(pos, 2, "/");
		pos = newPath.find("//", pos + 1);
	}
	return newPath;
}

char* CCloudService::HttpStatusToError(int status)
{
	switch (status) {
	case HTTP_CODE_OK:
		return "OK";
	case HTTP_CODE_BAD_REQUEST:
		return "Bad input parameter. Error message should indicate which one and why";
	case HTTP_CODE_UNAUTHORIZED:
		return "Bad or expired token. This can happen if the user or Dropbox revoked or expired an access token. To fix, you should re-authenticate the user";
	case HTTP_CODE_FORBIDDEN:
		return "Bad OAuth request (wrong consumer key, bad nonce, expired timestamp...). Unfortunately, re-authenticating the user won't help here";
	case HTTP_CODE_NOT_FOUND:
		return "File or folder not found at the specified path";
	case HTTP_CODE_METHOD_NOT_ALLOWED:
		return "Request method not expected (generally should be GET or POST)";
	case HTTP_CODE_TOO_MANY_REQUESTS:
		return "Your app is making too many requests and is being rate limited. 429s can trigger on a per-app or per-user basis";
	case HTTP_CODE_SERVICE_UNAVAILABLE:
		return "If the response includes the Retry-After header, this means your OAuth 1.0 app is being rate limited. Otherwise, this indicates a transient server error, and your app should retry its request.";
	}

	return "Unknown error";
}

void CCloudService::HttpResponseToError(MHttpResponse *response)
{
	if (response == nullptr)
		throw Exception(HttpStatusToError());
	if (response->body.GetLength())
		throw Exception(response->body.c_str());
	throw Exception(HttpStatusToError(response->resultCode));
}

void CCloudService::HandleHttpError(MHttpResponse *response)
{
	if (response == nullptr)
		throw Exception(HttpStatusToError());

	if (HTTP_CODE_SUCCESS(response->resultCode))
		return;

	if (response->resultCode == HTTP_CODE_UNAUTHORIZED)
		delSetting("TokenSecret");

	HttpResponseToError(response);
}

JSONNode CCloudService::GetJsonResponse(MHttpResponse *response)
{
	HandleHttpError(response);

	JSONNode root = JSONNode::parse(response->body);
	if (root.isnull())
		throw Exception(HttpStatusToError());

	HandleJsonError(root);

	return root;
}

UINT CCloudService::Upload(CCloudService *service, FileTransferParam *ftp)
{
	try {
		if (!service->IsLoggedIn())
			service->Login();

		if (!service->IsLoggedIn()) {
			ftp->SetStatus(ACKRESULT_FAILED);
			return ACKRESULT_FAILED;
		}

		service->Upload(ftp);
	}
	catch (Exception &ex) {
		service->debugLogA("%s: %s", service->GetModuleName(), ex.what());
		ftp->SetStatus(ACKRESULT_FAILED);
		return ACKRESULT_FAILED;
	}

	ftp->SetStatus(ACKRESULT_SUCCESS);
	return ACKRESULT_SUCCESS;
}

int CCloudService::UnInit(PROTO_INTERFACE *proto)
{
	g_arServices.remove((CCloudService *)proto);
	delete proto;
	return 0;
}

INT_PTR CCloudService::UploadMenuCommand(WPARAM hContact, LPARAM)
{
	OpenUploadDialog(hContact);
	return 0;
}
