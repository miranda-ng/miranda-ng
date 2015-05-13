#ifndef _DROPBOX_API_UPLOAD_H_
#define _DROPBOX_API_UPLOAD_H_

class UploadFileRequest : public HttpRequest
{
public:
	UploadFileRequest(const char *token, const char *fileName, const char *data, int length, const char *root = "auto") :
		HttpRequest(REQUEST_PUT, FORMAT, DROPBOX_APICONTENT_URL "/files_put/%s/%s", root, fileName)
	{
		AddBearerAuthHeader(token);
		pData = (char*)data;
		dataLength = length;
	}
};

class UploadFileChunkRequest : public HttpRequest
{
public:
	UploadFileChunkRequest(const char *token, const char *data, int length) :
		HttpRequest(REQUEST_PUT, DROPBOX_APICONTENT_URL "/chunked_upload")
	{
		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/octet-stream");
		pData = (char*)data;
		dataLength = length;
	}

	UploadFileChunkRequest(const char *token, const char *uploadId, size_t offset, const char *data, int length) :
		HttpRequest(REQUEST_PUT, FORMAT, DROPBOX_APICONTENT_URL "/chunked_upload?upload_id=%s&offset=%i", uploadId, offset)
	{
		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/octet-stream");
		pData = (char*)data;
		dataLength = length;
	}

	UploadFileChunkRequest(const char *token, const char *uploadId, const char *path, const char *root = "auto") :
		HttpRequest(REQUEST_POST, FORMAT, DROPBOX_APICONTENT_URL "/commit_chunked_upload/%s/%s", root, path)
	{
		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
		AddUrlParameter("upload_id=%s", uploadId);
	}
};

#endif //_DROPBOX_API_UPLOAD_H_
