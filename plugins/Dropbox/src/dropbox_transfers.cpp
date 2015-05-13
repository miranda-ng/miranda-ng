#include "stdafx.h"

void CDropbox::SendFile(const char *path, const char *data, int length)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	ptrA encodedPath(mir_utf8encode(path));
	UploadFileRequest request(token, encodedPath, data, length);
	mir_ptr<NETLIBHTTPREQUEST> response(request.Send(hNetlibConnection));
	HandleHttpResponseError(response);
}

void CDropbox::SendFileChunkedFirst(const char *data, int length, char *uploadId, size_t &offset)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	UploadFileChunkRequest request(token, data, length);
	mir_ptr<NETLIBHTTPREQUEST> response(request.Send(hNetlibConnection));
	HandleHttpResponseError(response);
	JSONROOT root(response->pData);
	if (root)
	{
		JSONNODE *node = json_get(root, "upload_id");
		strcpy(uploadId, mir_u2a(json_as_string(node)));

		node = json_get(root, "offset");
		offset = json_as_int(node);
	}
}

void CDropbox::SendFileChunkedNext(const char *data, int length, const char *uploadId, size_t &offset)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	UploadFileChunkRequest request(token, uploadId, offset, data, length);
	mir_ptr<NETLIBHTTPREQUEST> response(request.Send(hNetlibConnection));
	HandleHttpResponseError(response);
	JSONROOT root(response->pData);
	if (root)
	{
		JSONNODE *node = json_get(root, "offset");
		offset = json_as_int(node);
	}
}

void CDropbox::SendFileChunkedLast(const char *path, const char *uploadId)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	ptrA encodedPath(mir_utf8encode(path));
	UploadFileChunkRequest request(token, uploadId, (char*)encodedPath);
	mir_ptr<NETLIBHTTPREQUEST> response(request.Send(hNetlibConnection));
	HandleHttpResponseError(response);
}

void CDropbox::CreateFolder(const char *path)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	ptrA encodedPath(mir_utf8encode(path));
	CreateFolderRequest request(token, encodedPath);
	mir_ptr<NETLIBHTTPREQUEST> response(request.Send(hNetlibConnection));

	// forder exists on server
	if (response->resultCode == HTTP_STATUS_FORBIDDEN)
		return;

	HandleHttpResponseError(response);
}

void CDropbox::CreateDownloadUrl(const char *path, char *url)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	ptrA encodedPath(mir_utf8encode(path));
	bool useShortUrl = db_get_b(NULL, MODULE, "UseSortLinks", 1) > 0;
	ShareRequest request(token, encodedPath, useShortUrl);
	mir_ptr<NETLIBHTTPREQUEST> response(request.Send(hNetlibConnection));
	HandleHttpResponseError(response);
	JSONROOT root(response->pData);
	if (root)
	{
		JSONNODE *node = json_get(root, "url");
		mir_strcpy(url, _T2A(json_as_string(node)));
	}
}

UINT CDropbox::SendFilesAsync(void *owner, void *arg)
{
	CDropbox *instance = (CDropbox*)owner;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ftp->hProcess, 0);

	try
	{
		if (ftp->ptszFolders)
		{
			for (int i = 0; ftp->ptszFolders[i]; i++)
			{
				if(ftp->isTerminated)
					throw TransferException("Transfer was terminated");

				ptrA utf8_folderName(mir_utf8encodeW(ftp->ptszFolders[i]));

				instance->CreateFolder(utf8_folderName);
				if (!strchr(utf8_folderName, '\\'))
				{
					char url[MAX_PATH];
					instance->CreateDownloadUrl(utf8_folderName, url);
					ftp->AddUrl(url);
				}
			}
		}

		for (int i = 0; ftp->pfts.ptszFiles[i]; i++)
		{
			if (ftp->isTerminated)
				throw TransferException("Transfer was terminated");

			FILE *hFile = _tfopen(ftp->pfts.ptszFiles[i], _T("rb"));
			if (hFile == NULL)
				throw TransferException("Unable to open file");

			const TCHAR *fileName = NULL;
			if (!ftp->relativePathStart)
				fileName = _tcsrchr(ftp->pfts.ptszFiles[i], L'\\') + 1;
			else
				fileName = &ftp->pfts.ptszFiles[i][ftp->relativePathStart];

			_fseeki64(hFile, 0, SEEK_END);
			uint64_t fileSize = _ftelli64(hFile);
			rewind(hFile);

			ftp->pfts.currentFileNumber = i;
			ftp->pfts.currentFileSize = fileSize;
			ftp->pfts.currentFileProgress = 0;
			ftp->pfts.tszCurrentFile = wcsrchr(ftp->pfts.ptszFiles[i], '\\') + 1;

			ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftp->hProcess, (LPARAM)&ftp->pfts);

			//
			size_t offset = 0;
			char *uploadId = new char[32];

			int chunkSize = DROPBOX_FILE_CHUNK_SIZE / 4;
			if (fileSize < 1024 * 1024)
				chunkSize = DROPBOX_FILE_CHUNK_SIZE / 20;
			else if (fileSize > 20 * 1024 * 1024)
				chunkSize = DROPBOX_FILE_CHUNK_SIZE;

			while (!feof(hFile) && fileSize != offset)
			{
				if (ferror(hFile))
					throw TransferException("Error while file sending");

				if (ftp->isTerminated)
					throw TransferException("Transfer was terminated");

				char *data = new char[chunkSize + 1];
				int count = (int)fread(data, sizeof(char), chunkSize, hFile);

				if (offset == 0)
					instance->SendFileChunkedFirst(data, count, uploadId, offset);
				else
					instance->SendFileChunkedNext(data, count, uploadId, offset);

				ftp->pfts.currentFileProgress += count;
				ftp->pfts.totalProgress += count;

				ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftp->hProcess, (LPARAM)&ftp->pfts);
			}

			fclose(hFile);

			ptrA utf8_fileName(mir_utf8encodeW(fileName));

			instance->SendFileChunkedLast(utf8_fileName, uploadId);

			if (!_tcschr(fileName, L'\\'))
			{
				char url[MAX_PATH];
				instance->CreateDownloadUrl(utf8_fileName, url);
				ftp->AddUrl(url);
			}

			ftp->pfts.currentFileProgress = ftp->pfts.currentFileSize;

			if (i < ftp->pfts.totalFiles - 1)
				ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ftp->hProcess, 0);
		}
	}
	catch (TransferException &ex)
	{
		Netlib_Logf(instance->hNetlibConnection, "%s: %s", MODULE, ex.what());
		ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ftp->hProcess, 0);
		return ACKRESULT_FAILED;
	}

	ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ftp->hProcess, 0);

	return 0;
}

