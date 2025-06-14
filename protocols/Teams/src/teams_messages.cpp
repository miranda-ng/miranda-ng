/*
Copyright (c) 2015-25 Miranda NG team (https://miranda-ng.org)

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

/////////////////////////////////////////////////////////////////////////////////////////
// MESSAGE SENDING

void CTeamsProto::OnMessageSent(MHttpResponse *response, AsyncHttpRequest *pRequest)
{
	// to delete it in any case
	std::unique_ptr<COwnMessage> pMessage((COwnMessage *)pRequest->pUserInfo);

	MCONTACT hContact = pRequest->hContact;
	if (response == nullptr) {
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, pRequest->pUserInfo, (LPARAM)TranslateT("Network error!"));
		return;
	}

	if (response->resultCode == 201) {
		JsonReply reply(response);
		auto &pRoot = reply.data();

		if (pMessage) {
			if (!Contact::IsGroupChat(hContact)) {
				pMessage->iTimestamp = _wtoi64(pRoot["OriginalArrivalTime"].as_mstring());

				CMStringA szMsgId(FORMAT, "%lld", pMessage->hClientMessageId);
				ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)pMessage->hMessage, (LPARAM)szMsgId.c_str());
			}

			mir_cslock lck(m_lckOutMessagesList);
			m_OutMessages.remove(pMessage.get());
		}
	}
	else {
		std::string strError = Translate("Unknown error!");

		if (!response->body.IsEmpty()) {
			JSONNode jRoot = JSONNode::parse(response->body);
			const JSONNode &jErr = jRoot["errorCode"];
			if (jErr)
				strError = jErr.as_string();
		}

		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, pRequest->pUserInfo, _A2T(strError.c_str()));
	}
}

// outcoming message flow

int CTeamsProto::SendServerMsg(MCONTACT hContact, const char *szMessage, int64_t existingMsgId)
{
	if (!IsOnline())
		return -1;

	CMStringA str(szMessage);
	bool bRich = AddBbcodes(str);
	m_iMessageId++;

	CMStringA szChatId(getMStringA(hContact, "ChatId"));
	if (szChatId.IsEmpty())
		szChatId = getId(hContact);

	CMStringA szUrl = "/users/ME/conversations/" + mir_urlEncode(szChatId) + "/messages";
	if (existingMsgId)
		szUrl.AppendFormat("/%lld", existingMsgId);

	JSONNode node;
	node << CHAR_PARAM("messagetype", bRich ? "RichText" : "Text") << CHAR_PARAM("contenttype", "text");
	if (strncmp(str, "/me ", 4) == 0)
		node << CHAR_PARAM("content", m_szSkypename + " " + str);
	else
		node << CHAR_PARAM("content", str);
	
	COwnMessage *pOwnMessage = nullptr;
	if (!existingMsgId) {
		int64_t iRandomId = getRandomId();
		node << INT64_PARAM("clientmessageid", iRandomId);
			
		mir_cslock lck(m_lckOutMessagesList);
		m_OutMessages.insert(pOwnMessage = new COwnMessage(m_iMessageId, iRandomId));
	}

	AsyncHttpRequest *pReq = new AsyncHttpRequest(existingMsgId ? REQUEST_PUT : REQUEST_POST, HOST_CHATS, szUrl, &CTeamsProto::OnMessageSent);
	pReq->hContact = hContact;
	pReq->pUserInfo = pOwnMessage;
	pReq->m_szParam = node.write().c_str();
	PushRequest(pReq);

	return m_iMessageId;
}

// preparing message/action to be written into db
int CTeamsProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent*)lParam;
	if (mir_strcmp(Proto_GetBaseAccountName(evt->hContact), m_szModuleName))
		return 0;

	auto &dbei = evt->dbei;
	if (dbei->szId) {
		int64_t msgId = _atoi64(dbei->szId);
		for (auto &it : m_OutMessages) {
			if (it->hClientMessageId == msgId) {
				dbei->bMsec = true;
				dbei->iTimestamp = it->iTimestamp;
	}	}	}

	return 0;
}

/* MESSAGE EVENT */

bool CTeamsProto::ParseMessage(const JSONNode &node, DB::EventInfo &dbei)
{
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

	if (strMessageType == "Text") {
		dbei.eventType = EVENTTYPE_MESSAGE;
	}
	else if (strMessageType == "RichText/Html" || strMessageType == "RichText") {
		wszContent = RemoveHtml(wszContent);
		dbei.eventType = EVENTTYPE_MESSAGE;
	}
	else if (strMessageType == "RichText/Media_Album")
		return false;

	replaceStr(dbei.pBlob, mir_utf8encodeW(wszContent));
	dbei.cbBlob = (uint32_t)mir_strlen(dbei.pBlob);
	return true;
}

