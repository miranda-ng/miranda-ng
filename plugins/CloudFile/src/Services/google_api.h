#ifndef _GDRIVESERVICE_API_H_
#define _GDRIVESERVICE_API_H_

namespace GDriveAPI
{
#define GOOGLE_OAUTH "https://accounts.google.com/o/oauth2/v2"
#define GDRIVE_API "https://www.googleapis.com/drive/v2/files"

#define GOOGLE_APP_ID "271668553802-3sd3tubkf165ibgrqnrhe3id8mcgnaf7.apps.googleusercontent.com"
#include "../../../miranda-private-keys/Google/client_secret.h"

	class GetAccessTokenRequest : public HttpRequest
	{
	public:
		GetAccessTokenRequest(const char *code) :
			HttpRequest(REQUEST_POST, "https://www.googleapis.com/oauth2/v4/token")
		{
			AddHeader("Content-Type", "application/x-www-form-urlencoded");

			CMStringA data(CMStringDataFormat::FORMAT,
				"redirect_uri=urn:ietf:wg:oauth:2.0:oob&client_id=%s&client_secret=%s&grant_type=authorization_code&code=%s",
				GOOGLE_APP_ID, GOOGLE_CLIENT_SECRET, code);
			SetData(data.GetBuffer(), data.GetLength());
		}
	};

	class RevokeAccessTokenRequest : public HttpRequest
	{
	public:
		RevokeAccessTokenRequest(const char *token) :
			HttpRequest(REQUEST_POST, GOOGLE_OAUTH "/revoke")
		{
			AddUrlParameter("token=%s", token);
		}
	};

	class StartUploadFileRequest : public HttpRequest
	{
	public:
		StartUploadFileRequest(const char *token) :
			HttpRequest(REQUEST_POST, GDRIVE_API)
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");
			AddUrlParameter("uploadType=resumable");
		}
	};

	class UploadFileRequest : public HttpRequest
	{
	public:
		UploadFileRequest(const char *token, const char *data, size_t size) :
			HttpRequest(REQUEST_POST, GDRIVE_API)
		{
			AddBearerAuthHeader(token);
			AddUrlParameter("uploadType=resumable");

			SetData(data, size);
		}
	};

	class CreateFolderRequest : public HttpRequest
	{
	public:
		CreateFolderRequest(const char *token, const char *path) :
			HttpRequest(REQUEST_PUT, GDRIVE_API)
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode params(JSON_NODE);
			params
				<< JSONNode("name", path)
				<< JSONNode("mimeType", "pplication/vnd.google-apps.folder");

			json_string data = params.write();
			SetData(data.c_str(), data.length());
		}
	};

	class ShareRequest : public HttpRequest
	{
	public:
		ShareRequest(const char *token, const char *fileId) :
			HttpRequest(REQUEST_PUT, FORMAT, GDRIVE_API "/%s/permissions", fileId)
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode params(JSON_NODE);
			params
				<< JSONNode("role", "reader")
				<< JSONNode("type", "anyone");

			json_string data = params.write();
			SetData(data.c_str(), data.length());
		}
	};
};

#endif //_GDRIVESERVICE_API_H_
