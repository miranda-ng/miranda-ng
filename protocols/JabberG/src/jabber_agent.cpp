/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-22 Miranda NG team

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

/////////////////////////////////////////////////////////////////////////////////////////
// Agent registration progress dialog

class CAgentRegProgressDlg : public CJabberDlgBase
{
	CCtrlButton m_ok;

public:
	CAgentRegProgressDlg(CJabberProto *_ppro, HWND _owner) :
		CJabberDlgBase(_ppro, IDD_OPT_REGISTER),
		m_ok(this, IDOK)
	{
		SetParent(_owner);
	}

	bool OnInitDialog() override
	{
		m_proto->m_pDlgReg = this;
		SetWindowText(m_hwnd, TranslateT("Jabber Agent Registration"));
		TranslateDialogDefault(m_hwnd);
		return true;
	}

	void OnDestroy() override
	{
		m_proto->m_pDlgReg = nullptr;
	}

	void Update(int progress, const wchar_t *pwszText)
	{
		if (this == nullptr)
			return;

		SetDlgItemText(m_hwnd, IDC_REG_STATUS, pwszText);
		SendDlgItemMessage(m_hwnd, IDC_PROGRESS_REG, PBM_SETPOS, progress, 0);

		if (progress >= 100)
			m_ok.SetText(TranslateT("OK"));
	}
};

