#ifndef _DROPBOX_API_OPERATIONS_H_
#define _DROPBOX_API_OPERATIONS_H_

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

class SearchRequest : public HttpRequest
{
public:
	SearchRequest(const char *token, const char *query) :
		HttpRequest(REQUEST_POST, DROPBOX_API_RPC "/files/search")
	{
		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/json");

		JSONNode params(JSON_NODE);
		params
			<< JSONNode("path", "")
			<< JSONNode("query", query)
			<< JSONNode("max_results", 10);

		json_string data = params.write();
		SetData(data.c_str(), data.length());
	}
};

class DeleteRequest : public HttpRequest
{
public:
	DeleteRequest(const char *token, const char *path) :
		HttpRequest(REQUEST_POST, DROPBOX_API_RPC "/files/delete")
	{
		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/json");

		JSONNode root(JSON_NODE);
		root << JSONNode("path", path);

		json_string data = root.write();
		SetData(data.c_str(), data.length());
	}
};

class CreateFolderRequest : public HttpRequest
{
public:
	CreateFolderRequest(const char *token, const char *path) :
		HttpRequest(REQUEST_POST, DROPBOX_API_RPC "/files/create_folder")
	{
		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/json");

		JSONNode root(JSON_NODE);
		root << JSONNode("path", path);

		json_string data = root.write();
		SetData(data.c_str(), data.length());
	}
};

class GetMetadataRequest : public HttpRequest
{
public:
	GetMetadataRequest(const char *token, const char *path) :
		HttpRequest(REQUEST_POST, DROPBOX_API_RPC "/files/get_metadata")
	{
		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/json");

		JSONNode root(JSON_NODE);
		root << JSONNode("path", path);

		json_string data = root.write();
		SetData(data.c_str(), data.length());
	}
};

class ListFolderRequest : public HttpRequest
{
public:
	ListFolderRequest(const char *token, const char *path) :
		HttpRequest(REQUEST_POST, DROPBOX_API_RPC "/files/list_folder")
	{
		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/json");

		JSONNode root(JSON_NODE);
		root << JSONNode("path", path);

		json_string data = root.write();
		SetData(data.c_str(), data.length());
	}
};

#endif //_DROPBOX_API_OPERATIONS_H_
