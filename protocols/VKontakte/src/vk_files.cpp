/*
Copyright (c) 2013-25 Miranda NG team (https://miranda-ng.org)

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

CMStringW CVkProto::GetVkFileItem(CMStringW& _wszUrl, MCONTACT hContact, VKMessageID_t iMessageId)
{

	wchar_t buf[MAX_PATH];
	File::GetReceivedFolder(hContact, buf, _countof(buf));

	debugLogW(L"CVkProto::GetVkFileItem: (%s) %d", buf, hContact);
	
	if (_wszUrl.IsEmpty()) {
		debugLogW(L"CVkProto::GetVkFileItem: url empty");
		return _wszUrl;
	}

	CreateDirectoryTreeW(buf);

	CMStringW wszUrl = _wszUrl;
	wszUrl.Replace(L"\\", L"/");
	
	int i = wszUrl.Find('?');
	if (i > -1)
		wszUrl.Truncate(i);
		
	i = wszUrl.ReverseFind('/');
	if (i > -1)
		wszUrl = wszUrl.Mid(i + 1);

	i = wszUrl.ReverseFind('.');
	if (i < 0) { 
		char szHash[33];
		uint8_t digest[16];
		mir_md5_hash((uint8_t*)_wszUrl.c_str(), _wszUrl.GetLength() * sizeof(wchar_t), digest);
		bin2hex(digest, sizeof(digest), szHash);

		wszUrl = szHash;
		wszUrl += ".jpg";
	}
		
	wszUrl.Insert(0, buf);

	if (::_waccess(wszUrl.c_str(), 0) && IsOnline())
		if (iMessageId != -1 && m_vkOptions.bLoadFilesAsync) {
			AsyncHttpRequest* pReq = new AsyncHttpRequest();
			pReq->flags = NLHRF_NODUMP | NLHRF_REDIRECT;
			pReq->m_szUrl = CMStringA(_wszUrl);
			pReq->pUserInfo = new CVkFileDownloadParam(hContact, iMessageId, wszUrl.c_str());
			pReq->m_pFunc = &CVkProto::OnGetVkFileItem;
			pReq->requestType = REQUEST_GET;
			pReq->m_bApiReq = false;
			pReq->m_priority = AsyncHttpRequest::rpLowLow;
			Push(pReq);
		}
		else {
			MHttpRequest req(REQUEST_GET);
			req.m_szUrl = CMStringA(_wszUrl);
			req.flags = VK_NODUMPHEADERS;

			auto* reply = Netlib_HttpTransaction(m_hNetlibUser, &req);
			if (reply == nullptr)
				return _wszUrl;

			if (reply->resultCode != 200) {
				debugLogA("CVkProto::GetVkFileItem: failed with code %d", reply->resultCode);
				return _wszUrl;
			}

			FILE* out = _wfopen(wszUrl.c_str(), L"wb+");
			if (out) {
				fwrite(reply->body, 1, reply->body.GetLength(), out);
				fclose(out);
				debugLogW(L"CVkProto::GetVkFileItem file %s saved", wszUrl.c_str());
			}
			else 
				debugLogW(L"CVkProto::GetVkFileItem error open file %s", wszUrl.c_str());

		}
	else 
		debugLogW(L"CVkProto::GetVkFileItem file %s already exist or connection lost", wszUrl.c_str());

	wszUrl.Insert(0, L"file://");
	wszUrl.Replace(L"\\", L"/");

	return wszUrl;
}

void CVkProto::OnGetVkFileItem(MHttpResponse* reply, AsyncHttpRequest* pReq)
{
	
	CVkFileDownloadParam* param = (CVkFileDownloadParam*)pReq->pUserInfo;
	
	if (reply->resultCode != 200 || !param) {
		debugLogW(L"CVkProto::OnGetVkFileItem error load file %s", param && param->wszFileName ? param->wszFileName : L"NULL");
		delete param;
		return;
	}

	debugLogW(L"CVkProto::OnGetVkFileItem %s", param->wszFileName);

	FILE* out = _wfopen(param->wszFileName, L"wb");
	if (out) {
		fwrite(reply->body, 1, reply->body.GetLength(), out);
		fclose(out);
		debugLogW(L"CVkProto::OnGetVkFileItem file %s saved %d %d", param->wszFileName, param->hContact, param->iMsgID);

		MessageWindowData mwd = {};
		if (!Srmm_GetWindowData(param->hContact, mwd) && mwd.uState) {
			char szMid[40];
			_itoa(param->iMsgID, szMid, 10);
			MEVENT hDbEvent =  db_event_getById(m_szModuleName, szMid);
			
			DBEVENTINFO dbei = {};
			if (!db_event_get(hDbEvent, &dbei)) {
				int i = db_event_edit(hDbEvent, &dbei, true);
				debugLogW(L"CVkProto::OnGetVkFileItem file %s even edit %d", param->wszFileName, i);
			}
		}
	}
	else
		debugLogW(L"CVkProto::OnGetVkFileItem error open file %s", param->wszFileName);

	delete param;
	return;
}

/////////////////////////////////////////////////////////////////////////////////////////

HANDLE CVkProto::SendFile(MCONTACT hContact, const wchar_t *desc, wchar_t **files)
{
	debugLogA("CVkProto::SendFile");

	VKUserID_t iUserId = ReadVKUserID(hContact);
	if (!IsOnline() || ((iUserId == VK_INVALID_USER || iUserId == VK_FEED_USER) && !isChatRoom(hContact)) || !files || !files[0])
		return (HANDLE)nullptr;

	CVkFileUploadParam *fup = new CVkFileUploadParam(hContact, desc, files);

	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)fup);

	if (!fup->IsAccess()) {
		SendFileFailed(fup, VKERR_FILE_NOT_EXIST);
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
		SendFileFailed(fup, VKERR_FTYPE_NOT_SUPPORTED);
		return (HANDLE)nullptr;
	}
	pReq->pUserInfo = fup;
	Push(pReq);

	if (files[1])
		SendFile(hContact, L"", &files[1]);

	return (HANDLE)fup;
}

void CVkProto::SendFileFailed(CVkFileUploadParam *fup, int ErrorCode)
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


void CVkProto::OnReciveUploadServer(MHttpResponse *reply, AsyncHttpRequest *pReq)
{
	CVkFileUploadParam *fup = (CVkFileUploadParam *)pReq->pUserInfo;
	if (!IsOnline()) {
		SendFileFailed(fup, VKERR_OFFLINE);
		return;
	}

	debugLogA("CVkProto::OnReciveUploadServer %d", reply->resultCode);
	if (reply->resultCode != 200) {
		SendFileFailed(fup, VKERR_INVALID_SERVER);
		return;
	}

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse || pReq->m_iErrorCode) {
		if (!pReq->bNeedsRestart)
			SendFileFailed(fup, pReq->m_iErrorCode);
		return;
	}

	CMStringA uri(jnResponse["upload_url"].as_mstring());
	if (uri.IsEmpty()) {
		SendFileFailed(fup, VKERR_INVALID_URL);
		return;
	}

	FILE *pFile = _wfopen(fup->wszFileName, L"rb");
	if (pFile == nullptr) {
		SendFileFailed(fup, VKERR_ERR_OPEN_FILE);
		return;
	}

	fseek(pFile, 0, SEEK_END);
	long iFileLen = ftell(pFile); //FileSize
	if (iFileLen < 1) {
		fclose(pFile);
		SendFileFailed(fup, VKERR_ERR_READ_FILE);
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
	pUploadReq->m_szParam.Truncate(dataLength);
	char *pData = pUploadReq->m_szParam.GetBuffer();
	memcpy(pData, (void *)DataBegin.GetBuffer(), DataBegin.GetLength());

	pData += DataBegin.GetLength();
	long lBytes = (long)fread(pData, 1, iFileLen, pFile);
	fclose(pFile);

	if (lBytes != iFileLen) {
		SendFileFailed(fup, VKERR_ERR_READ_FILE);
		delete pUploadReq;
		return;
	}

	pData += iFileLen;
	memcpy(pData, (void *)DataEnd.GetBuffer(), DataEnd.GetLength());
	// } Body

	pUploadReq->pUserInfo = pReq->pUserInfo;
	Push(pUploadReq);
}

void CVkProto::OnReciveUpload(MHttpResponse *reply, AsyncHttpRequest *pReq)
{
	CVkFileUploadParam *fup = (CVkFileUploadParam *)pReq->pUserInfo;
	if (!IsOnline()) {
		SendFileFailed(fup, VKERR_OFFLINE);
		return;
	}

	debugLogA("CVkProto::OnReciveUploadServer %d", reply->resultCode);
	if (reply->resultCode != 200) {
		SendFileFailed(fup, VKERR_FILE_NOT_UPLOADED);
		return;
	}

	JSONNode jnRoot;
	CheckJsonResponse(pReq, reply, jnRoot);

	if (pReq->m_iErrorCode) {
		SendFileFailed(fup, pReq->m_iErrorCode);
		return;
	}

	if ((!jnRoot["server"] || !jnRoot["hash"]) && !jnRoot["file"]) {
		SendFileFailed(fup, VKERR_INVALID_PARAMETERS);
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
			SendFileFailed(fup, VKERR_INVALID_PARAMETERS);
			return;
		}
		pUploadReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/photos.saveMessagesPhoto.json", true, &CVkProto::OnReciveUploadFile);
		pUploadReq << WCHAR_PARAM("server", server) << WCHAR_PARAM("photo", upload) << WCHAR_PARAM("hash", hash);
		break;
	case CVkFileUploadParam::typeAudio:
		upload = jnRoot["audio"].as_mstring();
		if (upload == L"[]") {
			SendFileFailed(fup, VKERR_INVALID_PARAMETERS);
			return;
		}
		pUploadReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/audio.save.json", true, &CVkProto::OnReciveUploadFile);
		pUploadReq << WCHAR_PARAM("server", server) << WCHAR_PARAM("audio", upload) << WCHAR_PARAM("hash", hash);
		break;
	case CVkFileUploadParam::typeDoc:
	case CVkFileUploadParam::typeAudioMsg:
		upload = jnRoot["file"].as_mstring();
		if (upload.IsEmpty()) {
			SendFileFailed(fup, VKERR_INVALID_PARAMETERS);
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
		SendFileFailed(fup, VKERR_FTYPE_NOT_SUPPORTED);
		return;
	}

	pUploadReq->pUserInfo = pReq->pUserInfo;
	Push(pUploadReq);
}

void CVkProto::OnReciveUploadFile(MHttpResponse *reply, AsyncHttpRequest *pReq)
{
	CVkFileUploadParam *fup = (CVkFileUploadParam *)pReq->pUserInfo;
	if (!IsOnline()) {
		SendFileFailed(fup, VKERR_OFFLINE);
		return;
	}

	debugLogA("CVkProto::OnReciveUploadFile %d", reply->resultCode);
	if (reply->resultCode != 200) {
		SendFileFailed(fup, VKERR_FILE_NOT_UPLOADED);
		return;
	}

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse || pReq->m_iErrorCode) {
		if (!pReq->bNeedsRestart)
			SendFileFailed(fup, pReq->m_iErrorCode);
		return;
	}

	int id; 
	VKUserID_t iOwnerId;
	if ((fup->GetType() == CVkFileUploadParam::typeDoc) || (fup->GetType() == CVkFileUploadParam::typeAudioMsg)) {
		CMStringA wszType(jnResponse["type"].as_mstring());
		const JSONNode& jnDoc = jnResponse[wszType];
		id = jnDoc["id"].as_int();
		iOwnerId = jnDoc["owner_id"].as_int();
	}
	else {
		id = fup->GetType() == CVkFileUploadParam::typeAudio ? jnResponse["id"].as_int() : (*jnResponse.begin())["id"].as_int();
		iOwnerId = fup->GetType() == CVkFileUploadParam::typeAudio ? jnResponse["owner_id"].as_int() : (*jnResponse.begin())["owner_id"].as_int();
	}

	if ((id == 0) || (iOwnerId == 0)) {
		SendFileFailed(fup, VKERR_INVALID_PARAMETERS);
		return;
	}

	CMStringW Attachment;

	switch (fup->GetType()) {
	case CVkFileUploadParam::typeImg:
		Attachment.AppendFormat(L"photo%d_%d", iOwnerId, id);
		break;
	case CVkFileUploadParam::typeAudio:
		Attachment.AppendFormat(L"audio%d_%d", iOwnerId, id);
		break;
	case CVkFileUploadParam::typeDoc:
	case CVkFileUploadParam::typeAudioMsg:
		Attachment.AppendFormat(L"doc%d_%d", iOwnerId, id);
		break;
	default:
		SendFileFailed(fup, VKERR_FTYPE_NOT_SUPPORTED);
		return;
	}

	AsyncHttpRequest *pMsgReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true, &CVkProto::OnSendMessage, AsyncHttpRequest::rpHigh);

	if (isChatRoom(fup->hContact)) {
		CVkChatInfo *cc = GetChatByContact(fup->hContact);
		if (cc == nullptr) {
			SendFileFailed(fup, VKERR_INVALID_USER);
			return;
		}

		pMsgReq << INT_PARAM("chat_id", cc->m_iChatId);
		pMsgReq->pUserInfo = pReq->pUserInfo;

	}
	else {
		VKUserID_t iUserId = ReadVKUserID(fup->hContact);
		if (iUserId == VK_INVALID_USER || iUserId == VK_FEED_USER) {
			SendFileFailed(fup, VKERR_INVALID_USER);
			return;
		}

		pMsgReq << INT_PARAM("peer_id", iUserId);
		pMsgReq->pUserInfo = new CVkSendMsgParam(fup->hContact, fup);
	}

	ULONG uMsgId = ::InterlockedIncrement(&m_iMsgId);
	pMsgReq 
		<< WCHAR_PARAM("message", fup->wszDesc) 
		<< WCHAR_PARAM("attachment", Attachment)
		<< INT_PARAM("random_id", ((long)time(0)) * 100 + uMsgId % 100);

	pMsgReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	Push(pMsgReq);
}