void CJabberProto::OnIqAgentSetRegister(const TiXmlElement *iqNode, CJabberIqInfo *)
{
	// RECVED: result of registration process
	// ACTION: notify of successful agent registration
	debugLogA("<iq/> iqIdSetRegister");

	const char *type, *from;
	if ((type = XmlGetAttr(iqNode, "type")) == nullptr) return;
	if ((from = XmlGetAttr(iqNode, "from")) == nullptr) return;

	if (!mir_strcmp(type, "result")) {
		MCONTACT hContact = HContactFromJID(from);
		if (hContact != 0)
			setByte(hContact, "IsTransport", true);

		m_pDlgReg->Update(100, TranslateT("Registration successful"));
	}
	else if (!mir_strcmp(type, "error")) {
		m_pDlgReg->Update(100, JabberErrorMsg(iqNode).c_str());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Transport registration form

class CAgentRegDlg : public CJabberDlgBase
{
	int m_curPos;
	int m_formHeight, m_frameHeight;
	RECT m_frameRect;
	TiXmlDocument m_doc;
	TiXmlElement *m_agentRegIqNode;
	char *m_jid;

	HWND m_statusBar;

public:
	CAgentRegDlg(CJabberProto *_ppro, char *_jid) :
		CJabberDlgBase(_ppro, IDD_FORM),
		m_jid(_jid),
		m_agentRegIqNode(nullptr)
	{
	}

	bool OnInitDialog() override
	{
		m_proto->m_pDlgAgentReg = this;

		EnableWindow(GetParent(m_hwnd), FALSE);
		SetWindowText(m_hwnd, TranslateT("Jabber Agent Registration"));
		SetDlgItemText(m_hwnd, IDOK, TranslateT("Register"));

		m_statusBar = CreateWindowExW(0, STATUSCLASSNAME, nullptr, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, m_hwnd, nullptr, g_plugin.getInst(), nullptr);
		SendMessage(m_statusBar, WM_SIZE, 0, 0);
		SetWindowTextW(m_statusBar, TranslateT("Please wait..."));

		m_proto->m_ThreadInfo->send(
			XmlNodeIq(m_proto->AddIQ(&CJabberProto::OnIqAgentGetRegister, JABBER_IQ_TYPE_GET, m_jid)) << XQUERY(JABBER_FEAT_REGISTER));

		// Enable WS_EX_CONTROLPARENT on IDC_FRAME (so tab stop goes through all its children)
		LONG_PTR frameExStyle = GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_FRAME), GWL_EXSTYLE);
		frameExStyle |= WS_EX_CONTROLPARENT;
		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_FRAME), GWL_EXSTYLE, frameExStyle);
		return true;
	}

	bool OnApply() override
	{
		if (m_agentRegIqNode == nullptr)
			return true;

		auto *queryNode = XmlFirstChild(m_agentRegIqNode, "query");
		const char *from = XmlGetAttr(m_agentRegIqNode, "from");
		if (from == nullptr || queryNode == nullptr)
			return true;

		HWND hwndFrame = GetDlgItem(m_hwnd, IDC_FRAME);
		int id = 0;

		XmlNodeIq iq(m_proto->AddIQ(&CJabberProto::OnIqAgentSetRegister, JABBER_IQ_TYPE_SET, from));
		TiXmlElement *query = iq << XQUERY(JABBER_FEAT_REGISTER);

		if (auto *xNode = XmlFirstChild(queryNode, "x")) {
			// use new jabber:x:data form
			JabberFormGetData(hwndFrame, query, xNode);
		}
		else {
			// use old registration information form
			for (auto *n : TiXmlEnum(queryNode)) {
				const char *pszName = n->Name();
				if (pszName) {
					if (!mir_strcmp(pszName, "key")) {
						// field that must be passed along with the registration
						if (n->GetText())
							XmlAddChildA(query, pszName, n->GetText());
						else
							XmlAddChild(query, pszName);
					}
					else if (!mir_strcmp(pszName, "registered") || !mir_strcmp(pszName, "instructions")) {
						// do nothing, we will skip these
					}
					else {
						wchar_t str[128];
						GetDlgItemText(hwndFrame, id, str, _countof(str));
						XmlAddChildA(query, pszName, T2Utf(str).get());
						id++;
					}
				}
			}
		}

		m_proto->m_ThreadInfo->send(iq);

		CAgentRegProgressDlg(m_proto, m_hwnd).DoModal();
		return true;
	}

	void OnDestroy() override
	{
		JabberFormDestroyUI(GetDlgItem(m_hwnd, IDC_FRAME));
		m_proto->m_pDlgAgentReg = nullptr;
		EnableWindow(GetParent(m_hwnd), TRUE);
		SetActiveWindow(GetParent(m_hwnd));
	}

	void Success(const TiXmlElement *pNode)
	{
		HWND hwndFrame = GetDlgItem(m_hwnd, IDC_FRAME);
		SetWindowTextW(m_statusBar, L"");

		if (pNode == nullptr)
			return;

		m_agentRegIqNode = pNode->DeepClone(&m_doc)->ToElement();
		auto *queryNode = XmlFirstChild(m_agentRegIqNode, "query");
		if (queryNode == nullptr)
			return;

		m_curPos = 0;
		GetClientRect(GetDlgItem(m_hwnd, IDC_FRAME), &m_frameRect);

		RECT rect;
		GetClientRect(GetDlgItem(m_hwnd, IDC_VSCROLL), &rect);
		m_frameRect.right -= (rect.right - rect.left);
		GetClientRect(GetDlgItem(m_hwnd, IDC_FRAME), &rect);
		m_frameHeight = rect.bottom - rect.top;

		if (auto *xNode = XmlFirstChild(queryNode, "x")) {
			// use new jabber:x:data form
			if (const char *pszText = XmlGetChildText(xNode, "instructions"))
				JabberFormSetInstruction(m_hwnd, pszText);

			JabberFormCreateUI(hwndFrame, xNode, &m_formHeight);
		}
		else {
			// use old registration information form
			TJabberFormLayoutInfo layout_info(hwndFrame, false);
			for (auto *n : TiXmlEnum(queryNode)) {
				const char *pszName = n->Name();
				if (pszName) {
					if (!mir_strcmp(pszName, "instructions")) {
						JabberFormSetInstruction(m_hwnd, n->GetText());
					}
					else if (!mir_strcmp(pszName, "key") || !mir_strcmp(pszName, "registered")) {
						// do nothing
					}
					else if (!mir_strcmp(pszName, "password"))
						layout_info.AppendControl(JFORM_CTYPE_TEXT_PRIVATE, pszName, n->GetText());
					else 	// everything else is a normal text field
						layout_info.AppendControl(JFORM_CTYPE_TEXT_SINGLE, pszName, n->GetText());
				}
			}
			layout_info.OrderControls(&m_formHeight);
		}

		if (m_formHeight > m_frameHeight) {
			HWND hwndScroll = GetDlgItem(m_hwnd, IDC_VSCROLL);
			EnableWindow(hwndScroll, TRUE);
			SetScrollRange(hwndScroll, SB_CTL, 0, m_formHeight - m_frameHeight, FALSE);
			m_curPos = 0;
		}

		EnableWindow(GetDlgItem(m_hwnd, IDOK), TRUE);
	}

	void Fail(const CMStringW &wszErrMsg)
	{
		SetWindowText(m_statusBar, wszErrMsg);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case WM_CTLCOLORSTATIC:
			switch (GetDlgCtrlID((HWND)lParam)) {
			case IDC_WHITERECT: case IDC_INSTRUCTION: case IDC_TITLE:
				return (INT_PTR)GetStockObject(WHITE_BRUSH);
			default:
				return 0;
			}

		case WM_VSCROLL:
			int pos = m_curPos;
			switch (LOWORD(wParam)) {
				case SB_LINEDOWN:   pos += 10;                    break;
				case SB_LINEUP:     pos -= 10;                    break;
				case SB_PAGEDOWN:   pos += (m_frameHeight - 10);  break;
				case SB_PAGEUP:     pos -= (m_frameHeight - 10);  break;
				case SB_THUMBTRACK: pos = HIWORD(wParam);         break;
			}
			if (pos > (m_formHeight - m_frameHeight))
				pos = m_formHeight - m_frameHeight;
			if (pos < 0)
				pos = 0;
			if (m_curPos != pos) {
				ScrollWindow(GetDlgItem(m_hwnd, IDC_FRAME), 0, m_curPos - pos, nullptr, &(m_frameRect));
				SetScrollPos(GetDlgItem(m_hwnd, IDC_VSCROLL), SB_CTL, pos, TRUE);
				m_curPos = pos;
			}
		}

		return CJabberDlgBase::DlgProc(msg, wParam, lParam);
	}
};

