/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Artem Shpynov
Copyright (ñ) 2012-15 Miranda NG project

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

#include "jabber.h"
#include <CommCtrl.h>
#include "jabber_iq.h"
#include "jabber_caps.h"

///////////////////////////////////////////////////////////////////////////////
// Subclassing of IDC_FRAME to implement more user-friendly fields scrolling
//
static int JabberSearchFrameProc(HWND hwnd, int msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_COMMAND && lParam != 0) {
		HWND hwndDlg=GetParent(hwnd);
		JabberSearchData * dat=(JabberSearchData *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (dat && lParam) {
			int pos=dat->curPos;
			RECT MineRect;
			RECT FrameRect;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_FRAME),&FrameRect);
			GetWindowRect((HWND)lParam, &MineRect);
			if (MineRect.top-10 < FrameRect.top) {
				pos=dat->curPos+(MineRect.top-14-FrameRect.top);
				if (pos<0) pos=0;
			}
			else if (MineRect.bottom > FrameRect.bottom) {
				pos=dat->curPos+(MineRect.bottom-FrameRect.bottom);
				if (dat->frameHeight+pos>dat->CurrentHeight)
					pos=dat->CurrentHeight-dat->frameHeight;
			}
			if (pos != dat->curPos) {
				ScrollWindow(GetDlgItem(hwndDlg, IDC_FRAME), 0, dat->curPos - pos, NULL,  &(dat->frameRect));
				SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, pos, TRUE);
				RECT Invalid=dat->frameRect;
				if (dat->curPos - pos >0)
					Invalid.bottom=Invalid.top+(dat->curPos - pos);
				else
					Invalid.top=Invalid.bottom+(dat->curPos - pos);

				RedrawWindow(GetDlgItem(hwndDlg, IDC_FRAME), NULL, NULL, RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW |RDW_ALLCHILDREN);
				dat->curPos = pos;
			}
		}
		if (HIWORD(wParam)==EN_SETFOCUS) { //Transmit focus set notification to parent window
			PostMessage(GetParent(hwndDlg),WM_COMMAND, MAKEWPARAM(0,EN_SETFOCUS), (LPARAM)hwndDlg);
		}
	}

	if (msg == WM_PAINT) {
		PAINTSTRUCT ps;
		HDC hdc=BeginPaint(hwnd, &ps);
		FillRect(hdc,&(ps.rcPaint),GetSysColorBrush(COLOR_BTNFACE));
		EndPaint(hwnd, &ps);
	}

	return DefWindowProc(hwnd,msg,wParam,lParam);
}

