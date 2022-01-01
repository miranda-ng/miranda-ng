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
#include "jabber_caps.h"

struct TJabberFormControlInfo : public MZeroedObject
{
	TJabberFormControlType type;
	int  bRequired;
	SIZE szBlock;
	POINT ptLabel, ptCtrl;
	HWND hLabel, hCtrl;
};

typedef LIST<TJabberFormControlInfo> TJabberFormControlList;

static LRESULT CALLBACK JabberFormItemWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYDOWN:
		if (wParam == VK_TAB) {
			SetFocus(GetNextDlgTabItem(GetParent(GetParent(hwnd)), hwnd, GetKeyState(VK_SHIFT) < 0 ? TRUE : FALSE));
			return TRUE;
		};
		break;
	}
	return mir_callNextSubclass(hwnd, JabberFormItemWndProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// TJabberFormLayoutInfo class

TJabberFormLayoutInfo::TJabberFormLayoutInfo(HWND hwndCtrl, bool bCompact) :
	m_hwnd(hwndCtrl),
	m_bCompact(bCompact)
{
	RECT frameRect;
	GetClientRect(hwndCtrl, &frameRect);

	m_ctrlHeight = 20;
	m_id = 0;
	m_width = frameRect.right - frameRect.left - 20;
	if (!bCompact)
		m_width -= 10;
	m_ySpacing = bCompact ? 1 : 5;
	m_maxLabelWidth = m_width * 2 / 5;
	m_offset = 10;
	m_yPos = bCompact ? 0 : 14;
}

TJabberFormControlInfo* TJabberFormLayoutInfo::AppendControl(TJabberFormControlType type, const char *labelStr, const char *valueStr)
{
	TJabberFormControlList *controls = (TJabberFormControlList *)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);
	if (!controls) {
		controls = new TJabberFormControlList(5);
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)controls);
	}

	TJabberFormControlInfo *item = (TJabberFormControlInfo *)mir_alloc(sizeof(TJabberFormControlInfo));
	item->type = type;
	item->hLabel = item->hCtrl = nullptr;
	Utf2T wszLabel(labelStr), wszValue(valueStr);

	switch (type) {
	case JFORM_CTYPE_TEXT_PRIVATE:
		item->hLabel = CreateLabel(wszLabel);
		item->hCtrl = CreateWindowExW(WS_EX_CLIENTEDGE, L"edit", wszValue,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | ES_PASSWORD,
			0, 0, 0, 0,
			m_hwnd, (HMENU)m_id, g_plugin.getInst(), nullptr);
		++m_id;
		break;

	case JFORM_CTYPE_TEXT_MULTI:
		item->hLabel = CreateLabel(wszLabel);
		item->hCtrl = CreateWindowExW(WS_EX_CLIENTEDGE, L"edit", wszValue,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
			0, 0, 0, 0,
			m_hwnd, (HMENU)m_id, g_plugin.getInst(), nullptr);
		++m_id;
		break;

	case JFORM_CTYPE_BOOLEAN:
		item->hCtrl = CreateWindowExW(0, L"button", wszLabel,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX | BS_MULTILINE,
			0, 0, 0, 0,
			m_hwnd, (HMENU)m_id, g_plugin.getInst(), nullptr);
		if (valueStr && !mir_wstrcmp(wszValue, L"1"))
			SendMessage(item->hCtrl, BM_SETCHECK, 1, 0);
		++m_id;
		break;

	case JFORM_CTYPE_LIST_SINGLE:
		item->hLabel = CreateLabel(wszLabel);
		item->hCtrl = CreateWindowExW(WS_EX_CLIENTEDGE, L"combobox", nullptr,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
			0, 0, 0, 0,
			m_hwnd, (HMENU)m_id, g_plugin.getInst(), nullptr);
		++m_id;
		break;

	case JFORM_CTYPE_LIST_MULTI:
		item->hLabel = CreateLabel(wszLabel);
		item->hCtrl = CreateWindowExW(WS_EX_CLIENTEDGE, L"listbox",
			nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | LBS_MULTIPLESEL,
			0, 0, 0, 0,
			m_hwnd, (HMENU)m_id, g_plugin.getInst(), nullptr);
		++m_id;
		break;

	case JFORM_CTYPE_FIXED:
		item->hCtrl = CreateWindowExW(0, L"edit", wszValue,
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL,
			0, 0, 0, 0,
			m_hwnd, (HMENU)-1, g_plugin.getInst(), nullptr);
		break;

	case JFORM_CTYPE_HIDDEN:
		break;

	case JFORM_CTYPE_TEXT_SINGLE:
		item->hLabel = labelStr ? (CreateLabel(wszLabel)) : nullptr;
		item->hCtrl = CreateWindowExW(WS_EX_CLIENTEDGE, L"edit", wszValue,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,
			0, 0, 0, 0,
			m_hwnd, (HMENU)m_id, g_plugin.getInst(), nullptr);
		++m_id;
		break;
	}

	HFONT hFont = (HFONT)SendMessage(GetParent(m_hwnd), WM_GETFONT, 0, 0);
	if (item->hLabel)
		SendMessage(item->hLabel, WM_SETFONT, (WPARAM)hFont, 0);
	if (item->hCtrl) {
		SendMessage(item->hCtrl, WM_SETFONT, (WPARAM)hFont, 0);

		if (GetWindowStyle(item->hCtrl) & WS_TABSTOP)
			mir_subclassWindow(item->hCtrl, JabberFormItemWndProc);
	}

	PositionControl(item, labelStr, valueStr);

	controls->insert(item);
	return item;
}

