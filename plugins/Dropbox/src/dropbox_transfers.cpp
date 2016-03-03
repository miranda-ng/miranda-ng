#include "stdafx.h"

char* CDropbox::UploadFile(const char *data, size_t size, char *path)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	ptrA encodedPath(mir_utf8encode(path));
	UploadFileRequest request(token, encodedPath, data, size);
	NLHR_PTR response(request.Send(hNetlibConnection));

	HandleJsonResponseError(response);

	JSONNode root = JSONNode::parse(response->pData);
	if (!root.empty())
	{
		JSONNode node = root.at("path_lower");
		mir_strcpy(path, node.as_string().c_str());
	}
	return path;
}

void CDropbox::StartUploadSession(const char *data, size_t size, char *sessionId)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	StartUploadSessionRequest request(token, data, size);
	NLHR_PTR response(request.Send(hNetlibConnection));

	HandleJsonResponseError(response);

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty())
		return;

	JSONNode node = root.at("session_id");
	mir_strcpy(sessionId, node.as_string().c_str());
}

void CDropbox::AppendToUploadSession(const char *data, size_t size, const char *sessionId, size_t offset)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	AppendToUploadSessionRequest request(token, sessionId, offset, data, size);
	NLHR_PTR response(request.Send(hNetlibConnection));

	HandleJsonResponseError(response);
}

char* CDropbox::FinishUploadSession(const char *data, size_t size, const char *sessionId, size_t offset, char *path)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	FinishUploadSessionRequest request(token, sessionId, offset, path, data, size);
	NLHR_PTR response(request.Send(hNetlibConnection));

	HandleJsonResponseError(response);

	JSONNode root = JSONNode::parse(response->pData);
	if (!root.empty())
	{
		JSONNode node = root.at("path_lower");
		mir_strcpy(path, node.as_string().c_str());
	}
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

	HandleJsonResponseError(response);
}

void CDropbox::CreateDownloadUrl(const char *path, char *url)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	bool useShortUrl = db_get_b(NULL, MODULE, "UseSortLinks", 1) > 0;
	HttpRequest *request;
	if (useShortUrl)
		request = new ShareOldRequest(token, path);
	else
		request = new ShareRequest(token, path);
	NLHR_PTR response(request->Send(hNetlibConnection));
	delete request;

	HandleJsonResponseError(response);

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty())
		return;

	JSONNode node = root.at("url");
	mir_strcpy(url, node.as_string().c_str());
}

UINT CDropbox::UploadToDropbox(void *owner, void *arg)
{
	CDropbox *instance = (CDropbox*)owner;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	try {
		const TCHAR *folderName = ftp->GetFolderName();
		if (folderName) {
			char path[MAX_PATH], url[MAX_PATH];
			PreparePath(folderName, path);
			instance->CreateFolder(path);
			instance->CreateDownloadUrl(path, url);
			ftp->AppendFormatData(_T("%s\r\n"), ptrT(mir_utf8decodeT(url)));
		}

		ftp->FirstFile();
		do
		{
			const TCHAR *fileName = ftp->GetCurrentRelativeFilePath();
			uint64_t fileSize = ftp->GetCurrentFileSize();

			int chunkSize = ftp->GetCurrentFileChunkSize();
			char *data = (char*)mir_calloc(chunkSize);
			size_t size = ftp->ReadCurrentFile(data, chunkSize);

			size_t offset = 0;
			char sessionId[32];
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
			const TCHAR *serverPath = ftp->GetServerPath();
			if (serverPath)
				PreparePath(serverPath, path);
			else
				PreparePath(fileName, path);
			instance->FinishUploadSession(data, size, sessionId, offset, path);

			ftp->Progress(size);

			if (!_tcschr(fileName, L'\\')) {
				char url[MAX_PATH];
				instance->CreateDownloadUrl(path, url);
				ftp->AppendFormatData(_T("%s\r\n"), ptrT(mir_utf8decodeT(url)));
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

UINT CDropbox::SendFilesAndReportAsync(void *owner, void *arg)
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

UINT CDropbox::SendFilesAndEventAsync(void *owner, void *arg)
{
	CDropbox *instance = (CDropbox*)owner;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	int res = UploadToDropbox(owner, arg);

	T2Utf data(ftp->GetData());
	char *pdata = data;

	TRANSFERINFO ti = { 0 };
	ti.hProcess = (HANDLE)ftp->GetId();
	ti.status = res;
	ti.data = &pdata;

	NotifyEventHooks(instance->hFileSentEventHook, ftp->GetHContact(), (LPARAM)&ti);

	instance->transfers.remove(ftp);
	delete ftp;

	return res;
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