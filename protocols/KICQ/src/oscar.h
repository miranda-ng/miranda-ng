/*
Copyright (C) 2026 Miranda NG team

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

// The OSCAR byte layer, ported from JasmineKICQ (the Symbian ICQ client for kicq.ru):
// a writer and a bounds-checked reader over std::string, TLV lists, text codecs, and the
// builders of every packet the client sends. The builders reproduce byte for byte what
// Jasmine IM sends to kicq.ru - a third party OSCAR server - so their framing is the
// specification here, not the AOL documents.

#pragma once

typedef std::string bytes_t;

/////////////////////////////////////////////////////////////////////////////////////////

class OscarWriter
{
	bytes_t m_d;

public:
	OscarWriter& u8(int v) { m_d.push_back(char(v)); return *this; }
	OscarWriter& u16(int v) { m_d.push_back(char(v >> 8)); m_d.push_back(char(v)); return *this; }
	OscarWriter& u16le(int v) { m_d.push_back(char(v)); m_d.push_back(char(v >> 8)); return *this; }
	OscarWriter& u32(uint32_t v) { u16(v >> 16); u16(v & 0xFFFF); return *this; }
	OscarWriter& u32le(uint32_t v) { u16le(v & 0xFFFF); u16le(v >> 16); return *this; }
	OscarWriter& bytes(const bytes_t &b) { m_d.append(b); return *this; }
	OscarWriter& ascii(const char *s) { m_d.append(s); return *this; }
	// one-byte length followed by the ASCII string (UINs in ICBM packets)
	OscarWriter& ascii8(const char *s) { size_t n = strlen(s); u8(int(n)); m_d.append(s, n); return *this; }
	// two-byte length followed by the string (SSI item names)
	OscarWriter& ascii16(const char *s) { size_t n = strlen(s); u16(int(n)); m_d.append(s, n); return *this; }
	OscarWriter& tlv(int type, const bytes_t &data) { u16(type); u16(int(data.size())); m_d.append(data); return *this; }
	OscarWriter& tlv(int type, const OscarWriter &w) { return tlv(type, w.data()); }
	OscarWriter& tlvU32(int type, uint32_t v) { return tlv(type, OscarWriter().u32(v).data()); }
	OscarWriter& tlvAscii(int type, const char *s) { return tlv(type, bytes_t(s)); }

	const bytes_t& data() const { return m_d; }
	int size() const { return int(m_d.size()); }
};

class OscarReader
{
	bytes_t m_d;
	int m_pos;

public:
	explicit OscarReader(const bytes_t &d, int pos = 0) : m_d(d), m_pos(pos) {}

	int remaining() const { return int(m_d.size()) - m_pos; }
	bool atEnd() const { return m_pos >= (int)m_d.size(); }
	void skip(int n) { m_pos = min((int)m_d.size(), m_pos + max(0, n)); }

	int u8() { return remaining() >= 1 ? (uint8_t)m_d[m_pos++] : 0; }
	int u16() { int a = u8(); int b = u8(); return (a << 8) | b; }
	int u16le() { int a = u8(); int b = u8(); return a | (b << 8); }
	uint32_t u32() { uint32_t a = u16(); uint32_t b = u16(); return (a << 16) | b; }
	uint32_t u32le() { uint32_t a = u16le(); uint32_t b = u16le(); return a | (b << 16); }
	bytes_t bytes(int n) { n = max(0, min(n, remaining())); bytes_t r = m_d.substr(m_pos, n); m_pos += n; return r; }
	bytes_t ascii8() { return bytes(u8()); }
	bytes_t rest() { return bytes(remaining()); }
};

struct OscarTlv
{
	int type;
	bytes_t data;

	int u16() const { return OscarReader(data).u16(); }
	uint32_t u32() const { return OscarReader(data).u32(); }
};

class OscarTlvList
{
	std::vector<OscarTlv> m_list;

public:
	// reads TLVs until the reader is exhausted, or at most `count` of them when count >= 0
	explicit OscarTlvList(OscarReader &r, int count = -1)
	{
		while (r.remaining() >= 4 && count != 0) {
			int type = r.u16();
			int len = r.u16();
			m_list.push_back({ type, r.bytes(len) });
			if (count > 0)
				count--;
		}
	}

	explicit OscarTlvList(const bytes_t &d)
	{
		OscarReader r(d);
		*this = OscarTlvList(r);
	}

	const OscarTlv* find(int type) const
	{
		for (auto &it : m_list)
			if (it.type == type)
				return &it;
		return nullptr;
	}

	bool has(int type) const { return find(type) != nullptr; }
};

/////////////////////////////////////////////////////////////////////////////////////////
// text codecs

namespace OscarText
{
	bytes_t toUcs2(const wchar_t *s);
	CMStringW fromUcs2(const bytes_t &b);
	bytes_t toCp1251(const wchar_t *s);
	CMStringW fromCp1251(const bytes_t &b);
	bytes_t toUtf8(const wchar_t *s);
	CMStringW fromUtf8(const bytes_t &b);

	bool isUtf8(const bytes_t &b);
	bool isUcs2(const bytes_t &b);

	// UTF-8 when the bytes are valid UTF-8, CP1251 otherwise
	CMStringW textOrCp1251(const bytes_t &b);
	// UCS-2BE, UTF-8 or CP1251 - whichever the bytes look like
	CMStringW decodeGuess(const bytes_t &b);
	// a channel 1 text fragment by its charset id
	CMStringW decodeCharset(const bytes_t &b, int charset);
	// <HTML>...</HTML> bodies of old clients lose their tags, entities are unescaped
	CMStringW cleanup(const CMStringW &s);

	CMStringA hex(const bytes_t &b);
	bytes_t unhex(const char *s);
}

/////////////////////////////////////////////////////////////////////////////////////////
// a SNAC ready to be framed

struct OscarSnac
{
	OscarSnac(int f, int s, uint32_t id, const bytes_t &d = bytes_t()) :
		family(f), subtype(s), reqId(id), data(d)
	{}

	int family, subtype;
	uint32_t reqId;
	bytes_t data;
};

// status words as they travel in TLV 0x06 (low 16 bits)
enum
{
	ICQ_STATUS_ONLINE = 0x0000,
	ICQ_STATUS_AWAY = 0x0001,
	ICQ_STATUS_DND = 0x0002,
	ICQ_STATUS_NA = 0x0004,
	ICQ_STATUS_OCCUPIED = 0x0010,
	ICQ_STATUS_FFC = 0x0020,
	ICQ_STATUS_INVISIBLE = 0x0100,
};

#define CAP_SERVER_RELAY "094613494C7F11D18222444553540000"
#define CAP_UTF8         "0946134E4C7F11D18222444553540000"
#define CAP_FILE         "094613434C7F11D18222444553540000"

// request ids the SSI results come back with (Jasmine's values)
enum
{
	REQ_RENAME = 131075,
	REQ_ADD_CONTACT = 131080,
	REQ_DEL_CONTACT = 131082,
	REQ_UPDATE_GROUP = 131104,
	REQ_DEL_GROUP = 131105,
	REQ_ADD_GROUP = 131106,

	REQ_OFFLINE_MSGS = 64017
};

namespace OscarPackets
{
	bytes_t flap(int channel, int seq, const bytes_t &payload);
	bytes_t snacBytes(const OscarSnac &s);

	// login (FLAP channel 1 / 4): the "roasted" XOR login cuts the password to 8 characters
	bytes_t xorLogin(const char *uin, const char *password);
	bytes_t bosCookie(const bytes_t &cookie);

	// session setup
	OscarSnac clientFamilies();
	OscarSnac ratesRequest();
	OscarSnac ackRates(int groups);
	OscarSnac reqSelfInfo();
	OscarSnac reqSsiRights();
	OscarSnac reqLocationRights();
	OscarSnac reqBuddyRights();
	OscarSnac reqIcbmParams();
	OscarSnac reqPrivacyRights();
	OscarSnac requestRoster();
	OscarSnac rosterActivate();
	OscarSnac setUserInfo(int major, int minor, int patch);
	OscarSnac setIcbmParams();
	OscarSnac clientReady();
	OscarSnac setDcInfo(int status, int flags, int protoVersion);
	OscarSnac setStatus(int status, int flags);
	OscarSnac setAwayText(const wchar_t *text);

	// offline messages and user details (old ICQ "meta" family 0x15)
	OscarSnac offlineMsgsRequest(uint32_t uin, int seq);
	OscarSnac deleteOfflineMsgs(uint32_t uin, int seq);
	OscarSnac anotherOfflineMsgsRequest();
	OscarSnac userInfoRequest(uint32_t myUin, uint32_t uin, int metaSeq, uint32_t reqId);

	// messaging
	//   charset 2: UCS-2BE; charset 0: 8-bit bytes as given (CP1251 or UTF-8)
	OscarSnac messageChannel1(const char *receiver, const bytes_t &text, int charset, const bytes_t &cookie, uint32_t reqId);
	// channel 2 (server relay), UTF-8 or CP1251
	OscarSnac messageChannel2(const char *receiver, const wchar_t *text, bool utf8, const bytes_t &cookie, uint32_t reqId);
	OscarSnac messageAck(const bytes_t &cookie, const char *receiver);
	OscarSnac typingNotify(const char *uin, int type);
	// Jasmine's keep-alive: a deliberately malformed ICBM that the server always answers
	// with an error, which proves the link is alive
	OscarSnac keepAliveProbe();

	// roster (SSI, family 0x13)
	OscarSnac ssiEditStart();
	OscarSnac ssiEditEnd();
	OscarSnac addContact(const char *uin, const wchar_t *nick, int groupId, int ssiId, bool awaitingAuth);
	OscarSnac deleteContact(const char *uin, int groupId, int ssiId);
	OscarSnac renameContact(const char *uin, const wchar_t *nick, int groupId, int ssiId, bool awaitingAuth);
	OscarSnac addGroup(const wchar_t *name, int groupId);
	OscarSnac updateGroup(const wchar_t *name, int groupId, const std::vector<int> &contactIds);
	OscarSnac authRequest(const char *uin, const wchar_t *reason);
	OscarSnac futureAuthGrant(const char *uin);
	OscarSnac authReply(const char *uin, bool granted);
}
