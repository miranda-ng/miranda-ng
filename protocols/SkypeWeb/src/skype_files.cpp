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
	SendRequest(new ASMObjectCreateRequest(this, CMStringA(FORMAT, "%d:%s", isChatRoom(fup->hContact) ? 19 : 8, getId(fup->hContact).c_str()), strrchr((const char*)uszFileName + 1, '\\')), &CSkypeProto::OnASMObjectCreated, fup);
}

void CSkypeProto::OnASMObjectCreated(const NETLIBHTTPREQUEST *response, void *arg)
{
	CFileUploadParam *fup = (CFileUploadParam*)arg;
	if (response == nullptr || response->pData == nullptr) {
LBL_Error:
		FILETRANSFER_FAILED(fup);
		return;
	}

	if (response->resultCode != 200 && response->resultCode != 201) {
		debugLogA("Object creation failed with error code %d", response->resultCode);
		goto LBL_Error;
	}

	JSONNode node = JSONNode::parse((char*)response->pData);
	std::string strObjectId = node["id"].as_string();
	if (strObjectId.empty()) {
		debugLogA("Invalid server response (empty object id)");
		goto LBL_Error;
	}
	
	fup->uid = mir_strdup(strObjectId.c_str());
	FILE *pFile = _wfopen(fup->tszFileName, L"rb");
	if (pFile == nullptr) return;

	fseek(pFile, 0, SEEK_END);
	long lFileLen = ftell(pFile);
	if (lFileLen < 1) {
		fclose(pFile);
		goto LBL_Error;
	}

	fseek(pFile, 0, SEEK_SET);

	mir_ptr<BYTE> pData((BYTE*)mir_alloc(lFileLen));
	long lBytes = (long)fread(pData, sizeof(BYTE), lFileLen, pFile);
	if (lBytes != lFileLen) {
		fclose(pFile);
		goto LBL_Error;
	}
	fup->size = lBytes;
	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)fup);
	SendRequest(new ASMObjectUploadRequest(this, strObjectId.c_str(), pData, lBytes), &CSkypeProto::OnASMObjectUploaded, fup);
	fclose(pFile);
}

void CSkypeProto::OnASMObjectUploaded(const NETLIBHTTPREQUEST *response, void *arg)
{
	CFileUploadParam *fup = (CFileUploadParam*)arg;
	if (response == nullptr) {
		FILETRANSFER_FAILED(fup);
		return;
	}

	wchar_t *tszFile = wcsrchr(fup->tszFileName, L'\\') + 1;

	TiXmlDocument doc;
	auto *pRoot = doc.NewElement("URIObject");
	doc.InsertEndChild(pRoot);

	auto *pTitle = doc.NewElement("Title"); pTitle->SetText(tszFile); pRoot->InsertEndChild(pTitle);
	auto *pDescr = doc.NewElement("Description"); pDescr->SetText(fup->tszDesc.get()); pRoot->InsertEndChild(pDescr);

	auto *xmlA = doc.NewElement("a"); xmlA->SetText(CMStringA(FORMAT, "https://login.skype.com/login/sso?go=webclient.xmm&docid=%s", fup->uid.get()));
	xmlA->SetAttribute("href", CMStringA(FORMAT, "https://login.skype.com/login/sso?go=webclient.xmm&docid=%s", fup->uid.get()));
	pRoot->InsertEndChild(xmlA);

	auto *xmlOrigName = doc.NewElement("OriginalName"); xmlOrigName->SetAttribute("v", tszFile); pRoot->InsertEndChild(xmlOrigName);
	auto *xmlSize = doc.NewElement("FileSize"); xmlSize->SetAttribute("v", (int)fup->size); pRoot->InsertEndChild(xmlSize);

	pRoot->SetAttribute("Type", "File.1");
	pRoot->SetAttribute("uri", CMStringA(FORMAT, "https://api.asm.skype.com/v1/objects/%s", fup->uid.get()));
	pRoot->SetAttribute("url_thumbnail", CMStringA(FORMAT, "https://api.asm.skype.com/v1/objects/%s/views/thumbnail", fup->uid.get()));

	tinyxml2::XMLPrinter printer(0, true);
	doc.Print(&printer);
	SendRequest(new SendMessageRequest(getId(fup->hContact), time(NULL), printer.CStr(), this, "RichText/Media_GenericFile"));

	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)fup);
	delete fup;
}
