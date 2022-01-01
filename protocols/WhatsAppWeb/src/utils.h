/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#define LIST_EMPTY   0
#define STREAM_END   2
#define DICTIONARY_0 236
#define DICTIONARY_1 237
#define DICTIONARY_2 238
#define DICTIONARY_3 239
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
	friend class WAWriter;

	struct Attr
	{
		Attr(const char *pszName, const char *pszValue) :
			name(pszName),
			value(pszValue)
		{}

		CMStringA name, value;
	};

	std::list<Attr*> attrs;

public:
	WANode();
	~WANode();

	void addAttr(const char *pszName, const char *pszValue);
	CMStringA getAttr(const char *pszFieldValue) const;

	void print(CMStringA &dest, int level = 0) const;

	CMStringA title;
	MBinBuffer content;
	std::list<WANode*> children;
};

class WAReader
{
	const BYTE *m_buf, *m_limit;

	uint32_t readIntN(int i);
	CMStringA readStringFromChars(int size);

	__forceinline uint32_t readInt8() { return readIntN(1); }
	__forceinline uint32_t readInt16() { return readIntN(2); }
	__forceinline uint32_t readInt32() { return readIntN(4); }

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
};

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
	void writePacked(const CMStringA &str);
	void writeString(const char *str, bool bRaw = false);
	bool writeToken(const char *str);

public:
	void writeNode(const WANode *pNode);

	MBinBuffer body;
};
