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

CVkFileUploadParam::CVkFileUploadParam(MCONTACT _hContact, const PROTOCHAR* _desc, PROTOCHAR** _files) :
hContact(_hContact), filetype(typeInvalid), atr(NULL), fname(NULL), iErrorCode(0)
{
	Desc = mir_tstrdup(_desc);
	FileName = mir_tstrdup(_files[0]);
}

CVkFileUploadParam::~CVkFileUploadParam()
{
	mir_free(Desc);
	mir_free(FileName);
	mir_free(atr);
	mir_free(fname);
}

CVkFileUploadParam::VKFileType CVkFileUploadParam::GetType()
{
	if (filetype != typeInvalid)
		return filetype;
	
	TCHAR img[] = L".jpg .jpeg .png .bmp";
	TCHAR audio[] = L".mp3";
	
	TCHAR  DRIVE[3], DIR[256], FNAME[256], EXT[256];
	_tsplitpath(FileName, DRIVE, DIR, FNAME, EXT);
	
	CMStringA fn;
	fn.AppendFormat("%s%s", mir_utf8encodeT(FNAME), mir_utf8encodeT(EXT));
	fname = mir_strdup(fn.GetBuffer());

	if (tlstrstr(img, EXT)){
		filetype = CVkFileUploadParam::typeImg;
		atr = mir_strdup("photo");
	}
	else if (tlstrstr(audio, EXT)){
		filetype = CVkFileUploadParam::typeAudio;
		atr = mir_strdup("file");
	}
	else{
		filetype = CVkFileUploadParam::typeDoc;
		atr = mir_strdup("file");
	}
	
	return filetype;
}

HANDLE CVkProto::SendFile(MCONTACT hContact, const PROTOCHAR *desc, PROTOCHAR **files)
{
	debugLogA("CVkProto::SendFile");
	if (!IsOnline())
		return (HANDLE)0;
	CVkFileUploadParam *fup = new CVkFileUploadParam(hContact, desc, files);
	ForkThread(&CVkProto::SendFileThread, (void *)fup);
	return (HANDLE)fup;
}

void CVkProto::SendFileFiled(CVkFileUploadParam *fup, TCHAR *reason)
{
	debugLog(L"CVkProto::SendFileFiled <%s> Error code <%d>", reason, fup->iErrorCode);
	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)fup, 0);
	CMString tszError;
	switch (fup->iErrorCode){
	case VKERR_COULD_NOT_SAVE_FILE:
		tszError = TranslateT("Couldn't save file");
		break;
	case VKERR_INVALID_ALBUM_ID:
		tszError = TranslateT("Invalid album id");
		break;
	case VKERR_INVALID_SERVER:
		tszError = TranslateT("Invalid server");
		break;
	case VKERR_INVALID_HASH:
		tszError = TranslateT("Invalid hash");
		break;
	case VKERR_INVALID_AUDIO:
		tszError = TranslateT("Invalid audio");
		break;
	case VKERR_AUDIO_DEL_COPYRIGHT:
		tszError = TranslateT("The audio file was removed by the copyright holder and cannot be reuploaded");
		break;
	case VKERR_INVALID_FILENAME:
		tszError = TranslateT("Invalid filename");
		break;
	case VKERR_INVALID_FILESIZE:
		tszError = TranslateT("Invalid filesize");
		break;
	default:
		tszError = TranslateT("Unknown error occurred");
	}
	MsgPopup(NULL, tszError.GetBuffer(), TranslateT("File upload error"), true);
	delete fup;
}

void CVkProto::SendFileThread(void *p)
{
	CVkFileUploadParam *fup = (CVkFileUploadParam *)p;
	debugLog(L"CVkProto::SendFileThread %d %s", fup->GetType(), fup->fileName());
	if (!IsOnline()){
		SendFileFiled(fup, L"NotOnline");
		return;
	}
	if (!fup->IsAccess()){
		SendFileFiled(fup, L"FileIsNotAccess");
		return;
	}

	AsyncHttpRequest *pReq;
	switch (fup->GetType()){
	case CVkFileUploadParam::typeImg:
		pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/photos.getMessagesUploadServer.json", true, &CVkProto::OnReciveUploadServer)
			<< VER_API;
		break;
	case CVkFileUploadParam::typeAudio:
		pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/audio.getUploadServer.json", true, &CVkProto::OnReciveUploadServer)
			<< VER_API;
		break;
	case CVkFileUploadParam::typeDoc:
		pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/docs.getUploadServer.json", true, &CVkProto::OnReciveUploadServer)
			<< VER_API;
		break;
	default:
		SendFileFiled(fup, L"FileTypeNotSupported");
		return;
	}
	pReq->pUserInfo = p;
	Push(pReq);
}

