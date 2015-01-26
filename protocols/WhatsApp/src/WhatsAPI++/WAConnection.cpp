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
	rawConn->log(tmp);
}

/////////////////////////////////////////////////////////////////////////////////////////

WAConnection::WAConnection(const std::string &user, const std::string &resource, IMutex *mutex, WAListener *event_handler, WAGroupListener *group_event_handler)
{
	this->mutex = mutex;
	this->event_handler = event_handler;
	this->group_event_handler = group_event_handler;

	this->in = NULL;
	this->out = NULL;

	this->msg_id = 0;
	this->retry = true;

	this->user = user;
	this->resource = resource;
	this->domain = "s.whatsapp.net";
	this->jid = user + "@" + domain;

	this->supports_receipt_acks = false;
	this->iqid = 0;
	this->verbose = true;
	this->lastTreeRead = 0;
	this->expire_date = 0L;
	this->account_kind = -1;
}

WAConnection::~WAConnection()
{
	delete this->in;
	delete this->out;
	std::map<string, IqResultHandler*>::iterator it;
	for (it = this->pending_server_requests.begin(); it != this->pending_server_requests.end(); it++)
		delete it->second;
}

void WAConnection::init(IMutex *mutex, ISocketConnection *conn)
{
	rawConn = conn;
	in = new BinTreeNodeReader(this, conn);
	out = new BinTreeNodeWriter(this, conn, mutex);
}

void WAConnection::setLogin(WALogin* login)
{
	if (login->expire_date != 0L)
		this->expire_date = login->expire_date;

	if (login->account_kind != -1)
		this->account_kind = login->account_kind;
}

