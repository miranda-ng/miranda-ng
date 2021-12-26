/*
Copyright (C) 2012 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

static CMStringA DetectEncoding(const TiXmlDocument &doc)
{
	auto *pChild = doc.FirstChild();
	if (pChild)
		if (auto *pDecl = pChild->ToDeclaration())
			if (auto *pVal = pDecl->Value())
				if (!memcmp(pVal, "xml", 3)) {
					const char *p1 = strstr(pVal, "encoding=\""), *p2 = 0;
					if (p1) {
						p1 += 10;
						p2 = strchr(p1, '\"');
					}
					if (p1 && p2)
						return CMStringA(p1, int(p2-p1));
				}

	return CMStringA();
}

static wchar_t* EncodeResult(const char *szString, const CMStringA &szEncoding)
{
	if (szEncoding == "koi8-r")
		return mir_a2u_cp(szString, 20866);
	if (szEncoding == "windows-1251")
		return mir_a2u_cp(szString, 1251);

	return mir_utf8decodeW(szString);
}

static void SetAvatar(MCONTACT hContact, const char *pszValue)
{
	Utf2T url(pszValue);
	g_plugin.setWString(hContact, "ImageURL", url);

	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.hContact = hContact;

	ptrW szNick(g_plugin.getWStringA(hContact, "Nick"));
	if (szNick) {
		wchar_t *ext = wcsrchr((wchar_t *)url, '.') + 1;
		ai.format = ProtoGetAvatarFormat(ext);

		wchar_t *filename = szNick;
		mir_snwprintf(ai.filename, L"%s\\%s.%s", tszRoot, filename, ext);
		if (DownloadFile(url, ai.filename)) {
			g_plugin.setWString(hContact, "ImagePath", ai.filename);
			ProtoBroadcastAck(MODULENAME, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai, NULL);
		}
		else ProtoBroadcastAck(MODULENAME, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&ai, NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// touches a feed to verify whether it exists

LPCTSTR CheckFeed(wchar_t *tszURL, CFeedEditor *pEditDlg)
{
	Netlib_LogfW(hNetlibUser, L"Started validating feed %s.", tszURL);
	char *szData = nullptr;
	GetNewsData(tszURL, &szData, NULL, pEditDlg);
	if (szData) {
		TiXmlDocument doc;
		int ret = doc.Parse(szData);
		mir_free(szData);
		if (ret == ERROR_SUCCESS) {
			CMStringA codepage = DetectEncoding(doc);

			for (auto *it : TiXmlEnum(&doc)) {
				auto *szNodeName = it->Name();
				const TiXmlElement *pNode;
				if (!mir_strcmpi(szNodeName, "rss") || !mir_strcmpi(szNodeName, "rdf"))
					pNode = it->FirstChildElement();
				else if (!mir_strcmpi(szNodeName, "feed"))
					pNode = it;
				else continue;

				for (auto *child : TiXmlFilter(pNode, "title")) {
					wchar_t mes[MAX_PATH];
					mir_snwprintf(mes, TranslateT("%s\nis a valid feed's address."), tszURL);
					MessageBox(pEditDlg->GetHwnd(), mes, TranslateT("News Aggregator"), MB_OK | MB_ICONINFORMATION);
					return EncodeResult(child->GetText(), codepage);
				}
			}
		}
	}

	Netlib_LogfW(hNetlibUser, L"%s is not a valid feed's address.", tszURL);
	wchar_t mes[MAX_PATH];
	mir_snwprintf(mes, TranslateT("%s\nis not a valid feed's address."), tszURL);
	MessageBox(pEditDlg->GetHwnd(), mes, TranslateT("News Aggregator"), MB_OK | MB_ICONERROR);
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// loads a feed completely, with messages

static void XmlToMsg(MCONTACT hContact, CMStringW &title, CMStringW &link, CMStringW &descr, CMStringW &author, CMStringW &comments, CMStringW &guid, CMStringW &category, time_t stamp)
{
	CMStringW message = g_plugin.getWStringA(hContact, "MsgFormat");
	if (!message)
		message = TAGSDEFAULT;

	if (title.IsEmpty())
		message.Replace(L"#<title>#", TranslateT("empty"));
	else
		message.Replace(L"#<title>#", title);

	if (link.IsEmpty())
		message.Replace(L"#<link>#", TranslateT("empty"));
	else
		message.Replace(L"#<link>#", link);

	if (descr.IsEmpty())
		message.Replace(L"#<description>#", TranslateT("empty"));
	else
		message.Replace(L"#<description>#", descr);

	if (author.IsEmpty())
		message.Replace(L"#<author>#", TranslateT("empty"));
	else
		message.Replace(L"#<author>#", author);

	if (comments.IsEmpty())
		message.Replace(L"#<comments>#", TranslateT("empty"));
	else
		message.Replace(L"#<comments>#", comments);

	if (guid.IsEmpty())
		message.Replace(L"#<guid>#", TranslateT("empty"));
	else
		message.Replace(L"#<guid>#", guid);

	if (category.IsEmpty())
		message.Replace(L"#<category>#", TranslateT("empty"));
	else
		message.Replace(L"#<category>#", category);

	DBEVENTINFO olddbei = {};
	bool  MesExist = false;
	T2Utf pszTemp(message);
	uint32_t cbMemoLen = 10000, cbOrigLen = (uint32_t)mir_strlen(pszTemp);
	uint8_t *pbBuffer = (uint8_t*)mir_alloc(cbMemoLen);

	DB::ECPTR pCursor(DB::EventsRev(hContact));
	while (MEVENT hDbEvent = pCursor.FetchNext()) {
		olddbei.cbBlob = db_event_getBlobSize(hDbEvent);
		if (olddbei.cbBlob > cbMemoLen)
			pbBuffer = (uint8_t*)mir_realloc(pbBuffer, (size_t)(cbMemoLen = olddbei.cbBlob));
		olddbei.pBlob = pbBuffer;
		db_event_get(hDbEvent, &olddbei);

		// there's no need to look for the elder events
		if (stamp > 0 && olddbei.timestamp < (uint32_t)stamp)
			break;

		if ((uint32_t)mir_strlen((char*)olddbei.pBlob) == cbOrigLen && !mir_strcmp((char*)olddbei.pBlob, pszTemp)) {
			MesExist = true;
			break;
		}
	}
	mir_free(pbBuffer);

	if (!MesExist) {
		if (stamp == 0)
			stamp = time(0);

		T2Utf pszMessage(message);

		PROTORECVEVENT recv = { 0 };
		recv.timestamp = (uint32_t)stamp;
		recv.szMessage = pszMessage;
		ProtoChainRecvMsg(hContact, &recv);
	}
}

void CheckCurrentFeed(MCONTACT hContact)
{
	// Check is disabled by the user?
	if (!g_plugin.getByte(hContact, "CheckState", 1) != 0)
		return;

	wchar_t *szURL = g_plugin.getWStringA(hContact, "URL");
	if (szURL == nullptr)
		return;

	Netlib_LogfW(hNetlibUser, L"Started checking feed %s.", szURL);

	char *szData = nullptr;
	GetNewsData(szURL, &szData, hContact, nullptr);
	mir_free(szURL);

	if (szData == nullptr)
		return;

	TiXmlDocument doc;
	int ret = doc.Parse(szData);
	mir_free(szData);
	if (ret != ERROR_SUCCESS)
		return;

	CMStringA codepage = DetectEncoding(doc);

	CMStringW szValue;
	
	for (auto *it : TiXmlEnum(&doc)) {
		auto *szNodeName = it->Name();
		bool isRSS = !mir_strcmpi(szNodeName, "rss"), isAtom = !mir_strcmpi(szNodeName, "rdf");
		if (isRSS || isAtom) {
			if (isRSS) {
				if (auto *pVersion = it->Attribute("version")) {
					char ver[MAX_PATH];
					mir_snprintf(ver, "RSS %s", pVersion);
					g_plugin.setString(hContact, "MirVer", ver);
				}
			}
			else if (isAtom)
				g_plugin.setWString(hContact, "MirVer", L"RSS 1.0");

			for (auto *child : TiXmlEnum(it->FirstChildElement())) {
				auto *childName = child->Name();
				if (!mir_strcmpi(childName, "title")) {
					ptrW szChildText(EncodeResult(child->GetText(), codepage));
					if (szChildText)
						g_plugin.setWString(hContact, "FirstName", ClearText(szValue, szChildText));
				}
				else if (!mir_strcmpi(childName, "link")) {
					ptrW szChildText(EncodeResult(child->GetText(), codepage));
					if (szChildText)
						g_plugin.setWString(hContact, "Homepage", ClearText(szValue, szChildText));
				}
				else if (!mir_strcmpi(childName, "description")) {
					ptrW szChildText(EncodeResult(child->GetText(), codepage));
					if (szChildText) {
						ClearText(szValue, szChildText);
						g_plugin.setWString(hContact, "About", szValue);
						db_set_ws(hContact, "CList", "StatusMsg", szValue);
					}
				}
				else if (!mir_strcmpi(childName, "language")) {
					ptrW szChildText(EncodeResult(child->GetText(), codepage));
					if (szChildText)
						g_plugin.setWString(hContact, "Language1", ClearText(szValue, szChildText));
				}
				else if (!mir_strcmpi(childName, "managingEditor")) {
					ptrW szChildText(EncodeResult(child->GetText(), codepage));
					if (szChildText)
						g_plugin.setWString(hContact, "e-mail", ClearText(szValue, szChildText));
				}
				else if (!mir_strcmpi(childName, "category")) {
					ptrW szChildText(EncodeResult(child->GetText(), codepage));
					if (szChildText)
						g_plugin.setWString(hContact, "Interest0Text", ClearText(szValue, szChildText));
				}
				else if (!mir_strcmpi(childName, "copyright")) {
					ptrW szChildText(EncodeResult(child->GetText(), codepage));
					if (szChildText)
						db_set_s(hContact, "UserInfo", "MyNotes", _T2A(ClearText(szValue, szChildText)));
				}
				else if (!mir_strcmpi(childName, "image")) {
					for (auto *xmlImage : TiXmlFilter(child, "url"))
						SetAvatar(hContact, xmlImage->GetText());
				}
				else if (!mir_strcmpi(childName, "lastBuildDate")) {
					time_t stamp = DateToUnixTime(child->GetText(), 0);
					double deltaupd = difftime(time(0), stamp);
					double deltacheck = difftime(time(0), (time_t)g_plugin.getDword(hContact, "LastCheck"));
					if (deltaupd - deltacheck >= 0) {
						g_plugin.setDword(hContact, "LastCheck", (uint32_t)time(0));
						return;
					}
				}
				else if (!mir_strcmpi(childName, "item")) {
					CMStringW title, link, descr, author, comments, guid, category;
					time_t stamp = 0;
					for (auto *itemval : TiXmlEnum(child)) {
						auto *itemName = itemval->Name();
						ptrW value(EncodeResult(itemval->GetText(), codepage));

						// We only use the first tag for now and ignore the rest.
						if (!mir_strcmpi(itemName, "title"))
							ClearText(title, value);

						else if (!mir_strcmpi(itemName, "link"))
							ClearText(link, value);

						else if (!mir_strcmpi(itemName, "pubDate") || !mir_strcmpi(itemName, "date")) {
							if (stamp == 0)
								stamp = DateToUnixTime(itemval->GetText(), 0);
						}
						else if (!mir_strcmpi(itemName, "description") || !mir_strcmpi(itemName, "encoded"))
							ClearText(descr, value);

						else if (!mir_strcmpi(itemName, "author") || !mir_strcmpi(itemName, "creator"))
							ClearText(author, value);

						else if (!mir_strcmpi(itemName, "comments"))
							ClearText(comments, value);

						else if (!mir_strcmpi(itemName, "guid"))
							ClearText(guid, value);

						else if (!mir_strcmpi(itemName, "category"))
							ClearText(category, value);
					}

					XmlToMsg(hContact, title, link, descr, author, comments, guid, category, stamp);
				}
			}
		}
		else if (!mir_strcmpi(szNodeName, "feed")) {
			g_plugin.setWString(hContact, "MirVer", L"Atom 3");
			for (auto *child : TiXmlEnum(it)) {
				auto *szChildName = child->Name();
				if (!mir_strcmpi(szChildName, "title")) {
					ptrW szChildText(EncodeResult(child->GetText(), codepage));
					if (szChildText)
						g_plugin.setWString(hContact, "FirstName", ClearText(szValue, szChildText));
				}
				else if (!mir_strcmpi(szChildName, "link")) {
					if (!child->Attribute("rel", "self"))
						if (auto *pHref = child->Attribute("href"))
							g_plugin.setWString(hContact, "Homepage", Utf2T(pHref));
				}
				else if (!mir_strcmpi(szChildName, "subtitle")) {
					ptrW szChildText(EncodeResult(child->GetText(), codepage));
					if (szChildText) {
						ClearText(szValue, szChildText);
						g_plugin.setWString(hContact, "About", szValue);
						db_set_ws(hContact, "CList", "StatusMsg", szValue);
					}
				}
				else if (!mir_strcmpi(szChildName, "language")) {
					ptrW szChildText(EncodeResult(child->GetText(), codepage));
					if (szChildText)
						g_plugin.setWString(hContact, "Language1", ClearText(szValue, szChildText));
				}
				else if (!mir_strcmpi(szChildName, "author")) {
					for (auto *authorval : TiXmlFilter(child, "email")) {
						g_plugin.setWString(hContact, "e-mail", ptrW(EncodeResult(authorval->GetText(), codepage)));
						break;
					}
				}
				else if (!mir_strcmpi(szChildName, "category")) {
					ptrW szChildText(EncodeResult(child->GetText(), codepage));
					if (szChildText)
						g_plugin.setWString(hContact, "Interest0Text", ClearText(szValue, szChildText));
				}
				else if (!mir_strcmpi(szChildName, "icon")) {
					for (auto *imageval : TiXmlFilter(child, "url"))
						SetAvatar(hContact, imageval->GetText());
				}
				else if (!mir_strcmpi(szChildName, "updated")) {
					time_t stamp = DateToUnixTime(child->GetText(), 1);
					double deltaupd = difftime(time(0), stamp);
					double deltacheck = difftime(time(0), (time_t)g_plugin.getDword(hContact, "LastCheck"));
					if (deltaupd - deltacheck >= 0) {
						g_plugin.setDword(hContact, "LastCheck", (uint32_t)time(0));
						return;
					}
				}
				else if (!mir_strcmpi(szChildName, "entry")) {
					CMStringW title, link, descr, author, comments, guid, category;
					time_t stamp = 0;
					for (auto *itemval : TiXmlEnum(child)) {
						LPCSTR szItemName = itemval->Name();
						if (!mir_strcmpi(szItemName, "title")) {
							ptrW szItemText(EncodeResult(itemval->GetText(), codepage));
							if (szItemText)
								ClearText(title, szItemText);
						}
						else if (!mir_strcmpi(szItemName, "link")) {
							if (auto *pszLink = itemval->Attribute("href"))
								ClearText(link, ptrW(EncodeResult(pszLink, codepage)));
						}
						else if (!mir_strcmpi(szItemName, "updated")) {
							if (stamp == 0)
								stamp = DateToUnixTime(itemval->GetText(), 0);
						}
						else if (!mir_strcmpi(szItemName, "summary") || !mir_strcmpi(szItemName, "content")) {
							ptrW szItemText(EncodeResult(itemval->GetText(), codepage));
							if (szItemText)
								ClearText(descr, szItemText);
						}
						else if (!mir_strcmpi(szItemName, "author")) {
							for (auto *authorval : TiXmlFilter(itemval, "name")) {
								ptrW szItemText(EncodeResult(authorval->GetText(), codepage));
								if (szItemText)
									ClearText(author, szItemText);
								break;
							}
						}
						else if (!mir_strcmpi(szItemName, "comments")) {
							ptrW szItemText(EncodeResult(itemval->GetText(), codepage));
							if (szItemText)
								ClearText(comments, szItemText);
						}
						else if (!mir_strcmpi(szItemName, "id")) {
							ptrW szItemText(EncodeResult(itemval->GetText(), codepage));
							if (szItemText)
								ClearText(guid, szItemText);
						}
						else if (!mir_strcmpi(szItemName, "category")) {
							if (auto *p = itemval->Attribute("term"))
								ClearText(link, ptrW(EncodeResult(p, codepage)));
						}
					}

					XmlToMsg(hContact, title, link, descr, author, comments, guid, category, stamp);
				}
			}
		}
	}
	g_plugin.setDword(hContact, "LastCheck", (uint32_t)time(0));
}

/////////////////////////////////////////////////////////////////////////////////////////
// downloads avatars from a given feed

void CheckCurrentFeedAvatar(MCONTACT hContact)
{
	if (!g_plugin.getByte(hContact, "CheckState", 1))
		return;

	wchar_t *szURL = g_plugin.getWStringA(hContact, "URL");
	if (szURL == nullptr)
		return;

	char *szData = nullptr;
	GetNewsData(szURL, &szData, hContact, nullptr);
	mir_free(szURL);

	if (szData == nullptr)
		return;

	TiXmlDocument doc;
	int ret = doc.Parse(szData);
	mir_free(szData);
	if (ret != ERROR_SUCCESS)
		return;

	for (auto *it : TiXmlEnum(&doc)) {
		auto *szNodeName = it->Name();
		if (!mir_strcmpi(szNodeName, "rss") || !mir_strcmpi(szNodeName, "rdf")) {
			for (auto *child : TiXmlFilter(it->FirstChildElement(), "image"))
				for (auto *xmlImage : TiXmlFilter(child, "url"))
					SetAvatar(hContact, xmlImage->GetText());
		}
		else if (!mir_strcmpi(szNodeName, "feed")) {
			for (auto *child : TiXmlFilter(it, "icon"))
				for (auto *xmlImage : TiXmlFilter(child, "url"))
					SetAvatar(hContact, xmlImage->GetText());
		}
	}
}