void CVkProto::OnReciveUploadServer(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	CVkFileUploadParam *fup = (CVkFileUploadParam *)pReq->pUserInfo;
	if (!IsOnline()){
		SendFileFiled(fup, L"NotOnline");
		return;
	}

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
	
	// Boundary
	srand(time(NULL));
	int iboundary = rand();
	boundary.AppendFormat("Miranda%dNG%d", iboundary, time(NULL));
	// Header
	header.AppendFormat("multipart/form-data; boundary=%s", boundary.GetBuffer());
	pUploadReq->AddHeader("Content-Type", header.GetBuffer());
	// Content-Disposition {
	CMStringA DataBegin = "--";
	DataBegin += boundary;
	DataBegin += "\r\n";
	DataBegin += "Content-Disposition: form-data; name=\"";
	DataBegin += NamePart;
	DataBegin += "\"; filename=\"";
	DataBegin += FNamePart;
	DataBegin += "\";\r\n\r\n";
	// } Content-Disposition
	CMStringA DataEnd = "\r\n--";
	DataEnd += boundary;
	DataEnd += "--\r\n";
	// Body size
	size_t dataLength = szFileLen + DataBegin.GetLength() + DataEnd.GetLength();
	// Body {
	char* pData = (char *)mir_alloc(dataLength);
	memcpy(pData, (void *)DataBegin.GetBuffer(), DataBegin.GetLength());
	pUploadReq->pData = pData;

	pData += DataBegin.GetLength();
	fread(pData, 1, szFileLen, pFile);
	fclose(pFile);

	pData += szFileLen;
	memcpy(pData, (void *)DataEnd.GetBuffer(), DataEnd.GetLength());
	// } Body

	pUploadReq->	dataLength = dataLength;
	pUploadReq->pUserInfo = pReq->pUserInfo;
	Push(pUploadReq);
}

void CVkProto::OnReciveUpload(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	CVkFileUploadParam *fup = (CVkFileUploadParam *)pReq->pUserInfo;
	if (!IsOnline()){
		SendFileFiled(fup, L"NotOnline");
		return;
	}

	debugLogA("CVkProto::OnReciveUploadServer %d", reply->resultCode);
	if (reply->resultCode != 200){
		SendFileFiled(fup);
		return;
	}

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);

	CMString server = json_as_string(json_get(pRoot, "server"));
	CMString hash = json_as_string(json_get(pRoot, "hash"));
	CMString upload;

	AsyncHttpRequest *pUploadReq;

	switch (fup->GetType()){
	case CVkFileUploadParam::typeImg:
		upload = json_as_string(json_get(pRoot, "photo"));
		if (upload == L"[]"){
			SendFileFiled(fup, L"NotUpload Photo");
		}
		pUploadReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/photos.saveMessagesPhoto.json", true, &CVkProto::OnReciveUploadFile)
			<< TCHAR_PARAM("server", server)
			<< TCHAR_PARAM("photo", upload)
			<< TCHAR_PARAM("hash", hash)
			<< VER_API;
		break;
	case CVkFileUploadParam::typeAudio:
		upload = json_as_string(json_get(pRoot, "audio"));
		if (upload == L"[]"){
			SendFileFiled(fup, L"NotUpload Audio");
			return;
		}
		pUploadReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/audio.save.json", true, &CVkProto::OnReciveUploadFile)
			<< TCHAR_PARAM("server", server)
			<< TCHAR_PARAM("audio", upload)
			<< TCHAR_PARAM("hash", hash)
			<< VER_API;
		break;
	case CVkFileUploadParam::typeDoc:
		upload = json_as_string(json_get(pRoot, "file"));
		if (upload.IsEmpty()){
			SendFileFiled(fup, L"NotUpload Doc");
			return;
		}
		pUploadReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/docs.save.json", true, &CVkProto::OnReciveUploadFile)
			<< CHAR_PARAM("title", fup->fileName())
			<< TCHAR_PARAM("file", upload)	
			<< VER_API;
		break;
	default:
		SendFileFiled(fup);
		return;
	}
	
	pUploadReq->pUserInfo = pReq->pUserInfo;
	Push(pUploadReq);
}

void CVkProto::OnReciveUploadFile(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	CVkFileUploadParam *fup = (CVkFileUploadParam *)pReq->pUserInfo;
	if (!IsOnline()){
		SendFileFiled(fup, L"NotOnline");
		return;
	}

	debugLogA("CVkProto::OnReciveUploadFile %d", reply->resultCode);
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

	int id = json_as_int(json_get(fup->GetType() == CVkFileUploadParam::typeAudio ? pResponse : json_at(pResponse, 0), "id"));
	int owner_id = json_as_int(json_get(fup->GetType() == CVkFileUploadParam::typeAudio ? pResponse : json_at(pResponse, 0), "owner_id"));
	if ((id == 0) || (owner_id == 0)){
		SendFileFiled(fup);
		return;
	}
	
	CMString Attachment;

	switch (fup->GetType()){
	case CVkFileUploadParam::typeImg:
		Attachment.AppendFormat(L"photo%d_%d", owner_id, id);
		break;
	case CVkFileUploadParam::typeAudio:
		Attachment.AppendFormat(L"audio%d_%d", owner_id, id);
		break;
	case CVkFileUploadParam::typeDoc:
		Attachment.AppendFormat(L"doc%d_%d", owner_id, id);
		break;
	default:
		SendFileFiled(fup);
		return;
	}
	
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