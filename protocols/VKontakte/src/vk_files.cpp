/*
Copyright (c) 2013-21 Miranda NG team (https://miranda-ng.org)

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

HANDLE CVkProto::SendFile(MCONTACT hContact, const wchar_t *desc, wchar_t **files)
{
	debugLogA("CVkProto::SendFile");

	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	if (!IsOnline() || ((userID == VK_INVALID_USER || userID == VK_FEED_USER) && !isChatRoom(hContact)) || !files || !files[0])
		return (HANDLE)nullptr;

	CVkFileUploadParam *fup = new CVkFileUploadParam(hContact, desc, files);

	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)fup);

	if (!fup->IsAccess()) {
		SendFileFiled(fup, VKERR_FILE_NOT_EXIST);
		return (HANDLE)nullptr;
	}

	AsyncHttpRequest *pReq;
	switch (fup->GetType()) {
	case CVkFileUploadParam::typeImg:
		pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/photos.getMessagesUploadServer.json", true, &CVkProto::OnReciveUploadServer);
		break;
	case CVkFileUploadParam::typeAudio:
		pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/audio.getUploadServer.json", true, &CVkProto::OnReciveUploadServer);
		break;
	case CVkFileUploadParam::typeAudioMsg:
		pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/docs.getUploadServer.json", true, &CVkProto::OnReciveUploadServer);
		pReq << CHAR_PARAM("type", "audio_message");
		break;
	case CVkFileUploadParam::typeDoc:
		pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/docs.getUploadServer.json", true, &CVkProto::OnReciveUploadServer);
		break;
	default:
		SendFileFiled(fup, VKERR_FTYPE_NOT_SUPPORTED);
		return (HANDLE)nullptr;
	}
	pReq->pUserInfo = fup;
	Push(pReq);

	if (files[1])
		SendFile(hContact, L"", &files[1]);

	return (HANDLE)fup;
}

void CVkProto::SendFileFiled(CVkFileUploadParam *fup, int ErrorCode)
{
	CMStringW wszError;
	switch (ErrorCode) {
	case VKERR_OFFLINE:
		wszError = TranslateT("Protocol is offline");
		break;
	case VKERR_FILE_NOT_EXIST:
		wszError = TranslateT("File does not exist");
		break;
	case VKERR_FTYPE_NOT_SUPPORTED:
		wszError = TranslateT("File type not supported");
		break;
	case VKERR_ERR_OPEN_FILE:
		wszError = TranslateT("Error open file");
		break;
	case VKERR_ERR_READ_FILE:
		wszError = TranslateT("Error read file");
		break;
	case VKERR_FILE_NOT_UPLOADED:
		wszError = TranslateT("File upload error");
		break;
	case VKERR_INVALID_URL:
		wszError = TranslateT("Upload server returned empty URL");
		break;
	case VKERR_INVALID_USER:
		wszError = TranslateT("Invalid or unknown recipient user ID");
		break;
	case VKERR_INVALID_PARAMETERS:
		wszError = TranslateT("One of the parameters specified was missing or invalid");
		break;
	case VKERR_COULD_NOT_SAVE_FILE:
		wszError = TranslateT("Couldn't save file");
		break;
	case VKERR_INVALID_ALBUM_ID:
		wszError = TranslateT("Invalid album id");
		break;
	case VKERR_INVALID_SERVER:
		wszError = TranslateT("Invalid server");
		break;
	case VKERR_INVALID_HASH:
		wszError = TranslateT("Invalid hash");
		break;
	case VKERR_INVALID_AUDIO:
		wszError = TranslateT("Invalid audio");
		break;
	case VKERR_AUDIO_DEL_COPYRIGHT:
		wszError = TranslateT("The audio file was removed by the copyright holder and cannot be reuploaded");
		break;
	case VKERR_INVALID_FILENAME:
		wszError = TranslateT("Invalid filename");
		break;
	case VKERR_INVALID_FILESIZE:
		wszError = TranslateT("Invalid filesize");
		break;
	default:
		wszError = TranslateT("Unknown error occurred");
	}
	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ErrorCode == VKERR_AUDIO_DEL_COPYRIGHT ? ACKRESULT_DENIED : ACKRESULT_FAILED, (HANDLE)fup);
	debugLogW(L"CVkProto::SendFileFiled error code = %d (%s)", ErrorCode, wszError.c_str());
	MsgPopup(wszError, TranslateT("File upload error"), true);
	delete fup;
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
		if (!pReq->bNeedsRestart)
			SendFileFiled(fup, pReq->m_iErrorCode);
		return;
	}

	CMStringA uri(jnResponse["upload_url"].as_mstring());
	if (uri.IsEmpty()) {
		SendFileFiled(fup, VKERR_INVALID_URL);
		return;
	}

	FILE *pFile = _wfopen(fup->FileName, L"rb");
	if (pFile == nullptr) {
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
	boundary.AppendFormat("Miranda%dNG%d", iboundary, time(0));
	// Header
	header.AppendFormat("multipart/form-data; boundary=%s", boundary.c_str());
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
	char *pData = (char *)mir_alloc(dataLength);
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

	CMStringW server(jnRoot["server"].as_mstring());
	CMStringW hash(jnRoot["hash"].as_mstring());
	CMStringW upload;

	AsyncHttpRequest *pUploadReq;

	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)fup);
	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, (HANDLE)fup);

	switch (fup->GetType()) {
	case CVkFileUploadParam::typeImg:
		upload = jnRoot["photo"].as_mstring();
		if (upload == L"[]") {
			SendFileFiled(fup, VKERR_INVALID_PARAMETERS);
			return;
		}
		pUploadReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/photos.saveMessagesPhoto.json", true, &CVkProto::OnReciveUploadFile);
		pUploadReq << WCHAR_PARAM("server", server) << WCHAR_PARAM("photo", upload) << WCHAR_PARAM("hash", hash);
		break;
	case CVkFileUploadParam::typeAudio:
		upload = jnRoot["audio"].as_mstring();
		if (upload == L"[]") {
			SendFileFiled(fup, VKERR_INVALID_PARAMETERS);
			return;
		}
		pUploadReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/audio.save.json", true, &CVkProto::OnReciveUploadFile);
		pUploadReq << WCHAR_PARAM("server", server) << WCHAR_PARAM("audio", upload) << WCHAR_PARAM("hash", hash);
		break;
	case CVkFileUploadParam::typeDoc:
	case CVkFileUploadParam::typeAudioMsg:
		upload = jnRoot["file"].as_mstring();
		if (upload.IsEmpty()) {
			SendFileFiled(fup, VKERR_INVALID_PARAMETERS);
			return;
		}
		pUploadReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/docs.save.json", true, &CVkProto::OnReciveUploadFile);
		pUploadReq
			<< CHAR_PARAM("title", fup->fileName())
			<< WCHAR_PARAM("file", upload)
			<< CHAR_PARAM("tags", fup->fileName())
			<< INT_PARAM("return_tags", 0);
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
		if (!pReq->bNeedsRestart)
			SendFileFiled(fup, pReq->m_iErrorCode);
		return;
	}

	int id, owner_id;
	if ((fup->GetType() == CVkFileUploadParam::typeDoc) || (fup->GetType() == CVkFileUploadParam::typeAudioMsg)) {
		CMStringA wszType(jnResponse["type"].as_mstring());
		const JSONNode& jnDoc = jnResponse[wszType];
		id = jnDoc["id"].as_int();
		owner_id = jnDoc["owner_id"].as_int();
	}
	else {
		id = fup->GetType() == CVkFileUploadParam::typeAudio ? jnResponse["id"].as_int() : (*jnResponse.begin())["id"].as_int();
		owner_id = fup->GetType() == CVkFileUploadParam::typeAudio ? jnResponse["owner_id"].as_int() : (*jnResponse.begin())["owner_id"].as_int();
	}

	if ((id == 0) || (owner_id == 0)) {
		SendFileFiled(fup, VKERR_INVALID_PARAMETERS);
		return;
	}

	CMStringW Attachment;

	switch (fup->GetType()) {
	case CVkFileUploadParam::typeImg:
		Attachment.AppendFormat(L"photo%d_%d", owner_id, id);
		break;
	case CVkFileUploadParam::typeAudio:
		Attachment.AppendFormat(L"audio%d_%d", owner_id, id);
		break;
	case CVkFileUploadParam::typeDoc:
	case CVkFileUploadParam::typeAudioMsg:
		Attachment.AppendFormat(L"doc%d_%d", owner_id, id);
		break;
	default:
		SendFileFiled(fup, VKERR_FTYPE_NOT_SUPPORTED);
		return;
	}

	AsyncHttpRequest *pMsgReq;

	if (isChatRoom(fup->hContact)) {

		ptrW wszChatID(getWStringA(fup->hContact, "ChatRoomID"));
		if (!wszChatID) {
			SendFileFiled(fup, VKERR_INVALID_USER);
			return;
		}

		CVkChatInfo *cc = GetChatById(wszChatID);
		if (cc == nullptr) {
			SendFileFiled(fup, VKERR_INVALID_USER);
			return;
		}

		pMsgReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true, &CVkProto::OnSendChatMsg, AsyncHttpRequest::rpHigh);
		pMsgReq << INT_PARAM("chat_id", cc->m_iChatId);
		pMsgReq->pUserInfo = pReq->pUserInfo;

	}
	else {
		LONG userID = getDword(fup->hContact, "ID", VK_INVALID_USER);
		if (userID == VK_INVALID_USER || userID == VK_FEED_USER) {
			SendFileFiled(fup, VKERR_INVALID_USER);
			return;
		}

		pMsgReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true, &CVkProto::OnSendMessage, AsyncHttpRequest::rpHigh);
		pMsgReq << INT_PARAM("user_id", userID);
		pMsgReq->pUserInfo = new CVkSendMsgParam(fup->hContact, fup);
	}

	ULONG uMsgId = ::InterlockedIncrement(&m_msgId);
	pMsgReq << WCHAR_PARAM("message", fup->Desc) << WCHAR_PARAM("attachment", Attachment);
	pMsgReq << INT_PARAM("random_id", ((LONG)time(0)) * 100 + uMsgId % 100);
	pMsgReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");

	Push(pMsgReq);
}