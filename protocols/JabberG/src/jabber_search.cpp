/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Artem Shpynov
Copyright (C) 2012-22 Miranda NG team

Module implements a search according to XEP-0055: Jabber Search
http://www.xmpp.org/extensions/xep-0055.html

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"
#include <CommCtrl.h>
#include "jabber_iq.h"
#include "jabber_caps.h"

///////////////////////////////////////////////////////////////////////////////
// Subclassing of IDC_FRAME to implement more user-friendly fields scrolling

static int JabberSearchFrameProc(HWND hwnd, int msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_COMMAND && lParam != 0) {
		HWND hwndDlg = GetParent(hwnd);
		JabberSearchData *dat = (JabberSearchData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (dat && lParam) {
			int pos = dat->curPos;
			RECT MineRect;
			RECT FrameRect;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_FRAME), &FrameRect);
			GetWindowRect((HWND)lParam, &MineRect);
			if (MineRect.top - 10 < FrameRect.top) {
				pos = dat->curPos + (MineRect.top - 14 - FrameRect.top);
				if (pos < 0) pos = 0;
			}
			else if (MineRect.bottom > FrameRect.bottom) {
				pos = dat->curPos + (MineRect.bottom - FrameRect.bottom);
				if (dat->frameHeight + pos > dat->CurrentHeight)
					pos = dat->CurrentHeight - dat->frameHeight;
			}
			if (pos != dat->curPos) {
				ScrollWindow(GetDlgItem(hwndDlg, IDC_FRAME), 0, dat->curPos - pos, nullptr, &(dat->frameRect));
				SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, pos, TRUE);
				RECT Invalid = dat->frameRect;
				if (dat->curPos - pos > 0)
					Invalid.bottom = Invalid.top + (dat->curPos - pos);
				else
					Invalid.top = Invalid.bottom + (dat->curPos - pos);

				RedrawWindow(GetDlgItem(hwndDlg, IDC_FRAME), nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
				dat->curPos = pos;
			}
		}

		// Transmit focus set notification to parent window
		if (HIWORD(wParam) == EN_SETFOCUS)
			PostMessage(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(0, EN_SETFOCUS), (LPARAM)hwndDlg);
	}

	if (msg == WM_PAINT) {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &(ps.rcPaint), GetSysColorBrush(COLOR_BTNFACE));
		EndPaint(hwnd, &ps);
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////
//  Add Search field to form

static int JabberSearchAddField(HWND hwndDlg, Data* FieldDat)
{
	if (!FieldDat || !FieldDat->Label || !FieldDat->Var)
		return FALSE;

	HFONT hFont = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0);
	HWND hwndParent = GetDlgItem(hwndDlg, IDC_FRAME);
	LONG_PTR frameExStyle = GetWindowLongPtr(hwndParent, GWL_EXSTYLE);
	frameExStyle |= WS_EX_CONTROLPARENT;
	SetWindowLongPtr(hwndParent, GWL_EXSTYLE, frameExStyle);
	SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FRAME), GWLP_WNDPROC, (LONG_PTR)JabberSearchFrameProc);

	int CornerX = 1;
	int CornerY = 1;
	RECT rect;
	GetClientRect(hwndParent, &rect);
	int width = rect.right - 5 - CornerX;

	int Order = (FieldDat->bHidden) ? -1 : FieldDat->Order;

	HWND hwndLabel = CreateWindowEx(0, L"STATIC", (const wchar_t *)TranslateW(FieldDat->Label), WS_CHILD, CornerX, CornerY + Order * 40, width, 13, hwndParent, nullptr, g_plugin.getInst(), nullptr);
	HWND hwndVar = CreateWindowEx(0 | WS_EX_CLIENTEDGE, L"EDIT", FieldDat->defValue, WS_CHILD | WS_TABSTOP, CornerX + 5, CornerY + Order * 40 + 14, width, 20, hwndParent, nullptr, g_plugin.getInst(), nullptr);
	SendMessage(hwndLabel, WM_SETFONT, (WPARAM)hFont, 0);
	SendMessage(hwndVar, WM_SETFONT, (WPARAM)hFont, 0);
	if (!FieldDat->bHidden) {
		ShowWindow(hwndLabel, SW_SHOW);
		ShowWindow(hwndVar, SW_SHOW);
		EnableWindow(hwndLabel, !FieldDat->bReadOnly);
		SendMessage(hwndVar, EM_SETREADONLY, (WPARAM)FieldDat->bReadOnly, 0);
	}
	
	// remade list
	// reallocation
	JabberSearchData *dat = (JabberSearchData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (dat) {
		dat->pJSInf = (JabberSearchFieldsInfo*)realloc(dat->pJSInf, sizeof(JabberSearchFieldsInfo)*(dat->nJSInfCount + 1));
		dat->pJSInf[dat->nJSInfCount].hwndCaptionItem = hwndLabel;
		dat->pJSInf[dat->nJSInfCount].hwndValueItem = hwndVar;
		dat->pJSInf[dat->nJSInfCount].szFieldCaption = wcsdup(FieldDat->Label);
		dat->pJSInf[dat->nJSInfCount].szFieldName = wcsdup(FieldDat->Var);
		dat->nJSInfCount++;
	}
	return CornerY + Order * 40 + 14 + 20;
}

