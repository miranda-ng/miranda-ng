/*
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

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

// returns 0, if error or no events
uint32_t JabberGetLastContactMessageTime(MCONTACT hContact)
{
	// TODO: time cache can improve performance
	MEVENT hDbEvent = db_event_last(hContact);
	if (!hDbEvent)
		return 0;

	DB::EventInfo dbei(hDbEvent, false);
	return (dbei) ? dbei.getUnixtime() : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// common message processing function

void CJabberProto::MessageProcess(XmppMsg &M)
{
	if (M.from == nullptr) {
		debugLogA("no 'M.from' attribute, returning");
		return;
	}

	MessageHandleMam(M);

	M.pFromResource = new pResourceStatus(ResourceInfoFromJID(M.from));

	// Message receipts delivery request. Reply here, before a call to HandleMessagePermanent() to make sure message receipts are handled for external plugins too.
	if (IsSendAck(HContactFromJID(M.from)) && M.bEnableDelivery && (!M.type || mir_strcmpi(M.type, "error"))) {
		bool bSendReceipt = XmlGetChildByTag(M.node, "request", "xmlns", JABBER_FEAT_MESSAGE_RECEIPTS) != 0;
		bool bSendMark = XmlGetChildByTag(M.node, "markable", "xmlns", JABBER_FEAT_CHAT_MARKERS) != 0;
		if (bSendReceipt || bSendMark) {
			XmlNode reply("message"); reply << XATTR("to", M.from) << XATTR("id", M.idStr);
			if (bSendReceipt) {
				if (*M.pFromResource)
					(*M.pFromResource)->m_jcbManualDiscoveredCaps |= JABBER_CAPS_MESSAGE_RECEIPTS;
				reply << XCHILDNS("received", JABBER_FEAT_MESSAGE_RECEIPTS) << XATTR("id", M.idStr);
			}
			if (bSendMark) {
				if (*M.pFromResource)
					(*M.pFromResource)->m_jcbManualDiscoveredCaps |= JABBER_CAPS_CHAT_MARKERS;
				reply << XCHILDNS("received", JABBER_FEAT_CHAT_MARKERS) << XATTR("id", M.idStr);
			}
			M.info->send(reply);
		}
	}

	if (m_messageManager.HandleMessagePermanent(M.node, M.info)) {
		debugLogA("permanent message handler succeeded, returning");
		return;
	}

	MessageHandleCarbon(M);

	M.hContact = HContactFromJID(M.from);
	JABBER_LIST_ITEM *chatItem = ListGetItemPtr(LIST_CHATROOM, M.from);
	if (chatItem) {
		auto *xCaptcha = XmlFirstChild(M.node, "captcha");
		if (xCaptcha)
			if (ProcessCaptcha(xCaptcha, M.node, m_ThreadInfo)) {
				debugLogA("captcha processing succeeded, returning");
				return;
			}
	}

	auto *bodyNode = XmlGetChildByTag(M.node, "body", "xml:lang", m_tszSelectedLang);
	if (bodyNode == nullptr)
		bodyNode = XmlFirstChild(M.node, "body");

	auto *subject = XmlGetChildText(M.node, "subject");
	if (subject) {
		M.szMessage.Append("Subject: ");
		M.szMessage.Append(subject);
		M.szMessage.Append("\r\n");
	}

	if (M.szMessage) if (auto *n = XmlGetChildByTag(M.node, "addresses", "xmlns", JABBER_FEAT_EXT_ADDRESSING)) {
		auto *addressNode = XmlGetChildByTag(n, "address", "type", "oM.from");
		if (addressNode) {
			const char *szJid = XmlGetAttr(addressNode, "jid");
			if (szJid) {
				M.szMessage.AppendFormat(TranslateU("Message redirected M.from: %s\r\n"), M.from);
				M.from = szJid;
				// rewrite hContact
				M.hContact = HContactFromJID(M.from);
			}
		}
	}

	if (bodyNode != nullptr)
		M.szMessage.Append(bodyNode->GetText());

	// If a message is M.from a stranger (not in roster), item is nullptr
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, M.from);
	if (item == nullptr)
		item = ListGetItemPtr(LIST_VCARD_TEMP, M.from);

	MessageHandleChatstates(M);

	// message receipts delivery notification
	if (auto *n = XmlGetChildByTag(M.node, "received", "xmlns", JABBER_FEAT_MESSAGE_RECEIPTS)) {
		int nPacketId = JabberGetPacketID(n);
		if (nPacketId == -1)
			nPacketId = JabberGetPacketID(M.node);
		if (nPacketId != -1)
			ProtoBroadcastAck(M.hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)nPacketId);

		db_event_delivered(M.hContact, 0);
	}

	if (auto *n = XmlGetChildByTag(M.node, "displayed", "xmlns", JABBER_FEAT_CHAT_MARKERS))
		if (g_plugin.bMessageState)
			CallService(MS_MESSAGESTATE_UPDATE, M.hContact, MRD_TYPE_READ);

	// Timestamp
	time_t now = time(0);
	bool bOffline = false;
	if (!M.msgTime) {
		if (auto *n = JabberProcessDelay(M.node, M.msgTime)) {
			if ((m_ThreadInfo->jabberServerCaps & JABBER_CAPS_MSGOFFLINE) && !mir_strcmp(n->GetText(), "Offline Storage"))
				bOffline = true;
		}
		else M.msgTime = now;
	}

	if (m_bFixIncorrectTimestamps && (M.msgTime > now || (M.hContact && (M.msgTime < (time_t)JabberGetLastContactMessageTime(M.hContact)))))
		M.msgTime = now;

	// XEP-0224 support (Attention/Nudge)
	if (XmlGetChildByTag(M.node, "attention", "xmlns", JABBER_FEAT_ATTENTION)) {
		if (!M.hContact)
			M.hContact = CreateTemporaryContact(M.from, chatItem);
		if (M.hContact)
			NotifyEventHooks(m_hEventNudge, M.hContact, 0);
	}

	if (auto *n = XmlGetChildByTag(M.node, "confirm", "xmlns", JABBER_FEAT_HTTP_AUTH)) if (m_bAcceptHttpAuth) {
		auto *szId = XmlGetAttr(n, "id");
		auto *szMethod = XmlGetAttr(n, "method");
		auto *szUrl = XmlGetAttr(n, "url");
		if (!szId || !szMethod || !szUrl) {
			debugLogA("missing attributes in confirm, returning");
			return;
		}

		CJabberHttpAuthParams *pParams = (CJabberHttpAuthParams *)mir_calloc(sizeof(CJabberHttpAuthParams));
		memset(pParams, 0, sizeof(CJabberHttpAuthParams));
		pParams->m_nType = CJabberHttpAuthParams::MSG;
		pParams->m_szFrom = mir_strdup(M.from);
		pParams->m_szThreadId = mir_strdup(XmlGetChildText(M.node, "thread"));
		pParams->m_szId = mir_strdup(szId);
		pParams->m_szMethod = mir_strdup(szMethod);
		pParams->m_szUrl = mir_strdup(szUrl);

		AddClistHttpAuthEvent(pParams);
		debugLogA("http auth event added, returning");
		return;
	}

	MessageHandleOmemo(M);

	// parsing extensions
	for (auto *xNode : TiXmlEnum(M.node)) {
		if (0 != mir_strcmp(xNode->Name(), "x"))
			continue;

		auto *pszXmlns = XmlGetAttr(xNode, "xmlns");
		if (pszXmlns == nullptr)
			continue;

		if (!mir_strcmp(pszXmlns, JABBER_FEAT_MIRANDA_NOTES)) {
			if (OnIncomingNote(M.from, XmlFirstChild(xNode, "note"))) {
				debugLogA("OMEMO: no 'note' attribute, returning");
				return;
			}
		}
		else if (!mir_strcmp(pszXmlns, "jabber:x:encrypted")) {
			auto *ptszText = xNode->GetText();
			if (ptszText == nullptr) {
				debugLogA("OMEMO: no 'encrypted' attribute, returning");
				return;
			}

			if (M.carbon && M.dbei.bSent)
				M.szMessage = TranslateU("Unable to decrypt a carbon copy of the encrypted outgoing message");
			else {
				// XEP-0027 is not strict enough, different clients have different implementations
				// additional validation is required

				CMStringA tempstring;
				if (!strstr(ptszText, PGP_PROLOG))
					tempstring.Format("%s%s%s", PGP_PROLOG, ptszText, PGP_EPILOG);
				else
					tempstring = ptszText;

				M.szMessage += tempstring;
			}
		}
		else if (!mir_strcmp(pszXmlns, JABBER_FEAT_OOB2)) {
			if (auto *url = XmlGetChildText(xNode, "url"))
				FileProcessHttpDownload(M.dbei, url, XmlGetChildText(xNode, "desc"));
			else
				debugLogA("No URL in OOB file transfer, ignoring");
		}
		else if (!mir_strcmp(pszXmlns, JABBER_FEAT_MUC_USER)) {
			auto *inviteNode = XmlFirstChild(xNode, "invite");
			if (inviteNode != nullptr) {
				auto *inviteReason = XmlGetChildText(inviteNode, "reason");
				if (inviteReason == nullptr)
					inviteReason = M.szMessage;
				if (!(m_bIgnoreMUCInvites))
					GroupchatProcessInvite(M.from, XmlGetAttr(inviteNode, "from"), inviteReason, XmlGetChildText(xNode, "password"));
				return;
			}
		}
		else if (!mir_strcmp(pszXmlns, JABBER_FEAT_ROSTER_EXCHANGE) && item != nullptr && (item->subscription == SUB_BOTH || item->subscription == SUB_TO)) {
			char chkJID[JABBER_MAX_JID_LEN] = "@";
			JabberStripJid(M.from, chkJID + 1, _countof(chkJID) - 1);
			for (auto *iNode : TiXmlFilter(xNode, "item")) {
				auto *action = XmlGetAttr(iNode, "action");
				auto *jid = XmlGetAttr(iNode, "jid");
				auto *nick = XmlGetAttr(iNode, "name");
				auto *group = XmlGetChildText(iNode, "group");
				if (action && jid && strstr(jid, chkJID)) {
					if (!mir_strcmp(action, "add")) {
						MCONTACT cc = DBCreateContact(jid, nick, false, false);
						if (group)
							db_set_utf(cc, "CList", "Group", group);
					}
					else if (!mir_strcmp(action, "delete")) {
						MCONTACT cc = HContactFromJID(jid);
						if (cc)
							db_delete_contact(cc, CDF_FROM_SERVER);
					}
				}
			}
		}
		else if (!mir_strcmp(pszXmlns, JABBER_FEAT_DIRECT_MUC_INVITE)) {
			auto *inviteReason = xNode->GetText();
			if (!inviteReason)
				inviteReason = M.szMessage;
			if (!(m_bIgnoreMUCInvites))
				GroupchatProcessInvite(XmlGetAttr(xNode, "jid"), M.from, inviteReason, nullptr);
			return;
		}
	}

	M.szMessage += ExtractImage(M.node);

	// all service info was already processed
	if (M.dbei.eventType == EVENTTYPE_MESSAGE && M.szMessage.IsEmpty()) {
		debugLogA("empty message, returning");
		return;
	}

	// we ignore messages without a server id either if MAM is enabled
	if ((m_ThreadInfo->jabberServerCaps & JABBER_CAPS_MAM) && m_bEnableMam && m_iMamMode != 0
		&& M.szMamMsgId == nullptr) {
		debugLogA("MAM is enabled, but there's no stanza-id: ignoting a message");
		return;
	}

	M.szMessage.Replace("\n", "\r\n");

	if (item != nullptr) {
		if (M.pFromResource) {
			JABBER_RESOURCE_STATUS *pLast = item->m_pLastSeenResource;
			item->m_pLastSeenResource = *M.pFromResource;
			if (item->resourceMode == RSMODE_LASTSEEN && pLast == *M.pFromResource)
				UpdateMirVer(item);
		}
	}

	// Create a temporary contact, if needed
	if (M.hContact == 0) {
		if (item)
			M.hContact = item->hContact;
		else
			M.hContact = CreateTemporaryContact(M.from, chatItem);
	}
	if (!bOffline)
		CallService(MS_PROTO_CONTACTISTYPING, M.hContact, PROTOTYPE_CONTACTTYPING_OFF);

	// Add a message to database
	M.dbei.iTimestamp = (uint32_t)M.msgTime;
	M.dbei.szId = M.szMamMsgId;

	MEVENT hDbEVent;
	if (M.dbei.eventType == EVENTTYPE_FILE) {
		M.dbei.flags |= DBEF_TEMPORARY;
		hDbEVent = (MEVENT)ProtoChainRecvFile(M.hContact, DB::FILE_BLOB(M.dbei), M.dbei);
	}
	else {
		M.dbei.pBlob = M.szMessage.GetBuffer();
		hDbEVent = (MEVENT)ProtoChainRecvMsg(M.hContact, M.dbei);
	}

	if (M.idStr)
		m_arChatMarks.insert(new CChatMark(hDbEVent, M.idStr, M.from));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Message Archive processing

void CJabberProto::MessageHandleMam(XmppMsg &M)
{
	if (auto *mamResult = XmlGetChildByTag(M.node, "result", "xmlns", JABBER_FEAT_MAM)) {
		M.dbei.bRead = true;
		M.szMamMsgId = XmlGetAttr(mamResult, "id");
		if (M.szMamMsgId)
			setString("LastMamId", M.szMamMsgId);

		auto *xmlForwarded = XmlGetChildByTag(mamResult, "forwarded", "xmlns", JABBER_XMLNS_FORWARD);
		if (auto *xmlMessage = XmlFirstChild(xmlForwarded, "message")) {
			M.node = xmlMessage;
			M.type = XmlGetAttr(M.node, "type");
			M.from = XmlGetAttr(M.node, "from");
			auto *to = XmlGetAttr(M.node, "to");

			char szJid[JABBER_MAX_JID_LEN];
			JabberStripJid(M.from, szJid, _countof(szJid));
			if (!mir_strcmpi(szJid, m_szJabberJID)) {
				M.dbei.bSent = true;
				std::swap(M.from, to);
			}

			// we disable message reading with our resource only for the missing messages
			if (!m_bMamCreateRead && !mir_strcmpi(to, M.info->fullJID)) {
				debugLogA("MAM: outgoing message M.from this machine (%s), ignored", M.from);
				return;
			}
		}

		JabberProcessDelay(xmlForwarded, M.msgTime);

		M.bEnableDelivery = false;
		M.dbei.bRead = m_bMamCreateRead;
	}

	if (auto *n = XmlGetChildByTag(M.node, "stanza-id", "xmlns", JABBER_FEAT_SID))
		if (M.szMamMsgId = n->Attribute("id"))
			setString("LastMamId", M.szMamMsgId);
}

/////////////////////////////////////////////////////////////////////////////////////////
// carbon copies processing

void CJabberProto::MessageHandleCarbon(XmppMsg &M)
{
	// Handle carbons. The message MUST be coming M.from our bare JID.
	if (IsMyOwnJID(M.from)) {
		M.carbon = XmlGetChildByTag(M.node, "received", "xmlns", JABBER_FEAT_CARBONS);
		if (!M.carbon) {
			if (M.carbon = XmlGetChildByTag(M.node, "sent", "xmlns", JABBER_FEAT_CARBONS))
				M.dbei.bSent = true;
		}
		if (M.carbon) {
			// If carbons are disabled in options, we should ignore occasional carbons sent to us by server
			if (!m_bEnableCarbons) {
				debugLogA("carbons aren't enabled, returning");
				return;
			}

			M.dbei.bRead = true;
			auto *xmlForwarded = XmlGetChildByTag(M.carbon, "forwarded", "xmlns", JABBER_XMLNS_FORWARD);
			auto *xmlMessage = XmlFirstChild(xmlForwarded, "message");
			// Carbons MUST have forwarded/message content
			if (xmlMessage == nullptr) {
				debugLogA("no 'forwarded' attribute in carbons, returning");
				return;
			}

			// Unwrap the carbon in any case
			M.node = xmlMessage;
			M.type = XmlGetAttr(M.node, "type");

			if (!M.dbei.bSent) {
				// Received should just be treated like incoming messages, except maybe not flash the flasher. Simply unwrap.
				M.from = XmlGetAttr(M.node, "from");
				if (M.from == nullptr) {
					debugLogA("no 'M.from' attribute in carbons, returning");
					return;
				}
			}
			else {
				// Sent should set SENT flag and invert M.from/to.
				M.from = XmlGetAttr(M.node, "to");
				if (M.from == nullptr) {
					debugLogA("no 'to' attribute in carbons, returning");
					return;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// OMEMO processing

bool CJabberProto::MessageHandleOmemo(XmppMsg &M)
{
	if (m_bUseOMEMO) {
		if (auto *encNode = XmlGetChildByTag(M.node, "encrypted", "xmlns", JABBER_FEAT_OMEMO)) {
			OmemoHandleMessage(&M, encNode, M.from, M.msgTime, M.dbei.bSent);
			return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// chat states' processing

void CJabberProto::MessageHandleChatstates(XmppMsg &M)
{
	// check chatstates availability
	if (*M.pFromResource && XmlGetChildByTag(M.node, "active", "xmlns", JABBER_FEAT_CHATSTATES))
		(*M.pFromResource)->m_jcbManualDiscoveredCaps |= JABBER_CAPS_CHATSTATES;

	// chatstates composing event
	if (M.hContact && XmlGetChildByTag(M.node, "composing", "xmlns", JABBER_FEAT_CHATSTATES))
		CallService(MS_PROTO_CONTACTISTYPING, M.hContact, 60);

	// chatstates paused event
	if (M.hContact && XmlGetChildByTag(M.node, "paused", "xmlns", JABBER_FEAT_CHATSTATES))
		CallService(MS_PROTO_CONTACTISTYPING, M.hContact, PROTOTYPE_CONTACTTYPING_OFF);

	// chatstates inactive event
	if (M.hContact && XmlGetChildByTag(M.node, "inactive", "xmlns", JABBER_FEAT_CHATSTATES))
		CallService(MS_PROTO_CONTACTISTYPING, M.hContact, PROTOTYPE_CONTACTTYPING_OFF);
	
	// chatstates gone event
	if (M.hContact && XmlGetChildByTag(M.node, "gone", "xmlns", JABBER_FEAT_CHATSTATES) && m_bLogChatstates) {
		char bEventType = JABBER_DB_EVENT_CHATSTATES_GONE; // gone event
		DBEVENTINFO _dbei = {};
		_dbei.pBlob = &bEventType;
		_dbei.cbBlob = 1;
		_dbei.eventType = EVENTTYPE_JABBER_CHATSTATES;
		_dbei.flags = DBEF_READ;
		_dbei.iTimestamp = time(0);
		_dbei.szModule = m_szModuleName;
		db_event_add(M.hContact, &_dbei);
	}
}
