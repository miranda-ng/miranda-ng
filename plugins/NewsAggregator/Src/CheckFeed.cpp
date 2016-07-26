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

LPCTSTR CheckFeed(wchar_t *tszURL, HWND hwndDlg)
{
	Netlib_LogfT(hNetlibUser, L"Started validating feed %s.", tszURL);
	char *szData = NULL;
	GetNewsData(tszURL, &szData, NULL, hwndDlg);
	if (szData) {
		wchar_t *tszData = mir_utf8decodeT(szData);
		if (!tszData)
			tszData = mir_a2t(szData);
		int bytesParsed = 0;
		HXML hXml = xmlParseString(tszData, &bytesParsed, NULL);
		mir_free(tszData);
		mir_free(szData);
		if (hXml != NULL) {
			LPCTSTR codepage = NULL;
			int childcount = 0;
			HXML node;
			if (!mir_tstrcmpi(xmlGetName(hXml), L"xml")) {
				int attrcount = xmlGetAttrCount(hXml);
				for (int i = 0; i < attrcount; i++) {
					LPCTSTR szAttrName = xmlGetAttrName(hXml, i);
					if (!mir_tstrcmpi(szAttrName, L"encoding")) {
						codepage = xmlGetAttrValue(hXml, szAttrName);
						break;
					}
				}
				node = xmlGetChild(hXml, childcount);
			}
			else
				node = hXml;
			while (node) {
				LPCTSTR szNodeName = xmlGetName(node);
				if (!mir_tstrcmpi(szNodeName, L"rss") || !mir_tstrcmpi(szNodeName, L"rdf")) {
					HXML chan = xmlGetChild(node, 0);
					for (int j = 0; j < xmlGetChildCount(chan); j++) {
						HXML child = xmlGetChild(chan, j);
						if (!mir_tstrcmpi(xmlGetName(child), L"title")) {
							wchar_t mes[MAX_PATH];
							mir_sntprintf(mes, TranslateT("%s\nis a valid feed's address."), tszURL);
							MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONINFORMATION);
							if (!mir_tstrcmpi(codepage, L"koi8-r")) {
								wchar_t buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								return mir_tstrdup(buf);
							}
							else
								return mir_tstrdup(xmlGetText(child));
						}
					}
				}
				else if (!mir_tstrcmpi(szNodeName, L"feed")) {
					for (int j = 0; j < xmlGetChildCount(node); j++) {
						HXML child = xmlGetChild(node, j);
						if (!mir_tstrcmpi(xmlGetName(child), L"title")) {
							wchar_t mes[MAX_PATH];
							mir_sntprintf(mes, TranslateT("%s\nis a valid feed's address."), tszURL);
							MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONINFORMATION);
							if (!mir_tstrcmpi(codepage, L"koi8-r")) {
								wchar_t buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								return mir_tstrdup(buf);
							}
							else
								return mir_tstrdup(xmlGetText(child));
						}
					}
				}
				node = xmlGetChild(hXml, ++childcount);
			}
		}
		xmlDestroyNode(hXml);
	}
	Netlib_LogfT(hNetlibUser, L"%s is not a valid feed's address.", tszURL);
	wchar_t mes[MAX_PATH];
	mir_sntprintf(mes, TranslateT("%s\nis not a valid feed's address."), tszURL);
	MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONERROR);
	return NULL;
}

static void XmlToMsg(MCONTACT hContact, CMString &title, CMString &link, CMString &descr, CMString &author, CMString &comments, CMString &guid, CMString &category, time_t stamp)
{
	CMString message = db_get_tsa(hContact, MODULE, "MsgFormat");
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

	DBEVENTINFO olddbei = { 0 };
	olddbei.cbSize = sizeof(olddbei);

	bool  MesExist = false;
	T2Utf pszTemp(message);
	DWORD cbMemoLen = 10000, cbOrigLen = (DWORD)mir_strlen(pszTemp);
	BYTE *pbBuffer = (BYTE*)mir_alloc(cbMemoLen);
	for (MEVENT hDbEvent = db_event_last(hContact); hDbEvent; hDbEvent = db_event_prev(hContact, hDbEvent)) {
		olddbei.cbBlob = db_event_getBlobSize(hDbEvent);
		if (olddbei.cbBlob > cbMemoLen)
			pbBuffer = (PBYTE)mir_realloc(pbBuffer, (size_t)(cbMemoLen = olddbei.cbBlob));
		olddbei.pBlob = pbBuffer;
		db_event_get(hDbEvent, &olddbei);

		// there's no need to look for the elder events
		if (stamp > 0 && olddbei.timestamp < (DWORD)stamp)
			break;

		if ((DWORD)mir_strlen((char*)olddbei.pBlob) == cbOrigLen && !mir_strcmp((char*)olddbei.pBlob, pszTemp)) {
			MesExist = true;
			break;
		}
	}
	mir_free(pbBuffer);

	if (!MesExist) {
		if (stamp == 0)
			stamp = time(NULL);

		T2Utf pszMessage(message);

		PROTORECVEVENT recv = { 0 };
		recv.timestamp = (DWORD)stamp;
		recv.szMessage = pszMessage;
		ProtoChainRecvMsg(hContact, &recv);
	}
}

