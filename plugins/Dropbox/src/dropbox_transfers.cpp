#include "stdafx.h"

void CDropbox::SendFile(const char *path, const char *data, size_t size)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	ptrA encodedPath(mir_utf8encode(path));
	UploadFileRequest request(token, encodedPath, data, size);
	NLHR_PTR response(request.Send(hNetlibConnection));
	HandleHttpResponseError(response);
}

void CDropbox::SendFileChunkedFirst(const char *data, size_t size, char *uploadId, size_t &offset)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	UploadFileChunkRequest request(token, data, size);
	NLHR_PTR response(request.Send(hNetlibConnection));

	HandleHttpResponseError(response);

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty())
		return;

	JSONNode node = root.at("upload_id");
	mir_strcpy(uploadId, node.as_string().c_str());

	node = root.at("offset");
	offset = node.as_int();
}

void CDropbox::SendFileChunkedNext(const char *data, size_t size, const char *uploadId, size_t &offset)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	UploadFileChunkRequest request(token, uploadId, offset, data, size);
	NLHR_PTR response(request.Send(hNetlibConnection));

	HandleHttpResponseError(response);
	
	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty())
		return;

	offset = root.at("offset").as_int();
}

void CDropbox::SendFileChunkedLast(const char *path, const char *uploadId)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	UploadFileChunkRequest request(token, uploadId, path);
	NLHR_PTR response(request.Send(hNetlibConnection));
	HandleHttpResponseError(response);
}

void CDropbox::CreateFolder(const char *path)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	CreateFolderRequest request(token, path);
	NLHR_PTR response(request.Send(hNetlibConnection));

	// forder exists on server
	if (response->resultCode == HTTP_STATUS_FORBIDDEN)
		return;

	HandleHttpResponseError(response);
}

void CDropbox::CreateDownloadUrl(const char *path, char *url)
{
	ptrA token(db_get_sa(NULL, MODULE, "TokenSecret"));
	bool useShortUrl = db_get_b(NULL, MODULE, "UseSortLinks", 1) > 0;
	ShareRequest request(token, path, useShortUrl);
	NLHR_PTR response(request.Send(hNetlibConnection));

	HandleHttpResponseError(response);

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty())
		return;
	
	JSONNode node = root.at("url");
	mir_strcpy(url, node.as_string().c_str());
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
			ftp->pfts.tszCurrentFile = _tcsrchr(ftp->pfts.ptszFiles[i], '\\') + 1;

			size_t offset = 0;
			char uploadId[32];
			int chunkSize = DROPBOX_FILE_CHUNK_SIZE / 4;
			if (fileSize < 1024 * 1024)
				chunkSize = DROPBOX_FILE_CHUNK_SIZE / 20;
			else if (fileSize > 20 * 1024 * 1024)
				chunkSize = DROPBOX_FILE_CHUNK_SIZE;

			char *data = (char*)mir_alloc(chunkSize);
			while (!feof(hFile) && fileSize != offset)
			{
				try
				{
					if (ferror(hFile))
						throw TransferException("Error while file sending");

					if (ftp->isTerminated)
						throw TransferException("Transfer was terminated");

					size_t size = fread(data, sizeof(char), chunkSize, hFile);

					if (offset == 0)
						instance->SendFileChunkedFirst(data, size, uploadId, offset);
					else
						instance->SendFileChunkedNext(data, size, uploadId, offset);

					ftp->pfts.currentFileProgress += size;
					ftp->pfts.totalProgress += size;
				}
				catch (TransferException&)
				{
					mir_free(data);
					fclose(hFile);
					throw;
				}

				ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftp->hProcess, (LPARAM)&ftp->pfts);
			}
			mir_free(data);
			fclose(hFile);

			if (ftp->pfts.currentFileProgress < ftp->pfts.currentFileSize)
				throw TransferException("Transfer was terminated");

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

	mir_forkthread(&CDropbox::RequestAccountInfo, instance);

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

	instance->Report(ftp->hContact, urls.GetBuffer());

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