#ifndef _GDRIVESERVICE_API_H_
#define _GDRIVESERVICE_API_H_

namespace GDriveAPI
{
#define GOOGLE_OAUTH "https://accounts.google.com/o/oauth2/v2"
#define GDRIVE_API "https://www.googleapis.com/drive/v3/files"
#define GDRIVE_UPLOAD "https://www.googleapis.com/upload/drive/v3/files"
#define GDRIVE_SHARE "https://drive.google.com/open?id=%s"

#define GOOGLE_APP_ID "528761318515-9hp30q3pcsk7c3qhbajs5ntvi7aiqp0b.apps.googleusercontent.com"
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
		StartUploadFileRequest(const char *token, const char *name) :
			HttpRequest(REQUEST_POST, GDRIVE_UPLOAD)
		{
			AddUrlParameter("access_token=%s", token);
			AddUrlParameter("uploadType=resumable");

			AddHeader("Content-Type", "application/json");

			JSONNode params(JSON_NODE);
			params << JSONNode("name", name);

			json_string data = params.write();
			SetData(data.c_str(), data.length());
		}
	};

	class UploadFileRequest : public HttpRequest
	{
	public:
		UploadFileRequest(const char *uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize) :
			HttpRequest(REQUEST_PUT, uploadUri)
		{
			uint64_t rangeMin = offset;
			uint64_t rangeMax = offset + chunkSize - 1;
			CMStringA range(CMStringDataFormat::FORMAT, "bytes %I64u-%I64u/%I64u", rangeMin, rangeMax, fileSize);
			AddHeader("Content-Range", range);

			SetData(chunk, chunkSize);
		}
	};

	class CreateFolderRequest : public HttpRequest
	{
	public:
		CreateFolderRequest(const char *token, const char *path) :
			HttpRequest(REQUEST_PUT, GDRIVE_API)
		{
			AddUrlParameter("access_token=%s", token);
			AddHeader("Content-Type", "application/json");

			JSONNode params(JSON_NODE);
			params
				<< JSONNode("name", path)
				<< JSONNode("mimeType", "pplication/vnd.google-apps.folder");

			json_string data = params.write();
			SetData(data.c_str(), data.length());
		}
	};

	class GrantPermissionsRequest : public HttpRequest
	{
	public:
		GrantPermissionsRequest(const char *token, const char *fileId) :
			HttpRequest(REQUEST_POST, FORMAT, GDRIVE_API "/%s/permissions", fileId)
		{
			AddUrlParameter("access_token=%s", token);

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
