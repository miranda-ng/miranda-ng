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

	class RefreshTokenRequest : public HttpRequest
	{
	public:
		RefreshTokenRequest(const char *refreshToken) :
			HttpRequest(REQUEST_POST, "https://www.googleapis.com/oauth2/v4/token")
		{
			AddHeader("Content-Type", "application/x-www-form-urlencoded");

			CMStringA data(CMStringDataFormat::FORMAT,
				"client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s",
				GOOGLE_APP_ID, GOOGLE_CLIENT_SECRET, refreshToken);
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

	class UploadFileRequest : public HttpRequest
	{
	public:
		UploadFileRequest(const char *token, const char *parentId, const char *name, const char *data, size_t size) :
			HttpRequest(REQUEST_POST, GDRIVE_UPLOAD)
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "multipart/related; boundary=upload");

			CMStringA body = "--upload";
			body.AppendChar(0x0A);
			body.Append("Content-Type: application/json");
			body.AppendChar(0x0A);
			body.AppendChar(0x0A);
			body.Append("{");
			body.AppendFormat("\"name\": \"%s\"", name);
			if (mir_strlen(parentId))
				body.AppendFormat("\"parents\": [\"%s\"]", parentId);
			body.Append("}");
			body.AppendChar(0x0A);
			body.AppendChar(0x0A);
			body.Append("--upload");
			body.AppendChar(0x0A);
			body.Append("Content-Type: application/octet-stream");
			body.AppendChar(0x0A);
			body.AppendChar(0x0A);
			body.Append(data, size);
			body.AppendChar(0x0A);
			body.Append("--upload--");

			SetData(body.GetBuffer(), body.GetLength());
		}
	};

	class CreateUploadSessionRequest : public HttpRequest
	{
	public:
		CreateUploadSessionRequest(const char *token, const char *parentId, const char *name) :
			HttpRequest(REQUEST_POST, GDRIVE_UPLOAD)
		{
			AddUrlParameter("uploadType=resumable");

			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode parents(JSON_ARRAY);
			parents << JSONNode("", parentId);

			JSONNode params(JSON_NODE);
			params << JSONNode("name", name);
			params << parents;

			json_string data = params.write();
			SetData(data.c_str(), data.length());
		}
	};

	class UploadFileChunkRequest : public HttpRequest
	{
	public:
		UploadFileChunkRequest(const char *uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize):
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

	class GrantPermissionsRequest : public HttpRequest
	{
	public:
		GrantPermissionsRequest(const char *token, const char *fileId) :
			HttpRequest(REQUEST_POST, FORMAT, GDRIVE_API "/%s/permissions", fileId)
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