void WAConnection::sendMessageWithMedia(FMessage* message)  throw (WAException)
{
	logData("Send message with media %s %d", message->media_name.c_str(), message->media_size);
	logData("media-url:%s", message->media_url.c_str());
	if (message->media_wa_type == FMessage::WA_TYPE_SYSTEM)
		throw new WAException("Cannot send system message over the network");
	
	ProtocolTreeNode *mediaNode;
	if (message->media_wa_type == FMessage::WA_TYPE_CONTACT && !message->media_name.empty()) {
		ProtocolTreeNode* vcardNode = new ProtocolTreeNode("vcard", new std::vector<unsigned char>(message->data.begin(), message->data.end()))
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

	this->out->write(WAConnection::getMessageNode(message, mediaNode));
}

void WAConnection::sendMessageWithBody(FMessage* message) throw (WAException)
{
	ProtocolTreeNode* bodyNode = new ProtocolTreeNode("body", new std::vector<unsigned char>(message->data.begin(), message->data.end()));
	this->out->write(WAConnection::getMessageNode(message, bodyNode));
}

ProtocolTreeNode WAConnection::getMessageNode(FMessage* message, ProtocolTreeNode* child)
{
	ProtocolTreeNode* requestNode = NULL;
	ProtocolTreeNode* serverNode = new ProtocolTreeNode("server");
	std::vector<ProtocolTreeNode*>* children = new std::vector<ProtocolTreeNode*>(1);
	(*children)[0] = serverNode;
	ProtocolTreeNode* xNode = new ProtocolTreeNode("x", NULL, children) << XATTR("xmlns", "jabber:x:event");
	int childCount = (requestNode == NULL ? 0 : 1) + 2;
	std::vector<ProtocolTreeNode*>* messageChildren = new std::vector<ProtocolTreeNode*>(childCount);
	int i = 0;
	if (requestNode != NULL) {
		(*messageChildren)[i] = requestNode;
		i++;
	}
	(*messageChildren)[i] = xNode;
	i++;
	(*messageChildren)[i] = child;
	i++;

	return ProtocolTreeNode("message", NULL, messageChildren) <<
		XATTR("to", message->key->remote_jid) << XATTR("type", "chat") << XATTR("id", message->key->id);
}

void WAConnection::sendMessage(FMessage* message) throw(WAException)
{
	if (message->media_wa_type != 0)
		sendMessageWithMedia(message);
	else
		sendMessageWithBody(message);
}

void WAConnection::setVerboseId(bool b)
{
	this->verbose = b;
}

void WAConnection::sendAvailableForChat() throw(WAException)
{
	this->out->write(ProtocolTreeNode("presence") << XATTR("name", this->nick));
}

bool WAConnection::read() throw(WAException)
{
	ProtocolTreeNode* node;
	try {
		node = this->in->nextTree();
		this->lastTreeRead = time(NULL);
	}
	catch (exception& ex) {
		throw WAException(ex.what(), WAException::CORRUPT_STREAM_EX, 0);
	}

	if (node == NULL)
		return false;

	#ifdef _DEBUG
	{
		string tmp = node->toString();
		rawConn->log(tmp.c_str());
	}
	#endif

	if (ProtocolTreeNode::tagEquals(node, "iq")) {
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
				ProtocolTreeNode* accountNode = node->getChild(0);
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
				if (this->event_handler != NULL)
					this->event_handler->onAccountChange(this->account_kind, this->expire_date);
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
			ProtocolTreeNode* childNode = node->getChild(0);
			if (ProtocolTreeNode::tagEquals(childNode, "ping")) {
				if (this->event_handler != NULL)
					this->event_handler->onPing(id);
			}
			else if ((ProtocolTreeNode::tagEquals(childNode, "query") && !from.empty()) ? false : (ProtocolTreeNode::tagEquals(childNode, "relay")) && !from.empty()) {
				const string &pin = childNode->getAttributeValue("pin");
				if (!pin.empty() && this->event_handler != NULL) {
					int timeoutSeconds = atoi(childNode->getAttributeValue("timeout").c_str());
					this->event_handler->onRelayRequest(pin, timeoutSeconds, id);
				}
			}
		}
		else if (type == "set") {
			ProtocolTreeNode* childNode = node->getChild(0);
			if (ProtocolTreeNode::tagEquals(childNode, "query")) {
				const string &xmlns = childNode->getAttributeValue("xmlns");
				if (xmlns == "jabber:iq:roster") {
					std::vector<ProtocolTreeNode*> itemNodes(childNode->getAllChildren("item"));
					for (size_t i = 0; i < itemNodes.size(); i++) {
						ProtocolTreeNode* itemNode = itemNodes[i];
						const string &jid = itemNode->getAttributeValue("jid");
						const string &subscription = itemNode->getAttributeValue("subscription");
						// ask = itemNode->getAttributeValue("ask");
					}
				}
			}
		}
		else throw WAException("unknown iq type attribute: " + type, WAException::CORRUPT_STREAM_EX, 0);
	}
	else if (ProtocolTreeNode::tagEquals(node, "presence")) {
		const string &xmlns = node->getAttributeValue("xmlns");
		const string &from = node->getAttributeValue("from");
		if (xmlns == "urn:xmpp" && !from.empty()) {
			const string &type = node->getAttributeValue("type");
			if (type == "unavailable") {
				if (this->event_handler != NULL)
					this->event_handler->onAvailable(from, false);
			}
			else if (type == "available") {
				if (this->event_handler != NULL)
					this->event_handler->onAvailable(from, true);
			}
		}
		else if (xmlns == "w" && !from.empty()) {
			const string &add = node->getAttributeValue("add");
			const string &remove = node->getAttributeValue("remove");
			const string &status = node->getAttributeValue("status");
			if (!add.empty()) {
				if (this->group_event_handler != NULL)
					this->group_event_handler->onGroupAddUser(from, add);
			}
			else if (!remove.empty()) {
				if (this->group_event_handler != NULL)
					this->group_event_handler->onGroupRemoveUser(from, remove);
			}
			else if (status == "dirty") {
				std::map<string, string>* categories = parseCategories(node);
				if (this->event_handler != NULL)
					this->event_handler->onDirty(*categories);
				delete categories;
			}
		}
	}
	else if (ProtocolTreeNode::tagEquals(node, "message")) {
		parseMessageInitialTagAlreadyChecked(node);
	}

	delete node;
	return true;
}

void WAConnection::sendPing() throw(WAException)
{
	std::string id = makeId("ping_");
	this->pending_server_requests[id] = new IqResultPingHandler(this);

	ProtocolTreeNode* pingNode = new ProtocolTreeNode("ping") << XATTR("xmlns", "w:p");
	this->out->write(ProtocolTreeNode("iq", pingNode) << XATTR("id", id) << XATTR("type", "get"));
}

void WAConnection::sendPong(const std::string& id) throw(WAException)
{
	this->out->write(ProtocolTreeNode("iq")
		<< XATTR("type", "result") << XATTR("to", this->domain) << XATTR("id", id));
}

