#ifndef _DROPBOX_API_OPERATIONS_H_
#define _DROPBOX_API_OPERATIONS_H_

class ShareRequest : public HttpRequest
{
public:
	ShareRequest(const char *token, const char *path, bool useShortUrl, const char *root = "auto") :
		HttpRequest(REQUEST_POST, FORMAT, DROPBOX_API_URL "/shares/%s/%s", root, path)
	{
		if (!useShortUrl)
			AddUrlParameter("short_url=false");

		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

class DeleteRequest : public HttpRequest
{
public:
	DeleteRequest(const char *token, const char *path, const char *root = "auto") :
		HttpRequest(REQUEST_POST, DROPBOX_API_URL "/fileops/delete")
	{
		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/x-www-form-urlencoded");

		CMStringA data(CMStringDataFormat::FORMAT, "root=%s&path=%s", root, path);
		data.Replace('\\', '/');
		SetData(data.GetBuffer(), data.GetLength());
	}
};

class CreateFolderRequest : public HttpRequest
{
public:
	CreateFolderRequest(const char *token, const char *path, const char *root = "auto") :
		HttpRequest(REQUEST_POST, DROPBOX_API_URL "/fileops/create_folder")
	{
		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/x-www-form-urlencoded");

		CMStringA data(CMStringDataFormat::FORMAT, "root=%s&path=%s", root, path);
		data.Replace('\\', '/');
		SetData(data.GetBuffer(), data.GetLength());
	}
};

class GetMetadataRequest : public HttpRequest
{
public:
	GetMetadataRequest(const char *token, const char *path, const char *root = "auto") :
		HttpRequest(REQUEST_GET, FORMAT, DROPBOX_API_URL "/metadata/%s/%s", root, path)
	{
		AddBearerAuthHeader(token);
	}
};

#endif //_DROPBOX_API_OPERATIONS_H_