void CheckCurrentFeed(MCONTACT hContact)
{
	// Check is disabled by the user?
	if (!db_get_b(hContact, MODULE, "CheckState", 1) != 0)
		return;

	wchar_t *szURL = db_get_tsa(hContact, MODULE, "URL");
	if (szURL == NULL)
		return;

	Netlib_LogfT(hNetlibUser, L"Started checking feed %s.", szURL);

	char *szData = NULL;
	GetNewsData(szURL, &szData, hContact, NULL);
	mir_free(szURL);

	if (szData) {
		wchar_t *tszData = mir_utf8decodeT(szData);
		if (!tszData)
			tszData = mir_a2t(szData);
		int bytesParsed = 0;
		HXML hXml = xmlParseString(tszData, &bytesParsed, NULL);
		mir_free(tszData);
		mir_free(szData);

		CMString szValue;
		if (hXml != NULL) {
			LPCTSTR codepage = NULL;
			int childcount = 0;
			HXML node;
			if (!mir_tstrcmpi(xmlGetName(hXml), L"xml")) {
				int attrcount = xmlGetAttrCount(hXml);
				for (int i = 0; i < attrcount; i++) {
					LPCTSTR szAttrName = xmlGetAttrName(hXml, i);
					if (!mir_tstrcmpi(szAttrName, L"encoding")) {
						codepage = xmlGetAttrValue(hXml, szAttrName);
						break;
					}
				}
				node = xmlGetChild(hXml, childcount);
			}
			else
				node = hXml;
			while (node) {
				LPCTSTR szNodeName = xmlGetName(node);
				bool isRSS = !mir_tstrcmpi(szNodeName, L"rss"), isAtom = !mir_tstrcmpi(szNodeName, L"rdf");
				if (isRSS || isAtom) {
					if (isRSS) {
						for (int i = 0; i < xmlGetAttrCount(node); i++) {
							LPCTSTR szAttrName = xmlGetAttrName(node, i);
							if (!mir_tstrcmpi(szAttrName, L"version")) {
								wchar_t ver[MAX_PATH];
								mir_sntprintf(ver, L"RSS %s", xmlGetAttrValue(node, szAttrName));
								db_set_ts(hContact, MODULE, "MirVer", ver);
								break;
							}
						}
					}
					else if (isAtom)
						db_set_ts(hContact, MODULE, "MirVer", L"RSS 1.0");

					HXML chan = xmlGetChild(node, 0);
					for (int j = 0; j < xmlGetChildCount(chan); j++) {
						HXML child = xmlGetChild(chan, j);
						LPCTSTR childName = xmlGetName(child);
						if (!mir_tstrcmpi(childName, L"title")) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, L"koi8-r")) {
								wchar_t buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "FirstName", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, L"link")) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, L"koi8-r")) {
								wchar_t buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Homepage", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, L"description")) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, L"koi8-r")) {
								wchar_t buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText) {
								ClearText(szValue, szChildText);
								db_set_ts(hContact, MODULE, "About", szValue);
								db_set_ts(hContact, "CList", "StatusMsg", szValue);
							}
						}
						else if (!mir_tstrcmpi(childName, L"language")) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, L"koi8-r")) {
								wchar_t buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Language1", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, L"managingEditor")) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, L"koi8-r")) {
								wchar_t buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "e-mail", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, L"category")) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, L"koi8-r")) {
								wchar_t buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Interest0Text", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, L"copyright")) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, L"koi8-r")) {
								wchar_t buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText)
								db_set_s(hContact, "UserInfo", "MyNotes", _T2A(ClearText(szValue, szChildText)));
						}
						else if (!mir_tstrcmpi(childName, L"image")) {
							for (int x = 0; x < xmlGetChildCount(child); x++) {
								HXML imageval = xmlGetChild(child, x);
								if (!mir_tstrcmpi(xmlGetName(imageval), L"url")) {
									LPCTSTR url = xmlGetText(imageval);
									db_set_ts(hContact, MODULE, "ImageURL", url);

									PROTO_AVATAR_INFORMATION ai = { 0 };
									ai.hContact = hContact;

									wchar_t *szNick = db_get_tsa(hContact, MODULE, "Nick");
									if (szNick) {
										wchar_t *ext = wcsrchr((wchar_t *)url, '.') + 1;
										ai.format = ProtoGetAvatarFormat(url);

										CMString filename = szNick;
										filename.Replace(L"/", L"_");
										mir_sntprintf(ai.filename, L"%s\\%s.%s", tszRoot, filename.c_str(), ext);
										CreateDirectoryTreeT(tszRoot);
										if (DownloadFile(url, ai.filename)) {
											db_set_ts(hContact, MODULE, "ImagePath", ai.filename);
											ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai, NULL);
										}
										else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&ai, NULL);
										mir_free(szNick);
										break;
									}
								}
							}
						}
						else if (!mir_tstrcmpi(childName, L"lastBuildDate")) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, L"koi8-r")) {
								wchar_t buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText) {
								time_t stamp = DateToUnixTime(szChildText, 0);
								double deltaupd = difftime(time(NULL), stamp);
								double deltacheck = difftime(time(NULL), (time_t)db_get_dw(hContact, MODULE, "LastCheck", 0));
								if (deltaupd - deltacheck >= 0) {
									db_set_dw(hContact, MODULE, "LastCheck", (DWORD)time(NULL));
									xmlDestroyNode(hXml);
									return;
								}
							}
						}
						else if (!mir_tstrcmpi(childName, L"item")) {
							CMString title, link, descr, author, comments, guid, category;
							time_t stamp = 0;
							for (int z = 0; z < xmlGetChildCount(child); z++) {
								HXML itemval = xmlGetChild(child, z);
								LPCTSTR itemName = xmlGetName(itemval);
								LPCTSTR value = NULL;
								if (!mir_tstrcmpi(codepage, L"koi8-r")) {
									wchar_t buf[MAX_PATH];
									MultiByteToWideChar(20866, 0, _T2A(xmlGetText(itemval)), -1, buf, _countof(buf));
									value = buf;
								}
								else
									value = xmlGetText(itemval);

								// We only use the first tag for now and ignore the rest.
								if (!mir_tstrcmpi(itemName, L"title"))
									ClearText(title, value);

								else if (!mir_tstrcmpi(itemName, L"link"))
									ClearText(link, value);

								else if (!mir_tstrcmpi(itemName, L"pubDate") || !mir_tstrcmpi(itemName, L"date")) {
									if (stamp == 0)
										stamp = DateToUnixTime(value, 0);
								}
								else if (!mir_tstrcmpi(itemName, L"description") || !mir_tstrcmpi(itemName, L"encoded"))
									ClearText(descr, value);

								else if (!mir_tstrcmpi(itemName, L"author") || !mir_tstrcmpi(itemName, L"creator"))
									ClearText(author, value);

								else if (!mir_tstrcmpi(itemName, L"comments"))
									ClearText(comments, value);

								else if (!mir_tstrcmpi(itemName, L"guid"))
									ClearText(guid, value);

								else if (!mir_tstrcmpi(itemName, L"category"))
									ClearText(category, value);
							}

							XmlToMsg(hContact, title, link, descr, author, comments, guid, category, stamp);
						}
					}
				}
				else if (!mir_tstrcmpi(szNodeName, L"feed")) {
					db_set_ts(hContact, MODULE, "MirVer", L"Atom 3");
					for (int j = 0; j < xmlGetChildCount(node); j++) {
						HXML child = xmlGetChild(node, j);
						LPCTSTR szChildName = xmlGetName(child);
						if (!mir_tstrcmpi(szChildName, L"title")) {
							LPCTSTR szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "FirstName", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(szChildName, L"link")) {
							for (int x = 0; x < xmlGetAttrCount(child); x++) {
								if (!mir_tstrcmpi(xmlGetAttrName(child, x), L"rel"))
									if (!mir_tstrcmpi(xmlGetAttrValue(child, xmlGetAttrName(child, x)), L"self"))
										break;

								if (!mir_tstrcmpi(xmlGetAttrName(child, x), L"href"))
									db_set_ts(hContact, MODULE, "Homepage", xmlGetAttrValue(child, xmlGetAttrName(child, x)));
							}
						}
						else if (!mir_tstrcmpi(szChildName, L"subtitle")) {
							LPCTSTR szChildText = xmlGetText(child);
							if (szChildText) {
								ClearText(szValue, szChildText);
								db_set_ts(hContact, MODULE, "About", szValue);
								db_set_ts(hContact, "CList", "StatusMsg", szValue);
							}
						}
						else if (!mir_tstrcmpi(szChildName, L"language")) {
							LPCTSTR szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Language1", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(szChildName, L"author")) {
							for (int x = 0; x < xmlGetChildCount(child); x++) {
								HXML authorval = xmlGetChild(child, x);
								if (!mir_tstrcmpi(xmlGetName(authorval), L"email")) {
									db_set_ts(hContact, MODULE, "e-mail", xmlGetText(authorval));
									break;
								}
							}
						}
						else if (!mir_tstrcmpi(szChildName, L"category")) {
							LPCTSTR szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Interest0Text", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(szChildName, L"icon")) {
							for (int x = 0; x < xmlGetChildCount(child); x++) {
								HXML imageval = xmlGetChild(child, x);
								if (!mir_tstrcmpi(xmlGetName(imageval), L"url")) {
									LPCTSTR url = xmlGetText(imageval);
									db_set_ts(hContact, MODULE, "ImageURL", url);

									ptrT szNick(db_get_tsa(hContact, MODULE, "Nick"));
									if (szNick) {
										PROTO_AVATAR_INFORMATION ai = { 0 };
										ai.hContact = hContact;
										wchar_t *ext = wcsrchr((wchar_t *)url, '.') + 1;
										ai.format = ProtoGetAvatarFormat(ext);

										wchar_t *filename = szNick;
										mir_sntprintf(ai.filename, L"%s\\%s.%s", tszRoot, filename, ext);
										if (DownloadFile(url, ai.filename)) {
											db_set_ts(hContact, MODULE, "ImagePath", ai.filename);
											ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai, NULL);
										}
										else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&ai, NULL);
										break;
									}
								}
							}
						}
						else if (!mir_tstrcmpi(szChildName, L"updated")) {
							LPCTSTR szChildText = xmlGetText(child);
							if (szChildText) {
								wchar_t *lastupdtime = (wchar_t *)szChildText;
								time_t stamp = DateToUnixTime(lastupdtime, 1);
								double deltaupd = difftime(time(NULL), stamp);
								double deltacheck = difftime(time(NULL), (time_t)db_get_dw(hContact, MODULE, "LastCheck", 0));
								if (deltaupd - deltacheck >= 0) {
									db_set_dw(hContact, MODULE, "LastCheck", (DWORD)time(NULL));
									xmlDestroyNode(hXml);
									return;
								}
							}
						}
						else if (!mir_tstrcmpi(szChildName, L"entry")) {
							CMString title, link, descr, author, comments, guid, category;
							time_t stamp = 0;
							for (int z = 0; z < xmlGetChildCount(child); z++) {
								HXML itemval = xmlGetChild(child, z);
								LPCTSTR szItemName = xmlGetName(itemval);
								if (!mir_tstrcmpi(szItemName, L"title")) {
									LPCTSTR szItemText = xmlGetText(itemval);
									if (szItemText)
										ClearText(title, szItemText);
								}
								else if (!mir_tstrcmpi(szItemName, L"link")) {
									for (int x = 0; x < xmlGetAttrCount(itemval); x++) {
										if (!mir_tstrcmpi(xmlGetAttrName(itemval, x), L"href")) {
											ClearText(link, xmlGetAttrValue(itemval, xmlGetAttrName(itemval, x)));
											break;
										}
									}
								}
								else if (!mir_tstrcmpi(szItemName, L"updated")) {
									if (stamp == 0)
										stamp = DateToUnixTime(xmlGetText(itemval), 0);
								}
								else if (!mir_tstrcmpi(szItemName, L"summary") || !mir_tstrcmpi(szItemName, L"content")) {
									LPCTSTR szItemText = xmlGetText(itemval);
									if (szItemText)
										ClearText(descr, szItemText);
								}
								else if (!mir_tstrcmpi(szItemName, L"author")) {
									for (int x = 0; x < xmlGetChildCount(itemval); x++) {
										HXML authorval = xmlGetChild(itemval, x);
										if (!mir_tstrcmpi(xmlGetName(authorval), L"name") && xmlGetText(authorval)) {
											ClearText(author, xmlGetText(authorval));
											break;
										}
									}
								}
								else if (!mir_tstrcmpi(szItemName, L"comments")) {
									LPCTSTR szItemText = xmlGetText(itemval);
									if (szItemText)
										ClearText(comments, szItemText);
								}
								else if (!mir_tstrcmpi(szItemName, L"id")) {
									LPCTSTR szItemText = xmlGetText(itemval);
									if (szItemText)
										ClearText(guid, xmlGetText(itemval));
								}
								else if (!mir_tstrcmpi(szItemName, L"category")) {
									for (int x = 0; x < xmlGetAttrCount(itemval); x++) {
										LPCTSTR szAttrName = xmlGetAttrName(itemval, x);
										if (!mir_tstrcmpi(szAttrName, L"term") && xmlGetText(itemval)) {
											ClearText(category, xmlGetAttrValue(itemval, szAttrName));
											break;
										}
									}
								}
							}

							XmlToMsg(hContact, title, link, descr, author, comments, guid, category, stamp);
						}
					}
				}
				node = xmlGetChild(hXml, ++childcount);
			}
			xmlDestroyNode(hXml);
		}
	}
	db_set_dw(hContact, MODULE, "LastCheck", (DWORD)time(NULL));
}