///////////////////////////////////////////////////////////////////////////////
//  Add Search field to form
//
static int JabberSearchAddField(HWND hwndDlg, Data* FieldDat)
{
	if (!FieldDat || !FieldDat->Label || !FieldDat->Var)
		return FALSE;

	HFONT hFont = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0);
	HWND hwndParent=GetDlgItem(hwndDlg,IDC_FRAME);
	LONG_PTR frameExStyle = GetWindowLongPtr(hwndParent, GWL_EXSTYLE);
	frameExStyle |= WS_EX_CONTROLPARENT;
	SetWindowLongPtr(hwndParent, GWL_EXSTYLE, frameExStyle);
	SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_FRAME),GWLP_WNDPROC,(LONG_PTR)JabberSearchFrameProc);

	int CornerX=1;
	int CornerY=1;
	RECT rect;
	GetClientRect(hwndParent,&rect);
	int width=rect.right-5-CornerX;

	int Order=(FieldDat->bHidden) ? -1 : FieldDat->Order;

	HWND hwndLabel=CreateWindowEx(0,_T("STATIC"),(LPCTSTR)TranslateTS(FieldDat->Label),WS_CHILD, CornerX, CornerY + Order*40, width, 13,hwndParent,NULL,hInst,0);
	HWND hwndVar=CreateWindowEx(0|WS_EX_CLIENTEDGE,_T("EDIT"),(LPCTSTR)FieldDat->defValue,WS_CHILD|WS_TABSTOP, CornerX+5, CornerY + Order*40+14, width ,20,hwndParent,NULL,hInst,0);
	SendMessage(hwndLabel, WM_SETFONT, (WPARAM)hFont,0);
	SendMessage(hwndVar, WM_SETFONT, (WPARAM)hFont,0);
	if (!FieldDat->bHidden) {
		ShowWindow(hwndLabel,SW_SHOW);
		ShowWindow(hwndVar,SW_SHOW);
		EnableWindow(hwndLabel,!FieldDat->bReadOnly);
		SendMessage(hwndVar, EM_SETREADONLY, (WPARAM)FieldDat->bReadOnly,0);
	}
	//remade list
	//reallocation
	JabberSearchData *dat=(JabberSearchData *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	if (dat) {
		dat->pJSInf=(JabberSearchFieldsInfo*)realloc(dat->pJSInf, sizeof(JabberSearchFieldsInfo)*(dat->nJSInfCount+1));
		dat->pJSInf[dat->nJSInfCount].hwndCaptionItem=hwndLabel;
		dat->pJSInf[dat->nJSInfCount].hwndValueItem=hwndVar;
		dat->pJSInf[dat->nJSInfCount].szFieldCaption=_tcsdup(FieldDat->Label);
		dat->pJSInf[dat->nJSInfCount].szFieldName=_tcsdup(FieldDat->Var);
		dat->nJSInfCount++;
	}
	return CornerY + Order*40+14 +20;
}

////////////////////////////////////////////////////////////////////////////////
// Available search field request result handler  (XEP-0055. Examples 2, 7)

