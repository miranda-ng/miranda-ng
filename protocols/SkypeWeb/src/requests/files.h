#pragma once

struct ASMObjectCreateRequest : public AsyncHttpRequest
{
	ASMObjectCreateRequest(CSkypeProto *ppro, CFileUploadParam *fup) :
		AsyncHttpRequest(REQUEST_POST, HOST_OTHER, "https://api.asm.skype.com/v1/objects", &CSkypeProto::OnASMObjectCreated)
	{
		flags &= (~NLHRF_DUMPASTEXT);
		pUserInfo = fup;
		
		AddHeader("Authorization", CMStringA(FORMAT, "skype_token %s", ppro->m_szApiToken.get()));
		AddHeader("Content-Type", "text/json");
		AddHeader("X-Client-Version", "0/0.0.0.0");

		CMStringA szContact(ppro->getId(fup->hContact));
		T2Utf uszFileName(fup->tszFileName);
		const char *szFileName = strrchr(uszFileName.get() + 1, '\\');

		JSONNode node, jPermissions, jPermission(JSON_ARRAY);
		jPermissions.set_name("permissions");
		jPermission.set_name(szContact.c_str());
		jPermission << CHAR_PARAM("", "read");
		jPermissions << jPermission;
		node << CHAR_PARAM("type", "sharing/file") << CHAR_PARAM("filename", szFileName) << jPermissions;
		m_szParam = node.write().c_str();
	}
};

struct ASMObjectUploadRequest : public AsyncHttpRequest
{
	ASMObjectUploadRequest(CSkypeProto *ppro, const char *szObject, const uint8_t *data, const size_t size, CFileUploadParam *fup) :
		AsyncHttpRequest(REQUEST_PUT, HOST_OTHER, 0, &CSkypeProto::OnASMObjectUploaded)
	{
		m_szUrl.AppendFormat("https://api.asm.skype.com/v1/objects/%s/content/original", szObject);
		pUserInfo = fup;

		AddHeader("Authorization", CMStringA(FORMAT, "skype_token %s", ppro->m_szApiToken.get()));
		AddHeader("Content-Type", "application/octet-stream");

		pData = (char*)mir_alloc(size);
		memcpy(pData, data, size);
		dataLength = (int)size;
	}

	~ASMObjectUploadRequest()
	{
		mir_free(pData);
	}
};