HWND TJabberFormLayoutInfo::CreateLabel(const wchar_t *pwszLabel)
{
	return CreateWindowExW(0, L"static", pwszLabel, WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, 0, 0, 0, 0, m_hwnd, (HMENU)-1, g_plugin.getInst(), nullptr);
}

void TJabberFormLayoutInfo::OrderControls(int *formHeight)
{
	TJabberFormControlList *controls = (TJabberFormControlList *)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);
	if (!controls)
		return;

	for (auto &it : *controls) {
		if (it->type == JFORM_CTYPE_HIDDEN)
			continue;

		if (it->hLabel)
			SetWindowPos(it->hLabel, nullptr,
				m_offset + it->ptLabel.x, m_yPos + it->ptLabel.y, 0, 0,
				SWP_NOZORDER | SWP_NOSIZE);

		if (it->hCtrl)
			SetWindowPos(it->hCtrl, nullptr,
				m_offset + it->ptCtrl.x, m_yPos + it->ptCtrl.y, 0, 0,
				SWP_NOZORDER | SWP_NOSIZE);

		m_yPos += it->szBlock.cy;
		m_yPos += m_ySpacing;
	}

	*formHeight = m_yPos + (m_bCompact ? 0 : 9);
}

void TJabberFormLayoutInfo::PositionControl(TJabberFormControlInfo *item, const char *labelStr, const char *valueStr)
{
	Utf2T wszLabel(labelStr), wszValue(valueStr);

	RECT rcLabel = { 0 }, rcCtrl = { 0 };
	if (item->hLabel) {
		SetRect(&rcLabel, 0, 0, m_width, 0);
		HDC hdc = GetDC(item->hLabel);
		HFONT hfntSave = (HFONT)SelectObject(hdc, (HFONT)SendMessage(item->hLabel, WM_GETFONT, 0, 0));
		DrawTextW(hdc, wszLabel, -1, &rcLabel, DT_CALCRECT | DT_WORDBREAK);
		SelectObject(hdc, hfntSave);
		ReleaseDC(item->hLabel, hdc);
	}

	int indent = m_bCompact ? 10 : 20;

	if ((m_bCompact && (item->type != JFORM_CTYPE_BOOLEAN) && (item->type != JFORM_CTYPE_FIXED)) ||
		(rcLabel.right >= m_maxLabelWidth) ||
		(rcLabel.bottom > m_ctrlHeight) ||
		(item->type == JFORM_CTYPE_LIST_MULTI) ||
		(item->type == JFORM_CTYPE_TEXT_MULTI)) {
		int height = m_ctrlHeight;
		if ((item->type == JFORM_CTYPE_LIST_MULTI) || (item->type == JFORM_CTYPE_TEXT_MULTI)) height *= 3;
		SetRect(&rcCtrl, indent, rcLabel.bottom, m_width, rcLabel.bottom + height);
	}
	else if (item->type == JFORM_CTYPE_BOOLEAN) {
		SetRect(&rcCtrl, 0, 0, m_width - 20, 0);
		HDC hdc = GetDC(item->hCtrl);
		HFONT hfntSave = (HFONT)SelectObject(hdc, (HFONT)SendMessage(item->hCtrl, WM_GETFONT, 0, 0));
		DrawText(hdc, wszLabel, -1, &rcCtrl, DT_CALCRECT | DT_RIGHT | DT_WORDBREAK);
		SelectObject(hdc, hfntSave);
		ReleaseDC(item->hCtrl, hdc);
		rcCtrl.right += 20;
	}
	else if (item->type == JFORM_CTYPE_FIXED) {
		SetRect(&rcCtrl, 0, 0, m_width, 0);
		HDC hdc = GetDC(item->hCtrl);
		HFONT hfntSave = (HFONT)SelectObject(hdc, (HFONT)SendMessage(item->hCtrl, WM_GETFONT, 0, 0));
		DrawText(hdc, wszValue, -1, &rcCtrl, DT_CALCRECT | DT_EDITCONTROL);
		rcCtrl.right += 20;
		SelectObject(hdc, hfntSave);
		ReleaseDC(item->hCtrl, hdc);
	}
	else {
		SetRect(&rcCtrl, rcLabel.right + 5, 0, m_width, m_ctrlHeight);
		rcLabel.bottom = rcCtrl.bottom;
	}

	if (item->hLabel)
		SetWindowPos(item->hLabel, nullptr, 0, 0, rcLabel.right - rcLabel.left, rcLabel.bottom - rcLabel.top, SWP_NOZORDER | SWP_NOMOVE);
	if (item->hCtrl)
		SetWindowPos(item->hCtrl, nullptr, 0, 0, rcCtrl.right - rcCtrl.left, rcCtrl.bottom - rcCtrl.top, SWP_NOZORDER | SWP_NOMOVE);

	item->ptLabel.x = rcLabel.left;
	item->ptLabel.y = rcLabel.top;
	item->ptCtrl.x = rcCtrl.left;
	item->ptCtrl.y = rcCtrl.top;
	item->szBlock.cx = m_width;
	item->szBlock.cy = max(rcLabel.bottom, rcCtrl.bottom);
}

