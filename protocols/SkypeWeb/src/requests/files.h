#pragma once

struct ASMObjectCreateRequest : public AsyncHttpRequest
{
	ASMObjectCreateRequest(CSkypeProto *ppro, CFileUploadParam *fup) :
		AsyncHttpRequest(REQUEST_POST, "api.asm.skype.com/v1/objects", &CSkypeProto::OnASMObjectCreated)
	{
		flags &= (~NLHRF_DUMPASTEXT);
		pUserInfo = fup;
		
		AddHeader("Authorization", CMStringA(FORMAT, "skype_token %s", ppro->m_szApiToken.get()));
		AddHeader("Content-Type", "text/json");
		AddHeader("X-Client-Version", "0/0.0.0.0");

		CMStringA szContact(FORMAT, "%d:%s", ppro->isChatRoom(fup->hContact) ? 19 : 8, ppro->getId(fup->hContact).c_str());
		T2Utf uszFileName(fup->tszFileName);
		const char *szFileName = strrchr(uszFileName.get() + 1, '\\');

		JSONNode node, jPermissions, jPermission(JSON_ARRAY);
		jPermissions.set_name("permissions");
		jPermission.set_name(szContact.c_str());
		jPermission << JSONNode("", "read");
		jPermissions << jPermission;
		node << JSONNode("type", "sharing/file") << JSONNode("filename", szFileName) << jPermissions;
		m_szParam = node.write().c_str();
	}
};

struct ASMObjectUploadRequest : public AsyncHttpRequest
{
	ASMObjectUploadRequest(CSkypeProto *ppro, const char *szObject, const PBYTE data, const size_t size, CFileUploadParam *fup) :
		AsyncHttpRequest(REQUEST_PUT, 0, &CSkypeProto::OnASMObjectUploaded)
	{
		m_szUrl.Format("api.asm.skype.com/v1/objects/%s/content/original", szObject);
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
