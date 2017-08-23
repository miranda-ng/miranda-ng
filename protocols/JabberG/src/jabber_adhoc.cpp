/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Artem Shpynov
Copyright (ñ) 2012-17 Miranda NG project

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

#include "stdafx.h"
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
	if (pInfo->GetUserData() != nullptr)
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
	SendMessage(GetWindowFromIq(pInfo), JAHM_COMMANDLISTRESULT, 0, (LPARAM)xmlCopyNode(iqNode));
}

void CJabberProto::OnIqResult_CommandExecution(HXML iqNode, CJabberIqInfo *pInfo)
{
	SendMessage(GetWindowFromIq(pInfo), JAHM_PROCESSRESULT, (WPARAM)xmlCopyNode(iqNode), 0);
}

void CJabberProto::AdHoc_RequestListOfCommands(wchar_t * szResponder, HWND hwndDlg)
{
	m_ThreadInfo->send(XmlNodeIq(AddIQ(&CJabberProto::OnIqResult_ListOfCommands, JABBER_IQ_TYPE_GET, szResponder, 0, -1, hwndDlg))
		<< XQUERY(JABBER_FEAT_DISCO_ITEMS) << XATTR(L"node", JABBER_FEAT_COMMANDS));
}

int CJabberProto::AdHoc_ExecuteCommand(HWND hwndDlg, wchar_t*, JabberAdHocData* dat)
{
	for (int i = 1;; i++) {
		HXML itemNode = XmlGetNthChild(dat->CommandsNode, L"item", i);
		if (!itemNode)
			break;

		if (BST_UNCHECKED == IsDlgButtonChecked(GetDlgItem(hwndDlg, IDC_FRAME), i))
			continue;

		const wchar_t *node = XmlGetAttrValue(itemNode, L"node");
		if (node) {
			const wchar_t *jid2 = XmlGetAttrValue(itemNode, L"jid");
			m_ThreadInfo->send(
				XmlNodeIq(AddIQ(&CJabberProto::OnIqResult_CommandExecution, JABBER_IQ_TYPE_SET, jid2, 0, -1, hwndDlg))
				<< XCHILDNS(L"command", JABBER_FEAT_COMMANDS) << XATTR(L"node", node) << XATTR(L"action", L"execute"));

			EnableDlgItem(hwndDlg, IDC_SUBMIT, FALSE);
			SetDlgItemText(hwndDlg, IDC_SUBMIT, TranslateT("OK"));
		}
	}

	xmlDestroyNode(dat->CommandsNode); dat->CommandsNode = nullptr;
	return TRUE;
}