/////////////////////////////////////////////////////////////////////////////////////////

void JabberFormSetInstruction(HWND hwndForm, const char *text)
{
	CMStringW buf(text == nullptr ? "" : Utf2T(text));
	buf.Replace(L"\n\r", L"\r\n");
	SetDlgItemText(hwndForm, IDC_INSTRUCTION, buf);

	RECT rcText;
	GetWindowRect(GetDlgItem(hwndForm, IDC_INSTRUCTION), &rcText);
	int oldWidth = rcText.right - rcText.left;
	int deltaHeight = -(rcText.bottom - rcText.top);

	SetRect(&rcText, 0, 0, rcText.right - rcText.left, 0);
	HDC hdcEdit = GetDC(GetDlgItem(hwndForm, IDC_INSTRUCTION));
	HFONT hfntSave = (HFONT)SelectObject(hdcEdit, (HFONT)SendDlgItemMessage(hwndForm, IDC_INSTRUCTION, WM_GETFONT, 0, 0));
	DrawTextExW(hdcEdit, buf.GetBuffer(), buf.GetLength(), &rcText, DT_CALCRECT | DT_EDITCONTROL | DT_TOP | DT_WORDBREAK, nullptr);
	SelectObject(hdcEdit, hfntSave);
	ReleaseDC(GetDlgItem(hwndForm, IDC_INSTRUCTION), hdcEdit);

	RECT rcWindow; GetClientRect(hwndForm, &rcWindow);
	if (rcText.bottom - rcText.top > (rcWindow.bottom - rcWindow.top) / 5) {
		HWND hwndEdit = GetDlgItem(hwndForm, IDC_INSTRUCTION);
		SetWindowLongPtr(hwndEdit, GWL_STYLE, WS_VSCROLL | GetWindowLongPtr(hwndEdit, GWL_STYLE));
		rcText.bottom = rcText.top + (rcWindow.bottom - rcWindow.top) / 5;
	}
	else {
		HWND hwndEdit = GetDlgItem(hwndForm, IDC_INSTRUCTION);
		SetWindowLongPtr(hwndEdit, GWL_STYLE, ~WS_VSCROLL & GetWindowLongPtr(hwndEdit, GWL_STYLE));
	}
	deltaHeight += rcText.bottom - rcText.top;

	SetWindowPos(GetDlgItem(hwndForm, IDC_INSTRUCTION), nullptr, 0, 0,
		oldWidth,
		rcText.bottom - rcText.top,
		SWP_NOMOVE | SWP_NOZORDER);

	GetWindowRect(GetDlgItem(hwndForm, IDC_WHITERECT), &rcText);
	MapWindowPoints(nullptr, hwndForm, (LPPOINT)&rcText, 2);
	rcText.bottom += deltaHeight;
	SetWindowPos(GetDlgItem(hwndForm, IDC_WHITERECT), nullptr, 0, 0,
		rcText.right - rcText.left,
		rcText.bottom - rcText.top,
		SWP_NOMOVE | SWP_NOZORDER);

	GetWindowRect(GetDlgItem(hwndForm, IDC_FRAME1), &rcText);
	MapWindowPoints(nullptr, hwndForm, (LPPOINT)&rcText, 2);
	rcText.top += deltaHeight;
	SetWindowPos(GetDlgItem(hwndForm, IDC_FRAME1), nullptr,
		rcText.left,
		rcText.top,
		0, 0,
		SWP_NOSIZE | SWP_NOZORDER);

	GetWindowRect(GetDlgItem(hwndForm, IDC_FRAME), &rcText);
	MapWindowPoints(nullptr, hwndForm, (LPPOINT)&rcText, 2);
	rcText.top += deltaHeight;
	SetWindowPos(GetDlgItem(hwndForm, IDC_FRAME), nullptr,
		rcText.left,
		rcText.top,
		rcText.right - rcText.left,
		rcText.bottom - rcText.top,
		SWP_NOZORDER);

	GetWindowRect(GetDlgItem(hwndForm, IDC_VSCROLL), &rcText);
	MapWindowPoints(nullptr, hwndForm, (LPPOINT)&rcText, 2);
	rcText.top += deltaHeight;
	SetWindowPos(GetDlgItem(hwndForm, IDC_VSCROLL), nullptr,
		rcText.left,
		rcText.top,
		rcText.right - rcText.left,
		rcText.bottom - rcText.top,
		SWP_NOZORDER);
}

