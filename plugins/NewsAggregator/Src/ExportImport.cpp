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

INT_PTR CALLBACK DlgProcImportOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND FeedsList = GetDlgItem(hwndDlg, IDC_FEEDSLIST);
	HWND FeedsImportList = GetDlgItem(hwndDlg, IDC_FEEDSIMPORTLIST);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG)lParam);
		Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULE, "ImportDlg");
		EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), FALSE);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				wchar_t FileName[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_IMPORTFILEPATH, FileName, _countof(FileName));
				int bytesParsed = 0;
				HXML hXml = xmlParseFile(FileName, &bytesParsed, NULL);
				if(hXml != NULL) {
					HWND hwndList = (HWND)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					bool isTextUTF = false, isURLUTF = false, isSiteURLUTF = false, isGroupUTF = false;
					HXML node = xmlGetChildByPath(hXml, L"opml/body/outline", 0);
					if ( !node)
						node = xmlGetChildByPath(hXml, L"body/outline", 0);
					int count = (int)SendMessage(FeedsImportList, LB_GETCOUNT, 0, 0);
					int DUPES = 0;
					if (node) {
						while (node) {
							int outlineAttr = xmlGetAttrCount(node);
							int outlineChildsCount = xmlGetChildCount(node);
							wchar_t *xmlUrl = (wchar_t *)xmlGetAttrValue(node, L"xmlUrl");
							if (!xmlUrl && !outlineChildsCount) {
								HXML tmpnode = node;
								node = xmlGetNextNode(node);
								if ( !node) {
									do {
										node = tmpnode;
										node = xmlGetParent(node);
										tmpnode = node;
										node = xmlGetNextNode(node);
										if (node)
											break;
									} while (mir_wstrcmpi(xmlGetName(node), L"body"));
								}
							}
							else if (!xmlUrl && outlineChildsCount)
								node = xmlGetFirstChild(node);
							else if (xmlUrl) {
								wchar_t *text = NULL, *url = NULL, *siteurl = NULL, *group = NULL;
								BYTE NeedToImport = FALSE;
								for (int i = 0; i < outlineAttr; i++) {
									if (!mir_wstrcmpi(xmlGetAttrName(node, i), L"text")) {
										text = mir_utf8decodeW(_T2A(xmlGetAttrValue(node, xmlGetAttrName(node, i))));
										if (!text) {
											isTextUTF = 0;
											text = (wchar_t *)xmlGetAttrValue(node, xmlGetAttrName(node, i));
										} else
											isTextUTF = 1;

										for (int j = 0; j < count; j++) {
											wchar_t item[MAX_PATH];
											SendMessage(FeedsImportList, LB_GETTEXT, (WPARAM)j, (LPARAM)item);
											if (!mir_wstrcmpi(item, text)) {
												NeedToImport = TRUE;
												break;
											}
										}
										continue;
									}
									if (!mir_wstrcmpi(xmlGetAttrName(node, i), L"xmlUrl")) {
										url = mir_utf8decodeW(_T2A(xmlGetAttrValue(node, xmlGetAttrName(node, i))));
										if ( !url) {
											isURLUTF = false;
											url = (wchar_t *)xmlGetAttrValue(node, xmlGetAttrName(node, i));
										} else
											isURLUTF = true;
										if (GetContactByURL(url) && NeedToImport) {
											NeedToImport = FALSE;
											DUPES++;
										}
										continue;
									}
									if (!mir_wstrcmpi(xmlGetAttrName(node, i), L"htmlUrl")) {
										siteurl = mir_utf8decodeW(_T2A(xmlGetAttrValue(node, xmlGetAttrName(node, i))));
										if ( !siteurl) {
											isSiteURLUTF = false;
											siteurl = (wchar_t *)xmlGetAttrValue(node, xmlGetAttrName(node, i));
										} else
											isSiteURLUTF = true;
										continue;
									}
									if (text && url && siteurl)
										break;
								}

								if (NeedToImport) {
									HXML parent = xmlGetParent(node);
									wchar_t tmpgroup[1024];
									while (mir_wstrcmpi(xmlGetName(parent), L"body")) {
										for (int i = 0; i < xmlGetAttrCount(parent); i++) {
											if (!mir_wstrcmpi(xmlGetAttrName(parent, i), L"text")) {
												if ( !group)
													group = (wchar_t *)xmlGetAttrValue(parent, xmlGetAttrName(parent, i));
												else {
													mir_snwprintf(tmpgroup, L"%s\\%s", xmlGetAttrValue(parent, xmlGetAttrName(parent, i)), group);
													group = tmpgroup;
												}
												break;
											}
										}
										parent = xmlGetParent(parent);
									}

									wchar_t *ptszGroup = NULL;
									if (group) {
										ptszGroup = mir_utf8decodeW(_T2A(group));
										if ( !ptszGroup) {
											isGroupUTF = false;
											ptszGroup = group;
										} else
											isGroupUTF = 1;
									}

									MCONTACT hContact = db_add_contact();
									Proto_AddToContact(hContact, MODULE);
									db_set_ws(hContact, MODULE, "Nick", text);
									db_set_ws(hContact, MODULE, "URL", url);
									db_set_ws(hContact, MODULE, "Homepage", siteurl);
									db_set_b(hContact, MODULE, "CheckState", 1);
									db_set_dw(hContact, MODULE, "UpdateTime", DEFAULT_UPDATE_TIME);
									db_set_ws(hContact, MODULE, "MsgFormat", TAGSDEFAULT);
									db_set_w(hContact, MODULE, "Status", CallProtoService(MODULE, PS_GETSTATUS, 0, 0));
									if (ptszGroup) {
										db_set_ws(hContact, "CList", "Group", ptszGroup);
										Clist_GroupCreate(0, ptszGroup);
									}
									if (isGroupUTF)
										mir_free(ptszGroup);
								}
								if (isTextUTF)
									mir_free(text);
								if (isURLUTF)
									mir_free(url);
								if (isSiteURLUTF)
									mir_free(siteurl);

								HXML tmpnode = node;
								node = xmlGetNextNode(node);
								if ( !node) {
									do {
										node = tmpnode;
										node = xmlGetParent(node);
										tmpnode = node;
										node = xmlGetNextNode(node);
										if (node)
											break;
									}
										while (mir_wstrcmpi(xmlGetName(tmpnode), L"body"));
								}
							}
						}
					}
					xmlDestroyNode(hXml);
					if (hwndList) {
						DeleteAllItems(hwndList);
						UpdateList(hwndList);
					}
					wchar_t mes[MAX_PATH];
					if (DUPES)
						mir_snwprintf(mes, TranslateT("Imported %d feed(s)\r\nNot imported %d duplicate(s)."), count - DUPES, DUPES);
					else
						mir_snwprintf(mes, TranslateT("Imported %d feed(s)."), count);
					MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONINFORMATION);
				}
			}
			// fall through
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case IDC_BROWSEIMPORTFILE:
			{
				wchar_t FileName[MAX_PATH];
				VARSW tszMirDir(L"%miranda_path%");

				OPENFILENAME ofn = {0};
				ofn.lStructSize = sizeof(ofn);
				wchar_t tmp[MAX_PATH];
				mir_snwprintf(tmp, L"%s (*.opml, *.xml)%c*.opml;*.xml%c%c", TranslateT("OPML files"), 0, 0, 0);
				ofn.lpstrFilter = tmp;
				ofn.hwndOwner = 0;
				ofn.lpstrFile = FileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.Flags = OFN_HIDEREADONLY;
				ofn.lpstrInitialDir = tszMirDir;
				*FileName = '\0';
				ofn.lpstrDefExt = L"";

				if (GetOpenFileName(&ofn)) {
					int bytesParsed = 0;
					HXML hXml = xmlParseFile(FileName, &bytesParsed, NULL);
					if(hXml != NULL) {
						HXML node = xmlGetChildByPath(hXml, L"opml/body/outline", 0);
						if ( !node)
							node = xmlGetChildByPath(hXml, L"body/outline", 0);
						if (node) {
							while (node) {
								int outlineAttr = xmlGetAttrCount(node);
								int outlineChildsCount = xmlGetChildCount(node);
								wchar_t *xmlUrl = (wchar_t *)xmlGetAttrValue(node, L"xmlUrl");
								if (!xmlUrl && !outlineChildsCount) {
									HXML tmpnode = node;
									node = xmlGetNextNode(node);
									if ( !node) {
										do {
											node = tmpnode;
											node = xmlGetParent(node);
											tmpnode = node;
											node = xmlGetNextNode(node);
											if (node)
												break;
										} while (mir_wstrcmpi(xmlGetName(node), L"body"));
									}
								}
								else if (!xmlUrl && outlineChildsCount)
									node = xmlGetFirstChild(node);
								else if (xmlUrl) {
									for (int i = 0; i < outlineAttr; i++) {
										if (!mir_wstrcmpi(xmlGetAttrName(node, i), L"text")) {
											wchar_t *text = mir_utf8decodeW(_T2A(xmlGetAttrValue(node, xmlGetAttrName(node, i))));
											bool isTextUTF;
											if (!text) {
												isTextUTF = false;
												text = (wchar_t *)xmlGetAttrValue(node, xmlGetAttrName(node, i));
											} else
												isTextUTF = true;
											SendMessage(FeedsList, LB_ADDSTRING, 0, (LPARAM)text);
											EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), TRUE);
											EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), TRUE);
											if (isTextUTF)
												mir_free(text);
										}
									}


									HXML tmpnode = node;
									node = xmlGetNextNode(node);
									if ( !node) {
										do {
											node = tmpnode;
											node = xmlGetParent(node);
											tmpnode = node;
											node = xmlGetNextNode(node);
											if (node)
												break;
										} while (mir_wstrcmpi(xmlGetName(tmpnode), L"body"));
									}
								}
							}
						}
						else
							MessageBox(hwndDlg, TranslateT("Not valid import file."), TranslateT("Error"), MB_OK | MB_ICONERROR);
						xmlDestroyNode(hXml);
						SetDlgItemText(hwndDlg, IDC_IMPORTFILEPATH, FileName);
					}
					else
						MessageBox(hwndDlg, TranslateT("Not valid import file."), TranslateT("Error"), MB_OK | MB_ICONERROR);

					break;
				}
				break;
			}
			break;

		case IDC_ADDFEED:
			{
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_REMOVEFEED))) 
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), TRUE);
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS))) 
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), TRUE);
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDOK)))
					EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
				int cursel = (int)SendMessage(FeedsList, LB_GETCURSEL, 0, 0);
				wchar_t item[MAX_PATH];
				SendMessage(FeedsList, LB_GETTEXT, (WPARAM)cursel, (LPARAM)item);
				SendMessage(FeedsImportList, LB_ADDSTRING, 0, (LPARAM)item);
				SendMessage(FeedsList, LB_DELETESTRING, (WPARAM)cursel, 0);
				if (!SendMessage(FeedsList, LB_GETCOUNT, 0, 0)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), FALSE);
				}
			}
			break;

		case IDC_REMOVEFEED:
			{
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADDFEED)))
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), TRUE);
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS)))
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), TRUE);
				int cursel = (int)SendMessage(FeedsImportList, LB_GETCURSEL, 0, 0);
				wchar_t item[MAX_PATH];
				SendMessage(FeedsImportList, LB_GETTEXT, (WPARAM)cursel, (LPARAM)item);
				SendMessage(FeedsList, LB_ADDSTRING, 0, (LPARAM)item);
				SendMessage(FeedsImportList, LB_DELETESTRING, (WPARAM)cursel, 0);
				if (!SendMessage(FeedsImportList, LB_GETCOUNT, 0, 0))
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
				}
			}
			break;

		case IDC_ADDALLFEEDS:
			{
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_REMOVEFEED))) 
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), TRUE);
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS))) 
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), TRUE);
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDOK)))
					EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
				int count = (int)SendMessage(FeedsList, LB_GETCOUNT, 0, 0);
				for (int i = 0; i < count; i++) {
					wchar_t item[MAX_PATH];
					SendMessage(FeedsList, LB_GETTEXT, (WPARAM)i, (LPARAM)item);
					SendMessage(FeedsImportList, LB_ADDSTRING, 0, (LPARAM)item);
				}
				for (int i = count - 1; i > -1; i--)
					SendMessage(FeedsList, LB_DELETESTRING, (WPARAM)i, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), FALSE);
			}
			break;

		case IDC_REMOVEALLFEEDS:
			{
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADDFEED)))
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), TRUE);
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS)))
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), TRUE);
				int count = (int)SendMessage(FeedsImportList, LB_GETCOUNT, 0, 0);
				for (int i = 0; i < count; i++) {
					wchar_t item[MAX_PATH];
					SendMessage(FeedsImportList, LB_GETTEXT, (WPARAM)i, (LPARAM)item);
					SendMessage(FeedsList, LB_ADDSTRING, 0, (LPARAM)item);
				}
				for (int i = count - 1; i > -1; i--)
					SendMessage(FeedsImportList, LB_DELETESTRING, (WPARAM)i, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
			}
			break;

		case IDC_FEEDSLIST:
			{
				if (HIWORD(wParam) == LBN_DBLCLK)
				{
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_REMOVEFEED))) 
						EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), TRUE);
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS))) 
						EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), TRUE);
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDOK)))
						EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
					int cursel = (int)SendMessage(FeedsList, LB_GETCURSEL, 0, 0);
					wchar_t item[MAX_PATH];
					SendMessage(FeedsList, LB_GETTEXT, (WPARAM)cursel, (LPARAM)item);
					SendMessage(FeedsImportList, LB_ADDSTRING, 0, (LPARAM)item);
					SendMessage(FeedsList, LB_DELETESTRING, (WPARAM)cursel, 0);
					if (!SendMessage(FeedsList, LB_GETCOUNT, 0, 0)) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), FALSE);
					}
				}
			}
			break;

		case IDC_FEEDSIMPORTLIST:
			{
				if (HIWORD(wParam) == LBN_DBLCLK)
				{
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADDFEED)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), TRUE);
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), TRUE);
					int cursel = (int)SendMessage(FeedsImportList, LB_GETCURSEL, 0, 0);
					wchar_t item[MAX_PATH];
					SendMessage(FeedsImportList, LB_GETTEXT, (WPARAM)cursel, (LPARAM)item);
					SendMessage(FeedsList, LB_ADDSTRING, 0, (LPARAM)item);
					SendMessage(FeedsImportList, LB_DELETESTRING, (WPARAM)cursel, 0);
					if (!SendMessage(FeedsImportList, LB_GETCOUNT, 0, 0))
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
					}
				}
			}
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "ImportDlg");
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProcExportOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	HWND FeedsList = GetDlgItem(hwndDlg, IDC_FEEDSLIST);
	HWND FeedsExportList = GetDlgItem(hwndDlg, IDC_FEEDSEXPORTLIST);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULE, "ExportDlg");
		for (MCONTACT hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
			wchar_t *message = db_get_wsa(hContact, MODULE, "Nick");
			if (message != NULL) {
				SendMessage(FeedsList, LB_ADDSTRING, 0, (LPARAM)message);
				mir_free(message);
			}
		}
		EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
		if (!SendMessage(FeedsList, LB_GETCOUNT, 0, 0)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), FALSE);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				wchar_t FileName[MAX_PATH];
				VARSW tszMirDir(L"%miranda_path%");

				OPENFILENAME ofn = {0};
				ofn.lStructSize = sizeof(ofn);
				wchar_t tmp[MAX_PATH];
				mir_snwprintf(tmp, L"%s (*.opml)%c*.opml%c%c", TranslateT("OPML files"), 0, 0, 0);
				ofn.lpstrFilter = tmp;
				ofn.hwndOwner = 0;
				ofn.lpstrFile = FileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.Flags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
				ofn.lpstrInitialDir = tszMirDir;
				*FileName = '\0';
				ofn.lpstrDefExt = L"";

				if (GetSaveFileName(&ofn)) {
					HXML hXml = xmlCreateNode(L"opml", NULL, FALSE);
					xmlAddAttr(hXml, L"version", L"1.0");
					HXML header = xmlAddChild(hXml, L"head", NULL);
					xmlAddChild(header, L"title", L"Miranda NG NewsAggregator plugin export");
					header = xmlAddChild(hXml, L"body", NULL);

					int count = (int)SendMessage(FeedsExportList, LB_GETCOUNT, 0, 0);
					for (int i = 0; i < count; i++) {
						wchar_t item[MAX_PATH];
						SendMessage(FeedsExportList, LB_GETTEXT, (WPARAM)i, (LPARAM)item);
						MCONTACT hContact = GetContactByNick(item);
						wchar_t
							*title = db_get_wsa(hContact, MODULE, "Nick"),
							*url = db_get_wsa(hContact, MODULE, "URL"),
							*siteurl = db_get_wsa(hContact, MODULE, "Homepage"),
							*group = db_get_wsa(hContact, "CList", "Group");

						HXML elem = header;
						if (group)
						{
							wchar_t *section = wcstok(group, L"\\");
							while (section != NULL)
							{
								HXML existgroup = xmlGetChildByAttrValue(header, L"outline", L"title", section);
								if ( !existgroup)
								{
									elem = xmlAddChild(elem, L"outline", NULL);
									xmlAddAttr(elem, L"title", section);
									xmlAddAttr(elem, L"text", section);
								} else {
									elem = existgroup;
								}
								section = wcstok(NULL, L"\\");
							}
							elem = xmlAddChild(elem, L"outline", NULL);
						}
						else
							elem = xmlAddChild(elem, L"outline", NULL);
						xmlAddAttr(elem, L"text", title);
						xmlAddAttr(elem, L"title", title);
						xmlAddAttr(elem, L"type", L"rss");
						xmlAddAttr(elem, L"xmlUrl", url);
						xmlAddAttr(elem, L"htmlUrl", siteurl);

						mir_free(title);
						mir_free(url);
						mir_free(siteurl);
						mir_free(group);
					}
					xmlToFile(hXml, FileName, 1);
					xmlDestroyNode(hXml);
				}
			}
			// fall through
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case IDC_ADDFEED:
			{
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_REMOVEFEED))) 
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), TRUE);
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS))) 
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), TRUE);
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDOK)))
					EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
				int cursel = (int)SendMessage(FeedsList, LB_GETCURSEL, 0, 0);
				wchar_t item[MAX_PATH];
				SendMessage(FeedsList, LB_GETTEXT, (WPARAM)cursel, (LPARAM)item);
				SendMessage(FeedsExportList, LB_ADDSTRING, 0, (LPARAM)item);
				SendMessage(FeedsList, LB_DELETESTRING, (WPARAM)cursel, 0);
				if (!SendMessage(FeedsList, LB_GETCOUNT, 0, 0)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), FALSE);
				}
			}
			break;
	
		case IDC_REMOVEFEED:
			{
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADDFEED)))
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), TRUE);
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS)))
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), TRUE);
				int cursel = (int)SendMessage(FeedsExportList, LB_GETCURSEL, 0, 0);
				wchar_t item[MAX_PATH];
				SendMessage(FeedsExportList, LB_GETTEXT, (WPARAM)cursel, (LPARAM)item);
				SendMessage(FeedsList, LB_ADDSTRING, 0, (LPARAM)item);
				SendMessage(FeedsExportList, LB_DELETESTRING, (WPARAM)cursel, 0);
				if (!SendMessage(FeedsExportList, LB_GETCOUNT, 0, 0))
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
				}
			}
			break;

		case IDC_ADDALLFEEDS:
			{
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_REMOVEFEED))) 
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), TRUE);
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS))) 
					EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), TRUE);
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDOK)))
					EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
				int count = (int)SendMessage(FeedsList, LB_GETCOUNT, 0, 0);
				for (int i = 0; i < count; i++) {
					wchar_t item[MAX_PATH];
					SendMessage(FeedsList, LB_GETTEXT, (WPARAM)i, (LPARAM)item);
					SendMessage(FeedsExportList, LB_ADDSTRING, 0, (LPARAM)item);
				}
				for (int i = count - 1; i > -1; i--)
					SendMessage(FeedsList, LB_DELETESTRING, (WPARAM)i, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), FALSE);
			}
			break;

		case IDC_REMOVEALLFEEDS:
			{
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADDFEED)))
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), TRUE);
				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS)))
					EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), TRUE);
				int count = (int)SendMessage(FeedsExportList, LB_GETCOUNT, 0, 0);
				for (int i = 0; i < count; i++) {
					wchar_t item[MAX_PATH];
					SendMessage(FeedsExportList, LB_GETTEXT, (WPARAM)i, (LPARAM)item);
					SendMessage(FeedsList, LB_ADDSTRING, 0, (LPARAM)item);
				}
				for (int i = count - 1; i > -1; i--)
					SendMessage(FeedsExportList, LB_DELETESTRING, (WPARAM)i, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
			}
			break;

		case IDC_FEEDSLIST:
			{
				if (HIWORD(wParam) == LBN_DBLCLK)
				{
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_REMOVEFEED))) 
						EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), TRUE);
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS))) 
						EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), TRUE);
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDOK)))
						EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
					int cursel = (int)SendMessage(FeedsList, LB_GETCURSEL, 0, 0);
					wchar_t item[MAX_PATH];
					SendMessage(FeedsList, LB_GETTEXT, (WPARAM)cursel, (LPARAM)item);
					SendMessage(FeedsExportList, LB_ADDSTRING, 0, (LPARAM)item);
					SendMessage(FeedsList, LB_DELETESTRING, (WPARAM)cursel, 0);
					if (!SendMessage(FeedsList, LB_GETCOUNT, 0, 0)) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), FALSE);
					}
				}
			}
			break;

		case IDC_FEEDSEXPORTLIST:
			{
				if (HIWORD(wParam) == LBN_DBLCLK)
				{
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADDFEED)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADDFEED), TRUE);
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_ADDALLFEEDS), TRUE);
					int cursel = (int)SendMessage(FeedsExportList, LB_GETCURSEL, 0, 0);
					wchar_t item[MAX_PATH];
					SendMessage(FeedsExportList, LB_GETTEXT, (WPARAM)cursel, (LPARAM)item);
					SendMessage(FeedsList, LB_ADDSTRING, 0, (LPARAM)item);
					SendMessage(FeedsExportList, LB_DELETESTRING, (WPARAM)cursel, 0);
					if (!SendMessage(FeedsExportList, LB_GETCOUNT, 0, 0))
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEFEED), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEALLFEEDS), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
					}
				}
			}
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "ExportDlg");
		break;
	}

	return FALSE;
}