void CJabberProto::RegisterAgent(HWND hwndParent, char *jid)
{
	auto *pDlg = new CAgentRegDlg(this, jid);
	pDlg->SetParent(hwndParent);
	pDlg->Show();
}

/////////////////////////////////////////////////////////////////////////////////////////

struct TAgentParam
{
	CAgentRegDlg *pDlg;
	bool bSuccess;
	const TiXmlElement *iqNode;
};

static INT_PTR CALLBACK sttFinish(void *param)
{
	auto *p = (TAgentParam *)param;
	if (p->pDlg) {
		if (p->bSuccess)
			p->pDlg->Success(p->iqNode);
		else
			p->pDlg->Fail(JabberErrorMsg(p->iqNode));
	}
	return 0;
}

void CJabberProto::OnIqAgentGetRegister(const TiXmlElement *iqNode, CJabberIqInfo *)
{
	// RECVED: result of the request for (agent) registration mechanism
	// ACTION: activate (agent) registration input dialog
	debugLogA("<iq/> iqIdGetRegister");

	const TiXmlElement *queryNode;
	const char *type;
	if ((type = XmlGetAttr(iqNode, "type")) == nullptr) return;
	if ((queryNode = XmlFirstChild(iqNode, "query")) == nullptr) return;

	if (!mir_strcmp(type, "result")) {
		TAgentParam param = { m_pDlgAgentReg, true, iqNode };
		CallFunctionSync(sttFinish, &param);
	}
	else if (!mir_strcmp(type, "error")) {
		TAgentParam param = { m_pDlgAgentReg, false, iqNode };
		CallFunctionSync(sttFinish, &param);
	}
}

void CJabberProto::AgentShutdown()
{
	UI_SAFE_CLOSE(m_pDlgReg);
	UI_SAFE_CLOSE(m_pDlgAgentReg);
}