void CJabberProto::OnIqResultGetSearchFields(HXML iqNode, CJabberIqInfo*)
{
	if (!searchHandleDlg)
		return;

	LPCTSTR type = xmlGetAttrValue(iqNode, _T("type"));
	if (type == NULL)
		return;

	if (!mir_tstrcmp(type, _T("result"))) {
		HXML queryNode = xmlGetNthChild(iqNode, _T("query"), 1);
		HXML xNode = xmlGetChildByTag(queryNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);

		ShowWindow(searchHandleDlg,SW_HIDE);
		if (xNode) {
			//1. Form
			PostMessage(searchHandleDlg, WM_USER+11, (WPARAM)xi.copyNode(xNode), 0);
			HXML xcNode = xmlGetNthChild(xNode, _T("instructions"), 1);
			if (xcNode)
				SetDlgItemText(searchHandleDlg, IDC_INSTRUCTIONS, xmlGetText(xcNode));
		}
		else {
			int Order=0;
			for (int i=0; ; i++) {
				HXML chNode = xmlGetChild(queryNode, i);
				if (!chNode)
					break;

				if (!_tcsicmp(xmlGetName(chNode), _T("instructions")) && xmlGetText(chNode))
					SetDlgItemText(searchHandleDlg,IDC_INSTRUCTIONS,TranslateTS(xmlGetText(chNode)));
				else if (xmlGetName(chNode)) {
					Data *MyData=(Data*)malloc(sizeof(Data));
					memset(MyData,0,sizeof(Data));

					MyData->Label = mir_tstrdup(xmlGetName(chNode));
					MyData->Var = mir_tstrdup(xmlGetName(chNode));
					MyData->defValue = mir_tstrdup(xmlGetText(chNode));
					MyData->Order = Order;
					if (MyData->defValue) MyData->bReadOnly = TRUE;
					PostMessage(searchHandleDlg,WM_USER+10,FALSE,(LPARAM)MyData);
					Order++;
				}
			}
		}

		const TCHAR *szFrom = xmlGetAttrValue(iqNode, _T("from"));
		if (szFrom)
			SearchAddToRecent(szFrom,searchHandleDlg);
		PostMessage(searchHandleDlg,WM_USER+10,0,0);
		ShowWindow(searchHandleDlg,SW_SHOW);
	}
	else if (!mir_tstrcmp(type, _T("error"))) {
		const TCHAR *code=NULL;
		const TCHAR *description=NULL;
		TCHAR buff[255];
		HXML errorNode = xmlGetChild(iqNode, "error");
		if (errorNode) {
			code = xmlGetAttrValue(errorNode, _T("code"));
			description = xmlGetText(errorNode);
		}
		mir_sntprintf(buff, SIZEOF(buff), TranslateT("Error %s %s\r\nPlease select other server"), code ? code : _T(""), description ? description : _T(""));
		SetDlgItemText(searchHandleDlg,IDC_INSTRUCTIONS,buff);
	}
	else SetDlgItemText(searchHandleDlg, IDC_INSTRUCTIONS, TranslateT("Error: unknown reply received\r\nPlease select other server"));
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Return results to search dialog
//  The	pmFields is the pointer to map of <field Name, field Label> Not unical but ordered
//	This can help to made result parser routines more simple

void CJabberProto::SearchReturnResults(HANDLE  id, void * pvUsersInfo, U_TCHAR_MAP * pmAllFields)
{
	LIST<TCHAR> ListOfNonEmptyFields(20,(LIST<TCHAR>::FTSortFunc)TCharKeyCmp);
	LIST<TCHAR> ListOfFields(20);
	LIST<void>* plUsersInfo = (LIST<void>*)pvUsersInfo;
	int i, nUsersFound = plUsersInfo->getCount();

	// lets fill the ListOfNonEmptyFields but in users order
	for (i=0; i < nUsersFound; i++) {
		U_TCHAR_MAP* pmUserData = (U_TCHAR_MAP*)plUsersInfo->operator [](i);
		int nUserFields = pmUserData->getCount();
		for (int j=0; j < nUserFields; j++) {
			TCHAR *var = pmUserData->getKeyName(j);
			if (var && ListOfNonEmptyFields.getIndex(var) < 0)
					ListOfNonEmptyFields.insert(var);
	}	}

	// now fill the ListOfFields but order is from pmAllFields
	int nAllCount = pmAllFields->getCount();
	for (i=0; i < nAllCount; i++) {
		TCHAR * var=pmAllFields->getUnOrderedKeyName(i);
		if (var && ListOfNonEmptyFields.getIndex(var) < 0)
			continue;
		ListOfFields.insert(var);
	}

	// now lets transfer field names
	int nFieldCount = ListOfFields.getCount();

	JABBER_CUSTOMSEARCHRESULTS Results={0};
	Results.nSize=sizeof(Results);
	Results.pszFields=(TCHAR**)mir_alloc(sizeof(TCHAR*)*nFieldCount);
	Results.nFieldCount=nFieldCount;

	/* Sending Columns Titles */
	for (i=0; i < nFieldCount; i++) {
		TCHAR *var = ListOfFields[i];
		if (var)
			Results.pszFields[i] = pmAllFields->operator [](var);
	}

	Results.jsr.hdr.cbSize = 0; // sending column names
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SEARCHRESULT, id, (LPARAM) &Results);

	/* Sending Users Data */
	Results.jsr.hdr.cbSize = sizeof(Results.jsr); // sending user data

	for (i=0; i < nUsersFound; i++) {
		TCHAR buff[200];
		buff[0] = 0;
		Results.jsr.jid[0] = 0;
		U_TCHAR_MAP * pmUserData = (U_TCHAR_MAP *) plUsersInfo->operator [](i);
		for (int j=0; j < nFieldCount; j++) {
			TCHAR* var = ListOfFields[j];
			TCHAR* value = pmUserData->operator [](var);
			Results.pszFields[j] = value ? value : (TCHAR *)_T(" ");
			if (!_tcsicmp(var,_T("jid")) && value)
				_tcsncpy_s(Results.jsr.jid, value, _TRUNCATE);
		}
		{
			TCHAR * nickfields[]={	_T("nick"), 	_T("nickname"),
						_T("fullname"),	_T("name"),
						_T("given"),	_T("first"),
						_T("jid"), 	NULL };
			TCHAR * nick = NULL;
			int k = 0;
			while (nickfields[k] && !nick) {
				nick = pmUserData->operator [](nickfields[k++]);
			}
			if (nick) {
				if (_tcsicmp(nick, Results.jsr.jid)) {
					mir_sntprintf(buff, SIZEOF(buff), _T("%s (%s)"), nick, Results.jsr.jid);
				} else {
					_tcsncpy_s(buff, nick, _TRUNCATE);
				}
				nick = buff;
			}
			Results.jsr.hdr.nick = nick;
			Results.jsr.hdr.flags = PSR_TCHAR;
		}
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SEARCHRESULT, id, (LPARAM) &Results);
		Results.jsr.hdr.nick=NULL;
	}
	mir_free(Results.pszFields);
}

