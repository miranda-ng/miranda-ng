/*
Copyright (c) 2015-24 Miranda NG team (https://miranda-ng.org)

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

#pragma warning(disable:4566)

time_t CSkypeProto::IsoToUnixTime(const std::string &stamp)
{
	char date[9];
	int i, y;

	if (stamp.empty())
		return 0;

	char *p = NEWSTR_ALLOCA(stamp.c_str());

	// skip '-' chars
	int si = 0, sj = 0;
	while (true) {
		if (p[si] == '-')
			si++;
		else if (!(p[sj++] = p[si++]))
			break;
	}

	// Get the date part
	for (i = 0; *p != '\0' && i < 8 && isdigit(*p); p++, i++)
		date[i] = *p;

	// Parse year
	if (i == 6) {
		// 2-digit year (1970-2069)
		y = (date[0] - '0') * 10 + (date[1] - '0');
		if (y < 70) y += 100;
	}
	else if (i == 8) {
		// 4-digit year
		y = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + date[3] - '0';
		y -= 1900;
	}
	else return 0;

	struct tm timestamp;
	timestamp.tm_year = y;

	// Parse month
	timestamp.tm_mon = (date[i - 4] - '0') * 10 + date[i - 3] - '0' - 1;

	// Parse date
	timestamp.tm_mday = (date[i - 2] - '0') * 10 + date[i - 1] - '0';

	// Skip any date/time delimiter
	for (; *p != '\0' && !isdigit(*p); p++);

	// Parse time
	if (sscanf(p, "%d:%d:%d", &timestamp.tm_hour, &timestamp.tm_min, &timestamp.tm_sec) != 3)
		return (time_t)0;

	timestamp.tm_isdst = 0;	// DST is already present in _timezone below
	time_t t = mktime(&timestamp);

	_tzset();
	t -= _timezone;
	return (t >= 0) ? t : 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

struct HtmlEntity
{
	const char *entity;
	const char *symbol;
};

const HtmlEntity htmlEntities[] =
{
	{ "AElig", "\u00C6" },
	{ "Aacute", "\u00C1" },
	{ "Acirc", "\u00C2" },
	{ "Agrave", "\u00C0" },
	{ "Alpha", "\u0391" },
	{ "Aring", "\u00C5" },
	{ "Atilde", "\u00C3" },
	{ "Auml", "\u00C4" },
	{ "Beta", "\u0392" },
	{ "Ccedil", "\u00C7" },
	{ "Chi", "\u03A7" },
	{ "Dagger", "‡" },
	{ "Delta", "\u0394" },
	{ "ETH", "\u00D0" },
	{ "Eacute", "\u00C9" },
	{ "Ecirc", "\u00CA" },
	{ "Egrave", "\u00C8" },
	{ "Epsilon", "\u0395" },
	{ "Eta", "\u0397" },
	{ "Euml", "\u00CB" },
	{ "Gamma", "\u0393" },
	{ "Iacute", "\u00CD" },
	{ "Icirc", "\u00CE" },
	{ "Igrave", "\u00CC" },
	{ "Iota", "\u0399" },
	{ "Iuml", "\u00CF" },
	{ "Kappa", "\u039A" },
	{ "Lambda", "\u039B" },
	{ "Mu", "\u039C" },
	{ "Ntilde", "\u00D1" },
	{ "Nu", "\u039D" },
	{ "OElig", "\u0152" },
	{ "Oacute", "\u00D3" },
	{ "Ocirc", "\u00D4" },
	{ "Ograve", "\u00D2" },
	{ "Omega", "\u03A9" },
	{ "Omicron", "\u039F" },
	{ "Oslash", "\u00D8" },
	{ "Otilde", "\u00D5" },
	{ "Ouml", "\u00D6" },
	{ "Phi", "\u03A6" },
	{ "Pi", "\u03A0" },
	{ "Prime", "\u2033" },
	{ "Psi", "\u03A8" },
	{ "Rho", "\u03A1" },
	{ "Scaron", "Š" },
	{ "Sigma", "Σ" },
	{ "THORN", "Þ" },
	{ "Tau", "Τ" },
	{ "Theta", "Θ" },
	{ "Uacute", "Ú" },
	{ "Ucirc", "Û" },
	{ "Ugrave", "Ù" },
	{ "Upsilon", "Υ" },
	{ "Uuml", "Ü" },
	{ "Xi", "Ξ" },
	{ "Yacute", "Ý" },
	{ "Yuml", "Ÿ" },
	{ "Zeta", "Ζ" },
	{ "aacute", "á" },
	{ "acirc", "â" },
	{ "acute", "´" },
	{ "aelig", "æ" },
	{ "agrave", "à" },
	{ "alefsym", "ℵ" },
	{ "alpha", "α" },
	{ "amp", "&" },
	{ "and", "∧" },
	{ "ang", "∠" },
	{ "apos", "'" },
	{ "aring", "å" },
	{ "asymp", "≈" },
	{ "atilde", "ã" },
	{ "auml", "ä" },
	{ "bdquo", "„" },
	{ "beta", "β" },
	{ "brvbar", "¦" },
	{ "bull", "•" },
	{ "cap", "∩" },
	{ "ccedil", "ç" },
	{ "cedil", "¸" },
	{ "cent", "¢" },
	{ "chi", "χ" },
	{ "circ", "ˆ" },
	{ "clubs", "♣" },
	{ "cong", "≅" },
	{ "copy", "©" },
	{ "crarr", "↵" },
	{ "cup", "∪" },
	{ "curren", "¤" },
	{ "dArr", "⇓" },
	{ "dagger", "†" },
	{ "darr", "↓" },
	{ "deg", "°" },
	{ "delta", "δ" },
	{ "diams", "♦" },
	{ "divide", "÷" },
	{ "eacute", "é" },
	{ "ecirc", "ê" },
	{ "egrave", "è" },
	{ "empty", "∅" },
	{ "emsp", " " },
	{ "ensp", " " },
	{ "epsilon", "ε" },
	{ "equiv", "≡" },
	{ "eta", "η" },
	{ "eth", "ð" },
	{ "euml", "ë" },
	{ "euro", "€" },
	{ "exist", "∃" },
	{ "fnof", "ƒ" },
	{ "forall", "∀" },
	{ "frac12", "½" },
	{ "frac14", "¼" },
	{ "frac34", "¾" },
	{ "frasl", "⁄" },
	{ "gamma", "γ" },
	{ "ge", "≥" },
	{ "gt", ">" },
	{ "hArr", "⇔" },
	{ "harr", "↔" },
	{ "hearts", "♥" },
	{ "hellip", "…" },
	{ "iacute", "í" },
	{ "icirc", "î" },
	{ "iexcl", "¡" },
	{ "igrave", "ì" },
	{ "image", "ℑ" },
	{ "infin", "∞" },
	{ "int", "∫" },
	{ "iota", "ι" },
	{ "iquest", "¿" },
	{ "isin", "∈" },
	{ "iuml", "ï" },
	{ "kappa", "κ" },
	{ "lArr", "⇐" },
	{ "lambda", "λ" },
	{ "lang", "〈" },
	{ "laquo", "«" },
	{ "larr", "←" },
	{ "lceil", "⌈" },
	{ "ldquo", "“" },
	{ "le", "≤" },
	{ "lfloor", "⌊" },
	{ "lowast", "∗" },
	{ "loz", "◊" },
	{ "lrm", "\xE2\x80\x8E" },
	{ "lsaquo", "‹" },
	{ "lsquo", "‘" },
	{ "lt", "<" },
	{ "macr", "¯" },
	{ "mdash", "—" },
	{ "micro", "µ" },
	{ "middot", "·" },
	{ "minus", "−" },
	{ "mu", "μ" },
	{ "nabla", "∇" },
	{ "nbsp", " " },
	{ "ndash", "–" },
	{ "ne", "≠" },
	{ "ni", "∋" },
	{ "not", "¬" },
	{ "notin", "∉" },
	{ "nsub", "⊄" },
	{ "ntilde", "ñ" },
	{ "nu", "ν" },
	{ "oacute", "ó" },
	{ "ocirc", "ô" },
	{ "oelig", "œ" },
	{ "ograve", "ò" },
	{ "oline", "‾" },
	{ "omega", "ω" },
	{ "omicron", "ο" },
	{ "oplus", "⊕" },
	{ "or", "∨" },
	{ "ordf", "ª" },
	{ "ordm", "º" },
	{ "oslash", "ø" },
	{ "otilde", "õ" },
	{ "otimes", "⊗" },
	{ "ouml", "ö" },
	{ "para", "¶" },
	{ "part", "∂" },
	{ "permil", "‰" },
	{ "perp", "⊥" },
	{ "phi", "φ" },
	{ "pi", "π" },
	{ "piv", "ϖ" },
	{ "plusmn", "±" },
	{ "pound", "£" },
	{ "prime", "′" },
	{ "prod", "∏" },
	{ "prop", "∝" },
	{ "psi", "ψ" },
	{ "quot", "\"" },
	{ "rArr", "⇒" },
	{ "radic", "√" },
	{ "rang", "〉" },
	{ "raquo", "»" },
	{ "rarr", "→" },
	{ "rceil", "⌉" },
	{ "rdquo", "”" },
	{ "real", "ℜ" },
	{ "reg", "®" },
	{ "rfloor", "⌋" },
	{ "rho", "ρ" },
	{ "rlm", "\xE2\x80\x8F" },
	{ "rsaquo", "›" },
	{ "rsquo", "’" },
	{ "sbquo", "‚" },
	{ "scaron", "š" },
	{ "sdot", "⋅" },
	{ "sect", "§" },
	{ "shy", "\xC2\xAD" },
	{ "sigma", "σ" },
	{ "sigmaf", "ς" },
	{ "sim", "∼" },
	{ "spades", "♠" },
	{ "sub", "⊂" },
	{ "sube", "⊆" },
	{ "sum", "∑" },
	{ "sup", "⊃" },
	{ "sup1", "¹" },
	{ "sup2", "²" },
	{ "sup3", "³" },
	{ "supe", "⊇" },
	{ "szlig", "ß" },
	{ "tau", "τ" },
	{ "there4", "∴" },
	{ "theta", "θ" },
	{ "thetasym", "ϑ" },
	{ "thinsp", " " },
	{ "thorn", "þ" },
	{ "tilde", "˜" },
	{ "times", "×" },
	{ "trade", "™" },
	{ "uArr", "⇑" },
	{ "uacute", "ú" },
	{ "uarr", "↑" },
	{ "ucirc", "û" },
	{ "ugrave", "ù" },
	{ "uml", "¨" },
	{ "upsih", "ϒ" },
	{ "upsilon", "υ" },
	{ "uuml", "ü" },
	{ "weierp", "℘" },
	{ "xi", "ξ" },
	{ "yacute", "ý" },
	{ "yen", "¥" },
	{ "yuml", "ÿ" },
	{ "zeta", "ζ" },
	{ "zwj", "\xE2\x80\x8D" },
	{ "zwnj", "\xE2\x80\x8C" }
};

CMStringW CSkypeProto::RemoveHtml(const CMStringW &data, bool bCheckSS)
{
	bool inSS = false;
	CMStringW new_string;

	for (int i = 0; i < data.GetLength(); i++) {
		wchar_t c = data[i];
		if (c == '<') {
			if (bCheckSS && !wcsncmp(data.c_str() + i + 1, L"ss ", 3))
				inSS = true;
			else if (!wcsncmp(data.c_str() + i + 1, L"/ss>", 4)) {
				CMStringW wszStatusMsg = data.Mid(i + 5);
				wszStatusMsg.Trim();
				m_wstrMoodMessage = wszStatusMsg;
				inSS = false;
			}

			i = data.Find('>', i);
			if (i == -1)
				break;

			continue;
		}

		// special character
		if (c == '&') {
			int begin = i;
			i = data.Find(';', i);
			if (i == -1)
				i = begin;
			else {
				CMStringW entity = data.Mid(begin + 1, i - begin - 1);

				bool found = false;
				if (entity.GetLength() > 1 && entity[0] == '#') {
					// Numeric replacement
					bool hex = false;
					if (entity[1] == 'x') {
						hex = true;
						entity.Delete(0, 2);
					}
					else entity.Delete(0, 1);

					if (!entity.IsEmpty()) {
						found = true;
						errno = 0;
						unsigned long value = wcstoul(entity, nullptr, hex ? 16 : 10);
						if (errno != 0) { // error with conversion in strtoul, ignore the result
							found = false;
						}
						else if (value <= 127) { // U+0000 .. U+007F
							new_string += (char)value;
						}
						else if (value >= 128 && value <= 2047) { // U+0080 .. U+07FF
							new_string += (char)(192 + (value / 64));
							new_string += (char)(128 + (value % 64));
						}
						else if (value >= 2048 && value <= 65535) { // U+0800 .. U+FFFF
							new_string += (char)(224 + (value / 4096));
							new_string += (char)(128 + ((value / 64) % 64));
							new_string += (char)(128 + (value % 64));
						}
						else {
							new_string += (char)((value >> 24) & 0xFF);
							new_string += (char)((value >> 16) & 0xFF);
							new_string += (char)((value >> 8) & 0xFF);
							new_string += (char)((value) & 0xFF);
						}
					}
				}
				else {
					// Keyword replacement
					CMStringA tmp = entity;
					for (auto &it : htmlEntities) {
						if (!mir_strcmpi(tmp, it.entity)) {
							new_string += it.symbol;
							found = true;
							break;
						}
					}
				}

				if (found)
					continue;
				else
					i = begin;
			}
		}

		if (c == '(' && inSS) {
			int iEnd = data.Find(')', i);
			if (iEnd != -1) {
				CMStringW ss(data.Mid(i + 1, iEnd - i - 1));
				uint32_t code = getMoodIndex(T2Utf(ss));
				if (code != -1)
					m_iMood = code;
				else if (1 == swscanf(ss, L"%x_", &code)) {
					Utf32toUtf16(code, new_string);
					m_wstrMoodEmoji = new_string;
				}

				i = iEnd;
				continue;
			}
		}

		new_string.AppendChar(c);
	}

	return new_string;
}

//////////////////////////////////////////////////////////////////////////////////////////

void Utf32toUtf16(uint32_t c, CMStringW &dest)
{
	if (c < 0x10000)
		dest.AppendChar(c);
	else {
		unsigned int t = c - 0x10000;
		dest.AppendChar((((t << 12) >> 22) + 0xD800));
		dest.AppendChar((((t << 22) >> 22) + 0xDC00));
	}
}

bool is_surrogate(wchar_t uc) { return (uc - 0xd800u) < 2048u; }
bool is_high_surrogate(wchar_t uc) { return (uc & 0xfffffc00) == 0xd800; }
bool is_low_surrogate(wchar_t uc) { return (uc & 0xfffffc00) == 0xdc00; }

uint32_t Utf16toUtf32(const wchar_t *str)
{
	if (!is_surrogate(str[0]))
		return str[0];

	if (is_high_surrogate(str[0]) && is_low_surrogate(str[1]))
		return (str[0] << 10) + str[1] - 0x35fdc00;

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

const char* CSkypeProto::MirandaToSkypeStatus(int status)
{
	switch (status) {
	case ID_STATUS_AWAY:
		return "Away";

	case ID_STATUS_DND:
		return "Busy";

	case ID_STATUS_IDLE:
		return "Idle";

	case ID_STATUS_INVISIBLE:
		return "Hidden";
	}
	return "Online";
}

int CSkypeProto::SkypeToMirandaStatus(const char *status)
{
	if (!mir_strcmpi(status, "Online"))
		return ID_STATUS_ONLINE;
	else if (!mir_strcmpi(status, "Hidden"))
		return ID_STATUS_INVISIBLE;
	else if (!mir_strcmpi(status, "Away"))
		return ID_STATUS_AWAY;
	else if (!mir_strcmpi(status, "Idle"))
		return ID_STATUS_AWAY;
	else if (!mir_strcmpi(status, "Busy"))
		return ID_STATUS_DND;
	else
		return ID_STATUS_OFFLINE;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool CSkypeProto::IsMe(const wchar_t *str)
{
	return (!mir_wstrcmpi(str, Utf2T(m_szMyname)) || !mir_wstrcmp(str, Utf2T(m_szOwnSkypeId)));
}

bool CSkypeProto::IsMe(const char *str)
{
	return (!mir_strcmpi(str, m_szMyname) || !mir_strcmp(str, m_szOwnSkypeId));
}

//////////////////////////////////////////////////////////////////////////////////////////

int64_t CSkypeProto::getLastTime(MCONTACT hContact)
{
	ptrA szLastTime(getStringA(hContact, "LastMsgTime"));
	return (szLastTime) ? _atoi64(szLastTime) : 0;
}

void CSkypeProto::setLastTime(MCONTACT hContact, int64_t iValue)
{
	char buf[100];
	_i64toa(iValue, buf, 10);
	setString(hContact, "LastMsgTime", buf);
}

//////////////////////////////////////////////////////////////////////////////////////////

bool CSkypeProto::IsFileExists(std::wstring path)
{
	return _waccess(path.c_str(), 0) == 0;
}

const char* GetSkypeNick(const char *szSkypeId)
{
	if (auto *p = strchr(szSkypeId, ':'))
		return p + 1;
	return szSkypeId;
}

const wchar_t* GetSkypeNick(const wchar_t *szSkypeId)
{
	if (auto *p = wcsrchr(szSkypeId, ':'))
		return p + 1;
	return szSkypeId;
}

void CSkypeProto::SetString(MCONTACT hContact, const char *pszSetting, const JSONNode &node)
{
	CMStringW str = node.as_mstring();
	if (str.IsEmpty() || str == L"null")
		delSetting(hContact, pszSetting);
	else
		setWString(hContact, pszSetting, str);
}

/////////////////////////////////////////////////////////////////////////////////////////
// url parsing

CMStringA ParseUrl(const char *url, const char *token)
{
	if (url == nullptr)
		return CMStringA();

	auto *start = strstr(url, token);
	if (start == nullptr)
		return CMStringA();

	auto *end = strchr(++start, '/');
	if (end == nullptr)
		return CMStringA(start);
	return CMStringA(start, end - start);
}

CMStringW ParseUrl(const wchar_t *url, const wchar_t *token)
{
	if (url == nullptr)
		return CMStringW();

	auto *start = wcsstr(url, token);
	if (start == nullptr)
		return CMStringW();

	auto *end = wcschr(++start, '/');
	if (end == nullptr)
		return CMStringW(start);
	return CMStringW(start, end - start);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int possibleTypes[] = { 1, 2, 4, 8, 19, 28 };

bool IsPossibleUserType(const char *pszUserId)
{
	int iType = atoi(pszUserId);

	// we don't process 28 and other shit
	for (auto &it : possibleTypes)
		if (it == iType)
			return true;

	return false;
}

CMStringA UrlToSkypeId(const char *url, int *pUserType)
{
	int userType = -1;
	CMStringA szResult;

	if (url != nullptr) {
		for (auto &it : possibleTypes) {
			char tmp[10];
			sprintf_s(tmp, "/%d:", it);
			if (strstr(url, tmp)) {
				userType = it;
				szResult = ParseUrl(url, tmp);
				break;
			}
		}
	}

	if (pUserType)
		*pUserType = userType;

	return szResult;
}

CMStringW UrlToSkypeId(const wchar_t *url, int *pUserType)
{
	int userType = -1;
	CMStringW szResult;

	if (url != nullptr) {
		for (auto &it : possibleTypes) {
			wchar_t tmp[10];
			swprintf_s(tmp, L"/%d:", it);
			if (wcsstr(url, tmp)) {
				userType = it;
				szResult = ParseUrl(url, tmp);
				break;
			}
		}
	}

	if (pUserType)
		*pUserType = userType;

	return szResult;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CSkypeProto::ParseSkypeUriService(WPARAM, LPARAM lParam)
{
	wchar_t *arg = (wchar_t *)lParam;
	if (arg == nullptr)
		return 1;

	// skip leading prefix
	wchar_t szUri[1024];
	wcsncpy_s(szUri, arg, _TRUNCATE);
	wchar_t *szJid = wcschr(szUri, ':');
	if (szJid == nullptr)
		return 1;

	// empty jid?
	if (!*szJid)
		return 1;

	// command code
	wchar_t *szCommand = szJid;
	szCommand = wcschr(szCommand, '?');
	if (szCommand)
		*(szCommand++) = 0;

	// parameters
	wchar_t *szSecondParam = szCommand ? wcschr(szCommand, '&') : nullptr;
	if (szSecondParam)
		*(szSecondParam++) = 0;

	// no command or message command
	if (!szCommand || !mir_wstrcmpi(szCommand, L"chat")) {
		if (szSecondParam) {
			wchar_t *szChatId = wcsstr(szSecondParam, L"id=");
			if (szChatId) {
				szChatId += 5;
				StartChatRoom(szChatId, szChatId);
				return 0;
			}
		}
		MCONTACT hContact = AddContact(_T2A(szJid), nullptr, true);
		CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, NULL);
		return 0;
	}

	if (!mir_wstrcmpi(szCommand, L"call")) {
		MCONTACT hContact = AddContact(_T2A(szJid), nullptr, true);
		NotifyEventHooks(g_hCallEvent, (WPARAM)hContact, (LPARAM)0);
		return 0;
	}

	if (!mir_wstrcmpi(szCommand, L"userinfo"))
		return 0;

	if (!mir_wstrcmpi(szCommand, L"add")) {
		MCONTACT hContact = FindContact(_T2A(szJid));
		if (hContact == NULL) {
			PROTOSEARCHRESULT psr = { 0 };
			psr.cbSize = sizeof(psr);
			psr.id.w = mir_wstrdup(szJid);
			psr.nick.w = mir_wstrdup(szJid);
			psr.flags = PSR_UNICODE;
			Contact::AddBySearch(m_szModuleName, &psr);
		}
		return 0;
	}

	if (!mir_wstrcmpi(szCommand, L"sendfile")) {
		MCONTACT hContact = AddContact(_T2A(szJid), nullptr, true);
		File::Send(hContact);
		return 1;
	}

	if (!mir_wstrcmpi(szCommand, L"voicemail"))
		return 1;

	return 1;
}

INT_PTR CSkypeProto::GlobalParseSkypeUriService(WPARAM wParam, LPARAM lParam)
{
	for (auto &it : CMPlugin::g_arInstances)
		if (it->IsOnline())
			return it->ParseSkypeUriService(wParam, lParam);

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

JsonReply::JsonReply(MHttpResponse *pReply)
{
	if (pReply == nullptr) {
		m_errorCode = 500;
		return;
	}

	m_errorCode = pReply->resultCode;
	if (m_errorCode != 200)
		return;

	m_root = json_parse(pReply->body);
	if (m_root == nullptr) {
		m_errorCode = 500;
		return;
	}

	m_errorCode = (*m_root)["status"]["code"].as_int();
}

JsonReply::~JsonReply()
{
	json_delete(m_root);
}
