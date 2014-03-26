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

TCHAR * CheckFeed(TCHAR *tszURL, HWND hwndDlg)
{
	char *szData = NULL;
	DBVARIANT dbVar = {0};
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
			if ( !lstrcmpi(xi.getName(hXml), _T("xml")))
				node = xi.getChild(hXml, childcount);
			else
				node = hXml;
			while (node) {
				if (!lstrcmpi(xi.getName(node), _T("rss")) || !lstrcmpi(xi.getName(node), _T("rdf"))) {
					HXML chan = xi.getChild(node, 0);
					for (int j = 0; j < xi.getChildCount(chan); j++) {
						HXML child = xi.getChild(chan, j);
						if (!lstrcmpi(xi.getName(child), _T("title"))) {
							TCHAR mes[MAX_PATH];
							mir_sntprintf(mes, SIZEOF(mes), TranslateT("%s\nis a valid feed's address."), tszURL);
							MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK|MB_ICONINFORMATION);
							TCHAR *tszTitle = (TCHAR *)xi.getText(child);
							return tszTitle;
						}
					}
				}
				else if (!lstrcmpi(xi.getName(node), _T("feed"))) {
					for (int j = 0; j < xi.getChildCount(node); j++) {
						HXML child = xi.getChild(node, j);
						if (!lstrcmpi(xi.getName(child), _T("title"))) {
							TCHAR mes[MAX_PATH];
							mir_sntprintf(mes, SIZEOF(mes), TranslateT("%s\nis a valid feed's address."), tszURL);
							MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK|MB_ICONINFORMATION);
							TCHAR *tszTitle = (TCHAR *)xi.getText(child);
							return tszTitle;
						}
					}
				}
				childcount +=1;
				node = xi.getChild(hXml, childcount);
			}
		}
		xi.destroyNode(hXml);
	}
	TCHAR mes[MAX_PATH];
	mir_sntprintf(mes, SIZEOF(mes), TranslateT("%s\nis not a valid feed's address."), tszURL);
	MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK|MB_ICONERROR);
	return NULL;
}

