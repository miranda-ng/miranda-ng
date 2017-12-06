#ifndef _ONEDRIVESERVICE_API_H_
#define _ONEDRIVESERVICE_API_H_

namespace OneDriveAPI
{
#define MS_OAUTH "https://login.microsoftonline.com/common/oauth2/v2.0"
#define ONEDRIVE_API "https://graph.microsoft.com/v1.0/drive"

#define MS_APP_ID "72b87ac7-42eb-4a97-a620-91a7f8d8b5ae"
#include "../../../miranda-private-keys/Microsoft/client_secret.h"

	class GetAccessTokenRequest : public HttpRequest
	{
	public:
		GetAccessTokenRequest(const char *code) :
			HttpRequest(REQUEST_POST, MS_OAUTH "/token")
		{
			AddHeader("Content-Type", "application/x-www-form-urlencoded");

			CMStringA data = "redirect_uri=https://oauth.miranda-ng.org/verification";
			data.Append("&scope=offline_access https://graph.microsoft.com/files.readWrite");
			data.AppendFormat("&client_id=%s&client_secret=%s", MS_APP_ID, MS_CLIENT_SECRET);
			data.AppendFormat("&grant_type=authorization_code&code=%s", code);
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
				"client_id=%s&grant_type=refresh_token&refresh_token=%s",
				MS_APP_ID, refreshToken);
			SetData(data.GetBuffer(), data.GetLength());
		}
	};

	class UploadFileRequest : public HttpRequest
	{
	public:
		UploadFileRequest(const char *token, const char *name, const char *data, size_t size, OnConflict strategy = NONE) :
			HttpRequest(REQUEST_PUT, FORMAT, ONEDRIVE_API "/special/approot:/%s:/content", ptrA(mir_urlEncode(name)))
		{
			if (strategy == OnConflict::RENAME)
				AddUrlParameter("@microsoft.graph.conflictBehavior=rename");
			else if (strategy == OnConflict::REPLACE)
				AddUrlParameter("@microsoft.graph.conflictBehavior=replace");

			AddBearerAuthHeader(token);

			SetData(data, size);
		}

		UploadFileRequest(const char *token, const char *parentId, const char *name, const char *data, size_t size, OnConflict strategy = NONE) :
			HttpRequest(REQUEST_PUT, FORMAT, ONEDRIVE_API "/items/{parent-id}:/{filename}:/content", parentId, ptrA(mir_urlEncode(name)))
		{
			if (strategy == OnConflict::RENAME)
				AddUrlParameter("@microsoft.graph.conflictBehavior=rename");
			else if (strategy == OnConflict::REPLACE)
				AddUrlParameter("@microsoft.graph.conflictBehavior=replace");

			AddBearerAuthHeader(token);

			SetData(data, size);
		}
	};

	class CreateUploadSessionRequest : public HttpRequest
	{
	public:
		CreateUploadSessionRequest(const char *token, const char *name,  OnConflict strategy = NONE) :
			HttpRequest(REQUEST_POST, FORMAT, ONEDRIVE_API "/special/approot:/%s:/createUploadSession", ptrA(mir_urlEncode(name)))
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode item(JSON_NODE);
			item.set_name("item");
			if (strategy == OnConflict::RENAME)
				item << JSONNode("@microsoft.graph.conflictBehavior", "rename");
			if (strategy == OnConflict::REPLACE)
				item << JSONNode("@microsoft.graph.conflictBehavior", "replace");

			JSONNode params(JSON_NODE);
			params << item;

			json_string data = params.write();
			SetData(data.c_str(), data.length());
		}

		CreateUploadSessionRequest(const char *token, const char *parentId, const char *name, OnConflict strategy = NONE) :
			HttpRequest(REQUEST_POST, FORMAT, ONEDRIVE_API "/items/%s:/%s:/createUploadSession", parentId, name)
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode item(JSON_NODE);
			item.set_name("item");
			if (strategy == OnConflict::RENAME)
				item << JSONNode("@microsoft.graph.conflictBehavior", "rename");
			if (strategy == OnConflict::REPLACE)
				item << JSONNode("@microsoft.graph.conflictBehavior", "replace");

			JSONNode params(JSON_NODE);
			params << item;

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
			HttpRequest(REQUEST_PUT, ONEDRIVE_API "/special/approot/children")
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode folder(JSON_NODE);
			folder.set_name("folder");
			folder << JSONNode(JSON_NODE);

			JSONNode params(JSON_NODE);
			params
				<< JSONNode("name", path)
				<< folder;

			json_string data = params.write();
			SetData(data.c_str(), data.length());
		}
	};

	class CreateSharedLinkRequest : public HttpRequest
	{
	public:
		CreateSharedLinkRequest(const char *token, const char *itemId) :
			HttpRequest(REQUEST_POST, FORMAT, ONEDRIVE_API "/items/%s/createLink", itemId)
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