void DestroyKey(TCHAR* key)
{
	mir_free(key);
}

TCHAR* CopyKey(TCHAR* key)
{
	return mir_tstrdup(key);
}

////////////////////////////////////////////////////////////////////////////////
// Search field request result handler  (XEP-0055. Examples 3, 8)

void CJabberProto::OnIqResultAdvancedSearch(HXML iqNode, CJabberIqInfo*)
{
	const TCHAR *type;
	int id;

	U_TCHAR_MAP mColumnsNames(10);
	LIST<void>  SearchResults(2);

	if (((id = JabberGetPacketID(iqNode)) == -1) || ((type = xmlGetAttrValue(iqNode, _T("type"))) == NULL)) {
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
		return;
	}

	if (!mir_tstrcmp(type, _T("result"))) {
		HXML queryNode = xmlGetNthChild(iqNode, _T("query"), 1);
		HXML xNode = xmlGetChildByTag(queryNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (xNode) {
			//1. Form search results info
			HXML reportNode = xmlGetNthChild(xNode, _T("reported"), 1);
			if (reportNode) {
				int i = 1;
				while (HXML fieldNode = xmlGetNthChild(reportNode, _T("field"), i++)) {
					TCHAR *var = (TCHAR*)xmlGetAttrValue(fieldNode, _T("var"));
					if (var) {
						TCHAR *Label = (TCHAR*)xmlGetAttrValue(fieldNode, _T("label"));
						mColumnsNames.insert(var, (Label != NULL) ? Label : var);
			}	}	}

			int i=1;
			HXML itemNode;
			while (itemNode = xmlGetNthChild(xNode, _T("item"), i++)) {
				U_TCHAR_MAP *pUserColumn = new U_TCHAR_MAP(10);
				int j = 1;
				while (HXML fieldNode = xmlGetNthChild(itemNode, _T("field"), j++)) {
					if (TCHAR* var = (TCHAR*)xmlGetAttrValue(fieldNode, _T("var"))) {
						if (TCHAR* Text = (TCHAR*)xmlGetText(xmlGetChild(fieldNode, _T("value")))) {
							if (!mColumnsNames[var])
								mColumnsNames.insert(var,var);
							pUserColumn->insert(var,Text);
				}	}	}

				SearchResults.insert((void*)pUserColumn);
			}
		}
		else {
			//2. Field list search results info
			int i=1;
			while (HXML itemNode = xmlGetNthChild(queryNode, _T("item"), i++)) {
				U_TCHAR_MAP *pUserColumn=new U_TCHAR_MAP(10);

				TCHAR *jid = (TCHAR*)xmlGetAttrValue(itemNode, _T("jid"));
				TCHAR *keyReturned;
				mColumnsNames.insertCopyKey(_T("jid"),_T("jid"),&keyReturned, CopyKey, DestroyKey);
				mColumnsNames.insert(_T("jid"), keyReturned);
				pUserColumn->insertCopyKey(_T("jid"), jid, NULL, CopyKey, DestroyKey);

				for (int j=0; ; j++) {
					HXML child = xmlGetChild(itemNode, j);
					if (!child)
						break;

					const TCHAR *szColumnName = xmlGetName(child);
					if (szColumnName) {
						LPCTSTR ptszChild = xmlGetText(child);
						if (ptszChild && *ptszChild) {
							mColumnsNames.insertCopyKey((TCHAR*)szColumnName,_T(""),&keyReturned, CopyKey, DestroyKey);
							mColumnsNames.insert((TCHAR*)szColumnName,keyReturned);
							pUserColumn->insertCopyKey((TCHAR*)szColumnName, (TCHAR*)ptszChild, NULL, CopyKey, DestroyKey);
				}	}	}

				SearchResults.insert((void*)pUserColumn);
		}	}
	}
	else if (!mir_tstrcmp(type, _T("error"))) {
		const TCHAR *code = NULL;
		const TCHAR *description = NULL;
		TCHAR buff[255];
		HXML errorNode = xmlGetChild(iqNode , "error");
		if (errorNode) {
			code = xmlGetAttrValue(errorNode, _T("code"));
			description = xmlGetText(errorNode);
		}

		mir_sntprintf(buff, SIZEOF(buff), TranslateT("Error %s %s\r\nTry to specify more detailed"), code ? code : _T(""), description ? description : _T(""));
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
		if (searchHandleDlg)
			SetDlgItemText(searchHandleDlg,IDC_INSTRUCTIONS,buff);
		else
			MessageBox(NULL, buff, TranslateT("Search error"), MB_OK|MB_ICONSTOP);
		return;
	}

	SearchReturnResults((HANDLE)id, (void*)&SearchResults, (U_TCHAR_MAP *)&mColumnsNames);

	for (int i=0; i < SearchResults.getCount(); i++)
		delete ((U_TCHAR_MAP *)SearchResults[i]);

	//send success to finish searching
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)id, 0);
}