void WAConnection::sendComposing(const std::string& to) throw(WAException)
{
	ProtocolTreeNode* composingNode = new ProtocolTreeNode("composing")
		<< XATTR("xmlns", "http://jabber.org/protocol/chatstates");

	this->out->write(ProtocolTreeNode("message", composingNode) 
		<< XATTR("to", to) << XATTR("type", "chat"));
}

void WAConnection::sendActive() throw(WAException)
{
	this->out->write(ProtocolTreeNode("presence") << XATTR("type", "active"));
}

void WAConnection::sendInactive() throw(WAException)
{
	this->out->write(ProtocolTreeNode("presence") << XATTR("type", "inactive"));
}

void WAConnection::sendPaused(const std::string& to) throw(WAException)
{
	ProtocolTreeNode* pausedNode = new ProtocolTreeNode("paused"); 
	*pausedNode << XATTR("xmlns", "http://jabber.org/protocol/chatstates");

	this->out->write(ProtocolTreeNode("message", pausedNode) << XATTR("to", to) << XATTR("type", "chat"));
}

void WAConnection::sendSubjectReceived(const std::string& to, const std::string& id)throw(WAException)
{
	ProtocolTreeNode* receivedNode = new ProtocolTreeNode("received") << XATTR("xmlns", "urn:xmpp:receipts");

	this->out->write(ProtocolTreeNode("message", receivedNode) 
		<< XATTR("to", to) << XATTR("type", "subject") << XATTR("id", id));
}

void WAConnection::sendMessageReceived(FMessage* message) throw(WAException)
{
	ProtocolTreeNode* receivedNode = new ProtocolTreeNode("received")
		<< XATTR("xmlns", "urn:xmpp:receipts");

	this->out->write(ProtocolTreeNode("message", receivedNode)
		<< XATTR("to", message->key->remote_jid) << XATTR("type", "chat") << XATTR("id", message->key->id));
}

void WAConnection::sendDeliveredReceiptAck(const std::string& to,
	const std::string& id) throw(WAException)
{
	this->out->write(getReceiptAck(to, id, "delivered"));
}

void WAConnection::sendVisibleReceiptAck(const std::string& to, const std::string& id) throw (WAException)
{
	this->out->write(getReceiptAck(to, id, "visible"));
}

void WAConnection::sendPresenceSubscriptionRequest(const std::string& to) throw(WAException)
{
	this->out->write(ProtocolTreeNode("presence") << XATTR("type", "subscribe") << XATTR("to", to));
}

