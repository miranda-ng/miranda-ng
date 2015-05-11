#include "stdafx.h"

int CDropbox::SendFile(const char *fileName, const char *data, int length)
{
	char *utf8_fileName = mir_utf8encode(fileName);

	CMStringA url;
	url.AppendFormat(DROPBOX_APICONTENT_URL "/files_put/%s/%s",
		DROPBOX_API_ROOT,
		utf8_fileName);
	url.Replace('\\', '/');

	mir_free(utf8_fileName);

	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_PUT, DROPBOX_APICONTENT_URL "/files_put");
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	request->pData = (char*)mir_alloc(sizeof(char)* length);
	memcpy(request->pData, data, length);
	request->dataLength = length;

	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	return HandleHttpResponseError(hNetlibUser, response);
}

int CDropbox::SendFileChunkedFirst(const char *data, int length, char *uploadId, size_t &offset)
{
	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_PUT, DROPBOX_APICONTENT_URL "/chunked_upload");
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	request->AddHeader("Content-Type", "application/octet-stream");
	request->pData = (char*)mir_alloc(sizeof(char)* length);
	memcpy(request->pData, data, length);
	request->dataLength = length;

	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	if (response && response->resultCode == HTTP_STATUS_OK)
	{
		JSONROOT root(response->pData);
		if (root)
		{
			JSONNODE *node = json_get(root, "upload_id");
			strcpy(uploadId, mir_u2a(json_as_string(node)));

			node = json_get(root, "offset");
			offset = json_as_int(node);
		}
	}

	return HandleHttpResponseError(hNetlibUser, response);
}

int CDropbox::SendFileChunkedNext(const char *data, int length, const char *uploadId, size_t &offset)
{
	CMStringA url = DROPBOX_APICONTENT_URL "/chunked_upload";
	url.AppendFormat("?upload_id=%s&offset=%i", uploadId, offset);

	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_PUT, url);
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	request->AddHeader("Content-Type", "application/octet-stream");
	request->pData = (char*)mir_alloc(sizeof(char)* length);
	memcpy(request->pData, data, length);
	request->dataLength = length;

	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	if (response && response->resultCode == HTTP_STATUS_OK)
	{
		JSONROOT root(response->pData);
		if (root)
		{
			JSONNODE *node = json_get(root, "offset");
			offset = json_as_int(node);
		}
	}

	return HandleHttpResponseError(hNetlibUser, response);
}

int CDropbox::SendFileChunkedLast(const char *fileName, const char *uploadId)
{
	CMStringA url;
	url.AppendFormat(
		"%s/commit_chunked_upload/%s/%s",
		DROPBOX_APICONTENT_URL,
		DROPBOX_API_ROOT,
		fileName);
	url.Replace('\\', '/');

	CMStringA param = CMStringA("upload_id=") + uploadId;

	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, url);
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request->pData = mir_strdup(param);
	request->dataLength = param.GetLength();

	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	return HandleHttpResponseError(hNetlibUser, response);
}

int CDropbox::CreateFolder(const char *folderName)
{
	CMStringA folder = folderName;
	folder.Replace('\\', '/');

	CMStringA param;
	param.AppendFormat("root=%s&path=%s",
		DROPBOX_API_ROOT,
		folder);

	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/fileops/create_folder");
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request->pData = mir_strdup(param);
	request->dataLength = param.GetLength();

	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	// forder exists on server
	if (response->resultCode == HTTP_STATUS_FORBIDDEN)
		return 0;

	return HandleHttpResponseError(hNetlibUser, response);
}

int CDropbox::CreateDownloadUrl(const char *path, wchar_t *url)
{
	CMStringA api_url = DROPBOX_API_URL;
	api_url.AppendFormat("/shares/%s/%s",
		DROPBOX_API_ROOT,
		path);

	if (!db_get_b(NULL, MODULE, "UseSortLinks", 1))
		api_url += "?short_url=false";

	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, api_url);
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));

	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	if (response && response->resultCode == HTTP_STATUS_OK)
	{
		JSONROOT root(response->pData);
		if (root)
		{
			JSONNODE *node = json_get(root, "url");
			wcscpy(url, json_as_string(node));
		}
	}

	return HandleHttpResponseError(hNetlibUser, response);
}

