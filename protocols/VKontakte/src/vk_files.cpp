/*
Copyright (c) 2013-15 Miranda NG project (http://miranda-ng.org)

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

HANDLE CVkProto::SendFile(MCONTACT hContact, const TCHAR *desc, TCHAR **files)
{
	debugLogA("CVkProto::SendFile");

	LONG userID = getDword(hContact, "ID", -1);
	if (!IsOnline() || ((userID == -1 || userID == VK_FEED_USER) && !isChatRoom(hContact)))
		return (HANDLE)0;

	CVkFileUploadParam *fup = new CVkFileUploadParam(hContact, desc, files);
	ForkThread(&CVkProto::SendFileThread, (void *)fup);
	return (HANDLE)fup;
}

void CVkProto::SendFileFiled(CVkFileUploadParam *fup, int ErrorCode)
{
	CMString tszError;
	switch (ErrorCode) {
	case VKERR_OFFLINE:
		tszError = TranslateT("Protocol is offline");
		break;
	case VKERR_FILE_NOT_EXIST:
		tszError = TranslateT("File does not exist");
		break;
	case VKERR_FTYPE_NOT_SUPPORTED:
		tszError = TranslateT("File type not supported");
		break;
	case VKERR_ERR_OPEN_FILE:
		tszError = TranslateT("Error open file");
		break;
	case VKERR_ERR_READ_FILE:
		tszError = TranslateT("Error read file");
		break;
	case VKERR_FILE_NOT_UPLOADED:
		tszError = TranslateT("File upload error");
		break;
	case VKERR_INVALID_URL:
		tszError = TranslateT("Upload server returned empty URL");
		break;
	case VKERR_INVALID_USER:
		tszError = TranslateT("Invalid or unknown recipient user ID");
		break;
	case VKERR_INVALID_PARAMETERS:
		tszError = TranslateT("One of the parameters specified was missing or invalid");
		break;		
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
	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ErrorCode== VKERR_AUDIO_DEL_COPYRIGHT ? ACKRESULT_DENIED : ACKRESULT_FAILED, (HANDLE)fup);
	debugLog(_T("CVkProto::SendFileFiled error code = %d (%s)"), ErrorCode, tszError);
	MsgPopup(NULL, tszError, TranslateT("File upload error"), true);
	delete fup;
}

void CVkProto::SendFileThread(void *p)
{
	CVkFileUploadParam *fup = (CVkFileUploadParam *)p;
	debugLog(_T("CVkProto::SendFileThread %d %s"), fup->GetType(), fup->fileName());
	if (!IsOnline()) {
		SendFileFiled(fup, VKERR_OFFLINE);
		return;
	}
	if (!fup->IsAccess()) {
		SendFileFiled(fup, VKERR_FILE_NOT_EXIST);
		return;
	}

	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)fup);

	AsyncHttpRequest *pReq;
	switch (fup->GetType()) {
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
		SendFileFiled(fup, VKERR_FTYPE_NOT_SUPPORTED);
		return;
	}
	pReq->pUserInfo = p;
	Push(pReq);
}

void CVkProto::OnReciveUploadServer(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	CVkFileUploadParam *fup = (CVkFileUploadParam *)pReq->pUserInfo;
	if (!IsOnline()) {
		SendFileFiled(fup, VKERR_OFFLINE);
		return;
	}

	debugLogA("CVkProto::OnReciveUploadServer %d", reply->resultCode);
	if (reply->resultCode != 200) {
		SendFileFiled(fup, VKERR_INVALID_SERVER);
		return;
	}

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse || pReq->m_iErrorCode) {
		SendFileFiled(fup, pReq->m_iErrorCode);
		return;
	}

	CMStringA uri(jnResponse["upload_url"].as_mstring());
	if (uri.IsEmpty()) {
		SendFileFiled(fup, VKERR_INVALID_URL);
		return;
	}
	
	FILE *pFile = _tfopen(fup->FileName, _T("rb"));
	if (pFile == NULL) {
		SendFileFiled(fup, VKERR_ERR_OPEN_FILE);
		return;
	}

	fseek(pFile, 0, SEEK_END);
	long iFileLen = ftell(pFile); //FileSize
	if (iFileLen < 1) {
		fclose(pFile);
		SendFileFiled(fup, VKERR_ERR_READ_FILE);
		return;
	}
	fseek(pFile, 0, SEEK_SET);

	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)fup);

	AsyncHttpRequest *pUploadReq = new AsyncHttpRequest(this, REQUEST_POST, uri, false, &CVkProto::OnReciveUpload);
	pUploadReq->m_bApiReq = false;
	pUploadReq->m_szParam = "";
	CMStringA boundary, header;
	CMStringA NamePart = fup->atrName();
	CMStringA FNamePart = fup->fileName();
	
	// Boundary
	int iboundary;
	Utils_GetRandom(&iboundary, sizeof(iboundary));
	boundary.AppendFormat("Miranda%dNG%d", iboundary, time(NULL));
	// Header
	header.AppendFormat("multipart/form-data; boundary=%s", boundary);
	pUploadReq->AddHeader("Content-Type", header);
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
	long dataLength = iFileLen + DataBegin.GetLength() + DataEnd.GetLength();
	// Body {
	char* pData = (char *)mir_alloc(dataLength);
	memcpy(pData, (void *)DataBegin.GetBuffer(), DataBegin.GetLength());
	pUploadReq->pData = pData;

	pData += DataBegin.GetLength();
	long lBytes = (long)fread(pData, 1, iFileLen, pFile);
	fclose(pFile);

	if (lBytes != iFileLen) {
		SendFileFiled(fup, VKERR_ERR_READ_FILE);
		mir_free(pUploadReq->pData);
		delete pUploadReq;
		return;
	}

	pData += iFileLen;
	memcpy(pData, (void *)DataEnd.GetBuffer(), DataEnd.GetLength());
	// } Body

	pUploadReq->dataLength = (int)dataLength;
	pUploadReq->pUserInfo = pReq->pUserInfo;
	Push(pUploadReq);
}

void CVkProto::OnReciveUpload(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	CVkFileUploadParam *fup = (CVkFileUploadParam *)pReq->pUserInfo;
	if (!IsOnline()) {
		SendFileFiled(fup, VKERR_OFFLINE);
		return;
	}

	debugLogA("CVkProto::OnReciveUploadServer %d", reply->resultCode);
	if (reply->resultCode != 200) {
		SendFileFiled(fup, VKERR_FILE_NOT_UPLOADED);
		return;
	}

	JSONNode jnRoot;
	CheckJsonResponse(pReq, reply, jnRoot);

	if (pReq->m_iErrorCode) {
		SendFileFiled(fup, pReq->m_iErrorCode);
		return;
	}

	if ((!jnRoot["server"] || !jnRoot["hash"]) && !jnRoot["file"]) {
		SendFileFiled(fup, VKERR_INVALID_PARAMETERS);
		return;
	}

	CMString server(jnRoot["server"].as_mstring());
	CMString hash(jnRoot["hash"].as_mstring());
	CMString upload;

	AsyncHttpRequest *pUploadReq;

	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)fup);

	switch (fup->GetType()) {
	case CVkFileUploadParam::typeImg:
		upload = jnRoot["photo"].as_mstring();
		if (upload == _T("[]")) {
			SendFileFiled(fup, VKERR_INVALID_PARAMETERS);
			return;
		}
		pUploadReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/photos.saveMessagesPhoto.json", true, &CVkProto::OnReciveUploadFile)
			<< TCHAR_PARAM("server", server)
			<< TCHAR_PARAM("photo", upload)
			<< TCHAR_PARAM("hash", hash)
			<< VER_API;
		break;
	case CVkFileUploadParam::typeAudio:
		upload = jnRoot["audio"].as_mstring();
		if (upload == _T("[]")) {
			SendFileFiled(fup, VKERR_INVALID_PARAMETERS);
			return;
		}
		pUploadReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/audio.save.json", true, &CVkProto::OnReciveUploadFile)
			<< TCHAR_PARAM("server", server)
			<< TCHAR_PARAM("audio", upload)
			<< TCHAR_PARAM("hash", hash)
			<< VER_API;
		break;
	case CVkFileUploadParam::typeDoc:
		upload = jnRoot["file"].as_mstring();
		if (upload.IsEmpty()) {
			SendFileFiled(fup, VKERR_INVALID_PARAMETERS);
			return;
		}
		pUploadReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/docs.save.json", true, &CVkProto::OnReciveUploadFile)
			<< CHAR_PARAM("title", fup->fileName())
			<< TCHAR_PARAM("file", upload)	
			<< VER_API;
		break;
	default:
		SendFileFiled(fup, VKERR_FTYPE_NOT_SUPPORTED);
		return;
	}
	
	pUploadReq->pUserInfo = pReq->pUserInfo;
	Push(pUploadReq);
}

void CVkProto::OnReciveUploadFile(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	CVkFileUploadParam *fup = (CVkFileUploadParam *)pReq->pUserInfo;
	if (!IsOnline()) {
		SendFileFiled(fup, VKERR_OFFLINE);
		return;
	}

	debugLogA("CVkProto::OnReciveUploadFile %d", reply->resultCode);
	if (reply->resultCode != 200) {
		SendFileFiled(fup, VKERR_FILE_NOT_UPLOADED);
		return;
	}

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse || pReq->m_iErrorCode) {
		SendFileFiled(fup, pReq->m_iErrorCode);
		return;
	}

	int id = fup->GetType() == CVkFileUploadParam::typeAudio ? jnResponse["id"].as_int() : (*jnResponse.begin())["id"].as_int();
	int owner_id = fup->GetType() == CVkFileUploadParam::typeAudio ? jnResponse["owner_id"].as_int() : (*jnResponse.begin())["owner_id"].as_int(); 	
	if ((id == 0) || (owner_id == 0)) {
		SendFileFiled(fup, VKERR_INVALID_PARAMETERS);
		return;
	}
	
	CMString Attachment;

	switch (fup->GetType()) {
	case CVkFileUploadParam::typeImg:
		Attachment.AppendFormat(_T("photo%d_%d"), owner_id, id);
		break;
	case CVkFileUploadParam::typeAudio:
		Attachment.AppendFormat(_T("audio%d_%d"), owner_id, id);
		break;
	case CVkFileUploadParam::typeDoc:
		Attachment.AppendFormat(_T("doc%d_%d"), owner_id, id);
		break;
	default:
		SendFileFiled(fup, VKERR_FTYPE_NOT_SUPPORTED);
		return;
	}

	AsyncHttpRequest *pMsgReq;
	
	if (isChatRoom(fup->hContact)) {

		ptrT tszChatID(getTStringA(fup->hContact, "ChatRoomID"));
		if (!tszChatID) {
			SendFileFiled(fup, VKERR_INVALID_USER);
			return;
		}

		CVkChatInfo *cc = GetChatById(tszChatID);
		if (cc == NULL) {
			SendFileFiled(fup, VKERR_INVALID_USER);
			return;
		}

		pMsgReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true, &CVkProto::OnSendChatMsg, AsyncHttpRequest::rpHigh)
			<< INT_PARAM("chat_id", cc->m_chatid);
		pMsgReq->pUserInfo = pReq->pUserInfo;

	}
	else {
		LONG userID = getDword(fup->hContact, "ID", -1);
		if (userID == -1 || userID == VK_FEED_USER) {
			SendFileFiled(fup, VKERR_INVALID_USER);
			return;
		}

		pMsgReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true, &CVkProto::OnSendMessage, AsyncHttpRequest::rpHigh)
			<< INT_PARAM("user_id", userID);
		pMsgReq->pUserInfo = new CVkSendMsgParam(fup->hContact, fup);
		
	}

	pMsgReq << TCHAR_PARAM("message", fup->Desc) << TCHAR_PARAM("attachment", Attachment) << VER_API;
	pMsgReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	
	Push(pMsgReq);
}