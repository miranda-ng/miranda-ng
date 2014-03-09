#include "common.h"

int CDropbox::HandleFileTransferError(HANDLE hNetlibUser, NETLIBHTTPREQUEST *response)
{
	if (!response)
	{
		Netlib_Logf(hNetlibUser, "%s", "Server does not respond");
		//CDropbox::ShowNotification(TranslateT("Server does not respond"), MB_ICONERROR);
		return ACKRESULT_FAILED;
	}

	if (response->resultCode != HTTP_STATUS_OK)
	{
		Netlib_Logf(hNetlibUser, "%s", HttpStatusToText((HTTP_STATUS)response->resultCode));
		//CDropbox::ShowNotification(TranslateTS(HttpStatusToText((HTTP_STATUS)response->resultCode)), MB_ICONERROR);
		return response->resultCode;
	}

	return 0;
}

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

	return HandleFileTransferError(hNetlibUser, response);
}

int CDropbox::SendFileChunkedFirst(const char *data, int length, char *uploadId, int &offset)
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
		JSONNODE *root = json_parse(response->pData);
		if (root)
		{
			JSONNODE *node = json_get(root, "upload_id");
			strcpy(uploadId, mir_u2a(json_as_string(node)));

			node = json_get(root, "offset");
			offset = json_as_int(node);

			return 0;
		}

		return HandleFileTransferError(hNetlibUser, response);
	}

	return HandleFileTransferError(hNetlibUser, response);
}

int CDropbox::SendFileChunkedNext(const char *data, int length, const char *uploadId, int &offset)
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
		JSONNODE *root = json_parse(response->pData);
		if (root)
		{
			JSONNODE *node = json_get(root, "offset");
			offset = json_as_int(node);

			return 0;
		}

		return HandleFileTransferError(hNetlibUser, response);
	}

	return HandleFileTransferError(hNetlibUser, response);
}

int CDropbox::SendFileChunkedLast(const char *fileName, const char *uploadId)
{
	char *utf8_fileName = mir_utf8encode(fileName);

	CMStringA url;
	url.AppendFormat(
		"%s/commit_chunked_upload/%s/%s",
		DROPBOX_APICONTENT_URL,
		DROPBOX_API_ROOT,
		utf8_fileName);
	url.Replace('\\', '/');

	mir_free(utf8_fileName);

	CMStringA param = CMStringA("upload_id=") + uploadId;

	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, url);
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request->pData = mir_strdup(param);
	request->dataLength = param.GetLength();

	mir_ptr<NETLIBHTTPREQUEST> response(request->Send());

	delete request;

	//if (response && response->resultCode == HTTP_STATUS_OK)
	//{
	//	if (!strchr(fileName, '\\'))
	//	{
	//		url.Replace(DROPBOX_APICONTENT_URL, DROPBOX_API_URL);
	//		url.Replace("commit_chunked_upload", "shares");

	//		request = new HttpRequest(hNetlibUser, REQUEST_POST, url);
	//		request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	//		if (!db_get_b(NULL, MODULE, "UseSortLinks", 1))
	//		{
	//			request->pData = mir_strdup("short_url=false");
	//			request->dataLength = strlen(request->pData);
	//		}

	//		response = request->Send();

	//		delete request;

	//		if (response &&response->resultCode == HTTP_STATUS_OK)
	//		{
	//			JSONNODE *root = json_parse(response->pData);
	//			if (root)
	//			{
	//				JSONNODE *node = json_get(root, "url");

	//				char message[1024];
	//				mir_snprintf(
	//					message,
	//					SIZEOF(message),
	//					Translate("Link to download file \"%s\": %s"),
	//					fileName,
	//					mir_u2a(json_as_string(node)));

	//				if (hContact != CDropbox::GetDefaultContact())
	//				{
	//					if (CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)&message) != ACKRESULT_FAILED)
	//					{
	//						DBEVENTINFO dbei = { sizeof(dbei) };
	//						dbei.szModule = MODULE;
	//						dbei.timestamp = time(NULL);
	//						dbei.eventType = EVENTTYPE_MESSAGE;
	//						dbei.cbBlob = strlen(message);
	//						dbei.pBlob = (PBYTE)message;
	//						dbei.flags = DBEF_SENT | DBEF_UTF;
	//						db_event_add(hContact, &dbei);
	//					}
	//				}
	//				else
	//				{
	//					DBEVENTINFO dbei = { sizeof(dbei) };
	//					dbei.szModule = MODULE;
	//					dbei.timestamp = time(NULL);
	//					dbei.eventType = EVENTTYPE_MESSAGE;
	//					dbei.cbBlob = strlen(message);
	//					dbei.pBlob = (PBYTE)message;
	//					dbei.flags = DBEF_UTF;
	//					db_event_add(hContact, &dbei);
	//				}

	//				return 0;
	//			}

	//			return HandleFileTransferError(hNetlibUser, response);
	//		}

	//		return HandleFileTransferError(hNetlibUser, response);
	//	}
	//}

	return HandleFileTransferError(hNetlibUser, response);
}