void CTeamsProto::ProcessNewMessage(const JSONNode &node)
{
	int iUserType;
	UrlToSkypeId(node["conversationLink"].as_string().c_str(), &iUserType);

	int64_t timestamp = _wtoi64(node["id"].as_mstring());
	CMStringA szMessageId(getMessageId(node));
	CMStringA szConversationName(UrlToSkypeId(node["conversationLink"].as_string().c_str()));
	CMStringA szFromSkypename(UrlToSkypeId(node["from"].as_mstring()));

	if (iUserType == 19)
		if (OnChatEvent(node))
			return;

	MCONTACT hContact = AddContact(szConversationName, nullptr, true);

	if (m_bHistorySynced && timestamp > getLastTime(hContact))
		setLastTime(hContact, timestamp);

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
	dbei.iTimestamp = timestamp;
	dbei.szId = szMessageId;
	dbei.bUtf = dbei.bMsec = true;
	dbei.bSent = IsMe(szFromSkypename);
	if (iUserType == 19)
		dbei.szUserId = szFromSkypename;

	if (ParseMessage(node, dbei)) {
		if (dbei)
			db_event_edit(dbei.getEvent(), &dbei, true);
		else
			ProtoChainRecvMsg(hContact, dbei);
	}
}

void CTeamsProto::OnMarkRead(MCONTACT hContact, MEVENT hDbEvent)
{
	if (IsOnline()) {
		DB::EventInfo dbei(hDbEvent, false);
		if (dbei && dbei.szId) {
			auto *pReq = new AsyncHttpRequest(REQUEST_PUT, HOST_CHATS, "/users/ME/conversations/" + mir_urlEncode(getId(hContact)) + "/properties?name=consumptionhorizon");
			auto msgTimestamp = _atoi64(dbei.szId);

			JSONNode node(JSON_NODE);
			node << CHAR_PARAM("consumptionhorizon", CMStringA(::FORMAT, "%lld;%lld;%lld", msgTimestamp, msgTimestamp, msgTimestamp));
			pReq->m_szParam = node.write().c_str();
			
			PushRequest(pReq);
		}
	}
}

void CTeamsProto::OnReceiveOfflineFile(DB::EventInfo &dbei, DB::FILE_BLOB &blob)
{
	if (auto *ft = (CSkypeTransfer *)blob.getUserInfo()) {
		blob.setUrl(ft->url);
		blob.setSize(ft->iFileSize);

		auto &json = dbei.setJson();
		json << CHAR_PARAM("skft", ft->fileType);
		if (ft->iHeight != -1)
			json << INT_PARAM("h", ft->iHeight);
		if (ft->iWidth != -1)
			json << INT_PARAM("w", ft->iWidth);
		delete ft;
	}
}

void CTeamsProto::ProcessFileRecv(MCONTACT hContact, const char *szContent, DB::EventInfo &dbei)
{
	TiXmlDocument doc;
	if (0 != doc.Parse(szContent))
		return;

	auto *xmlRoot = doc.FirstChildElement("URIObject");
	if (xmlRoot == nullptr)
		return;

	CSkypeTransfer *ft = new CSkypeTransfer;
	if (auto *str = xmlRoot->Attribute("doc_id"))
		ft->docId = str;
	if (auto *str = xmlRoot->Attribute("uri"))
		ft->url = str;
	ft->iWidth = xmlRoot->IntAttribute("width", -1);
	ft->iHeight = xmlRoot->IntAttribute("heighr", -1);
	if (auto *str = xmlRoot->Attribute("type"))
		ft->fileType = str;
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

	int idx = ft->fileType.Find('/');
	if (idx != -1)
		ft->fileType = ft->fileType.Left(idx);

	// ordinary file
	if (ft->fileType == "File.1" || ft->fileType == "Picture.1" || ft->fileType == "Video.1") {
		MEVENT hEvent;
		dbei.flags |= DBEF_TEMPORARY | DBEF_JSON;
		if (dbei) {
			DB::FILE_BLOB blob(dbei);
			OnReceiveOfflineFile(dbei, blob);
			blob.write(dbei);
			db_event_edit(dbei.getEvent(), &dbei, true);
			delete ft;
			hEvent = dbei.getEvent();
		}
		else hEvent = ProtoChainRecvFile(hContact, DB::FILE_BLOB(ft, ft->fileName), dbei);
	}
	else debugLogA("Invalid or unsupported file type <%s> ignored", ft->fileType.c_str());
}

void CTeamsProto::ProcessContactRecv(MCONTACT hContact, const char *szContent, DB::EventInfo &dbei)
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
