/*
Copyright (c) 2015-24 Miranda NG team (https://miranda-ng.org)

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

/* MESSAGE SENDING */

// outcoming message flow
int CSkypeProto::SendMsg(MCONTACT hContact, MEVENT, const char *szMessage)
{
	if (!IsOnline())
		return -1;

	SendMessageParam *param = new SendMessageParam();
	param->hContact = hContact;
	Utils_GetRandom(&param->hMessage, sizeof(param->hMessage));
	param->hMessage &= ~0x80000000;

	CMStringA id(getId(hContact));

	AsyncHttpRequest *pReq;
	if (strncmp(szMessage, "/me ", 4) == 0)
		pReq = new SendActionRequest(id, param->hMessage, &szMessage[4], this);
	else
		pReq = new SendMessageRequest(id, param->hMessage, szMessage);
	pReq->pUserInfo = param;
	PushRequest(pReq);

	mir_cslock lck(m_lckOutMessagesList);
	m_OutMessages.insert((void*)param->hMessage);
	return param->hMessage;
}

void CSkypeProto::OnMessageSent(MHttpResponse *response, AsyncHttpRequest *pRequest)
{
	auto *param = (SendMessageParam*)pRequest->pUserInfo;
	MCONTACT hContact = param->hContact;
	HANDLE hMessage = (HANDLE)param->hMessage;
	delete param;

	if (response != nullptr) {
		if (response->resultCode != 201) {
			std::string strError = Translate("Unknown error!");

			if (!response->body.IsEmpty()) {
				JSONNode jRoot = JSONNode::parse(response->body);
				const JSONNode &jErr = jRoot["errorCode"];
				if (jErr)
					strError = jErr.as_string();
			}

			ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, hMessage, _A2T(strError.c_str()));
		}
	}
	else ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, hMessage, (LPARAM)TranslateT("Network error!"));
}

// preparing message/action to be written into db
int CSkypeProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent*)lParam;
	if (mir_strcmp(Proto_GetBaseAccountName(evt->hContact), m_szModuleName))
		return 0;

	char *message = (char*)evt->dbei->pBlob;
	if (strncmp(message, "/me ", 4) == 0) {
		evt->dbei->cbBlob = evt->dbei->cbBlob - 4;
		memmove(evt->dbei->pBlob, &evt->dbei->pBlob[4], evt->dbei->cbBlob);
		evt->dbei->eventType = SKYPE_DB_EVENT_TYPE_ACTION;
	}
	return 0;
}

/* MESSAGE EVENT */

bool CSkypeProto::ParseMessage(const JSONNode &node, DB::EventInfo &dbei)
{
	int nEmoteOffset = node["skypeemoteoffset"].as_int();

	auto &pContent = node["content"];
	if (!pContent) {
LBL_Deleted:
		if (dbei)
			db_event_delete(dbei.getEvent());
		return false;
	}
	
	CMStringW wszContent = pContent.as_mstring();
	if (wszContent.IsEmpty())
		goto LBL_Deleted;

	std::string strMessageType = node["messagetype"].as_string();
	if (strMessageType == "RichText/Media_GenericFile" || strMessageType == "RichText/Media_Video" || strMessageType == "RichText/UriObject" ) {
		ProcessFileRecv(dbei.hContact, node["content"].as_string().c_str(), dbei);
		return false;
	}
	if (strMessageType == "RichText/Contacts") {
		ProcessContactRecv(dbei.hContact, node["content"].as_string().c_str(), dbei);
		return false;
	}

	if (strMessageType == "Text" || strMessageType == "RichText") {
		if ((dbei.flags & DBEF_SENT) && dbei.szId) {
			HANDLE hMessage = (HANDLE)atoi(dbei.szId);
			if (m_OutMessages.getIndex(hMessage) != -1) {
				ProtoBroadcastAck(dbei.hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, hMessage, (LPARAM)dbei.szId);

				mir_cslock lck(m_lckOutMessagesList);
				m_OutMessages.remove(hMessage);
				return false;
			}
		}

		if (strMessageType == "RichText")
			wszContent = RemoveHtml(wszContent);

		dbei.eventType = nEmoteOffset == 0 ? EVENTTYPE_MESSAGE : SKYPE_DB_EVENT_TYPE_ACTION;
	}
	else if (strMessageType == "Event/Call") {
		dbei.eventType = SKYPE_DB_EVENT_TYPE_CALL_INFO;
	}
	else if (strMessageType == "RichText/Files") {
		dbei.eventType = SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO;
	}
	else if (strMessageType == "RichText/Media_FlikMsg") {
		dbei.eventType = SKYPE_DB_EVENT_TYPE_MOJI;
	}
	else if (strMessageType == "RichText/Media_Album") {
		return false;
	}
	else {
		dbei.eventType = SKYPE_DB_EVENT_TYPE_UNKNOWN;
	}

	replaceStr(dbei.pBlob, mir_utf8encodeW(wszContent));
	dbei.cbBlob = (uint32_t)mir_strlen(dbei.pBlob);
	return true;
}