int CDropbox::CreateFolder(const char *folderName)
{
	char *utf8_folderName = mir_utf8encode(folderName);

	CMStringA folder = utf8_folderName;
	folder.Replace('\\', '/');

	mir_free(utf8_folderName);

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

	//if (response && (response->resultCode == HTTP_STATUS_OK || response->resultCode == HTTP_STATUS_FORBIDDEN))
	//{
	//	if (!strchr(folderName, '\\'))
	//	{
	//		CMStringA url = DROPBOX_API_URL;
	//		url.AppendFormat("/shares/%s/%s",
	//			DROPBOX_API_ROOT,
	//			folder);

	//		request = new HttpRequest(hNetlibUser, REQUEST_POST, url);
	//		request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	//		if (!db_get_b(NULL, MODULE, "UseSortLinks", 1))
	//		{
	//			request->pData = mir_strdup("short_url=false");
	//			request->dataLength = strlen(request->pData);
	//		}

	//		mir_free(response);

	//		response = request->Send();

	//		if (response && response->resultCode == HTTP_STATUS_OK)
	//		{
	//			JSONNODE *root = json_parse(response->pData);
	//			if (root != NULL)
	//			{
	//				JSONNODE *node = json_get(root, "url");
	//				char message[1024];
	//				mir_snprintf(
	//					message,
	//					SIZEOF(message),
	//					Translate("Link to download folder \"%s\": %s"),
	//					folderName,
	//					mir_u2a(json_as_string(node)));

	//				if (hContact != CDropbox::GetDefaultContact())
	//				{
	//					if (CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)&message) != ACKRESULT_FAILED)
	//					{
	//						DBEVENTINFO dbei = { sizeof(dbei) };
	//						dbei.szModule = MODULE;
	//						dbei.timestamp = time(NULL);
	//						dbei.eventType = EVENTTYPE_MESSAGE;
	//						dbei.cbBlob = strlen(message);
	//						dbei.pBlob = (PBYTE)message;
	//						dbei.flags = DBEF_SENT | DBEF_UTF;
	//						db_event_add(hContact, &dbei);
	//					}
	//				}
	//				else
	//				{
	//					DBEVENTINFO dbei = { sizeof(dbei) };
	//					dbei.szModule = MODULE;
	//					dbei.timestamp = time(NULL);
	//					dbei.eventType = EVENTTYPE_MESSAGE;
	//					dbei.cbBlob = strlen(message);
	//					dbei.pBlob = (PBYTE)message;
	//					dbei.flags = DBEF_UTF;
	//					db_event_add(hContact, &dbei);
	//				}

	//				return 0;
	//			}

	//			return HandleFileTransferError(hNetlibUser, response);
	//		}

	//		return HandleFileTransferError(hNetlibUser, response);
	//	}
	//}

	return HandleFileTransferError(hNetlibUser, response);
}

int CDropbox::CreateDownloadUrl(const char *path, char *url)
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
		JSONNODE *root = json_parse(response->pData);
		if (root != NULL)
		{
			JSONNODE *node = json_get(root, "url");
			strcpy(url, mir_u2a(json_as_string(node)));

			return 0;
		}

		return HandleFileTransferError(hNetlibUser, response);
	}

	return HandleFileTransferError(hNetlibUser, response);
}

