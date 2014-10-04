/*
Copyright (c) 2013-14 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

HANDLE CVkProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR *path) { return NULL; }
int CVkProto::FileCancel(MCONTACT hContact, HANDLE hTransfer) {	return 1; }
int CVkProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR *reason) { return 1; }
int CVkProto::FileResume(HANDLE hTransfer, int *action, const PROTOCHAR **filename) { return 1; }
int CVkProto::RecvFile(MCONTACT hContact, PROTORECVFILET *) { return 1; }

FileUploadParam::FileUploadParam(MCONTACT _hContact, const PROTOCHAR* _desc, PROTOCHAR** _files):
hContact(_hContact), filetype(typeInvalid), atr(NULL), fname(NULL)
{
	Desc = mir_tstrdup(_desc);
	FileName = mir_tstrdup(_files[0]);
}

FileUploadParam::~FileUploadParam()
{
	mir_free(Desc);
	mir_free(FileName);
	mir_free(atr);
	mir_free(fname);
}

FileUploadParam::VKFileType FileUploadParam::GetType()
{
	if (filetype != typeInvalid)
		return filetype;
	
	TCHAR img[] = L"jpg jpeg png bmp";
	TCHAR audio[] = L"mp3";
	
	TCHAR  DRIVE[3], DIR[256], FNAME[256], EXT[256];
	_tsplitpath(FileName, DRIVE, DIR, FNAME, EXT);
	
	CMStringA fn(mir_utf8encodeT(FNAME));
	fn.AppendChar('.');
	fn.AppendFormat("%s", mir_utf8encodeT(EXT));

	fname = mir_strdup(fn.GetBuffer());

	if (tlstrstr(img, EXT)){
		filetype = FileUploadParam::typeImg;
		atr = mir_strdup("photo");
	}
	else if (tlstrstr(audio, EXT)){
		filetype = FileUploadParam::typeAudio;
		atr = mir_strdup("file");
	}
	else{
		filetype = FileUploadParam::typeDoc;
		atr = mir_strdup("file");
	}
	
	return filetype;
}


HANDLE CVkProto::SendFile(MCONTACT hContact, const PROTOCHAR *desc, PROTOCHAR **files)
{
	debugLogA("CVkProto::SendFile");
	FileUploadParam *fup = new FileUploadParam(hContact, desc, files);
	ForkThread(&CVkProto::SendFileThread, (void *)fup);
	return (HANDLE)fup;
}

void CVkProto::SendFileFiled(FileUploadParam *fup, TCHAR *reason)
{
	debugLog(L"CVkProto::SendFileFiled <%s>", reason);
	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)fup, 0);
	delete fup;
}

void CVkProto::SendFileThread(void *p)
{
	FileUploadParam *fup = (FileUploadParam *)p;
	debugLogA("CVkProto::SendFileThread");
	if (!fup->IsAccess()){
		SendFileFiled(fup, L"FileIsNotAccess");
		return;
	}
	AsyncHttpRequest *pReq;
	switch (fup->GetType()){
	case FileUploadParam::typeImg:
		pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/photos.getMessagesUploadServer.json", true, &CVkProto::OnReciveUploadServer)
			<< VER_API;
		pReq->pUserInfo = p;
		Push(pReq);
		break;
	case FileUploadParam::typeAudio:
		// Audio
		SendFileFiled(fup, L"FileTypeIsNotSupported");
		break;
	case FileUploadParam::typeDoc:
		// Doc
		SendFileFiled(fup, L"FileTypeIsNotSupported");
		break;
	default:
		SendFileFiled(fup);
	}
}

void CVkProto::OnReciveUploadServer(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	FileUploadParam *fup = (FileUploadParam *)pReq->pUserInfo;

	debugLogA("CVkProto::OnReciveUploadServer %d", reply->resultCode);
	if (reply->resultCode != 200){
		SendFileFiled(fup, L"NotUploadServer");
		return;
	}

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL){
		SendFileFiled(fup);
		return;
	}

	CMStringA uri = json_as_string(json_get(pResponse, "upload_url"));
	if (uri.IsEmpty()){
		SendFileFiled(fup);
		return;
	}
	
	FILE *pFile = _tfopen(fup->FileName, _T("rb"));
	if (pFile == NULL){
		SendFileFiled(fup, L"ErrorOpenFile");
		return;
	}

	fseek(pFile, 0, SEEK_END);
	size_t szFileLen = ftell(pFile); //FileSize
	fseek(pFile, 0, SEEK_SET);

	AsyncHttpRequest *pUploadReq = new AsyncHttpRequest(this, REQUEST_POST, uri.GetBuffer(), false, &CVkProto::OnReciveUpload);
	pUploadReq->m_bApiReq = false;
	pUploadReq->m_szParam = "";
	CMStringA boundary, header;
	CMStringA NamePart = fup->atrName();
	CMStringA FNamePart = fup->fileName();
	srand(time(NULL));
	
	int iboundary = rand();
	boundary.AppendFormat("Miranda%dNG%d", iboundary, time(NULL));
	header.AppendFormat("multipart/form-data; boundary=%s", boundary.GetBuffer());
	pUploadReq->AddHeader("Content-Type", header.GetBuffer());

	CMStringA DataBegin = "--";
	DataBegin += boundary;
	DataBegin += "\r\n";
	DataBegin += "Content-Disposition: form-data; name=\"";
	DataBegin += NamePart;
	DataBegin += "\"; filename=\"";
	DataBegin += FNamePart;
	DataBegin += "\";\r\n\r\n";

	CMStringA DataEnd = "\r\n--";
	DataEnd += boundary;
	DataEnd += "--\r\n";
	
	size_t dataLength = szFileLen + DataBegin.GetLength() + DataEnd.GetLength();

	char* pData = (char *)mir_alloc(dataLength);
	memcpy(pData, (void *)DataBegin.GetBuffer(), DataBegin.GetLength());
	pUploadReq->pData = pData;
	pData += DataBegin.GetLength();
	fread(pData, 1, szFileLen, pFile);
	pData += szFileLen;
	memcpy(pData, (void *)DataEnd.GetBuffer(), DataEnd.GetLength());
	pUploadReq->	dataLength = dataLength;
	pUploadReq->pUserInfo = pReq->pUserInfo;
	debugLogA("CVkProto::OnReciveUploadServer \n<%d>", dataLength);
	Push(pUploadReq);
	fclose(pFile);
}

void CVkProto::OnReciveUpload(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	FileUploadParam *fup = (FileUploadParam *)pReq->pUserInfo;

	debugLogA("CVkProto::OnReciveUploadServer %d", reply->resultCode);
	if (reply->resultCode != 200){
		SendFileFiled(fup);
		return;
	}

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	
	CMString server = json_as_string(json_get(pRoot, "server"));
	CMString photo = json_as_string(json_get(pRoot, "photo"));
	CMString hash = json_as_string(json_get(pRoot, "hash"));

	AsyncHttpRequest *pUploadReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/photos.saveMessagesPhoto.json", true, &CVkProto::OnReciveUploadFile)
		<< TCHAR_PARAM("server", server)
		<< TCHAR_PARAM("photo", photo)
		<< TCHAR_PARAM("hash", hash)
		<< VER_API;
	pUploadReq->pUserInfo = pReq->pUserInfo;
	Push(pUploadReq);
}

void CVkProto::OnReciveUploadFile(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	FileUploadParam *fup = (FileUploadParam *)pReq->pUserInfo;

	debugLogA("CVkProto::OnReciveUploadPhoto %d", reply->resultCode);
	if (reply->resultCode != 200){
		SendFileFiled(fup);
		return;
	}

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL){
		SendFileFiled(fup);
		return;
	}

	int id = json_as_int(json_get(json_at(pResponse,0), "id"));
	int owner_id = json_as_int(json_get(json_at(pResponse, 0), "owner_id"));
	CMString Attachment;
	Attachment.AppendFormat(L"photo%d_%d", owner_id, id);

	debugLog(L"CVkProto::OnReciveUploadPhoto Att %s", Attachment.GetBuffer());

	LONG userID = getDword(fup->hContact, "ID", -1);
	if (userID == -1){
		SendFileFiled(fup);
		return;
	}

	AsyncHttpRequest *pMsgReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true, &CVkProto::OnSendMessage)
		<< INT_PARAM("user_id", userID)
		<< TCHAR_PARAM("message", fup->Desc)
		<< TCHAR_PARAM("attachment", Attachment.GetBuffer())
		<< VER_API;
	pMsgReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	pMsgReq->pUserInfo = new CVkSendMsgParam(fup->hContact, -1, (int)pReq->pUserInfo);
	
	Push(pMsgReq);
}