static BOOL CALLBACK DeleteChildWindowsProc(HWND hwnd, LPARAM)
{
	DestroyWindow(hwnd);
	return TRUE;
}

static void JabberSearchFreeData(HWND hwndDlg, JabberSearchData * dat)
{
	//lock
	if (!dat->fSearchRequestIsXForm && dat->nJSInfCount && dat->pJSInf) {
		for (int i=0; i < dat->nJSInfCount; i++) {
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
		dat->pJSInf=NULL;
	}
	else EnumChildWindows(GetDlgItem(hwndDlg,IDC_FRAME),DeleteChildWindowsProc,0);

	if (dat->xNode)
		xi.destroyNode(dat->xNode);

	SendDlgItemMessage(hwndDlg,IDC_FRAME, WM_SETFONT, (WPARAM)SendMessage(hwndDlg, WM_GETFONT, 0, 0),0);
	dat->nJSInfCount=0;
	ShowWindow(GetDlgItem(hwndDlg,IDC_VSCROLL),SW_HIDE);
	SetDlgItemText(hwndDlg,IDC_INSTRUCTIONS,TranslateT("Select/type search service URL above and press <Go>"));
	//unlock
}

static void JabberSearchRefreshFrameScroll(HWND hwndDlg, JabberSearchData * dat)
{
	HWND hFrame = GetDlgItem(hwndDlg, IDC_FRAME);
	HWND hwndScroll = GetDlgItem(hwndDlg, IDC_VSCROLL);
	RECT rc;
	GetClientRect(hFrame, &rc);
	GetClientRect(hFrame, &dat->frameRect);
	dat->frameHeight = rc.bottom-rc.top;
	if (dat->frameHeight < dat->CurrentHeight) {
		ShowWindow(hwndScroll, SW_SHOW);
		EnableWindow(hwndScroll, TRUE);
	}
	else ShowWindow(hwndScroll, SW_HIDE);

	SetScrollRange(hwndScroll, SB_CTL, 0, dat->CurrentHeight-dat->frameHeight, FALSE);
}

int CJabberProto::SearchRenewFields(HWND hwndDlg, JabberSearchData * dat)
{
	TCHAR szServerName[100];
	EnableWindow(GetDlgItem(hwndDlg, IDC_GO),FALSE);
	GetDlgItemText(hwndDlg,IDC_SERVER,szServerName,SIZEOF(szServerName));
	dat->CurrentHeight = 0;
	dat->curPos = 0;
	SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, 0, FALSE);

	JabberSearchFreeData(hwndDlg, dat);
	JabberSearchRefreshFrameScroll(hwndDlg, dat);

	SetDlgItemText(hwndDlg,IDC_INSTRUCTIONS,m_bJabberOnline ? TranslateT("Please wait...\r\nConnecting search server...") : TranslateT("You have to be connected to server"));

	if (!m_bJabberOnline)
		return 0;

	searchHandleDlg = hwndDlg;

	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultGetSearchFields, JABBER_IQ_TYPE_GET, szServerName);
	m_ThreadInfo->send( XmlNodeIq(pInfo) << XQUERY(_T("jabber:iq:search")));
	return pInfo->GetIqId();
}

