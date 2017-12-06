#pragma once

class ASMObjectCreateRequest : public HttpRequest
{
public:
	ASMObjectCreateRequest(LoginInfo &li, const char *szContact, const char *szFileName) :
		HttpRequest(REQUEST_POST, "api.asm.skype.com/v1/objects")
	{
		flags &= (~NLHRF_DUMPASTEXT);
		Headers
			<< FORMAT_VALUE("Authorization", "skype_token %s", li.api.szToken)
			<< CHAR_VALUE("Content-Type", "text/json");

		JSONNode node, jPermissions, jPermission(JSON_ARRAY);
		jPermissions.set_name("permissions");
		jPermission.set_name(szContact);
		jPermission << JSONNode("", "read");
		jPermissions << jPermission;
		node << JSONNode("type", "sharing/file") << JSONNode("filename", szFileName) << jPermissions;

		Body << VALUE(node.write().c_str());

	}// {"id":"0-neu-d1-d0649c1fb4e4c60f2d2d1f2165a99f60"}
};

class ASMObjectUploadRequest : public HttpRequest
{
public:
	ASMObjectUploadRequest(LoginInfo &li, const char *szObject, const PBYTE data, const size_t size) :
		HttpRequest(REQUEST_PUT, FORMAT, "api.asm.skype.com/v1/objects/%s/content/original", szObject)
	{
		Headers
			<< FORMAT_VALUE("Authorization", "skype_token %s", li.api.szToken)
			<< CHAR_VALUE("Content-Type", "application/octet-stream");

		pData = (char*)mir_alloc(size);
		memcpy(pData, data, size);
		dataLength = (int)size;

	}
	~ASMObjectUploadRequest()
	{
		mir_free(pData);
	}
};