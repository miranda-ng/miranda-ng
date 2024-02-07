/*

WhatsApp plugin for Miranda NG
Copyright © 2019-24 George Hazan

*/

#include "stdafx.h"
#include "dicts.h"

struct Attr
{
	Attr(const char *pszName, const char *pszValue) :
		name(pszName),
		value(pszValue)
	{}

	Attr(const char *pszName, int iValue) :
		name(pszName),
		value(FORMAT, "%d", iValue)
	{}

	CMStringA name, value;
};

/////////////////////////////////////////////////////////////////////////////////////////
// WANodeIq members

WANodeIq::WANodeIq(IQ::Type type, const char *pszXmlns, const char *pszTo) :
	WANode("iq")
{
	switch (type) {
	case IQ::GET: addAttr("type", "get"); break;
	case IQ::SET: addAttr("type", "set"); break;
	case IQ::RESULT: addAttr("type", "result"); break;
	}

	if (pszXmlns)
		addAttr("xmlns", pszXmlns);

	addAttr("to", pszTo ? pszTo : S_WHATSAPP_NET);
}

/////////////////////////////////////////////////////////////////////////////////////////
// WANode members

WANode::WANode() :
	attrs(1),
	children(1)
{}

WANode::WANode(const char *pszTitle) :
	attrs(1),
	children(1),
	title(pszTitle)
{}

WANode::~WANode()
{
}

const char *WANode::getAttr(const char *pszName) const
{
	if (this != nullptr)
		for (auto &p : attrs)
			if (p->name == pszName)
				return p->value.c_str();

	return nullptr;
}

int WANode::getAttrInt(const char *pszName) const
{
	if (this != nullptr)
		for (auto &p : attrs)
			if (p->name == pszName)
				return atoi(p->value.c_str());

	return 0;
}

void WANode::addAttr(const char *pszName, const char *pszValue)
{
	attrs.insert(new Attr(pszName, pszValue));
}

void WANode::addAttr(const char *pszName, int iValue)
{
	attrs.insert(new Attr(pszName, iValue));
}

CMStringA WANode::getBody() const
{
	return CMStringA((char *)content.data(), (int)content.length());
}

WANode *WANode::addChild(const char *pszName)
{
	auto *pNew = new WANode(pszName);
	pNew->pParent = this;
	children.insert(pNew);
	return pNew;
}

WANode* WANode::getChild(const char *pszName) const
{
	if (this == nullptr)
		return nullptr;

	for (auto &it : children)
		if (it->title == pszName)
			return it;

	return nullptr;
}

WANode* WANode::getFirstChild(void) const
{
	return (children.getCount()) ? &children[0] : nullptr;
}

void WANode::print(CMStringA &dest, int level) const
{
	for (int i = 0; i < level; i++)
		dest.Append("  ");

	dest.AppendFormat("<%s ", title.c_str());
	for (auto &p : attrs)
		dest.AppendFormat("%s=\"%s\" ", p->name.c_str(), p->value.c_str());
	dest.Truncate(dest.GetLength() - 1);

	if (content.isEmpty() && !children.getCount()) {
		dest.Append("/>\n");
		return;
	}

	dest.Append(">");
	if (!content.isEmpty()) {
		ptrA tmp((char *)mir_alloc(content.length() * 2 + 1));
		bin2hex(content.data(), content.length(), tmp);
		dest.AppendFormat("%s", tmp.get());
	}

	if (children.getCount()) {
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
		pParent->children.insert(pNew);
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

WANode *WAReader::readNode()
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

	if (!readAttributes(ret.get(), (listSize - 1) >> 1))
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

		ret->content.assign((void *)m_buf, size);
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

CMStringA WAReader::readString(int tag)
{
	if (tag >= 1 && tag < _countof(SingleByteTokens))
		return SingleByteTokens[tag];

	int idx;
	switch (tag) {
	case DICTIONARY_0:
		idx = readInt8();
		return (idx < _countof(dict0)) ? dict0[idx] : "";

	case DICTIONARY_1:
		idx = readInt8();
		return (idx < _countof(dict1)) ? dict1[idx] : "";

	case DICTIONARY_2:
		idx = readInt8();
		return (idx < _countof(dict2)) ? dict2[idx] : "";

	case DICTIONARY_3:
		idx = readInt8();
		return (idx < _countof(dict3)) ? dict3[idx] : "";

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

	case AD_JID:
		{
			int agent = readInt8();
			int device = readInt8();
			WAJid jid(readString(readInt8()), "s.whatsapp.net", device, agent);
			return jid.toString();
		}

	case JID_PAIR:
		CMStringA s1 = readString(readInt8());
		CMStringA s2 = readString(readInt8());
		if (s1.IsEmpty() && s2.IsEmpty())
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

	CMStringA ret((char *)m_buf, size);
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
	// we never send zipped content
	if (pNode->pParent == nullptr)
		writeByte(0);

	int numAttrs = (int)pNode->attrs.getCount();
	int hasContent = pNode->content.length() != 0 || pNode->children.getCount() != 0;
	writeListSize(2 * numAttrs + 1 + hasContent);

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
	else if (pNode->children.getCount()) {
		writeListSize(pNode->children.getCount());
		for (auto &it : pNode->children)
			writeNode(it);
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
	if (c >= '0' && c <= '9')
		return c - '0';

	if (c >= 'A' && c <= 'F')
		return 10 + c - 'A';

	if (c >= 'a' && c <= 'f')
		return 10 + c - 'a';

	if (c == 0)
		return 15;

	return -1;
}

static BYTE packPair(int type, char c1, char c2)
{
	BYTE b1 = (type == NIBBLE_8) ? packNibble(c1) : packHex(c1);
	BYTE b2 = (type == NIBBLE_8) ? packNibble(c2) : packHex(c2);
	return (b1 << 4) + b2;
}

static bool isNibble(const CMStringA &str)
{
	return strspn(str, "0123456789-.") == str.GetLength();
}

static bool isHex(const CMStringA &str)
{
	return strspn(str, "0123456789abcdefABCDEF") == str.GetLength();
}

void WAWriter::writePacked(const CMStringA &str, int tag)
{
	if (str.GetLength() > 254)
		return;

	writeByte(tag);

	int len = str.GetLength() / 2;
	BYTE firstByte = (str.GetLength() % 2) == 0 ? 0 : 0x81;
	writeByte(firstByte + len);

	const char *p = str;
	for (int i = 0; i < len; i++, p += 2)
		writeByte(packPair(tag, p[0], p[1]));

	if (firstByte != 0)
		writeByte(packPair(tag, p[0], 0));
}

void WAWriter::writeString(const char *str)
{
	if (writeToken(str))
		return;

	auto *pszDelimiter = strchr(str, '@');
	if (pszDelimiter) {
		WAJid jid(str);
		if (jid.device || jid.agent) {
			writeByte(AD_JID);
			writeByte(jid.agent);
			writeByte(jid.device);
			writeString(jid.user);
		}
		else {
			writeByte(JID_PAIR);

			if (jid.user.IsEmpty()) // empty user
				writeByte(LIST_EMPTY);
			else
				writeString(jid.user);

			writeString(jid.server);
		}
	}
	else {
		CMStringA buf(str);
		if (isNibble(buf))
			writePacked(buf, NIBBLE_8);
		else if (isHex(buf))
			writePacked(buf, HEX_8);
		else {
			writeLength(buf.GetLength());
			body.append(buf, buf.GetLength());
		}
	}
}