static void JabberSearchAddUrlToRecentCombo(HWND hwndDlg, const TCHAR *szAddr)
{
	int lResult = SendDlgItemMessage(hwndDlg, IDC_SERVER, (UINT) CB_FINDSTRING, 0, (LPARAM)szAddr);
	if (lResult == -1)
		SendDlgItemMessage(hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, (LPARAM)szAddr);
}

void CJabberProto::SearchDeleteFromRecent(const TCHAR *szAddr, BOOL deleteLastFromDB)
{
	//search in recent
	for (int i=0; i<10; i++) {
		char key[30];
		mir_snprintf(key, SIZEOF(key), "RecentlySearched_%d", i);
		ptrT szValue( getTStringA(key));
		if (szValue == NULL || _tcsicmp(szAddr, szValue))
			continue;

		for (int j=i; j < 10; j++) {
			mir_snprintf(key, SIZEOF(key), "RecentlySearched_%d", j + 1);
			szValue = getTStringA(key);
			if (szValue != NULL) {
				mir_snprintf(key, SIZEOF(key), "RecentlySearched_%d", j);
				setTString(NULL, key, szValue);
			}
			else {
				if (deleteLastFromDB) {
					mir_snprintf(key, SIZEOF(key), "RecentlySearched_%d", j);
					delSetting(NULL,key);
				}
				break;
		}	}
		break;
}	}

void CJabberProto::SearchAddToRecent(const TCHAR *szAddr, HWND hwndDialog)
{
	char key[30];
	SearchDeleteFromRecent(szAddr);

	for (int j=9; j > 0; j--) {
		mir_snprintf(key, SIZEOF(key), "RecentlySearched_%d", j - 1);
		ptrT szValue( getTStringA(key));
		if (szValue != NULL) {
			mir_snprintf(key, SIZEOF(key), "RecentlySearched_%d", j);
			setTString(NULL, key, szValue);
	}	}

	mir_snprintf(key, SIZEOF(key), "RecentlySearched_%d", 0);
	setTString(key, szAddr);
	if (hwndDialog)
		JabberSearchAddUrlToRecentCombo(hwndDialog, szAddr);
}

