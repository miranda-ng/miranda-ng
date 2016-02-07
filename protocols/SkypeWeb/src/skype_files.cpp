#include "stdafx.h"

HANDLE CSkypeProto::SendFile(MCONTACT hContact, const TCHAR *szDescription, TCHAR **ppszFiles)
{
	if (IsOnline())
	{
		CFileUploadParam *fup = new CFileUploadParam(hContact, szDescription, ppszFiles);
		ForkThread(&CSkypeProto::SendFileThread, (void*)fup);
		return (HANDLE)fup;
	}
	return INVALID_HANDLE_VALUE;
}

void CSkypeProto::SendFileThread(void *p)
{
	CFileUploadParam *fup = (CFileUploadParam *)p;
	if (!IsOnline()) {
		//SendFileFiled(fup, VKERR_OFFLINE);
		return;
	}
//	if (!fup->IsAccess()) {
//		//SendFileFiled(fup, VKERR_FILE_NOT_EXIST);
//		return;
//	}

	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)fup);

	SendRequest(new ASMObjectCreateRequest(li, CMStringA(FORMAT, "%d:%s", isChatRoom(fup->hContact) ? 19 : 8, ptrA(getStringA(fup->hContact, SKYPE_SETTINGS_ID))), T2Utf(fup->tszFileName)), &CSkypeProto::OnASMObjectCreated, fup);
}

void CSkypeProto::OnASMObjectCreated(const NETLIBHTTPREQUEST *response, void *arg)
{
	CFileUploadParam *fup = (CFileUploadParam*)arg;
	if (response && response->pData)
	{
		JSONNode node = JSONNode::parse((char*)response->pData);
		std::string strObjectId = node["id"].as_string();
		fup->uid = mir_strdup(strObjectId.c_str());
		FILE *pFile = _tfopen(fup->tszFileName, _T("rb"));
		if (pFile == NULL) return;

		fseek(pFile, 0, SEEK_END);
		long lFileLen = ftell(pFile);

		if(lFileLen < 1) {
			fclose(pFile);
			return;
		}

		fseek(pFile, 0, SEEK_SET);
		
		mir_ptr<BYTE> pData((BYTE*)mir_alloc(lFileLen));
		long lBytes = (long)fread(pData, sizeof(BYTE), lFileLen, pFile);

		if (lBytes != lFileLen) {
			fclose(pFile);
			mir_free(pData);
			return;
		}
		fup->size = lBytes;
		ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)fup);
		SendRequest(new ASMObjectUploadRequest(li, strObjectId.c_str(), pData, lBytes), &CSkypeProto::OnASMObjectUploaded, fup);
	}
}

void CSkypeProto::OnASMObjectUploaded(const NETLIBHTTPREQUEST *response, void *arg)
{
	CFileUploadParam *fup = (CFileUploadParam*)arg;
	if (response == nullptr) return;
	CMStringA url(response->szUrl);

	TCHAR *tszFile = fup->tszFileName;

	HXML xml = xmlCreateNode(L"URIObject", nullptr, 0);
	HXML xmlTitle = xmlAddChild(xml, L"Title", tszFile);
	HXML xmlDescr = xmlAddChild(xml, L"Description", fup->tszDesc);
	HXML xmlA = xmlAddChild(xml, L"a", CMStringW(FORMAT, L"https://login.skype.com/login/sso?go=webclient.xmm&docid=%s", _A2T(fup->uid)));
	xmlAddAttr(xmlA, L"href", CMStringW(FORMAT, L"https://login.skype.com/login/sso?go=webclient.xmm&docid=%s", _A2T(fup->uid)));
	HXML xmlOrigName = xmlAddChild(xml, L"OriginalName", nullptr);
	xmlAddAttr(xmlOrigName, L"v", tszFile);
	HXML xmlSize = xmlAddChild(xml, L"FileSize", nullptr);
	xmlAddAttr(xmlSize, L"v", CMStringW(FORMAT, L"%d", fup->size));
	
	xmlAddAttr(xml, L"Type", L"File.1");
	xmlAddAttr(xml, L"uri", CMStringW(FORMAT, L"https://api.asm.skype.com/v1/objects/%s", _A2T(fup->uid)));
	xmlAddAttr(xml, L"url_thumbnail", CMStringW(FORMAT, L"https://api.asm.skype.com/v1/objects/%s/views/thumbnail", _A2T(fup->uid)));

	SendRequest(new SendMessageRequest(ptrA(getStringA(fup->hContact, SKYPE_SETTINGS_ID)), time(NULL), T2Utf(xmlToString(xml, nullptr)), li, "RichText/Media_GenericFile"));
	xmlDestroyNode(xml);
	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)fup);
}