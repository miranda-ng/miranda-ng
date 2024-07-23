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
	PushRequest(new ASMObjectCreateRequest(this, fup));
}

void CSkypeProto::OnASMObjectCreated(MHttpResponse *response, AsyncHttpRequest *pRequest)
{
	auto *fup = (CFileUploadParam*)pRequest->pUserInfo;
	if (response == nullptr || response->body.IsEmpty()) {
LBL_Error:
		FILETRANSFER_FAILED(fup);
		return;
	}

	if (response->resultCode != 200 && response->resultCode != 201) {
		debugLogA("Object creation failed with error code %d", response->resultCode);
		goto LBL_Error;
	}

	JSONNode node = JSONNode::parse(response->body);
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

	mir_ptr<uint8_t> pData((uint8_t*)mir_alloc(lFileLen));
	long lBytes = (long)fread(pData, sizeof(uint8_t), lFileLen, pFile);
	if (lBytes != lFileLen) {
		fclose(pFile);
		goto LBL_Error;
	}
	fup->size = lBytes;
	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)fup);
	PushRequest(new ASMObjectUploadRequest(this, strObjectId.c_str(), pData, lBytes, fup));
	fclose(pFile);
}

void CSkypeProto::OnASMObjectUploaded(MHttpResponse *response, AsyncHttpRequest *pRequest)
{
	auto *fup = (CFileUploadParam*)pRequest->pUserInfo;
	if (response == nullptr) {
		FILETRANSFER_FAILED(fup);
		return;
	}

	wchar_t *tszFile = wcsrchr(fup->tszFileName, L'\\') + 1;

	TiXmlDocument doc;
	auto *pRoot = doc.NewElement("URIObject");
	doc.InsertEndChild(pRoot);

	pRoot->SetAttribute("doc_id", fup->uid.get());
	pRoot->SetAttribute("uri", CMStringA(FORMAT, "https://api.asm.skype.com/v1/objects/%s", fup->uid.get()));

	// is that a picture?
	CMStringA href;
	bool bIsPictture = false;
	if (auto *pBitmap = FreeImage_LoadU(FreeImage_GetFIFFromFilenameU(fup->tszFileName), fup->tszFileName)) {
		bIsPictture = true;
		pRoot->SetAttribute("type", "File.1" /*"Picture.1"*/);
		pRoot->SetAttribute("url_thumbnail", CMStringA(FORMAT, "https://api.asm.skype.com/v1/objects/%s/views/imgt1_anim", fup->uid.get()));
		pRoot->SetAttribute("width", FreeImage_GetWidth(pBitmap));
		pRoot->SetAttribute("height", FreeImage_GetHeight(pBitmap));
		FreeImage_Unload(pBitmap);

		href.Format("https://login.skype.com/login/sso?go=xmmfallback?pic=%s", fup->uid.get());
	}
	else {
		pRoot->SetAttribute("type", "File.1");
		pRoot->SetAttribute("url_thumbnail", CMStringA(FORMAT, "https://api.asm.skype.com/v1/objects/%s/views/original", fup->uid.get()));
		href.Format("https://login.skype.com/login/sso?go=webclient.xmm&docid=%s", fup->uid.get());
	}

	auto *pTitle = doc.NewElement("Title"); pTitle->SetText(tszFile); pRoot->InsertEndChild(pTitle);
	auto *pDescr = doc.NewElement("Description"); pDescr->SetText(fup->tszDesc.get()); pRoot->InsertEndChild(pDescr);

	auto *xmlA = doc.NewElement("a"); xmlA->SetText(href);
	xmlA->SetAttribute("href", href);
	pRoot->InsertEndChild(xmlA);

	auto *xmlOrigName = doc.NewElement("OriginalName"); xmlOrigName->SetAttribute("v", tszFile); pRoot->InsertEndChild(xmlOrigName);
	auto *xmlSize = doc.NewElement("FileSize"); xmlSize->SetAttribute("v", (int)fup->size); pRoot->InsertEndChild(xmlSize);

	if (bIsPictture) {
		auto xmlMeta = doc.NewElement("meta"); 
		xmlMeta->SetAttribute("type", "photo"); xmlMeta->SetAttribute("originalName", tszFile);
		pRoot->InsertEndChild(xmlMeta);
	}

	tinyxml2::XMLPrinter printer(0, true);
	doc.Print(&printer);

	SendMessageParam *param = new SendMessageParam();
	param->hContact = fup->hContact;
	Utils_GetRandom(&param->hMessage, sizeof(param->hMessage));
	param->hMessage &= ~0x80000000;

	auto *pReq = new SendFileRequest(getId(fup->hContact), time(NULL), printer.CStr(), "RichText/Media_GenericFile", fup->uid);
	pReq->pUserInfo = param;
	PushRequest(pReq);

	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)fup);
	delete fup;
}
