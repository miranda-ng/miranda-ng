/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Artem Shpynov
Copyright (c) 2012-14  Miranda NG project

Module implements an XMPP protocol extension for reporting and executing ad-hoc,
human-oriented commands according to XEP-0050: Ad-Hoc Commands
http://www.xmpp.org/extensions/xep-0050.html

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
#include "jabber_iq.h"
#include "jabber_caps.h"

#define ShowDlgItem(a, b, c)	 ShowWindow(GetDlgItem(a, b), c)
#define EnableDlgItem(a, b, c) EnableWindow(GetDlgItem(a, b), c)

enum
{
	JAHM_COMMANDLISTRESULT = WM_USER+1,
	JAHM_PROCESSRESULT
};

//Declarations
static INT_PTR CALLBACK JabberAdHoc_CommandDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

//implementations

// convert iqID to dialog hwnd
HWND CJabberProto::GetWindowFromIq(CJabberIqInfo *pInfo)
{
	if (pInfo->GetUserData() != NULL)
		return HWND(pInfo->GetUserData());
	return m_hwndCommandWindow;
}

// Callback to clear form content
static BOOL CALLBACK sttDeleteChildWindowsProc(HWND hwnd, LPARAM)
{
	DestroyWindow(hwnd);
	return TRUE;
}

static void sttEnableControls(HWND hwndDlg, BOOL bEnable, const int *controlsID)
{
	for (int i = 0; controlsID[i] != 0; i++)
		EnableDlgItem(hwndDlg, controlsID[i], bEnable);
}

static void sttShowControls(HWND hwndDlg, BOOL bShow, int *controlsID)
{
	for (int i = 0; controlsID[i] != 0; i++)
		ShowDlgItem(hwndDlg, controlsID[i], (bShow) ? SW_SHOW : SW_HIDE);
}

static void JabberAdHoc_RefreshFrameScroll(HWND hwndDlg, JabberAdHocData * dat)
{
	HWND hFrame = GetDlgItem(hwndDlg, IDC_FRAME);
	HWND hwndScroll = GetDlgItem(hwndDlg, IDC_VSCROLL);
	RECT rc;
	RECT rcScrollRc;
	GetClientRect(hFrame, &rc);
	GetClientRect(hFrame, &dat->frameRect);
	GetWindowRect(hwndScroll, &rcScrollRc);
	dat->frameRect.right -= (rcScrollRc.right - rcScrollRc.left);
	dat->frameHeight = rc.bottom - rc.top;
	if (dat->frameHeight < dat->CurrentHeight) {
		ShowWindow(hwndScroll, SW_SHOW);
		EnableWindow(hwndScroll, TRUE);
	}
	else ShowWindow(hwndScroll, SW_HIDE);

	SetScrollRange(hwndScroll, SB_CTL, 0, dat->CurrentHeight - dat->frameHeight, FALSE);
}

//////////////////////////////////////////////////////////////////////////
// Iq handlers
// Forwards to dialog window procedure

void CJabberProto::OnIqResult_ListOfCommands(HXML iqNode, CJabberIqInfo *pInfo)
{
	SendMessage(GetWindowFromIq(pInfo), JAHM_COMMANDLISTRESULT, 0, (LPARAM)xi.copyNode(iqNode));
}

void CJabberProto::OnIqResult_CommandExecution(HXML iqNode, CJabberIqInfo *pInfo)
{
	SendMessage(GetWindowFromIq(pInfo), JAHM_PROCESSRESULT, (WPARAM)xi.copyNode(iqNode), 0);
}

void CJabberProto::AdHoc_RequestListOfCommands(TCHAR * szResponder, HWND hwndDlg)
{
	m_ThreadInfo->send(XmlNodeIq(AddIQ(&CJabberProto::OnIqResult_ListOfCommands, JABBER_IQ_TYPE_GET, szResponder, 0, -1, hwndDlg))
		<< XQUERY(JABBER_FEAT_DISCO_ITEMS) << XATTR(_T("node"), JABBER_FEAT_COMMANDS));
}