UINT CDropbox::SendFilesAndReportAsync(void *owner, void *arg)
{
	CDropbox *instance = (CDropbox*)owner;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	int res = SendFilesAsync(owner, arg);
	if (res)
	{
		instance->transfers.remove(ftp);
		delete ftp;
		return res;
	}

	CMStringA urls;
	for (int i = 0; i < ftp->urlList.getCount(); i++)
		urls.AppendFormat("%s\r\n", ftp->urlList[i]);
	char *data = urls.GetBuffer();

	if (db_get_b(NULL, MODULE, "UrlAutoSend", 1))
	{
		char *message = mir_utf8encode(data);
		if (ftp->hContact != instance->GetDefaultContact())
		{
			if (CallContactService(ftp->hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)message) != ACKRESULT_FAILED)
			{
				DBEVENTINFO dbei = { sizeof(dbei) };
				dbei.flags = DBEF_UTF | DBEF_SENT/* | DBEF_READ*/;
				dbei.szModule = MODULE;
				dbei.timestamp = time(NULL);
				dbei.eventType = EVENTTYPE_MESSAGE;
				dbei.cbBlob = (int)mir_strlen(data);
				dbei.pBlob = (PBYTE)message;
				db_event_add(ftp->hContact, &dbei);
			}
			else CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)ftp->hContact, (LPARAM)data);
		}
		else
		{
			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.flags = DBEF_UTF;
			dbei.szModule = MODULE;
			dbei.timestamp = time(NULL);
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.cbBlob = (int)mir_strlen(data);
			dbei.pBlob = (PBYTE)message;
			db_event_add(ftp->hContact, &dbei);
		}
	}

	if (db_get_b(NULL, MODULE, "UrlPasteToMessageInputArea", 0))
		CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)ftp->hContact, (LPARAM)data);

	if (db_get_b(NULL, MODULE, "UrlCopyToClipboard", 0))
	{
		if (OpenClipboard(NULL))
		{
			EmptyClipboard();
			size_t size = sizeof(TCHAR) * (urls.GetLength() + 1);
			HGLOBAL hClipboardData = GlobalAlloc(NULL, size);
			if (hClipboardData)
			{
				TCHAR *pchData = (TCHAR*)GlobalLock(hClipboardData);
				if (pchData)
				{
					memcpy(pchData, (TCHAR*)data, size);
					GlobalUnlock(hClipboardData);
					SetClipboardData(CF_TEXT, hClipboardData);
				}
			}
			CloseClipboard();
		}
	}

	instance->transfers.remove(ftp);
	delete ftp;

	return res;
}

UINT CDropbox::SendFilesAndEventAsync(void *owner, void *arg)
{
	CDropbox *instance = (CDropbox*)owner;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	int res = SendFilesAsync(owner, arg);

	TRANSFERINFO ti = { 0 };
	ti.hProcess = ftp->hProcess;
	ti.status = res;
	ti.data = ftp->urlList.getArray();

	NotifyEventHooks(instance->hFileSentEventHook, ftp->hContact, (LPARAM)&ti);

	instance->transfers.remove(ftp);
	delete ftp;

	return res;
}