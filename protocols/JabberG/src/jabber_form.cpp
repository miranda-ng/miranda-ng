/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-19 Miranda NG team

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

static LRESULT CALLBACK JabberFormMultiLineWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYDOWN:
		if (wParam == VK_TAB) {
			SetFocus(GetNextDlgTabItem(GetParent(GetParent(hwnd)), hwnd, GetKeyState(VK_SHIFT) < 0 ? TRUE : FALSE));
			return TRUE;
		};
		break;
	}
	return mir_callNextSubclass(hwnd, JabberFormMultiLineWndProc, msg, wParam, lParam);
}

struct TJabberFormControlInfo
{
	TJabberFormControlType type;
	SIZE szBlock;
	POINT ptLabel, ptCtrl;
	HWND hLabel, hCtrl;
};
typedef LIST<TJabberFormControlInfo> TJabberFormControlList;

struct TJabberFormLayoutInfo
{
	int ctrlHeight;
	int offset, width, maxLabelWidth;
	int y_pos, y_spacing;
	int id;
	bool compact;
};

void JabberFormCenterContent(HWND hwndStatic)
{
	RECT rcWindow;
	GetWindowRect(hwndStatic, &rcWindow);
	int minX = rcWindow.right;

	HWND oldChild = nullptr;
	HWND hWndChild = GetWindow(hwndStatic, GW_CHILD);
	while (hWndChild != oldChild && hWndChild != nullptr) {
		DWORD style = GetWindowLongPtr(hWndChild, GWL_STYLE);
		RECT rc;
		GetWindowRect(hWndChild, &rc);
		if ((style & SS_RIGHT) && !(style & WS_TABSTOP)) {
			RECT calcRect = rc;
			int len = GetWindowTextLength(hWndChild);
			wchar_t *text = (wchar_t*)_alloca(sizeof(wchar_t)*(len + 1));
			GetWindowText(hWndChild, text, len + 1);
			HDC hdc = GetDC(hWndChild);
			HFONT hfntSave = (HFONT)SelectObject(hdc, (HFONT)SendMessage(hWndChild, WM_GETFONT, 0, 0));
			DrawText(hdc, text, -1, &calcRect, DT_CALCRECT | DT_WORDBREAK);
			minX = min(minX, rc.right - (calcRect.right - calcRect.left));
			SelectObject(hdc, hfntSave);
			ReleaseDC(hWndChild, hdc);
		}
		else minX = min(minX, rc.left);

		oldChild = hWndChild;
		hWndChild = GetWindow(hWndChild, GW_HWNDNEXT);
	}

	if (minX > rcWindow.left + 5) {
		int dx = (minX - rcWindow.left) / 2;
		oldChild = nullptr;
		hWndChild = GetWindow(hwndStatic, GW_CHILD);
		while (hWndChild != oldChild && hWndChild != nullptr) {
			DWORD style = GetWindowLongPtr(hWndChild, GWL_STYLE);
			RECT rc;
			GetWindowRect(hWndChild, &rc);
			if ((style & SS_RIGHT) && !(style & WS_TABSTOP))
				MoveWindow(hWndChild, rc.left - rcWindow.left, rc.top - rcWindow.top, rc.right - rc.left - dx, rc.bottom - rc.top, TRUE);
			else
				MoveWindow(hWndChild, rc.left - dx - rcWindow.left, rc.top - rcWindow.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
			oldChild = hWndChild;
			hWndChild = GetWindow(hWndChild, GW_HWNDNEXT);
		}
	}
}

void JabberFormSetInstruction(HWND hwndForm, const char *text)
{
	CMStringW buf(text == nullptr ? "" : text);
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

void JabberFormLayoutSingleControl(TJabberFormControlInfo *item, TJabberFormLayoutInfo *layout_info, const char *labelStr, const char *valueStr)
{
	Utf2T wszLabel(labelStr), wszValue(valueStr);

	RECT rcLabel = { 0 }, rcCtrl = { 0 };
	if (item->hLabel) {
		SetRect(&rcLabel, 0, 0, layout_info->width, 0);
		HDC hdc = GetDC(item->hLabel);
		HFONT hfntSave = (HFONT)SelectObject(hdc, (HFONT)SendMessage(item->hLabel, WM_GETFONT, 0, 0));
		DrawTextW(hdc, wszLabel, -1, &rcLabel, DT_CALCRECT | DT_WORDBREAK);
		SelectObject(hdc, hfntSave);
		ReleaseDC(item->hLabel, hdc);
	}

	int indent = layout_info->compact ? 10 : 20;

	if ((layout_info->compact && (item->type != JFORM_CTYPE_BOOLEAN) && (item->type != JFORM_CTYPE_FIXED)) ||
		(rcLabel.right >= layout_info->maxLabelWidth) ||
		(rcLabel.bottom > layout_info->ctrlHeight) ||
		(item->type == JFORM_CTYPE_LIST_MULTI) ||
		(item->type == JFORM_CTYPE_TEXT_MULTI)) {
		int height = layout_info->ctrlHeight;
		if ((item->type == JFORM_CTYPE_LIST_MULTI) || (item->type == JFORM_CTYPE_TEXT_MULTI)) height *= 3;
		SetRect(&rcCtrl, indent, rcLabel.bottom, layout_info->width, rcLabel.bottom + height);
	}
	else if (item->type == JFORM_CTYPE_BOOLEAN) {
		SetRect(&rcCtrl, 0, 0, layout_info->width - 20, 0);
		HDC hdc = GetDC(item->hCtrl);
		HFONT hfntSave = (HFONT)SelectObject(hdc, (HFONT)SendMessage(item->hCtrl, WM_GETFONT, 0, 0));
		DrawText(hdc, wszLabel, -1, &rcCtrl, DT_CALCRECT | DT_RIGHT | DT_WORDBREAK);
		SelectObject(hdc, hfntSave);
		ReleaseDC(item->hCtrl, hdc);
		rcCtrl.right += 20;
	}
	else if (item->type == JFORM_CTYPE_FIXED) {
		SetRect(&rcCtrl, 0, 0, layout_info->width, 0);
		HDC hdc = GetDC(item->hCtrl);
		HFONT hfntSave = (HFONT)SelectObject(hdc, (HFONT)SendMessage(item->hCtrl, WM_GETFONT, 0, 0));
		DrawText(hdc, wszValue, -1, &rcCtrl, DT_CALCRECT | DT_EDITCONTROL);
		rcCtrl.right += 20;
		SelectObject(hdc, hfntSave);
		ReleaseDC(item->hCtrl, hdc);
	}
	else {
		SetRect(&rcCtrl, rcLabel.right + 5, 0, layout_info->width, layout_info->ctrlHeight);
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
	item->szBlock.cx = layout_info->width;
	item->szBlock.cy = max(rcLabel.bottom, rcCtrl.bottom);
}

#define JabberFormCreateLabel()	\
	CreateWindow(L"static", wszLabel, WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE, \
		0, 0, 0, 0, hwndStatic, (HMENU)-1, g_plugin.getInst(), nullptr)

HJFORMCTRL JabberFormAppendControl(HWND hwndStatic, HJFORMLAYOUT layout_info, TJabberFormControlType type, const char *labelStr, const char *valueStr)
{
	TJabberFormControlList *controls = (TJabberFormControlList *)GetWindowLongPtr(hwndStatic, GWLP_USERDATA);
	if (!controls) {
		controls = new TJabberFormControlList(5);
		SetWindowLongPtr(hwndStatic, GWLP_USERDATA, (LONG_PTR)controls);
	}

	TJabberFormControlInfo *item = (TJabberFormControlInfo *)mir_alloc(sizeof(TJabberFormControlInfo));
	item->type = type;
	item->hLabel = item->hCtrl = nullptr;
	Utf2T wszLabel(labelStr), wszValue(valueStr);

	switch (type) {
	case JFORM_CTYPE_TEXT_PRIVATE:
		item->hLabel = JabberFormCreateLabel();
		item->hCtrl = CreateWindowEx(WS_EX_CLIENTEDGE, L"edit", wszValue,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | ES_PASSWORD,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, g_plugin.getInst(), nullptr);
		++layout_info->id;
		break;

	case JFORM_CTYPE_TEXT_MULTI:
		item->hLabel = JabberFormCreateLabel();
		item->hCtrl = CreateWindowEx(WS_EX_CLIENTEDGE, L"edit", wszValue,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, g_plugin.getInst(), nullptr);
		mir_subclassWindow(item->hCtrl, JabberFormMultiLineWndProc);
		++layout_info->id;
		break;

	case JFORM_CTYPE_BOOLEAN:
		item->hCtrl = CreateWindowEx(0, L"button", wszLabel,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX | BS_MULTILINE,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, g_plugin.getInst(), nullptr);
		if (valueStr && !mir_wstrcmp(wszValue, L"1"))
			SendMessage(item->hCtrl, BM_SETCHECK, 1, 0);
		++layout_info->id;
		break;

	case JFORM_CTYPE_LIST_SINGLE:
		item->hLabel = JabberFormCreateLabel();
		item->hCtrl = CreateWindowExA(WS_EX_CLIENTEDGE, "combobox", nullptr,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, g_plugin.getInst(), nullptr);
		++layout_info->id;
		break;

	case JFORM_CTYPE_LIST_MULTI:
		item->hLabel = JabberFormCreateLabel();
		item->hCtrl = CreateWindowExA(WS_EX_CLIENTEDGE, "listbox",
			nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | LBS_MULTIPLESEL,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, g_plugin.getInst(), nullptr);
		++layout_info->id;
		break;

	case JFORM_CTYPE_FIXED:
		item->hCtrl = CreateWindow(L"edit", wszValue,
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL,
			0, 0, 0, 0,
			hwndStatic, (HMENU)-1, g_plugin.getInst(), nullptr);
		break;

	case JFORM_CTYPE_HIDDEN:
		break;

	case JFORM_CTYPE_TEXT_SINGLE:
		item->hLabel = labelStr ? (JabberFormCreateLabel()) : nullptr;
		item->hCtrl = CreateWindowEx(WS_EX_CLIENTEDGE, L"edit", wszValue,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, g_plugin.getInst(), nullptr);
		++layout_info->id;
		break;
	}

	HFONT hFont = (HFONT)SendMessage(GetParent(hwndStatic), WM_GETFONT, 0, 0);
	if (item->hLabel) SendMessage(item->hLabel, WM_SETFONT, (WPARAM)hFont, 0);
	if (item->hCtrl) SendMessage(item->hCtrl, WM_SETFONT, (WPARAM)hFont, 0);

	JabberFormLayoutSingleControl(item, layout_info, labelStr, valueStr);

	controls->insert(item);
	return item;
}

static void JabberFormAddListItem(TJabberFormControlInfo *item, const char *text, bool selected)
{
	Utf2T wszText(text);

	DWORD dwIndex;
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

void JabberFormLayoutControls(HWND hwndStatic, HJFORMLAYOUT layout_info, int *formHeight)
{
	TJabberFormControlList *controls = (TJabberFormControlList *)GetWindowLongPtr(hwndStatic, GWLP_USERDATA);
	if (!controls) return;

	for (auto &it : *controls) {
		if (it->hLabel)
			SetWindowPos(it->hLabel, nullptr,
				layout_info->offset + it->ptLabel.x, layout_info->y_pos + it->ptLabel.y, 0, 0,
				SWP_NOZORDER | SWP_NOSIZE);

		if (it->hCtrl)
			SetWindowPos(it->hCtrl, nullptr,
				layout_info->offset + it->ptCtrl.x, layout_info->y_pos + it->ptCtrl.y, 0, 0,
				SWP_NOZORDER | SWP_NOSIZE);

		layout_info->y_pos += it->szBlock.cy;
		layout_info->y_pos += layout_info->y_spacing;
	}

	*formHeight = layout_info->y_pos + (layout_info->compact ? 0 : 9);
}

HJFORMLAYOUT JabberFormCreateLayout(HWND hwndStatic)
{
	RECT frameRect;
	GetClientRect(hwndStatic, &frameRect);

	TJabberFormLayoutInfo *layout_info = (TJabberFormLayoutInfo *)mir_alloc(sizeof(TJabberFormLayoutInfo));
	layout_info->compact = false;
	layout_info->ctrlHeight = 20;
	layout_info->id = 0;
	layout_info->width = frameRect.right - frameRect.left - 20 - 10;
	layout_info->y_spacing = 5;
	layout_info->maxLabelWidth = layout_info->width * 2 / 5;
	layout_info->offset = 10;
	layout_info->y_pos = 14;
	return layout_info;
}

void JabberFormCreateUI(HWND hwndStatic, TiXmlElement *xNode, int *formHeight, BOOL bCompact)
{
	JabberFormDestroyUI(hwndStatic);

	const char *typeName, *str, *valueText, *labelStr;
	char *valueStr;

	if (xNode == nullptr || xNode->Name() == nullptr || mir_strcmp(xNode->Name(), "x") || hwndStatic == nullptr)
		return;

	RECT frameRect;
	GetClientRect(hwndStatic, &frameRect);

	TJabberFormLayoutInfo layout_info;
	layout_info.compact = bCompact ? true : false;
	layout_info.ctrlHeight = 20;
	layout_info.id = 0;
	layout_info.width = frameRect.right - frameRect.left - 20;
	if (!bCompact) layout_info.width -= 10;
	layout_info.y_spacing = bCompact ? 1 : 5;
	layout_info.maxLabelWidth = layout_info.width * 2 / 5;
	layout_info.offset = 10;
	layout_info.y_pos = bCompact ? 0 : 14;
	for (auto *n : TiXmlFilter(xNode, "field")) {
		if ((typeName = n->Attribute("type")) == nullptr)
			continue;

		if (auto *label = n->Attribute("label"))
			labelStr = label;
		else
			labelStr = n->Attribute("var");

		TJabberFormControlType type = JabberFormTypeNameToId(typeName);

		if (auto *v = n->FirstChildElement("value")) {
			valueText = v->GetText();
			if (type != JFORM_CTYPE_TEXT_MULTI)
				valueStr = mir_strdup(valueText);
			else {
				CMStringA tmp;
				for (auto *it : TiXmlEnum(n)) {
					if (it->Name() && !mir_strcmp(it->Name(), "value") && it->GetText()) {
						if (!tmp.IsEmpty())
							tmp.Append("\r\n");
						tmp.Append(it->GetText());
					}
				}
				valueStr = tmp.Detach();
			}
		}
		else valueText = valueStr = nullptr;

		TJabberFormControlInfo *item = JabberFormAppendControl(hwndStatic, &layout_info, type, labelStr, valueStr);

		mir_free(valueStr);

		if (type == JFORM_CTYPE_LIST_SINGLE) {
			for (auto *o : TiXmlFilter(n, "option")) {
				auto *v = o->FirstChildElement("value");
				if (v == nullptr || v->GetText() == nullptr)
					continue;
				if ((str = o->Attribute("label")) == nullptr)
					str = v->GetText();
				if (str == nullptr)
					continue;

				bool selected = !mir_strcmp(valueText, v->GetText());
				JabberFormAddListItem(item, str, selected);
			}
		}
		else if (type == JFORM_CTYPE_LIST_MULTI) {
			for (auto *o : TiXmlFilter(n, "option")) {
				auto *v = o->FirstChildElement("value");
				if (v == nullptr || v->GetText() == nullptr)
					continue;

				if ((str = o->Attribute("label")) == nullptr)
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

	JabberFormLayoutControls(hwndStatic, &layout_info, formHeight);
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

TiXmlElement* JabberFormGetData(HWND hwndStatic, TiXmlDocument *doc, TiXmlElement *xNode)
{
	const char *varName, *type, *labelText, *str2;
	wchar_t *p, *q, *str;

	if (xNode == nullptr || xNode->Name() == nullptr || mir_strcmp(xNode->Name(), "x") || hwndStatic == nullptr)
		return nullptr;

	HWND hFrame = hwndStatic;
	int id = 0;
	XmlNode x("x");
	x << XATTR("xmlns", JABBER_FEAT_DATA_FORMS) << XATTR("type", "submit");

	for (auto *n : TiXmlFilter(xNode, "field")) {
		if ((varName = n->Attribute("var")) == nullptr || (type = n->Attribute("type")) == nullptr)
			continue;

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
				auto *v = o->FirstChildElement("value");
				if (v != nullptr && v->GetText()) {
					if ((str2 = o->Attribute("label")) == nullptr)
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
							auto *v = o->FirstChildElement("value");
							if (v != nullptr && v->GetText()) {
								if ((labelText = o->Attribute("label")) == nullptr)
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
			auto *v = n->FirstChildElement("value");
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

	return x.ToElement()->DeepClone(doc)->ToElement();
}

struct JABBER_FORM_INFO
{
	~JABBER_FORM_INFO();

	CJabberProto *ppro;
	TiXmlDocument doc;
	TiXmlElement *xNode;
	wchar_t defTitle[128];	// Default title if no <title/> in xNode
	RECT frameRect;		// Clipping region of the frame to scroll
	int frameHeight;	// Height of the frame (can be eliminated, redundant to frameRect)
	int formHeight;		// Actual height of the form
	int curPos;			// Current scroll position
	JABBER_FORM_SUBMIT_FUNC pfnSubmit;
	void *userdata;
};

static INT_PTR CALLBACK JabberFormDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	JABBER_FORM_INFO *jfi = (JABBER_FORM_INFO*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		// lParam is (JABBER_FORM_INFO *)
		TranslateDialogDefault(hwndDlg);
		ShowWindow(GetDlgItem(hwndDlg, IDC_FRAME_TEXT), SW_HIDE);
		jfi = (JABBER_FORM_INFO*)lParam;
		if (jfi != nullptr) {
			LONG frameExStyle;
			// Set dialog title
			if (auto *pszText = XmlGetChildText(jfi->xNode, "title"))
				SetWindowTextUtf(hwndDlg, pszText);
			else
				SetWindowText(hwndDlg, TranslateW(jfi->defTitle));

			// Set instruction field
			if (auto *pszText = XmlGetChildText(jfi->xNode, "instructions"))
				JabberFormSetInstruction(hwndDlg, pszText);
			else if (pszText = XmlGetChildText(jfi->xNode, "title"))
				JabberFormSetInstruction(hwndDlg, pszText);
			else
				JabberFormSetInstruction(hwndDlg, TranslateU(T2Utf(jfi->defTitle)));

			// Create form
			if (jfi->xNode != nullptr) {
				RECT rect;
				GetClientRect(GetDlgItem(hwndDlg, IDC_FRAME), &(jfi->frameRect));
				GetClientRect(GetDlgItem(hwndDlg, IDC_VSCROLL), &rect);
				jfi->frameRect.right -= (rect.right - rect.left);
				GetClientRect(GetDlgItem(hwndDlg, IDC_FRAME), &rect);
				jfi->frameHeight = rect.bottom - rect.top;
				JabberFormCreateUI(GetDlgItem(hwndDlg, IDC_FRAME), jfi->xNode, &(jfi->formHeight));
			}

			if (jfi->formHeight > jfi->frameHeight) {
				HWND hwndScroll = GetDlgItem(hwndDlg, IDC_VSCROLL);
				EnableWindow(hwndScroll, TRUE);
				SetScrollRange(hwndScroll, SB_CTL, 0, jfi->formHeight - jfi->frameHeight, FALSE);
				jfi->curPos = 0;
			}

			// Enable WS_EX_CONTROLPARENT on IDC_FRAME (so tab stop goes through all its children)
			frameExStyle = GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FRAME), GWL_EXSTYLE);
			frameExStyle |= WS_EX_CONTROLPARENT;
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FRAME), GWL_EXSTYLE, frameExStyle);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)jfi);
			if (jfi->pfnSubmit != nullptr)
				EnableWindow(GetDlgItem(hwndDlg, IDC_SUBMIT), TRUE);
		}
		return TRUE;

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
					SendMessage(hwndDlg, WM_VSCROLL, (zDelta < 0) ? SB_LINEDOWN : SB_LINEUP, 0);
			}
		}
		break;

	case WM_VSCROLL:
		if (jfi != nullptr) {
			int pos = jfi->curPos;
			switch (LOWORD(wParam)) {
			case SB_LINEDOWN:
				pos += 15;
				break;
			case SB_LINEUP:
				pos -= 15;
				break;
			case SB_PAGEDOWN:
				pos += (jfi->frameHeight - 10);
				break;
			case SB_PAGEUP:
				pos -= (jfi->frameHeight - 10);
				break;
			case SB_THUMBTRACK:
				pos = HIWORD(wParam);
				break;
			}
			if (pos > (jfi->formHeight - jfi->frameHeight))
				pos = jfi->formHeight - jfi->frameHeight;
			if (pos < 0)
				pos = 0;
			if (jfi->curPos != pos) {
				ScrollWindow(GetDlgItem(hwndDlg, IDC_FRAME), 0, jfi->curPos - pos, nullptr, &(jfi->frameRect));
				SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, pos, TRUE);
				jfi->curPos = pos;
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SUBMIT:
			if (jfi != nullptr) {
				TiXmlElement *n = JabberFormGetData(GetDlgItem(hwndDlg, IDC_FRAME), &jfi->doc, jfi->xNode);
				(jfi->ppro->*(jfi->pfnSubmit))(n, jfi->userdata);
			}
			__fallthrough;

		case IDCANCEL:
		case IDCLOSE:
			DestroyWindow(hwndDlg);
			return TRUE;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		JabberFormDestroyUI(GetDlgItem(hwndDlg, IDC_FRAME));
		delete jfi;
		break;
	}

	return FALSE;
}

static VOID CALLBACK JabberFormCreateDialogApcProc(void* param)
{
	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FORM), nullptr, JabberFormDlgProc, (LPARAM)param);
}

void CJabberProto::FormCreateDialog(const TiXmlElement *xNode, char *defTitle, JABBER_FORM_SUBMIT_FUNC pfnSubmit, void *userdata)
{
	JABBER_FORM_INFO *jfi = new JABBER_FORM_INFO;
	memset(jfi, 0, sizeof(JABBER_FORM_INFO));
	jfi->ppro = this;
	jfi->xNode = xNode->DeepClone(&jfi->doc)->ToElement();
	if (defTitle)
		wcsncpy_s(jfi->defTitle, Utf2T(defTitle), _TRUNCATE);
	jfi->pfnSubmit = pfnSubmit;
	jfi->userdata = userdata;

	CallFunctionAsync(JabberFormCreateDialogApcProc, jfi);
}

//=======================================================================================

JABBER_FORM_INFO::~JABBER_FORM_INFO()
{
	mir_free(userdata);
}