int CJabberProto::AdHoc_ExecuteCommand(HWND hwndDlg, TCHAR*, JabberAdHocData* dat)
{
	for (int i = 1;; i++) {
		HXML itemNode = xmlGetNthChild(dat->CommandsNode, _T("item"), i);
		if (!itemNode)
			break;

		if (BST_UNCHECKED == IsDlgButtonChecked(GetDlgItem(hwndDlg, IDC_FRAME), i))
			continue;

		const TCHAR *node = xmlGetAttrValue(itemNode, _T("node"));
		if (node) {
			const TCHAR *jid2 = xmlGetAttrValue(itemNode, _T("jid"));
			m_ThreadInfo->send(
				XmlNodeIq(AddIQ(&CJabberProto::OnIqResult_CommandExecution, JABBER_IQ_TYPE_SET, jid2, 0, -1, hwndDlg))
				<< XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS) << XATTR(_T("node"), node) << XATTR(_T("action"), _T("execute")));

			EnableDlgItem(hwndDlg, IDC_SUBMIT, FALSE);
			SetDlgItemText(hwndDlg, IDC_SUBMIT, TranslateT("OK"));
		}
	}

	xi.destroyNode(dat->CommandsNode); dat->CommandsNode = NULL;
	return TRUE;
}

// Messages handlers
int CJabberProto::AdHoc_OnJAHMCommandListResult(HWND hwndDlg, HXML iqNode, JabberAdHocData* dat)
{
	int nodeIdx = 0;
	const TCHAR * type = xmlGetAttrValue(iqNode, _T("type"));
	if (!type || !_tcscmp(type, _T("error"))) {
		// error occurred here
		TCHAR buff[255];
		const TCHAR *code = NULL;
		const TCHAR *description = NULL;

		HXML errorNode = xmlGetChild(iqNode, "error");
		if (errorNode) {
			code = xmlGetAttrValue(errorNode, _T("code"));
			description = xmlGetText(errorNode);
		}
		mir_sntprintf(buff, SIZEOF(buff), TranslateT("Error %s %s"), (code) ? code : _T(""), (description) ? description : _T(""));
		JabberFormSetInstruction(hwndDlg, buff);
	}
	else if (!_tcscmp(type, _T("result"))) {
		BOOL validResponse = FALSE;
		EnumChildWindows(GetDlgItem(hwndDlg, IDC_FRAME), sttDeleteChildWindowsProc, 0);
		dat->CurrentHeight = 0;
		dat->curPos = 0;
		SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, 0, FALSE);
		HXML queryNode = xmlGetChild(iqNode, "query");
		if (queryNode) {
			const TCHAR *xmlns = xmlGetAttrValue(queryNode, _T("xmlns"));
			const TCHAR *node = xmlGetAttrValue(queryNode, _T("node"));
			if (xmlns && node && !_tcscmp(xmlns, JABBER_FEAT_DISCO_ITEMS) && !_tcscmp(node, JABBER_FEAT_COMMANDS))
				validResponse = TRUE;
		}
		if (queryNode && xmlGetChild(queryNode, 0) && validResponse) {
			dat->CommandsNode = xi.copyNode(queryNode);

			int ypos = 20;
			for (nodeIdx = 1;; nodeIdx++) {
				HXML itemNode = xmlGetNthChild(queryNode, _T("item"), nodeIdx);
				if (!itemNode)
					break;

				const TCHAR *name = xmlGetAttrValue(itemNode, _T("name"));
				if (!name) name = xmlGetAttrValue(itemNode, _T("node"));
				ypos = AdHoc_AddCommandRadio(GetDlgItem(hwndDlg, IDC_FRAME), TranslateTS(name), nodeIdx, ypos, (nodeIdx == 1) ? 1 : 0);
				dat->CurrentHeight = ypos;
			}
		}

		if (nodeIdx > 1) {
			JabberFormSetInstruction(hwndDlg, TranslateT("Select Command"));
			ShowDlgItem(hwndDlg, IDC_FRAME, SW_SHOW);
			ShowDlgItem(hwndDlg, IDC_VSCROLL, SW_SHOW);
			EnableDlgItem(hwndDlg, IDC_SUBMIT, TRUE);
		}
		else JabberFormSetInstruction(hwndDlg, TranslateT("Not supported"));
	}

	JabberAdHoc_RefreshFrameScroll(hwndDlg, dat);
	return (TRUE);
}

