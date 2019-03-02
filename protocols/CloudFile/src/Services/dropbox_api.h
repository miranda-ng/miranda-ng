#ifndef _DROPBOXSERVICE_API_H_
#define _DROPBOXSERVICE_API_H_

// https://www.dropbox.com/developers/documentation/http/documentation
namespace DropboxAPI
{
#define DROPBOX_API_VER "/2"
#define DROPBOX_API "https://api.dropboxapi.com"
#define DROPBOX_API_OAUTH DROPBOX_API "/oauth2"
#define DROPBOX_API_RPC DROPBOX_API DROPBOX_API_VER
#define DROPBOX_CONTENT "https://content.dropboxapi.com"
#define DROPBOX_API_CU DROPBOX_CONTENT DROPBOX_API_VER

#define DROPBOX_APP_KEY "fa8du7gkf2q8xzg"
#include "../../../miranda-private-keys/Dropbox/secret_key.h"

#define DROPBOX_API_AUTH "https://www.dropbox.com/oauth2/authorize?response_type=code&redirect_uri=https%3A%2F%2Foauth.miranda-ng.org%2Fverification&client_id=" DROPBOX_APP_KEY

	class GetAccessTokenRequest : public HttpRequest
	{
	public:
		GetAccessTokenRequest(const char *code) :
			HttpRequest(REQUEST_POST, DROPBOX_API_OAUTH "/token")
		{
			AddHeader("Content-Type", "application/x-www-form-urlencoded");

			CMStringA data = "redirect_uri=https://oauth.miranda-ng.org/verification";
			data.AppendFormat("&client_id=%s&client_secret=%s", DROPBOX_APP_KEY, DROPBOX_API_SECRET);
			data.AppendFormat("&grant_type=authorization_code&code=%s", code);
			SetData(data.GetBuffer(), data.GetLength());
		}
	};

	class RevokeAccessTokenRequest : public HttpRequest
	{
	public:
		RevokeAccessTokenRequest(const char *token) :
			HttpRequest(REQUEST_POST, DROPBOX_API_OAUTH "/token/revoke")
		{
			AddBearerAuthHeader(token);
		}
	};

	class UploadFileRequest : public HttpRequest
	{
	public:
		UploadFileRequest(const char *token, const char *path, const char *data, size_t size, OnConflict strategy = NONE) :
			HttpRequest(REQUEST_POST, DROPBOX_API_CU "/files/upload")
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/octet-stream");

			JSONNode params(JSON_NODE);
			params
				<< JSONNode("path", path);
			if (strategy == OnConflict::RENAME) {
				params
					<< JSONNode("mode", "add")
					<< JSONNode("autorename", true);
			}
			else if (strategy == OnConflict::REPLACE) {
				params
					<< JSONNode("mode", "overwrite")
					<< JSONNode("autorename", false);
			}

			AddHeader("Dropbox-API-Arg", params.write().c_str());

			SetData(data, size);
		}
	};

	class CreateUploadSessionRequest : public HttpRequest
	{
	public:
		CreateUploadSessionRequest(const char *token, const char *chunk, size_t chunkSize) :
			HttpRequest(REQUEST_POST, DROPBOX_API_CU "/files/upload_session/start")
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/octet-stream");

			SetData(chunk, chunkSize);
		}
	};

	class UploadFileChunkRequest : public HttpRequest
	{
	public:
		UploadFileChunkRequest(const char *token, const char *sessionId, size_t offset, const char *chunk, size_t chunkSize) :
			HttpRequest(REQUEST_POST, DROPBOX_API_CU "/files/upload_session/append_v2")
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/octet-stream");


			JSONNode cursor;
			cursor.set_name("cursor");
			cursor
				<< JSONNode("session_id", sessionId)
				<< JSONNode("offset", (unsigned long)offset);

			JSONNode param;
			param << cursor;

			AddHeader("Dropbox-API-Arg", param.write().c_str());

			SetData(chunk, chunkSize);
		}
	};

	class CommitUploadSessionRequest : public HttpRequest
	{
	public:
		CommitUploadSessionRequest(const char *token, const char *sessionId, size_t offset, const char *path, const char *chunk, size_t chunkSize, OnConflict strategy = NONE) :
			HttpRequest(REQUEST_POST, DROPBOX_API_CU "/files/upload_session/finish")
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/octet-stream");

			JSONNode cursor(JSON_NODE);
			cursor.set_name("cursor");
			cursor
				<< JSONNode("session_id", sessionId)
				<< JSONNode("offset", (unsigned long)offset);

			JSONNode commit(JSON_NODE);
			commit.set_name("commit");
			commit
				<< JSONNode("path", path);
			if (strategy == OnConflict::RENAME) {
				commit
					<< JSONNode("mode", "add")
					<< JSONNode("autorename", true);
			}
			else if (strategy == OnConflict::REPLACE) {
				commit
					<< JSONNode("mode", "overwrite")
					<< JSONNode("autorename", false);
			}

			JSONNode params(JSON_NODE);
			params
				<< cursor
				<< commit;

			AddHeader("Dropbox-API-Arg", params.write().c_str());

			SetData(chunk, chunkSize);
		}
	};

	class CreateFolderRequest : public HttpRequest
	{
	public:
		CreateFolderRequest(const char *token, const char *path) :
			HttpRequest(REQUEST_POST, DROPBOX_API_RPC "/files/create_folder_v2")
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode root(JSON_NODE);
			root << JSONNode("path", path);

			json_string data = root.write();
			SetData(data.c_str(), data.length());
		}
	};

	class GetTemporaryLinkRequest : public HttpRequest
	{
	public:
		GetTemporaryLinkRequest(const char *token, const char *path) :
			HttpRequest(REQUEST_POST, DROPBOX_API_RPC "/files/get_temporary_link")
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode root(JSON_NODE);
			root << JSONNode("path", path);

			json_string data = root.write();
			SetData(data.c_str(), data.length());
		}
	};

	class CreateSharedLinkRequest : public HttpRequest
	{
	public:
		CreateSharedLinkRequest(const char *token, const char *path) :
			HttpRequest(REQUEST_POST, DROPBOX_API_RPC "/sharing/create_shared_link_with_settings")
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode root(JSON_NODE);
			root << JSONNode("path", path);

			json_string data = root.write();
			SetData(data.c_str(), data.length());
		}
	};

	class GetSharedLinkRequest : public HttpRequest
	{
	public:
		GetSharedLinkRequest(const char *token, const char *path) :
			HttpRequest(REQUEST_POST, DROPBOX_API_RPC "/sharing/list_shared_links")
		{
			AddBearerAuthHeader(token);
			AddHeader("Content-Type", "application/json");

			JSONNode root(JSON_NODE);
			root << JSONNode("path", path);

			json_string data = root.write();
			SetData(data.c_str(), data.length());
		}
	};
};

#endif //_DROPBOXSERVICE_API_H_
