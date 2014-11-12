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

TCHAR* CheckFeed(TCHAR *tszURL, HWND hwndDlg)
{
	Netlib_LogfT(hNetlibUser, _T("Started validating feed %s."), tszURL);
	char *szData = NULL;
	GetNewsData(tszURL, &szData, NULL, hwndDlg);
	if (szData) {
		TCHAR *tszData = mir_utf8decodeT(szData);
		if (!tszData)
			tszData = mir_a2t(szData);
		int bytesParsed = 0;
		HXML hXml = xi.parseString(tszData, &bytesParsed, NULL);
		mir_free(tszData);
		mir_free(szData);
		if (hXml != NULL) {
			int childcount = 0;
			HXML node;
			if (!lstrcmpi(xi.getName(hXml), _T("xml")))
				node = xi.getChild(hXml, childcount);
			else
				node = hXml;
			while (node) {
				LPCTSTR szNodeName = xi.getName(node);
				if (!lstrcmpi(szNodeName, _T("rss")) || !lstrcmpi(szNodeName, _T("rdf"))) {
					HXML chan = xi.getChild(node, 0);
					for (int j = 0; j < xi.getChildCount(chan); j++) {
						HXML child = xi.getChild(chan, j);
						if (!lstrcmpi(xi.getName(child), _T("title"))) {
							TCHAR mes[MAX_PATH];
							mir_sntprintf(mes, SIZEOF(mes), TranslateT("%s\nis a valid feed's address."), tszURL);
							MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONINFORMATION);
							TCHAR *tszTitle = (TCHAR *)xi.getText(child);
							return tszTitle;
						}
					}
				}
				else if (!lstrcmpi(szNodeName, _T("feed"))) {
					for (int j = 0; j < xi.getChildCount(node); j++) {
						HXML child = xi.getChild(node, j);
						if (!lstrcmpi(xi.getName(child), _T("title"))) {
							TCHAR mes[MAX_PATH];
							mir_sntprintf(mes, SIZEOF(mes), TranslateT("%s\nis a valid feed's address."), tszURL);
							MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONINFORMATION);
							TCHAR *tszTitle = (TCHAR *)xi.getText(child);
							return tszTitle;
						}
					}
				}
				node = xi.getChild(hXml, ++childcount);
			}
		}
		xi.destroyNode(hXml);
	}
	Netlib_LogfT(hNetlibUser, _T("%s is not a valid feed's address."), tszURL);
	TCHAR mes[MAX_PATH];
	mir_sntprintf(mes, SIZEOF(mes), TranslateT("%s\nis not a valid feed's address."), tszURL);
	MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONERROR);
	return NULL;
}

