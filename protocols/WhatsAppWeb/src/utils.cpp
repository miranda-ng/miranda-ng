/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019 George Hazan

*/

#include "stdafx.h"

WAUser* WhatsAppProto::FindUser(const char *szId)
{
	mir_cslock lck(m_csUsers);
	auto *tmp = (WAUser *)_alloca(sizeof(WAUser));
	tmp->szId = (char*)szId;
	return m_arUsers.find(tmp);
}

WAUser* WhatsAppProto::AddUser(const char *szId, bool bTemporary)
{
	auto *pUser = FindUser(szId);
	if (pUser != nullptr)
		return pUser;

	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);
	setString(hContact, DBKEY_ID, szId);
	pUser = new WAUser(hContact, mir_strdup(szId));
	if (bTemporary)
		Contact::RemoveFromList(hContact);
	if (m_wszDefaultGroup)
		Clist_SetGroup(hContact, m_wszDefaultGroup);

	mir_cslock lck(m_csUsers);
	m_arUsers.insert(pUser);
	return pUser;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool WhatsAppProto::getBlob(const char *szSetting, MBinBuffer &buf)
{
	DBVARIANT dbv = { DBVT_BLOB };
	if (db_get(0, m_szModuleName, szSetting, &dbv))
		return false;

	buf.assign(dbv.pbVal, dbv.cpbVal);
	db_free(&dbv);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// sends a piece of JSON to a server via a websocket, masked

int WhatsAppProto::WSSend(const MessageLite &msg, WA_PKT_HANDLER /*pHandler*/, void * /*pUserInfo*/)
{
	if (m_hServerConn == nullptr)
		return -1;

	// debugLogA("Sending packet: %s", msg..DebugString().c_str());

	int cbLen = msg.ByteSize();
	ptrA protoBuf((char *)mir_alloc(cbLen));
	msg.SerializeToArray(protoBuf, cbLen);

	MBinBuffer payload = m_noise->encodeFrame(protoBuf, cbLen);
	WebSocket_SendBinary(m_hServerConn, payload.data(), payload.length());
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static char zeroData[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int WhatsAppProto::WSSendNode(const char *pszPrefix, int flags, WANode &node, WA_PKT_HANDLER pHandler)
{
	if (m_hServerConn == nullptr)
		return 0;

	{
		char str[100];
		_i64toa(_time64(0), str, 10);
		node.addAttr("epoch", str);

		CMStringA szText;
		node.print(szText);
		debugLogA("Sending binary node: %s", szText.c_str());
	}

	WAWriter writer;
	writer.writeNode(&node);

	// AES block size = 16 bytes, let's expand data to block size boundary
	size_t rest = writer.body.length() % 16;
	if (rest != 0)
		writer.body.append(zeroData, 16 - rest);

	BYTE iv[16];
	Utils_GetRandom(iv, sizeof(iv));

	int pktId = ++m_iPktNumber;

	if (pHandler != nullptr) {
		auto *pReq = new WARequest;
		pReq->pHandler = pHandler;
		pReq->szPrefix = pszPrefix;

		mir_cslock lck(m_csPacketQueue);
		m_arPacketQueue.insert(pReq);
	}

	char postPrefix[3] = {',', 0, (char)flags};

	MBinBuffer ret;
	ret.append(pszPrefix, strlen(pszPrefix));
	ret.append(postPrefix, sizeof(postPrefix));
	WebSocket_SendBinary(m_hServerConn, ret.data(), ret.length());

	return pktId;
}

/////////////////////////////////////////////////////////////////////////////////////////
// WANode members

WANode::WANode()
{}

WANode::~WANode()
{
	for (auto &p: attrs)
		delete p;

	for (auto &p: children)
		delete p;
}

CMStringA WANode::getAttr(const char *pszFieldName) const
{
	for (auto &p: attrs)
		if (p->name == pszFieldName)
			return p->value;

	return "";
}

void WANode::addAttr(const char *pszName, const char *pszValue)
{
	attrs.push_back(new Attr(pszName, pszValue));
}

void WANode::print(CMStringA &dest, int level) const
{
	for (int i = 0; i < level; i++)
		dest.Append("  ");

	dest.AppendFormat("<%s ", title.c_str());
	for (auto &p: attrs)
		dest.AppendFormat("%s=\"%s\" ", p->name.c_str(), p->value.c_str());
	dest.Truncate(dest.GetLength() - 1);

	if (content.isEmpty() && children.empty()) {
		dest.Append("/>\n");
		return;
	}

	dest.Append(">");
	if (!content.isEmpty()) {
		ptrA tmp((char *)mir_alloc(content.length() * 2 + 1));
		bin2hex(content.data(), content.length(), tmp);
		dest.AppendFormat("%s", tmp.get());
	}

	if (!children.empty()) {
		dest.Append("\n");

		for (auto &p : children)
			p->print(dest, level + 1);

		for (int i = 0; i < level; i++)
			dest.Append("  ");
	}

	dest.AppendFormat("</%s>\n", title.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////
// WAReader class members

bool WAReader::readAttributes(WANode *pNode, int count)
{
	if (count == 0)
		return true;

	for (int i = 0; i < count; i++) {
		CMStringA name = readString(readInt8());
		if (name.IsEmpty())
			return false;

		CMStringA value = readString(readInt8());
		if (value.IsEmpty())
			return false;

		pNode->addAttr(name, value);
	}
	return true;
}

uint32_t WAReader::readInt20()
{
	if (m_limit - m_buf < 3)
		return 0;

	int ret = (int(m_buf[0] & 0x0F) << 16) + (int(m_buf[1]) << 8) + int(m_buf[2]);
	m_buf += 3;
	return ret;
}

uint32_t WAReader::readIntN(int n)
{
	if (m_limit - m_buf < n)
		return 0;

	uint32_t res = 0;
	for (int i = 0; i < n; i++, m_buf++)
		res = (res <<= 8) + *m_buf;
	return res;
}

bool WAReader::readList(WANode *pParent, int tag)
{
	int size = readListSize(tag);
	if (size == -1)
		return false;

	for (int i = 0; i < size; i++) {
		WANode *pNew = readNode();
		if (pNew == nullptr)
			return false;
		pParent->children.push_back(pNew);
	}
	
	return true;
}

int WAReader::readListSize(int tag)
{
	switch (tag) {
	case LIST_EMPTY:
		return 0;
	case LIST_8:
		return readInt8();
	case LIST_16:
		return readInt16();
	}
	return -1;
}

WANode* WAReader::readNode()
{
	int listSize = readListSize(readInt8());
	if (listSize == -1)
		return nullptr;

	int descrTag = readInt8();
	if (descrTag == STREAM_END)
		return nullptr;

	CMStringA name = readString(descrTag);
	if (name.IsEmpty())
		return nullptr;

	std::unique_ptr<WANode> ret(new WANode());
	ret->title = name.c_str();

	if (!readAttributes(ret.get(), (listSize-1)>>1))
		return nullptr;

	if ((listSize % 2) == 1)
		return ret.release();

	int size, tag = readInt8();
	switch (tag) {
	case LIST_EMPTY:	case LIST_8:  case LIST_16:
		readList(ret.get(), tag);
		break;

	case BINARY_8:
		size = readInt8();

LBL_Binary:
		if (m_limit - m_buf < size)
			return false;

		ret->content.assign((void*)m_buf, size);
		m_buf += size;
		break;

	case BINARY_20:
		size = readInt20();
		goto LBL_Binary;

	case BINARY_32:
		size = readInt32();
		goto LBL_Binary;

	default:
		CMStringA str = readString(tag);
		ret->content.assign(str.GetBuffer(), str.GetLength() + 1);
	}
	
	return ret.release();
}

/////////////////////////////////////////////////////////////////////////////////////////

static int unpackHex(int val)
{
	if (val < 0 || val > 15)
		return -1;

	return (val < 10) ? val + '0' : val - 10 + 'A';
}

static int unpackNibble(int val)
{
	if (val < 0 || val > 15)
		return -1;

	switch (val) {
	case 10: return '-';
	case 11: return '.';
	case 15: return 0;
	default: return '0' + val;
	}
}

CMStringA WAReader::readPacked(int tag)
{
	int startByte = readInt8();
	bool bTrim = false;
	if (startByte & 0x80) {
		startByte &= ~0x80;
		bTrim = true;
	}

	CMStringA ret;
	for (int i = 0; i < startByte; i++) {
		BYTE b = readInt8();
		int lower = (tag == NIBBLE_8) ? unpackNibble(b >> 4) : unpackHex(b >> 4);
		if (lower == -1)
			return "";

		int higher = (tag == NIBBLE_8) ? unpackNibble(b & 0x0F) : unpackHex(b & 0x0F);
		if (higher == -1)
			return "";

		ret.AppendChar(lower);
		ret.AppendChar(higher);
	}

	if (bTrim && !ret.IsEmpty())
		ret.Truncate(ret.GetLength() - 1);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////

static char *SingleByteTokens[] = {
	"", "", "", "200", "400", "404", "500", "501", "502", "action", "add",
	"after", "archive", "author", "available", "battery", "before", "body",
	"broadcast", "chat", "clear", "code", "composing", "contacts", "count",
	"create", "debug", "delete", "demote", "duplicate", "encoding", "error",
	"false", "filehash", "from", "g.us", "group", "groups_v2", "height", "id",
	"image", "in", "index", "invis", "item", "jid", "kind", "last", "leave",
	"live", "log", "media", "message", "mimetype", "missing", "modify", "name",
	"notification", "notify", "out", "owner", "participant", "paused",
	"picture", "played", "presence", "preview", "promote", "query", "raw",
	"read", "receipt", "received", "recipient", "recording", "relay",
	"remove", "response", "resume", "retry", "s.whatsapp.net", "seconds",
	"set", "size", "status", "subject", "subscribe", "t", "text", "to", "true",
	"type", "unarchive", "unavailable", "url", "user", "value", "web", "width",
	"mute", "read_only", "admin", "creator", "short", "update", "powersave",
	"checksum", "epoch", "block", "previous", "409", "replaced", "reason",
	"spam", "modify_tag", "message_info", "delivery", "emoji", "title",
	"description", "canonical-url", "matched-text", "star", "unstar",
	"media_key", "filename", "identity", "unread", "page", "page_count",
	"search", "media_message", "security", "call_log", "profile", "ciphertext",
	"invite", "gif", "vcard", "frequent", "privacy", "blacklist", "whitelist",
	"verify", "location", "document", "elapsed", "revoke_invite", "expiration",
	"unsubscribe", "disable", "vname", "old_jid", "new_jid", "announcement",
	"locked", "prop", "label", "color", "call", "offer", "call-id",
	"quick_reply", "sticker", "pay_t", "accept", "reject", "sticker_pack",
	"invalid", "canceled", "missed", "connected", "result", "audio",
	"video", "recent" };

CMStringA WAReader::readString(int tag)
{
	if (tag >= 3 && tag < _countof(SingleByteTokens)) {
		CMStringA ret = SingleByteTokens[tag];
		if (ret == "s.whatsapp.net")
			return "c.us";
		return ret;
	}

	switch (tag) {
//	case DICTIONARY_0: return dict0[readInt8()];
//	case DICTIONARY_1: return dict1[readInt8()];
//	case DICTIONARY_2: return dict2[readInt8()];
//	case DICTIONARY_3: return dict3[readInt8()];
	case LIST_EMPTY:
		return "";

	case BINARY_8:
		return readStringFromChars(readInt8());

	case BINARY_20:
		return readStringFromChars(readInt20());

	case BINARY_32:
		return readStringFromChars(readInt32());

	case NIBBLE_8:
	case HEX_8:
		return readPacked(tag);

	case JID_PAIR:
		CMStringA s1 = readString(readInt8());
		if (s1.IsEmpty())
			break;

		CMStringA s2 = readString(readInt8());
		if (s2.IsEmpty())
			break;

		return CMStringA(FORMAT, "%s@%s", s1.c_str(), s2.c_str());
	}

	// error
	return "";
}

CMStringA WAReader::readStringFromChars(int size)
{
	if (m_limit - m_buf < size)
		return "";

	CMStringA ret((char*)m_buf, size);
	m_buf += size;
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////
// WAWriter class members

void WAWriter::writeByte(uint8_t b)
{
	body.append(&b, 1);
}

void WAWriter::writeIntN(int value, int n)
{
	for (int i = n - 1; i >= 0; i--)
		writeByte((value >> i * 8) & 0xFF);
}

void WAWriter::writeInt20(int value)
{
	writeByte((value >> 16) & 0xFF);
	writeByte((value >> 8) & 0xFF);
	writeByte(value & 0xFF);
}

void WAWriter::writeLength(int value)
{
	if (value >= (1 << 20)) {
		writeByte(BINARY_32);
		writeInt32(value);
	}
	else if (value >= 256) {
		writeByte(BINARY_20);
		writeInt20(value);
	}
	else {
		writeByte(BINARY_8);
		writeInt8(value);
	}
}

void WAWriter::writeListSize(int length)
{
	if (length == 0)
		writeByte(LIST_EMPTY);
	else if (length < 256) {
		writeByte(LIST_8);
		writeInt8(length);
	}
	else {
		writeByte(LIST_16);
		writeInt16(length);
	}
}

void WAWriter::writeNode(const WANode *pNode)
{
	int numAttrs = (int)pNode->attrs.size();
	int hasContent = pNode->content.length() != 0;
	writeListSize(2*numAttrs + 1 + hasContent);

	writeString(pNode->title.c_str());

	// write attributes
	for (auto &it : pNode->attrs) {
		if (it->value.IsEmpty())
			continue;

		writeString(it->name.c_str());
		writeString(it->value.c_str());
	}

	// write contents
	if (pNode->content.length()) {
		writeLength((int)pNode->content.length());
		body.append(pNode->content.data(), pNode->content.length());
	}

	// write children
	if (pNode->children.size()) {
		writeListSize((int)pNode->children.size());
		for (auto &it : pNode->children)
			writeNode(it);
	}
}

void WAWriter::writeString(const char *str, bool bRaw)
{
	if (!bRaw && !mir_strcmp(str, "c.us")) {
		writeToken("s.whatsapp.net");
		return;
	}

	if (writeToken(str))
		return;

	auto *pszDelimiter = strchr(str, '@');
	if (pszDelimiter) {
		writeByte(JID_PAIR);

		if (pszDelimiter == str) // empty jid
			writeByte(LIST_EMPTY);
		else
			writePacked(CMStringA(str, int(pszDelimiter - str)));
	}
	else {
		int len = (int)strlen(str);
		writeLength(len);
		body.append(str, len);
	}
}

bool WAWriter::writeToken(const char *str)
{
	for (auto &it : SingleByteTokens)
		if (!strcmp(str, it)) {
			writeByte(int(&it - SingleByteTokens));
			return true;
		}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

static BYTE packNibble(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';

	switch (c) {
	case '-': return 10;
	case '.': return 11;
	case 0: return 15;
	}

	return -1;
}

static BYTE packHex(char c)
{
	if (c == 0)
		return 15;

	if (c >= '0' && c <= '9')
		return c - '0';

	if (c >= 'A' && c <= 'F')
		return c - 'A';

	if (c >= 'a' && c <= 'f')
		return c - 'a';

	return -1;
}

static BYTE packPair(int type, char c1, char c2)
{
	BYTE b1 = (type == NIBBLE_8) ? packNibble(c1) : packHex(c1);
	BYTE b2 = (type == NIBBLE_8) ? packNibble(c2) : packHex(c2);
	return (b1 << 4) + b2;
}

void WAWriter::writePacked(const CMStringA &str)
{
	if (str.GetLength() > 254)
		return;

	// all symbols of str can be a nibble?
	int type = (strspn(str, "0123456789-.") == str.GetLength()) ? NIBBLE_8 : HEX_8;

	int len = str.GetLength() / 2;
	BYTE firstByte = (len % 2) == 0 ? 0 : 0x80;
	writeByte(firstByte | len);

	const char *p = str;
	for (int i = 0; i < len; i++, p += 2)
		writeByte(packPair(type, p[0], p[1]));

	if (firstByte != 0)
		writeByte(packPair(type, p[0], 0));
}

/////////////////////////////////////////////////////////////////////////////////////////

MBinBuffer encodeBigEndian(uint32_t num, size_t len)
{
	MBinBuffer res;
	for (int i = 0; i < len; i++) {
		uint8_t c = num & 0xFF;
		res.append(&c, 1);
		num >>= 8;
	}
	return res;
}