static INT_PTR CALLBACK JabberSearchAdvancedDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	JabberSearchData* dat = (JabberSearchData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			dat = (JabberSearchData *)mir_calloc(sizeof(JabberSearchData));
			dat->ppro = (CJabberProto*)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

			/* Server Combo box */
			ptrA jud(dat->ppro->getStringA("Jud"));
			char *szServerName = (jud == NULL) ? "users.jabber.org": jud;
			SetDlgItemTextA(hwndDlg,IDC_SERVER,szServerName);
			SendDlgItemMessageA(hwndDlg,IDC_SERVER,CB_ADDSTRING,0,(LPARAM)szServerName);
			//TO DO: Add Transports here
			int i, transpCount = dat->ppro->m_lstTransports.getCount();
			for (i=0; i < transpCount; i++) {
				TCHAR *szTransp = dat->ppro->m_lstTransports[i];
				if (szTransp)
					JabberSearchAddUrlToRecentCombo(hwndDlg, szTransp);
			}

			for (i=0; i < 10; i++) {
				char key[30];
				mir_snprintf(key, SIZEOF(key), "RecentlySearched_%d", i);
				ptrT szValue( dat->ppro->getTStringA(key));
				if (szValue != NULL)
					JabberSearchAddUrlToRecentCombo(hwndDlg, szValue);
			}

			//TO DO: Add 4 recently used
			dat->lastRequestIq = dat->ppro->SearchRenewFields(hwndDlg,dat);
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SERVER) {
			switch (HIWORD(wParam)) {
			case CBN_SETFOCUS:
				PostMessage(GetParent(hwndDlg),WM_COMMAND, MAKEWPARAM(0,EN_SETFOCUS), (LPARAM)hwndDlg);
				return TRUE;

			case CBN_EDITCHANGE:
				EnableWindow(GetDlgItem(hwndDlg, IDC_GO),TRUE);
				return TRUE;

			case CBN_EDITUPDATE:
				JabberSearchFreeData(hwndDlg, dat);
				EnableWindow(GetDlgItem(hwndDlg, IDC_GO),TRUE);
				return TRUE;

			case CBN_SELENDOK:
				EnableWindow(GetDlgItem(hwndDlg, IDC_GO),TRUE);
				PostMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_GO,BN_CLICKED),0);
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
			GetWindowRect(hwndDlg,  &rcForm);
			RECT rcFrame;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_FRAME), &rcFrame);
			rcFrame.bottom = rcForm.bottom;
			SetWindowPos(GetDlgItem(hwndDlg,IDC_FRAME),NULL,0,0,rcFrame.right-rcFrame.left,rcFrame.bottom-rcFrame.top,SWP_NOZORDER|SWP_NOMOVE);
			GetWindowRect(GetDlgItem(hwndDlg,IDC_VSCROLL), &rcForm);
			SetWindowPos(GetDlgItem(hwndDlg,IDC_VSCROLL),NULL,0,0,rcForm.right-rcForm.left,rcFrame.bottom-rcFrame.top,SWP_NOZORDER|SWP_NOMOVE);
			JabberSearchRefreshFrameScroll(hwndDlg, dat);
		}
		return TRUE;

	case WM_USER+11:
		{
			dat->fSearchRequestIsXForm=TRUE;
			dat->xNode = (HXML)wParam;
			JabberFormCreateUI(GetDlgItem(hwndDlg, IDC_FRAME), dat->xNode, &dat->CurrentHeight,TRUE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_FRAME), SW_SHOW);
			dat->nJSInfCount=1;
			return TRUE;
		}
	case WM_USER+10:
		{
			Data* MyDat = (Data*)lParam;
			if (MyDat) {
				dat->fSearchRequestIsXForm = (BOOL)wParam;
				dat->CurrentHeight = JabberSearchAddField(hwndDlg,MyDat);
				mir_free(MyDat->Label);
				mir_free(MyDat->Var);
				mir_free(MyDat->defValue);
				free(MyDat);
			}
			else
			{
				JabberSearchRefreshFrameScroll(hwndDlg,dat);
				ScrollWindow(GetDlgItem(hwndDlg, IDC_FRAME), 0, dat->curPos - 0, NULL,  &(dat->frameRect));
				SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, 0, FALSE);
				dat->curPos=0;
			}
			return TRUE;
		}
	case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (zDelta) {
				int nScrollLines=0;
				SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,(void*)&nScrollLines,0);
				for (int i=0; i<(nScrollLines+1)/2; i++)
					SendMessage(hwndDlg,WM_VSCROLL, (zDelta<0)?SB_LINEDOWN:SB_LINEUP,0);
		}	}
		return TRUE;

	case WM_VSCROLL:
		{
			int pos;
			if (dat != NULL) {
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
					ScrollWindow(GetDlgItem(hwndDlg, IDC_FRAME), 0, dat->curPos - pos, NULL ,  &(dat->frameRect));
					SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, pos, TRUE);
					RECT Invalid=dat->frameRect;
					if (dat->curPos - pos >0)
						Invalid.bottom=Invalid.top+(dat->curPos - pos);
					else
						Invalid.top=Invalid.bottom+(dat->curPos - pos);

					RedrawWindow(GetDlgItem(hwndDlg, IDC_FRAME), NULL, NULL, RDW_UPDATENOW |RDW_ALLCHILDREN);
					dat->curPos = pos;
		}	}	}
		return TRUE;

	case WM_DESTROY:
		JabberSearchFreeData(hwndDlg, dat);
		JabberFormDestroyUI(GetDlgItem(hwndDlg, IDC_FRAME));
		mir_free(dat);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		return TRUE;
	}
	return FALSE;
}

