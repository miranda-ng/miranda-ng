#include "common.h"

void CDropbox::SendFileChunkedFirst(const char *data, int length, char *uploadId, int &offset)
{
	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_PUT, DROPBOX_APICONTENT_URL "/chunked_upload");
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	request->AddHeader("Content-Type", "application/octet-stream");
	request->pData = (char*)mir_alloc(sizeof(char) * length );
	memcpy(request->pData, data, length);
	request->dataLength = length;

	NETLIBHTTPREQUEST *response = request->Send();

	delete request;

	if (response)
	{
		if (response->resultCode == HTTP_STATUS::OK)
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
	CMStringA url = DROPBOX_APICONTENT_URL "/chunked_upload";
	url.AppendFormat("?upload_id=%s&offset=%i", uploadId, offset);

	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_PUT, url);
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	request->AddHeader("Content-Type", "application/octet-stream");
	request->pData = (char*)mir_alloc(sizeof(char) * length);
	memcpy(request->pData, data, length);
	request->dataLength = length;

	NETLIBHTTPREQUEST *response = request->Send();

	delete request;

	if (response)
	{
		if (response->resultCode == HTTP_STATUS::OK)
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

	CMStringA param = CMStringA("upload_id=") + uploadId;

	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, url);
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request->pData = mir_strdup(param);
	request->dataLength = param.GetLength();

	NETLIBHTTPREQUEST *response = request->Send();

	delete request;

	if (response)
	{
		if (response->resultCode == HTTP_STATUS::OK && !strchr(fileName, '\\'))
		{
			url.Replace(DROPBOX_APICONTENT_URL, DROPBOX_API_URL);
			url.Replace("commit_chunked_upload", "shares");

			request = new HttpRequest(hNetlibUser, REQUEST_POST, url);
			request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));

			mir_free(response);

			response = request->Send();

			if (response)
			{
				if (response->resultCode == HTTP_STATUS::OK)
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

						CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)&message);

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

	CMStringA param;
	param.AppendFormat("root=%s&path=%s",
		DROPBOX_API_ROOT,
		folder);

	HttpRequest *request = new HttpRequest(hNetlibUser, REQUEST_POST, DROPBOX_API_URL "/fileops/create_folder");
	request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));
	request->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request->pData = mir_strdup(param);
	request->dataLength = param.GetLength();

	NETLIBHTTPREQUEST *response = request->Send();

	delete request;

	if (response)
	{
		if (response->resultCode == HTTP_STATUS::OK && !strchr(folderName, '\\'))
		{
			CMStringA url = DROPBOX_API_URL;
			url.AppendFormat("/shares/%s/%s",
				DROPBOX_API_ROOT,
				folder);

			request = new HttpRequest(hNetlibUser, REQUEST_POST, url);
			request->AddBearerAuthHeader(db_get_sa(NULL, MODULE, "TokenSecret"));

			mir_free(response);

			response = request->Send();

			if (response)
			{
				if (response->resultCode == HTTP_STATUS::OK)
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

						CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)&message);

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
	FileTransfer *ftp = (FileTransfer*)arg;

	ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ftp->hProcess, 0);

	for (int i = 0; ftp->pszFolders[i]; i++)
		INSTANCE->CreateFolder(ftp->pszFolders[i], ftp->hContact);

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
				int chunkSize = DROPBOX_FILE_CHUNK_SIZE;
				if (fileSize < 1024*1024)
					chunkSize = DROPBOX_FILE_CHUNK_SIZE / 5;
				else if (fileSize > 20*1024*1024)
					chunkSize = DROPBOX_FILE_CHUNK_SIZE * 4;

				char *data = new char[chunkSize + 1];
				int count = fread(data, sizeof(char), chunkSize, file);

				if (!offset)
					INSTANCE->SendFileChunkedFirst(data, count, uploadId, offset);
				else
					INSTANCE->SendFileChunkedNext(data, count, uploadId, offset);

				ftp->pfts.currentFileProgress += count;
				ftp->pfts.totalProgress += count;

				ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ftp->hProcess, (LPARAM)&ftp->pfts);
			}

			fclose(file);

			INSTANCE->SendFileChunkedLast(fileName, uploadId, ftp->hContact);
			ftp->pfts.currentFileProgress = ftp->pfts.currentFileSize;

			if (i < ftp->pfts.totalFiles - 1)
				ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ftp->hProcess, 0);
		}
	}

	if (INSTANCE->hContactTransfer)
		INSTANCE->hContactTransfer = 0;

	ProtoBroadcastAck(MODULE, ftp->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ftp->hProcess, 0);

	delete ftp;
}