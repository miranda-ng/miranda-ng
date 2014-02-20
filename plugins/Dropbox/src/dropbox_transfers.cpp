#include "common.h"

HttpRequest *CDropbox::CreateFileSendChunkedRequest(const char *data, int length)
{
	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_PUT, DROPBOX_APICONTENT_URL "/chunked_upload");
	request->AddParameter("access_token", db_get_sa(NULL, MODULE, "TokenSecret"));
	if (length > 0)
	{
		request->AddHeader("Content-Type", "application/octet-stream");
		request->dataLength = length;
		request->pData = (char*)mir_alloc(sizeof(char) * (length + 1));
		memcpy(request->pData, data, length);
		request->pData[length] = 0;
	}

	return request;
}

void CDropbox::SendFileChunkedFirst(const char *data, int length, char *uploadId, int &offset)
{
	HttpRequest *request = CreateFileSendChunkedRequest(data, length);
	NETLIBHTTPREQUEST *response = request->Send();

	delete request;

	if (response)
	{
		if (response->resultCode == HttpStatus::OK)
		{
			JSONNODE *root = json_parse(response->pData);
			if (root != NULL)
			{
				JSONNODE *node = json_get(root, "upload_id");
				strcpy(uploadId, mir_u2a(json_as_string(node)));

				node = json_get(root, "offset");
				offset = json_as_int(node);

				delete node;
				delete root;
			}
		}

		mir_free(response);
	}
}

void CDropbox::SendFileChunkedNext(const char *data, int length, const char *uploadId, int &offset)
{
	HttpRequest *request = CreateFileSendChunkedRequest(data, length);
	request->AddParameter("upload_id", uploadId);
	request->AddParameter("offset", offset);

	NETLIBHTTPREQUEST *response = request->Send();

	delete request;

	if (response)
	{
		if (response->resultCode == HttpStatus::OK)
		{
			JSONNODE *root = json_parse(response->pData);
			if (root != NULL)
			{
				JSONNODE *node = json_get(root, "offset");
				offset = json_as_int(node);

				delete node;
				delete root;
			}
		}

		mir_free(response);
	}
}

void CDropbox::SendFileChunkedLast(const char *fileName, const char *uploadId, MCONTACT hContact)
{
	CMStringA url;
	url.AppendFormat(
		"%s/commit_chunked_upload/%s/%s",
		DROPBOX_APICONTENT_URL,
		DROPBOX_API_ROOT,
		fileName);
	url.Replace('\\', '/');

	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, url);
	request->AddParameter("upload_id", uploadId);
	request->AddParameter("access_token", db_get_sa(NULL, MODULE, "TokenSecret"));

	NETLIBHTTPREQUEST *response = request->Send();

	delete request;

	if (response)
	{
		if (response->resultCode == HttpStatus::OK && !strchr(fileName, '\\'))
		{
			url.Replace(DROPBOX_APICONTENT_URL, DROPBOX_API_URL);
			url.Replace("commit_chunked_upload", "shares");

			request = new HttpRequest(hNetlibUser, REQUEST_POST, url);
			request->AddParameter("access_token", db_get_sa(NULL, MODULE, "TokenSecret"));

			mir_free(response);

			response = request->Send();

			if (response)
			{
				if (response->resultCode == HttpStatus::OK)
				{
					JSONNODE *root = json_parse(response->pData);
					if (root != NULL)
					{
						JSONNODE *node = json_get(root, "url");
						char message[1024];
						mir_snprintf(
							message,
							SIZEOF(message),
							Translate("Link to download file \"%s\": %s"),
							fileName,
							mir_utf8encodeW(json_as_string(node)));

						DBEVENTINFO dbei = { sizeof(dbei) };
						dbei.szModule = MODULE;
						dbei.timestamp = time(NULL);
						dbei.eventType = EVENTTYPE_MESSAGE;
						dbei.cbBlob = strlen(message);
						dbei.pBlob = (PBYTE)mir_strdup(message);
						dbei.flags = DBEF_UTF;
						::db_event_add(hContact, &dbei);

						delete node;
						delete root;
					}
				}

				mir_free(response);
			}
		}
		else
			mir_free(response);
	}
}

