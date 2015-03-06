/*
 * WAConnection.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include "../common.h" // #TODO Remove Miranda-dependency

#include "ProtocolTreeNode.h"
#include "utilities.h"

const char* dictionary[] = {
	"", "", "", "account", "ack", "action", "active", "add", "after", "all", "allow", "apple", "auth", "author", "available",
	"bad-protocol", "bad-request", "before", "body", "broadcast", "cancel", "category", "challenge", "chat", "clean", "code",
	"composing", "config", "contacts", "count", "create", "creation", "debug", "default", "delete", "delivery", "delta", "deny",
	"digest", "dirty", "duplicate", "elapsed", "enable", "encoding", "error", "event", "expiration", "expired", "fail", "failure",
	"false", "favorites", "feature", "features", "feature-not-implemented", "field", "first", "free", "from", "g.us", "get", "google",
	"group", "groups", "groups_v2", "http://etherx.jabber.org/streams", "http://jabber.org/protocol/chatstates", "ib", "id", "image",
	"img", "index", "internal-server-error", "ip", "iq", "item-not-found", "item", "jabber:iq:last", "jabber:iq:privacy", "jabber:x:event",
	"jid", "kind", "last", "leave", "list", "max", "mechanism", "media", "message_acks", "message", "method", "microsoft", "missing",
	"modify", "mute", "name", "nokia", "none", "not-acceptable", "not-allowed", "not-authorized", "notification", "notify", "off",
	"offline", "order", "owner", "owning", "p_o", "p_t", "paid", "participant", "participants", "participating", "paused", "picture",
	"pin", "ping", "platform", "port", "presence", "preview", "probe", "prop", "props", "query", "raw", "read", "readreceipts", "reason",
	"receipt", "relay", "remote-server-timeout", "remove", "request", "required", "resource-constraint", "resource", "response", "result",
	"retry", "rim", "s_o", "s_t", "s.us", "s.whatsapp.net", "seconds", "server-error", "server", "service-unavailable", "set", "show", "silent",
	"stat", "status", "stream:error", "stream:features", "subject", "subscribe", "success", "sync", "t", "text", "timeout", "timestamp", "to",
	"true", "type", "unavailable", "unsubscribe", "uri", "url", "urn:ietf:params:xml:ns:xmpp-sasl", "urn:ietf:params:xml:ns:xmpp-stanzas",
	"urn:ietf:params:xml:ns:xmpp-streams", "urn:xmpp:ping", "urn:xmpp:whatsapp:account", "urn:xmpp:whatsapp:dirty", "urn:xmpp:whatsapp:mms",
	"urn:xmpp:whatsapp:push", "urn:xmpp:whatsapp", "user", "user-not-found", "value", "version", "w:g", "w:p:r", "w:p", "w:profile:picture",
	"w", "wait", "WAUTH-2", "xmlns:stream", "xmlns", "1", "chatstate", "crypto", "phash", "enc", "class", "off_cnt", "w:g2", "promote",
	"demote", "creator", "Bell.caf", "Boing.caf", "Glass.caf", "Harp.caf", "TimePassing.caf", "Tri-tone.caf", "Xylophone.caf", "background",
	"backoff", "chunked", "context", "full", "in", "interactive", "out", "registration", "sid", "urn:xmpp:whatsapp:sync", "flt", "s16", "u8",
	"adpcm", "amrnb", "amrwb", "mp3", "pcm", "qcelp", "wma", "h263", "h264", "jpeg"
};

const char* extended_dict[] = {
	"mpeg4", "wmv", "audio/3gpp", "audio/aac", "audio/amr", "audio/mp4", "audio/mpeg", "audio/ogg", "audio/qcelp", "audio/wav",
	"audio/webm", "audio/x-caf", "audio/x-ms-wma", "image/gif", "image/jpeg", "image/png", "video/3gpp", "video/avi", "video/mp4",
	"video/mpeg", "video/quicktime", "video/x-flv", "video/x-ms-asf", "302", "400", "401", "402", "403", "404", "405", "406", "407",
	"409", "410", "500", "501", "503", "504", "abitrate", "acodec", "app_uptime", "asampfmt", "asampfreq", "audio", "clear", "conflict",
	"conn_no_nna", "cost", "currency", "duration", "extend", "file", "fps", "g_notify", "g_sound", "gcm", "gone", "google_play", "hash",
	"height", "invalid", "jid-malformed", "latitude", "lc", "lg", "live", "location", "log", "longitude", "max_groups", "max_participants",
	"max_subject", "mimetype", "mode", "napi_version", "normalize", "orighash", "origin", "passive", "password", "played",
	"policy-violation", "pop_mean_time", "pop_plus_minus", "price", "pricing", "redeem", "Replaced by new connection", "resume",
	"signature", "size", "sound", "source", "system-shutdown", "username", "vbitrate", "vcard", "vcodec", "video", "width",
	"xml-not-well-formed", "checkmarks", "image_max_edge", "image_max_kbytes", "image_quality", "ka", "ka_grow", "ka_shrink", "newmedia",
	"library", "caption", "forward", "c0", "c1", "c2", "c3", "clock_skew", "cts", "k0", "k1", "login_rtt", "m_id", "nna_msg_rtt",
	"nna_no_off_count", "nna_offline_ratio", "nna_push_rtt", "no_nna_con_count", "off_msg_rtt", "on_msg_rtt", "stat_name", "sts",
	"suspect_conn", "lists", "self", "qr", "web", "w:b", "recipient", "w:stats", "forbidden", "aurora.m4r", "bamboo.m4r", "chord.m4r",
	"circles.m4r", "complete.m4r", "hello.m4r", "input.m4r", "keys.m4r", "note.m4r", "popcorn.m4r", "pulse.m4r", "synth.m4r", "filehash",
	"max_list_recipients", "en-AU", "en-GB", "es-MX", "pt-PT", "zh-Hans", "zh-Hant", "relayelection", "relaylatency", "interruption",
	"Apex.m4r", "Beacon.m4r", "Bulletin.m4r", "By The Seaside.m4r", "Chimes.m4r", "Circuit.m4r", "Constellation.m4r", "Cosmic.m4r",
	"Crystals.m4r", "Hillside.m4r", "Illuminate.m4r", "Night Owl.m4r", "Opening.m4r", "Playtime.m4r", "Presto.m4r", "Radar.m4r",
	"Radiate.m4r", "Ripples.m4r", "Sencha.m4r", "Signal.m4r", "Silk.m4r", "Slow Rise.m4r", "Stargaze.m4r", "Summit.m4r", "Twinkle.m4r",
	"Uplift.m4r", "Waves.m4r", "voip", "eligible", "upgrade", "planned", "current", "future", "disable", "expire", "start", "stop",
	"accuracy", "speed", "bearing", "recording", "encrypt", "key", "identity", "w:gp2", "admin", "locked", "unlocked", "new", "battery",
	"archive", "adm", "plaintext_size", "compressed_size", "delivered", "msg", "pkmsg", "everyone", "v", "transport", "call-id"
};

static map<string, int> tokenMap1, tokenMap2;

void WAConnection::globalInit()
{
	for (int i = 0; i < _countof(dictionary); i++)
		if (*dictionary[i] != 0)
			tokenMap1[dictionary[i]] = i;			

	for (int i = 0; i < _countof(extended_dict); i++)
		tokenMap2[extended_dict[i]] = i;
}

int WAConnection::tokenLookup(const std::string &str)
{
	std::map<string, int>::iterator it = tokenMap1.find(str);
	if (it != tokenMap1.end())
		return it->second;

	it = tokenMap2.find(str);
	if (it != tokenMap2.end())
		return it->second + 0x100;

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

void WAConnection::logData(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	char tmp[4000];
	vsprintf_s(tmp, format, args);
	rawConn->log(">> ", tmp);
}

/////////////////////////////////////////////////////////////////////////////////////////

WAConnection::WAConnection(const std::string &user, const std::string &resource, IMutex *mutex, IMutex *write_mutex, ISocketConnection *conn, WAListener *pEventHandler, WAGroupListener *pGroupEventHandler) :
	in(this, conn),
	out(this, conn, write_mutex)
{
	m_pMutex = mutex;
	m_pEventHandler = pEventHandler;
	m_pGroupEventHandler = pGroupEventHandler;

	rawConn = conn;

	this->retry = true;

	this->user = user;
	this->resource = resource;
	this->domain = "s.whatsapp.net";
	this->jid = user + "@" + domain;

	this->supports_receipt_acks = false;
	this->iqid = 0;
	this->lastTreeRead = 0;
	this->expire_date = 0L;
	this->account_kind = -1;
}

WAConnection::~WAConnection()
{
	std::map<string, IqResultHandler*>::iterator it;
	for (it = this->pending_server_requests.begin(); it != this->pending_server_requests.end(); it++)
		delete it->second;
}

std::string WAConnection::gidToGjid(const std::string &gid)
{
	return gid + "@g.us";
}

std::string WAConnection::makeId(const std::string &prefix)
{
	return prefix + Utilities::itoa(++this->iqid, 16);
}

ProtocolTreeNode* WAConnection::getReceiptAck(const std::string &to, const std::string &id, const std::string &receiptType) throw(WAException)
{
	ProtocolTreeNode *ackNode = new ProtocolTreeNode("ack")
		<< XATTR("xmlns", "urn:xmpp:receipts") << XATTR("type", receiptType);

	return new ProtocolTreeNode("message", ackNode) << XATTR("to", to) << XATTR("type", "chat") << XATTR("id", id);
}

bool WAConnection::supportsReceiptAcks()
{
	return supports_receipt_acks;
}

std::string WAConnection::removeResourceFromJid(const std::string &jid)
{
	size_t slashidx = jid.find('/');
	if (slashidx == std::string::npos)
		return jid;

	return jid.substr(0, slashidx + 1);
}

void WAConnection::setLogin(WALogin* login)
{
	if (login->m_tExpireDate != 0L)
		this->expire_date = login->m_tExpireDate;

	if (login->m_iAccountKind != -1)
		this->account_kind = login->m_iAccountKind;
}

bool WAConnection::read() throw(WAException)
{
	ProtocolTreeNode *node;
	try {
		node = in.nextTree();
		this->lastTreeRead = time(NULL);
	}
	catch (exception& ex) {
		throw WAException(ex.what(), WAException::CORRUPT_STREAM_EX, 0);
	}

	if (node == NULL)
		return false;

	string tmp = node->toString();
	rawConn->log("XML received\n", tmp.c_str());

	if (ProtocolTreeNode::tagEquals(node, "iq"))
		parseIq(node);
	else if (ProtocolTreeNode::tagEquals(node, "presence"))
		parsePresense(node);
	else if (ProtocolTreeNode::tagEquals(node, "message"))
		parseMessage(node);
	else if (ProtocolTreeNode::tagEquals(node, "notification"))
		parseNotification(node);
	else if (ProtocolTreeNode::tagEquals(node, "ack"))
		parseAck(node);
	else if (ProtocolTreeNode::tagEquals(node, "receipt"))
		parseReceipt(node);
	else if (ProtocolTreeNode::tagEquals(node, "chatstate"))
		parseChatStates(node);

	delete node;
	return true;
}

void WAConnection::readGroupList(ProtocolTreeNode *node, std::vector<std::string>& groups) throw (WAException)
{
	std::vector<ProtocolTreeNode*> nodes(node->getAllChildren("group"));
	for (size_t i = 0; i < nodes.size(); i++) {
		ProtocolTreeNode *groupNode = nodes[i];
		const string &gid = groupNode->getAttributeValue("id");
		string gjid = gidToGjid(gid);
		const string &owner = groupNode->getAttributeValue("owner");
		const string &subject = groupNode->getAttributeValue("subject");
		const string &subject_t = groupNode->getAttributeValue("s_t");
		const string &subject_owner = groupNode->getAttributeValue("s_o");
		const string &creation = groupNode->getAttributeValue("creation");
		if (m_pGroupEventHandler != NULL)
			m_pGroupEventHandler->onGroupInfo(gjid, owner, subject, subject_owner, atoi(subject_t.c_str()), atoi(creation.c_str()));
		groups.push_back(gjid);
	}
}

std::map<string, string> WAConnection::parseCategories(ProtocolTreeNode *dirtyNode) throw (WAException)
{
	std::map<string, string> categories;
	if (dirtyNode->children != NULL) {
		for (size_t i = 0; i < dirtyNode->children->size(); i++) {
			ProtocolTreeNode *childNode = (*dirtyNode->children)[i];
			if (ProtocolTreeNode::tagEquals(childNode, "category")) {
				const string &categoryName = childNode->getAttributeValue("name");
				const string &timestamp = childNode->getAttributeValue("timestamp");
				categories[categoryName] = timestamp;
			}
		}
	}

	return categories;
}

void WAConnection::parseAck(ProtocolTreeNode *node) throw(WAException)
{
	const string &from = node->getAttributeValue("from");
	const string &cls = node->getAttributeValue("class");
	const string &id = node->getAttributeValue("id");
	const string &ts = node->getAttributeValue("t");

	if (cls == "message" && m_pEventHandler != NULL) {
		FMessage msg(from, true, id);
		msg.status = FMessage::STATUS_RECEIVED_BY_SERVER;
		m_pEventHandler->onMessageStatusUpdate(msg);
	}
}

void WAConnection::parseChatStates(ProtocolTreeNode *node) throw (WAException)
{
	const string &from = node->getAttributeValue("from");

	std::vector<ProtocolTreeNode*> messageChildren(node->getAllChildren());
	for (size_t i = 0; i < messageChildren.size(); i++) {
		ProtocolTreeNode *childNode = messageChildren[i];
		if (ProtocolTreeNode::tagEquals(childNode, "composing")) {
			if (m_pEventHandler != NULL)
				m_pEventHandler->onIsTyping(from, true);
		}
		else if (ProtocolTreeNode::tagEquals(childNode, "paused")) {
			if (m_pEventHandler != NULL)
				m_pEventHandler->onIsTyping(from, false);
		}
	}
}

void WAConnection::parseIq(ProtocolTreeNode *node) throw(WAException)
{
	const string &type = node->getAttributeValue("type");
	if (type.empty())
		throw WAException("missing 'type' attribute in iq stanza", WAException::CORRUPT_STREAM_EX, 0);

	const string &id = node->getAttributeValue("id");
	const string &from = node->getAttributeValue("from");

	if (type == "result") {
		if (id.empty())
			throw WAException("missing 'id' attribute in iq stanza", WAException::CORRUPT_STREAM_EX, 0);

		std::map<string, IqResultHandler*>::iterator it = this->pending_server_requests.find(id);
		if (it != this->pending_server_requests.end()) {
			it->second->parse(node, from);
			delete it->second;
			this->pending_server_requests.erase(id);
		}
		else if (id.compare(0, this->user.size(), this->user) == 0) {
			ProtocolTreeNode *accountNode = node->getChild(0);
			ProtocolTreeNode::require(accountNode, "account");
			const string &kind = accountNode->getAttributeValue("kind");
			if (kind == "paid")
				this->account_kind = 1;
			else if (kind == "free")
				this->account_kind = 0;
			else
				this->account_kind = -1;

			const string &expiration = accountNode->getAttributeValue("expiration");
			if (expiration.empty())
				throw WAException("no expiration");

			this->expire_date = atol(expiration.c_str());
			if (this->expire_date == 0)
				throw WAException("invalid expire date: " + expiration);
			if (m_pEventHandler != NULL)
				m_pEventHandler->onAccountChange(this->account_kind, this->expire_date);
		}
		else {
			ProtocolTreeNode *childNode = node->getChild(0);
			if (ProtocolTreeNode::tagEquals(childNode, "leave")) {
				std::vector<ProtocolTreeNode*> nodes(childNode->getAllChildren("group"));
				for (size_t i = 0; i < nodes.size(); i++) {
					ProtocolTreeNode *groupNode = nodes[i];
					const string &gjid = groupNode->getAttributeValue("id");
					if (m_pGroupEventHandler != NULL)
						m_pGroupEventHandler->onLeaveGroup(gjid);
				}
			}
		}
	}
	else if (type == "error") {
		std::map<string, IqResultHandler*>::iterator it = this->pending_server_requests.find(id);
		if (it != this->pending_server_requests.end()) {
			it->second->error(node);
			delete it->second;
			this->pending_server_requests.erase(id);
		}
	}
	else if (type == "get") {
		ProtocolTreeNode *childNode = node->getChild(0);
		if (ProtocolTreeNode::tagEquals(childNode, "ping")) {
			if (m_pEventHandler != NULL)
				m_pEventHandler->onPing(id);
		}
		else if ((ProtocolTreeNode::tagEquals(childNode, "query") && !from.empty()) ? false : (ProtocolTreeNode::tagEquals(childNode, "relay")) && !from.empty()) {
			const string &pin = childNode->getAttributeValue("pin");
			if (!pin.empty() && m_pEventHandler != NULL) {
				int timeoutSeconds = atoi(childNode->getAttributeValue("timeout").c_str());
				m_pEventHandler->onRelayRequest(pin, timeoutSeconds, id);
			}
		}
	}
	else if (type == "set") {
		ProtocolTreeNode *childNode = node->getChild(0);
		if (ProtocolTreeNode::tagEquals(childNode, "query")) {
			const string &xmlns = childNode->getAttributeValue("xmlns");
			if (xmlns == "jabber:iq:roster") {
				std::vector<ProtocolTreeNode*> itemNodes(childNode->getAllChildren("item"));
				for (size_t i = 0; i < itemNodes.size(); i++) {
					ProtocolTreeNode *itemNode = itemNodes[i];
					const string &jid = itemNode->getAttributeValue("jid");
					const string &subscription = itemNode->getAttributeValue("subscription");
					// ask = itemNode->getAttributeValue("ask");
				}
			}
		}
	}
	else throw WAException("unknown iq type attribute: " + type, WAException::CORRUPT_STREAM_EX, 0);
}

void WAConnection::parseMessage(ProtocolTreeNode *messageNode) throw (WAException)
{
	const string &id = messageNode->getAttributeValue("id");
	const string &attribute_t = messageNode->getAttributeValue("t");
	const string &from = messageNode->getAttributeValue("from");

	const string &typeAttribute = messageNode->getAttributeValue("type");
	if (typeAttribute.empty())
		return;

	const string &participant = messageNode->getAttributeValue("participant");
	if (typeAttribute == "error") {
		int errorCode = 0;
		std::vector<ProtocolTreeNode*> errorNodes(messageNode->getAllChildren("error"));
		for (size_t i = 0; i < errorNodes.size(); i++) {
			ProtocolTreeNode *errorNode = errorNodes[i];
			errorCode = atoi(errorNode->getAttributeValue("code").c_str());
		}

		FMessage message(from, true, id);
		message.status = FMessage::STATUS_SERVER_BOUNCE;

		if (m_pEventHandler != NULL)
			m_pEventHandler->onMessageError(message, errorCode);
		return;
	}
	
	if (from.empty() || id.empty())
		return;
	FMessage fmessage(from, false, id);

	if (typeAttribute == "text") {
		ProtocolTreeNode *body = messageNode->getChild("body");
		if (body == NULL || body->data == NULL || body->data->empty())
			return;

		fmessage.wants_receipt = false;
		fmessage.timestamp = atoi(attribute_t.c_str());
		fmessage.remote_resource = participant;
		fmessage.notifyname = messageNode->getAttributeValue("notify");
		fmessage.data = body->getDataAsString();
		fmessage.status = FMessage::STATUS_UNSENT;
		if (fmessage.timestamp == 0) {
			fmessage.timestamp = time(NULL);
			fmessage.offline = false;
		}
	}
	else if (typeAttribute == "media") {
		ProtocolTreeNode *media = messageNode->getChild("media");
		if (media == NULL)
			return;

		fmessage.wants_receipt = false;
		fmessage.timestamp = atoi(attribute_t.c_str());
		fmessage.notifyname = messageNode->getAttributeValue("notify");
		fmessage.remote_resource = messageNode->getAttributeValue("participant");
		fmessage.media_wa_type = FMessage::getMessage_WA_Type(media->getAttributeValue("type"));
		fmessage.media_url = media->getAttributeValue("url");
		fmessage.media_name = media->getAttributeValue("file");
		fmessage.media_size = Utilities::parseLongLong(media->getAttributeValue("size"));
		fmessage.media_duration_seconds = atoi(media->getAttributeValue("seconds").c_str());

		if (fmessage.media_wa_type == FMessage::WA_TYPE_LOCATION) {
			const string &name = media->getAttributeValue("name");
			const string &latitudeString = media->getAttributeValue("latitude");
			const string &longitudeString = media->getAttributeValue("longitude");
			if (latitudeString.empty() || longitudeString.empty())
				throw WAException("location message missing lat or long attribute", WAException::CORRUPT_STREAM_EX, 0);

			fmessage.latitude = atof(latitudeString.c_str());
			fmessage.longitude = atof(longitudeString.c_str());
			if (!name.empty())
				fmessage.data = name;
		}
		else if (fmessage.media_wa_type == FMessage::WA_TYPE_CONTACT) {
			ProtocolTreeNode *contactChildNode = media->getChild(0);
			if (contactChildNode != NULL) {
				fmessage.media_name = contactChildNode->getAttributeValue("name");
				fmessage.data = contactChildNode->getDataAsString();
				size_t off = fmessage.data.find("TEL;");
				if (off != string::npos) {
					if ((off = fmessage.data.find(":", off)) != string::npos) {
						std::string number;
						for (off++;; off++) {
							char c = fmessage.data[off];
							if (isdigit(c))
								number += c;
							else if (c == '+' || c == ' ')
								continue;
							else
								break;
						}
						if (!number.empty())
							fmessage.media_url = number;
					}
				}
			}
		}
		else {
			const string &encoding = media->getAttributeValue("encoding");
			if (encoding.empty() || encoding == "text")
				fmessage.data = media->getDataAsString();
			else
				fmessage.data = media->getAttributeValue("caption");
		}
	}
	else return;

	if (fmessage.remote_resource.empty()) {
		if (m_pEventHandler != NULL)
			m_pEventHandler->onMessageForMe(fmessage);
	}
	else if (m_pGroupEventHandler != NULL)
		m_pGroupEventHandler->onGroupMessage(fmessage);
}

void WAConnection::parseNotification(ProtocolTreeNode *node) throw(WAException)
{
	const string &id = node->getAttributeValue("id");
	const string &from = node->getAttributeValue("from");
	const string &type = node->getAttributeValue("type");
	if (type.empty() || from.empty() || m_pEventHandler == NULL)
		return;

	const string &participant = node->getAttributeValue("participant");
	int ts = atoi(node->getAttributeValue("t").c_str());

	if (type == "contacts") {
		std::vector<ProtocolTreeNode*> children(node->getAllChildren());
		for (size_t i = 0; i < children.size(); i++) {
			ProtocolTreeNode *child = children[i];

			const string &jid = node->getAttributeValue("jid");
			if (jid.empty()) continue;

			bool bAdded;
			if (ProtocolTreeNode::tagEquals(child, "add"))
				bAdded = true;
			else if (ProtocolTreeNode::tagEquals(child, "delete"))
				bAdded = false;
			else
				continue;

			m_pEventHandler->onContactChanged(jid, bAdded);
		}
	}
	else if (type == "picture") {
		std::vector<ProtocolTreeNode*> children2(node->getAllChildren());
		for (unsigned j = 0; j < children2.size(); j++) {
			ProtocolTreeNode *child2 = children2[j];
			if (ProtocolTreeNode::tagEquals(child2, "set")) {
				const string &id = child2->getAttributeValue("id");
				const string &jid = child2->getAttributeValue("jid");
				if (!id.empty())
					m_pEventHandler->onPictureChanged(jid, id, true);
			}
			else if (ProtocolTreeNode::tagEquals(child2, "delete")) {
				const string &jid = child2->getAttributeValue("jid");
				m_pEventHandler->onPictureChanged(jid, id, false);
			}
		}
	}
	// group chats 
	else if (type == "participant") {
		ProtocolTreeNode *subNode = node->getChild("add");
		if (subNode != NULL) {
			const string &jid = subNode->getAttributeValue("jid");
			if (m_pGroupEventHandler)
				m_pGroupEventHandler->onGroupAddUser(from, jid, ts);
		}
		else if ((subNode = node->getChild("remove")) != NULL) {
			const string &jid = subNode->getAttributeValue("jid");
			if (m_pGroupEventHandler)
				m_pGroupEventHandler->onGroupRemoveUser(from, jid, ts);
		}
		else return;

	}
	else if (type == "subject") {
		ProtocolTreeNode *bodyNode = node->getChild("body");
		if (bodyNode != NULL && m_pGroupEventHandler != NULL)
			m_pGroupEventHandler->onGroupNewSubject(from, participant, bodyNode->getDataAsString(), ts);
	}

	ProtocolTreeNode sendNode("ack");
	sendNode << XATTR("to", from) << XATTR("id", id) << XATTR("type", type) << XATTR("class", "notification");
	const string &to = node->getAttributeValue("to");
	if (!to.empty())
		sendNode << XATTR("from", to);
	if (!participant.empty())
		sendNode << XATTR("participant", participant);
	out.write(sendNode);
}

void WAConnection::parsePresense(ProtocolTreeNode *node) throw(WAException)
{
	const string &xmlns = node->getAttributeValue("xmlns");
	const string &from = node->getAttributeValue("from");
	if (from.empty())
		return;

	int ts = atoi(node->getAttributeValue("t").c_str());
	if (xmlns == "w" && !from.empty()) {
		const string &add = node->getAttributeValue("add");
		const string &remove = node->getAttributeValue("remove");
		const string &status = node->getAttributeValue("status");
		if (!add.empty()) {
			if (m_pGroupEventHandler != NULL)
				m_pGroupEventHandler->onGroupAddUser(from, add, ts);
		}
		else if (!remove.empty()) {
			if (m_pGroupEventHandler != NULL)
				m_pGroupEventHandler->onGroupRemoveUser(from, remove, ts);
		}
		else if (status == "dirty") {
			std::map<string, string> categories = parseCategories(node);
			if (m_pEventHandler != NULL)
				m_pEventHandler->onDirty(categories);
		}
		return;
	}
	
	if (m_pEventHandler != NULL) {
		const string &type = node->getAttributeValue("type");
		if (type == "unavailable")
			m_pEventHandler->onAvailable(from, false);
		else if (type == "available" || type == "")
			m_pEventHandler->onAvailable(from, true);
	}
}

void WAConnection::parseReceipt(ProtocolTreeNode *node) throw(WAException)
{
	const string &from = node->getAttributeValue("from");
	const string &id = node->getAttributeValue("id");
	const string &ts = node->getAttributeValue("t");

	if (m_pEventHandler != NULL) {
		FMessage msg(from, false, id);
		msg.status = FMessage::STATUS_RECEIVED_BY_TARGET;
		m_pEventHandler->onMessageStatusUpdate(msg);
	}

	out.write(ProtocolTreeNode("ack")
		<< XATTR("to", from) << XATTR("id", id) << XATTR("type", "read") << XATTRI("t", time(0)));
}

std::vector<ProtocolTreeNode*>* WAConnection::processGroupSettings(const std::vector<GroupSetting>& groups)
{
	std::vector<ProtocolTreeNode*>* result = new std::vector<ProtocolTreeNode*>(groups.size());
	if (!groups.empty()) {
		time_t now = time(NULL);
		for (size_t i = 0; i < groups.size(); i++) {
			(*result)[i] = new ProtocolTreeNode("item")
				<< XATTR("jid", groups[i].jid) << XATTR("notify", (groups[i].enabled ? "1" : "0"))
				<< XATTRI("mute", (groups[i].muteExpiry > now) ? groups[i].muteExpiry - now : 0);
		}
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Send* functions

void WAConnection::sendActive() throw(WAException)
{
	out.write(ProtocolTreeNode("presence") << XATTR("type", "active"));
}

void WAConnection::sendAvailableForChat() throw(WAException)
{
	out.write(ProtocolTreeNode("presence") << XATTR("name", this->nick));
}

void WAConnection::sendClientConfig(const std::string &sound, const std::string &pushID, bool preview, const std::string &platform) throw(WAException)
{
	ProtocolTreeNode *configNode = new ProtocolTreeNode("config")
		<< XATTR("xmlns", "urn:xmpp:whatsapp:push") << XATTR("sound", sound) << XATTR("id", pushID) << XATTR("preview", preview ? "1" : "0") << XATTR("platform", platform);

	std::string id = makeId("config_");
	this->pending_server_requests[id] = new IqSendClientConfigHandler(this);

	out.write(ProtocolTreeNode("iq", configNode)
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", this->domain));
}

void WAConnection::sendClientConfig(const std::string &pushID, bool preview, const std::string &platform, bool defaultSettings, bool groupSettings, const std::vector<GroupSetting>& groups) throw(WAException)
{
	ProtocolTreeNode *configNode = new ProtocolTreeNode("config", NULL, this->processGroupSettings(groups))
		<< XATTR("xmlns", "urn:xmpp:whatsapp:push") << XATTR("id", pushID) << XATTR("lg", "en") << XATTR("lc", "US") << XATTR("clear", "0")
		<< XATTR("preview", preview ? "1" : "0") << XATTR("platform", platform)
		<< XATTR("default", defaultSettings ? "1" : "0") << XATTR("groups", groupSettings ? "1" : "0");

	std::string id = makeId("config_");
	out.write(ProtocolTreeNode("iq", configNode) << XATTR("id", id) << XATTR("type", "set") << XATTR("to", this->domain));
}

void WAConnection::sendClose() throw(WAException)
{
	out.write(ProtocolTreeNode("presence") << XATTR("type", "unavailable"));
	out.streamEnd();
}

void WAConnection::sendComposing(const std::string &to) throw(WAException)
{
	out.write(ProtocolTreeNode("chatstate", new ProtocolTreeNode("composing")) << XATTR("to", to));
}

void WAConnection::sendDeleteAccount() throw (WAException)
{
	std::string id = makeId("del_acct_");
	this->pending_server_requests[id] = new IqResultSendDeleteAccount(this);

	ProtocolTreeNode *node1 = new ProtocolTreeNode("remove") << XATTR("xmlns", "urn:xmpp:whatsapp:account");
	out.write(ProtocolTreeNode("iq", node1)
		<< XATTR("id", id) << XATTR("type", "get") << XATTR("to", "s.whatsapp.net"));
}

void WAConnection::sendGetGroups() throw (WAException)
{
	m_pMutex->lock();
	std::string id = makeId("iq_");
	this->pending_server_requests[id] = new IqResultGetGroupsHandler(this, "participating");

	out.write(ProtocolTreeNode("iq", new ProtocolTreeNode("list") << XATTR("type", "participating"))
		<< XATTR("xmlns", "w:g") << XATTR("id", id) << XATTR("type", "get") << XATTR("to", "g.us"));
	m_pMutex->unlock();
}

void WAConnection::sendGetPicture(const char *jid, const char *type) throw (WAException)
{
	std::string id = makeId("iq_");
	this->pending_server_requests[id] = new IqResultGetPhotoHandler(this, jid);

	ProtocolTreeNode *listNode = new ProtocolTreeNode("picture") << XATTR("type", type);

	out.write(ProtocolTreeNode("iq", listNode)
		<< XATTR("id", id) << XATTR("to", jid) << XATTR("xmlns", "w:profile:picture") << XATTR("type", "get"));
}

void WAConnection::sendGetPrivacyList() throw (WAException)
{
	std::string id = makeId("privacylist_");
	this->pending_server_requests[id] = new IqResultPrivayListHandler(this);

	ProtocolTreeNode *listNode = new ProtocolTreeNode("list") << XATTR("name", "default");
	ProtocolTreeNode *queryNode = new ProtocolTreeNode("query", listNode) << XATTR("xmlns", "jabber:iq:privacy");
	out.write(ProtocolTreeNode("iq", queryNode) << XATTR("id", id) << XATTR("type", "get"));
}

void WAConnection::sendGetServerProperties() throw (WAException)
{
	std::string id = makeId("get_server_properties_");
	this->pending_server_requests[id] = new IqResultServerPropertiesHandler(this);

	ProtocolTreeNode *listNode = new ProtocolTreeNode("list") << XATTR("type", "props");

	out.write(ProtocolTreeNode("iq", listNode) << XATTR("xmlns", "w:g2")
		<< XATTR("id", id) << XATTR("type", "get") << XATTR("to", "g.us"));
}

void WAConnection::sendInactive() throw(WAException)
{
	out.write(ProtocolTreeNode("presence") << XATTR("type", "inactive"));
}

/////////////////////////////////////////////////////////////////////////////////////////

ProtocolTreeNode* WAConnection::getMessageNode(FMessage* message, ProtocolTreeNode *child)
{
	std::vector<ProtocolTreeNode*>* messageChildren = new std::vector<ProtocolTreeNode*>();
	messageChildren->push_back(new ProtocolTreeNode("x", new ProtocolTreeNode("server")) << XATTR("xmlns", "jabber:x:event"));
	messageChildren->push_back(child);

	return new ProtocolTreeNode("message", NULL, messageChildren) <<
		XATTR("to", message->key.remote_jid) << XATTR("type", "text") << XATTR("id", message->key.id) << XATTRI("t", message->timestamp);
}

void WAConnection::sendMessage(FMessage* message) throw(WAException)
{
	if (message->media_wa_type != 0)
		sendMessageWithMedia(message);
	else
		sendMessageWithBody(message);
}

void WAConnection::sendMessageWithMedia(FMessage* message)  throw (WAException)
{
	logData("Send message with media %s %d", message->media_name.c_str(), message->media_size);
	logData("media-url:%s", message->media_url.c_str());
	if (message->media_wa_type == FMessage::WA_TYPE_SYSTEM)
		throw new WAException("Cannot send system message over the network");

	ProtocolTreeNode *mediaNode;
	if (message->media_wa_type == FMessage::WA_TYPE_CONTACT && !message->media_name.empty()) {
		ProtocolTreeNode *vcardNode = new ProtocolTreeNode("vcard", new std::vector<unsigned char>(message->data.begin(), message->data.end()))
			<< XATTR("name", message->media_name);
		mediaNode = new ProtocolTreeNode("media", vcardNode);
	}
	else {
		mediaNode = new ProtocolTreeNode("media", new std::vector<unsigned char>(message->data.begin(), message->data.end()), NULL)
			<< XATTR("encoding", "text");
	}

	mediaNode << XATTR("xmlns", "urn:xmpp:whatsapp:mms") << XATTR("type", FMessage::getMessage_WA_Type_StrValue(message->media_wa_type));

	if (message->media_wa_type == FMessage::WA_TYPE_LOCATION)
		mediaNode << XATTR("latitude", Utilities::doubleToStr(message->latitude)) << XATTR("longitude", Utilities::doubleToStr(message->longitude));
	else {
		mediaNode << XATTR("file", message->media_name) << XATTRI("size", message->media_size) << XATTR("url", message->media_url);
		if (message->media_wa_type == FMessage::WA_TYPE_CONTACT || message->media_name.empty() || message->media_url.empty() || message->media_size <= 0)
			mediaNode << XATTRI("seconds", message->media_duration_seconds);
	}

	ProtocolTreeNode *n = WAConnection::getMessageNode(message, mediaNode);
	out.write(*n);
	delete n;
}

void WAConnection::sendMessageWithBody(FMessage* message) throw (WAException)
{
	ProtocolTreeNode *bodyNode = new ProtocolTreeNode("body", new std::vector<unsigned char>(message->data.begin(), message->data.end()));
	ProtocolTreeNode *n = WAConnection::getMessageNode(message, bodyNode);
	out.write(*n);
	delete n;
}

void WAConnection::sendMessageReceived(const FMessage &message) throw(WAException)
{
	out.write(ProtocolTreeNode("receipt") << XATTR("type", "read")
		<< XATTR("to", message.key.remote_jid) << XATTR("id", message.key.id) << XATTRI("t", (int)time(0)));
}

/////////////////////////////////////////////////////////////////////////////////////////

void WAConnection::sendPaused(const std::string &to) throw(WAException)
{
	out.write(ProtocolTreeNode("chatstate", new ProtocolTreeNode("paused")) << XATTR("to", to));
}

void WAConnection::sendPing() throw(WAException)
{
	std::string id = makeId("ping_");
	this->pending_server_requests[id] = new IqResultPingHandler(this);

	ProtocolTreeNode *pingNode = new ProtocolTreeNode("ping");
	out.write(ProtocolTreeNode("iq", pingNode) 
		<< XATTR("id", id) << XATTR("xmlns", "w:p") << XATTR("type", "get") << XATTR("to", "s.whatsapp.net"));
}

void WAConnection::sendPong(const std::string &id) throw(WAException)
{
	out.write(ProtocolTreeNode("iq")
		<< XATTR("type", "result") << XATTR("to", this->domain) << XATTR("id", id));
}

void WAConnection::sendPresenceSubscriptionRequest(const std::string &to) throw(WAException)
{
	out.write(ProtocolTreeNode("presence") << XATTR("type", "subscribe") << XATTR("to", to));
}

void WAConnection::sendQueryLastOnline(const std::string &jid) throw (WAException)
{
	if (jid == "Server@s.whatsapp.net") // to avoid error 405
		return;

	std::string id = makeId("iq_");
	this->pending_server_requests[id] = new IqResultQueryLastOnlineHandler(this);

	out.write(ProtocolTreeNode("iq", new ProtocolTreeNode("query"))
		<< XATTR("id", id) << XATTR("type", "get") << XATTR("to", jid) << XATTR("xmlns", "jabber:iq:last"));
}

void WAConnection::sendSetPicture(const char *jid, std::vector<unsigned char>* data, std::vector<unsigned char>* preview) throw (WAException)
{
	std::string id = this->makeId("set_photo_");
	this->pending_server_requests[id] = new IqResultSetPhotoHandler(this, jid);

	std::vector<ProtocolTreeNode*>* messageChildren = new std::vector<ProtocolTreeNode*>();
	if (preview)
		messageChildren->push_back(new ProtocolTreeNode("picture", preview, NULL) << XATTR("type", "preview"));
	if (data)
		messageChildren->push_back(new ProtocolTreeNode("picture", data, NULL) << XATTR("type", "image"));
	out.write(ProtocolTreeNode("iq", NULL, messageChildren)
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", jid) << XATTR("xmlns", "w:profile:picture"));
}

void WAConnection::sendStatusUpdate(std::string& status) throw (WAException)
{
	std::string id = this->makeId(Utilities::intToStr((int)time(NULL)));
	FMessage message("s.us", true, id);
	ProtocolTreeNode *body = new ProtocolTreeNode("body", new std::vector<unsigned char>(status.begin(), status.end()), NULL);
	ProtocolTreeNode *n = getMessageNode(&message, body);
	out.write(*n);
	delete n;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Group chats

void WAConnection::sendGetGroupInfo(const std::string &gjid) throw (WAException)
{
	std::string id = makeId("iq_");
	this->pending_server_requests[id] = new IqResultGetGroupInfoHandler(this);

	ProtocolTreeNode *queryNode = new ProtocolTreeNode("query") << XATTR("request", "interactive");
	out.write(ProtocolTreeNode("iq", queryNode) << XATTR("xmlns", "w:g2")
		<< XATTR("id", id) << XATTR("type", "get") << XATTR("to", gjid));
}

void WAConnection::sendGetParticipants(const std::string &gjid) throw (WAException)
{
	std::string id = makeId("iq_");
	this->pending_server_requests[id] = new IqResultGetGroupParticipantsHandler(this);

	ProtocolTreeNode *listNode = new ProtocolTreeNode("list");
	out.write(ProtocolTreeNode("iq", listNode) << XATTR("xmlns", "w:g")
		<< XATTR("id", id) << XATTR("type", "get") << XATTR("to", gjid));
}

void WAConnection::readAttributeList(ProtocolTreeNode *node, std::vector<std::string>& vector, const std::string &tag, const std::string &attribute) throw (WAException)
{
	std::vector<ProtocolTreeNode*> nodes(node->getAllChildren(tag));
	for (size_t i = 0; i < nodes.size(); i++) {
		ProtocolTreeNode *tagNode = nodes[i];
		vector.push_back(tagNode->getAttributeValue(attribute));
	}
}

void WAConnection::sendCreateGroupChat(const std::string &subject) throw (WAException)
{
	logData("sending create group: %s", subject.c_str());
	std::string id = makeId("create_group_");
	this->pending_server_requests[id] = new IqResultCreateGroupChatHandler(this, subject);

	ProtocolTreeNode *groupNode = new ProtocolTreeNode("group") << XATTR("action", "create") << XATTR("subject", subject);

	out.write(ProtocolTreeNode("iq", groupNode) << XATTR("xmlns", "w:g")
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", "g.us"));
}

void WAConnection::sendClearDirty(const std::string &category) throw (WAException)
{
	std::string id = makeId("clean_dirty_");
	this->pending_server_requests[id] = new IqResultClearDirtyHandler(this);

	ProtocolTreeNode *categoryNode = new ProtocolTreeNode("category") << XATTR("name", category);
	ProtocolTreeNode *cleanNode = new ProtocolTreeNode("clean", categoryNode) << XATTR("xmlns", "urn:xmpp:whatsapp:dirty");
	out.write(ProtocolTreeNode("iq", cleanNode)
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", "s.whatsapp.net"));
}

void WAConnection::sendJoinLeaveGroup(const char *gjid, bool bJoin) throw (WAException)
{
	std::string id = makeId("iq_");

	ProtocolTreeNode *groupNode = new ProtocolTreeNode("group") << XATTR("id", gjid);
	ProtocolTreeNode *leaveNode = new ProtocolTreeNode((bJoin) ? "join" : "leave", groupNode);
	out.write(ProtocolTreeNode("iq", leaveNode) << XATTR("xmlns", "w:g2")
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", "g.us"));
}

void WAConnection::sendAddParticipants(const std::string &gjid, const std::vector<std::string> &participants) throw (WAException)
{
	sendVerbParticipants(gjid, participants, "add");
}

void WAConnection::sendRemoveParticipants(const std::string &gjid, const std::vector<std::string> &participants) throw (WAException)
{
	sendVerbParticipants(gjid, participants, "remove");
}

void WAConnection::sendVerbParticipants(const std::string &gjid, const std::vector<std::string> &participants, const std::string &inner_tag) throw (WAException)
{
	std::string id = makeId("iq_");

	size_t size = participants.size();
	std::vector<ProtocolTreeNode*>* children = new std::vector<ProtocolTreeNode*>(size);
	for (size_t i = 0; i < size; i++)
		(*children)[i] = new ProtocolTreeNode("participant") << XATTR("jid", participants[i]);

	ProtocolTreeNode *innerNode = new ProtocolTreeNode(inner_tag, NULL, children);

	out.write(ProtocolTreeNode("iq", innerNode) << XATTR("xmlns", "w:g")
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", gjid));
}

void WAConnection::sendSetNewSubject(const std::string &gjid, const std::string &subject) throw (WAException)
{
	std::string id = this->makeId("iq_");

	std::vector<unsigned char> *data = new std::vector<unsigned char>(subject.begin(), subject.end());
	ProtocolTreeNode *subjectNode = new ProtocolTreeNode("subject", data);

	out.write(ProtocolTreeNode("iq", subjectNode) << XATTR("xmlns", "w:g2")
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", gjid));
}