////////////////////////////////////////////////////////////////////////////////
// Available search field request result handler  (XEP-0055. Examples 2, 7)

void CJabberProto::OnIqResultGetSearchFields(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	if (!searchHandleDlg)
		return;

	const char *type = XmlGetAttr(iqNode, "type");
	if (type == nullptr)
		return;

	if (!mir_strcmp(type, "result")) {
		auto *queryNode = XmlFirstChild(iqNode, "query");
		auto *xNode = XmlGetChildByTag(queryNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);

		ShowWindow(searchHandleDlg, SW_HIDE);
		if (xNode) {
			// 1. Form
			PostMessage(searchHandleDlg, WM_USER + 11, (WPARAM)xNode, 0);
			auto *xcNode = XmlFirstChild(xNode, "instructions");
			if (xcNode)
				SetDlgItemTextUtf(searchHandleDlg, IDC_INSTRUCTIONS, xcNode->GetText());
		}
		else {
			int Order = 0;
			for (auto *chNode : TiXmlEnum(queryNode)) {
				if (!mir_strcmpi(chNode->Name(), "instructions") && chNode->GetText())
					SetDlgItemText(searchHandleDlg, IDC_INSTRUCTIONS, TranslateW(Utf2T(chNode->GetText())));
				else if (chNode->Name()) {
					Data *MyData = (Data*)malloc(sizeof(Data));
					memset(MyData, 0, sizeof(Data));

					MyData->Label = mir_utf8decodeW(chNode->Name());
					MyData->Var = mir_utf8decodeW(chNode->Name());
					MyData->defValue = mir_utf8decodeW(chNode->GetText());
					MyData->Order = Order;
					if (MyData->defValue)
						MyData->bReadOnly = true;
					PostMessage(searchHandleDlg, WM_USER + 10, FALSE, (LPARAM)MyData);
					Order++;
				}
			}
		}

		const char *szFrom = XmlGetAttr(iqNode, "from");
		if (szFrom)
			SearchAddToRecent(szFrom, searchHandleDlg);
		PostMessage(searchHandleDlg, WM_USER + 10, 0, 0);
		ShowWindow(searchHandleDlg, SW_SHOW);
	}
	else if (!mir_strcmp(type, "error")) {
		const char *code = "";
		const char *description = "";
		auto *errorNode = XmlFirstChild(iqNode, "error");
		if (errorNode) {
			code = XmlGetAttr(errorNode, "code");
			description = errorNode->GetText();
		}

		char buff[255];
		mir_snprintf(buff, TranslateU("Error %s %s\r\nPlease select other server"), code, description);
		SetDlgItemTextUtf(searchHandleDlg, IDC_INSTRUCTIONS, buff);
	}
	else SetDlgItemText(searchHandleDlg, IDC_INSTRUCTIONS, TranslateT("Error: unknown reply received\r\nPlease select other server"));
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Return results to search dialog
//  The pmFields is the pointer to map of <field Name, field Label> Not unical but ordered
//	This can help to made result parser routines more simple

static char *nickfields[] = { "nick", "nickname", "fullname", "name", "given", "first", "jid", nullptr };

static int TCharKeyCmp(const char *p1, const char *p2)
{
	return mir_strcmpi(p1, p2);
}

static void SearchReturnResults(CJabberProto *ppro, HANDLE id, LIST<UNIQUE_MAP> &plUsersInfo, UNIQUE_MAP &pmAllFields)
{
	LIST<char> ListOfNonEmptyFields(20, TCharKeyCmp);
	LIST<char> ListOfFields(20);

	// lets fill the ListOfNonEmptyFields but in users order
	for (auto &pmUserData : plUsersInfo) {
		int nUserFields = pmUserData->getCount();
		for (int j = 0; j < nUserFields; j++) {
			char *var = pmUserData->getKeyName(j);
			if (var && ListOfNonEmptyFields.getIndex(var) < 0)
				ListOfNonEmptyFields.insert(var);
		}
	}

	// now fill the ListOfFields but order is from pmAllFields
	int nAllCount = pmAllFields.getCount();
	for (int i = 0; i < nAllCount; i++) {
		char *var = pmAllFields.getUnOrderedKeyName(i);
		if (var && ListOfNonEmptyFields.getIndex(var) < 0)
			continue;
		ListOfFields.insert(var);
	}

	// now lets transfer field names
	int nFieldCount = ListOfFields.getCount();

	CUSTOMSEARCHRESULTS Results = { 0 };
	Results.nSize = sizeof(Results);
	Results.pszFields = (wchar_t**)mir_alloc(sizeof(wchar_t*)*nFieldCount);
	Results.nFieldCount = nFieldCount;

	// Sending Columns Titles
	for (int i = 0; i < nFieldCount; i++) {
		char *var = ListOfFields[i];
		if (var)
			Results.pszFields[i] = mir_utf8decodeW(pmAllFields[var]);
	}

	Results.psr.cbSize = 0; // sending column names
	ppro->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SEARCHRESULT, id, (LPARAM)&Results);
	for (int i = 0; i < nFieldCount; i++)
		replaceStrW(Results.pszFields[i], nullptr);

	// Sending Users Data
	Results.psr.cbSize = sizeof(Results.psr);

	for (auto &pmUserData : plUsersInfo) {
		for (int j = 0; j < nFieldCount; j++) {
			char *var = ListOfFields[j];
			char *value = pmUserData->operator [](var);
			Results.pszFields[j] = value ? mir_utf8decodeW(value) : mir_wstrdup(L" ");
			if (!mir_strcmpi(var, "jid") && value)
				Results.psr.id.w = Results.pszFields[j];
		}

		const char *nick = nullptr;
		for (int k = 0; k < _countof(nickfields) && !nick; k++)
			nick = pmUserData->operator [](nickfields[k]);

		if (nick) {
			Utf2T wszNick(nick);
			wchar_t buff[200];
			if (mir_wstrcmpi(wszNick, Results.psr.id.w))
				mir_snwprintf(buff, L"%s (%s)", wszNick.get(), Results.psr.id.w);
			else
				wcsncpy_s(buff, wszNick, _TRUNCATE);

			Results.psr.nick.w = buff;
		}
		else Results.psr.nick.w = L"";
		Results.psr.flags = PSR_UNICODE;

		ppro->ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SEARCHRESULT, id, (LPARAM)&Results);
		for (int i = 0; i < nFieldCount; i++)
			replaceStrW(Results.pszFields[i], nullptr);
	}
	
	mir_free(Results.pszFields);
}

