/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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

bool CSkypeProto::IsOnline()
{
	return m_iStatus > ID_STATUS_OFFLINE && m_hPollingThread;
}

MEVENT CSkypeProto::AddEventToDb(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = this->m_szModuleName;
	dbei.timestamp = timestamp;
	dbei.eventType = type;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	dbei.flags = flags;
	return db_event_add(hContact, &dbei);
}

time_t CSkypeProto::IsoToUnixTime(const TCHAR *stamp)
{
	TCHAR date[9];
	int i, y;

	if (stamp == NULL)
		return 0;

	TCHAR *p = NEWTSTR_ALLOCA(stamp);

	// skip '-' chars
	int si = 0, sj = 0;
	while (true) {
		if (p[si] == _T('-'))
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
	if (_stscanf(p, _T("%d:%d:%d"), &timestamp.tm_hour, &timestamp.tm_min, &timestamp.tm_sec) != 3)
		return (time_t)0;

	timestamp.tm_isdst = 0;	// DST is already present in _timezone below
	time_t t = mktime(&timestamp);

	_tzset();
	t -= _timezone;
	return (t >= 0) ? t : 0;
}

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

char *CSkypeProto::RemoveHtml(const char *text)
{
	std::string new_string = "";
	std::string data = text;

	if (data.find("\x1b\xe3\xac\x8d\x1d") != -1)
		data = "CONVERSATION MEMBERS:" + data.substr(5, data.length() - 5);

	for (std::string::size_type i = 0; i < data.length(); i++)
	{
		if (data.at(i) == '<')
		{
			i = data.find(">", i);
			if (i == std::string::npos)
				break;

			continue;
		}

		if (data.at(i) == '&') {
			std::string::size_type begin = i;
			i = data.find(";", i);
			if (i == std::string::npos) {
				i = begin;
			}
			else {
				std::string entity = data.substr(begin + 1, i - begin - 1);

				bool found = false;
				for (int j = 0; j < SIZEOF(htmlEntities); j++)
				{
					if (!stricmp(entity.c_str(), htmlEntities[j].entity))
					{
						new_string += htmlEntities[j].symbol;
						found = true;
						break;
					}
				}

				if (found)
					continue;
				else
					i = begin;
			}
		}

		new_string += data.at(i);
	}

	return mir_strdup(new_string.c_str());
}

bool CSkypeProto::IsMe(const char *skypeName)
{
	ptrA mySkypeName(getStringA(SKYPE_SETTINGS_ID));
	ptrA SelfEndpointName(getStringA("SelfEndpointName"));
	if (!mir_strcmp(skypeName, mySkypeName) || !mir_strcmp(skypeName, SelfEndpointName))
		return true;
	return false;
}

char *CSkypeProto::MirandaToSkypeStatus(int status)
{
	switch (status)
	{
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
		return /*ID_STATUS_IDLE*/ID_STATUS_AWAY;
	else if (!mir_strcmpi(status, "Busy"))
		return ID_STATUS_DND;
	else 
		return ID_STATUS_OFFLINE;
}

void CSkypeProto::ShowNotification(const TCHAR *caption, const TCHAR *message, int flags, MCONTACT hContact, int type)
{
	if (Miranda_Terminated())
		return;

	if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1))
	{
		POPUPDATAT ppd = { 0 };
		ppd.lchContact = hContact;
		_tcsncpy(ppd.lptzContactName, caption, MAX_CONTACTNAME);
		_tcsncpy(ppd.lptzText, message, MAX_SECONDLINE);
		ppd.lchIcon = Skin_GetIcon("Skype_main");
		if (type == SKYPE_DB_EVENT_TYPE_INCOMING_CALL)
			ppd.PluginWindowProc = PopupDlgProcCall;

		if (!PUAddPopupT(&ppd))
			return;
	}

	MessageBox(NULL, message, caption, MB_OK | flags);
}

LRESULT CSkypeProto::PopupDlgProcCall(HWND hPopup, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CONTEXTMENU:
		PUDeletePopup(hPopup);
		CallService(MODULE"/IncomingCallPP", 0, PUGetContact(hPopup));
		break;
	case WM_COMMAND:
		PUDeletePopup(hPopup);
		CallService(MODULE"/IncomingCallPP", 1, PUGetContact(hPopup));
		break;
	}

	return DefWindowProc(hPopup, uMsg, wParam, lParam);
}

void CSkypeProto::ShowNotification(const TCHAR *message, int flags, MCONTACT hContact)
{
	ShowNotification(_T(MODULE), message, flags, hContact);
}

bool CSkypeProto::IsFileExists(std::tstring path)
{
	return _taccess(path.c_str(), 0) == 0;
}

// url parsing

char *CSkypeProto::ParseUrl(const char *url, const char *token)
{
	const char *start = strstr(url, token);
	if (start == NULL)
		return NULL;
	start = start + mir_strlen(token);
	const char *end = strchr(start, '/');
	if (end == NULL)
		return mir_strdup(start);
	return mir_strndup(start, end - start);
}

char *CSkypeProto::ContactUrlToName(const char *url)
{
	return ParseUrl(url, "/8:");
}

char *CSkypeProto::SelfUrlToName(const char *url)
{
	return ParseUrl(url, "/1:");
}

char *CSkypeProto::ChatUrlToName(const char *url)
{
	return ParseUrl(url, "/19:");
}

char *CSkypeProto::GetServerFromUrl(const char *url)
{
	return ParseUrl(url, "://");
}

