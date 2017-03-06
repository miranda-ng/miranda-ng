#ifndef _DROPBOX_API_UPLOAD_H_
#define _DROPBOX_API_UPLOAD_H_

class UploadFileRequest : public HttpRequest
{
public:
	UploadFileRequest(const char *token, const char *path, const char *data, size_t size) :
		HttpRequest(REQUEST_POST, DROPBOX_API_CU "/files/upload")
	{
		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/octet-stream");

		JSONNode params(JSON_NODE);
		params
			<< JSONNode("path", path)
			<< JSONNode("mode", "overwrite");

		AddHeader("Dropbox-API-Arg", params.write().c_str());

		SetData(data, size);
	}
};

class StartUploadSessionRequest : public HttpRequest
{
public:
	StartUploadSessionRequest(const char *token, const char *data, size_t size) :
		HttpRequest(REQUEST_POST, DROPBOX_API_CU "/files/upload_session/start")
	{
		AddBearerAuthHeader(token);
		AddHeader("Content-Type", "application/octet-stream");

		SetData(data, size);
	}
};

class AppendToUploadSessionRequest : public HttpRequest
{
public:
	AppendToUploadSessionRequest(const char *token, const char *sessionId, size_t offset, const char *data, size_t size) :
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

		SetData(data, size);
	}
};

class FinishUploadSessionRequest : public HttpRequest
{
public:
	FinishUploadSessionRequest(const char *token, const char *sessionId, size_t offset, const char *path, const char *data, size_t size) :
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
			<< JSONNode("path", path)
			<< JSONNode("mode", "overwrite");

		JSONNode params(JSON_NODE);
		params
			<< cursor
			<< commit;

		AddHeader("Dropbox-API-Arg", params.write().c_str());

		SetData(data, size);
	}
};

#endif //_DROPBOX_API_UPLOAD_H_