////////////////////////////////////////////////////////////////////////////////
// Search field request result handler  (XEP-0055. Examples 3, 8)

void CJabberProto::OnIqResultAdvancedSearch(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	const char *type;
	int id;

	UNIQUE_MAP mColumnsNames(10);
	LIST<UNIQUE_MAP> SearchResults(2);

	if (((id = JabberGetPacketID(iqNode)) == -1) || ((type = XmlGetAttr(iqNode, "type")) == nullptr)) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id);
		return;
	}

	if (!mir_strcmp(type, "result")) {
		auto *queryNode = XmlFirstChild(iqNode, "query");
		auto *xNode = XmlGetChildByTag(queryNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (xNode) {
			// 1. Form search results info
			for (auto *fieldNode : TiXmlFilter(XmlFirstChild(xNode, "reported"), "field")) {
				auto *var = XmlGetAttr(fieldNode, "var");
				if (var) {
					auto *label = XmlGetAttr(fieldNode, "label");
					mColumnsNames.insert(var, (label != nullptr) ? label : var);
				}
			}

			for (auto *itemNode : TiXmlFilter(xNode, "item")) {
				UNIQUE_MAP *pUserColumn = new UNIQUE_MAP(10);
				for (auto *fieldNode : TiXmlFilter(itemNode, "field")) {
					if (auto *var = XmlGetAttr(fieldNode, "var")) {
						if (auto *textNode = XmlFirstChild(fieldNode, "value")) {
							if (!mColumnsNames[var])
								mColumnsNames.insert(var, var);
							pUserColumn->insert(var, textNode->GetText());
						}
					}
				}

				SearchResults.insert(pUserColumn);
			}
		}
		else {
			// 2. Field list search results info
			for (auto *itemNode : TiXmlFilter(queryNode, "item")) {
				UNIQUE_MAP *pUserColumn = new UNIQUE_MAP(10);

				auto *jid = XmlGetAttr(itemNode, "jid");
				char *keyReturned;
				mColumnsNames.insertCopyKey("jid", "jid", &keyReturned);
				mColumnsNames.insert("jid", keyReturned);
				pUserColumn->insertCopyKey("jid", jid, nullptr);

				for (auto *child : TiXmlEnum(itemNode)) {
					const char *szColumnName = child->Name();
					if (szColumnName) {
						const char *pszChild = child->GetText();
						if (pszChild && *pszChild) {
							mColumnsNames.insertCopyKey(szColumnName, "", &keyReturned);
							mColumnsNames.insert(szColumnName, keyReturned);
							pUserColumn->insertCopyKey(szColumnName, pszChild, nullptr);
						}
					}
				}

				SearchResults.insert(pUserColumn);
			}
		}
	}
	else if (!mir_strcmp(type, "error")) {
		const char *code = "";
		const char *description = "";
		auto *errorNode = XmlFirstChild(iqNode, "error");
		if (errorNode) {
			code = XmlGetAttr(errorNode, "code");
			description = errorNode->GetText();
		}

		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id);

		char buff[255];
		mir_snprintf(buff, TranslateU("Error %s %s\r\nTry to specify more detailed"), code, description);
		if (searchHandleDlg)
			SetDlgItemTextUtf(searchHandleDlg, IDC_INSTRUCTIONS, buff);
		else
			MessageBox(nullptr, Utf2T(buff), TranslateT("Search error"), MB_OK | MB_ICONSTOP);
		return;
	}

	SearchReturnResults(this, (HANDLE)id, SearchResults, mColumnsNames);

	for (auto &it : SearchResults)
		delete ((UNIQUE_MAP*)it);

	//send success to finish searching
	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id);
}

