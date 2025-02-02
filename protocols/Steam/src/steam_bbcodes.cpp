/*
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

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

void EncodeBbcodes(SESSION_INFO *si, CMStringW &wszText)
{
	int idx = wszText.Find(':');
	if (idx != -1) {
		CMStringW wszNick(wszText.Left(idx));
		for (auto &it : si->getUserList()) {
			if (wszNick == it->pszNick) {
				wszText.Delete(0, idx + 1);

				CMStringW wszReplace(FORMAT, L"[mention=%lld]@%s[/mention]", SteamIdToAccountId(_wtoi64(it->pszUID)), it->pszNick);
				wszText = wszReplace + wszText;
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

struct BBCode
{
	CMStringA szTag, szValue, szBody;
	std::map<CMStringA, CMStringA> attrs;
};

static bool p2str(CMStringA &str, const char *&text, const char *subStr)
{
	if (auto *p = strpbrk(text, subStr)) {
		str.Append(text, p - text);
		text = p;
		return true;
	}

	return false;
}

int parseBbcode(const char *p, BBCode &ret)
{
	auto *pSave = p;

	while (isalpha(*p))
		ret.szTag.AppendChar(*p++);

	// [tag=value][/tag]
	if (*p == '=') {
		if (!p2str(ret.szValue, p, "]"))
			return -1;
	}
	// [tag attr=value][/tag]
	else if (*p == ' ') {
		p++;
		while (*p != ']') {
			CMStringA szName, szValue;
			if (!p2str(szName, p, "="))
				return -1;
			
			if (*++p == '\"') {
				auto *p1 = strchr(++p, '\"');
				if (p1 == nullptr)
					return -1;

				szValue.Append(p, p1 - p);
				p = p1 + 1;
			}
			else {
				if (!p2str(szValue, p, " ]"))
					return -1;
			}

			ret.attrs[szName] = szValue;
			if (*p == ' ')
				p++;
		}
	}
	// [tag][/tag]
	else if (*p != ']')
		return -1;

	p++; // skip first ]

	CMStringA szClose = "[/" + ret.szTag + "]";
	auto *pEnd = strstr(p, szClose);
	if (pEnd == nullptr)
		return -1;

	if (pEnd != p)
		ret.szValue.Append(p, pEnd - p);

	return pEnd + szClose.GetLength() - pSave;
}

void CSteamProto::DecodeBbcodes(SESSION_INFO *si, CMStringA &szText)
{
	for (int idx = szText.Find('['); idx != -1; idx = szText.Find('[', idx + 1)) {
		BBCode code;
		int iLength = parseBbcode(szText.c_str() + idx + 1, code) + 1;
		if (iLength == 0)
			continue;

		bool bPlaceFirst = false;
		CMStringA szReplace;
		if (code.szTag == "emoticon")
			szReplace.Format(":%s:", code.szValue.c_str());
		else if (code.szTag == "sticker")
			szReplace.Format(":%s:", code.attrs["type"].c_str());
		else if (code.szTag == "mention") {
			CMStringW wszId(FORMAT, L"%lld", AccountIdToSteamId(_atoi64(code.szValue)));
			if (auto *pUser = g_chatApi.UM_FindUser(si, wszId)) {
				szReplace.Format("%s:", T2Utf(pUser->pszNick).get());
				bPlaceFirst = true;
			}
		}
		else if (code.szTag == "lobbyinvite") {
			szReplace = TranslateU("You were invited to play a game");
		}
		else if (code.szTag == "img") {
			auto szUrl = code.attrs["src"];
			if (szUrl.IsEmpty())
				szUrl = code.attrs["thumbnail_src"];

			if (!szUrl.IsEmpty())
				szReplace = "[url]" + szUrl + "[/url]";

			szUrl = code.attrs["associated_app"];
			if (!szUrl.IsEmpty()) {
				CMStringA szSetting = "AppInfo_" + szUrl;
				ptrA szName(g_plugin.getUStringA(szSetting));
				if (szName)
					szReplace.AppendFormat("\r\n%s: %s", TranslateU("Associated application"), szName.get());
				else
					SendAppInfoRequest(atoi(szUrl));
			}
		}
		else if (code.szTag == "spoiler") {
			szReplace = code.szValue;
		}
		else continue;

		szText.Delete(idx, iLength);
		if (!szReplace.IsEmpty()) {
			if (bPlaceFirst)
				szText = szReplace + szText;
			else
				szText.Insert(idx, szReplace);
		}
	}
}
