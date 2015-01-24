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
				TCHAR FileName[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_IMPORTFILEPATH, FileName, SIZEOF(FileName));
				int bytesParsed = 0;
				HXML hXml = xi.parseFile(FileName, &bytesParsed, NULL);
				if(hXml != NULL) {
					HWND hwndList = (HWND)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					bool isTextUTF = false, isURLUTF = false, isSiteURLUTF = false, isGroupUTF = false;
					HXML node = xi.getChildByPath(hXml, _T("opml/body/outline"), 0);
					if ( !node)
						node = xi.getChildByPath(hXml, _T("body/outline"), 0);
					int count = (int)SendMessage(FeedsImportList, LB_GETCOUNT, 0, 0);
					int DUPES = 0;
					if (node) {
						while (node) {
							int outlineAttr = xi.getAttrCount(node);
							int outlineChildsCount = xi.getChildCount(node);
							TCHAR *xmlUrl = (TCHAR *)xi.getAttrValue(node, _T("xmlUrl"));
							if (!xmlUrl && !outlineChildsCount) {
								HXML tmpnode = node;
								node = xi.getNextNode(node);
								if ( !node) {
									do {
										node = tmpnode;
										node = xi.getParent(node);
										tmpnode = node;
										node = xi.getNextNode(node);
										if (node)
											break;
									} while (mir_tstrcmpi(xi.getName(node), _T("body")));
								}
							}
							else if (!xmlUrl && outlineChildsCount)
								node = xi.getFirstChild(node);
							else if (xmlUrl) {
								TCHAR *text = NULL, *url = NULL, *siteurl = NULL, *group = NULL;
								BYTE NeedToImport = FALSE;
								for (int i = 0; i < outlineAttr; i++) {
									if (!mir_tstrcmpi(xi.getAttrName(node, i), _T("text"))) {
										text = mir_utf8decodeT(_T2A(xi.getAttrValue(node, xi.getAttrName(node, i))));
										if (!text) {
											isTextUTF = 0;
											text = (TCHAR *)xi.getAttrValue(node, xi.getAttrName(node, i));
										} else
											isTextUTF = 1;

										for (int i = 0; i < count; i++) {
											TCHAR item[MAX_PATH];
											SendMessage(FeedsImportList, LB_GETTEXT, (WPARAM)i, (LPARAM)item);
											if (!mir_tstrcmpi(item, text)) {
												NeedToImport = TRUE;
												break;
											}
										}
										continue;
									}
									if (!mir_tstrcmpi(xi.getAttrName(node, i), _T("xmlUrl"))) {
										url = mir_utf8decodeT(_T2A(xi.getAttrValue(node, xi.getAttrName(node, i))));
										if ( !url) {
											isURLUTF = false;
											url = (TCHAR *)xi.getAttrValue(node, xi.getAttrName(node, i));
										} else
											isURLUTF = true;
										if (GetContactByURL(url) && NeedToImport) {
											NeedToImport = FALSE;
											DUPES++;
										}
										continue;
									}
									if (!mir_tstrcmpi(xi.getAttrName(node, i), _T("htmlUrl"))) {
										siteurl = mir_utf8decodeT(_T2A(xi.getAttrValue(node, xi.getAttrName(node, i))));
										if ( !siteurl) {
											isSiteURLUTF = false;
											siteurl = (TCHAR *)xi.getAttrValue(node, xi.getAttrName(node, i));
										} else
											isSiteURLUTF = true;
										continue;
									}
									if (text && url && siteurl)
										break;
								}

								if (NeedToImport) {
									HXML parent = xi.getParent(node);
									TCHAR tmpgroup[1024];
									while (mir_tstrcmpi(xi.getName(parent), _T("body"))) {
										for (int i = 0; i < xi.getAttrCount(parent); i++) {
											if (!mir_tstrcmpi(xi.getAttrName(parent, i), _T("text"))) {
												if ( !group)
													group = (TCHAR *)xi.getAttrValue(parent, xi.getAttrName(parent, i));
												else {
													mir_sntprintf(tmpgroup, SIZEOF(tmpgroup), _T("%s\\%s"), xi.getAttrValue(parent, xi.getAttrName(parent, i)), group);
													group = tmpgroup;
												}
												break;
											}
										}
										parent = xi.getParent(parent);
									}

									TCHAR *utfgroup = NULL;
									if (group) {
										utfgroup = mir_utf8decodeT(_T2A(group));
										if ( !utfgroup) {
											isGroupUTF = false;
											utfgroup = group;
										} else
											isGroupUTF = 1;
									}

									MCONTACT hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
									CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)MODULE);
									db_set_ts(hContact, MODULE, "Nick", text);
									db_set_ts(hContact, MODULE, "URL", url);
									db_set_ts(hContact, MODULE, "Homepage", siteurl);
									db_set_b(hContact, MODULE, "CheckState", 1);
									db_set_dw(hContact, MODULE, "UpdateTime", DEFAULT_UPDATE_TIME);
									db_set_ts(hContact, MODULE, "MsgFormat", TAGSDEFAULT);
									db_set_w(hContact, MODULE, "Status", CallProtoService(MODULE, PS_GETSTATUS, 0, 0));
									if (utfgroup) {
										db_set_ts(hContact, "CList", "Group", utfgroup);
										int hGroup = 1;
										char *group_name;
										BYTE GroupExist = 0;
										do {
											group_name = (char *)CallService(MS_CLIST_GROUPGETNAME, (WPARAM)hGroup, 0);
											if (group_name != NULL && !strcmp(group_name, _T2A(utfgroup))) {
												GroupExist = 1;
												break;
											}
											hGroup++;
										}
											while (group_name);

										if(!GroupExist)
											CallService(MS_CLIST_GROUPCREATE, 0, (LPARAM)utfgroup);
									}
									if (isGroupUTF)
										mir_free(utfgroup);
								}
								if (isTextUTF)
									mir_free(text);
								if (isURLUTF)
									mir_free(url);
								if (isSiteURLUTF)
									mir_free(siteurl);

								HXML tmpnode = node;
								node = xi.getNextNode(node);
								if ( !node) {
									do {
										node = tmpnode;
										node = xi.getParent(node);
										tmpnode = node;
										node = xi.getNextNode(node);
										if (node)
											break;
									}
										while (mir_tstrcmpi(xi.getName(tmpnode), _T("body")));
								}
							}
						}
					}
					xi.destroyNode(hXml);
					if (hwndList) {
						DeleteAllItems(hwndList);
						UpdateList(hwndList);
					}
					TCHAR mes[MAX_PATH];
					if (DUPES)
						mir_sntprintf(mes, SIZEOF(mes), TranslateT("Imported %d feed(s)\r\nNot imported %d duplicate(s)."), count - DUPES, DUPES);
					else
						mir_sntprintf(mes, SIZEOF(mes), TranslateT("Imported %d feed(s)."), count);
					MessageBox(hwndDlg, mes, TranslateT("News Aggregator"), MB_OK | MB_ICONINFORMATION);
				}
			}

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case IDC_BROWSEIMPORTFILE:
			{
				TCHAR FileName[MAX_PATH];
				VARST tszMirDir(_T("%miranda_path%"));

				OPENFILENAME ofn = {0};
				ofn.lStructSize = sizeof(ofn);
				TCHAR tmp[MAX_PATH];
				mir_sntprintf(tmp, SIZEOF(tmp), _T("%s (*.opml, *.xml)%c*.opml;*.xml%c%c"), TranslateT("OPML files"), 0, 0, 0);
				ofn.lpstrFilter = tmp;
				ofn.hwndOwner = 0;
				ofn.lpstrFile = FileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.Flags = OFN_HIDEREADONLY;
				ofn.lpstrInitialDir = tszMirDir;
				*FileName = '\0';
				ofn.lpstrDefExt = _T("");

				if (GetOpenFileName(&ofn)) {
					int bytesParsed = 0;
					HXML hXml = xi.parseFile(FileName, &bytesParsed, NULL);
					if(hXml != NULL) {
						HXML node = xi.getChildByPath(hXml, _T("opml/body/outline"), 0);
						if ( !node)
							node = xi.getChildByPath(hXml, _T("body/outline"), 0);
						if (node) {
							while (node) {
								int outlineAttr = xi.getAttrCount(node);
								int outlineChildsCount = xi.getChildCount(node);
								TCHAR *xmlUrl = (TCHAR *)xi.getAttrValue(node, _T("xmlUrl"));
								if (!xmlUrl && !outlineChildsCount) {
									HXML tmpnode = node;
									node = xi.getNextNode(node);
									if ( !node) {
										do {
											node = tmpnode;
											node = xi.getParent(node);
											tmpnode = node;
											node = xi.getNextNode(node);
											if (node)
												break;
										} while (mir_tstrcmpi(xi.getName(node), _T("body")));
									}
								}
								else if (!xmlUrl && outlineChildsCount)
									node = xi.getFirstChild(node);
								else if (xmlUrl) {
									for (int i = 0; i < outlineAttr; i++) {
										if (!mir_tstrcmpi(xi.getAttrName(node, i), _T("text"))) {
											TCHAR *text = mir_utf8decodeT(_T2A(xi.getAttrValue(node, xi.getAttrName(node, i))));
											bool isTextUTF;
											if (!text) {
												isTextUTF = false;
												text = (TCHAR *)xi.getAttrValue(node, xi.getAttrName(node, i));
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
									node = xi.getNextNode(node);
									if ( !node) {
										do {
											node = tmpnode;
											node = xi.getParent(node);
											tmpnode = node;
											node = xi.getNextNode(node);
											if (node)
												break;
										} while (mir_tstrcmpi(xi.getName(tmpnode), _T("body")));
									}
								}
							}
						}
						else
							MessageBox(hwndDlg, TranslateT("Not valid import file."), TranslateT("Error"), MB_OK | MB_ICONERROR);
						xi.destroyNode(hXml);
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
				TCHAR item[MAX_PATH];
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
				TCHAR item[MAX_PATH];
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
					TCHAR item[MAX_PATH];
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
					TCHAR item[MAX_PATH];
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
					TCHAR item[MAX_PATH];
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
					TCHAR item[MAX_PATH];
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
			TCHAR *message = db_get_tsa(hContact, MODULE, "Nick");
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
				TCHAR FileName[MAX_PATH];
				VARST tszMirDir(_T("%miranda_path%"));

				OPENFILENAME ofn = {0};
				ofn.lStructSize = sizeof(ofn);
				TCHAR tmp[MAX_PATH];
				mir_sntprintf(tmp, SIZEOF(tmp), _T("%s (*.opml)%c*.opml%c%c"), TranslateT("OPML files"), 0, 0, 0);
				ofn.lpstrFilter = tmp;
				ofn.hwndOwner = 0;
				ofn.lpstrFile = FileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.Flags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
				ofn.lpstrInitialDir = tszMirDir;
				*FileName = '\0';
				ofn.lpstrDefExt = _T("");

				if (GetSaveFileName(&ofn)) {
					HXML hXml = xi.createNode(_T("opml"), NULL, FALSE);
					xi.addAttr(hXml, _T("version"), _T("1.0"));
					HXML header = xi.addChild(hXml, _T("head"), NULL);
					xi.addChild(header, _T("title"), _T("Miranda NG NewsAggregator plugin export"));
					header = xi.addChild(hXml, _T("body"), NULL);

					int count = (int)SendMessage(FeedsExportList, LB_GETCOUNT, 0, 0);
					for (int i = 0; i < count; i++) {
						TCHAR item[MAX_PATH];
						SendMessage(FeedsExportList, LB_GETTEXT, (WPARAM)i, (LPARAM)item);
						MCONTACT hContact = GetContactByNick(item);
						TCHAR
							*title = db_get_tsa(hContact, MODULE, "Nick"),
							*url = db_get_tsa(hContact, MODULE, "URL"),
							*siteurl = db_get_tsa(hContact, MODULE, "Homepage"),
							*group = db_get_tsa(hContact, "CList", "Group");

						HXML elem = header;
						if (group)
						{
							TCHAR *section = _tcstok(group, _T("\\"));
							while (section != NULL)
							{
								HXML existgroup = xi.getChildByAttrValue(header, _T("outline"), _T("title"), section);
								if ( !existgroup)
								{
									elem = xi.addChild(elem, _T("outline"), NULL);
									xi.addAttr(elem, _T("title"), section);
									xi.addAttr(elem, _T("text"), section);
								} else {
									elem = existgroup;
								}
								section = _tcstok(NULL, _T("\\"));
							}
							elem = xi.addChild(elem, _T("outline"), NULL);
						}
						else
							elem = xi.addChild(elem, _T("outline"), NULL);
						xi.addAttr(elem, _T("text"), title);
						xi.addAttr(elem, _T("title"), title);
						xi.addAttr(elem, _T("type"), _T("rss"));
						xi.addAttr(elem, _T("xmlUrl"), url);
						xi.addAttr(elem, _T("htmlUrl"), siteurl);

						mir_free(title);
						mir_free(url);
						mir_free(siteurl);
						mir_free(group);
					}
					xi.toFile(hXml, FileName, 1);
					xi.destroyNode(hXml);
				}
			}

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
				TCHAR item[MAX_PATH];
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
				TCHAR item[MAX_PATH];
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
					TCHAR item[MAX_PATH];
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
					TCHAR item[MAX_PATH];
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
					TCHAR item[MAX_PATH];
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
					TCHAR item[MAX_PATH];
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