HWND __cdecl CJabberProto::CreateExtendedSearchUI(HWND parent)
{
	if (parent && hInst) {
		ptrT szServer( getTStringA("LoginServer"));
		if (szServer == NULL || _tcsicmp(szServer, _T("S.ms")))
			return CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SEARCHUSER), parent, JabberSearchAdvancedDlgProc, (LPARAM)this);
	}

	return 0; // Failure
}

//////////////////////////////////////////////////////////////////////////
// The function formats request to server

HWND __cdecl CJabberProto::SearchAdvanced(HWND hwndDlg)
{
	if (!m_bJabberOnline || !hwndDlg)
		return 0;	//error

	JabberSearchData * dat=(JabberSearchData *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	if (!dat)
		return 0; //error

	// check if server connected (at least one field exists)
	if (dat->nJSInfCount == 0)
		return 0;

	// formating request
	BOOL fRequestNotEmpty=FALSE;

	// get server name
	TCHAR szServerName[100];
	GetDlgItemText(hwndDlg, IDC_SERVER, szServerName, SIZEOF(szServerName));

	// formating query
	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultAdvancedSearch, JABBER_IQ_TYPE_SET, szServerName);
	XmlNodeIq iq(pInfo);
	HXML query = iq << XQUERY(_T("jabber:iq:search"));

	if (m_tszSelectedLang)
		iq << XATTR(_T("xml:lang"), m_tszSelectedLang); // i'm sure :)

	// next can be 2 cases:
	// Forms: XEP-0055 Example 7
	if (dat->fSearchRequestIsXForm) {
		fRequestNotEmpty=TRUE;
		HXML n = JabberFormGetData(GetDlgItem(hwndDlg, IDC_FRAME), dat->xNode);
		xmlAddChild(query, n);
		xi.destroyNode(n);
	}
	else { //and Simple fields: XEP-0055 Example 3
		for (int i=0; i<dat->nJSInfCount; i++) {
			TCHAR szFieldValue[100];
			GetWindowText(dat->pJSInf[i].hwndValueItem, szFieldValue, SIZEOF(szFieldValue));
			if (szFieldValue[0] != 0) {
				xmlAddChild(query, dat->pJSInf[i].szFieldName, szFieldValue);
				fRequestNotEmpty=TRUE;
	}	}	}

	if (fRequestNotEmpty) {
		m_ThreadInfo->send(iq);
		return (HWND)pInfo->GetIqId();
	}
	return 0;
}