static BOOL CALLBACK DeleteChildWindowsProc(HWND hwnd, LPARAM)
{
	DestroyWindow(hwnd);
	return TRUE;
}

static void JabberSearchFreeData(HWND hwndDlg, JabberSearchData * dat)
{
	if (!dat->fSearchRequestIsXForm && dat->nJSInfCount && dat->pJSInf) {
		for (int i = 0; i < dat->nJSInfCount; i++) {
			if (dat->pJSInf[i].hwndValueItem)
				DestroyWindow(dat->pJSInf[i].hwndValueItem);
			if (dat->pJSInf[i].hwndCaptionItem)
				DestroyWindow(dat->pJSInf[i].hwndCaptionItem);
			if (dat->pJSInf[i].szFieldCaption)
				free(dat->pJSInf[i].szFieldCaption);
			if (dat->pJSInf[i].szFieldName)
				free(dat->pJSInf[i].szFieldName);
		}
		free(dat->pJSInf);
		dat->pJSInf = nullptr;
	}
	else EnumChildWindows(GetDlgItem(hwndDlg, IDC_FRAME), DeleteChildWindowsProc, 0);

	SendDlgItemMessage(hwndDlg, IDC_FRAME, WM_SETFONT, (WPARAM)SendMessage(hwndDlg, WM_GETFONT, 0, 0), 0);
	dat->nJSInfCount = 0;
	ShowWindow(GetDlgItem(hwndDlg, IDC_VSCROLL), SW_HIDE);
	SetDlgItemText(hwndDlg, IDC_INSTRUCTIONS, TranslateT("Select/type search service URL above and press <Go>"));
}