static void XmlToMsg(MCONTACT hContact, CMString &title, CMString &link, CMString &descr, CMString &author, CMString &comments, CMString &guid, CMString &category, time_t stamp)
{
	TCHAR *message = db_get_tsa(hContact, MODULE, "MsgFormat");
	if (!message)
		message = mir_tstrdup(TAGSDEFAULT);

	if (title.IsEmpty())
		StrReplace(_T("#<title>#"), TranslateT("empty"), message);
	else
		StrReplace(_T("#<title>#"), title, message);

	if (link.IsEmpty())
		StrReplace(_T("#<link>#"), TranslateT("empty"), message);
	else
		StrReplace(_T("#<link>#"), link, message);

	if (descr.IsEmpty())
		StrReplace(_T("#<description>#"), TranslateT("empty"), message);
	else
		StrReplace(_T("#<description>#"), descr, message);

	if (author.IsEmpty())
		StrReplace(_T("#<author>#"), TranslateT("empty"), message);
	else
		StrReplace(_T("#<author>#"), author, message);

	if (comments.IsEmpty())
		StrReplace(_T("#<comments>#"), TranslateT("empty"), message);
	else
		StrReplace(_T("#<comments>#"), comments, message);

	if (guid.IsEmpty())
		StrReplace(_T("#<guid>#"), TranslateT("empty"), message);
	else
		StrReplace(_T("#<guid>#"), guid, message);

	if (category.IsEmpty())
		StrReplace(_T("#<category>#"), TranslateT("empty"), message);
	else
		StrReplace(_T("#<category>#"), category, message);

	if (stamp == 0)
		stamp = time(NULL);

	DBEVENTINFO olddbei = { 0 };
	olddbei.cbSize = sizeof(olddbei);

	bool  MesExist = false;
	ptrA  pszTemp(mir_utf8encodeT(message));
	DWORD cbMemoLen = 10000, cbOrigLen = lstrlenA(pszTemp);
	BYTE *pbBuffer = (BYTE*)mir_alloc(cbMemoLen);
	for (HANDLE hDbEvent = db_event_last(hContact); hDbEvent; hDbEvent = db_event_prev(hContact, hDbEvent)) {
		olddbei.cbBlob = db_event_getBlobSize(hDbEvent);
		if (olddbei.cbBlob > cbMemoLen)
			pbBuffer = (PBYTE)mir_realloc(pbBuffer, cbMemoLen = olddbei.cbBlob);
		olddbei.pBlob = pbBuffer;
		db_event_get(hDbEvent, &olddbei);

		// there's no need to look for the elder events
		if (olddbei.timestamp < stamp)
			break;

		if (strlen((char*)olddbei.pBlob) == cbOrigLen && !lstrcmpA((char*)olddbei.pBlob, pszTemp)) {
			MesExist = true;
			break;
		}
	}
	mir_free(pbBuffer);

	if (!MesExist) {
		PROTORECVEVENT recv = { 0 };
		recv.flags = PREF_TCHAR;
		recv.timestamp = stamp;
		recv.tszMessage = message;
		ProtoChainRecvMsg(hContact, &recv);
	}
	mir_free(message);
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
		HXML hXml = xi.parseString(tszData, &bytesParsed, NULL);
		mir_free(tszData);
		mir_free(szData);

		CMString szValue;
		if (hXml != NULL) {
			int childcount = 0;
			HXML node;
			if (!lstrcmpi(xi.getName(hXml), _T("xml")))
				node = xi.getChild(hXml, childcount);
			else
				node = hXml;
			while (node) {
				LPCTSTR szNodeName = xi.getName(node);
				bool isRSS = !lstrcmpi(szNodeName, _T("rss")), isAtom = !lstrcmpi(szNodeName, _T("rdf"));
				if (isRSS || isAtom) {
					if (isRSS) {
						for (int i = 0; i < xi.getAttrCount(node); i++) {
							LPCTSTR szAttrName = xi.getAttrName(node, i);
							if (!lstrcmpi(szAttrName, _T("version"))) {
								TCHAR ver[MAX_PATH];
								mir_sntprintf(ver, SIZEOF(ver), _T("RSS %s"), xi.getAttrValue(node, szAttrName));
								db_set_ts(hContact, MODULE, "MirVer", ver);
								break;
							}
						}
					}
					else if (isAtom)
						db_set_ts(hContact, MODULE, "MirVer", _T("RSS 1.0"));

					HXML chan = xi.getChild(node, 0);
					for (int j = 0; j < xi.getChildCount(chan); j++) {
						HXML child = xi.getChild(chan, j);
						LPCTSTR childName = xi.getName(child);
						if (!lstrcmpi(childName, _T("title"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "FirstName", ClearText(szValue, szChildText));
						}
						else if (!lstrcmpi(childName, _T("link"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Homepage", ClearText(szValue, szChildText));
						}
						else if (!lstrcmpi(childName, _T("description"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText) {
								ClearText(szValue, szChildText);
								db_set_ts(hContact, MODULE, "About", szValue);
								db_set_ts(hContact, "CList", "StatusMsg", szValue);
							}
						}
						else if (!lstrcmpi(childName, _T("language"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Language1", ClearText(szValue, szChildText));
						}
						else if (!lstrcmpi(childName, _T("managingEditor"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "e-mail", ClearText(szValue, szChildText));
						}
						else if (!lstrcmpi(childName, _T("category"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Interest0Text", ClearText(szValue, szChildText));
						}
						else if (!lstrcmpi(childName, _T("copyright"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_s(hContact, "UserInfo", "MyNotes", _T2A(ClearText(szValue, szChildText)));
						}
						else if (!lstrcmpi(childName, _T("image"))) {
							for (int x = 0; x < xi.getChildCount(child); x++) {
								HXML imageval = xi.getChild(child, x);
								if (!lstrcmpi(xi.getName(imageval), _T("url"))) {
									LPCTSTR url = xi.getText(imageval);
									db_set_ts(hContact, MODULE, "ImageURL", url);

									PROTO_AVATAR_INFORMATIONT pai = { NULL };
									pai.cbSize = sizeof(pai);
									pai.hContact = hContact;

									TCHAR *szNick = db_get_tsa(hContact, MODULE, "Nick");
									if (szNick) {
										TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
										pai.format = ProtoGetAvatarFormat(url);

										TCHAR *filename = szNick;
										StrReplace(_T("/"), _T("_"), filename);
										mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
										CreateDirectoryTreeT(tszRoot);
										if (DownloadFile(url, pai.filename)) {
											db_set_ts(hContact, MODULE, "ImagePath", pai.filename);
											ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, NULL);
										}
										else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&pai, NULL);
										mir_free(szNick);
										break;
									}
								}
							}
						}
						else if (!lstrcmpi(childName, _T("lastBuildDate"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText) {
								TCHAR *lastupdtime = (TCHAR *)xi.getText(child);
								time_t stamp = DateToUnixTime(lastupdtime, 0);
								double deltaupd = difftime(time(NULL), stamp);
								double deltacheck = difftime(time(NULL), db_get_dw(hContact, MODULE, "LastCheck", 0));
								if (deltaupd - deltacheck >= 0) {
									db_set_dw(hContact, MODULE, "LastCheck", time(NULL));
									xi.destroyNode(hXml);
									return;
								}
							}
						}
						else if (!lstrcmpi(childName, _T("item"))) {
							CMString title, link, descr, author, comments, guid, category;
							time_t stamp = 0;
							for (int z = 0; z < xi.getChildCount(child); z++) {
								HXML itemval = xi.getChild(child, z);
								LPCTSTR itemName = xi.getName(itemval);
								// We only use the first tag for now and ignore the rest.
								if (!lstrcmpi(itemName, _T("title")))
									ClearText(title, xi.getText(itemval));

								else if (!lstrcmpi(itemName, _T("link")))
									ClearText(link, xi.getText(itemval));

								else if (!lstrcmpi(itemName, _T("pubDate")) || !lstrcmpi(itemName, _T("date"))) {
									if (stamp == 0)
										stamp = DateToUnixTime(xi.getText(itemval), 0);
								}
								else if (!lstrcmpi(itemName, _T("description")) || !lstrcmpi(itemName, _T("encoded")))
									ClearText(descr, xi.getText(itemval));

								else if (!lstrcmpi(itemName, _T("author")) || !lstrcmpi(itemName, _T("creator")))
									ClearText(author, xi.getText(itemval));

								else if (!lstrcmpi(itemName, _T("comments")))
									ClearText(comments, xi.getText(itemval));

								else if (!lstrcmpi(itemName, _T("guid")))
									ClearText(guid, xi.getText(itemval));

								else if (!lstrcmpi(itemName, _T("category")))
									ClearText(category, xi.getText(itemval));
							}

							XmlToMsg(hContact, title, link, descr, author, comments, guid, category, stamp);
						}
					}
				}
				else if (!lstrcmpi(szNodeName, _T("feed"))) {
					db_set_ts(hContact, MODULE, "MirVer", _T("Atom 3"));
					for (int j = 0; j < xi.getChildCount(node); j++) {
						HXML child = xi.getChild(node, j);
						LPCTSTR szChildName = xi.getName(child);
						if (!lstrcmpi(szChildName, _T("title"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "FirstName", ClearText(szValue, szChildText));
						}
						else if (!lstrcmpi(szChildName, _T("link"))) {
							for (int x = 0; x < xi.getAttrCount(child); x++) {
								if (!lstrcmpi(xi.getAttrName(child, x), _T("rel")))
									if (!lstrcmpi(xi.getAttrValue(child, xi.getAttrName(child, x)), _T("self")))
										break;

								if (!lstrcmpi(xi.getAttrName(child, x), _T("href")))
									db_set_ts(hContact, MODULE, "Homepage", xi.getAttrValue(child, xi.getAttrName(child, x)));
							}
						}
						else if (!lstrcmpi(szChildName, _T("subtitle"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText) {
								ClearText(szValue, szChildText);
								db_set_ts(hContact, MODULE, "About", szValue);
								db_set_ts(hContact, "CList", "StatusMsg", szValue);
							}
						}
						else if (!lstrcmpi(szChildName, _T("language"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Language1", ClearText(szValue, szChildText));
						}
						else if (!lstrcmpi(szChildName, _T("author"))) {
							for (int x = 0; x < xi.getChildCount(child); x++) {
								HXML authorval = xi.getChild(child, x);
								if (!lstrcmpi(xi.getName(authorval), _T("email"))) {
									db_set_ts(hContact, MODULE, "e-mail", xi.getText(authorval));
									break;
								}
							}
						}
						else if (!lstrcmpi(szChildName, _T("category"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Interest0Text", ClearText(szValue, szChildText));
						}
						else if (!lstrcmpi(szChildName, _T("icon"))) {
							for (int x = 0; x < xi.getChildCount(child); x++) {
								HXML imageval = xi.getChild(child, x);
								if (!lstrcmpi(xi.getName(imageval), _T("url"))) {
									LPCTSTR url = xi.getText(imageval);
									db_set_ts(hContact, MODULE, "ImageURL", url);

									TCHAR *szNick = db_get_tsa(hContact, MODULE, "Nick");
									if (szNick) {
										PROTO_AVATAR_INFORMATIONT pai = { NULL };
										pai.cbSize = sizeof(pai);
										pai.hContact = hContact;
										TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
										pai.format = ProtoGetAvatarFormat(ext);

										TCHAR *filename = szNick;
										mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
										if (DownloadFile(url, pai.filename)) {
											db_set_ts(hContact, MODULE, "ImagePath", pai.filename);
											ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, NULL);
										}
										else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&pai, NULL);
										mir_free(szNick);
										break;
									}
								}
							}
						}
						else if (!lstrcmpi(szChildName, _T("updated"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText) {
								TCHAR *lastupdtime = (TCHAR *)szChildText;
								time_t stamp = DateToUnixTime(lastupdtime, 1);
								double deltaupd = difftime(time(NULL), stamp);
								double deltacheck = difftime(time(NULL), db_get_dw(hContact, MODULE, "LastCheck", 0));
								if (deltaupd - deltacheck >= 0) {
									db_set_dw(hContact, MODULE, "LastCheck", time(NULL));
									xi.destroyNode(hXml);
									return;
								}
							}
						}
						else if (!lstrcmpi(szChildName, _T("entry"))) {
							CMString title, link, descr, author, comments, guid, category;
							time_t stamp = 0;
							for (int z = 0; z < xi.getChildCount(child); z++) {
								HXML itemval = xi.getChild(child, z);
								LPCTSTR szItemName = xi.getName(itemval);
								if (!lstrcmpi(szItemName, _T("title"))) {
									LPCTSTR szItemText = xi.getText(itemval);
									if (szItemText)
										ClearText(title, szItemText);
								}
								else if (!lstrcmpi(szItemName, _T("link"))) {
									for (int x = 0; x < xi.getAttrCount(itemval); x++) {
										if (!lstrcmpi(xi.getAttrName(itemval, x), _T("href"))) {
											ClearText(link, xi.getAttrValue(itemval, xi.getAttrName(itemval, x)));
											break;
										}
									}
								}
								else if (!lstrcmpi(szItemName, _T("updated"))) {
									if (stamp == 0)
										stamp = DateToUnixTime(xi.getText(itemval), 0);
								}
								else if (!lstrcmpi(szItemName, _T("summary")) || !lstrcmpi(szItemName, _T("content"))) {
									LPCTSTR szItemText = xi.getText(itemval);
									if (szItemText)
										ClearText(descr, szItemText);
								}
								else if (!lstrcmpi(szItemName, _T("author"))) {
									for (int x = 0; x < xi.getChildCount(itemval); x++) {
										HXML authorval = xi.getChild(itemval, x);
										if (!lstrcmpi(xi.getName(authorval), _T("name")) && xi.getText(authorval)) {
											ClearText(author, xi.getText(authorval));
											break;
										}
									}
								}
								else if (!lstrcmpi(szItemName, _T("comments"))) {
									LPCTSTR szItemText = xi.getText(itemval);
									if (szItemText)
										ClearText(comments, szItemText);
								}
								else if (!lstrcmpi(szItemName, _T("id"))) {
									LPCTSTR szItemText = xi.getText(itemval);
									if (szItemText)
										ClearText(guid, xi.getText(itemval));
								}
								else if (!lstrcmpi(szItemName, _T("category"))) {
									for (int x = 0; x < xi.getAttrCount(itemval); x++) {
										LPCTSTR szAttrName = xi.getAttrName(itemval, x);
										if (!lstrcmpi(szAttrName, _T("term")) && xi.getText(itemval)) {
											ClearText(category, xi.getAttrValue(itemval, szAttrName));
											break;
										}
									}
								}
							}

							XmlToMsg(hContact, title, link, descr, author, comments, guid, category, stamp);
						}
					}
				}
				node = xi.getChild(hXml, ++childcount);
			}
			xi.destroyNode(hXml);
		}
	}
	db_set_dw(hContact, MODULE, "LastCheck", time(NULL));
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
	HXML hXml = xi.parseString(tszData, &bytesParsed, NULL);
	mir_free(tszData);
	mir_free(szData);
	if (hXml == NULL)
		return;

	int childcount = 0;
	HXML node = xi.getChild(hXml, childcount);
	while (node) {
		LPCTSTR szNodeName = xi.getName(node);
		if (!lstrcmpi(szNodeName, _T("rss")) || !lstrcmpi(szNodeName, _T("rdf"))) {
			HXML chan = xi.getChild(node, 0);
			for (int j = 0; j < xi.getChildCount(chan); j++) {
				HXML child = xi.getChild(chan, j);
				if (!lstrcmpi(xi.getName(child), _T("image"))) {
					for (int x = 0; x < xi.getChildCount(child); x++) {
						HXML imageval = xi.getChild(child, x);
						if (!lstrcmpi(xi.getName(imageval), _T("url"))) {
							LPCTSTR url = xi.getText(imageval);
							db_set_ts(hContact, MODULE, "ImageURL", url);

							PROTO_AVATAR_INFORMATIONT pai = { NULL };
							pai.cbSize = sizeof(pai);
							pai.hContact = hContact;

							TCHAR *szNick = db_get_tsa(hContact, MODULE, "Nick");
							if (szNick) {
								TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
								pai.format = ProtoGetAvatarFormat(ext);

								TCHAR *filename = szNick;
								mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
								if (DownloadFile(url, pai.filename)) {
									db_set_ts(hContact, MODULE, "ImagePath", pai.filename);
									ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, NULL);
								}
								else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&pai, NULL);
								mir_free(szNick);
								break;
							}
						}
					}
				}
			}
		}
		else if (!lstrcmpi(szNodeName, _T("feed"))) {
			for (int j = 0; j < xi.getChildCount(node); j++) {
				HXML child = xi.getChild(node, j);
				if (!lstrcmpi(xi.getName(child), _T("icon"))) {
					for (int x = 0; x < xi.getChildCount(child); x++) {
						HXML imageval = xi.getChild(child, x);
						if (!lstrcmpi(xi.getName(imageval), _T("url"))) {
							LPCTSTR url = xi.getText(imageval);
							db_set_ts(hContact, MODULE, "ImageURL", url);

							TCHAR *szNick = db_get_tsa(hContact, MODULE, "Nick");
							if (szNick) {
								PROTO_AVATAR_INFORMATIONT pai = { NULL };
								pai.cbSize = sizeof(pai);
								pai.hContact = hContact;

								TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
								pai.format = ProtoGetAvatarFormat(ext);

								TCHAR *filename = szNick;
								mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
								if (DownloadFile(url, pai.filename)) {
									db_set_ts(hContact, MODULE, "ImagePath", pai.filename);
									ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, NULL);
								}
								else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&pai, NULL);
								mir_free(szNick);
								break;
							}
						}
					}
				}
			}
		}
		node = xi.getChild(hXml, ++childcount);
	}
	xi.destroyNode(hXml);
}
