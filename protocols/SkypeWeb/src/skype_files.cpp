#include "stdafx.h"

#define FILETRANSFER_FAILED(fup) { ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)fup); delete fup; fup = nullptr;} 

HANDLE CSkypeProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles)
{
	if (IsOnline()) {
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
		FILETRANSFER_FAILED(fup);
		return;
	}
	if (!fup->IsAccess()) {
		FILETRANSFER_FAILED(fup);
		return;
	}

	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)fup);
	T2Utf uszFileName(fup->tszFileName);
	SendRequest(new ASMObjectCreateRequest(li, CMStringA(FORMAT, "%d:%s", isChatRoom(fup->hContact) ? 19 : 8, Contacts[fup->hContact]), strrchr((const char*)uszFileName + 1, '\\')), &CSkypeProto::OnASMObjectCreated, fup);
}

void CSkypeProto::OnASMObjectCreated(const NETLIBHTTPREQUEST *response, void *arg)
{
	CFileUploadParam *fup = (CFileUploadParam*)arg;
	if (response && response->pData) {
		JSONNode node = JSONNode::parse((char*)response->pData);
		std::string strObjectId = node["id"].as_string();
		fup->uid = mir_strdup(strObjectId.c_str());
		FILE *pFile = _wfopen(fup->tszFileName, L"rb");
		if (pFile == NULL) return;

		fseek(pFile, 0, SEEK_END);
		long lFileLen = ftell(pFile);

		if (lFileLen < 1) {
			fclose(pFile);
			return;
		}

		fseek(pFile, 0, SEEK_SET);

		mir_ptr<BYTE> pData((BYTE*)mir_alloc(lFileLen));
		long lBytes = (long)fread(pData, sizeof(BYTE), lFileLen, pFile);

		if (lBytes != lFileLen) {
			fclose(pFile);
			FILETRANSFER_FAILED(fup);
			return;
		}
		fup->size = lBytes;
		ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)fup);
		SendRequest(new ASMObjectUploadRequest(li, strObjectId.c_str(), pData, lBytes), &CSkypeProto::OnASMObjectUploaded, fup);
		fclose(pFile);
	}
}

void CSkypeProto::OnASMObjectUploaded(const NETLIBHTTPREQUEST *response, void *arg)
{
	CFileUploadParam *fup = (CFileUploadParam*)arg;
	if (response == nullptr) {
		FILETRANSFER_FAILED(fup);
		return;
	}

	wchar_t *tszFile = wcsrchr(fup->tszFileName, L'\\') + 1;

	HXML xml = xmlCreateNode(L"URIObject", nullptr, 0);
	xmlAddChild(xml, L"Title", tszFile);
	xmlAddChild(xml, L"Description", fup->tszDesc);
	HXML xmlA = xmlAddChild(xml, L"a", CMStringW(FORMAT, L"https://login.skype.com/login/sso?go=webclient.xmm&docid=%s", _A2T(fup->uid)));
	xmlAddAttr(xmlA, L"href", CMStringW(FORMAT, L"https://login.skype.com/login/sso?go=webclient.xmm&docid=%s", _A2T(fup->uid)));
	HXML xmlOrigName = xmlAddChild(xml, L"OriginalName", nullptr);
	xmlAddAttr(xmlOrigName, L"v", tszFile);
	HXML xmlSize = xmlAddChild(xml, L"FileSize", nullptr);
	xmlAddAttr(xmlSize, L"v", CMStringW(FORMAT, L"%d", fup->size));

	xmlAddAttr(xml, L"Type", L"File.1");
	xmlAddAttr(xml, L"uri", CMStringW(FORMAT, L"https://api.asm.skype.com/v1/objects/%s", _A2T(fup->uid)));
	xmlAddAttr(xml, L"url_thumbnail", CMStringW(FORMAT, L"https://api.asm.skype.com/v1/objects/%s/views/thumbnail", _A2T(fup->uid)));

	SendRequest(new SendMessageRequest(Contacts[fup->hContact], time(NULL), T2Utf(ptrW(xmlToString(xml, nullptr))), li, "RichText/Media_GenericFile"));
	xmlDestroyNode(xml);
	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)fup);
	delete fup;
}