void CSkypeProto::ProcessNewMessage(const JSONNode &node)
{
	int iUserType;
	UrlToSkypeId(node["conversationLink"].as_string().c_str(), &iUserType);

	CMStringA szMessageId = node["id"].as_mstring();
	CMStringA szConversationName(UrlToSkypeId(node["conversationLink"].as_string().c_str()));
	CMStringA szFromSkypename(UrlToSkypeId(node["from"].as_mstring()));

	if (iUserType == 19)
		if (OnChatEvent(node))
			return;

	MCONTACT hContact = AddContact(szConversationName, nullptr, true);

	if (m_bHistorySynced) {
		int64_t lastMsgId = _atoi64(szMessageId);
		if (lastMsgId > getLastTime(hContact))
			setLastTime(hContact, lastMsgId);
	}

	std::string strMessageType = node["messagetype"].as_string();
	if (strMessageType == "Control/Typing") {
		CallService(MS_PROTO_CONTACTISTYPING, hContact, 30);
		return;
	}
	if (strMessageType == "Control/ClearTyping") {
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);
		return;
	}

	DB::EventInfo dbei(db_event_getById(m_szModuleName, szMessageId));
	dbei.hContact = hContact;
	dbei.timestamp = time(0);
	dbei.szId = szMessageId;
	dbei.flags = DBEF_UTF;
	if (IsMe(szFromSkypename))
		dbei.flags |= DBEF_SENT;
	if (iUserType == 19)
		dbei.szUserId = szFromSkypename;

	if (ParseMessage(node, dbei)) {
		if (dbei)
			db_event_edit(dbei.getEvent(), &dbei, true);
		else
			ProtoChainRecvMsg(hContact, dbei);
	}
}

void CSkypeProto::OnMarkRead(MCONTACT hContact, MEVENT hDbEvent)
{
	if (IsOnline()) {
		DB::EventInfo dbei(hDbEvent, false);
		if (dbei && dbei.szId)
			PushRequest(new MarkMessageReadRequest(getId(hContact), _atoi64(dbei.szId)));
	}
}

void CSkypeProto::OnReceiveOfflineFile(DB::FILE_BLOB &blob)
{
	if (auto *ft = (CSkypeTransfer *)blob.getUserInfo()) {
		blob.setUrl(ft->url);
		blob.setSize(ft->iFileSize);
		delete ft;
	}
}

