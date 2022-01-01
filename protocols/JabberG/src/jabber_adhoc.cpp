/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Artem Shpynov
Copyright (C) 2012-22 Miranda NG team

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

void CJabberProto::OnIqResult_ListOfCommands(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	SendMessage(GetWindowFromIq(pInfo), JAHM_COMMANDLISTRESULT, 0, (LPARAM)iqNode);
}

void CJabberProto::OnIqResult_CommandExecution(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	SendMessage(GetWindowFromIq(pInfo), JAHM_PROCESSRESULT, 0, (WPARAM)iqNode);
}

void CJabberProto::AdHoc_RequestListOfCommands(char *szResponder, HWND hwndDlg)
{
	m_ThreadInfo->send(XmlNodeIq(AddIQ(&CJabberProto::OnIqResult_ListOfCommands, JABBER_IQ_TYPE_GET, szResponder, hwndDlg))
		<< XQUERY(JABBER_FEAT_DISCO_ITEMS) << XATTR("node", JABBER_FEAT_COMMANDS));
}

void CJabberProto::AdHoc_ExecuteCommand(HWND hwndDlg, char*, JabberAdHocData *dat)
{
	int i = 1;
	for (auto *itemNode : TiXmlFilter(dat->CommandsNode, "item")) {
		if (BST_UNCHECKED == IsDlgButtonChecked(GetDlgItem(hwndDlg, IDC_FRAME), i++))
			continue;

		const char *node = XmlGetAttr(itemNode, "node");
		if (node) {
			const char *jid2 = XmlGetAttr(itemNode, "jid");
			m_ThreadInfo->send(
				XmlNodeIq(AddIQ(&CJabberProto::OnIqResult_CommandExecution, JABBER_IQ_TYPE_SET, jid2, hwndDlg))
				<< XCHILDNS("command", JABBER_FEAT_COMMANDS) << XATTR("node", node) << XATTR("action", "execute"));

			EnableDlgItem(hwndDlg, IDOK, FALSE);
			SetDlgItemText(hwndDlg, IDOK, TranslateT("OK"));
		}
	}
}

// Messages handlers
void CJabberProto::AdHoc_OnJAHMCommandListResult(HWND hwndDlg, TiXmlElement *iqNode, JabberAdHocData *dat)
{
	const char *type = XmlGetAttr(iqNode, "type");
	if (!type || !mir_strcmp(type, "error")) {
		// error occurred here
		const char *code = "";
		const char *description = "";

		auto *errorNode = XmlFirstChild(iqNode, "error");
		if (errorNode) {
			code = XmlGetAttr(errorNode, "code");
			description = errorNode->GetText();
		}

		JabberFormSetInstruction(hwndDlg, CMStringA(FORMAT, TranslateU("Error %s %s"), code, description));
	}
	else if (!mir_strcmp(type, "result")) {
		EnumChildWindows(GetDlgItem(hwndDlg, IDC_FRAME), sttDeleteChildWindowsProc, 0);
		dat->CurrentHeight = 0;
		dat->curPos = 0;
		SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, 0, FALSE);

		bool validResponse = false;
		auto *queryNode = XmlGetChildByTag(iqNode, "query", "xmlns", JABBER_FEAT_DISCO_ITEMS);
		if (queryNode) 
			validResponse = queryNode->Attribute("node", JABBER_FEAT_COMMANDS) != 0;

		int nodeIdx = 1;
		if (queryNode && XmlFirstChild(queryNode, 0) && validResponse) {
			dat->CommandsNode = queryNode->DeepClone(&dat->doc)->ToElement();

			int ypos = 20;
			for (auto *itemNode : TiXmlFilter(queryNode, "item")) {
				const char *name = XmlGetAttr(itemNode, "name");
				if (!name)
					name = XmlGetAttr(itemNode, "node");
				ypos = AdHoc_AddCommandRadio(GetDlgItem(hwndDlg, IDC_FRAME), name, nodeIdx, ypos, (nodeIdx == 1) ? 1 : 0);
				dat->CurrentHeight = ypos;
				nodeIdx++;
			}
		}

		if (nodeIdx > 1) {
			JabberFormSetInstruction(hwndDlg, TranslateU("Select Command"));
			ShowDlgItem(hwndDlg, IDC_FRAME, SW_SHOW);
			ShowDlgItem(hwndDlg, IDC_VSCROLL, SW_SHOW);
			EnableDlgItem(hwndDlg, IDOK, TRUE);
		}
		else JabberFormSetInstruction(hwndDlg, TranslateU("Not supported"));
	}

	JabberAdHoc_RefreshFrameScroll(hwndDlg, dat);
}

