#pragma once

class ASMObjectCreateRequest : public HttpRequest
{
public:
	ASMObjectCreateRequest(LoginInfo &li, const char *szContact) :
		HttpRequest(REQUEST_POST, "api.asm.skype.com/v1/objects")
	{
		Headers
			<< CHAR_VALUE("Authorization:", CMStringA(::FORMAT, "skype_token %s", li.api.szToken));

		JSONNode node, jPermissions, jPermission(JSON_ARRAY);
		jPermissions.set_name("permissions");
		jPermission.set_name(CMStringA(::FORMAT, "8:%s", szContact).GetString());
		jPermission << JSONNode("read", (char*)NULL);
		jPermissions << jPermission;
		node << JSONNode("type", "pish/image") << jPermissions;

		Body << VALUE(node.write().c_str());

	}// {"id":"0-neu-d1-d0649c1fb4e4c60f2d2d1f2165a99f60"}
};

class ASMObjectUploadRequest : public HttpRequest
{
public:
	ASMObjectUploadRequest(LoginInfo &li, const char *szObject, const PBYTE data, const size_t size) :
		HttpRequest(REQUEST_POST, FORMAT, "api.asm.skype.com/v1/objects/%s/content/imgpsh", szObject)
	{
		Headers
			<< CHAR_VALUE("Authorization:", CMStringA(::FORMAT, "skype_token %s", li.api.szToken));

		pData = (char*)mir_alloc(size);
		memcpy(pData, data, size);
		dataLength = (int)size;

	}
	~ASMObjectUploadRequest()
	{
		mir_free(pData);
		HttpRequest::~HttpRequest();
	}
};