void WAConnection::sendClientConfig(const std::string& sound, const std::string& pushID, bool preview, const std::string& platform) throw(WAException)
{
	ProtocolTreeNode* configNode = new ProtocolTreeNode("config")
		<< XATTR("xmlns", "urn:xmpp:whatsapp:push") << XATTR("sound", sound) << XATTR("id", pushID) << XATTR("preview", preview ? "1" : "0") << XATTR("platform", platform);

	std::string id = makeId("config_");
	this->pending_server_requests[id] = new IqSendClientConfigHandler(this);

	this->out->write(ProtocolTreeNode("iq", configNode)
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", this->domain));
}

void WAConnection::sendClientConfig(const std::string& pushID, bool preview, const std::string& platform, bool defaultSettings, bool groupSettings, const std::vector<GroupSetting>& groups) throw(WAException)
{
	ProtocolTreeNode* configNode = new ProtocolTreeNode("config", NULL, this->processGroupSettings(groups))
		<< XATTR("xmlns", "urn:xmpp:whatsapp:push") << XATTR("id", pushID) << XATTR("lg", "en") << XATTR("lc", "US") << XATTR("clear", "0")
		<< XATTR("preview", preview ? "1" : "0") << XATTR("platform", platform)
		<< XATTR("default", defaultSettings ? "1" : "0") << XATTR("groups", groupSettings ? "1" : "0");

	std::string id = makeId("config_");
	this->out->write(ProtocolTreeNode("iq", configNode) << XATTR("id", id) << XATTR("type", "set") << XATTR("to", this->domain));
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

std::string WAConnection::makeId(const std::string& prefix)
{
	this->iqid++;
	std::string id;
	if (this->verbose)
		id = prefix + Utilities::intToStr(this->iqid);
	else
		id = Utilities::itoa(this->iqid, 16);

	return id;
}

ProtocolTreeNode WAConnection::getReceiptAck(const std::string& to, const std::string& id, const std::string& receiptType) throw(WAException)
{
	ProtocolTreeNode* ackNode = new ProtocolTreeNode("ack")
		<< XATTR("xmlns", "urn:xmpp:receipts") << XATTR("type", receiptType);

	return ProtocolTreeNode("message", ackNode) << XATTR("to", to) << XATTR("type", "chat") << XATTR("id", id);
}

std::map<string, string>* WAConnection::parseCategories(ProtocolTreeNode* dirtyNode) throw (WAException)
{
	std::map<string, string>* categories = new std::map<string, string>();
	if (dirtyNode->children != NULL) {
		for (size_t i = 0; i < dirtyNode->children->size(); i++) {
			ProtocolTreeNode* childNode = (*dirtyNode->children)[i];
			if (ProtocolTreeNode::tagEquals(childNode, "category")) {
				const string &categoryName = childNode->getAttributeValue("name");
				const string &timestamp = childNode->getAttributeValue("timestamp");
				(*categories)[categoryName] = timestamp;
			}
		}
	}

	return categories;
}

void WAConnection::parseMessageInitialTagAlreadyChecked(ProtocolTreeNode* messageNode) throw (WAException)
{
	const string &id = messageNode->getAttributeValue("id");
	const string &attribute_t = messageNode->getAttributeValue("t");
	const string &from = messageNode->getAttributeValue("from");
	const string &author = messageNode->getAttributeValue("author");

	const string &typeAttribute = messageNode->getAttributeValue("type");
	if (typeAttribute.empty())
		return;

	if (typeAttribute == "error") {
		int errorCode = 0;
		std::vector<ProtocolTreeNode*> errorNodes(messageNode->getAllChildren("error"));
		for (size_t i = 0; i < errorNodes.size(); i++) {
			ProtocolTreeNode *errorNode = errorNodes[i];
			errorCode = atoi(errorNode->getAttributeValue("code").c_str());
		}

		Key* key = new Key(from, true, id);
		FMessage* message = new FMessage(key);
		message->status = FMessage::STATUS_SERVER_BOUNCE;

		if (this->event_handler != NULL)
			this->event_handler->onMessageError(message, errorCode);
		delete message;
	}
	else if (typeAttribute == "subject") {
		bool receiptRequested = false;
		std::vector<ProtocolTreeNode*> requestNodes(messageNode->getAllChildren("request"));
		for (size_t i = 0; i < requestNodes.size(); i++) {
			ProtocolTreeNode *requestNode = requestNodes[i];
			if (requestNode->getAttributeValue("xmlns") == "urn:xmpp:receipts")
				receiptRequested = true;
		}

		ProtocolTreeNode* bodyNode = messageNode->getChild("body");
		if (bodyNode != NULL&& this->group_event_handler != NULL)
			this->group_event_handler->onGroupNewSubject(from, author, bodyNode->getDataAsString(), atoi(attribute_t.c_str()));

		if (receiptRequested)
			sendSubjectReceived(from, id);
	}
	else if (typeAttribute == "chat") {
		FMessage* fmessage = new FMessage();
		fmessage->wants_receipt = false;
		bool duplicate = false;

		std::vector<ProtocolTreeNode*> messageChildren(messageNode->getAllChildren());
		for (size_t i = 0; i < messageChildren.size(); i++) {
			ProtocolTreeNode* childNode = messageChildren[i];
			if (ProtocolTreeNode::tagEquals(childNode, "composing")) {
				if (this->event_handler != NULL)
					this->event_handler->onIsTyping(from, true);
			}
			else if (ProtocolTreeNode::tagEquals(childNode, "paused")) {
				if (this->event_handler != NULL)
					this->event_handler->onIsTyping(from, false);
			}
			else if (ProtocolTreeNode::tagEquals(childNode, "body")) {
				Key* key = new Key(from, false, id);
				fmessage->key = key;
				fmessage->remote_resource = author;
				fmessage->data = childNode->getDataAsString();
				fmessage->status = FMessage::STATUS_UNSENT;
			}
			else if (ProtocolTreeNode::tagEquals(childNode, "media") && !id.empty()) {
				fmessage->media_wa_type = FMessage::getMessage_WA_Type(childNode->getAttributeValue("type"));
				fmessage->media_url = childNode->getAttributeValue("url");
				fmessage->media_name = childNode->getAttributeValue("file");
				fmessage->media_size = Utilities::parseLongLong(childNode->getAttributeValue("size"));
				fmessage->media_duration_seconds = atoi(childNode->getAttributeValue("seconds").c_str());

				if (fmessage->media_wa_type == FMessage::WA_TYPE_LOCATION) {
					const string &latitudeString = childNode->getAttributeValue("latitude");
					const string &longitudeString = childNode->getAttributeValue("longitude");
					if (latitudeString.empty() || longitudeString.empty())
						throw WAException("location message missing lat or long attribute", WAException::CORRUPT_STREAM_EX, 0);

					double latitude = atof(latitudeString.c_str());
					double longitude = atof(longitudeString.c_str());
					fmessage->latitude = latitude;
					fmessage->longitude = longitude;
				}

				if (fmessage->media_wa_type == FMessage::WA_TYPE_CONTACT) {
					ProtocolTreeNode* contactChildNode = childNode->getChild(0);
					if (contactChildNode != NULL) {
						fmessage->media_name = contactChildNode->getAttributeValue("name");
						fmessage->data = contactChildNode->getDataAsString();
					}
				}
				else {
					const string &encoding = childNode->getAttributeValue("encoding");
					if (encoding.empty() || encoding == "text")
						fmessage->data = childNode->getDataAsString();
					else {
						logData("Media data encoding type '%s'", encoding.empty() ? "text" : encoding.c_str());
						fmessage->data = (childNode->data == NULL ? "" : std::string(base64_encode(childNode->data->data(), childNode->data->size())));
					}
				}

				Key* key = new Key(from, false, id);
				fmessage->key = key;
				fmessage->remote_resource = author;
			}
			else if (!ProtocolTreeNode::tagEquals(childNode, "active")) {
				if (ProtocolTreeNode::tagEquals(childNode, "request")) {
					fmessage->wants_receipt = true;
				}
				else if (ProtocolTreeNode::tagEquals(childNode, "notify")) {
					fmessage->notifyname = childNode->getAttributeValue("name");
				}
				else if (ProtocolTreeNode::tagEquals(childNode, "x")) {
					const string &xmlns = childNode->getAttributeValue("xmlns");
					if (xmlns == "jabber:x:event" && !id.empty()) {
						Key* key = new Key(from, true, id);
						FMessage* message = new FMessage(key);
						message->status = FMessage::STATUS_RECEIVED_BY_SERVER;
						if (this->event_handler != NULL)
							this->event_handler->onMessageStatusUpdate(message);
						delete message;
					}
				}
				else if (ProtocolTreeNode::tagEquals(childNode, "received")) {
					Key* key = new Key(from, true, id);
					FMessage* message = new FMessage(key);
					message->status = FMessage::STATUS_RECEIVED_BY_TARGET;
					if (this->event_handler != NULL)
						this->event_handler->onMessageStatusUpdate(message);
					delete message;
					if (this->supportsReceiptAcks()) {
						const string &receipt_type = childNode->getAttributeValue("type");
						if (receipt_type == "delivered")
							sendDeliveredReceiptAck(from, id);
						else if (receipt_type == "visible")
							sendVisibleReceiptAck(from, id);
					}
				}
				else if (ProtocolTreeNode::tagEquals(childNode, "offline")) {
					if (!attribute_t.empty())
						fmessage->timestamp = atoi(attribute_t.c_str());
					fmessage->offline = true;
				}
			}
		}

		if (fmessage->timestamp == 0) {
			fmessage->timestamp = time(NULL);
			fmessage->offline = false;
		}

		if (fmessage->key != NULL && this->event_handler != NULL)
			this->event_handler->onMessageForMe(fmessage, duplicate);

		delete fmessage;
	}
	else if (typeAttribute == "notification") {
		logData("Notification node %s", messageNode->toString().c_str());
		bool flag = false;
		std::vector<ProtocolTreeNode*> children(messageNode->getAllChildren());
		for (size_t i = 0; i < children.size(); i++) {
			ProtocolTreeNode* child = children[i];
			if (ProtocolTreeNode::tagEquals(child, "notification")) {
				const string &type = child->getAttributeValue("type");
				if (type == "picture" && this->event_handler != NULL) {
					std::vector<ProtocolTreeNode*> children2(child->getAllChildren());
					for (unsigned j = 0; j < children2.size(); j++) {
						ProtocolTreeNode* child2 = children2[j];
						if (ProtocolTreeNode::tagEquals(child2, "set")) {
							const string &id = child2->getAttributeValue("id");
							const string &author = child2->getAttributeValue("author");
							if (!id.empty())
								this->event_handler->onPictureChanged(from, author, true);
						}
						else if (ProtocolTreeNode::tagEquals(child2, "delete")) {
							const string &author = child2->getAttributeValue("author");
							this->event_handler->onPictureChanged(from, author, false);
						}
					}
				}
			}
			else if (ProtocolTreeNode::tagEquals(child, "request"))
				flag = true;
		}
		if (flag)
			this->sendNotificationReceived(from, id);
	}
}

bool WAConnection::supportsReceiptAcks()
{
	return supports_receipt_acks;
}

void WAConnection::sendNotificationReceived(const std::string& jid, const std::string& id) throw(WAException)
{
	ProtocolTreeNode* child = new ProtocolTreeNode("received") << XATTR("xmlns", "urn:xmpp:receipts");

	this->out->write(ProtocolTreeNode("message", child)
		<< XATTR("id", id) << XATTR("type", "notification") << XATTR("to", jid));
}

void WAConnection::sendClose() throw(WAException)
{
	this->out->write(ProtocolTreeNode("presence") << XATTR("type", "unavailable"));
	this->out->streamEnd();
}

void WAConnection::sendGetPrivacyList() throw (WAException)
{
	std::string id = makeId("privacylist_");
	this->pending_server_requests[id] = new IqResultPrivayListHandler(this);

	ProtocolTreeNode* listNode = new ProtocolTreeNode("list") << XATTR("name", "default");
	ProtocolTreeNode* queryNode = new ProtocolTreeNode("query", listNode) << XATTR("xmlns", "jabber:iq:privacy");
	this->out->write(ProtocolTreeNode("iq", queryNode) << XATTR("id", id) << XATTR("type", "get"));
}

void WAConnection::sendGetServerProperties() throw (WAException)
{
	std::string id = makeId("get_server_properties_");
	this->pending_server_requests[id] = new IqResultServerPropertiesHandler(this);
	
	ProtocolTreeNode* listNode = new ProtocolTreeNode("list")
		<< XATTR("xmlns", "w:g") << XATTR("type", "props");

	this->out->write(ProtocolTreeNode("iq", listNode)
		<< XATTR("id", id) << XATTR("type", "get") << XATTR("to", "g.us"));
}

void WAConnection::sendGetGroups() throw (WAException)
{
	this->mutex->lock();
	std::string id = makeId("get_groups_");
	this->pending_server_requests[id] = new IqResultGetGroupsHandler(this, "participating");

	sendGetGroups(id, "participating");
	this->mutex->unlock();
}

void WAConnection::sendGetOwningGroups() throw (WAException)
{
	this->mutex->lock();
	std::string id = makeId("get_owning_groups_");
	this->pending_server_requests[id] = new IqResultGetGroupsHandler(this, "owning");

	sendGetGroups(id, "owning");
	this->mutex->unlock();
}

void WAConnection::sendGetGroups(const std::string& id, const std::string& type) throw (WAException)
{
	ProtocolTreeNode* listNode = new ProtocolTreeNode("list")
		<< XATTR("xmlns", "w:g") << XATTR("type", type);

	this->out->write(ProtocolTreeNode("iq", listNode)
		<< XATTR("id", id) << XATTR("type", "get") << XATTR("to", "g.us"));
}

void WAConnection::readGroupList(ProtocolTreeNode* node, std::vector<std::string>& groups) throw (WAException)
{
	std::vector<ProtocolTreeNode*> nodes(node->getAllChildren("group"));
	for (size_t i = 0; i < nodes.size(); i++) {
		ProtocolTreeNode* groupNode = nodes[i];
		const string &gid = groupNode->getAttributeValue("id");
		string gjid = gidToGjid(gid);
		const string &owner = groupNode->getAttributeValue("owner");
		const string &subject = groupNode->getAttributeValue("subject");
		const string &subject_t = groupNode->getAttributeValue("s_t");
		const string &subject_owner = groupNode->getAttributeValue("s_o");
		const string &creation = groupNode->getAttributeValue("creation");
		if (this->group_event_handler != NULL)
			this->group_event_handler->onGroupInfoFromList(gjid, owner, subject, subject_owner, atoi(subject_t.c_str()), atoi(creation.c_str()));
		groups.push_back(gjid);
	}
}

std::string WAConnection::gidToGjid(const std::string& gid)
{
	return gid + "@g.us";
}

void WAConnection::sendQueryLastOnline(const std::string& jid) throw (WAException)
{
	std::string id = makeId("last_");
	this->pending_server_requests[id] = new IqResultQueryLastOnlineHandler(this);

	ProtocolTreeNode* queryNode = new ProtocolTreeNode("query") << XATTR("xmlns", "jabber:iq:last");
	this->out->write(ProtocolTreeNode("iq", queryNode)
		<< XATTR("id", id) << XATTR("type", "get") << XATTR("to", jid));
}

void WAConnection::sendGetGroupInfo(const std::string& gjid) throw (WAException)
{
	std::string id = makeId("get_g_info_");
	this->pending_server_requests[id] = new IqResultGetGroupInfoHandler(this);

	ProtocolTreeNode* queryNode = new ProtocolTreeNode("query") << XATTR("xmlns", "w:g");
	this->out->write(ProtocolTreeNode("iq", queryNode)
		<< XATTR("id", id) << XATTR("type", "get") << XATTR("to", gjid));
}

void WAConnection::sendGetParticipants(const std::string& gjid) throw (WAException)
{
	std::string id = makeId("get_participants_");
	this->pending_server_requests[id] = new IqResultGetGroupParticipantsHandler(this);

	ProtocolTreeNode* listNode = new ProtocolTreeNode("list") << XATTR("xmlns", "w:g");
	this->out->write(ProtocolTreeNode("iq", listNode)
		<< XATTR("id", id) << XATTR("type", "get") << XATTR("to", gjid));
}

void WAConnection::readAttributeList(ProtocolTreeNode* node, std::vector<std::string>& vector, const std::string& tag, const std::string& attribute) throw (WAException)
{
	std::vector<ProtocolTreeNode*> nodes(node->getAllChildren(tag));
	for (size_t i = 0; i < nodes.size(); i++) {
		ProtocolTreeNode* tagNode = nodes[i];
		vector.push_back(tagNode->getAttributeValue(attribute));
	}
}

void WAConnection::sendCreateGroupChat(const std::string& subject) throw (WAException)
{
	logData("sending create group: %s", subject.c_str());
	std::string id = makeId("create_group_");
	this->pending_server_requests[id] = new IqResultCreateGroupChatHandler(this);

	ProtocolTreeNode* groupNode = new ProtocolTreeNode("group")
		<< XATTR("xmlns", "w:g") << XATTR("action", "create") << XATTR("subject", subject);

	this->out->write(ProtocolTreeNode("iq", groupNode)
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", "g.us"));
}

void WAConnection::sendEndGroupChat(const std::string& gjid) throw (WAException)
{
	std::string id = makeId("remove_group_");

	ProtocolTreeNode* groupNode = new ProtocolTreeNode("group") << XATTR("xmlns", "w:g") << XATTR("action", "delete");
	this->out->write(ProtocolTreeNode("iq", groupNode)
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", gjid));
}

void WAConnection::sendClearDirty(const std::string& category) throw (WAException)
{
	std::string id = makeId("clean_dirty_");
	this->pending_server_requests[id] = new IqResultClearDirtyHandler(this);

	ProtocolTreeNode* categoryNode = new ProtocolTreeNode("category") << XATTR("name", category);
	ProtocolTreeNode* cleanNode = new ProtocolTreeNode("clean", categoryNode) << XATTR("xmlns", "urn:xmpp:whatsapp:dirty");
	this->out->write(ProtocolTreeNode("iq", cleanNode)
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", "s.whatsapp.net"));
}

void WAConnection::sendLeaveGroup(const std::string& gjid) throw (WAException)
{
	std::string id = makeId("leave_group_");

	ProtocolTreeNode* groupNode = new ProtocolTreeNode("group") << XATTR("id", gjid);
	ProtocolTreeNode* leaveNode = new ProtocolTreeNode("leave", groupNode) << XATTR("xmlns", "w:g");
	this->out->write(ProtocolTreeNode("iq", leaveNode)
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", "g.us"));
}

void WAConnection::sendAddParticipants(const std::string& gjid, const std::vector<std::string>& participants) throw (WAException)
{
	std::string id = makeId("add_group_participants_");
	this->sendVerbParticipants(gjid, participants, id, "add");
}

void WAConnection::sendRemoveParticipants(const std::string& gjid, const std::vector<std::string>& participants) throw (WAException)
{
	std::string id = makeId("remove_group_participants_");
	this->sendVerbParticipants(gjid, participants, id, "remove");
}

void WAConnection::sendVerbParticipants(const std::string& gjid, const std::vector<std::string>& participants, const std::string& id, const std::string& inner_tag) throw (WAException)
{
	size_t size = participants.size();
	std::vector<ProtocolTreeNode*>* children = new std::vector<ProtocolTreeNode*>(size);
	for (size_t i = 0; i < size; i++)
		(*children)[i] = new ProtocolTreeNode("participant") << XATTR("jid", participants[i]);

	ProtocolTreeNode* innerNode = new ProtocolTreeNode(inner_tag, NULL, children)
		<< XATTR("xmlns", "w:g");

	this->out->write(ProtocolTreeNode("iq", innerNode)
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", gjid));
}

void WAConnection::sendSetNewSubject(const std::string& gjid, const std::string& subject) throw (WAException)
{
	std::string id = this->makeId("set_group_subject_");

	ProtocolTreeNode* subjectNode = new ProtocolTreeNode("subject")
		<< XATTR("xmlns", "w:g") << XATTR("value", subject);

	this->out->write(ProtocolTreeNode("iq", subjectNode)
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", gjid));
}

std::string WAConnection::removeResourceFromJid(const std::string& jid)
{
	size_t slashidx = jid.find('/');
	if (slashidx == std::string::npos)
		return jid;

	return jid.substr(0, slashidx + 1);
}

void WAConnection::sendStatusUpdate(std::string& status) throw (WAException)
{
	std::string id = this->makeId(Utilities::intToStr((int)time(NULL)));
	FMessage* message = new FMessage(new Key("s.us", true, id));
	ProtocolTreeNode* body = new ProtocolTreeNode("body", new std::vector<unsigned char>(status.begin(), status.end()), NULL);
	this->out->write(getMessageNode(message, body));
	delete message;
}

void WAConnection::sendSetPicture(const std::string& jid, std::vector<unsigned char>* data) throw (WAException)
{
	std::string id = this->makeId("set_photo_");
	this->pending_server_requests[id] = new IqResultSetPhotoHandler(this, jid);

	ProtocolTreeNode* listNode = new ProtocolTreeNode("picture", data, NULL) << XATTR("xmlns", "w:profile:picture");
	this->out->write(ProtocolTreeNode("iq", listNode)
		<< XATTR("id", id) << XATTR("type", "set") << XATTR("to", jid));
}

void WAConnection::sendGetPicture(const std::string& jid, const std::string& type, const std::string& oldId, const std::string& newId) throw (WAException)
{
	std::string id = makeId("get_picture_");
	this->pending_server_requests[id] = new IqResultGetPhotoHandler(this, jid, oldId, newId);

	ProtocolTreeNode* listNode = new ProtocolTreeNode("picture")
		<< XATTR("xmlns", "w:profile:picture") << XATTR("type", type);

	this->out->write(ProtocolTreeNode("iq", listNode)
		<< XATTR("id", id) << XATTR("to", jid) << XATTR("type", "get"));
}

void WAConnection::sendGetPictureIds(const std::vector<std::string>& jids) throw (WAException)
{
	std::string id = makeId("get_picture_ids_");
	this->pending_server_requests[id] = new IqResultGetPictureIdsHandler(this);

	std::vector<ProtocolTreeNode*>* children = new std::vector<ProtocolTreeNode*>();
	for (size_t i = 0; i < jids.size(); i++) {
		ProtocolTreeNode* child = new ProtocolTreeNode("user") << XATTR("jid", jids[i]);
		children->push_back(child);
	}

	ProtocolTreeNode* queryNode = new ProtocolTreeNode("list", NULL, children) << XATTR("xmlns", "w:profile:picture");
	this->out->write(ProtocolTreeNode("iq", queryNode) << XATTR("id", id) << XATTR("type", "get"));
}

void WAConnection::sendDeleteAccount() throw (WAException)
{
	std::string id = makeId("del_acct_");
	this->pending_server_requests[id] = new IqResultSendDeleteAccount(this);

	ProtocolTreeNode* node1 = new ProtocolTreeNode("remove") << XATTR("xmlns", "urn:xmpp:whatsapp:account");
	this->out->write(ProtocolTreeNode("iq", node1) 
		<< XATTR("id", id) << XATTR("type", "get") << XATTR("to", "s.whatsapp.net"));
}
