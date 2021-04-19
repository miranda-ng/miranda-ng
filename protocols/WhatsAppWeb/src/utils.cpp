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
		Contact_RemoveFromList(hContact);
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

bool WhatsAppProto::decryptBinaryMessage(size_t cbSize, const void *buf, MBinBuffer &res)
{
	if (cbSize <= 32)
		return false;

	// validate message first
	{
		unsigned int md_len = 32;
		BYTE md[32];
		HMAC(EVP_sha256(), mac_key.data(), (int)mac_key.length(), (unsigned char *)buf+32, (int)cbSize-32, md, &md_len);
		if (memcmp(buf, md, sizeof(md))) {
			debugLogA("Message cannot be decrypted, check your keys");
			return false;
		}
	}
	
	// okay, let's decrypt this thing
	{
		BYTE iv[16];
		memcpy(iv, (char*)buf + 32, sizeof(iv));
		res.assign((char*)buf + 48, cbSize - 48);
		res.append((char*)mac_key.data(), 32); // reserve 32 more bytes for temp data

		int dec_len = 0, final_len = 0;
		EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
		EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (BYTE*)enc_key.data(), iv);
		EVP_DecryptUpdate(ctx, (BYTE*)res.data(), &dec_len, (BYTE*)buf + 48, (int)cbSize - 48);
		EVP_DecryptFinal_ex(ctx, (BYTE*)res.data() + dec_len, &final_len);
		EVP_CIPHER_CTX_free(ctx);
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool WAReader::readAttributes(JSONNode &ret, int count)
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

		ret << CHAR_PARAM(name, value);
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

bool WAReader::readList(JSONNode &parent, int tag)
{
	int size = readListSize(tag);
	if (size == -1)
		return false;

	JSONNode list(JSON_ARRAY); list.set_name("$list$");
	for (int i = 0; i < size; i++) {
		JSONNode tmp;
		if (!readNode(tmp))
			return false;
		list << tmp;
	}
	
	parent << list;
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

bool WAReader::readNode(JSONNode &ret)
{
	int listSize = readListSize(readInt8());
	if (listSize == -1)
		return false;

	int descrTag = readInt8();
	if (descrTag == STREAM_END)
		return false;

	CMStringA name = readString(descrTag);
	if (name.IsEmpty())
		return false;
	ret.set_name(name.c_str());

	if (!readAttributes(ret, (listSize-1)>>1))
		return false;

	if ((listSize % 2) == 1)
		return true;

	int size, tag = readInt8();
	switch (tag) {
	case LIST_EMPTY:	case LIST_8:  case LIST_16:
		readList(ret, tag);
		break;

	case BINARY_8:
		size = readInt8();

LBL_Binary:
		if (m_limit - m_buf < size)
			return false;

		ret << CHAR_PARAM("$bin$", ptrA(mir_base64_encode(m_buf, size)));
		m_buf += size;
		break;

	case BINARY_20:
		size = readInt20();
		goto LBL_Binary;

	case BINARY_32:
		size = readInt32();
		goto LBL_Binary;

	default:
		ret << CHAR_PARAM("$str$", readString(tag).c_str());
	}
	
	return true;
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
// Object serialization

bool WAS_Decoder::read(void *pObj, const WAS_Field *fields, size_t iFieldCount)
{
	BYTE *pDest = (BYTE *)pObj;

	for (int i = 0; i < iFieldCount; i++) {
		auto &F = fields[i];
		switch (F.type) {
		case WAS_STRING:
			if (*m_buf++ != 0x0A) return false;
			{
				char **d = (char **)&pDest[F.offset];
				int len = (F.len == 0) ? *m_buf++ : F.len;
				*d = (char *)mir_strndup((char*)m_buf, len);
				m_buf += len;
			}
			break;

		case WAS_BINARY:
			if (*m_buf++ != 0x1A) return false;
			{
				char **d = (char **)&pDest[F.offset];
				int len = (F.len == 0) ? *m_buf++ : F.len;
				*d = (char *)mir_strndup((char*)m_buf, len);
				m_buf += len;
			}
			break;

		case WAS_BOOL:
			if (*m_buf++ != 0x10) return false;
			{
				bool *d = (bool *)&pDest[F.offset];
				*d = *m_buf++;
			}
			break;

		case WAS_INT8:
			if (*m_buf++ != 0x12) return false;
			{
				BYTE *d = (BYTE*)&pDest[F.offset];
				*d = *m_buf++;
			}
			break;

		case WAS_INT64:
			if (*m_buf++ != 0x18) return false;
			{
				__int64 *d = (__int64 *)&pDest[F.offset];
				*d = *(__int64 *)m_buf;
				m_buf += sizeof(__int64);
			}
			break;
		}
	}

	return true;
}