static void JabberSearchRefreshFrameScroll(HWND hwndDlg, JabberSearchData *dat)
{
	HWND hFrame = GetDlgItem(hwndDlg, IDC_FRAME);
	HWND hwndScroll = GetDlgItem(hwndDlg, IDC_VSCROLL);
	RECT rc;
	GetClientRect(hFrame, &rc);
	GetClientRect(hFrame, &dat->frameRect);
	dat->frameHeight = rc.bottom - rc.top;
	if (dat->frameHeight < dat->CurrentHeight) {
		ShowWindow(hwndScroll, SW_SHOW);
		EnableWindow(hwndScroll, TRUE);
	}
	else ShowWindow(hwndScroll, SW_HIDE);

	SetScrollRange(hwndScroll, SB_CTL, 0, dat->CurrentHeight - dat->frameHeight, FALSE);
}

int CJabberProto::SearchRenewFields(HWND hwndDlg, JabberSearchData *dat)
{
	wchar_t szServerName[100];
	EnableWindow(GetDlgItem(hwndDlg, IDC_GO), FALSE);
	GetDlgItemText(hwndDlg, IDC_SERVER, szServerName, _countof(szServerName));
	dat->CurrentHeight = 0;
	dat->curPos = 0;
	SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, 0, FALSE);

	JabberSearchFreeData(hwndDlg, dat);
	JabberSearchRefreshFrameScroll(hwndDlg, dat);

	SetDlgItemText(hwndDlg, IDC_INSTRUCTIONS, m_bJabberOnline ? TranslateT("Please wait...\r\nConnecting search server...") : TranslateT("You have to be connected to server"));

	if (!m_bJabberOnline)
		return 0;

	searchHandleDlg = hwndDlg;

	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultGetSearchFields, JABBER_IQ_TYPE_GET, T2Utf(szServerName));
	m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY(JABBER_FEAT_JUD));
	return pInfo->GetIqId();
}

static void JabberSearchAddUrlToRecentCombo(HWND hwndDlg, const wchar_t *szAddr)
{
	int lResult = SendDlgItemMessage(hwndDlg, IDC_SERVER, (UINT)CB_FINDSTRING, 0, (LPARAM)szAddr);
	if (lResult == -1)
		SendDlgItemMessage(hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, (LPARAM)szAddr);
}

void CJabberProto::SearchDeleteFromRecent(const char *szAddr, bool deleteLastFromDB)
{
	// search in recent
	for (int i = 0; i < 10; i++) {
		char key[30];
		mir_snprintf(key, "RecentlySearched_%d", i);
		ptrA szValue(getUStringA(key));
		if (szValue == nullptr || mir_strcmpi(szAddr, szValue))
			continue;

		for (int j = i; j < 10; j++) {
			mir_snprintf(key, "RecentlySearched_%d", j + 1);
			szValue = getUStringA(key);
			if (szValue != nullptr) {
				mir_snprintf(key, "RecentlySearched_%d", j);
				setUString(0, key, szValue);
			}
			else {
				if (deleteLastFromDB) {
					mir_snprintf(key, "RecentlySearched_%d", j);
					delSetting(0, key);
				}
				break;
			}
		}
		break;
	}
}

void CJabberProto::SearchAddToRecent(const char *szAddr, HWND hwndDialog)
{
	char key[30];
	SearchDeleteFromRecent(szAddr, true);

	for (int j = 9; j > 0; j--) {
		mir_snprintf(key, "RecentlySearched_%d", j - 1);
		ptrW szValue(getWStringA(key));
		if (szValue != nullptr) {
			mir_snprintf(key, "RecentlySearched_%d", j);
			setWString(0, key, szValue);
		}
	}

	mir_snprintf(key, "RecentlySearched_%d", 0);
	setUString(key, szAddr);
	if (hwndDialog)
		JabberSearchAddUrlToRecentCombo(hwndDialog, Utf2T(szAddr));
}

