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

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// text codecs

namespace OscarText
{

bytes_t toUcs2(const wchar_t *s)
{
	bytes_t out;
	for (; *s; s++) {
		out.push_back(char(*s >> 8));
		out.push_back(char(*s & 0xFF));
	}
	return out;
}

CMStringW fromUcs2(const bytes_t &b)
{
	CMStringW s;
	for (size_t i = 0; i + 1 < b.size(); i += 2)
		s.AppendChar(wchar_t(((uint8_t)b[i] << 8) | (uint8_t)b[i + 1]));
	return s;
}

static bytes_t toCodepage(const wchar_t *s, int cp)
{
	int len = WideCharToMultiByte(cp, 0, s, -1, nullptr, 0, nullptr, nullptr);
	if (len <= 1)
		return bytes_t();

	bytes_t out(len, 0);
	WideCharToMultiByte(cp, 0, s, -1, &out[0], len, nullptr, nullptr);
	out.resize(len - 1);
	return out;
}

static CMStringW fromCodepage(const bytes_t &b, int cp)
{
	if (b.empty())
		return CMStringW();

	int len = MultiByteToWideChar(cp, 0, b.data(), (int)b.size(), nullptr, 0);
	CMStringW out;
	MultiByteToWideChar(cp, 0, b.data(), (int)b.size(), out.GetBuffer(len), len);
	out.ReleaseBuffer(len);
	return out;
}

bytes_t toCp1251(const wchar_t *s) { return toCodepage(s, 1251); }
CMStringW fromCp1251(const bytes_t &b) { return fromCodepage(b, 1251); }
bytes_t toUtf8(const wchar_t *s) { return toCodepage(s, CP_UTF8); }
CMStringW fromUtf8(const bytes_t &b) { return fromCodepage(b, CP_UTF8); }

bool isUtf8(const bytes_t &b)
{
	size_t i = 0, n = b.size();
	while (i < n) {
		uint8_t c = b[i];
		int extra;
		if (c < 0x80) extra = 0;
		else if ((c & 0xE0) == 0xC0) extra = 1;
		else if ((c & 0xF0) == 0xE0) extra = 2;
		else if ((c & 0xF8) == 0xF0) extra = 3;
		else return false;

		for (int k = 1; k <= extra; k++) {
			if (i + k >= n)
				return false;
			if (((uint8_t)b[i + k] & 0xC0) != 0x80)
				return false;
		}
		i += extra + 1;
	}
	return true;
}

bool isUcs2(const bytes_t &b)
{
	if (b.size() & 1)
		return false;

	bool result = true;
	for (size_t i = 0; i < b.size(); i += 2) {
		signed char hi = b[i];
		if (hi > 0 && hi < 9) return true;               // high byte of a non-Latin char
		if (hi == 0 && b[i + 1] != 0) return true;       // ASCII char in UCS-2 form
		if (hi > 32 || hi < 0) result = false;
	}
	return result;
}

CMStringW textOrCp1251(const bytes_t &b)
{
	return isUtf8(b) ? fromUtf8(b) : fromCp1251(b);
}

CMStringW decodeGuess(const bytes_t &in)
{
	bytes_t b = in;
	while (!b.empty() && b.back() == 0)
		b.pop_back();
	if (b.empty())
		return CMStringW();

	if (isUcs2(b)) {
		CMStringW s(fromUcs2(b));
		s.Remove('\r');
		return s;
	}
	return textOrCp1251(b);
}

CMStringW decodeCharset(const bytes_t &b, int charset)
{
	switch (charset) {
	case 2: {
		CMStringW s(fromUcs2(b));
		s.Remove('\r');
		return s;
	}
	case 3:
		return fromCodepage(b, 28591);
	case 0: // "ASCII", which in practice means the sender's ANSI codepage - or UTF-8
		return textOrCp1251(b);
	default:
		return decodeGuess(b);
	}
}

CMStringW cleanup(const CMStringW &str)
{
	CMStringW s(str);
	if (!wcsnicmp(s, L"<HTML>", 6) && s.GetLength() >= 7 && !wcsicmp(s.c_str() + s.GetLength() - 7, L"</HTML>")) {
		CMStringW out;
		bool tag = false;
		for (int i = 0; i < s.GetLength(); i++) {
			wchar_t c = s[i];
			if (c == '<')
				tag = true;
			else if (c == '>') {
				tag = false;
				continue;
			}
			if (!tag)
				out.AppendChar(c);
		}
		s = out;
	}

	if (s.Find('&') != -1) {
		s.Replace(L"&lt;", L"<");
		s.Replace(L"&gt;", L">");
		s.Replace(L"&quot;", L"\"");
		s.Replace(L"&apos;", L"'");
		s.Replace(L"&amp;", L"&");
	}
	s.Replace(L"\r\n", L"\n");

	// KICQ clients send EN SPACE instead of spaces so that Cyrillic text survives the offline
	// store of kicq.ru (see CKicqProto::SendMsg): turn them back into spaces
	s.Replace(0x2002, ' ');
	return s;
}

CMStringA hex(const bytes_t &b)
{
	CMStringA s;
	for (auto c : b)
		s.AppendFormat("%02X", (uint8_t)c);
	return s;
}

bytes_t unhex(const char *s)
{
	bytes_t out;
	for (; s[0] && s[1]; s += 2) {
		char tmp[3] = { s[0], s[1], 0 };
		out.push_back((char)strtoul(tmp, nullptr, 16));
	}
	return out;
}

}