int CJabberProto::AdHoc_OnJAHMProcessResult(HWND hwndDlg, HXML workNode, JabberAdHocData* dat)
{
	EnumChildWindows(GetDlgItem(hwndDlg, IDC_FRAME), sttDeleteChildWindowsProc, 0);
	dat->CurrentHeight = 0;
	dat->curPos = 0;
	SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, 0, FALSE);

	if (workNode == NULL)
		return TRUE;

	dat->AdHocNode = xi.copyNode(workNode);

	const TCHAR *type;
	if ((type = xmlGetAttrValue(workNode, _T("type"))) == NULL) return TRUE;
	if (!mir_tstrcmp(type, _T("result"))) {
		// wParam = <iq/> node from responder as a result of command execution
		HXML commandNode, xNode;
		if ((commandNode = xmlGetChild(dat->AdHocNode, _T("command"))) == NULL)
			return TRUE;

		const TCHAR *status = xmlGetAttrValue(commandNode, _T("status"));
		if (!status)
			status = _T("completed");

		if ((xNode = xmlGetChild(commandNode, "x"))) {
			// use jabber:x:data form
			HWND hFrame = GetDlgItem(hwndDlg, IDC_FRAME);
			ShowWindow(GetDlgItem(hwndDlg, IDC_FRAME_TEXT), SW_HIDE);
			if (LPCTSTR ptszInstr = xmlGetText(xmlGetChild(xNode, "instructions")))
				JabberFormSetInstruction(hwndDlg, ptszInstr);
			else if (LPCTSTR ptszTitle = xmlGetText(xmlGetChild(xNode, "title")))
				JabberFormSetInstruction(hwndDlg, ptszTitle);
			else
				JabberFormSetInstruction(hwndDlg, TranslateTS(status));
			JabberFormCreateUI(hFrame, xNode, &dat->CurrentHeight);
			ShowDlgItem(hwndDlg, IDC_FRAME, SW_SHOW);
		}
		else {
			//NO X FORM
			int toHide[] = { IDC_FRAME_TEXT, IDC_FRAME, IDC_VSCROLL, 0 };
			sttShowControls(hwndDlg, FALSE, toHide);

			LPCTSTR noteText = xmlGetText(xmlGetChild(commandNode, "note"));
			JabberFormSetInstruction(hwndDlg, noteText ? noteText : TranslateTS(status));
		}

		// check actions
		HXML actionsNode = xmlGetChild(commandNode, "actions");
		if (actionsNode != NULL) {
			ShowDlgItem(hwndDlg, IDC_PREV, (xmlGetChild(actionsNode, "prev") != NULL) ? SW_SHOW : SW_HIDE);
			ShowDlgItem(hwndDlg, IDC_NEXT, (xmlGetChild(actionsNode, "next") != NULL) ? SW_SHOW : SW_HIDE);
			ShowDlgItem(hwndDlg, IDC_COMPLETE, (xmlGetChild(actionsNode, "complete") != NULL) ? SW_SHOW : SW_HIDE);
			ShowDlgItem(hwndDlg, IDC_SUBMIT, SW_HIDE);

			int toEnable[] = { IDC_PREV, IDC_NEXT, IDC_COMPLETE, 0 };
			sttEnableControls(hwndDlg, TRUE, toEnable);
		}
		else {
			int toHide[] = { IDC_PREV, IDC_NEXT, IDC_COMPLETE, 0 };
			sttShowControls(hwndDlg, FALSE, toHide);

			ShowDlgItem(hwndDlg, IDC_SUBMIT, SW_SHOW);
			EnableDlgItem(hwndDlg, IDC_SUBMIT, TRUE);
		}

		if (!status || _tcscmp(status, _T("executing"))) {
			ShowDlgItem(hwndDlg, IDC_SUBMIT, SW_HIDE);
			SetWindowText(GetDlgItem(hwndDlg, IDCANCEL), TranslateT("Done"));
		}
	}
	else if (!mir_tstrcmp(type, _T("error"))) {
		// error occurred here
		int toHide[] = { IDC_FRAME, IDC_FRAME_TEXT, IDC_VSCROLL, IDC_PREV, IDC_NEXT, IDC_COMPLETE, IDC_SUBMIT, 0};
		sttShowControls(hwndDlg, FALSE, toHide);

		const TCHAR *code=NULL;
		const TCHAR *description=NULL;
		TCHAR buff[255];
		HXML errorNode = xmlGetChild(workNode , "error");
		if (errorNode) {
			code = xmlGetAttrValue(errorNode, _T("code"));
			description = xmlGetText(errorNode);
		}
		mir_sntprintf(buff, SIZEOF(buff), TranslateT("Error %s %s"), code ? code : _T(""), description ? description : _T(""));
		JabberFormSetInstruction(hwndDlg,buff);
	}
	JabberAdHoc_RefreshFrameScroll(hwndDlg, dat);
	return TRUE;
}

