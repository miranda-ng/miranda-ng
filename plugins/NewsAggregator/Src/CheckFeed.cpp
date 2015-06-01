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
			if (!mir_tstrcmpi(xi.getName(hXml), _T("xml")))
				node = xi.getChild(hXml, childcount);
			else
				node = hXml;
			while (node) {
				LPCTSTR szNodeName = xi.getName(node);
				if (!mir_tstrcmpi(szNodeName, _T("rss")) || !mir_tstrcmpi(szNodeName, _T("rdf"))) {
					HXML chan = xi.getChild(node, 0);
					for (int j = 0; j < xi.getChildCount(chan); j++) {
						HXML child = xi.getChild(chan, j);
						if (!mir_tstrcmpi(xi.getName(child), _T("title"))) {
							TCHAR mes[MAX_PATH];
							mir_sntprintf(mes, SIZEOF(mes), TranslateT("%s\nis a valid feed's address."), tszURL);
							MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONINFORMATION);
							TCHAR *tszTitle = (TCHAR *)xi.getText(child);
							return tszTitle;
						}
					}
				}
				else if (!mir_tstrcmpi(szNodeName, _T("feed"))) {
					for (int j = 0; j < xi.getChildCount(node); j++) {
						HXML child = xi.getChild(node, j);
						if (!mir_tstrcmpi(xi.getName(child), _T("title"))) {
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
		HXML hXml = xi.parseString(tszData, &bytesParsed, NULL);
		mir_free(tszData);
		mir_free(szData);

		CMString szValue;
		if (hXml != NULL) {
			int childcount = 0;
			HXML node;
			if (!mir_tstrcmpi(xi.getName(hXml), _T("xml")))
				node = xi.getChild(hXml, childcount);
			else
				node = hXml;
			while (node) {
				LPCTSTR szNodeName = xi.getName(node);
				bool isRSS = !mir_tstrcmpi(szNodeName, _T("rss")), isAtom = !mir_tstrcmpi(szNodeName, _T("rdf"));
				if (isRSS || isAtom) {
					if (isRSS) {
						for (int i = 0; i < xi.getAttrCount(node); i++) {
							LPCTSTR szAttrName = xi.getAttrName(node, i);
							if (!mir_tstrcmpi(szAttrName, _T("version"))) {
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
						if (!mir_tstrcmpi(childName, _T("title"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "FirstName", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, _T("link"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Homepage", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, _T("description"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText) {
								ClearText(szValue, szChildText);
								db_set_ts(hContact, MODULE, "About", szValue);
								db_set_ts(hContact, "CList", "StatusMsg", szValue);
							}
						}
						else if (!mir_tstrcmpi(childName, _T("language"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Language1", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, _T("managingEditor"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "e-mail", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, _T("category"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Interest0Text", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(childName, _T("copyright"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_s(hContact, "UserInfo", "MyNotes", _T2A(ClearText(szValue, szChildText)));
						}
						else if (!mir_tstrcmpi(childName, _T("image"))) {
							for (int x = 0; x < xi.getChildCount(child); x++) {
								HXML imageval = xi.getChild(child, x);
								if (!mir_tstrcmpi(xi.getName(imageval), _T("url"))) {
									LPCTSTR url = xi.getText(imageval);
									db_set_ts(hContact, MODULE, "ImageURL", url);

									PROTO_AVATAR_INFORMATION pai = { NULL };
									pai.hContact = hContact;

									TCHAR *szNick = db_get_tsa(hContact, MODULE, "Nick");
									if (szNick) {
										TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
										pai.format = ProtoGetAvatarFormat(url);

										CMString filename = szNick;
										filename.Replace(_T("/"), _T("_"));
										mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename.c_str(), ext);
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
						else if (!mir_tstrcmpi(childName, _T("lastBuildDate"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText) {
								TCHAR *lastupdtime = (TCHAR *)xi.getText(child);
								time_t stamp = DateToUnixTime(lastupdtime, 0);
								double deltaupd = difftime(time(NULL), stamp);
								double deltacheck = difftime(time(NULL), (time_t)db_get_dw(hContact, MODULE, "LastCheck", 0));
								if (deltaupd - deltacheck >= 0) {
									db_set_dw(hContact, MODULE, "LastCheck", (DWORD)time(NULL));
									xi.destroyNode(hXml);
									return;
								}
							}
						}
						else if (!mir_tstrcmpi(childName, _T("item"))) {
							CMString title, link, descr, author, comments, guid, category;
							time_t stamp = 0;
							for (int z = 0; z < xi.getChildCount(child); z++) {
								HXML itemval = xi.getChild(child, z);
								LPCTSTR itemName = xi.getName(itemval);
								// We only use the first tag for now and ignore the rest.
								if (!mir_tstrcmpi(itemName, _T("title")))
									ClearText(title, xi.getText(itemval));

								else if (!mir_tstrcmpi(itemName, _T("link")))
									ClearText(link, xi.getText(itemval));

								else if (!mir_tstrcmpi(itemName, _T("pubDate")) || !mir_tstrcmpi(itemName, _T("date"))) {
									if (stamp == 0)
										stamp = DateToUnixTime(xi.getText(itemval), 0);
								}
								else if (!mir_tstrcmpi(itemName, _T("description")) || !mir_tstrcmpi(itemName, _T("encoded")))
									ClearText(descr, xi.getText(itemval));

								else if (!mir_tstrcmpi(itemName, _T("author")) || !mir_tstrcmpi(itemName, _T("creator")))
									ClearText(author, xi.getText(itemval));

								else if (!mir_tstrcmpi(itemName, _T("comments")))
									ClearText(comments, xi.getText(itemval));

								else if (!mir_tstrcmpi(itemName, _T("guid")))
									ClearText(guid, xi.getText(itemval));

								else if (!mir_tstrcmpi(itemName, _T("category")))
									ClearText(category, xi.getText(itemval));
							}

							XmlToMsg(hContact, title, link, descr, author, comments, guid, category, stamp);
						}
					}
				}
				else if (!mir_tstrcmpi(szNodeName, _T("feed"))) {
					db_set_ts(hContact, MODULE, "MirVer", _T("Atom 3"));
					for (int j = 0; j < xi.getChildCount(node); j++) {
						HXML child = xi.getChild(node, j);
						LPCTSTR szChildName = xi.getName(child);
						if (!mir_tstrcmpi(szChildName, _T("title"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "FirstName", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(szChildName, _T("link"))) {
							for (int x = 0; x < xi.getAttrCount(child); x++) {
								if (!mir_tstrcmpi(xi.getAttrName(child, x), _T("rel")))
									if (!mir_tstrcmpi(xi.getAttrValue(child, xi.getAttrName(child, x)), _T("self")))
										break;

								if (!mir_tstrcmpi(xi.getAttrName(child, x), _T("href")))
									db_set_ts(hContact, MODULE, "Homepage", xi.getAttrValue(child, xi.getAttrName(child, x)));
							}
						}
						else if (!mir_tstrcmpi(szChildName, _T("subtitle"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText) {
								ClearText(szValue, szChildText);
								db_set_ts(hContact, MODULE, "About", szValue);
								db_set_ts(hContact, "CList", "StatusMsg", szValue);
							}
						}
						else if (!mir_tstrcmpi(szChildName, _T("language"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Language1", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(szChildName, _T("author"))) {
							for (int x = 0; x < xi.getChildCount(child); x++) {
								HXML authorval = xi.getChild(child, x);
								if (!mir_tstrcmpi(xi.getName(authorval), _T("email"))) {
									db_set_ts(hContact, MODULE, "e-mail", xi.getText(authorval));
									break;
								}
							}
						}
						else if (!mir_tstrcmpi(szChildName, _T("category"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText)
								db_set_ts(hContact, MODULE, "Interest0Text", ClearText(szValue, szChildText));
						}
						else if (!mir_tstrcmpi(szChildName, _T("icon"))) {
							for (int x = 0; x < xi.getChildCount(child); x++) {
								HXML imageval = xi.getChild(child, x);
								if (!mir_tstrcmpi(xi.getName(imageval), _T("url"))) {
									LPCTSTR url = xi.getText(imageval);
									db_set_ts(hContact, MODULE, "ImageURL", url);

									TCHAR *szNick = db_get_tsa(hContact, MODULE, "Nick");
									if (szNick) {
										PROTO_AVATAR_INFORMATION pai = { NULL };
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
						else if (!mir_tstrcmpi(szChildName, _T("updated"))) {
							LPCTSTR szChildText = xi.getText(child);
							if (szChildText) {
								TCHAR *lastupdtime = (TCHAR *)szChildText;
								time_t stamp = DateToUnixTime(lastupdtime, 1);
								double deltaupd = difftime(time(NULL), stamp);
								double deltacheck = difftime(time(NULL), (time_t)db_get_dw(hContact, MODULE, "LastCheck", 0));
								if (deltaupd - deltacheck >= 0) {
									db_set_dw(hContact, MODULE, "LastCheck", (DWORD)time(NULL));
									xi.destroyNode(hXml);
									return;
								}
							}
						}
						else if (!mir_tstrcmpi(szChildName, _T("entry"))) {
							CMString title, link, descr, author, comments, guid, category;
							time_t stamp = 0;
							for (int z = 0; z < xi.getChildCount(child); z++) {
								HXML itemval = xi.getChild(child, z);
								LPCTSTR szItemName = xi.getName(itemval);
								if (!mir_tstrcmpi(szItemName, _T("title"))) {
									LPCTSTR szItemText = xi.getText(itemval);
									if (szItemText)
										ClearText(title, szItemText);
								}
								else if (!mir_tstrcmpi(szItemName, _T("link"))) {
									for (int x = 0; x < xi.getAttrCount(itemval); x++) {
										if (!mir_tstrcmpi(xi.getAttrName(itemval, x), _T("href"))) {
											ClearText(link, xi.getAttrValue(itemval, xi.getAttrName(itemval, x)));
											break;
										}
									}
								}
								else if (!mir_tstrcmpi(szItemName, _T("updated"))) {
									if (stamp == 0)
										stamp = DateToUnixTime(xi.getText(itemval), 0);
								}
								else if (!mir_tstrcmpi(szItemName, _T("summary")) || !mir_tstrcmpi(szItemName, _T("content"))) {
									LPCTSTR szItemText = xi.getText(itemval);
									if (szItemText)
										ClearText(descr, szItemText);
								}
								else if (!mir_tstrcmpi(szItemName, _T("author"))) {
									for (int x = 0; x < xi.getChildCount(itemval); x++) {
										HXML authorval = xi.getChild(itemval, x);
										if (!mir_tstrcmpi(xi.getName(authorval), _T("name")) && xi.getText(authorval)) {
											ClearText(author, xi.getText(authorval));
											break;
										}
									}
								}
								else if (!mir_tstrcmpi(szItemName, _T("comments"))) {
									LPCTSTR szItemText = xi.getText(itemval);
									if (szItemText)
										ClearText(comments, szItemText);
								}
								else if (!mir_tstrcmpi(szItemName, _T("id"))) {
									LPCTSTR szItemText = xi.getText(itemval);
									if (szItemText)
										ClearText(guid, xi.getText(itemval));
								}
								else if (!mir_tstrcmpi(szItemName, _T("category"))) {
									for (int x = 0; x < xi.getAttrCount(itemval); x++) {
										LPCTSTR szAttrName = xi.getAttrName(itemval, x);
										if (!mir_tstrcmpi(szAttrName, _T("term")) && xi.getText(itemval)) {
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
	HXML hXml = xi.parseString(tszData, &bytesParsed, NULL);
	mir_free(tszData);
	mir_free(szData);
	if (hXml == NULL)
		return;

	int childcount = 0;
	HXML node = xi.getChild(hXml, childcount);
	while (node) {
		LPCTSTR szNodeName = xi.getName(node);
		if (!mir_tstrcmpi(szNodeName, _T("rss")) || !mir_tstrcmpi(szNodeName, _T("rdf"))) {
			HXML chan = xi.getChild(node, 0);
			for (int j = 0; j < xi.getChildCount(chan); j++) {
				HXML child = xi.getChild(chan, j);
				if (!mir_tstrcmpi(xi.getName(child), _T("image"))) {
					for (int x = 0; x < xi.getChildCount(child); x++) {
						HXML imageval = xi.getChild(child, x);
						if (!mir_tstrcmpi(xi.getName(imageval), _T("url"))) {
							LPCTSTR url = xi.getText(imageval);
							db_set_ts(hContact, MODULE, "ImageURL", url);

							PROTO_AVATAR_INFORMATION pai = { NULL };
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
		else if (!mir_tstrcmpi(szNodeName, _T("feed"))) {
			for (int j = 0; j < xi.getChildCount(node); j++) {
				HXML child = xi.getChild(node, j);
				if (!mir_tstrcmpi(xi.getName(child), _T("icon"))) {
					for (int x = 0; x < xi.getChildCount(child); x++) {
						HXML imageval = xi.getChild(child, x);
						if (!mir_tstrcmpi(xi.getName(imageval), _T("url"))) {
							LPCTSTR url = xi.getText(imageval);
							db_set_ts(hContact, MODULE, "ImageURL", url);

							TCHAR *szNick = db_get_tsa(hContact, MODULE, "Nick");
							if (szNick) {
								PROTO_AVATAR_INFORMATION pai = { NULL };
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