void CDropbox::CreateFolder(const char *folderName, MCONTACT hContact)
{
	CMStringA folder = folderName;
	folder.Replace('\\', '/');

	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/fileops/create_folder");
	request->AddParameter("root", DROPBOX_API_ROOT);
	request->AddParameter("path", folderName);
	request->AddParameter("access_token", db_get_sa(NULL, MODULE, "TokenSecret"));

	NETLIBHTTPREQUEST *response = request->Send();

	delete request;

	if (response)
	{
		if (response->resultCode == HttpStatus::OK && !strchr(folderName, '\\'))
		{
			char url[MAX_PATH];
			mir_snprintf(
				url,
				SIZEOF(url),
				"%s/shares/%s/%s",
				DROPBOX_API_URL,
				DROPBOX_API_ROOT,
				folder.GetBuffer());

			request = new HttpRequest(hNetlibUser, REQUEST_POST, url);
			request->AddParameter("access_token", db_get_sa(NULL, MODULE, "TokenSecret"));

			mir_free(response);

			response = request->Send();

			if (response)
			{
				if (response->resultCode == HttpStatus::OK)
				{
					JSONNODE *root = json_parse(response->pData);
					if (root != NULL)
					{
						JSONNODE *node = json_get(root, "url");
						char message[1024];
						mir_snprintf(
							message,
							SIZEOF(message),
							Translate("Link to download folder \"%s\": %s"),
							folderName,
							mir_utf8encodeW(json_as_string(node)));

						DBEVENTINFO dbei = { sizeof(dbei) };
						dbei.szModule = MODULE;
						dbei.timestamp = time(NULL);
						dbei.eventType = EVENTTYPE_MESSAGE;
						dbei.cbBlob = strlen(message);
						dbei.pBlob = (PBYTE)mir_strdup(message);
						dbei.flags = DBEF_UTF;
						::db_event_add(hContact, &dbei);

						delete node;
						delete root;
					}
				}

				mir_free(response);
			}
		}
		else
			mir_free(response);
	}
}

void _cdecl CDropbox::SendFileAsync(void *arg)
{
	FileTransferParam *ftp = (FileTransferParam *)arg;

	ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ftp->hProcess, 0);

	for (int i = 0; ftp->pszFolders[i]; i++)
		Singleton<CDropbox>::GetInstance()->CreateFolder(ftp->pszFolders[i], ftp->pfts.hContact);

	for (int i = 0; ftp->pfts.pszFiles[i]; i++)
	{
		FILE *file = fopen(ftp->pfts.pszFiles[i], "rb");
		if (file != NULL)
		{
			int offset = 0;
			char *uploadId = new char[32];

			//const char *fileName = strrchr(ftp->pfts.pszFiles[i], '\\') + 1;
			const char *fileName = &ftp->pfts.pszFiles[i][ftp->relativePathStart];

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
				int chunkSize = DROPBOX_FILE_CHUNK_SIZE;
				if (fileSize < 1024*1024)
					chunkSize = DROPBOX_FILE_CHUNK_SIZE / 5;
				else if (fileSize > 20*1024*1024)
					chunkSize = DROPBOX_FILE_CHUNK_SIZE * 4;

				char *data = new char[chunkSize + 1];
				size_t count = fread(data, sizeof(char), chunkSize, file);

				if (!offset)
					Singleton<CDropbox>::GetInstance()->SendFileChunkedFirst(data, count, uploadId, offset);
				else
					Singleton<CDropbox>::GetInstance()->SendFileChunkedNext(data, count, uploadId, offset);

				ftp->pfts.currentFileProgress += count;
				ftp->pfts.totalProgress += count;

				ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftp->hProcess, (LPARAM)&ftp->pfts);
			}

			fclose(file);

			Singleton<CDropbox>::GetInstance()->SendFileChunkedLast(fileName, uploadId, ftp->pfts.hContact);
			ftp->pfts.currentFileProgress = ftp->pfts.currentFileSize;

			if (i < ftp->pfts.totalFiles - 1)
				ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ftp->hProcess, 0);
		}
	}

	ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ftp->hProcess, 0);

	delete ftp;
}