static TJabberFormControlType JabberFormTypeNameToId(const char *type)
{
	if (!mir_strcmp(type, "text-private"))
		return JFORM_CTYPE_TEXT_PRIVATE;
	if (!mir_strcmp(type, "text-multi") || !mir_strcmp(type, "jid-multi"))
		return JFORM_CTYPE_TEXT_MULTI;
	if (!mir_strcmp(type, "boolean"))
		return JFORM_CTYPE_BOOLEAN;
	if (!mir_strcmp(type, "list-single"))
		return JFORM_CTYPE_LIST_SINGLE;
	if (!mir_strcmp(type, "list-multi"))
		return JFORM_CTYPE_LIST_MULTI;
	if (!mir_strcmp(type, "fixed"))
		return JFORM_CTYPE_FIXED;
	if (!mir_strcmp(type, "hidden"))
		return JFORM_CTYPE_HIDDEN;

	return JFORM_CTYPE_TEXT_SINGLE;
}

static void JabberFormAddListItem(TJabberFormControlInfo *item, const char *text, bool selected)
{
	Utf2T wszText(text);

	uint32_t dwIndex;
	switch (item->type) {
	case JFORM_CTYPE_LIST_MULTI:
		dwIndex = SendMessage(item->hCtrl, LB_ADDSTRING, 0, wszText);
		if (selected) SendMessage(item->hCtrl, LB_SETSEL, TRUE, dwIndex);
		break;

	case JFORM_CTYPE_LIST_SINGLE:
		dwIndex = SendMessage(item->hCtrl, CB_ADDSTRING, 0, wszText);
		if (selected) SendMessage(item->hCtrl, CB_SETCURSEL, dwIndex, 0);
		break;
	}
}