static INT_PTR CALLBACK JabberSearchAdvancedDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	JabberSearchData *dat = (JabberSearchData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			dat = new JabberSearchData();
			dat->ppro = (CJabberProto *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

			/* Server Combo box */
			ptrA jud(dat->ppro->getStringA("Jud"));
			if (jud != nullptr) {
				SetDlgItemTextA(hwndDlg, IDC_SERVER, jud);
				SendDlgItemMessageA(hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, jud);
			}

			//TO DO: Add Transports here
			for (auto &it : dat->ppro->m_lstTransports)
				if (it != nullptr)
					JabberSearchAddUrlToRecentCombo(hwndDlg, Utf2T(it));

			for (int i = 0; i < 10; i++) {
				char key[30];
				mir_snprintf(key, "RecentlySearched_%d", i);
				ptrW szValue(dat->ppro->getWStringA(key));
				if (szValue != nullptr)
					JabberSearchAddUrlToRecentCombo(hwndDlg, szValue);
			}

			//TO DO: Add 4 recently used
			dat->lastRequestIq = dat->ppro->SearchRenewFields(hwndDlg, dat);
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SERVER) {
			switch (HIWORD(wParam)) {
			case CBN_SETFOCUS:
				PostMessage(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(0, EN_SETFOCUS), (LPARAM)hwndDlg);
				return TRUE;

			case CBN_EDITCHANGE:
				EnableWindow(GetDlgItem(hwndDlg, IDC_GO), TRUE);
				return TRUE;

			case CBN_EDITUPDATE:
				JabberSearchFreeData(hwndDlg, dat);
				EnableWindow(GetDlgItem(hwndDlg, IDC_GO), TRUE);
				return TRUE;

			case CBN_SELENDOK:
				EnableWindow(GetDlgItem(hwndDlg, IDC_GO), TRUE);
				PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_GO, BN_CLICKED), 0);
				return TRUE;
			}
		}
		else if (LOWORD(wParam) == IDC_GO && HIWORD(wParam) == BN_CLICKED) {
			dat->ppro->SearchRenewFields(hwndDlg, dat);
			return TRUE;
		}
		break;

	case WM_SIZE:
		{
			//Resize IDC_FRAME to take full size
			RECT rcForm;
			GetWindowRect(hwndDlg, &rcForm);
			RECT rcFrame;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_FRAME), &rcFrame);
			rcFrame.bottom = rcForm.bottom;
			SetWindowPos(GetDlgItem(hwndDlg, IDC_FRAME), nullptr, 0, 0, rcFrame.right - rcFrame.left, rcFrame.bottom - rcFrame.top, SWP_NOZORDER | SWP_NOMOVE);
			GetWindowRect(GetDlgItem(hwndDlg, IDC_VSCROLL), &rcForm);
			SetWindowPos(GetDlgItem(hwndDlg, IDC_VSCROLL), nullptr, 0, 0, rcForm.right - rcForm.left, rcFrame.bottom - rcFrame.top, SWP_NOZORDER | SWP_NOMOVE);
			JabberSearchRefreshFrameScroll(hwndDlg, dat);
		}
		return TRUE;

	case WM_USER + 11:
		{
			dat->fSearchRequestIsXForm = TRUE;
			if (dat->xNode) {
				dat->doc.DeleteNode(dat->xNode);
				dat->xNode = nullptr;
			}
			TiXmlElement *pNode = (TiXmlElement *)wParam;
			if (pNode) {
				dat->xNode = pNode->DeepClone(&dat->doc)->ToElement();
				JabberFormCreateUI(GetDlgItem(hwndDlg, IDC_FRAME), dat->xNode, &dat->CurrentHeight, TRUE);
			}
			ShowWindow(GetDlgItem(hwndDlg, IDC_FRAME), SW_SHOW);
			dat->nJSInfCount = 1;
		}
		return TRUE;

	case WM_USER + 10:
		{
			Data *MyDat = (Data *)lParam;
			if (MyDat) {
				dat->fSearchRequestIsXForm = (BOOL)wParam;
				dat->CurrentHeight = JabberSearchAddField(hwndDlg, MyDat);
				mir_free(MyDat->Label);
				mir_free(MyDat->Var);
				mir_free(MyDat->defValue);
				free(MyDat);
			}
			else {
				JabberSearchRefreshFrameScroll(hwndDlg, dat);
				ScrollWindow(GetDlgItem(hwndDlg, IDC_FRAME), 0, dat->curPos - 0, nullptr, &(dat->frameRect));
				SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, 0, FALSE);
				dat->curPos = 0;
			}
		}
		return TRUE;

	case WM_MOUSEWHEEL:
		{
			short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (zDelta) {
				int nScrollLines = 0;
				SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, (void *)&nScrollLines, 0);
				for (int i = 0; i < (nScrollLines + 1) / 2; i++)
					SendMessage(hwndDlg, WM_VSCROLL, (zDelta < 0) ? SB_LINEDOWN : SB_LINEUP, 0);
			}
		}
		return TRUE;

	case WM_VSCROLL:
		{
			int pos;
			if (dat != nullptr) {
				pos = dat->curPos;
				switch (LOWORD(wParam)) {
				case SB_LINEDOWN:
					pos += 10;
					break;
				case SB_LINEUP:
					pos -= 10;
					break;
				case SB_PAGEDOWN:
					pos += (dat->CurrentHeight - 10);
					break;
				case SB_PAGEUP:
					pos -= (dat->CurrentHeight - 10);
					break;
				case SB_THUMBTRACK:
					pos = HIWORD(wParam);
					break;
				}
				if (pos > (dat->CurrentHeight - dat->frameHeight))
					pos = dat->CurrentHeight - dat->frameHeight;
				if (pos < 0)
					pos = 0;
				if (dat->curPos != pos) {
					ScrollWindow(GetDlgItem(hwndDlg, IDC_FRAME), 0, dat->curPos - pos, nullptr, &(dat->frameRect));
					SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, pos, TRUE);
					RECT Invalid = dat->frameRect;
					if (dat->curPos - pos > 0)
						Invalid.bottom = Invalid.top + (dat->curPos - pos);
					else
						Invalid.top = Invalid.bottom + (dat->curPos - pos);

					RedrawWindow(GetDlgItem(hwndDlg, IDC_FRAME), nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN);
					dat->curPos = pos;
				}
			}
		}
		return TRUE;

	case WM_DESTROY:
		JabberSearchFreeData(hwndDlg, dat);
		JabberFormDestroyUI(GetDlgItem(hwndDlg, IDC_FRAME));
		delete dat;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		return TRUE;
	}
	return FALSE;
}