int CJabberProto::AdHoc_SubmitCommandForm(HWND hwndDlg, JabberAdHocData* dat, TCHAR* action)
{
	HXML commandNode = xmlGetChild(dat->AdHocNode, "command");
	HXML xNode = xmlGetChild(commandNode, "x");
	HXML dataNode = JabberFormGetData(GetDlgItem(hwndDlg, IDC_FRAME), xNode);

	LPCTSTR jid2 = xmlGetAttrValue(dat->AdHocNode, _T("from"));
	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResult_CommandExecution, JABBER_IQ_TYPE_SET, jid2, 0, -1, hwndDlg));
	HXML command = iq << XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS);

	const TCHAR *sessionId = xmlGetAttrValue(commandNode, _T("sessionid"));
	if (sessionId)
		command << XATTR(_T("sessionid"), sessionId);

	const TCHAR *node = xmlGetAttrValue(commandNode, _T("node"));
	if (node)
		command << XATTR(_T("node"), node);

	if (action)
		command << XATTR(_T("action"), action);

	xmlAddChild(command, dataNode);
	m_ThreadInfo->send(iq);

	xi.destroyNode(dataNode);

	JabberFormSetInstruction(hwndDlg, TranslateT("In progress. Please Wait..."));

	static const int toDisable[] = { IDC_SUBMIT, IDC_PREV, IDC_NEXT, IDC_COMPLETE, 0 };
	sttEnableControls(hwndDlg, FALSE, toDisable);
	return TRUE;
}

int CJabberProto::AdHoc_AddCommandRadio(HWND hFrame, TCHAR * labelStr, int id, int ypos, int value)
{
	RECT strRect = { 0 };
	int verticalStep = 4;
	int ctrlMinHeight = 18;

	RECT rcFrame;
	GetClientRect(hFrame, &rcFrame);

	int ctrlOffset = 20;
	int ctrlWidth = rcFrame.right - ctrlOffset;

	HDC hdc = GetDC(hFrame);
	int labelHeight = max(ctrlMinHeight, DrawText(hdc, labelStr, -1, &strRect, DT_CALCRECT));
	ctrlWidth = min(ctrlWidth, strRect.right - strRect.left + 20);
	ReleaseDC(hFrame, hdc);

	HWND hCtrl = CreateWindowEx(0, _T("button"), labelStr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON, ctrlOffset, ypos, ctrlWidth, labelHeight, hFrame, (HMENU)id, hInst, NULL);
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)SendMessage(GetParent(hFrame), WM_GETFONT, 0, 0), 0);
	SendMessage(hCtrl, BM_SETCHECK, value, 0);
	return (ypos + labelHeight + verticalStep);
}