// Messages handlers
int CJabberProto::AdHoc_OnJAHMCommandListResult(HWND hwndDlg, HXML iqNode, JabberAdHocData* dat)
{
	int nodeIdx = 0;
	const wchar_t * type = XmlGetAttrValue(iqNode, L"type");
	if (!type || !mir_wstrcmp(type, L"error")) {
		// error occurred here
		wchar_t buff[255];
		const wchar_t *code = nullptr;
		const wchar_t *description = nullptr;

		HXML errorNode = XmlGetChild(iqNode, "error");
		if (errorNode) {
			code = XmlGetAttrValue(errorNode, L"code");
			description = XmlGetText(errorNode);
		}
		mir_snwprintf(buff, TranslateT("Error %s %s"), (code) ? code : L"", (description) ? description : L"");
		JabberFormSetInstruction(hwndDlg, buff);
	}
	else if (!mir_wstrcmp(type, L"result")) {
		BOOL validResponse = FALSE;
		EnumChildWindows(GetDlgItem(hwndDlg, IDC_FRAME), sttDeleteChildWindowsProc, 0);
		dat->CurrentHeight = 0;
		dat->curPos = 0;
		SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, 0, FALSE);
		HXML queryNode = XmlGetChild(iqNode, "query");
		if (queryNode) {
			const wchar_t *xmlns = XmlGetAttrValue(queryNode, L"xmlns");
			const wchar_t *node = XmlGetAttrValue(queryNode, L"node");
			if (xmlns && node && !mir_wstrcmp(xmlns, JABBER_FEAT_DISCO_ITEMS) && !mir_wstrcmp(node, JABBER_FEAT_COMMANDS))
				validResponse = TRUE;
		}
		if (queryNode && XmlGetChild(queryNode, 0) && validResponse) {
			dat->CommandsNode = xmlCopyNode(queryNode);

			int ypos = 20;
			for (nodeIdx = 1;; nodeIdx++) {
				HXML itemNode = XmlGetNthChild(queryNode, L"item", nodeIdx);
				if (!itemNode)
					break;

				const wchar_t *name = XmlGetAttrValue(itemNode, L"name");
				if (!name) name = XmlGetAttrValue(itemNode, L"node");
				ypos = AdHoc_AddCommandRadio(GetDlgItem(hwndDlg, IDC_FRAME), TranslateW(name), nodeIdx, ypos, (nodeIdx == 1) ? 1 : 0);
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

	if (workNode == nullptr)
		return TRUE;

	dat->AdHocNode = xmlCopyNode(workNode);

	const wchar_t *type;
	if ((type = XmlGetAttrValue(workNode, L"type")) == nullptr) return TRUE;
	if (!mir_wstrcmp(type, L"result")) {
		// wParam = <iq/> node from responder as a result of command execution
		HXML commandNode, xNode;
		if ((commandNode = XmlGetChild(dat->AdHocNode, L"command")) == nullptr)
			return TRUE;

		const wchar_t *status = XmlGetAttrValue(commandNode, L"status");
		if (!status)
			status = L"completed";

		if ((xNode = XmlGetChild(commandNode, "x"))) {
			// use jabber:x:data form
			HWND hFrame = GetDlgItem(hwndDlg, IDC_FRAME);
			ShowWindow(GetDlgItem(hwndDlg, IDC_FRAME_TEXT), SW_HIDE);
			if (LPCTSTR ptszInstr = XmlGetText(XmlGetChild(xNode, "instructions")))
				JabberFormSetInstruction(hwndDlg, ptszInstr);
			else if (LPCTSTR ptszTitle = XmlGetText(XmlGetChild(xNode, "title")))
				JabberFormSetInstruction(hwndDlg, ptszTitle);
			else
				JabberFormSetInstruction(hwndDlg, TranslateW(status));
			JabberFormCreateUI(hFrame, xNode, &dat->CurrentHeight);
			ShowDlgItem(hwndDlg, IDC_FRAME, SW_SHOW);
		}
		else {
			//NO X FORM
			int toHide[] = { IDC_FRAME_TEXT, IDC_FRAME, IDC_VSCROLL, 0 };
			sttShowControls(hwndDlg, FALSE, toHide);

			LPCTSTR noteText = XmlGetText(XmlGetChild(commandNode, "note"));
			JabberFormSetInstruction(hwndDlg, noteText ? noteText : TranslateW(status));
		}

		// check actions
		HXML actionsNode = XmlGetChild(commandNode, "actions");
		if (actionsNode != nullptr) {
			ShowDlgItem(hwndDlg, IDC_PREV, (XmlGetChild(actionsNode, "prev") != nullptr) ? SW_SHOW : SW_HIDE);
			ShowDlgItem(hwndDlg, IDC_NEXT, (XmlGetChild(actionsNode, "next") != nullptr) ? SW_SHOW : SW_HIDE);
			ShowDlgItem(hwndDlg, IDC_COMPLETE, (XmlGetChild(actionsNode, "complete") != nullptr) ? SW_SHOW : SW_HIDE);
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

		if (!status || mir_wstrcmp(status, L"executing")) {
			ShowDlgItem(hwndDlg, IDC_SUBMIT, SW_HIDE);
			SetDlgItemText(hwndDlg, IDCANCEL, TranslateT("Done"));
		}
	}
	else if (!mir_wstrcmp(type, L"error")) {
		// error occurred here
		int toHide[] = { IDC_FRAME, IDC_FRAME_TEXT, IDC_VSCROLL, IDC_PREV, IDC_NEXT, IDC_COMPLETE, IDC_SUBMIT, 0};
		sttShowControls(hwndDlg, FALSE, toHide);

		const wchar_t *code=nullptr;
		const wchar_t *description=nullptr;
		wchar_t buff[255];
		HXML errorNode = XmlGetChild(workNode , "error");
		if (errorNode) {
			code = XmlGetAttrValue(errorNode, L"code");
			description = XmlGetText(errorNode);
		}
		mir_snwprintf(buff, TranslateT("Error %s %s"), code ? code : L"", description ? description : L"");
		JabberFormSetInstruction(hwndDlg,buff);
	}
	JabberAdHoc_RefreshFrameScroll(hwndDlg, dat);
	return TRUE;
}

int CJabberProto::AdHoc_SubmitCommandForm(HWND hwndDlg, JabberAdHocData* dat, wchar_t* action)
{
	HXML commandNode = XmlGetChild(dat->AdHocNode, "command");
	HXML xNode = XmlGetChild(commandNode, "x");
	HXML dataNode = JabberFormGetData(GetDlgItem(hwndDlg, IDC_FRAME), xNode);

	LPCTSTR jid2 = XmlGetAttrValue(dat->AdHocNode, L"from");
	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResult_CommandExecution, JABBER_IQ_TYPE_SET, jid2, 0, -1, hwndDlg));
	HXML command = iq << XCHILDNS(L"command", JABBER_FEAT_COMMANDS);

	const wchar_t *sessionId = XmlGetAttrValue(commandNode, L"sessionid");
	if (sessionId)
		command << XATTR(L"sessionid", sessionId);

	const wchar_t *node = XmlGetAttrValue(commandNode, L"node");
	if (node)
		command << XATTR(L"node", node);

	if (action)
		command << XATTR(L"action", action);

	XmlAddChild(command, dataNode);
	m_ThreadInfo->send(iq);

	xmlDestroyNode(dataNode);

	JabberFormSetInstruction(hwndDlg, TranslateT("In progress. Please Wait..."));

	static const int toDisable[] = { IDC_SUBMIT, IDC_PREV, IDC_NEXT, IDC_COMPLETE, 0 };
	sttEnableControls(hwndDlg, FALSE, toDisable);
	return TRUE;
}

int CJabberProto::AdHoc_AddCommandRadio(HWND hFrame, wchar_t * labelStr, int id, int ypos, int value)
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

	HWND hCtrl = CreateWindowEx(0, L"button", labelStr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON, ctrlOffset, ypos, ctrlWidth, labelHeight, hFrame, (HMENU)id, hInst, nullptr);
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
			dat->ResponderJID = mir_wstrdup(pStartupParams->m_szJid);
			dat->proto = pStartupParams->m_pProto;

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			Window_SetIcon_IcoLib(hwndDlg, g_GetIconHandle(IDI_COMMAND));
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

				wchar_t Caption[512];
				mir_snwprintf(Caption, TranslateT("Jabber Ad-Hoc commands at %s"), dat->ResponderJID);
				SetWindowText(hwndDlg, Caption);
			}
			else {
				dat->proto->m_ThreadInfo->send(
					XmlNodeIq(dat->proto->AddIQ(&CJabberProto::OnIqResult_CommandExecution, JABBER_IQ_TYPE_SET, pStartupParams->m_szJid, 0, -1, hwndDlg))
					<< XCHILDNS(L"command", JABBER_FEAT_COMMANDS)
					<< XATTR(L"node", pStartupParams->m_szNode) << XATTR(L"action", L"execute"));

				EnableDlgItem(hwndDlg, IDC_SUBMIT, FALSE);
				SetDlgItemText(hwndDlg, IDC_SUBMIT, TranslateT("OK"));

				wchar_t Caption[512];
				mir_snwprintf(Caption, TranslateT("Sending Ad-Hoc command to %s"), dat->ResponderJID);
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
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_PREV:
			return dat->proto->AdHoc_SubmitCommandForm(hwndDlg, dat, L"prev");
		case IDC_NEXT:
			return dat->proto->AdHoc_SubmitCommandForm(hwndDlg, dat, L"next");
		case IDC_COMPLETE:
			return dat->proto->AdHoc_SubmitCommandForm(hwndDlg, dat, L"complete");
		case IDC_SUBMIT:
			if (!dat->AdHocNode && dat->CommandsNode && LOWORD(wParam) == IDC_SUBMIT)
				return dat->proto->AdHoc_ExecuteCommand(hwndDlg, dat->ResponderJID, dat);
			else
				return dat->proto->AdHoc_SubmitCommandForm(hwndDlg, dat, nullptr);
		case IDCLOSE:
		case IDCANCEL:
			xmlDestroyNode(dat->AdHocNode); dat->AdHocNode = nullptr;
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
			short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (zDelta) {
				int nScrollLines = 0;
				SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, (void*)&nScrollLines, 0);
				for (int i = 0; i < (nScrollLines + 1) / 2; i++)
					SendMessage(hwndDlg, WM_VSCROLL, (zDelta < 0) ? SB_LINEDOWN : SB_LINEUP, 0);
			}
		}
		return TRUE;

	case WM_VSCROLL:
		if (dat != nullptr) {
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
				ScrollWindow(GetDlgItem(hwndDlg, IDC_FRAME), 0, dat->curPos - pos, nullptr, &(dat->frameRect));
				SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, pos, TRUE);
				RECT Invalid = dat->frameRect;
				if (dat->curPos - pos >0)
					Invalid.bottom = Invalid.top + (dat->curPos - pos);
				else
					Invalid.top = Invalid.bottom + (dat->curPos - pos);

				RedrawWindow(GetDlgItem(hwndDlg, IDC_FRAME), nullptr, nullptr, RDW_UPDATENOW | RDW_ALLCHILDREN);
				dat->curPos = pos;
			}
		}
		break;

	case WM_DESTROY:
		JabberFormDestroyUI(GetDlgItem(hwndDlg, IDC_FRAME));
		Window_FreeIcon_IcoLib(hwndDlg);

		dat->proto->m_hwndCommandWindow = nullptr;
		mir_free(dat->ResponderJID);
		xmlDestroyNode(dat->CommandsNode);
		xmlDestroyNode(dat->AdHocNode);
		mir_free(dat);
		dat = nullptr;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}
	return FALSE;
}