void CJabberProto::AdHoc_OnJAHMProcessResult(HWND hwndDlg, TiXmlElement *workNode, JabberAdHocData *dat)
{
	EnumChildWindows(GetDlgItem(hwndDlg, IDC_FRAME), sttDeleteChildWindowsProc, 0);
	dat->CurrentHeight = 0;
	dat->curPos = 0;
	SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, 0, FALSE);

	if (workNode == nullptr)
		return;

	dat->AdHocNode = workNode->DeepClone(&dat->doc)->ToElement();

	const char *type;
	if ((type = XmlGetAttr(workNode, "type")) == nullptr)
		return;
	
	if (!mir_strcmp(type, "result")) {
		// wParam = <iq/> node from responder as a result of command execution
		const TiXmlElement *commandNode, *xNode;
		if ((commandNode = XmlFirstChild(dat->AdHocNode, "command")) == nullptr)
			return;

		const char *status = XmlGetAttr(commandNode, "status");
		if (!status)
			status = "completed";

		if ((xNode = XmlFirstChild(commandNode, "x"))) {
			// use jabber:x:data form
			HWND hFrame = GetDlgItem(hwndDlg, IDC_FRAME);
			if (auto *pszText = XmlGetChildText(xNode, "instructions"))
				JabberFormSetInstruction(hwndDlg, pszText);
			else if (pszText = XmlGetChildText(xNode, "title"))
				JabberFormSetInstruction(hwndDlg, pszText);
			else
				JabberFormSetInstruction(hwndDlg, TranslateU(status));
			JabberFormCreateUI(hFrame, xNode, &dat->CurrentHeight);
			ShowDlgItem(hwndDlg, IDC_FRAME, SW_SHOW);
		}
		else {
			// NO X FORM
			int toHide[] = { IDC_FRAME, IDC_VSCROLL, 0 };
			sttShowControls(hwndDlg, FALSE, toHide);

			auto *pszText = XmlGetChildText(commandNode, "note");
			JabberFormSetInstruction(hwndDlg, pszText ? pszText : TranslateU(status));
		}

		// check actions
		auto *actionsNode = XmlFirstChild(commandNode, "actions");
		if (actionsNode != nullptr) {
			ShowDlgItem(hwndDlg, IDC_PREV, (XmlFirstChild(actionsNode, "prev") != nullptr) ? SW_SHOW : SW_HIDE);
			ShowDlgItem(hwndDlg, IDC_NEXT, (XmlFirstChild(actionsNode, "next") != nullptr) ? SW_SHOW : SW_HIDE);
			ShowDlgItem(hwndDlg, IDC_COMPLETE, (XmlFirstChild(actionsNode, "complete") != nullptr) ? SW_SHOW : SW_HIDE);
			ShowDlgItem(hwndDlg, IDOK, SW_HIDE);

			int toEnable[] = { IDC_PREV, IDC_NEXT, IDC_COMPLETE, 0 };
			sttEnableControls(hwndDlg, TRUE, toEnable);
		}
		else {
			int toHide[] = { IDC_PREV, IDC_NEXT, IDC_COMPLETE, 0 };
			sttShowControls(hwndDlg, FALSE, toHide);

			ShowDlgItem(hwndDlg, IDOK, SW_SHOW);
			EnableDlgItem(hwndDlg, IDOK, TRUE);
		}

		if (!status || mir_strcmp(status, "executing")) {
			ShowDlgItem(hwndDlg, IDOK, SW_HIDE);
			SetDlgItemText(hwndDlg, IDCANCEL, TranslateT("Done"));
		}
	}
	else if (!mir_strcmp(type, "error")) {
		// error occurred here
		int toHide[] = { IDC_FRAME, IDC_VSCROLL, IDC_PREV, IDC_NEXT, IDC_COMPLETE, IDOK, 0 };
		sttShowControls(hwndDlg, FALSE, toHide);

		const char *code = "";
		const char *description = "";
		auto *errorNode = XmlFirstChild(workNode, "error");
		if (errorNode) {
			code = XmlGetAttr(errorNode, "code");
			description = errorNode->GetText();
		}
		
		JabberFormSetInstruction(hwndDlg, CMStringA(FORMAT, TranslateU("Error %s %s"), code, description));
	}
	JabberAdHoc_RefreshFrameScroll(hwndDlg, dat);
}