VOID CheckCurrentFeed(MCONTACT hContact)
{
	char *szData = NULL;
	DBVARIANT dbURL = {0};
	if (db_get_ts(hContact, MODULE, "URL", &dbURL))
		return;

	if (db_get_b(hContact, MODULE, "CheckState", 1) != 0) {
		GetNewsData(dbURL.ptszVal, &szData, hContact, NULL);
		db_free(&dbURL);
		if (szData) {
			TCHAR *tszData = mir_utf8decodeT(szData);
			if (!tszData)
				tszData = mir_a2t(szData);
			int bytesParsed = 0;
			HXML hXml = xi.parseString(tszData, &bytesParsed, NULL);
			mir_free(tszData);
			mir_free(szData);
			if(hXml != NULL) {
				int childcount = 0;
				HXML node;
				if ( !lstrcmpi(xi.getName(hXml), _T("xml")))
					node = xi.getChild(hXml, childcount);
				else
					node = hXml;
				while (node) {
					if (!lstrcmpi(xi.getName(node), _T("rss")) || !lstrcmpi(xi.getName(node), _T("rdf"))) {
						if (!lstrcmpi(xi.getName(node), _T("rss"))) {
							for (int i = 0; i < xi.getAttrCount(node); i++) {
								if (!lstrcmpi(xi.getAttrName(node, i), _T("version"))) {
									TCHAR ver[MAX_PATH];
									mir_sntprintf(ver, SIZEOF(ver), _T("RSS %s"), xi.getAttrValue(node, xi.getAttrName(node, i)));
									db_set_ts(hContact, MODULE, "MirVer", ver);
									break;
								}
							}
						}
						else if (!lstrcmpi(xi.getName(node), _T("rdf")))
							db_set_ts(hContact, MODULE, "MirVer", _T("RSS 1.0"));

						HXML chan = xi.getChild(node, 0);
						for (int j = 0; j < xi.getChildCount(chan); j++) {
							HXML child = xi.getChild(chan, j);
							if (!lstrcmpi(xi.getName(child), _T("title")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "FirstName", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("link")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "Homepage", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("description")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "About", string);
								db_set_ts(hContact, "CList", "StatusMsg", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("language")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "Language1", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("managingEditor")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "e-mail", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("category")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "Interest0Text", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("image"))) {
								for (int x = 0; x < xi.getChildCount(child); x++) {
									HXML imageval = xi.getChild(child, x);
									if (!lstrcmpi(xi.getName(imageval), _T("url"))) {
										LPCTSTR url = xi.getText(imageval);
										db_set_ts(hContact, MODULE, "ImageURL", url);

										PROTO_AVATAR_INFORMATIONT pai = {NULL};
										pai.cbSize = sizeof(pai);
										pai.hContact = hContact;
										DBVARIANT dbVar = {0};

										if (!db_get_ts(hContact, MODULE, "Nick", &dbVar)) {
											TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
											pai.format = ProtoGetAvatarFormat(url);

											TCHAR *filename = dbVar.ptszVal;
											StrReplace(_T("/"), _T("_"), filename);
											mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
											CreateDirectoryTreeT(tszRoot);
											if (DownloadFile(url, pai.filename)) {
												db_set_ts(hContact, MODULE, "ImagePath", pai.filename);
												ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE) &pai, NULL);
											}
											else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE) &pai, NULL);
											db_free(&dbVar);
											break;
										}
									}
								}
							}
							if (!lstrcmpi(xi.getName(child), _T("lastBuildDate")) && xi.getText(child)) {
								TCHAR *lastupdtime = (TCHAR *)xi.getText(child);
								time_t stamp = DateToUnixTime(lastupdtime, 0);
								double deltaupd = difftime(time(NULL), stamp);
								double deltacheck = difftime(time(NULL), db_get_dw(hContact, MODULE, "LastCheck", 0));
								if (deltaupd - deltacheck >= 0) {
									db_set_dw(hContact, MODULE, "LastCheck", time(NULL));
									xi.destroyNode(hXml);
									return;
								}
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("item"))) {
								TCHAR *title = NULL, *link = NULL, *datetime = NULL, *descr = NULL, *author = NULL, *comments = NULL, *guid = NULL, *category = NULL;
								for (int z = 0; z < xi.getChildCount(child); z++) {
									HXML itemval = xi.getChild(child, z);
									if (!lstrcmpi(xi.getName(itemval), _T("title"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										title = mir_tstrdup(string);
										mir_free(string);
									}
									else if (!lstrcmpi(xi.getName(itemval), _T("link"))) {
										// We only use the first <link> tag for now and ignore the rest.
										if (link == NULL) {
											TCHAR *string = mir_tstrdup(xi.getText(itemval));
											ClearText(string);
											link = mir_tstrdup(string);
											mir_free(string);
										}
									}
									else if (!lstrcmpi(xi.getName(itemval), _T("pubDate"))) {
										datetime = (TCHAR *)xi.getText(itemval);
									}
									else if (!lstrcmpi(xi.getName(itemval), _T("dc:date"))) {
										datetime = (TCHAR *)xi.getText(itemval);
									}
									else if (!lstrcmpi(xi.getName(itemval), _T("description"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										descr = mir_tstrdup(string);
										mir_free(string);
									}
									else if (!lstrcmpi(xi.getName(itemval), _T("author"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										author = mir_tstrdup(string);
										mir_free(string);
									}
									else if (!lstrcmpi(xi.getName(itemval), _T("comments"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										comments = mir_tstrdup(string);
										mir_free(string);
									}
									else if (!lstrcmpi(xi.getName(itemval), _T("guid"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										guid = mir_tstrdup(string);
										mir_free(string);
									}
									else if (!lstrcmpi(xi.getName(itemval), _T("category"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										category = mir_tstrdup(string);
										mir_free(string);
									}
								}
								TCHAR *message;
								DBVARIANT dbMsg = {0};
								if (db_get_ts(hContact, MODULE, "MsgFormat", &dbMsg))
									message = _T(TAGSDEFAULT);
								else
									message = mir_tstrdup(dbMsg.ptszVal);
								db_free(&dbMsg);
								if (!title)
									StrReplace(_T("#<title>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<title>#"), title, message);
									mir_free(title);
								}
								if (!link)
									StrReplace(_T("#<link>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<link>#"), link, message);
									mir_free(link);
								}
								if (!descr)
									StrReplace(_T("#<description>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<description>#"), descr, message);
									mir_free(descr);
								}
								if (!author)
									StrReplace(_T("#<author>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<author>#"), author, message);
									mir_free(author);
								}
								if (!comments)
									StrReplace(_T("#<comments>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<comments>#"), comments, message);
									mir_free(comments);
								}
								if (!guid)
									StrReplace(_T("#<guid>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<guid>#"), guid, message);
									mir_free(guid);
								}
								if (!category)
									StrReplace(_T("#<category>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<category>#"), category, message);
									mir_free(category);
								}

								time_t stamp;
								if (!datetime)
									stamp = time(NULL);
								else
									stamp = DateToUnixTime(datetime, 0);

								HANDLE hDbEvent = db_event_first(hContact);
								BOOL MesExist = FALSE;
								while (hDbEvent) {
									DBEVENTINFO olddbei = { sizeof(olddbei) };
									olddbei.cbBlob = db_event_getBlobSize(hDbEvent);
									olddbei.pBlob = (PBYTE)mir_alloc(olddbei.cbBlob);
									db_event_get(hDbEvent, &olddbei);
									char *pszTemp = mir_utf8encodeT(message);
									if (olddbei.cbBlob == lstrlenA(pszTemp) + 1 && !lstrcmpA((char *)olddbei.pBlob, pszTemp)) {
										MesExist = TRUE;
										break;
									}
									hDbEvent = db_event_next(hContact, hDbEvent);
									mir_free(olddbei.pBlob);
									mir_free(pszTemp);
								}

								if (!MesExist) {
									PROTORECVEVENT recv = { 0 };
									recv.flags = PREF_TCHAR;
									recv.timestamp = stamp;
									recv.tszMessage = message;
									ProtoChainRecvMsg(hContact, &recv);
								}
								mir_free(message);
							}
						}
					}
					else if (!lstrcmpi(xi.getName(node), _T("feed"))) {
						db_set_ts(hContact, MODULE, "MirVer", _T("Atom 3"));
						for (int j = 0; j < xi.getChildCount(node); j++) {
							HXML child = xi.getChild(node, j);
							if (!lstrcmpi(xi.getName(child), _T("title")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "FirstName", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("link"))) {
								for (int x = 0; x < xi.getAttrCount(child); x++) {
									if (!lstrcmpi(xi.getAttrName(child, x), _T("rel")))
										if (!lstrcmpi(xi.getAttrValue(child, xi.getAttrName(child, x)), _T("self")))
											break;

									if (!lstrcmpi(xi.getAttrName(child, x), _T("href")))
										db_set_ts(hContact, MODULE, "Homepage", xi.getAttrValue(child, xi.getAttrName(child, x)));
								}
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("subtitle")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "About", string);
								db_set_ts(hContact, "CList", "StatusMsg", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("language")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "Language1", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("author"))) {
								for (int x = 0; x < xi.getChildCount(child); x++) {
									HXML authorval = xi.getChild(child, x);
									if (!lstrcmpi(xi.getName(authorval), _T("name"))) {
										db_set_ts(hContact, MODULE, "e-mail", xi.getText(authorval));
										break;
									}
								}
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("category")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "Interest0Text", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("icon"))) {
								for (int x = 0; x < xi.getChildCount(child); x++) {
									HXML imageval = xi.getChild(child, x);
									if (!lstrcmpi(xi.getName(imageval), _T("url"))) {
										LPCTSTR url = xi.getText(imageval);
										db_set_ts(hContact, MODULE, "ImageURL", url);

										PROTO_AVATAR_INFORMATIONT pai = {NULL};
										pai.cbSize = sizeof(pai);
										pai.hContact = hContact;
										DBVARIANT dbVar = {0};

										if (!db_get_ts(hContact, MODULE, "Nick", &dbVar)) {
											TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
											pai.format = ProtoGetAvatarFormat(ext);

											TCHAR *filename = dbVar.ptszVal;
											mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
											if (DownloadFile(url, pai.filename)) {
												db_set_ts(hContact, MODULE, "ImagePath", pai.filename);
												ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE) &pai, NULL);
											}
											else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE) &pai, NULL);
											db_free(&dbVar);
											break;
										}
									}
								}
							}
							if (!lstrcmpi(xi.getName(child), _T("updated")) && xi.getText(child)) {
								TCHAR *lastupdtime = (TCHAR *)xi.getText(child);
								time_t stamp = DateToUnixTime(lastupdtime, 1);
								double deltaupd = difftime(time(NULL), stamp);
								double deltacheck = difftime(time(NULL), db_get_dw(hContact, MODULE, "LastCheck", 0));
								if (deltaupd - deltacheck >= 0) {
									db_set_dw(hContact, MODULE, "LastCheck", time(NULL));
									xi.destroyNode(hXml);
									return;
								}
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("entry"))) {
								TCHAR *title = NULL, *link = NULL, *datetime = NULL, *descr = NULL, *author = NULL, *comments = NULL, *guid = NULL, *category = NULL;
								for (int z = 0; z < xi.getChildCount(child); z++) {
									HXML itemval = xi.getChild(child, z);
									if (!lstrcmpi(xi.getName(itemval), _T("title")) && xi.getText(itemval)) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										title = string;
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("link"))) {
										for (int x = 0; x < xi.getAttrCount(itemval); x++) {
											if (!lstrcmpi(xi.getAttrName(itemval, x), _T("href"))) {
												TCHAR *string = mir_tstrdup(xi.getAttrValue(itemval, xi.getAttrName(itemval, x)));
												ClearText(string);
												link = string;
												break;
											}
										}
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("updated"))) {
										datetime = (TCHAR *)xi.getText(itemval);
										continue;
									}
									if ((!lstrcmpi(xi.getName(itemval), _T("summary")) || !lstrcmpi(xi.getName(itemval), _T("content"))) && xi.getText(itemval)) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										descr = string;
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("author"))) {
										for (int x = 0; x < xi.getChildCount(itemval); x++) {
											HXML authorval = xi.getChild(itemval, x);
											if (!lstrcmpi(xi.getName(authorval), _T("name")) && xi.getText(authorval)) {
												TCHAR *string = mir_tstrdup(xi.getText(authorval));
												ClearText(string);
												author = string;
												break;
											}
										}
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("comments")) && xi.getText(itemval)) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										comments = string;
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("id"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										guid = string;
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("category"))) {
										for (int x = 0; x < xi.getAttrCount(itemval); x++) {
											if (!lstrcmpi(xi.getAttrName(itemval, x), _T("term")) && xi.getText(itemval)) {
												TCHAR *string = mir_tstrdup(xi.getAttrValue(itemval, xi.getAttrName(itemval, x)));
												ClearText(string);
												category = string;
												break;
											}
										}
										continue;
									}
								}
								TCHAR *message;
								DBVARIANT dbMsg = {0};
								if (db_get_ts(hContact, MODULE, "MsgFormat", &dbMsg))
									message = _T(TAGSDEFAULT);
								else
									message = mir_tstrdup(dbMsg.ptszVal);
								db_free(&dbMsg);

								if (!title)
									StrReplace(_T("#<title>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<title>#"), title, message);
									mir_free(title);
								}
								if (!link)
									StrReplace(_T("#<link>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<link>#"), link, message);
									mir_free(link);
								}
								if (!descr)
									StrReplace(_T("#<description>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<description>#"), descr, message);
									mir_free(descr);
								}
								if (!author)
									StrReplace(_T("#<author>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<author>#"), author, message);
									mir_free(author);
								}
								if (!comments)
									StrReplace(_T("#<comments>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<comments>#"), comments, message);
									mir_free(comments);
								}
								if (!guid)
									StrReplace(_T("#<guid>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<guid>#"), guid, message);
									mir_free(guid);
								}
								if (!category)
									StrReplace(_T("#<category>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<category>#"), category, message);
									mir_free(category);
								}

								time_t stamp;
								if (!datetime)
									stamp = time(NULL);
								else
									stamp = DateToUnixTime(datetime, 1);

								HANDLE hDbEvent = db_event_first(hContact);
								BOOL MesExist = FALSE;
								while (hDbEvent) {
									DBEVENTINFO olddbei = { sizeof(olddbei) };
									olddbei.cbBlob = db_event_getBlobSize(hDbEvent);
									olddbei.pBlob = (PBYTE)mir_alloc(olddbei.cbBlob);
									db_event_get(hDbEvent, &olddbei);
									char *pszTemp = mir_utf8encodeT(message);
									if (olddbei.cbBlob == lstrlenA(pszTemp) + 1 && !lstrcmpA((char *)olddbei.pBlob, pszTemp))
										MesExist = TRUE;
									hDbEvent = db_event_next(hContact, hDbEvent);
									mir_free(olddbei.pBlob);
									mir_free(pszTemp);
								}

								if (!MesExist) {
									PROTORECVEVENT recv = { 0 };
									recv.flags = PREF_TCHAR;
									recv.timestamp = stamp;
									recv.tszMessage = message;
									ProtoChainRecvMsg(hContact, &recv);
								}
								mir_free(message);
							}
						}
					}
					childcount +=1;
					node = xi.getChild(hXml, childcount);
				}
				xi.destroyNode(hXml);
			}
		}
		db_set_dw(hContact, MODULE, "LastCheck", time(NULL));
	}
}

