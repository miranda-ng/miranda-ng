#ifndef _GDRIVESERVICE_API_H_
#define _GDRIVESERVICE_API_H_

// https://developers.google.com/drive/v3/reference/
namespace GDriveAPI
{
#define GOOGLE_OAUTH "https://accounts.google.com/o/oauth2/v2"
#define GOOGLE_API "https://www.googleapis.com"
#define GDRIVE_API_OAUTH GOOGLE_API "/oauth2/v4"
#define GDRIVE_API_VER "/v3"
#define GDRIVE_API GOOGLE_API "/drive" GDRIVE_API_VER "/files"
#define GDRIVE_UPLOAD GOOGLE_API "/upload/drive" GDRIVE_API_VER "/files"
#define GDRIVE_SHARE "https://drive.google.com/open?id="

#include "../../../miranda-private-keys/Google/client_secret.h"

#define GOOGLE_AUTH GOOGLE_OAUTH "/auth?response_type=code&scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fdrive.file&access_type=offline&prompt=consent&redirect_uri=https%3A%2F%2Foauth.miranda-ng.org%2Fverification&client_id=" GOOGLE_APP_ID

	class GetAccessTokenRequest : public HttpRequest
	{
	public:
		GetAccessTokenRequest(const char *code) :
			HttpRequest(REQUEST_POST, GDRIVE_API_OAUTH "/token")
		{
			AddHeader("Content-Type", "application/x-www-form-urlencoded");

			CMStringA data = "redirect_uri=https://oauth.miranda-ng.org/verification";
			data.AppendFormat("&client_id=%s&client_secret=%s", GOOGLE_APP_ID, GOOGLE_CLIENT_SECRET);
			data.AppendFormat("&grant_type=authorization_code&code=%s", code);
			SetData(data.GetBuffer(), data.GetLength());
		}
	};

	class RefreshTokenRequest : public HttpRequest
	{
	public:
		RefreshTokenRequest(const char *refreshToken) :
			HttpRequest(REQUEST_POST, GDRIVE_API_OAUTH "/token")
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
			AddUrlParameter("fields=id");

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
				body.AppendFormat(", \"parents\": [\"%s\"]", parentId);
			body.Append("}");
			body.AppendChar(0x0A);
			body.AppendChar(0x0A);
			body.Append("--upload");
			body.AppendChar(0x0A);
			body.Append("Content-Type: application/octet-stream");
			body.AppendChar(0x0A);
			body.Append("Content-Transfer-Encoding: base64");
			body.AppendChar(0x0A);
			body.AppendChar(0x0A);
			body.Append(ptrA(mir_base64_encode(data, size)));
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
			parents.set_name("parents");

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
			AddUrlParameter("fields=id");

			uint64_t rangeMin = offset;
			uint64_t rangeMax = offset + chunkSize - 1;
			CMStringA range(CMStringDataFormat::FORMAT, "bytes %I64u-%I64u/%I64u", rangeMin, rangeMax, fileSize);
			AddHeader("Content-Range", range);

			SetData(chunk, chunkSize);
		}
	};

	class GetFolderRequest : public HttpRequest
	{
	public:
		GetFolderRequest(const char *token, const char *parentId, const char *name) :
			HttpRequest(REQUEST_GET, GDRIVE_API)
		{
			AddUrlParameterWithEncode("q", "mimeType = 'application/vnd.google-apps.folder' and trashed = false and '%s' in parents and name = '%s'", mir_strlen(parentId) ? parentId : "root", name);
			AddUrlParameterWithEncode("fields", "files(id)");

			AddBearerAuthHeader(token);
		}
	};

	class CreateFolderRequest : public HttpRequest
	{
	public:
		CreateFolderRequest(const char *token, const char *parentId, const char *name) :
			HttpRequest(REQUEST_POST, GDRIVE_API)
		{
			AddUrlParameter("fields=id");

			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode parents(JSON_ARRAY);
			parents.set_name("parents");
			parents.push_back(JSONNode("", parentId));

			JSONNode params(JSON_NODE);
			params
				<< JSONNode("name", name)
				<< JSONNode("mimeType", "application/vnd.google-apps.folder")
				<< parents;

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
			AddUrlParameter("fields=id");

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