void CJabberProto::AdHoc_SubmitCommandForm(HWND hwndDlg, JabberAdHocData *dat, char* action)
{
	auto *commandNode = XmlFirstChild(dat->AdHocNode, "command");
	auto *xNode = XmlFirstChild(commandNode, "x");

	const char *jid2 = XmlGetAttr(dat->AdHocNode, "from");
	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResult_CommandExecution, JABBER_IQ_TYPE_SET, jid2, hwndDlg));
	TiXmlElement *command = iq << XCHILDNS("command", JABBER_FEAT_COMMANDS);

	const char *sessionId = XmlGetAttr(commandNode, "sessionid");
	if (sessionId)
		command << XATTR("sessionid", sessionId);

	const char *node = XmlGetAttr(commandNode, "node");
	if (node)
		command << XATTR("node", node);

	if (action)
		command << XATTR("action", action);

	JabberFormGetData(GetDlgItem(hwndDlg, IDC_FRAME), command, xNode);
	m_ThreadInfo->send(iq);

	JabberFormSetInstruction(hwndDlg, TranslateU("In progress. Please Wait..."));

	static const int toDisable[] = { IDOK, IDC_PREV, IDC_NEXT, IDC_COMPLETE, 0 };
	sttEnableControls(hwndDlg, FALSE, toDisable);
}

int CJabberProto::AdHoc_AddCommandRadio(HWND hFrame, const char *labelStr, int id, int ypos, int value)
{
	Utf2T wszLabel(labelStr);

	RECT strRect = { 0 };
	int verticalStep = 4;
	int ctrlMinHeight = 18;

	RECT rcFrame;
	GetClientRect(hFrame, &rcFrame);

	int ctrlOffset = 20;
	int ctrlWidth = rcFrame.right - ctrlOffset;

	HDC hdc = GetDC(hFrame);
	int labelHeight = max(ctrlMinHeight, DrawTextW(hdc, wszLabel, -1, &strRect, DT_CALCRECT));
	ctrlWidth = min(ctrlWidth, strRect.right - strRect.left + 20);
	ReleaseDC(hFrame, hdc);

	HWND hCtrl = CreateWindowExW(0, L"button", wszLabel, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON, ctrlOffset, ypos, ctrlWidth, labelHeight, hFrame, (HMENU)id, g_plugin.getInst(), nullptr);
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)SendMessage(GetParent(hFrame), WM_GETFONT, 0, 0), 0);
	SendMessage(hCtrl, BM_SETCHECK, value, 0);
	return (ypos + labelHeight + verticalStep);
}

