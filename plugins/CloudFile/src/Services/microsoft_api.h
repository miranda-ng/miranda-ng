#ifndef _ONEDRIVESERVICE_API_H_
#define _ONEDRIVESERVICE_API_H_

namespace OneDriveAPI
{
#define MS_OAUTH "https://login.microsoftonline.com/common/oauth2/v2.0"
#define ONEDRIVE_API "https://graph.microsoft.com/v1.0/drive"

#define MS_APP_ID "72b87ac7-42eb-4a97-a620-91a7f8d8b5ae"

	class GetAccessTokenRequest : public HttpRequest
	{
	public:
		GetAccessTokenRequest(const char *code) :
			HttpRequest(REQUEST_POST, MS_OAUTH "/token")
		{
			AddHeader("Content-Type", "application/x-www-form-urlencoded");

			CMStringA data = "redirect_uri=https%3A%2F%2Flogin.microsoftonline.com%2Fcommon%2Foauth2%2Fnativeclient";
			data.AppendFormat("&client_id=%s&grant_type=authorization_code&code=%s", MS_APP_ID, code);
			SetData(data.GetBuffer(), data.GetLength());
		}
	};

	class RefreshTokenRequest : public HttpRequest
	{
	public:
		RefreshTokenRequest(const char *refreshToken) :
			HttpRequest(REQUEST_POST, MS_OAUTH "/token")
		{
			AddHeader("Content-Type", "application/x-www-form-urlencoded");

			CMStringA data(CMStringDataFormat::FORMAT,
				"client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s",
				MS_APP_ID, MS_CLIENT_SECRET, refreshToken);
			SetData(data.GetBuffer(), data.GetLength());
		}
	};

	/*class RevokeAccessTokenRequest : public HttpRequest
	{
	public:
		RevokeAccessTokenRequest(const char *token) :
			HttpRequest(REQUEST_POST, MS_OAUTH "/logout")
		{
			AddUrlParameter("token=%s", token);
		}
	};*/

	class CreateUploadSessionRequest : public HttpRequest
	{
	public:
		CreateUploadSessionRequest(const char *token, const char *name) :
			HttpRequest(REQUEST_POST, FORMAT, ONEDRIVE_API "/root:/%s:/createUploadSession", name)
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode params(JSON_NODE);
			params
				<< JSONNode("@microsoft.graph.conflictBehavior", "rename")
				<< JSONNode("name", name);

			json_string data = params.write();
			SetData(data.c_str(), data.length());
		}

		CreateUploadSessionRequest(const char *token, const char *parentId, const char *name) :
			HttpRequest(REQUEST_POST, FORMAT, ONEDRIVE_API "/items/%s:/%s:/createUploadSession", parentId, name)
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode params(JSON_NODE);
			params
				<< JSONNode("@microsoft.graph.conflictBehavior", "rename")
				<< JSONNode("name", name);

			json_string data = params.write();
			SetData(data.c_str(), data.length());
		}
	};

	class UploadFileChunkRequest : public HttpRequest
	{
	public:
		UploadFileChunkRequest(const char *uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize) :
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
			HttpRequest(REQUEST_PUT, ONEDRIVE_API "/items/root/children")
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode params(JSON_NODE);
			params
				<< JSONNode("name", path)
				<< JSONNode("folder", "");

			json_string data = params.write();
			SetData(data.c_str(), data.length());
		}
	};

	class CreateSharedLinkRequest : public HttpRequest
	{
	public:
		CreateSharedLinkRequest(const char *token, const char *path) :
			HttpRequest(REQUEST_POST, FORMAT, ONEDRIVE_API "/items/%s/createLink", path)
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode params(JSON_NODE);
			params
				<< JSONNode("type", "view")
				<< JSONNode("scope", "anonymous");

			json_string data = params.write();
			SetData(data.c_str(), data.length());
		}
	};
};

#endif //_ONEDRIVESERVICE_API_H_