void JabberFormCreateUI(HWND hwndStatic, const TiXmlElement *xNode, int *formHeight, BOOL bCompact)
{
	JabberFormDestroyUI(hwndStatic);

	if (xNode == nullptr || xNode->Name() == nullptr || mir_strcmp(xNode->Name(), "x") || hwndStatic == nullptr)
		return;

	TJabberFormLayoutInfo layout_info(hwndStatic, bCompact);

	CMStringA valueStr;
	for (auto *n : TiXmlFilter(xNode, "field")) {
		const char *labelStr = XmlGetAttr(n, "label");
		if (labelStr == nullptr)
			labelStr = XmlGetAttr(n, "var");

		TJabberFormControlType type = JabberFormTypeNameToId(XmlGetAttr(n, "type"));

		const char *str, *valueText;
		if (auto *v = XmlFirstChild(n, "value")) {
			valueText = v->GetText();
			if (type != JFORM_CTYPE_TEXT_MULTI)
				valueStr = valueText;
			else {
				valueStr.Empty();
				for (auto *it : TiXmlEnum(n)) {
					if (it->Name() && !mir_strcmp(it->Name(), "value") && it->GetText()) {
						if (!valueStr.IsEmpty())
							valueStr.Append("\r\n");
						valueStr.Append(it->GetText());
					}
				}
			}
		}
		else valueText = nullptr, valueStr.Empty();

		TJabberFormControlInfo *item = layout_info.AppendControl(type, labelStr, valueStr);

		if (XmlFirstChild(n, "required"))
			item->bRequired = true;

		if (type == JFORM_CTYPE_LIST_SINGLE) {
			for (auto *o : TiXmlFilter(n, "option")) {
				auto *v = XmlFirstChild(o, "value");
				if (v == nullptr || v->GetText() == nullptr)
					continue;
				if ((str = XmlGetAttr(o, "label")) == nullptr)
					str = v->GetText();
				if (str == nullptr)
					continue;

				bool selected = !mir_strcmp(valueText, v->GetText());
				JabberFormAddListItem(item, str, selected);
			}
		}
		else if (type == JFORM_CTYPE_LIST_MULTI) {
			for (auto *o : TiXmlFilter(n, "option")) {
				auto *v = XmlFirstChild(o, "value");
				if (v == nullptr || v->GetText() == nullptr)
					continue;

				if ((str = XmlGetAttr(o, "label")) == nullptr)
					str = v->GetText();
				if (str == nullptr)
					continue;

				bool selected = false;
				for (auto *vs : TiXmlEnum(n)) {
					if (!mir_strcmp(vs->Name(), "value") && !mir_strcmp(vs->GetText(), v->GetText())) {
						selected = true;
						break;
					}
				}
				JabberFormAddListItem(item, str, selected);
			}
		}
	}

	layout_info.OrderControls(formHeight);
}

void JabberFormDestroyUI(HWND hwndStatic)
{
	TJabberFormControlList *controls = (TJabberFormControlList *)GetWindowLongPtr(hwndStatic, GWLP_USERDATA);
	if (controls) {
		for (auto &it : *controls)
			mir_free(it);
		delete controls;
		SetWindowLongPtr(hwndStatic, GWLP_USERDATA, 0);
	}
}

