#pragma once

struct ASMObjectCreateRequest : public AsyncHttpRequest
{
	ASMObjectCreateRequest(CSkypeProto *ppro, CFileUploadParam *fup) :
		AsyncHttpRequest(REQUEST_POST, HOST_OTHER, "https://api.asm.skype.com/v1/objects", &CSkypeProto::OnASMObjectCreated)
	{
		flags &= (~NLHRF_DUMPASTEXT);
		pUserInfo = fup;
		
		AddHeader("Authorization", CMStringA(FORMAT, "skype_token %s", ppro->m_szApiToken.get()));
		AddHeader("Content-Type", "application/json");
		AddHeader("X-Client-Version", "0/0.0.0.0");

		CMStringA szContact(ppro->getId(fup->hContact));
		T2Utf uszFileName(fup->tszFileName);
		const char *szFileName = strrchr(uszFileName.get() + 1, '\\');

		JSONNode node;
		if (fup->isPicture)
			node << CHAR_PARAM("type", "pish/image");
		else
			node << CHAR_PARAM("type", "sharing/file");
		
		JSONNode jPermission(JSON_ARRAY); jPermission.set_name(szContact.c_str()); jPermission << CHAR_PARAM("", "read");
		JSONNode jPermissions; jPermissions.set_name("permissions"); jPermissions << jPermission;
		node << CHAR_PARAM("filename", szFileName) << jPermissions;
		m_szParam = node.write().c_str();
	}
};

struct ASMObjectUploadRequest : public AsyncHttpRequest
{
	ASMObjectUploadRequest(CSkypeProto *ppro, const char *szObject, const uint8_t *data, int size, CFileUploadParam *fup) :
		AsyncHttpRequest(REQUEST_PUT, HOST_OTHER, 0, &CSkypeProto::OnASMObjectUploaded)
	{
		m_szUrl.AppendFormat("https://api.asm.skype.com/v1/objects/%s/content/%s", 
			szObject, fup->isPicture ? "imgpsh" : "original");
		pUserInfo = fup;

		AddHeader("Authorization", CMStringA(FORMAT, "skype_token %s", ppro->m_szApiToken.get()));
		AddHeader("Content-Type", fup->isPicture ? "application" : "application/octet-stream");

		m_szParam.Truncate(size);
		memcpy(m_szParam.GetBuffer(), data, size);
	}
};

struct SendFileRequest : public AsyncHttpRequest
{
	SendFileRequest(CFileUploadParam *fup, const char *username, const char *message) :
		AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, 0, &CSkypeProto::OnMessageSent)
	{
		m_szUrl.AppendFormat("/users/ME/conversations/%s/messages", mir_urlEncode(username).c_str());

		JSONNode ref(JSON_ARRAY); ref.set_name("amsreferences"); ref << CHAR_PARAM("", fup->uid);

		JSONNode node;
		if (fup->isPicture)
			node << CHAR_PARAM("messagetype", "RichText/UriObject");
		else
			node << CHAR_PARAM("messagetype", "RichText/Media_GenericFile");

		node << INT64_PARAM("clientmessageid", time(0)) << CHAR_PARAM("contenttype", "text") << CHAR_PARAM("content", message) << ref;
		m_szParam = node.write().c_str();
	}
};
