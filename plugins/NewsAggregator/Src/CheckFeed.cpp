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

#include "common.h"

LPCTSTR CheckFeed(TCHAR *tszURL, HWND hwndDlg)
{
	Netlib_LogfT(hNetlibUser, _T("Started validating feed %s."), tszURL);
	char *szData = NULL;
	GetNewsData(tszURL, &szData, NULL, hwndDlg);
	if (szData) {
		TCHAR *tszData = mir_utf8decodeT(szData);
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
			if (!mir_tstrcmpi(xmlGetName(hXml), _T("xml"))) {
				for (int i = 0; xmlGetAttrCount(hXml); i++) {
					LPCTSTR szAttrName = xmlGetAttrName(hXml, i);
					if (!mir_tstrcmpi(szAttrName, _T("encoding"))) {
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
				if (!mir_tstrcmpi(szNodeName, _T("rss")) || !mir_tstrcmpi(szNodeName, _T("rdf"))) {
					HXML chan = xmlGetChild(node, 0);
					for (int j = 0; j < xmlGetChildCount(chan); j++) {
						HXML child = xmlGetChild(chan, j);
						if (!mir_tstrcmpi(xmlGetName(child), _T("title"))) {
							TCHAR mes[MAX_PATH];
							mir_sntprintf(mes, _countof(mes), TranslateT("%s\nis a valid feed's address."), tszURL);
							MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONINFORMATION);
							if (!mir_tstrcmpi(codepage, _T("koi8-r"))) {
								TCHAR buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								return mir_tstrdup(buf);
							}
							else
								return mir_tstrdup(xmlGetText(child));
						}
					}
				}
				else if (!mir_tstrcmpi(szNodeName, _T("feed"))) {
					for (int j = 0; j < xmlGetChildCount(node); j++) {
						HXML child = xmlGetChild(node, j);
						if (!mir_tstrcmpi(xmlGetName(child), _T("title"))) {
							TCHAR mes[MAX_PATH];
							mir_sntprintf(mes, _countof(mes), TranslateT("%s\nis a valid feed's address."), tszURL);
							MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONINFORMATION);
							if (!mir_tstrcmpi(codepage, _T("koi8-r"))) {
								TCHAR buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								return buf;
							}
							else
								return xmlGetText(child);
						}
					}
				}
				node = xmlGetChild(hXml, ++childcount);
			}
		}
		xmlDestroyNode(hXml);
	}
	Netlib_LogfT(hNetlibUser, _T("%s is not a valid feed's address."), tszURL);
	TCHAR mes[MAX_PATH];
	mir_sntprintf(mes, _countof(mes), TranslateT("%s\nis not a valid feed's address."), tszURL);
	MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONERROR);
	return NULL;
}

