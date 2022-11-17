/*

WhatsApp plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#define DICT_VERSION 2

#define LIST_EMPTY   0
#define STREAM_END   2
#define DICTIONARY_0 236
#define DICTIONARY_1 237
#define DICTIONARY_2 238
#define DICTIONARY_3 239
#define AD_JID       247
#define LIST_8       248
#define LIST_16      249
#define JID_PAIR     250
#define HEX_8        251
#define BINARY_8     252
#define BINARY_20    253
#define BINARY_32    254
#define NIBBLE_8     255

class WANode // kinda XML
{
	friend class WAReader;
	friend class WAWriter;

	WANode *pParent = nullptr;
	OBJLIST<struct Attr> attrs;
	OBJLIST<WANode> children;

public:
	WANode();
	WANode(const char *pszTitle);
	~WANode();

	void addAttr(const char *pszName, const char *pszValue);
	void addAttr(const char *pszName, int iValue);
	int getAttrInt(const char *pszName) const;
	const char *getAttr(const char *pszName) const;

	CMStringA getBody() const;

	WANode *addChild(const char *pszName);
	WANode *getChild(const char *pszName) const;
	WANode *getFirstChild(void) const;
	const OBJLIST<WANode> &getChildren(void) const
	{	return children;
	}

	void print(CMStringA &dest, int level = 0) const;

	CMStringA title;
	MBinBuffer content;
};

__forceinline WANode &operator<<(WANode &node, const CHAR_PARAM &param)
{
	node.addAttr(param.szName, param.szValue);
	return node;
}

__forceinline WANode &operator<<(WANode &node, const INT_PARAM &param)
{
	node.addAttr(param.szName, param.iValue);
	return node;
}

/////////////////////////////////////////////////////////////////////////////////////////

namespace IQ
{
	enum Type { GET, SET, RESULT };
};

struct WANodeIq : public WANode
{
	WANodeIq(IQ::Type type, const char *pszXmlns = nullptr, const char *pszTo = nullptr);
};

/////////////////////////////////////////////////////////////////////////////////////////

struct XCHILD
{
	const char *name, *value;

	__forceinline XCHILD(const char *_name) :
		name(_name)
	{}
};

__forceinline WANode& operator<<(WANode &node, const XCHILD &child)
{
	node.addChild(child.name);
	return node;
}

/////////////////////////////////////////////////////////////////////////////////////////
// WAReader

class WAReader
{
	const BYTE *m_buf, *m_limit;

	uint32_t  readIntN(int i);
	CMStringA readStringFromChars(int size);

	bool      readAttributes(WANode *node, int count);
	uint32_t  readInt20();
	bool      readList(WANode *pParent, int tag);
	int       readListSize(int tag);
	CMStringA readPacked(int tag);
	CMStringA readString(int tag);

public:
	WAReader(const void *buf, size_t cbLen) :
		m_buf((BYTE*)buf),
		m_limit((BYTE*)buf + cbLen)
	{}

	WANode* readNode();

	__forceinline uint32_t readInt8() { return readIntN(1); }
	__forceinline uint32_t readInt16() { return readIntN(2); }
	__forceinline uint32_t readInt32() { return readIntN(4); }
};

/////////////////////////////////////////////////////////////////////////////////////////
// WAWriter

class WAWriter
{
	__forceinline void writeInt8(int value) { writeIntN(value, 1); }
	__forceinline void writeInt16(int value) { writeIntN(value, 2); }
	__forceinline void writeInt32(int value) { writeIntN(value, 4); }

	void writeByte(uint8_t b);
	void writeIntN(int value, int i);
	void writeInt20(int value);
	void writeLength(int value);
	void writeListSize(int tag);
	void writePacked(const CMStringA &str, int tag);
	void writeString(const char *str);
	bool writeToken(const char *str);

public:
	void writeNode(const WANode *pNode);

	MBinBuffer body;
};

/////////////////////////////////////////////////////////////////////////////////////////
// WAJid

struct WAJid
{
	int device, agent;
	CMStringA user, server;

	WAJid(const char *pszJid, int device = 0);
	WAJid(const char *pszUser, const char *pszServer, int device = 0, int agent = 0);

	CMStringA toString() const;

	bool isUser() const;
	bool isGroup() const;
	bool isBroadcast() const;
	bool isStatusBroadcast() const;
};

/////////////////////////////////////////////////////////////////////////////////////////
// WASendTask

struct WASendTask
{
	WASendTask(const char *jid) :
		payLoad("message"),
		arDest(1)
	{
		__int64 msgId;
		Utils_GetRandom(&msgId, sizeof(msgId));
		if (msgId < 0)
			msgId = -msgId;
		_i64toa(msgId, szMsgId, 10);

		payLoad << CHAR_PARAM("id", szMsgId) << CHAR_PARAM("type", "text") << CHAR_PARAM("to", jid);
	}

	char szMsgId[40];
	WANode payLoad;
	OBJLIST<WAJid> arDest;
	MBinBuffer content;
};

/////////////////////////////////////////////////////////////////////////////////////////
// LT_HASH

struct LT_HASH
{
	LT_HASH()
	{
		init();
	};

	uint16_t hash[64];

	void add(const void *pData, size_t len);
	void sub(const void *pData, size_t len);

	void init()
	{
		SecureZeroMemory(hash, sizeof(hash));
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// functions

void bin2file(const MBinBuffer &buf, const wchar_t *pwszFileName);

void string2file(const std::string &str, const wchar_t *pwszFileName);
CMStringA file2string(const wchar_t *pwszFileName);

CMStringA directPath2url(const char *pszDirectPath);

std::string decodeBinStr(const std::string &buf);
MBinBuffer decodeBufStr(const std::string &buf);

void padBuffer16(MBinBuffer &buf);
MBinBuffer unpadBuffer16(const MBinBuffer &buf);

CMStringA protobuf_c_text_to_string(const ProtobufCMessage *m);

MBinBuffer aesDecrypt(
	const EVP_CIPHER *cipher,
	const uint8_t *key,
	const uint8_t *iv,
	const void *data, size_t dataLen,
	const void *additionalData = 0, size_t additionalLen = 0);

MBinBuffer aesEncrypt(
	const EVP_CIPHER *cipher,
	const uint8_t *key,
	const uint8_t *iv,
	const void *data, size_t dataLen,
	const void *additionalData = 0, size_t additionalLen = 0);

uint32_t decodeBigEndian(const uint8_t *pData, size_t len);

__forceinline uint32_t decodeBigEndian(const ProtobufCBinaryData &buf) { 
	return decodeBigEndian(buf.data, buf.len);
}
__forceinline uint32_t decodeBigEndian(const MBinBuffer &buf) {
	return decodeBigEndian(buf.data(), buf.length());
}

std::string encodeBigEndian(uint32_t num, size_t len = sizeof(uint32_t));

void generateIV(uint8_t *iv, uint32_t &pVar);

unsigned char* HKDF(const EVP_MD *evp_md,
	const unsigned char *salt, size_t salt_len,
	const unsigned char *key, size_t key_len,
	const unsigned char *info, size_t info_len,
	unsigned char *okm, size_t okm_len);