void CSkypeProto::ProcessFileRecv(MCONTACT hContact, const char *szContent, DB::EventInfo &dbei)
{
	TiXmlDocument doc;
	if (0 != doc.Parse(szContent))
		return;

	auto *xmlRoot = doc.FirstChildElement("URIObject");
	if (xmlRoot == nullptr)
		return;

	const char *pszFileType = 0;
	CSkypeTransfer *ft = new CSkypeTransfer;
	if (auto *str = xmlRoot->Attribute("doc_id"))
		ft->docId = str;
	if (auto *str = xmlRoot->Attribute("uri"))
		ft->url = str;
	int iWidth = xmlRoot->IntAttribute("width", -1);
	int iHeight = xmlRoot->IntAttribute("heighr", -1);
	if (auto *str = xmlRoot->Attribute("type"))
		pszFileType = str;
	if (auto *xml = xmlRoot->FirstChildElement("FileSize"))
		if (auto *str = xml->Attribute("v"))
			ft->iFileSize = atoi(str);
	if (auto *xml = xmlRoot->FirstChildElement("OriginalName"))
		if (auto *str = xml->Attribute("v"))
			ft->fileName = str;

	if (ft->url.IsEmpty() || ft->fileName.IsEmpty() || ft->iFileSize == 0) {
		debugLogA("Missing file info: url=<%s> name=<%s> %d", ft->url.c_str(), ft->fileName.c_str(), ft->iFileSize);
		delete ft;
		return;
	}

	// ordinary file
	if (!mir_strcmp(pszFileType, "File.1") || !mir_strcmp(pszFileType, "Picture.1") || !mir_strcmp(pszFileType, "Video.1")) {
		MEVENT hEvent;
		dbei.flags |= DBEF_TEMPORARY | DBEF_JSON;
		if (dbei) {
			DB::FILE_BLOB blob(dbei);
			OnReceiveOfflineFile(blob);
			blob.write(dbei);
			db_event_edit(dbei.getEvent(), &dbei, true);
			delete ft;
			hEvent = dbei.getEvent();
		}
		else hEvent = ProtoChainRecvFile(hContact, DB::FILE_BLOB(ft, ft->fileName), dbei);

		DBVARIANT dbv = { DBVT_UTF8 };
		dbv.pszVal = (char*)pszFileType;
		db_event_setJson(hEvent, "skft", &dbv);

		dbv.type = DBVT_DWORD;
		if (iWidth != -1) {
			dbv.dVal = iWidth;
			db_event_setJson(hEvent, "w", &dbv);
		}
		if (iHeight != -1) {
			dbv.dVal = iHeight;
			db_event_setJson(hEvent, "h", &dbv);
		}
	}
	else debugLogA("Invalid or unsupported file type <%s> ignored", pszFileType);
}

void CSkypeProto::ProcessContactRecv(MCONTACT hContact, const char *szContent, DB::EventInfo &dbei)
{
	TiXmlDocument doc;
	if (0 != doc.Parse(szContent))
		return;

	auto *xmlNode = doc.FirstChildElement("contacts");
	if (xmlNode == nullptr)
		return;

	int nCount = 0;
	for (auto *it : TiXmlEnum(xmlNode)) {
		UNREFERENCED_PARAMETER(it);
		nCount++;
	}

	PROTOSEARCHRESULT **psr = (PROTOSEARCHRESULT**)mir_calloc(sizeof(PROTOSEARCHRESULT*) * nCount);
	
	nCount = 0;
	for (auto *xmlContact : TiXmlFilter(xmlNode, "c")) {
		psr[nCount] = (PROTOSEARCHRESULT*)mir_calloc(sizeof(PROTOSEARCHRESULT));
		psr[nCount]->cbSize = sizeof(psr);
		psr[nCount]->id.a = mir_strdup(xmlContact->Attribute("s"));
		nCount++;
	}

	if (nCount) {
		dbei.pBlob = (char*)psr;
		dbei.cbBlob = nCount;

		ProtoChainRecv(hContact, PSR_CONTACTS, 0, (LPARAM)&dbei);
		for (int i = 0; i < nCount; i++) {
			mir_free(psr[i]->id.a);
			mir_free(psr[i]);
		}
	}
	mir_free(psr);
}