INT_PTR CSkypeProto::ParseSkypeUriService(WPARAM, LPARAM lParam)
{
	TCHAR *arg = (TCHAR *)lParam;
	if (arg == NULL)
		return 1;

	// skip leading prefix
	TCHAR szUri[ 1024 ];
	_tcsncpy_s(szUri, arg, _TRUNCATE);
	TCHAR *szJid = _tcschr(szUri, _T(':'));
	if (szJid == NULL)
		return 1;

	// skip //
	for (++szJid; *szJid == _T('/'); ++szJid);

	// empty jid?
	if (!*szJid)
		return 1;

	// command code
	TCHAR *szCommand = szJid;
	szCommand = _tcschr(szCommand, _T('?'));
	if (szCommand)
		*(szCommand++) = 0;

	// parameters
	TCHAR *szSecondParam = szCommand ? _tcschr(szCommand, _T(';')) : NULL;
	if (szSecondParam)
		*(szSecondParam++) = 0;

	MCONTACT hContact = AddContact(_T2A(szJid), true);
	// no command or message command
	if (!szCommand || (szCommand && !_tcsicmp(szCommand, _T("chat")))) 
	{
		CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, NULL);
		return 0;
	}
	else if (szCommand && !_tcsicmp(szCommand, _T("call")))
	{
		NotifyEventHooks(m_hCallHook, (WPARAM)hContact, (LPARAM)0);
		return 0;
	}
	else if (szCommand && !_tcsicmp(szCommand, _T("userinfo"))){ return 0;}

	return 1; /* parse failed */
}

INT_PTR CSkypeProto::GlobalParseSkypeUriService(WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < Accounts.getCount(); i++)
		if (Accounts[i]->IsOnline())
			return Accounts[i]->ParseSkypeUriService(wParam, lParam);

	return 1;
}

LPCTSTR CSkypeProto::ClearText(CMString &result, const TCHAR *message)
{
	BSTR bstrHtml = SysAllocString(message), bstrRes = SysAllocString(_T(""));
	HRESULT hr = TestMarkupServices(bstrHtml, &TestDocumentText, bstrRes);
	if (SUCCEEDED(hr))
		result = bstrRes;
	else
		result = message;
	SysFreeString(bstrHtml);
	SysFreeString(bstrRes);

	return result;
}

HRESULT TestDocumentText(IHTMLDocument3 *pHtmlDoc, BSTR &message)
{
	IHTMLDocument2 *pDoc = NULL;
	IHTMLElement *pElem = NULL;
	BSTR bstrId = SysAllocString(L"test");

	HRESULT hr = pHtmlDoc->QueryInterface(IID_PPV_ARGS(&pDoc));
	if (SUCCEEDED(hr) && pDoc) {
		hr = pDoc->get_body(&pElem);
		if (SUCCEEDED(hr) && pElem) {
			BSTR bstrText = NULL;
			pElem->get_innerText(&bstrText);
			message = SysAllocString(bstrText);
			SysFreeString(bstrText);
			pElem->Release();
		}

		pDoc->Release();
	}

	SysFreeString(bstrId);
	return hr;
}



HRESULT TestMarkupServices(BSTR bstrHtml, MarkupCallback *pCallback, BSTR &message)
{
	IHTMLDocument3 *pHtmlDocRoot = NULL;

	// Create the root document -- a "workspace" for parsing.
	HRESULT hr = CoCreateInstance(CLSID_HTMLDocument, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pHtmlDocRoot));
	if (SUCCEEDED(hr) && pHtmlDocRoot) {
		IPersistStreamInit *pPersistStreamInit = NULL;

		HRESULT hr = pHtmlDocRoot->QueryInterface(IID_PPV_ARGS(&pPersistStreamInit));
		if (SUCCEEDED(hr)) {
			// Initialize the root document to a default state -- ready for parsing.
			pPersistStreamInit->InitNew();

			IMarkupServices *pMarkupServices = NULL;
			hr = pHtmlDocRoot->QueryInterface(IID_PPV_ARGS(&pMarkupServices));
			if (SUCCEEDED(hr)) {
				IMarkupPointer *pMarkupBegin = NULL;
				IMarkupPointer *pMarkupEnd = NULL;

				// These markup pointers indicate the insertion point.
				hr = pMarkupServices->CreateMarkupPointer(&pMarkupBegin);
				if (SUCCEEDED(hr))
					hr = pMarkupServices->CreateMarkupPointer(&pMarkupEnd);

				if (SUCCEEDED(hr) && pMarkupBegin && pMarkupEnd) {
					IMarkupContainer *pMarkupContainer = NULL;

					// Parse the string -- the markup container contains the parsed HTML.
					// Markup pointers are updated to point to begining and end of new container.
					hr = pMarkupServices->ParseString(bstrHtml, 0, &pMarkupContainer, pMarkupBegin, pMarkupEnd);
					if (SUCCEEDED(hr) && pMarkupContainer) {
						IHTMLDocument3 *pHtmlDoc = NULL;

						// Retrieve the document interface to the markup container.
						hr = pMarkupContainer->QueryInterface(IID_PPV_ARGS(&pHtmlDoc));
						if (SUCCEEDED(hr) && pHtmlDoc) {
							// Invoke the user-defined action for this new fragment.
							hr = pCallback(pHtmlDoc, message);

							// Clean up.
							pHtmlDoc->Release();
						}
						pMarkupContainer->Release();
					}
					pMarkupEnd->Release();
				}
				if (pMarkupBegin)
					pMarkupBegin->Release();
				pMarkupServices->Release();
			}
			pPersistStreamInit->Release();
		}
		pHtmlDocRoot->Release();
	}
	return hr;
}