void JabberFormGetData(HWND hwndStatic, TiXmlElement *xRoot, const TiXmlElement *xNode)
{
	const char *varName, *labelText, *str2;
	wchar_t *p, *q, *str;

	if (xNode == nullptr || xNode->Name() == nullptr || mir_strcmp(xNode->Name(), "x") || hwndStatic == nullptr)
		return;

	HWND hFrame = hwndStatic;
	int id = 0;
	auto *x = xRoot << XCHILD("x") << XATTR("xmlns", JABBER_FEAT_DATA_FORMS) << XATTR("type", "submit");

	for (auto *n : TiXmlFilter(xNode, "field")) {
		if ((varName = XmlGetAttr(n, "var")) == nullptr)
			continue;

		const char *type = XmlGetAttr(n, "type");
		if (type == nullptr)
			type = "text-single";

		HWND hCtrl = GetDlgItem(hFrame, id);
		TiXmlElement *field = x << XCHILD("field") << XATTR("var", varName);

		if (!mir_strcmp(type, "text-multi") || !mir_strcmp(type, "jid-multi")) {
			int len = GetWindowTextLength(GetDlgItem(hFrame, id));
			str = (wchar_t*)mir_alloc(sizeof(wchar_t)*(len + 1));
			GetDlgItemText(hFrame, id, str, len + 1);
			p = str;
			while (p != nullptr) {
				if ((q = wcsstr(p, L"\r\n")) != nullptr)
					*q = '\0';
				field << XCHILD("value", T2Utf(p));
				p = q ? q + 2 : nullptr;
			}
			mir_free(str);
			id++;
		}
		else if (!mir_strcmp(type, "boolean")) {
			wchar_t buf[10];
			_itow(IsDlgButtonChecked(hFrame, id) == BST_CHECKED ? 1 : 0, buf, 10);
			field << XCHILD("value", T2Utf(buf));
			id++;
		}
		else if (!mir_strcmp(type, "list-single")) {
			int len = GetWindowTextLength(GetDlgItem(hFrame, id));
			str = (wchar_t*)mir_alloc(sizeof(wchar_t)*(len + 1));
			GetDlgItemText(hFrame, id, str, len + 1);

			for (auto *o : TiXmlFilter(n, "option")) {
				auto *v = XmlFirstChild(o, "value");
				if (v != nullptr && v->GetText()) {
					if ((str2 = XmlGetAttr(o, "label")) == nullptr)
						str2 = v->GetText();
					if (!mir_strcmp(str2, T2Utf(str))) {
						field << XCHILD("value", v->GetText());
						break;
					}
				}
			}

			mir_free(str);
			id++;
		}
		else if (!mir_strcmp(type, "list-multi")) {
			int count = SendMessage(hCtrl, LB_GETCOUNT, 0, 0);
			for (int j = 0; j < count; j++) {
				if (SendMessage(hCtrl, LB_GETSEL, j, 0) > 0) {
					// an entry is selected
					int len = SendMessage(hCtrl, LB_GETTEXTLEN, j, 0);
					if ((str = (wchar_t*)mir_alloc((len + 1) * sizeof(wchar_t))) != nullptr) {
						SendMessage(hCtrl, LB_GETTEXT, j, (LPARAM)str);

						for (auto *o : TiXmlFilter(n, "option")) {
							auto *v = XmlFirstChild(o, "value");
							if (v != nullptr && v->GetText()) {
								if ((labelText = XmlGetAttr(o, "label")) == nullptr)
									labelText = v->GetText();

								if (!mir_strcmp(labelText, T2Utf(str)))
									field << XCHILD("value", v->GetText());
							}
						}
						mir_free(str);
					}
				}
			}
			id++;
		}
		else if (!mir_strcmp(type, "fixed") || !mir_strcmp(type, "hidden")) {
			auto *v = XmlFirstChild(n, "value");
			if (v != nullptr && v->GetText() != nullptr)
				field << XCHILD("value", v->GetText());
		}
		else { // everything else is considered "text-single" or "text-private"
			int len = GetWindowTextLength(GetDlgItem(hFrame, id));
			str = (wchar_t*)mir_alloc(sizeof(wchar_t)*(len + 1));
			GetDlgItemText(hFrame, id, str, len + 1);
			field << XCHILD("value", T2Utf(str));
			mir_free(str);
			id++;
		}
	}
}

CJabberFormDlg::CJabberFormDlg(CJabberProto *ppro, const TiXmlElement *xNode, char *defTitle, JABBER_FORM_SUBMIT_FUNC pfnSubmit, void *userdata) :
	CSuper(ppro, IDD_FORM),
	m_pfnSubmit(pfnSubmit),
	m_pUserdata(userdata),
	m_defTitle(mir_strdup(defTitle))
{
	m_xNode = xNode->DeepClone(&m_doc)->ToElement();
}