static INT_PTR CALLBACK JabberAdHoc_CommandDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	JabberAdHocData* dat = (JabberAdHocData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		dat = (JabberAdHocData*)mir_calloc(sizeof(JabberAdHocData));
		{
			CJabberAdhocStartupParams* pStartupParams = (CJabberAdhocStartupParams *)lParam;
			dat->ResponderJID = mir_tstrdup(pStartupParams->m_szJid);
			dat->proto = pStartupParams->m_pProto;

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			WindowSetIcon(hwndDlg, dat->proto, "adhoc");
			dat->proto->m_hwndCommandWindow = hwndDlg;
			TranslateDialogDefault(hwndDlg);

			// hide frame first
			LONG frameExStyle = GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FRAME), GWL_EXSTYLE);
			frameExStyle |= WS_EX_CONTROLPARENT;

			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FRAME), GWL_EXSTYLE, frameExStyle);

			int toHide[] = { IDC_FRAME, IDC_VSCROLL, IDC_PREV, IDC_NEXT, IDC_COMPLETE, IDC_FRAME_TEXT, 0 };
			sttShowControls(hwndDlg, FALSE, toHide);

			int toShow[] = { IDC_INSTRUCTION, IDC_SUBMIT, IDCANCEL, 0 };
			sttShowControls(hwndDlg, TRUE, toShow);

			EnableDlgItem(hwndDlg, IDC_VSCROLL, TRUE);

			SetWindowPos(GetDlgItem(hwndDlg, IDC_VSCROLL), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

			SetDlgItemText(hwndDlg, IDC_SUBMIT, TranslateT("Execute"));
			JabberFormSetInstruction(hwndDlg, TranslateT("Requesting command list. Please wait..."));

			if (!pStartupParams->m_szNode) {
				dat->proto->AdHoc_RequestListOfCommands(pStartupParams->m_szJid, hwndDlg);

				TCHAR Caption[512];
				mir_sntprintf(Caption, SIZEOF(Caption), TranslateT("Jabber Ad-Hoc commands at %s"), dat->ResponderJID);
				SetWindowText(hwndDlg, Caption);
			}
			else {
				dat->proto->m_ThreadInfo->send(
					XmlNodeIq(dat->proto->AddIQ(&CJabberProto::OnIqResult_CommandExecution, JABBER_IQ_TYPE_SET, pStartupParams->m_szJid, 0, -1, hwndDlg))
					<< XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS)
					<< XATTR(_T("node"), pStartupParams->m_szNode) << XATTR(_T("action"), _T("execute")));

				EnableDlgItem(hwndDlg, IDC_SUBMIT, FALSE);
				SetDlgItemText(hwndDlg, IDC_SUBMIT, TranslateT("OK"));

				TCHAR Caption[512];
				mir_sntprintf(Caption, SIZEOF(Caption), TranslateT("Sending Ad-Hoc command to %s"), dat->ResponderJID);
				SetWindowText(hwndDlg, Caption);
			}

			delete pStartupParams;
		}
		return TRUE;

	case WM_CTLCOLORSTATIC:
		switch (GetWindowLongPtr((HWND)lParam, GWL_ID)) {
		case IDC_WHITERECT: case IDC_INSTRUCTION: case IDC_TITLE:
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
		}
		return NULL;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_PREV:
			return dat->proto->AdHoc_SubmitCommandForm(hwndDlg, dat, _T("prev"));
		case IDC_NEXT:
			return dat->proto->AdHoc_SubmitCommandForm(hwndDlg, dat, _T("next"));
		case IDC_COMPLETE:
			return dat->proto->AdHoc_SubmitCommandForm(hwndDlg, dat, _T("complete"));
		case IDC_SUBMIT:
			if (!dat->AdHocNode && dat->CommandsNode && LOWORD(wParam) == IDC_SUBMIT)
				return dat->proto->AdHoc_ExecuteCommand(hwndDlg, dat->ResponderJID, dat);
			else
				return dat->proto->AdHoc_SubmitCommandForm(hwndDlg, dat, NULL);
		case IDCLOSE:
		case IDCANCEL:
			xi.destroyNode(dat->AdHocNode); dat->AdHocNode = NULL;
			DestroyWindow(hwndDlg);
			return TRUE;
		}
		break;

	case JAHM_COMMANDLISTRESULT:
		return dat->proto->AdHoc_OnJAHMCommandListResult(hwndDlg, (HXML)lParam, dat);

	case JAHM_PROCESSRESULT:
		return dat->proto->AdHoc_OnJAHMProcessResult(hwndDlg, (HXML)wParam, dat);

	case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (zDelta) {
				int nScrollLines = 0;
				SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, (void*)&nScrollLines, 0);
				for (int i = 0; i < (nScrollLines + 1) / 2; i++)
					SendMessage(hwndDlg, WM_VSCROLL, (zDelta < 0) ? SB_LINEDOWN : SB_LINEUP, 0);
			}
		}
		return TRUE;

	case WM_VSCROLL:
		if (dat != NULL) {
			int pos = dat->curPos;
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
				ScrollWindow(GetDlgItem(hwndDlg, IDC_FRAME), 0, dat->curPos - pos, NULL, &(dat->frameRect));
				SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, pos, TRUE);
				RECT Invalid = dat->frameRect;
				if (dat->curPos - pos >0)
					Invalid.bottom = Invalid.top + (dat->curPos - pos);
				else
					Invalid.top = Invalid.bottom + (dat->curPos - pos);

				RedrawWindow(GetDlgItem(hwndDlg, IDC_FRAME), NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN);
				dat->curPos = pos;
			}
		}
		break;

	case WM_DESTROY:
		JabberFormDestroyUI(GetDlgItem(hwndDlg, IDC_FRAME));
		WindowFreeIcon(hwndDlg);

		dat->proto->m_hwndCommandWindow = NULL;
		mir_free(dat->ResponderJID);
		xi.destroyNode(dat->CommandsNode);
		xi.destroyNode(dat->AdHocNode);
		mir_free(dat);
		dat = NULL;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}
	return FALSE;
}