int __cdecl CJabberProto::ContactMenuRunCommands(WPARAM hContact, LPARAM lParam)
{
	int res = -1;

	if ((hContact != 0 || lParam != 0) && m_bJabberOnline) {
		ptrW szJid(getWStringA(hContact, "jid"));
		if (hContact && szJid != nullptr) {
			JABBER_LIST_ITEM *item = nullptr;
			int selected = 0;
			wchar_t jid[JABBER_MAX_JID_LEN];
			wcsncpy_s(jid, szJid, _TRUNCATE);
			{
				mir_cslock lck(m_csLists);
				item = ListGetItemPtr(LIST_ROSTER, jid);
				if (item) {
					if (item->arResources.getCount() > 1) {
						HMENU hMenu = CreatePopupMenu();
						for (int i = 0; i < item->arResources.getCount(); i++)
							AppendMenu(hMenu, MF_STRING, i + 1, item->arResources[i]->m_tszResourceName);
						HWND hwndTemp = CreateWindowEx(WS_EX_TOOLWINDOW, L"button", L"PopupMenuHost", 0, 0, 0, 10, 10, nullptr, nullptr, hInst, nullptr);
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
							mir_wstrncat(jid, L"/", _countof(jid) - mir_wstrlen(jid));
							mir_wstrncat(jid, r->m_tszResourceName, _countof(jid) - mir_wstrlen(jid));
						}
						selected = 1;
					}
				}
			}

			if (item == nullptr || selected) {
				CJabberAdhocStartupParams* pStartupParams = new CJabberAdhocStartupParams(this, jid, nullptr);
				CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FORM), nullptr, JabberAdHoc_CommandDlgProc, (LPARAM)pStartupParams);
			}
		}
		else if (lParam != 0)
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FORM), nullptr, JabberAdHoc_CommandDlgProc, lParam);
	}
	return res;
}

void CJabberProto::ContactMenuAdhocCommands(CJabberAdhocStartupParams* param)
{
	if (param)
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FORM), nullptr, JabberAdHoc_CommandDlgProc, (LPARAM)param);
}