/////////////////////////////////////////////////////////////////////////////////////////
// packet builders

using OscarText::unhex;

static const uint8_t roast[16] = {
	0xF3, 0x26, 0x81, 0xC4, 0x39, 0x86, 0xDB, 0x92, 0x71, 0xA3, 0xB9, 0xE6, 0x53, 0x7A, 0x95, 0x7C
};

static const char *clientString = "ICQ Inc. - Product of ICQ (TM).2000b.4.65.1.3281.85";

// family/version pairs Jasmine announces
static const uint32_t families[] = {
	0x00220001, 0x00010004, 0x00130004, 0x00020001, 0x00030001, 0x00150001, 0x00040001, 0x00060001, 0x00090001, 0x000A0001, 0x000B0001
};

namespace OscarPackets
{

bytes_t flap(int channel, int seq, const bytes_t &payload)
{
	OscarWriter w;
	w.u8(0x2A).u8(channel).u16(seq & 0xFFFF).u16(int(payload.size())).bytes(payload);
	return w.data();
}

bytes_t snacBytes(const OscarSnac &s)
{
	OscarWriter w;
	w.u16(s.family).u16(s.subtype).u16(0).u32(s.reqId).bytes(s.data);
	return w.data();
}

bytes_t xorLogin(const char *uin, const char *password)
{
	bytes_t pw(password, min(strlen(password), size_t(8)));
	for (size_t i = 0; i < pw.size(); i++)
		pw[i] ^= roast[i % 16];

	OscarWriter w;
	w.u32(1);
	w.tlvAscii(0x01, uin);
	w.tlv(0x02, pw);
	w.tlvAscii(0x03, clientString);
	w.tlv(0x16, unhex("010A"));
	w.tlv(0x17, unhex("0004"));
	w.tlv(0x18, unhex("0041"));
	w.tlv(0x19, unhex("0001"));
	w.tlv(0x1A, unhex("0CD1"));
	w.tlvU32(0x14, 0x00000055);
	w.tlvAscii(0x0F, "en");
	w.tlvAscii(0x0E, "us");
	return w.data();
}

bytes_t bosCookie(const bytes_t &cookie)
{
	OscarWriter w;
	w.u32(1).tlv(0x06, cookie);
	return w.data();
}

OscarSnac clientFamilies()
{
	OscarWriter w;
	for (auto it : families)
		w.u32(it);
	return OscarSnac(1, 0x17, 0x17, w.data());
}

OscarSnac ratesRequest() { return OscarSnac(1, 6, 6); }

OscarSnac ackRates(int groups)
{
	OscarWriter w;
	for (int i = 0; i < groups; i++)
		w.u16(i + 1);
	return OscarSnac(1, 8, 8, w.data());
}

OscarSnac reqSelfInfo() { return OscarSnac(1, 0x0E, 0x0E); }

OscarSnac reqSsiRights()
{
	OscarWriter w;
	w.u16(0x0B).u16(2).u16(0x0F);
	return OscarSnac(0x13, 2, 2, w.data());
}

OscarSnac reqLocationRights() { return OscarSnac(2, 2, 2); }

OscarSnac reqBuddyRights()
{
	OscarWriter w;
	w.u16(5).u16(2).u16(3);
	return OscarSnac(3, 2, 2, w.data());
}

OscarSnac reqIcbmParams() { return OscarSnac(4, 4, 4); }
OscarSnac reqPrivacyRights() { return OscarSnac(9, 2, 2); }
OscarSnac requestRoster() { return OscarSnac(0x13, 4, 0); }
OscarSnac rosterActivate() { return OscarSnac(0x13, 7, 7); }

OscarSnac setUserInfo(int major, int minor, int patch)
{
	OscarWriter caps;
	caps.bytes(unhex("4D6972616E6461204B49435120232323"));   // "Miranda KICQ ###"
	caps.bytes(unhex("4a61736d696e6520766572ff"));           // "Jasmine ver" + 0xff, what kicq.ru knows
	caps.u8(major).u8(minor).u8(patch).u8(0);
	caps.bytes(unhex("094600004C7F11D18222444553540000"));
	caps.bytes(unhex(CAP_SERVER_RELAY));
	caps.bytes(unhex(CAP_UTF8));
	caps.bytes(unhex("563FC8090B6F41BD9F79422609DFA2F3"));
	caps.bytes(unhex("0946134D4C7F11D18222444553540000"));   // typing notifications
	OscarWriter w;
	w.tlv(5, caps);
	return OscarSnac(2, 4, 4, w.data());
}

OscarSnac setIcbmParams()
{
	OscarWriter w;
	w.u16(0).u32(1803).u16(8000).u16(999).u16(999).u16(0).u16(0);
	return OscarSnac(4, 2, 2, w.data());
}

OscarSnac clientReady()
{
	OscarWriter w;
	for (auto it : families)
		w.u32(it).u32(0x0110164F);
	return OscarSnac(1, 2, 0x17, w.data());
}

OscarSnac setDcInfo(int status, int flags, int protoVersion)
{
	OscarWriter w;
	w.tlv(6, OscarWriter().u16(flags).u16(status));
	w.tlv(8, OscarWriter().u16(0));
	OscarWriter dc;
	dc.u16(0).u16(0).u32(0).u8(4).u16(protoVersion).u16(0).u16(0).u32(0).u32(0).u32(0).u32(0).u32(0).u16(0);
	w.tlv(0x0C, dc);
	w.tlv(0x1F, OscarWriter().u16(0));
	return OscarSnac(1, 0x1E, 0x1E, w.data());
}

OscarSnac setStatus(int status, int flags)
{
	OscarWriter w;
	w.u16(6).u16(4).u16(flags).u16(status);
	return OscarSnac(1, 0x1E, 0x1E, w.data());
}

OscarSnac setAwayText(const wchar_t *text)
{
	CMStringW away(text);
	if (away.GetLength() > 253)
		away = away.Left(249) + L" ...";

	OscarWriter inner;
	if (!away.IsEmpty()) {
		bytes_t raw = OscarText::toUtf8(away);
		inner.u16(2).u8(4).u8(int(raw.size()) + 2).u16(int(raw.size())).bytes(raw).u16(0);
		inner.u16(0x0E).u16(8).ascii("icqmood5");
	}
	else inner.u16(2).u8(0).u8(0).u16(0x0E).u16(0);

	OscarWriter w;
	w.tlv(0x1D, inner);
	return OscarSnac(1, 0x1E, 0x1E, w.data());
}

OscarSnac offlineMsgsRequest(uint32_t uin, int seq)
{
	OscarWriter inner;
	inner.u16(0x0800).u32le(uin).u16(0x3C00).u16(seq + 1);
	OscarWriter w;
	w.tlv(1, inner);
	return OscarSnac(0x15, 2, REQ_OFFLINE_MSGS, w.data());
}

OscarSnac deleteOfflineMsgs(uint32_t uin, int seq)
{
	OscarWriter inner;
	inner.u16(0x0800).u32le(uin).u16(0x3E00).u16(seq + 1);
	OscarWriter w;
	w.tlv(1, inner);
	return OscarSnac(0x15, 2, 0, w.data());
}

OscarSnac anotherOfflineMsgsRequest() { return OscarSnac(4, 0x10, 262431); }

OscarSnac userInfoRequest(uint32_t myUin, uint32_t uin, int metaSeq, uint32_t reqId)
{
	OscarWriter inner;
	inner.u16le(14).u32le(myUin).u16le(0x07D0).u16le(metaSeq).u16le(0x04B2).u32le(uin);
	OscarWriter w;
	w.tlv(1, inner);
	return OscarSnac(0x15, 2, reqId, w.data());
}

OscarSnac messageChannel1(const char *receiver, const bytes_t &text, int charset, const bytes_t &cookie, uint32_t reqId)
{
	OscarWriter main;
	main.bytes(cookie).u16(1).ascii8(receiver);
	OscarWriter tlv2;
	tlv2.u8(5).u8(1).u16(2).u16(0x0106);                                         // features fragment
	tlv2.u8(1).u8(1).u16(int(text.size()) + 4).u16(charset).u16(0).bytes(text);  // text fragment
	main.tlv(2, tlv2);
	main.u16(6).u16(0);                                                          // store offline
	main.u16(3).u16(0);                                                          // request server ack
	return OscarSnac(4, 6, reqId, main.data());
}

OscarSnac messageChannel2(const char *receiver, const wchar_t *text, bool utf8, const bytes_t &cookie, uint32_t reqId)
{
	OscarWriter main;
	main.bytes(cookie).u16(2).ascii8(receiver);
	OscarWriter tlv5;
	tlv5.u16(0).bytes(cookie);
	tlv5.bytes(unhex("094613494C7F11D1822244455354000000"));
	tlv5.bytes(unhex("0A00020001000F0000"));
	OscarWriter ext;
	ext.bytes(unhex("1B000A00000000000000000000000000000000000000030000000000000E000000000000000000000000000000010000000100"));
	bytes_t msg = utf8 ? OscarText::toUtf8(text) : OscarText::toCp1251(text);
	ext.u16le(int(msg.size()) + 1).bytes(msg).u8(0);
	ext.u32(0).u32(0xFFFFFF00);
	if (utf8)
		ext.u32le(38).ascii("{0946134E-4C7F-11D1-8222-444553540000}");
	tlv5.tlv(0x2711, ext);
	main.tlv(5, tlv5);
	main.u16(3).u16(0);
	return OscarSnac(4, 6, reqId, main.data());
}

OscarSnac messageAck(const bytes_t &cookie, const char *receiver)
{
	OscarWriter w;
	if (cookie.size() == 8)
		w.bytes(cookie);
	else
		w.u32(0).u32(0);
	w.u16(2).ascii8(receiver).u16(3);
	w.bytes(unhex("1B00090000000000000000000000000000000000000001000000008CBE0E008CBE00000000000000000000000001000000000001000000000000FFFFFF00"));
	return OscarSnac(4, 0x0B, 0x0B, w.data());
}

OscarSnac typingNotify(const char *uin, int type)
{
	OscarWriter w;
	w.u32(0).u32(0).u16(1).ascii8(uin).u16(type);
	return OscarSnac(4, 0x14, 0x14, w.data());
}

OscarSnac keepAliveProbe()
{
	OscarWriter w;
	w.u8(9);
	return OscarSnac(4, 6, 0x0F, w.data());
}

OscarSnac ssiEditStart() { return OscarSnac(0x13, 0x11, 0x11); }
OscarSnac ssiEditEnd() { return OscarSnac(0x13, 0x12, 0x12); }

static bytes_t contactItem(const char *uin, const wchar_t *nick, int groupId, int ssiId, bool awaitingAuth)
{
	OscarWriter extra;
	extra.tlv(0x131, OscarText::toUtf8(nick));
	if (awaitingAuth)
		extra.tlv(0x66, bytes_t());

	OscarWriter w;
	w.ascii16(uin).u16(groupId).u16(ssiId).u16(0).u16(extra.size()).bytes(extra.data());
	return w.data();
}

OscarSnac addContact(const char *uin, const wchar_t *nick, int groupId, int ssiId, bool awaitingAuth)
{
	return OscarSnac(0x13, 8, REQ_ADD_CONTACT, contactItem(uin, nick, groupId, ssiId, awaitingAuth));
}

OscarSnac deleteContact(const char *uin, int groupId, int ssiId)
{
	OscarWriter w;
	w.ascii16(uin).u16(groupId).u16(ssiId).u16(0).u16(0);
	return OscarSnac(0x13, 0x0A, REQ_DEL_CONTACT, w.data());
}

OscarSnac renameContact(const char *uin, const wchar_t *nick, int groupId, int ssiId, bool awaitingAuth)
{
	return OscarSnac(0x13, 9, REQ_RENAME, contactItem(uin, nick, groupId, ssiId, awaitingAuth));
}

OscarSnac addGroup(const wchar_t *name, int groupId)
{
	bytes_t raw = OscarText::toUtf8(name);
	OscarWriter w;
	w.u16(int(raw.size())).bytes(raw).u16(groupId).u16(0).u16(1).u16(0);
	return OscarSnac(0x13, 8, REQ_ADD_GROUP, w.data());
}

OscarSnac updateGroup(const wchar_t *name, int groupId, const std::vector<int> &contactIds)
{
	bytes_t raw = OscarText::toUtf8(name);
	OscarWriter w;
	w.u16(int(raw.size())).bytes(raw).u16(groupId).u16(0).u16(1);
	if (!contactIds.empty()) {
		int n = int(contactIds.size());
		w.u16(n * 2 + 4).u16(0xC8).u16(n * 2);
		for (auto it : contactIds)
			w.u16(it);
	}
	else w.u16(0);
	return OscarSnac(0x13, 9, REQ_UPDATE_GROUP, w.data());
}

OscarSnac authRequest(const char *uin, const wchar_t *reason)
{
	bytes_t r = OscarText::toUtf8(reason ? reason : L"");
	OscarWriter w;
	w.ascii8(uin).u16(int(r.size())).bytes(r).u16(0);
	return OscarSnac(0x13, 0x18, 0x18, w.data());
}

OscarSnac futureAuthGrant(const char *uin)
{
	OscarWriter w;
	w.ascii8(uin).u32(0);
	return OscarSnac(0x13, 0x14, 0x14, w.data());
}

OscarSnac authReply(const char *uin, bool granted)
{
	OscarWriter w;
	w.ascii8(uin).u8(granted ? 1 : 0).u32(0);
	return OscarSnac(0x13, 0x1A, 0x1A, w.data());
}

}