VOID CheckCurrentFeedAvatar(MCONTACT hContact)
{
	char *szData = NULL;
	DBVARIANT dbURL = {0};
	if (db_get_ts(hContact, MODULE, "URL", &dbURL))
		return;
	
	if (db_get_b(hContact, MODULE, "CheckState", 1) != 0) {
		GetNewsData(dbURL.ptszVal, &szData, hContact, NULL);
		db_free(&dbURL);
		if (szData) {
			TCHAR *tszData = mir_utf8decodeT(szData);
			if (!tszData)
				tszData = mir_a2t(szData);
			int bytesParsed = 0;
			HXML hXml = xi.parseString(tszData, &bytesParsed, NULL);
			mir_free(tszData);
			mir_free(szData);
			if(hXml != NULL) {
				int childcount = 0;
				HXML node = xi.getChild(hXml, childcount);
				while (node) {
					if (!lstrcmpi(xi.getName(node), _T("rss")) || !lstrcmpi(xi.getName(node), _T("rdf"))) {
						HXML chan = xi.getChild(node, 0);
						for (int j = 0; j < xi.getChildCount(chan); j++) {
							HXML child = xi.getChild(chan, j);
							if (!lstrcmpi(xi.getName(child), _T("image"))) {
								for (int x = 0; x < xi.getChildCount(child); x++) {
									HXML imageval = xi.getChild(child, x);
									if (!lstrcmpi(xi.getName(imageval), _T("url"))) {
										LPCTSTR url = xi.getText(imageval);
										db_set_ts(hContact, MODULE, "ImageURL", url);

										PROTO_AVATAR_INFORMATIONT pai = {NULL};
										pai.cbSize = sizeof(pai);
										pai.hContact = hContact;
										DBVARIANT dbVar = {0};

										if (!db_get_ts(hContact, MODULE, "Nick", &dbVar)) {
											TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
											pai.format = ProtoGetAvatarFormat(ext);

											TCHAR *filename = dbVar.ptszVal;
											mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
											if (DownloadFile(url, pai.filename)) {
												db_set_ts(hContact, MODULE, "ImagePath", pai.filename);
												ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE) &pai, NULL);
											}
											else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE) &pai, NULL);
											db_free(&dbVar);
											break;
										}
									}
								}
							}
						}
					}
					else if (!lstrcmpi(xi.getName(node), _T("feed"))) {
						for (int j = 0; j < xi.getChildCount(node); j++) {
							HXML child = xi.getChild(node, j);
							if (!lstrcmpi(xi.getName(child), _T("icon"))) {
								for (int x = 0; x < xi.getChildCount(child); x++) {
									HXML imageval = xi.getChild(child, x);
									if (!lstrcmpi(xi.getName(imageval), _T("url"))) {
										LPCTSTR url = xi.getText(imageval);
										db_set_ts(hContact, MODULE, "ImageURL", url);

										PROTO_AVATAR_INFORMATIONT pai = {NULL};
										pai.cbSize = sizeof(pai);
										pai.hContact = hContact;
										DBVARIANT dbVar = {0};

										if (!db_get_ts(hContact, MODULE, "Nick", &dbVar)) {
											TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
											pai.format = ProtoGetAvatarFormat(ext);

											TCHAR *filename = dbVar.ptszVal;
											mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
											if (DownloadFile(url, pai.filename)) {
												db_set_ts(hContact, MODULE, "ImagePath", pai.filename);
												ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE) &pai, NULL);
											}
											else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE) &pai, NULL);
											db_free(&dbVar);
											break;
										}
									}
								}
							}
						}
					}
					childcount +=1;
					node = xi.getChild(hXml, childcount);
				}
				xi.destroyNode(hXml);
			}
		}
	}
}