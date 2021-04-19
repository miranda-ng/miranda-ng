/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-21 George Hazan

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

class WAReader
{
	const BYTE *m_buf, *m_limit;

	uint32_t readIntN(int i);
	CMStringA readStringFromChars(int size);

public:
	WAReader(const void *buf, size_t cbLen) :
		m_buf((BYTE*)buf),
		m_limit((BYTE*)buf + cbLen)
	{}

	__forceinline uint32_t readInt8() { return readIntN(1); }
	__forceinline uint32_t readInt16() { return readIntN(2); }
	__forceinline uint32_t readInt32() { return readIntN(4); }

	bool      readAttributes(JSONNode &node, int count);
	uint32_t  readInt20();
	bool      readList(JSONNode &node, int tag);
	int       readListSize(int tag);
	CMStringA readPacked(int tag);
	CMStringA readString(int tag);
	bool      readNode(JSONNode&);
};

/////////////////////////////////////////////////////////////////////////////////////////
// object serialization

#define WAS_STRING 1
#define WAS_BINARY 2
#define WAS_BOOL   3
#define WAS_INT8   4
#define WAS_INT64  5

struct WAS_Field
{
	uint8_t type;
	uint8_t len;
	uint16_t offset;
};

class WAS_Decoder
{
	const BYTE *m_buf, *m_limit;

public:
	WAS_Decoder(const void *pData, size_t cbLen) :
		m_buf((BYTE*)pData),
		m_limit((BYTE*)pData + cbLen)
	{}

	bool read(void *pObj, const WAS_Field *fields, size_t iFieldCount);
};