bool CJabberFormDlg::OnInitDialog()
{
	// Set dialog title
	if (auto *pszText = XmlGetChildText(m_xNode, "title"))
		SetWindowTextUtf(m_hwnd, pszText);
	else
		SetWindowTextUtf(m_hwnd, TranslateU(m_defTitle));

	// Set instruction field
	if (auto *pszText = XmlGetChildText(m_xNode, "instructions"))
		JabberFormSetInstruction(m_hwnd, pszText);
	else if (pszText = XmlGetChildText(m_xNode, "title"))
		JabberFormSetInstruction(m_hwnd, pszText);
	else
		JabberFormSetInstruction(m_hwnd, TranslateU(m_defTitle));

	// Create form
	if (m_xNode != nullptr) {
		RECT rect;
		GetClientRect(GetDlgItem(m_hwnd, IDC_FRAME), &m_frameRect);
		GetClientRect(GetDlgItem(m_hwnd, IDC_VSCROLL), &rect);
		m_frameRect.right -= (rect.right - rect.left);
		GetClientRect(GetDlgItem(m_hwnd, IDC_FRAME), &rect);
		m_frameHeight = rect.bottom - rect.top;
		JabberFormCreateUI(GetDlgItem(m_hwnd, IDC_FRAME), m_xNode, &m_formHeight);
	}

	m_curPos = 0;
	HWND hwndScroll = GetDlgItem(m_hwnd, IDC_VSCROLL);
	if (m_formHeight > m_frameHeight) {
		EnableWindow(hwndScroll, TRUE);
		SetScrollRange(hwndScroll, SB_CTL, 0, m_formHeight - m_frameHeight, FALSE);
	}
	else {
		ShowWindow(hwndScroll, SW_HIDE);
		SetScrollRange(hwndScroll, SB_CTL, 0, 0, FALSE);
	}

	// Enable WS_EX_CONTROLPARENT on IDC_FRAME (so tab stop goes through all its children)
	LONG frameExStyle = GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_FRAME), GWL_EXSTYLE);
	frameExStyle |= WS_EX_CONTROLPARENT;
	SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_FRAME), GWL_EXSTYLE, frameExStyle);

	if (m_pfnSubmit != nullptr)
		EnableWindow(GetDlgItem(m_hwnd, IDOK), TRUE);
	return true;
}

bool CJabberFormDlg::OnApply()
{
	(m_proto->*(m_pfnSubmit))(this, m_pUserdata);
	return true;
}

void CJabberFormDlg::OnDestroy()
{
	if (!m_bSucceeded && m_pfnCancel)
		(m_proto->*(m_pfnCancel))(this, m_pUserdata);

	JabberFormDestroyUI(GetDlgItem(m_hwnd, IDC_FRAME));
	mir_free(m_pUserdata);
}

INT_PTR CJabberFormDlg::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CTLCOLORSTATIC:
		if ((GetWindowLongPtr((HWND)lParam, GWL_ID) == IDC_WHITERECT) ||
			(GetWindowLongPtr((HWND)lParam, GWL_ID) == IDC_INSTRUCTION) ||
			(GetWindowLongPtr((HWND)lParam, GWL_ID) == IDC_TITLE)) {
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
		}
		return 0;

	case WM_MOUSEWHEEL:
		{
			short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (zDelta) {
				int nScrollLines = 0;
				SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, (void*)&nScrollLines, 0);
				for (int i = 0; i < (nScrollLines + 1) / 2; i++)
					SendMessage(m_hwnd, WM_VSCROLL, (zDelta < 0) ? SB_LINEDOWN : SB_LINEUP, 0);
			}
		}
		break;

	case WM_VSCROLL:
		int pos = m_curPos;
		switch (LOWORD(wParam)) {
		case SB_LINEDOWN:
			pos += 15;
			break;
		case SB_LINEUP:
			pos -= 15;
			break;
		case SB_PAGEDOWN:
			pos += (m_frameHeight - 10);
			break;
		case SB_PAGEUP:
			pos -= (m_frameHeight - 10);
			break;
		case SB_THUMBTRACK:
			pos = HIWORD(wParam);
			break;
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
		break;
	}
	return CSuper::DlgProc(msg, wParam, lParam);
}

static void CALLBACK JabberFormCreateDialogApcProc(void *param)
{
	((CJabberFormDlg*)param)->Show();
}

void CJabberFormDlg::GetData(TiXmlElement *xDest)
{
	JabberFormGetData(GetDlgItem(m_hwnd, IDC_FRAME), xDest, m_xNode);
}

void CJabberFormDlg::Display()
{
	CallFunctionAsync(JabberFormCreateDialogApcProc, this);
}
