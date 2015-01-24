/*
 * WAConnection.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include "WAConnection.h"
#include "ProtocolTreeNode.h"
#include <map>
#include <vector>
#include "utilities.h"

const char* WAConnection::dictionary[] = {
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

const int WAConnection::DICTIONARY_LEN = _countof(WAConnection::dictionary);

WAConnection::WAConnection(IMutex* mutex, WAListener* event_handler, WAGroupListener* group_event_handler)
{
	this->init(event_handler, group_event_handler, mutex);
}

WAConnection::~WAConnection()
{
	delete this->inputKey;
	delete this->outputKey;
	delete this->in;
	delete this->out;
	std::map<string, IqResultHandler*>::iterator it;
	for (it = this->pending_server_requests.begin(); it != this->pending_server_requests.end(); it++)
		delete it->second;
}

void WAConnection::init(WAListener* event_handler, WAGroupListener* group_event_handler, IMutex* mutex)
{
	this->login = NULL;
	this->event_handler = event_handler;
	this->group_event_handler = group_event_handler;
	this->inputKey = NULL;
	this->outputKey = NULL;
	this->in = NULL;
	this->out = NULL;

	this->msg_id = 0;
	this->state = 0; // 0 disconnected 1 connecting 2 connected
	this->retry = true;

	this->iqid = 0;
	this->verbose = true;
	this->lastTreeRead = 0;
	this->expire_date = 0L;
	this->account_kind = -1;
	this->mutex = mutex;
}

void WAConnection::setLogin(WALogin* login)
{
	this->login = login;

	if (login->expire_date != 0L) {
		this->expire_date = login->expire_date;
	}
	if (login->account_kind != -1) {
		this->account_kind = login->account_kind;
	}

	this->jid = this->login->user + "@" + this->login->domain;
	this->fromm = this->login->user + "@" + this->login->domain + "/" + this->login->resource;

	this->in = login->getTreeNodeReader();
	this->out = login->getTreeNodeWriter();
}

WALogin* WAConnection::getLogin()
{
	return this->login;
}

void WAConnection::sendMessageWithMedia(FMessage* message)  throw (WAException)
{
	_LOGDATA("Send message with media %s %d", message->media_name.c_str(), message->media_size);
	_LOGDATA("media-url:%s", message->media_url.c_str());
	if (message->media_wa_type == FMessage::WA_TYPE_SYSTEM)
		throw new WAException("Cannot send system message over the network");
	std::map<string, string>* attribs = new std::map<string, string>();
	(*attribs)["xmlns"] = "urn:xmpp:whatsapp:mms";
	(*attribs)["type"] = FMessage::getMessage_WA_Type_StrValue(message->media_wa_type);

	if (message->media_wa_type == FMessage::WA_TYPE_LOCATION) {
		(*attribs)["latitude"] = Utilities::doubleToStr(message->latitude);
		(*attribs)["longitude"] = Utilities::doubleToStr(message->longitude);
	}
	else {
		if (message->media_wa_type != FMessage::WA_TYPE_CONTACT && !message->media_name.empty() && !message->media_url.empty() && message->media_size > 0L) {
			(*attribs)["file"] = message->media_name;
			(*attribs)["size"] = Utilities::intToStr((int)message->media_size);
			(*attribs)["url"] = message->media_url;
		}
		else {
			(*attribs)["file"] = message->media_name;
			(*attribs)["size"] = Utilities::intToStr((int)message->media_size);
			(*attribs)["url"] = message->media_url;
			(*attribs)["seconds"] = Utilities::intToStr(message->media_duration_seconds);
		}
	}

	ProtocolTreeNode* mediaNode;
	if (message->media_wa_type == FMessage::WA_TYPE_CONTACT && !message->media_name.empty()) {
		std::map<string, string>* attribs2 = new std::map<string, string>();
		(*attribs2)["name"] = message->media_name;
		ProtocolTreeNode* vcardNode = new ProtocolTreeNode("vcard", attribs2, new std::vector<unsigned char>(message->data.begin(), message->data.end()));
		mediaNode = new ProtocolTreeNode("media", attribs, vcardNode);
	}
	else {
		(*attribs)["encoding"] = "text";
		mediaNode = new ProtocolTreeNode("media", attribs, new std::vector<unsigned char>(message->data.begin(), message->data.end()), NULL);
	}

	ProtocolTreeNode* root = WAConnection::getMessageNode(message, mediaNode);
	this->out->write(root);
	delete root;
}

void WAConnection::sendMessageWithBody(FMessage* message) throw (WAException)
{
	ProtocolTreeNode* bodyNode = new ProtocolTreeNode("body", NULL, new std::vector<unsigned char>(message->data.begin(), message->data.end()));
	ProtocolTreeNode* root = WAConnection::getMessageNode(message, bodyNode);
	this->out->write(root);
	delete root;
}

ProtocolTreeNode* WAConnection::getMessageNode(FMessage* message, ProtocolTreeNode* child)
{
	ProtocolTreeNode* requestNode = NULL;
	ProtocolTreeNode* serverNode = new ProtocolTreeNode("server", NULL);
	std::map<string, string>* attrib = new std::map<string, string>();
	(*attrib)["xmlns"] = "jabber:x:event";
	std::vector<ProtocolTreeNode*>* children = new std::vector<ProtocolTreeNode*>(1);
	(*children)[0] = serverNode;
	ProtocolTreeNode* xNode = new ProtocolTreeNode("x", attrib, NULL, children);
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

	std::map<string, string>* attrib2 = new std::map<string, string>();
	(*attrib2)["to"] = message->key->remote_jid;
	(*attrib2)["type"] = "chat";
	(*attrib2)["id"] = message->key->id;

	return new ProtocolTreeNode("message", attrib2, NULL, messageChildren);
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
	std::map<string, string>* attribs = new std::map<string, string>();
	(*attribs)["name"] = this->login->push_name;
	ProtocolTreeNode *presenceNode = new ProtocolTreeNode("presence", attribs);
	this->out->write(presenceNode);
	delete presenceNode;
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

	if (node == NULL) {
		return false;
	}

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
			else if (id.compare(0, this->login->user.size(), this->login->user) == 0) {
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

void WAConnection::sendNop() throw(WAException)
{
	this->out->write(NULL);
}

void WAConnection::sendPing() throw(WAException)
{
	std::string id = makeId("ping_");
	this->pending_server_requests[id] = new IqResultPingHandler(this);

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "w:p";
	ProtocolTreeNode* pingNode = new ProtocolTreeNode("ping", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "get";
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, pingNode);

	this->out->write(iqNode);
	delete iqNode;
}

void WAConnection::sendPong(const std::string& id) throw(WAException)
{
	std::map<string, string>* attribs = new std::map<string, string>();
	(*attribs)["type"] = "result";
	(*attribs)["to"] = this->login->domain;
	(*attribs)["id"] = id;
	ProtocolTreeNode *iqNode = new ProtocolTreeNode("iq", attribs);
	this->out->write(iqNode);
	delete iqNode;
}

void WAConnection::sendComposing(const std::string& to) throw(WAException)
{
	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "http://jabber.org/protocol/chatstates";
	ProtocolTreeNode* composingNode = new ProtocolTreeNode("composing", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["to"] = to;
	(*attribs2)["type"] = "chat";
	ProtocolTreeNode* messageNode = new ProtocolTreeNode("message", attribs2, composingNode);

	this->out->write(messageNode);

	delete messageNode;
}


void WAConnection::sendActive() throw(WAException)
{
	std::map<string, string>* attribs = new std::map<string, string>();
	(*attribs)["type"] = "active";
	ProtocolTreeNode* presenceNode = new ProtocolTreeNode("presence", attribs);

	this->out->write(presenceNode);

	delete presenceNode;
}

void WAConnection::sendInactive() throw(WAException)
{
	std::map<string, string>* attribs = new std::map<string, string>();
	(*attribs)["type"] = "inactive";
	ProtocolTreeNode* presenceNode = new ProtocolTreeNode("presence", attribs);

	this->out->write(presenceNode);

	delete presenceNode;
}

void WAConnection::sendPaused(const std::string& to) throw(WAException)
{
	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "http://jabber.org/protocol/chatstates";
	ProtocolTreeNode* pausedNode = new ProtocolTreeNode("paused", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["to"] = to;
	(*attribs2)["type"] = "chat";
	ProtocolTreeNode* messageNode = new ProtocolTreeNode("message", attribs2, pausedNode);

	this->out->write(messageNode);

	delete messageNode;
}

void WAConnection::sendSubjectReceived(const std::string& to, const std::string& id)throw(WAException)
{
	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "urn:xmpp:receipts";
	ProtocolTreeNode* receivedNode = new ProtocolTreeNode("received", attribs1);

	ProtocolTreeNode* messageNode = getSubjectMessage(to, id, receivedNode);

	this->out->write(messageNode);

	delete messageNode;
}

ProtocolTreeNode* WAConnection::getSubjectMessage(const std::string& to, const std::string& id, ProtocolTreeNode* child) throw (WAException)
{
	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["to"] = to;
	(*attribs1)["type"] = "subject";
	(*attribs1)["id"] = id;
	ProtocolTreeNode* messageNode = new ProtocolTreeNode("message", attribs1, child);

	return messageNode;
}

void WAConnection::sendMessageReceived(FMessage* message) throw(WAException)
{
	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "urn:xmpp:receipts";
	ProtocolTreeNode* receivedNode = new ProtocolTreeNode("received", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["to"] = message->key->remote_jid;
	(*attribs2)["type"] = "chat";
	(*attribs2)["id"] = message->key->id;

	ProtocolTreeNode* messageNode = new ProtocolTreeNode("message", attribs2, receivedNode);

	this->out->write(messageNode);
	delete messageNode;
}

void WAConnection::sendDeliveredReceiptAck(const std::string& to,
	const std::string& id) throw(WAException)
{
	ProtocolTreeNode *root = getReceiptAck(to, id, "delivered");
	this->out->write(root);
	delete root;
}

void WAConnection::sendVisibleReceiptAck(const std::string& to, const std::string& id) throw (WAException)
{
	ProtocolTreeNode *root = getReceiptAck(to, id, "visible");
	this->out->write(root);
	delete root;
}

void WAConnection::sendPresenceSubscriptionRequest(const std::string& to) throw(WAException)
{
	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["type"] = "subscribe";
	(*attribs1)["to"] = to;
	ProtocolTreeNode* presenceNode = new ProtocolTreeNode("presence", attribs1);
	this->out->write(presenceNode);
	delete presenceNode;
}

void WAConnection::sendClientConfig(const std::string& sound, const std::string& pushID, bool preview, const std::string& platform) throw(WAException)
{
	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "urn:xmpp:whatsapp:push";
	(*attribs1)["sound"] = sound;
	(*attribs1)["id"] = pushID;
	(*attribs1)["preview"] = preview ? "1" : "0";
	(*attribs1)["platform"] = platform;
	ProtocolTreeNode* configNode = new ProtocolTreeNode("config", attribs1);

	std::string id = makeId("config_");

	this->pending_server_requests[id] = new IqSendClientConfigHandler(this);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "set";
	(*attribs2)["to"] = this->login->domain;

	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, configNode);

	this->out->write(iqNode);
	delete iqNode;

}

void WAConnection::sendClientConfig(const std::string& pushID, bool preview, const std::string& platform, bool defaultSettings, bool groupSettings, const std::vector<GroupSetting>& groups) throw(WAException)
{
	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "urn:xmpp:whatsapp:push";
	(*attribs1)["id"] = pushID;
	(*attribs1)["lg"] = "en";
	(*attribs1)["lc"] = "US";
	(*attribs1)["clear"] = "0";
	(*attribs1)["preview"] = preview ? "1" : "0";
	(*attribs1)["platform"] = platform;
	(*attribs1)["default"] = defaultSettings ? "1" : "0";
	(*attribs1)["groups"] = groupSettings ? "1" : "0";
	ProtocolTreeNode* configNode = new ProtocolTreeNode("config", attribs1, NULL, this->processGroupSettings(groups));
	std::string id = makeId("config_");

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "set";
	(*attribs2)["to"] = this->login->domain;

	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, configNode);
	this->out->write(iqNode);
	delete iqNode;
}

std::vector<ProtocolTreeNode*>* WAConnection::processGroupSettings(const std::vector<GroupSetting>& groups)
{
	std::vector<ProtocolTreeNode*>* result = new std::vector<ProtocolTreeNode*>(groups.size());
	if (!groups.empty()) {
		time_t now = time(NULL);
		for (size_t i = 0; i < groups.size(); i++) {
			std::map<string, string>* attribs = new std::map<string, string>();
			(*attribs)["jid"] = groups[i].jid;
			(*attribs)["notify"] = (groups[i].enabled ? "1" : "0");
			(*attribs)["mute"] = Utilities::intToStr(int(groups[i].muteExpiry > now ? (groups[i].muteExpiry - now) : 0));
			_LOGDATA("mute group %s, %s", (*attribs)["jid"].c_str(), (*attribs)["mute"].c_str());

			(*result)[i] = new ProtocolTreeNode("item", attribs);
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

ProtocolTreeNode* WAConnection::getReceiptAck(const std::string& to, const std::string& id, const std::string& receiptType) throw(WAException)
{
	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "urn:xmpp:receipts";
	(*attribs1)["type"] = receiptType;
	ProtocolTreeNode* ackNode = new ProtocolTreeNode("ack", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["to"] = to;
	(*attribs2)["type"] = "chat";
	(*attribs2)["id"] = id;
	ProtocolTreeNode* messageNode = new ProtocolTreeNode("message", attribs2, ackNode);

	return messageNode;
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
						_LOGDATA("Media data encoding type '%s'", encoding.empty() ? "text" : encoding.c_str());
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
		_LOGDATA("Notification node %s", messageNode->toString().c_str());
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
	return (this->login != NULL) && (this->login->supports_receipt_acks);
}

void WAConnection::sendNotificationReceived(const std::string& jid, const std::string& id) throw(WAException)
{
	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "urn:xmpp:receipts";
	ProtocolTreeNode* child = new ProtocolTreeNode("received", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "notification";
	(*attribs2)["to"] = jid;
	ProtocolTreeNode* node = new ProtocolTreeNode("message", attribs2, child);

	this->out->write(node);
	delete node;
}

void WAConnection::sendClose() throw(WAException)
{
	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["type"] = "unavailable";
	ProtocolTreeNode* presenceNode = new ProtocolTreeNode("presence", attribs1);
	this->out->write(presenceNode);
	delete presenceNode;
	this->out->streamEnd();
}

void WAConnection::sendGetPrivacyList() throw (WAException)
{
	std::string id = makeId("privacylist_");
	this->pending_server_requests[id] = new IqResultPrivayListHandler(this);

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["name"] = "default";
	ProtocolTreeNode* listNode = new ProtocolTreeNode("list", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["xmlns"] = "jabber:iq:privacy";
	ProtocolTreeNode* queryNode = new ProtocolTreeNode("query", attribs2, listNode);

	std::map<string, string>* attribs3 = new std::map<string, string>();
	(*attribs3)["id"] = id;
	(*attribs3)["type"] = "get";
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs3, queryNode);

	this->out->write(iqNode);
	delete iqNode;
}

void WAConnection::sendGetServerProperties() throw (WAException)
{
	std::string id = makeId("get_server_properties_");
	this->pending_server_requests[id] = new IqResultServerPropertiesHandler(this);

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "w:g";
	(*attribs1)["type"] = "props";
	ProtocolTreeNode* listNode = new ProtocolTreeNode("list", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "get";
	(*attribs2)["to"] = "g.us";
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, listNode);

	this->out->write(iqNode);
	delete iqNode;
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
	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "w:g";
	(*attribs1)["type"] = type;
	ProtocolTreeNode* listNode = new ProtocolTreeNode("list", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "get";
	(*attribs2)["to"] = "g.us";
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, listNode);

	this->out->write(iqNode);
	delete iqNode;
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

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "jabber:iq:last";
	ProtocolTreeNode* queryNode = new ProtocolTreeNode("query", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "get";
	(*attribs2)["to"] = jid;
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, queryNode);

	this->out->write(iqNode);
	delete iqNode;
}

void WAConnection::sendGetGroupInfo(const std::string& gjid) throw (WAException)
{
	std::string id = makeId("get_g_info_");
	this->pending_server_requests[id] = new IqResultGetGroupInfoHandler(this);

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "w:g";
	ProtocolTreeNode* queryNode = new ProtocolTreeNode("query", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "get";
	(*attribs2)["to"] = gjid;
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, queryNode);

	this->out->write(iqNode);
	delete iqNode;
}

void WAConnection::sendGetParticipants(const std::string& gjid) throw (WAException)
{
	std::string id = makeId("get_participants_");
	this->pending_server_requests[id] = new IqResultGetGroupParticipantsHandler(this);

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "w:g";
	ProtocolTreeNode* listNode = new ProtocolTreeNode("list", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "get";
	(*attribs2)["to"] = gjid;
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, listNode);

	this->out->write(iqNode);
	delete iqNode;
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
	_LOGDATA("sending create group: %s", subject.c_str());
	std::string id = makeId("create_group_");
	this->pending_server_requests[id] = new IqResultCreateGroupChatHandler(this);

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "w:g";
	(*attribs1)["action"] = "create";
	(*attribs1)["subject"] = subject;
	ProtocolTreeNode* groupNode = new ProtocolTreeNode("group", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "set";
	(*attribs2)["to"] = "g.us";
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, groupNode);

	this->out->write(iqNode);
	delete iqNode;
}

void WAConnection::sendEndGroupChat(const std::string& gjid) throw (WAException)
{
	std::string id = makeId("remove_group_");

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "w:g";
	(*attribs1)["action"] = "delete";
	ProtocolTreeNode* groupNode = new ProtocolTreeNode("group", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "set";
	(*attribs2)["to"] = gjid;
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, groupNode);

	this->out->write(iqNode);
	delete iqNode;
}

void WAConnection::sendClearDirty(const std::string& category) throw (WAException)
{
	std::string id = makeId("clean_dirty_");
	this->pending_server_requests[id] = new IqResultClearDirtyHandler(this);

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["name"] = category;
	ProtocolTreeNode* categoryNode = new ProtocolTreeNode("category", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["xmlns"] = "urn:xmpp:whatsapp:dirty";
	ProtocolTreeNode* cleanNode = new ProtocolTreeNode("clean", attribs2, categoryNode);

	std::map<string, string>* attribs3 = new std::map<string, string>();
	(*attribs3)["id"] = id;
	(*attribs3)["type"] = "set";
	(*attribs3)["to"] = "s.whatsapp.net";
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs3, cleanNode);

	this->out->write(iqNode);
	delete iqNode;
}

void WAConnection::sendLeaveGroup(const std::string& gjid) throw (WAException)
{
	std::string id = makeId("leave_group_");

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["id"] = gjid;
	ProtocolTreeNode* groupNode = new ProtocolTreeNode("group", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["xmlns"] = "w:g";
	ProtocolTreeNode* leaveNode = new ProtocolTreeNode("leave", attribs2, groupNode);

	std::map<string, string>* attribs3 = new std::map<string, string>();
	(*attribs3)["id"] = id;
	(*attribs3)["type"] = "set";
	(*attribs3)["to"] = "g.us";
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs3, leaveNode);

	this->out->write(iqNode);
	delete iqNode;
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
	for (size_t i = 0; i < size; i++) {
		std::map<string, string>* attribs1 = new std::map<string, string>();
		(*attribs1)["jid"] = participants[i];
		(*children)[i] = new ProtocolTreeNode("participant", attribs1);
	}

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["xmlns"] = "w:g";
	ProtocolTreeNode* innerNode = new ProtocolTreeNode(inner_tag, attribs2, NULL, children);

	std::map<string, string>* attribs3 = new std::map<string, string>();
	(*attribs3)["id"] = id;
	(*attribs3)["type"] = "set";
	(*attribs3)["to"] = gjid;
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs3, innerNode);

	this->out->write(iqNode);
	delete iqNode;
}

void WAConnection::sendSetNewSubject(const std::string& gjid, const std::string& subject) throw (WAException)
{
	std::string id = this->makeId("set_group_subject_");

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "w:g";
	(*attribs1)["value"] = subject;
	ProtocolTreeNode* subjectNode = new ProtocolTreeNode("subject", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "set";
	(*attribs2)["to"] = gjid;
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, subjectNode);

	this->out->write(iqNode);
	delete iqNode;
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
	ProtocolTreeNode* body = new ProtocolTreeNode("body", NULL, new std::vector<unsigned char>(status.begin(), status.end()), NULL);
	ProtocolTreeNode* messageNode = getMessageNode(message, body);
	this->out->write(messageNode);
	delete messageNode;
	delete message;
}

void WAConnection::sendSetPicture(const std::string& jid, std::vector<unsigned char>* data) throw (WAException)
{
	std::string id = this->makeId("set_photo_");
	this->pending_server_requests[id] = new IqResultSetPhotoHandler(this, jid);

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "w:profile:picture";
	// (*attribs1)["type"] = "image";
	ProtocolTreeNode* listNode = new ProtocolTreeNode("picture", attribs1, data, NULL);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "set";
	(*attribs2)["to"] = jid;

	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, listNode);
	this->out->write(iqNode);
	delete iqNode;
}

void WAConnection::sendGetPicture(const std::string& jid, const std::string& type, const std::string& oldId, const std::string& newId) throw (WAException)
{
	std::string id = makeId("get_picture_");
	this->pending_server_requests[id] = new IqResultGetPhotoHandler(this, jid, oldId, newId);

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "w:profile:picture";
	(*attribs1)["type"] = type;
	ProtocolTreeNode* listNode = new ProtocolTreeNode("picture", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["to"] = jid;
	(*attribs2)["type"] = "get";
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, listNode);

	this->out->write(iqNode);
	delete iqNode;
}

void WAConnection::sendGetPictureIds(const std::vector<std::string>& jids) throw (WAException)
{
	std::string id = makeId("get_picture_ids_");
	this->pending_server_requests[id] = new IqResultGetPictureIdsHandler(this);

	std::vector<ProtocolTreeNode*>* children = new std::vector<ProtocolTreeNode*>();
	for (size_t i = 0; i < jids.size(); i++) {
		std::map<string, string>* attribs = new std::map<string, string>();
		(*attribs)["jid"] = jids[i];
		ProtocolTreeNode* child = new ProtocolTreeNode("user", attribs);
		children->push_back(child);
	}

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "w:profile:picture";
	ProtocolTreeNode* queryNode = new ProtocolTreeNode("list", attribs1, NULL, children);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "get";
	ProtocolTreeNode* iqNode = new ProtocolTreeNode("iq", attribs2, queryNode);

	this->out->write(iqNode);
	delete iqNode;
}

void WAConnection::sendDeleteAccount() throw (WAException)
{
	std::string id = makeId("del_acct_");
	this->pending_server_requests[id] = new IqResultSendDeleteAccount(this);

	std::map<string, string>* attribs1 = new std::map<string, string>();
	(*attribs1)["xmlns"] = "urn:xmpp:whatsapp:account";
	ProtocolTreeNode* node1 = new ProtocolTreeNode("remove", attribs1);

	std::map<string, string>* attribs2 = new std::map<string, string>();
	(*attribs2)["id"] = id;
	(*attribs2)["type"] = "get";
	(*attribs2)["to"] = "s.whatsapp.net";

	ProtocolTreeNode* node2 = new ProtocolTreeNode("iq", attribs2, node1);
	this->out->write(node2);
	delete node2;
}