static INT_PTR CALLBACK JabberAdHoc_CommandDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	JabberAdHocData *dat = (JabberAdHocData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		dat = new JabberAdHocData();
		{
			CJabberAdhocStartupParams *pStartupParams = (CJabberAdhocStartupParams *)lParam;
			dat->ResponderJID = mir_strdup(pStartupParams->m_szJid);
			dat->proto = pStartupParams->m_pProto;

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			Window_SetIcon_IcoLib(hwndDlg, g_plugin.getIconHandle(IDI_GROUP));
			dat->proto->m_hwndCommandWindow = hwndDlg;
			TranslateDialogDefault(hwndDlg);

			// hide frame first
			LONG frameExStyle = GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FRAME), GWL_EXSTYLE);
			frameExStyle |= WS_EX_CONTROLPARENT;
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FRAME), GWL_EXSTYLE, frameExStyle);

			int toHide[] = { IDC_FRAME, IDC_VSCROLL, IDC_PREV, IDC_NEXT, IDC_COMPLETE, 0 };
			sttShowControls(hwndDlg, FALSE, toHide);

			int toShow[] = { IDC_INSTRUCTION, IDOK, IDCANCEL, 0 };
			sttShowControls(hwndDlg, TRUE, toShow);

			EnableDlgItem(hwndDlg, IDC_VSCROLL, TRUE);

			SetWindowPos(GetDlgItem(hwndDlg, IDC_VSCROLL), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

			SetDlgItemText(hwndDlg, IDOK, TranslateT("Execute"));
			JabberFormSetInstruction(hwndDlg, TranslateU("Requesting command list. Please wait..."));

			char Caption[512];
			if (!pStartupParams->m_szNode) {
				dat->proto->AdHoc_RequestListOfCommands(pStartupParams->m_szJid, hwndDlg);

				mir_snprintf(Caption, TranslateU("Jabber Ad-Hoc commands at %s"), dat->ResponderJID);
			}
			else {
				dat->proto->m_ThreadInfo->send(
					XmlNodeIq(dat->proto->AddIQ(&CJabberProto::OnIqResult_CommandExecution, JABBER_IQ_TYPE_SET, pStartupParams->m_szJid, hwndDlg))
					<< XCHILDNS("command", JABBER_FEAT_COMMANDS)
					<< XATTR("node", pStartupParams->m_szNode) << XATTR("action", "execute"));

				EnableDlgItem(hwndDlg, IDOK, FALSE);
				SetDlgItemText(hwndDlg, IDOK, TranslateT("OK"));

				mir_snprintf(Caption, TranslateU("Sending Ad-Hoc command to %s"), dat->ResponderJID);
			}
			SetWindowTextUtf(hwndDlg, Caption);

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
			dat->proto->AdHoc_SubmitCommandForm(hwndDlg, dat, "prev");
			return TRUE;
		case IDC_NEXT:
			dat->proto->AdHoc_SubmitCommandForm(hwndDlg, dat, "next");
			return TRUE;
		case IDC_COMPLETE:
			dat->proto->AdHoc_SubmitCommandForm(hwndDlg, dat, "complete");
			return TRUE;
		case IDOK:
			if (!dat->AdHocNode && dat->CommandsNode && LOWORD(wParam) == IDOK)
				dat->proto->AdHoc_ExecuteCommand(hwndDlg, dat->ResponderJID, dat);
			else
				dat->proto->AdHoc_SubmitCommandForm(hwndDlg, dat, nullptr);
			return TRUE;
		case IDCLOSE:
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;
		}
		break;

	case JAHM_COMMANDLISTRESULT:
		dat->proto->AdHoc_OnJAHMCommandListResult(hwndDlg, (TiXmlElement*)lParam, dat);
		return TRUE;

	case JAHM_PROCESSRESULT:
		dat->proto->AdHoc_OnJAHMProcessResult(hwndDlg, (TiXmlElement*)lParam, dat);
		return TRUE;

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
		delete dat;
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
		ptrA szJid(getUStringA(hContact, "jid"));
		if (hContact && szJid != nullptr) {
			JABBER_LIST_ITEM *item = nullptr;
			int selected = 0;
			CMStringA jid(szJid);
			{
				mir_cslock lck(m_csLists);
				item = ListGetItemPtr(LIST_ROSTER, jid);
				if (item) {
					if (item->arResources.getCount() > 1) {
						HMENU hMenu = CreatePopupMenu();
						for (int i = 0; i < item->arResources.getCount(); i++)
							AppendMenu(hMenu, MF_STRING, i + 1, Utf2T(item->arResources[i]->m_szResourceName));
						HWND hwndTemp = CreateWindowEx(WS_EX_TOOLWINDOW, L"button", L"PopupMenuHost", 0, 0, 0, 10, 10, nullptr, nullptr, g_plugin.getInst(), nullptr);
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
							jid.AppendChar('/');
							jid.Append(r->m_szResourceName);
						}
						selected = 1;
					}
				}
			}

			if (item == nullptr || selected)
				ContactMenuAdhocCommands(new CJabberAdhocStartupParams(this, jid, nullptr));
		}
		else if (lParam != 0)
			ContactMenuAdhocCommands((CJabberAdhocStartupParams*)lParam);
	}
	return res;
}

void CJabberProto::ContactMenuAdhocCommands(CJabberAdhocStartupParams* param)
{
	if (param)
		CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FORM), nullptr, JabberAdHoc_CommandDlgProc, (LPARAM)param);
}