HWND CJabberProto::CreateExtendedSearchUI(HWND parent)
{
	if (parent && g_plugin.getInst()) {
		ptrW szServer(getWStringA("LoginServer"));
		if (szServer == nullptr || mir_wstrcmpi(szServer, L"S.ms"))
			return CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SEARCHUSER), parent, JabberSearchAdvancedDlgProc, (LPARAM)this);
	}

	return nullptr; // Failure
}

//////////////////////////////////////////////////////////////////////////
// The function formats request to server

HWND CJabberProto::SearchAdvanced(HWND hwndDlg)
{
	if (!m_bJabberOnline || !hwndDlg)
		return nullptr;	//error

	JabberSearchData *dat = (JabberSearchData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (!dat)
		return nullptr; //error

	// check if server connected (at least one field exists)
	if (dat->nJSInfCount == 0)
		return nullptr;

	// formating request
	bool fRequestNotEmpty = false;

	// get server name
	wchar_t szServerName[100];
	GetDlgItemText(hwndDlg, IDC_SERVER, szServerName, _countof(szServerName));

	// formating query
	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultAdvancedSearch, JABBER_IQ_TYPE_SET, T2Utf(szServerName));
	XmlNodeIq iq(pInfo);
	TiXmlElement *query = iq << XQUERY(JABBER_FEAT_JUD);

	if (m_tszSelectedLang)
		iq << XATTR("xml:lang", m_tszSelectedLang); // i'm sure :)

	// next can be 2 cases:
	// Forms: XEP-0055 Example 7
	if (dat->fSearchRequestIsXForm) {
		fRequestNotEmpty = true;
		JabberFormGetData(GetDlgItem(hwndDlg, IDC_FRAME), query, dat->xNode);
	}
	else { //and Simple fields: XEP-0055 Example 3
		for (int i = 0; i < dat->nJSInfCount; i++) {
			wchar_t szFieldValue[100];
			GetWindowText(dat->pJSInf[i].hwndValueItem, szFieldValue, _countof(szFieldValue));
			if (szFieldValue[0] != 0) {
				XmlAddChildA(query, T2Utf(dat->pJSInf[i].szFieldName).get(), T2Utf(szFieldValue).get());
				fRequestNotEmpty = true;
			}
		}
	}

	if (fRequestNotEmpty) {
		m_ThreadInfo->send(iq);
		return (HWND)pInfo->GetIqId();
	}
	return nullptr;
}