void _cdecl CDropbox::SendFileAsync(void *arg)
{
	bool error = false;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	CMStringA urls;

	ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ftp->hProcess, 0);

	for (int i = 0; ftp->pszFolders[i]; i++)
	{
		if (!ftp->instance->CreateFolder(ftp->pszFolders[i]))
		{
			if (!strchr(ftp->pszFolders[i], '\\'))
			{
				char url[MAX_PATH];
				if (!ftp->instance->CreateDownloadUrl(ftp->pszFolders[i], url))
				{
					urls += url;
					urls += "\r\n";
				}
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

	if (!error)
	{
		for (int i = 0; ftp->pfts.pszFiles[i]; i++)
		{
			FILE *file = fopen(ftp->pfts.pszFiles[i], "rb");
			if (file != NULL)
			{
				int offset = 0;
				char *uploadId = new char[32];

				const char *fileName = NULL;
				if (!ftp->relativePathStart)
					fileName = strrchr(ftp->pfts.pszFiles[i], '\\') + 1;
				else
					fileName = &ftp->pfts.pszFiles[i][ftp->relativePathStart];

				fseek(file, 0, SEEK_END);
				DWORD fileSize = ftell(file);
				fseek(file, 0, SEEK_SET);

				ftp->pfts.currentFileNumber = i;
				ftp->pfts.currentFileSize = fileSize;
				ftp->pfts.currentFileProgress = 0;
				ftp->pfts.szCurrentFile = strrchr(ftp->pfts.pszFiles[i], '\\') + 1;

				ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftp->hProcess, (LPARAM)&ftp->pfts);

				while (!feof(file) && !ferror(file))
				{
					int chunkSize = DROPBOX_FILE_CHUNK_SIZE / 4;
					if (fileSize < 1024 * 1024)
						chunkSize = DROPBOX_FILE_CHUNK_SIZE / 20;
					else if (fileSize > 20 * 1024 * 1024)
						chunkSize = DROPBOX_FILE_CHUNK_SIZE;

					char *data = new char[chunkSize + 1];
					int count = (int)fread(data, sizeof(char), chunkSize, file);

					if (!offset)
					{
						if (ftp->instance->SendFileChunkedFirst(data, count, uploadId, offset))
						{
							error = true;
							break;
						}
					}
					else
					{
						if (ftp->instance->SendFileChunkedNext(data, count, uploadId, offset))
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
					if (ftp->instance->SendFileChunkedLast(fileName, uploadId))
					{
						error = true;
						break;
					}
					else
					{
						if (!strchr(fileName, '\\'))
						{
							char url[MAX_PATH];
							if (!ftp->instance->CreateDownloadUrl(fileName, url))
							{
								urls += url;
								urls += "\r\n";
							}
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
		}
	}

	if (ftp->instance->hTransferContact)
		ftp->instance->hTransferContact = 0;

	if (!error)
	{
		ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ftp->hProcess, 0);

		if (db_get_b(NULL, MODULE, "UrlAutoSend", 0))
		{
			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.szModule = MODULE;
			dbei.flags = DBEF_UTF;
			dbei.timestamp = time(NULL);
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.cbBlob = urls.GetLength();
			dbei.pBlob = (PBYTE)urls.GetBuffer();

			if (ftp->hContact != ftp->instance->GetDefaultContact())
			{
				CallContactService(ftp->hContact, PSS_MESSAGE, 0, (LPARAM)urls.GetBuffer());
				dbei.flags |= DBEF_SENT | DBEF_READ;
			}

			db_event_add(ftp->hContact, &dbei);
		}
		else if (db_get_b(NULL, MODULE, "UrlPasteToMessageLog", 1))
			CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)ftp->hContact, (LPARAM)urls.GetBuffer());

		if (db_get_b(NULL, MODULE, "UrlCopyToClipboard", 1))
		{
			int length = urls.GetLength() + 1;
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, length);
			memcpy(GlobalLock(hMem), urls.GetBuffer(), length);
			GlobalUnlock(hMem);
			OpenClipboard(0);
			EmptyClipboard();
			SetClipboardData(CF_TEXT, hMem);
			CloseClipboard();
		}
	}
	else
		ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ftp->hProcess, 0);

	delete ftp;
}