void CheckCurrentFeedAvatar(MCONTACT hContact)
{
	if (!db_get_b(hContact, MODULE, "CheckState", 1))
		return;

	wchar_t *szURL = db_get_tsa(hContact, MODULE, "URL");
	if (szURL == NULL)
		return;

	char *szData = NULL;
	GetNewsData(szURL, &szData, hContact, NULL);
	mir_free(szURL);

	if (szData == NULL)
		return;

	wchar_t *tszData = mir_utf8decodeT(szData);
	if (!tszData)
		tszData = mir_a2t(szData);
	int bytesParsed = 0;
	HXML hXml = xmlParseString(tszData, &bytesParsed, NULL);
	mir_free(tszData);
	mir_free(szData);
	if (hXml == NULL)
		return;

	int childcount = 0;
	HXML node = xmlGetChild(hXml, childcount);
	while (node) {
		LPCTSTR szNodeName = xmlGetName(node);
		if (!mir_tstrcmpi(szNodeName, L"rss") || !mir_tstrcmpi(szNodeName, L"rdf")) {
			HXML chan = xmlGetChild(node, 0);
			for (int j = 0; j < xmlGetChildCount(chan); j++) {
				HXML child = xmlGetChild(chan, j);
				if (!mir_tstrcmpi(xmlGetName(child), L"image")) {
					for (int x = 0; x < xmlGetChildCount(child); x++) {
						HXML imageval = xmlGetChild(child, x);
						if (!mir_tstrcmpi(xmlGetName(imageval), L"url")) {
							LPCTSTR url = xmlGetText(imageval);
							db_set_ts(hContact, MODULE, "ImageURL", url);

							PROTO_AVATAR_INFORMATION ai = { 0 };
							ai.hContact = hContact;

							wchar_t *szNick = db_get_tsa(hContact, MODULE, "Nick");
							if (szNick) {
								wchar_t *ext = wcsrchr((wchar_t *)url, '.') + 1;
								ai.format = ProtoGetAvatarFormat(ext);

								wchar_t *filename = szNick;
								mir_sntprintf(ai.filename, L"%s\\%s.%s", tszRoot, filename, ext);
								if (DownloadFile(url, ai.filename)) {
									db_set_ts(hContact, MODULE, "ImagePath", ai.filename);
									ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai, NULL);
								}
								else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&ai, NULL);
								mir_free(szNick);
								break;
							}
						}
					}
				}
			}
		}
		else if (!mir_tstrcmpi(szNodeName, L"feed")) {
			for (int j = 0; j < xmlGetChildCount(node); j++) {
				HXML child = xmlGetChild(node, j);
				if (!mir_tstrcmpi(xmlGetName(child), L"icon")) {
					for (int x = 0; x < xmlGetChildCount(child); x++) {
						HXML imageval = xmlGetChild(child, x);
						if (!mir_tstrcmpi(xmlGetName(imageval), L"url")) {
							LPCTSTR url = xmlGetText(imageval);
							db_set_ts(hContact, MODULE, "ImageURL", url);

							ptrT szNick(db_get_tsa(hContact, MODULE, "Nick"));
							if (szNick) {
								PROTO_AVATAR_INFORMATION ai = { 0 };
								ai.hContact = hContact;

								wchar_t *ext = wcsrchr((wchar_t *)url, '.') + 1;
								ai.format = ProtoGetAvatarFormat(ext);

								wchar_t *filename = szNick;
								mir_sntprintf(ai.filename, L"%s\\%s.%s", tszRoot, filename, ext);
								if (DownloadFile(url, ai.filename)) {
									db_set_ts(hContact, MODULE, "ImagePath", ai.filename);
									ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai, NULL);
								}
								else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&ai, NULL);
								break;
							}
						}
					}
				}
			}
		}
		node = xmlGetChild(hXml, ++childcount);
	}
	xmlDestroyNode(hXml);
}
