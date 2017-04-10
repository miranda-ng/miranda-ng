#include "stdafx.h"

char* CDropbox::UploadFile(const char *data, size_t size, char *path)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	ptrA encodedPath(mir_utf8encode(path));
	UploadFileRequest request(token, encodedPath, data, size);
	NLHR_PTR response(request.Send(hNetlibConnection));

	JSONNode root = HandleJsonResponse(response);
	JSONNode node = root.at("path_lower");
	mir_strcpy(path, node.as_string().c_str());

	return path;
}

void CDropbox::StartUploadSession(const char *data, size_t size, char *sessionId)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	StartUploadSessionRequest request(token, data, size);
	NLHR_PTR response(request.Send(hNetlibConnection));

	JSONNode root = HandleJsonResponse(response);
	JSONNode node = root.at("session_id");
	mir_strcpy(sessionId, node.as_string().c_str());
}

void CDropbox::AppendToUploadSession(const char *data, size_t size, const char *sessionId, size_t offset)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	AppendToUploadSessionRequest request(token, sessionId, offset, data, size);
	NLHR_PTR response(request.Send(hNetlibConnection));

	HandleJsonResponse(response);
}

char* CDropbox::FinishUploadSession(const char *data, size_t size, const char *sessionId, size_t offset, char *path)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	FinishUploadSessionRequest request(token, sessionId, offset, path, data, size);
	NLHR_PTR response(request.Send(hNetlibConnection));

	JSONNode root = HandleJsonResponse(response);
	JSONNode node = root.at("path_lower");
	mir_strcpy(path, node.as_string().c_str());

	return path;
}

void CDropbox::CreateFolder(const char *path)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	CreateFolderRequest request(token, path);
	NLHR_PTR response(request.Send(hNetlibConnection));

	// forder exists on server 
	if (response->resultCode == HTTP_STATUS_FORBIDDEN)
		return;

	HandleJsonResponse(response);
}

void CDropbox::CreateDownloadUrl(const char *path, char *url)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	if (db_get_b(NULL, MODULE, "UrlIsTemporary", 0)) {
		GetTemporaryLinkRequest request(token, path);
		NLHR_PTR response(request.Send(hNetlibConnection));

		JSONNode root = HandleJsonResponse(response);
		JSONNode link = root.at("link");
		mir_strcpy(url, link.as_string().c_str());
		return;
	}

	CreateSharedLinkRequest shareRequest(token, path);
	NLHR_PTR response(shareRequest.Send(hNetlibConnection));

	HandleHttpResponse(response);

	JSONNode root = JSONNode::parse(response->pData);
	if (root.isnull())
		throw DropboxException(HttpStatusToText(HTTP_STATUS_ERROR));

	JSONNode error = root.at("error");
	if (error.isnull()) {
		JSONNode link = root.at("url");
		mir_strcpy(url, link.as_string().c_str());
		return;
	}

	json_string tag = error.at(".tag").as_string();
	if (tag != "shared_link_already_exists")
		throw DropboxException(tag.c_str());

	GetSharedLinkRequest getRequest(token, path);
	response = getRequest.Send(hNetlibConnection);

	root = HandleJsonResponse(response);

	JSONNode links = root.at("links").as_array();
	const JSONNode &link = (*links.begin()).at("url");
	mir_strcpy(url, link.as_string().c_str());
}

UINT CDropbox::UploadToDropbox(void *owner, void *arg)
{
	CDropbox *instance = (CDropbox*)owner;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	try {
		const wchar_t *folderName = ftp->GetFolderName();
		if (folderName) {
			char path[MAX_PATH], url[MAX_PATH];
			PreparePath(folderName, path);
			instance->CreateFolder(path);
			instance->CreateDownloadUrl(path, url);
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
			instance->StartUploadSession(data, size, sessionId);

			offset += size;
			ftp->Progress(size);

			for (size_t chunk = 0; chunk < (fileSize / chunkSize) - 1; chunk++)
			{
				ftp->CheckCurrentFile();

				size = ftp->ReadCurrentFile(data, chunkSize);
				instance->AppendToUploadSession(data, size, sessionId, offset);

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
			instance->FinishUploadSession(data, size, sessionId, offset, path);

			ftp->Progress(size);

			if (!wcschr(fileName, L'\\')) {
				char url[MAX_PATH];
				instance->CreateDownloadUrl(path, url);
				ftp->AppendFormatData(L"%s\r\n", ptrW(mir_utf8decodeW(url)));
			}
		} while (ftp->NextFile());
	}
	catch (DropboxException &ex) {
		Netlib_Logf(instance->hNetlibConnection, "%s: %s", MODULE, ex.what());
		ftp->SetStatus(ACKRESULT_FAILED);
		return ACKRESULT_FAILED;
	}

	ftp->SetStatus(ACKRESULT_SUCCESS);
	return ACKRESULT_SUCCESS;
}

UINT CDropbox::UploadAndReportProgress(void *owner, void *arg)
{
	CDropbox *instance = (CDropbox*)owner;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	int res = UploadToDropbox(owner, arg);
	if (res == ACKRESULT_SUCCESS)
		instance->Report(ftp->GetHContact(), ftp->GetData());

	instance->transfers.remove(ftp);
	delete ftp;

	return res;
}

UINT CDropbox::UploadAndRaiseEvent(void *owner, void *arg)
{
	CDropbox *instance = (CDropbox*)owner;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	int res = UploadToDropbox(owner, arg);

	DropboxUploadResult ur = { 0 };
	ur.hProcess = (HANDLE)ftp->GetId();
	ur.status = res;
	ur.data = T2Utf(ftp->GetData());

	NotifyEventHooks(instance->hUploadedEventHook, ftp->GetHContact(), (LPARAM)&ur);

	instance->transfers.remove(ftp);
	delete ftp;

	return res;
}