int __cdecl CJabberProto::ContactMenuRunCommands(WPARAM hContact, LPARAM lParam)
{
	int res = -1;

	if ((hContact != NULL || lParam != 0) && m_bJabberOnline) {
		ptrT szJid(getTStringA(hContact, "jid"));
		if (hContact && szJid != NULL) {
			JABBER_LIST_ITEM *item = NULL;
			int selected = 0;
			TCHAR jid[JABBER_MAX_JID_LEN];
			_tcsncpy_s(jid, szJid, _TRUNCATE);
			{
				mir_cslock lck(m_csLists);
				item = ListGetItemPtr(LIST_ROSTER, jid);
				if (item) {
					if (item->arResources.getCount() > 1) {
						HMENU hMenu = CreatePopupMenu();
						for (int i = 0; i < item->arResources.getCount(); i++)
							AppendMenu(hMenu, MF_STRING, i + 1, item->arResources[i]->m_tszResourceName);
						HWND hwndTemp = CreateWindowEx(WS_EX_TOOLWINDOW, _T("button"), _T("PopupMenuHost"), 0, 0, 0, 10, 10, NULL, NULL, hInst, NULL);
						SetForegroundWindow(hwndTemp);
						RECT rc;
						POINT pt;
						GetCursorPos(&pt);
						selected = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndTemp, &rc);
						DestroyMenu(hMenu);
						DestroyWindow(hwndTemp);
					}
					else selected = 1;

					if (selected > 0) {
						JABBER_RESOURCE_STATUS *r = item->arResources[selected - 1];
						if (r) {
							_tcsncat(jid, _T("/"), SIZEOF(jid));
							_tcsncat(jid, r->m_tszResourceName, SIZEOF(jid));
						}
						selected = 1;
					}
				}
			}

			if (item == NULL || selected) {
				CJabberAdhocStartupParams* pStartupParams = new CJabberAdhocStartupParams(this, jid, NULL);
				CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FORM), NULL, JabberAdHoc_CommandDlgProc, (LPARAM)(pStartupParams));
			}
		}
		else if (lParam != 0)
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FORM), NULL, JabberAdHoc_CommandDlgProc, lParam);
	}
	return res;
}

void CJabberProto::ContactMenuAdhocCommands(CJabberAdhocStartupParams* param)
{
	if (param)
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FORM), NULL, JabberAdHoc_CommandDlgProc, (LPARAM)param);
}
