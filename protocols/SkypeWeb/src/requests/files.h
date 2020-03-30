#pragma once

class ASMObjectCreateRequest : public HttpRequest
{
public:
	ASMObjectCreateRequest(CSkypeProto *ppro, const char *szContact, const char *szFileName) :
		HttpRequest(REQUEST_POST, "api.asm.skype.com/v1/objects")
	{
		flags &= (~NLHRF_DUMPASTEXT);
		Headers
			<< FORMAT_VALUE("Authorization", "skype_token %s", ppro->m_szApiToken.get())
			<< CHAR_VALUE("Content-Type", "text/json")
			<< CHAR_VALUE("X-Client-Version", "0/0.0.0.0");

		JSONNode node, jPermissions, jPermission(JSON_ARRAY);
		jPermissions.set_name("permissions");
		jPermission.set_name(szContact);
		jPermission << JSONNode("", "read");
		jPermissions << jPermission;
		node << JSONNode("type", "sharing/file") << JSONNode("filename", szFileName) << jPermissions;

		Body << VALUE(node.write().c_str());
	}
};

class ASMObjectUploadRequest : public HttpRequest
{
public:
	ASMObjectUploadRequest(CSkypeProto *ppro, const char *szObject, const PBYTE data, const size_t size) :
		HttpRequest(REQUEST_PUT, FORMAT, "api.asm.skype.com/v1/objects/%s/content/original", szObject)
	{
		Headers
			<< FORMAT_VALUE("Authorization", "skype_token %s", ppro->m_szApiToken.get())
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