static void XmlToMsg(MCONTACT hContact, CMString &title, CMString &link, CMString &descr, CMString &author, CMString &comments, CMString &guid, CMString &category, time_t stamp)
{
	CMString message = db_get_tsa(hContact, MODULE, "MsgFormat");
	if (!message)
		message = mir_tstrdup(TAGSDEFAULT);

	if (title.IsEmpty())
		message.Replace(_T("#<title>#"), TranslateT("empty"));
	else
		message.Replace(_T("#<title>#"), title);

	if (link.IsEmpty())
		message.Replace(_T("#<link>#"), TranslateT("empty"));
	else
		message.Replace(_T("#<link>#"), link);

	if (descr.IsEmpty())
		message.Replace(_T("#<description>#"), TranslateT("empty"));
	else
		message.Replace(_T("#<description>#"), descr);

	if (author.IsEmpty())
		message.Replace(_T("#<author>#"), TranslateT("empty"));
	else
		message.Replace(_T("#<author>#"), author);

	if (comments.IsEmpty())
		message.Replace(_T("#<comments>#"), TranslateT("empty"));
	else
		message.Replace(_T("#<comments>#"), comments);

	if (guid.IsEmpty())
		message.Replace(_T("#<guid>#"), TranslateT("empty"));
	else
		message.Replace(_T("#<guid>#"), guid);

	if (category.IsEmpty())
		message.Replace(_T("#<category>#"), TranslateT("empty"));
	else
		message.Replace(_T("#<category>#"), category);

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

	TCHAR *szURL = db_get_tsa(hContact, MODULE, "URL");
	if (szURL == NULL)
		return;

	Netlib_LogfT(hNetlibUser, _T("Started checking feed %s."), szURL);

	char *szData = NULL;
	GetNewsData(szURL, &szData, hContact, NULL);
	mir_free(szURL);

	if (szData) {
		TCHAR *tszData = mir_utf8decodeT(szData);
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
			if (!mir_tstrcmpi(xmlGetName(hXml), _T("xml"))) {
				for (int i = 0; xmlGetAttrCount(hXml); i++) {
					LPCTSTR szAttrName = xmlGetAttrName(hXml, i);
					if (!mir_tstrcmpi(szAttrName, _T("encoding"))) {
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
				bool isRSS = !mir_tstrcmpi(szNodeName, _T("rss")), isAtom = !mir_tstrcmpi(szNodeName, _T("rdf"));
				if (isRSS || isAtom) {
					if (isRSS) {
						for (int i = 0; i < xmlGetAttrCount(node); i++) {
							LPCTSTR szAttrName = xmlGetAttrName(node, i);
							if (!mir_tstrcmpi(szAttrName, _T("version"))) {
								TCHAR ver[MAX_PATH];
								mir_sntprintf(ver, _countof(ver), _T("RSS %s"), xmlGetAttrValue(node, szAttrName));
								db_set_ts(hContact, MODULE, "MirVer", ver);
								break;
							}
						}
					}
					else if (isAtom)
						db_set_ts(hContact, MODULE, "MirVer", _T("RSS 1.0"));

					HXML chan = xmlGetChild(node, 0);
					for (int j = 0; j < xmlGetChildCount(chan); j++) {
						HXML child = xmlGetChild(chan, j);
						LPCTSTR childName = xmlGetName(child);
						if (!mir_tstrcmpi(childName, _T("title"))) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, _T("koi8-r"))) {
								TCHAR buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "FirstName", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, _T("link"))) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, _T("koi8-r"))) {
								TCHAR buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Homepage", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, _T("description"))) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, _T("koi8-r"))) {
								TCHAR buf[MAX_PATH];
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
						else if (!mir_tstrcmpi(childName, _T("language"))) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, _T("koi8-r"))) {
								TCHAR buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Language1", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, _T("managingEditor"))) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, _T("koi8-r"))) {
								TCHAR buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "e-mail", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, _T("category"))) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, _T("koi8-r"))) {
								TCHAR buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Interest0Text", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, _T("copyright"))) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, _T("koi8-r"))) {
								TCHAR buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText)
								db_set_s(hContact, "UserInfo", "MyNotes", _T2A(ClearText(szValue, szChildText)));
						}
						else if (!mir_tstrcmpi(childName, _T("image"))) {
							for (int x = 0; x < xmlGetChildCount(child); x++) {
								HXML imageval = xmlGetChild(child, x);
								if (!mir_tstrcmpi(xmlGetName(imageval), _T("url"))) {
									LPCTSTR url = xmlGetText(imageval);
									db_set_ts(hContact, MODULE, "ImageURL", url);

									PROTO_AVATAR_INFORMATION ai = { 0 };
									ai.hContact = hContact;

									TCHAR *szNick = db_get_tsa(hContact, MODULE, "Nick");
									if (szNick) {
										TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
										ai.format = ProtoGetAvatarFormat(url);

										CMString filename = szNick;
										filename.Replace(_T("/"), _T("_"));
										mir_sntprintf(ai.filename, _countof(ai.filename), _T("%s\\%s.%s"), tszRoot, filename.c_str(), ext);
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
						else if (!mir_tstrcmpi(childName, _T("lastBuildDate"))) {
							LPCTSTR szChildText = NULL;
							if (!mir_tstrcmpi(codepage, _T("koi8-r"))) {
								TCHAR buf[MAX_PATH];
								MultiByteToWideChar(20866, 0, _T2A(xmlGetText(child)), -1, buf, _countof(buf));
								szChildText = buf;
							}
							else
								szChildText = xmlGetText(child);
							if (szChildText) {
								TCHAR *lastupdtime = (TCHAR *)xmlGetText(child);
								time_t stamp = DateToUnixTime(lastupdtime, 0);
								double deltaupd = difftime(time(NULL), stamp);
								double deltacheck = difftime(time(NULL), (time_t)db_get_dw(hContact, MODULE, "LastCheck", 0));
								if (deltaupd - deltacheck >= 0) {
									db_set_dw(hContact, MODULE, "LastCheck", (DWORD)time(NULL));
									xmlDestroyNode(hXml);
									return;
								}
							}
						}
						else if (!mir_tstrcmpi(childName, _T("item"))) {
							CMString title, link, descr, author, comments, guid, category;
							time_t stamp = 0;
							for (int z = 0; z < xmlGetChildCount(child); z++) {
								HXML itemval = xmlGetChild(child, z);
								LPCTSTR itemName = xmlGetName(itemval);
								LPCTSTR value = NULL;
								if (!mir_tstrcmpi(codepage, _T("koi8-r"))) {
									TCHAR buf[MAX_PATH];
									MultiByteToWideChar(20866, 0, _T2A(xmlGetText(itemval)), -1, buf, _countof(buf));
									value = buf;
								}
								else
									value = xmlGetText(itemval);

								// We only use the first tag for now and ignore the rest.
								if (!mir_tstrcmpi(itemName, _T("title")))
									ClearText(title, value);

								else if (!mir_tstrcmpi(itemName, _T("link")))
									ClearText(link, value);

								else if (!mir_tstrcmpi(itemName, _T("pubDate")) || !mir_tstrcmpi(itemName, _T("date"))) {
									if (stamp == 0)
										stamp = DateToUnixTime(value, 0);
								}
								else if (!mir_tstrcmpi(itemName, _T("description")) || !mir_tstrcmpi(itemName, _T("encoded")))
									ClearText(descr, value);

								else if (!mir_tstrcmpi(itemName, _T("author")) || !mir_tstrcmpi(itemName, _T("creator")))
									ClearText(author, value);

								else if (!mir_tstrcmpi(itemName, _T("comments")))
									ClearText(comments, value);

								else if (!mir_tstrcmpi(itemName, _T("guid")))
									ClearText(guid, value);

								else if (!mir_tstrcmpi(itemName, _T("category")))
									ClearText(category, value);
							}

							XmlToMsg(hContact, title, link, descr, author, comments, guid, category, stamp);
						}
					}
				}
				else if (!mir_tstrcmpi(szNodeName, _T("feed"))) {
					db_set_ts(hContact, MODULE, "MirVer", _T("Atom 3"));
					for (int j = 0; j < xmlGetChildCount(node); j++) {
						HXML child = xmlGetChild(node, j);
						LPCTSTR szChildName = xmlGetName(child);
						if (!mir_tstrcmpi(szChildName, _T("title"))) {
							LPCTSTR szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "FirstName", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(szChildName, _T("link"))) {
							for (int x = 0; x < xmlGetAttrCount(child); x++) {
								if (!mir_tstrcmpi(xmlGetAttrName(child, x), _T("rel")))
									if (!mir_tstrcmpi(xmlGetAttrValue(child, xmlGetAttrName(child, x)), _T("self")))
										break;

								if (!mir_tstrcmpi(xmlGetAttrName(child, x), _T("href")))
									db_set_ts(hContact, MODULE, "Homepage", xmlGetAttrValue(child, xmlGetAttrName(child, x)));
							}
						}
						else if (!mir_tstrcmpi(szChildName, _T("subtitle"))) {
							LPCTSTR szChildText = xmlGetText(child);
							if (szChildText) {
								ClearText(szValue, szChildText);
								db_set_ts(hContact, MODULE, "About", szValue);
								db_set_ts(hContact, "CList", "StatusMsg", szValue);
							}
						}
						else if (!mir_tstrcmpi(szChildName, _T("language"))) {
							LPCTSTR szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Language1", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(szChildName, _T("author"))) {
							for (int x = 0; x < xmlGetChildCount(child); x++) {
								HXML authorval = xmlGetChild(child, x);
								if (!mir_tstrcmpi(xmlGetName(authorval), _T("email"))) {
									db_set_ts(hContact, MODULE, "e-mail", xmlGetText(authorval));
									break;
								}
							}
						}
						else if (!mir_tstrcmpi(szChildName, _T("category"))) {
							LPCTSTR szChildText = xmlGetText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Interest0Text", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(szChildName, _T("icon"))) {
							for (int x = 0; x < xmlGetChildCount(child); x++) {
								HXML imageval = xmlGetChild(child, x);
								if (!mir_tstrcmpi(xmlGetName(imageval), _T("url"))) {
									LPCTSTR url = xmlGetText(imageval);
									db_set_ts(hContact, MODULE, "ImageURL", url);

									ptrT szNick(db_get_tsa(hContact, MODULE, "Nick"));
									if (szNick) {
										PROTO_AVATAR_INFORMATION ai = { 0 };
										ai.hContact = hContact;
										TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
										ai.format = ProtoGetAvatarFormat(ext);

										TCHAR *filename = szNick;
										mir_sntprintf(ai.filename, _countof(ai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
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
						else if (!mir_tstrcmpi(szChildName, _T("updated"))) {
							LPCTSTR szChildText = xmlGetText(child);
							if (szChildText) {
								TCHAR *lastupdtime = (TCHAR *)szChildText;
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
						else if (!mir_tstrcmpi(szChildName, _T("entry"))) {
							CMString title, link, descr, author, comments, guid, category;
							time_t stamp = 0;
							for (int z = 0; z < xmlGetChildCount(child); z++) {
								HXML itemval = xmlGetChild(child, z);
								LPCTSTR szItemName = xmlGetName(itemval);
								if (!mir_tstrcmpi(szItemName, _T("title"))) {
									LPCTSTR szItemText = xmlGetText(itemval);
									if (szItemText)
										ClearText(title, szItemText);
								}
								else if (!mir_tstrcmpi(szItemName, _T("link"))) {
									for (int x = 0; x < xmlGetAttrCount(itemval); x++) {
										if (!mir_tstrcmpi(xmlGetAttrName(itemval, x), _T("href"))) {
											ClearText(link, xmlGetAttrValue(itemval, xmlGetAttrName(itemval, x)));
											break;
										}
									}
								}
								else if (!mir_tstrcmpi(szItemName, _T("updated"))) {
									if (stamp == 0)
										stamp = DateToUnixTime(xmlGetText(itemval), 0);
								}
								else if (!mir_tstrcmpi(szItemName, _T("summary")) || !mir_tstrcmpi(szItemName, _T("content"))) {
									LPCTSTR szItemText = xmlGetText(itemval);
									if (szItemText)
										ClearText(descr, szItemText);
								}
								else if (!mir_tstrcmpi(szItemName, _T("author"))) {
									for (int x = 0; x < xmlGetChildCount(itemval); x++) {
										HXML authorval = xmlGetChild(itemval, x);
										if (!mir_tstrcmpi(xmlGetName(authorval), _T("name")) && xmlGetText(authorval)) {
											ClearText(author, xmlGetText(authorval));
											break;
										}
									}
								}
								else if (!mir_tstrcmpi(szItemName, _T("comments"))) {
									LPCTSTR szItemText = xmlGetText(itemval);
									if (szItemText)
										ClearText(comments, szItemText);
								}
								else if (!mir_tstrcmpi(szItemName, _T("id"))) {
									LPCTSTR szItemText = xmlGetText(itemval);
									if (szItemText)
										ClearText(guid, xmlGetText(itemval));
								}
								else if (!mir_tstrcmpi(szItemName, _T("category"))) {
									for (int x = 0; x < xmlGetAttrCount(itemval); x++) {
										LPCTSTR szAttrName = xmlGetAttrName(itemval, x);
										if (!mir_tstrcmpi(szAttrName, _T("term")) && xmlGetText(itemval)) {
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

	TCHAR *szURL = db_get_tsa(hContact, MODULE, "URL");
	if (szURL == NULL)
		return;

	char *szData = NULL;
	GetNewsData(szURL, &szData, hContact, NULL);
	mir_free(szURL);

	if (szData == NULL)
		return;

	TCHAR *tszData = mir_utf8decodeT(szData);
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
		if (!mir_tstrcmpi(szNodeName, _T("rss")) || !mir_tstrcmpi(szNodeName, _T("rdf"))) {
			HXML chan = xmlGetChild(node, 0);
			for (int j = 0; j < xmlGetChildCount(chan); j++) {
				HXML child = xmlGetChild(chan, j);
				if (!mir_tstrcmpi(xmlGetName(child), _T("image"))) {
					for (int x = 0; x < xmlGetChildCount(child); x++) {
						HXML imageval = xmlGetChild(child, x);
						if (!mir_tstrcmpi(xmlGetName(imageval), _T("url"))) {
							LPCTSTR url = xmlGetText(imageval);
							db_set_ts(hContact, MODULE, "ImageURL", url);

							PROTO_AVATAR_INFORMATION ai = { 0 };
							ai.hContact = hContact;

							TCHAR *szNick = db_get_tsa(hContact, MODULE, "Nick");
							if (szNick) {
								TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
								ai.format = ProtoGetAvatarFormat(ext);

								TCHAR *filename = szNick;
								mir_sntprintf(ai.filename, _countof(ai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
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
		else if (!mir_tstrcmpi(szNodeName, _T("feed"))) {
			for (int j = 0; j < xmlGetChildCount(node); j++) {
				HXML child = xmlGetChild(node, j);
				if (!mir_tstrcmpi(xmlGetName(child), _T("icon"))) {
					for (int x = 0; x < xmlGetChildCount(child); x++) {
						HXML imageval = xmlGetChild(child, x);
						if (!mir_tstrcmpi(xmlGetName(imageval), _T("url"))) {
							LPCTSTR url = xmlGetText(imageval);
							db_set_ts(hContact, MODULE, "ImageURL", url);

							ptrT szNick(db_get_tsa(hContact, MODULE, "Nick"));
							if (szNick) {
								PROTO_AVATAR_INFORMATION ai = { 0 };
								ai.hContact = hContact;

								TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
								ai.format = ProtoGetAvatarFormat(ext);

								TCHAR *filename = szNick;
								mir_sntprintf(ai.filename, _countof(ai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
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