UINT CDropbox::SendFilesAsync(void *owner, void *arg)
{
	bool error = false;
	CDropbox *instance = (CDropbox*)owner;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ftp->hProcess, 0);

	if (ftp->pwszFolders)
	{
		for (int i = 0; ftp->pwszFolders[i]; i++)
		{
			ptrA utf8_folderName(mir_utf8encodeW(ftp->pwszFolders[i]));

			if (!instance->CreateFolder(utf8_folderName))
			{
				if (!strchr(utf8_folderName, '\\'))
				{
					wchar_t url[MAX_PATH];
					if (!instance->CreateDownloadUrl(utf8_folderName, url))
						ftp->AddUrl(url);
					else
					{
						error = true;
						break;
					}
				}
			}
			else
			{
				error = true;
				break;
			}
		}
	}

	if (!error)
	{
		for (int i = 0; ftp->pfts.pwszFiles[i]; i++)
		{
			FILE *file = _wfopen(ftp->pfts.pwszFiles[i], L"rb");
			if (file)
			{
				const wchar_t *fileName = NULL;
				if (!ftp->relativePathStart)
					fileName = wcsrchr(ftp->pfts.pwszFiles[i], L'\\') + 1;
				else
					fileName = &ftp->pfts.pwszFiles[i][ftp->relativePathStart];

				fseek(file, 0, SEEK_END);
				size_t fileSize = ftell(file);
				fseek(file, 0, SEEK_SET);

				ftp->pfts.currentFileNumber = i;
				ftp->pfts.currentFileSize = fileSize;
				ftp->pfts.currentFileProgress = 0;
				ftp->pfts.wszCurrentFile = wcsrchr(ftp->pfts.pwszFiles[i], '\\') + 1;

				ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftp->hProcess, (LPARAM)&ftp->pfts);

				//
				size_t offset = 0;
				char *uploadId = new char[32];

				int chunkSize = DROPBOX_FILE_CHUNK_SIZE / 4;
				if (fileSize < 1024 * 1024)
					chunkSize = DROPBOX_FILE_CHUNK_SIZE / 20;
				else if (fileSize > 20 * 1024 * 1024)
					chunkSize = DROPBOX_FILE_CHUNK_SIZE;

				while (!feof(file) && fileSize != offset)
				{
					if (ferror(file))
					{
						error = true;
						break;
					}

					char *data = new char[chunkSize + 1];
					int count = (int)fread(data, sizeof(char), chunkSize, file);

					if (!offset)
					{
						if (instance->SendFileChunkedFirst(data, count, uploadId, offset))
						{
							error = true;
							break;
						}
					}
					else
					{
						if (instance->SendFileChunkedNext(data, count, uploadId, offset))
						{
							error = true;
							break;
						}
					}

					ftp->pfts.currentFileProgress += count;
					ftp->pfts.totalProgress += count;

					ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftp->hProcess, (LPARAM)&ftp->pfts);
				}

				fclose(file);

				if (!error)
				{
					ptrA utf8_fileName(mir_utf8encodeW(fileName));

					if (instance->SendFileChunkedLast(utf8_fileName, uploadId))
					{
						error = true;
						break;
					}
					else
					{
						if (!wcschr(fileName, L'\\'))
						{
							wchar_t url[MAX_PATH];
							if (!instance->CreateDownloadUrl(utf8_fileName, url))
								ftp->AddUrl(url);
							else
							{
								error = true;
								break;
							}
						}

						ftp->pfts.currentFileProgress = ftp->pfts.currentFileSize;

						if (i < ftp->pfts.totalFiles - 1)
							ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ftp->hProcess, 0);
					}
				}
			}
			else
			{
				error = true;
				break;
			}
		}
	}

	if (error)
	{
		ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ftp->hProcess, 0);

		return 1;
	}

	ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ftp->hProcess, 0);

	return 0;
}

UINT CDropbox::SendFilesAndReportAsync(void *owner, void *arg)
{
	CDropbox *instance = (CDropbox*)owner;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	int res = SendFilesAsync(owner, arg);
	if (!res)
	{
		CMString urls;
		for (int i = 0; ftp->pwszUrls[i]; i++)
			urls.AppendFormat(L"%s\r\n", ftp->pwszUrls[i]);
		wchar_t *data = urls.GetBuffer();

		if (db_get_b(NULL, MODULE, "UrlAutoSend", 1))
		{
			char *message = mir_utf8encodeW(data);
			if (ftp->hContact != instance->GetDefaultContact())
			{
				if (CallContactService(ftp->hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)message) != ACKRESULT_FAILED)
				{
					DBEVENTINFO dbei = { sizeof(dbei) };
					dbei.flags = DBEF_UTF | DBEF_SENT/* | DBEF_READ*/;
					dbei.szModule = MODULE;
					dbei.timestamp = time(NULL);
					dbei.eventType = EVENTTYPE_MESSAGE;
					dbei.cbBlob = (int)wcslen(data);
					dbei.pBlob = (PBYTE)message;
					db_event_add(ftp->hContact, &dbei);
				}
				else CallServiceSync(MS_MSG_SENDMESSAGEW, (WPARAM)ftp->hContact, (LPARAM)data);
			}
			else
			{
				DBEVENTINFO dbei = { sizeof(dbei) };
				dbei.flags = DBEF_UTF;
				dbei.szModule = MODULE;
				dbei.timestamp = time(NULL);
				dbei.eventType = EVENTTYPE_MESSAGE;
				dbei.cbBlob = (int)wcslen(data);
				dbei.pBlob = (PBYTE)message;
				db_event_add(ftp->hContact, &dbei);
			}
		}

		if (db_get_b(NULL, MODULE, "UrlPasteToMessageInputArea", 0))
			CallServiceSync(MS_MSG_SENDMESSAGEW, (WPARAM)ftp->hContact, (LPARAM)data);

		if (db_get_b(NULL, MODULE, "UrlCopyToClipboard", 0))
		{
			if (OpenClipboard(NULL))
			{
				EmptyClipboard();
				size_t size = sizeof(wchar_t) * (urls.GetLength() + 1);
				HGLOBAL hClipboardData = GlobalAlloc(NULL, size);
				if (hClipboardData)
				{
					wchar_t *pchData = (wchar_t*)GlobalLock(hClipboardData);
					if (pchData)
					{
						memcpy(pchData, (wchar_t*)data, size);
						GlobalUnlock(hClipboardData);
						SetClipboardData(CF_UNICODETEXT, hClipboardData);
					}
				}
				CloseClipboard();
			}
		}
	}

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
	ti.data = ftp->pwszUrls;

	NotifyEventHooks(instance->hFileSentEventHook, ftp->hContact, (LPARAM)&ti);

	delete ftp;

	return res;
}