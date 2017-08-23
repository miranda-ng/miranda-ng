/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (ñ) 2012-17 Miranda NG project

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

void JabberFormSetInstruction(HWND hwndForm, const wchar_t *text)
{
	if (!text) text = L"";

	size_t len = mir_wstrlen(text);
	size_t fixedLen = len;
	for (int i = 1; i < len; i++)
		if ((text[i - 1] == '\n') && (text[i] != '\r'))
			++fixedLen;

	wchar_t *fixedText = nullptr;
	if (fixedLen != len) {
		fixedText = (wchar_t *)mir_alloc(sizeof(wchar_t) * (fixedLen + 1));
		wchar_t *p = fixedText;
		for (int i = 0; i < len; i++) {
			*p = text[i];
			if (i && (text[i] == '\n') && (text[i] != '\r')) {
				*p++ = '\r';
				*p = '\n';
			}
			++p;
		}
		*p = 0;
		text = fixedText;
	}

	SetDlgItemText(hwndForm, IDC_INSTRUCTION, text);

	RECT rcText;
	GetWindowRect(GetDlgItem(hwndForm, IDC_INSTRUCTION), &rcText);
	int oldWidth = rcText.right - rcText.left;
	int deltaHeight = -(rcText.bottom - rcText.top);

	SetRect(&rcText, 0, 0, rcText.right - rcText.left, 0);
	HDC hdcEdit = GetDC(GetDlgItem(hwndForm, IDC_INSTRUCTION));
	HFONT hfntSave = (HFONT)SelectObject(hdcEdit, (HFONT)SendDlgItemMessage(hwndForm, IDC_INSTRUCTION, WM_GETFONT, 0, 0));
	DrawTextEx(hdcEdit, (wchar_t *)text, (int)mir_wstrlen(text), &rcText, DT_CALCRECT | DT_EDITCONTROL | DT_TOP | DT_WORDBREAK, nullptr);
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

	SetWindowPos(GetDlgItem(hwndForm, IDC_INSTRUCTION), 0, 0, 0,
		oldWidth,
		rcText.bottom - rcText.top,
		SWP_NOMOVE | SWP_NOZORDER);

	GetWindowRect(GetDlgItem(hwndForm, IDC_WHITERECT), &rcText);
	MapWindowPoints(nullptr, hwndForm, (LPPOINT)&rcText, 2);
	rcText.bottom += deltaHeight;
	SetWindowPos(GetDlgItem(hwndForm, IDC_WHITERECT), 0, 0, 0,
		rcText.right - rcText.left,
		rcText.bottom - rcText.top,
		SWP_NOMOVE | SWP_NOZORDER);

	GetWindowRect(GetDlgItem(hwndForm, IDC_FRAME1), &rcText);
	MapWindowPoints(nullptr, hwndForm, (LPPOINT)&rcText, 2);
	rcText.top += deltaHeight;
	SetWindowPos(GetDlgItem(hwndForm, IDC_FRAME1), 0,
		rcText.left,
		rcText.top,
		0, 0,
		SWP_NOSIZE | SWP_NOZORDER);

	GetWindowRect(GetDlgItem(hwndForm, IDC_FRAME), &rcText);
	MapWindowPoints(nullptr, hwndForm, (LPPOINT)&rcText, 2);
	rcText.top += deltaHeight;
	SetWindowPos(GetDlgItem(hwndForm, IDC_FRAME), 0,
		rcText.left,
		rcText.top,
		rcText.right - rcText.left,
		rcText.bottom - rcText.top,
		SWP_NOZORDER);

	GetWindowRect(GetDlgItem(hwndForm, IDC_VSCROLL), &rcText);
	MapWindowPoints(nullptr, hwndForm, (LPPOINT)&rcText, 2);
	rcText.top += deltaHeight;
	SetWindowPos(GetDlgItem(hwndForm, IDC_VSCROLL), 0,
		rcText.left,
		rcText.top,
		rcText.right - rcText.left,
		rcText.bottom - rcText.top,
		SWP_NOZORDER);

	if (fixedText) mir_free(fixedText);
}

static TJabberFormControlType JabberFormTypeNameToId(const wchar_t *type)
{
	if (!mir_wstrcmp(type, L"text-private"))
		return JFORM_CTYPE_TEXT_PRIVATE;
	if (!mir_wstrcmp(type, L"text-multi") || !mir_wstrcmp(type, L"jid-multi"))
		return JFORM_CTYPE_TEXT_MULTI;
	if (!mir_wstrcmp(type, L"boolean"))
		return JFORM_CTYPE_BOOLEAN;
	if (!mir_wstrcmp(type, L"list-single"))
		return JFORM_CTYPE_LIST_SINGLE;
	if (!mir_wstrcmp(type, L"list-multi"))
		return JFORM_CTYPE_LIST_MULTI;
	if (!mir_wstrcmp(type, L"fixed"))
		return JFORM_CTYPE_FIXED;
	if (!mir_wstrcmp(type, L"hidden"))
		return JFORM_CTYPE_HIDDEN;

	return JFORM_CTYPE_TEXT_SINGLE;
}

void JabberFormLayoutSingleControl(TJabberFormControlInfo *item, TJabberFormLayoutInfo *layout_info, const wchar_t *labelStr, const wchar_t *valueStr)
{
	RECT rcLabel = { 0 }, rcCtrl = { 0 };
	if (item->hLabel) {
		SetRect(&rcLabel, 0, 0, layout_info->width, 0);
		HDC hdc = GetDC(item->hLabel);
		HFONT hfntSave = (HFONT)SelectObject(hdc, (HFONT)SendMessage(item->hLabel, WM_GETFONT, 0, 0));
		DrawText(hdc, labelStr, -1, &rcLabel, DT_CALCRECT | DT_WORDBREAK);
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
		DrawText(hdc, labelStr, -1, &rcCtrl, DT_CALCRECT | DT_RIGHT | DT_WORDBREAK);
		SelectObject(hdc, hfntSave);
		ReleaseDC(item->hCtrl, hdc);
		rcCtrl.right += 20;
	}
	else if (item->type == JFORM_CTYPE_FIXED) {
		SetRect(&rcCtrl, 0, 0, layout_info->width, 0);
		HDC hdc = GetDC(item->hCtrl);
		HFONT hfntSave = (HFONT)SelectObject(hdc, (HFONT)SendMessage(item->hCtrl, WM_GETFONT, 0, 0));
		DrawText(hdc, valueStr, -1, &rcCtrl, DT_CALCRECT | DT_EDITCONTROL);
		rcCtrl.right += 20;
		SelectObject(hdc, hfntSave);
		ReleaseDC(item->hCtrl, hdc);
	}
	else {
		SetRect(&rcCtrl, rcLabel.right + 5, 0, layout_info->width, layout_info->ctrlHeight);
		rcLabel.bottom = rcCtrl.bottom;
	}

	if (item->hLabel)
		SetWindowPos(item->hLabel, 0, 0, 0, rcLabel.right - rcLabel.left, rcLabel.bottom - rcLabel.top, SWP_NOZORDER | SWP_NOMOVE);
	if (item->hCtrl)
		SetWindowPos(item->hCtrl, 0, 0, 0, rcCtrl.right - rcCtrl.left, rcCtrl.bottom - rcCtrl.top, SWP_NOZORDER | SWP_NOMOVE);

	item->ptLabel.x = rcLabel.left;
	item->ptLabel.y = rcLabel.top;
	item->ptCtrl.x = rcCtrl.left;
	item->ptCtrl.y = rcCtrl.top;
	item->szBlock.cx = layout_info->width;
	item->szBlock.cy = max(rcLabel.bottom, rcCtrl.bottom);
}

#define JabberFormCreateLabel()	\
	CreateWindow(L"static", labelStr, WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE, \
		0, 0, 0, 0, hwndStatic, (HMENU)-1, hInst, nullptr)

TJabberFormControlInfo *JabberFormAppendControl(HWND hwndStatic, TJabberFormLayoutInfo *layout_info, TJabberFormControlType type, const wchar_t *labelStr, const wchar_t *valueStr)
{
	TJabberFormControlList *controls = (TJabberFormControlList *)GetWindowLongPtr(hwndStatic, GWLP_USERDATA);
	if (!controls) {
		controls = new TJabberFormControlList(5);
		SetWindowLongPtr(hwndStatic, GWLP_USERDATA, (LONG_PTR)controls);
	}

	TJabberFormControlInfo *item = (TJabberFormControlInfo *)mir_alloc(sizeof(TJabberFormControlInfo));
	item->type = type;
	item->hLabel = item->hCtrl = nullptr;

	switch (type) {
	case JFORM_CTYPE_TEXT_PRIVATE:
		item->hLabel = JabberFormCreateLabel();
		item->hCtrl = CreateWindowEx(WS_EX_CLIENTEDGE, L"edit", valueStr,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | ES_PASSWORD,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, hInst, nullptr);
		++layout_info->id;
		break;

	case JFORM_CTYPE_TEXT_MULTI:
		item->hLabel = JabberFormCreateLabel();
		item->hCtrl = CreateWindowEx(WS_EX_CLIENTEDGE, L"edit", valueStr,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, hInst, nullptr);
		mir_subclassWindow(item->hCtrl, JabberFormMultiLineWndProc);
		++layout_info->id;
		break;

	case JFORM_CTYPE_BOOLEAN:
		item->hCtrl = CreateWindowEx(0, L"button", labelStr,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX | BS_MULTILINE,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, hInst, nullptr);
		if (valueStr && !mir_wstrcmp(valueStr, L"1"))
			SendMessage(item->hCtrl, BM_SETCHECK, 1, 0);
		++layout_info->id;
		break;

	case JFORM_CTYPE_LIST_SINGLE:
		item->hLabel = JabberFormCreateLabel();
		item->hCtrl = CreateWindowExA(WS_EX_CLIENTEDGE, "combobox", nullptr,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, hInst, nullptr);
		++layout_info->id;
		break;

	case JFORM_CTYPE_LIST_MULTI:
		item->hLabel = JabberFormCreateLabel();
		item->hCtrl = CreateWindowExA(WS_EX_CLIENTEDGE, "listbox",
			nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | LBS_MULTIPLESEL,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, hInst, nullptr);
		++layout_info->id;
		break;

	case JFORM_CTYPE_FIXED:
		item->hCtrl = CreateWindow(L"edit", valueStr,
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL,
			0, 0, 0, 0,
			hwndStatic, (HMENU)-1, hInst, nullptr);
		break;

	case JFORM_CTYPE_HIDDEN:
		break;

	case JFORM_CTYPE_TEXT_SINGLE:
		item->hLabel = labelStr ? (JabberFormCreateLabel()) : nullptr;
		item->hCtrl = CreateWindowEx(WS_EX_CLIENTEDGE, L"edit", valueStr,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, hInst, nullptr);
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

static void JabberFormAddListItem(TJabberFormControlInfo *item, const wchar_t *text, bool selected)
{
	DWORD dwIndex;
	switch (item->type) {
	case JFORM_CTYPE_LIST_MULTI:
		dwIndex = SendMessage(item->hCtrl, LB_ADDSTRING, 0, (LPARAM)text);
		if (selected) SendMessage(item->hCtrl, LB_SETSEL, TRUE, dwIndex);
		break;
	case JFORM_CTYPE_LIST_SINGLE:
		dwIndex = SendMessage(item->hCtrl, CB_ADDSTRING, 0, (LPARAM)text);
		if (selected) SendMessage(item->hCtrl, CB_SETCURSEL, dwIndex, 0);
		break;
	}
}

void JabberFormLayoutControls(HWND hwndStatic, TJabberFormLayoutInfo *layout_info, int *formHeight)
{
	TJabberFormControlList *controls = (TJabberFormControlList *)GetWindowLongPtr(hwndStatic, GWLP_USERDATA);
	if (!controls) return;

	for (int i = 0; i < controls->getCount(); i++) {
		if ((*controls)[i]->hLabel)
			SetWindowPos((*controls)[i]->hLabel, 0,
			layout_info->offset + (*controls)[i]->ptLabel.x, layout_info->y_pos + (*controls)[i]->ptLabel.y, 0, 0,
			SWP_NOZORDER | SWP_NOSIZE);
		if ((*controls)[i]->hCtrl)
			SetWindowPos((*controls)[i]->hCtrl, 0,
			layout_info->offset + (*controls)[i]->ptCtrl.x, layout_info->y_pos + (*controls)[i]->ptCtrl.y, 0, 0,
			SWP_NOZORDER | SWP_NOSIZE);

		layout_info->y_pos += (*controls)[i]->szBlock.cy;
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

void JabberFormCreateUI(HWND hwndStatic, HXML xNode, int *formHeight, BOOL bCompact)
{
	JabberFormDestroyUI(hwndStatic);

	HXML v, vs;

	const wchar_t *label, *typeName, *varStr, *str, *valueText;
	wchar_t *labelStr, *valueStr;
	RECT frameRect;

	if (xNode == nullptr || XmlGetName(xNode) == nullptr || mir_wstrcmp(XmlGetName(xNode), L"x") || hwndStatic == nullptr)
		return;

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
	for (int i = 0;; i++) {
		HXML n = XmlGetChild(xNode, i);
		if (!n)
			break;

		if (mir_wstrcmp(XmlGetName(n), L"field"))
			continue;

		varStr = XmlGetAttrValue(n, L"var");
		if ((typeName = XmlGetAttrValue(n, L"type")) == nullptr)
			continue;

		if ((label = XmlGetAttrValue(n, L"label")) != nullptr)
			labelStr = mir_wstrdup(label);
		else
			labelStr = mir_wstrdup(varStr);

		TJabberFormControlType type = JabberFormTypeNameToId(typeName);

		if ((v = XmlGetChild(n, "value")) != nullptr) {
			valueText = XmlGetText(v);
			if (type != JFORM_CTYPE_TEXT_MULTI)
				valueStr = mir_wstrdup(valueText);
			else {
				size_t size = 1;
				for (int j = 0;; j++) {
					v = XmlGetChild(n, j);
					if (!v)
						break;
					if (XmlGetName(v) && !mir_wstrcmp(XmlGetName(v), L"value") && XmlGetText(v))
						size += mir_wstrlen(XmlGetText(v)) + 2;
				}
				valueStr = (wchar_t*)mir_alloc(sizeof(wchar_t)*size);
				valueStr[0] = '\0';
				for (int j = 0;; j++) {
					v = XmlGetChild(n, j);
					if (!v)
						break;
					if (XmlGetName(v) && !mir_wstrcmp(XmlGetName(v), L"value") && XmlGetText(v)) {
						if (valueStr[0])
							mir_wstrcat(valueStr, L"\r\n");
						mir_wstrcat(valueStr, XmlGetText(v));
					}
				}
			}
		}
		else valueText = valueStr = nullptr;

		TJabberFormControlInfo *item = JabberFormAppendControl(hwndStatic, &layout_info, type, labelStr, valueStr);

		mir_free(labelStr);
		mir_free(valueStr);

		if (type == JFORM_CTYPE_LIST_SINGLE) {
			for (int j = 0;; j++) {
				HXML o = XmlGetChild(n, j);
				if (o == nullptr)
					break;

				if (mir_wstrcmp(XmlGetName(o), L"option"))
					continue;
				if ((v = XmlGetChild(o, "value")) == nullptr || XmlGetText(v) == nullptr)
					continue;
				if ((str = XmlGetAttrValue(o, L"label")) == nullptr)
					str = XmlGetText(v);
				if (str == nullptr)
					continue;

				bool selected = !mir_wstrcmp(valueText, XmlGetText(v));
				JabberFormAddListItem(item, str, selected);
			}
		}
		else if (type == JFORM_CTYPE_LIST_MULTI) {
			for (int j = 0;; j++) {
				HXML o = XmlGetChild(n, j);
				if (o == nullptr)
					break;

				if (mir_wstrcmp(XmlGetName(o), L"option"))
					continue;
				if ((v = XmlGetChild(o, "value")) == nullptr || XmlGetText(v) == nullptr)
					continue;
				if ((str = XmlGetAttrValue(o, L"label")) == nullptr)
					str = XmlGetText(v);
				if (str == nullptr)
					continue;

				bool selected = false;
				for (int k = 0;; k++) {
					vs = XmlGetChild(n, k);
					if (!vs)
						break;
					if (!mir_wstrcmp(XmlGetName(vs), L"value") && !mir_wstrcmp(XmlGetText(vs), XmlGetText(v))) {
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
		for (int i = 0; i < controls->getCount(); i++)
			mir_free((*controls)[i]);
		delete controls;
		SetWindowLongPtr(hwndStatic, GWLP_USERDATA, 0);
	}
}

HXML JabberFormGetData(HWND hwndStatic, HXML xNode)
{
	HWND hFrame, hCtrl;
	HXML n, v, o;
	int id, j, k, len;
	const wchar_t *varName, *type, *fieldStr, *labelText, *str2;
	wchar_t *p, *q, *str;

	if (xNode == nullptr || XmlGetName(xNode) == nullptr || mir_wstrcmp(XmlGetName(xNode), L"x") || hwndStatic == nullptr)
		return nullptr;

	hFrame = hwndStatic;
	id = 0;
	XmlNode x(L"x");
	x << XATTR(L"xmlns", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"submit");

	for (int i = 0;; i++) {
		n = XmlGetChild(xNode, i);
		if (!n)
			break;

		fieldStr = nullptr;
		if (mir_wstrcmp(XmlGetName(n), L"field"))
			continue;

		if ((varName = XmlGetAttrValue(n, L"var")) == nullptr || (type = XmlGetAttrValue(n, L"type")) == nullptr)
			continue;

		hCtrl = GetDlgItem(hFrame, id);
		HXML field = x << XCHILD(L"field") << XATTR(L"var", varName);

		if (!mir_wstrcmp(type, L"text-multi") || !mir_wstrcmp(type, L"jid-multi")) {
			len = GetWindowTextLength(GetDlgItem(hFrame, id));
			str = (wchar_t*)mir_alloc(sizeof(wchar_t)*(len + 1));
			GetDlgItemText(hFrame, id, str, len + 1);
			p = str;
			while (p != nullptr) {
				if ((q = wcsstr(p, L"\r\n")) != nullptr)
					*q = '\0';
				field << XCHILD(L"value", p);
				p = q ? q + 2 : nullptr;
			}
			mir_free(str);
			id++;
		}
		else if (!mir_wstrcmp(type, L"boolean")) {
			wchar_t buf[10];
			_itow(IsDlgButtonChecked(hFrame, id) == BST_CHECKED ? 1 : 0, buf, 10);
			field << XCHILD(L"value", buf);
			id++;
		}
		else if (!mir_wstrcmp(type, L"list-single")) {
			len = GetWindowTextLength(GetDlgItem(hFrame, id));
			str = (wchar_t*)mir_alloc(sizeof(wchar_t)*(len + 1));
			GetDlgItemText(hFrame, id, str, len + 1);
			v = nullptr;
			for (j = 0;; j++) {
				o = XmlGetChild(n, j);
				if (!o)
					break;

				if (!mir_wstrcmp(XmlGetName(o), L"option")) {
					if ((v = XmlGetChild(o, "value")) != nullptr && XmlGetText(v)) {
						if ((str2 = XmlGetAttrValue(o, L"label")) == nullptr)
							str2 = XmlGetText(v);
						if (!mir_wstrcmp(str2, str))
							break;
					}
				}
			}

			if (o)
				field << XCHILD(L"value", XmlGetText(v));

			mir_free(str);
			id++;
		}
		else if (!mir_wstrcmp(type, L"list-multi")) {
			int count = SendMessage(hCtrl, LB_GETCOUNT, 0, 0);
			for (j = 0; j < count; j++) {
				if (SendMessage(hCtrl, LB_GETSEL, j, 0) > 0) {
					// an entry is selected
					len = SendMessage(hCtrl, LB_GETTEXTLEN, j, 0);
					if ((str = (wchar_t*)mir_alloc((len + 1)*sizeof(wchar_t))) != nullptr) {
						SendMessage(hCtrl, LB_GETTEXT, j, (LPARAM)str);
						for (k = 0;; k++) {
							o = XmlGetChild(n, k);
							if (!o)
								break;

							if (XmlGetName(o) && !mir_wstrcmp(XmlGetName(o), L"option")) {
								if ((v = XmlGetChild(o, "value")) != nullptr && XmlGetText(v)) {
									if ((labelText = XmlGetAttrValue(o, L"label")) == nullptr)
										labelText = XmlGetText(v);

									if (!mir_wstrcmp(labelText, str))
										field << XCHILD(L"value", XmlGetText(v));
								}
							}
						}
						mir_free(str);
					}
				}
			}
			id++;
		}
		else if (!mir_wstrcmp(type, L"fixed") || !mir_wstrcmp(type, L"hidden")) {
			v = XmlGetChild(n, "value");
			if (v != nullptr && XmlGetText(v) != nullptr)
				field << XCHILD(L"value", XmlGetText(v));
		}
		else { // everything else is considered "text-single" or "text-private"
			len = GetWindowTextLength(GetDlgItem(hFrame, id));
			str = (wchar_t*)mir_alloc(sizeof(wchar_t)*(len + 1));
			GetDlgItemText(hFrame, id, str, len + 1);
			field << XCHILD(L"value", str);
			mir_free(str);
			id++;
		}
	}

	return xmlCopyNode(x);
}

struct JABBER_FORM_INFO
{
	~JABBER_FORM_INFO();

	CJabberProto *ppro;
	HXML xNode;
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
			HXML n;
			LONG frameExStyle;
			// Set dialog title
			if (jfi->xNode != nullptr && (n = XmlGetChild(jfi->xNode, L"title")) != nullptr && XmlGetText(n) != nullptr)
				SetWindowText(hwndDlg, XmlGetText(n));
			else
				SetWindowText(hwndDlg, TranslateW(jfi->defTitle));
			
			// Set instruction field
			if (jfi->xNode != nullptr && (n = XmlGetChild(jfi->xNode, L"instructions")) != nullptr && XmlGetText(n) != nullptr)
				JabberFormSetInstruction(hwndDlg, XmlGetText(n));
			else {
				if (jfi->xNode != nullptr && (n = XmlGetChild(jfi->xNode, L"title")) != nullptr && XmlGetText(n) != nullptr)
					JabberFormSetInstruction(hwndDlg, XmlGetText(n));
				else
					JabberFormSetInstruction(hwndDlg, TranslateW(jfi->defTitle));
			}

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
				HXML n = JabberFormGetData(GetDlgItem(hwndDlg, IDC_FRAME), jfi->xNode);
				(jfi->ppro->*(jfi->pfnSubmit))(n, jfi->userdata);
				xmlDestroyNode(n);
			}
			// fall through
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
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FORM), nullptr, JabberFormDlgProc, (LPARAM)param);
}

void CJabberProto::FormCreateDialog(HXML xNode, wchar_t* defTitle, JABBER_FORM_SUBMIT_FUNC pfnSubmit, void *userdata)
{
	JABBER_FORM_INFO *jfi = new JABBER_FORM_INFO;
	memset(jfi, 0, sizeof(JABBER_FORM_INFO));
	jfi->ppro = this;
	jfi->xNode = xmlCopyNode(xNode);
	if (defTitle)
		wcsncpy_s(jfi->defTitle, defTitle, _TRUNCATE);
	jfi->pfnSubmit = pfnSubmit;
	jfi->userdata = userdata;

	CallFunctionAsync(JabberFormCreateDialogApcProc, jfi);
}

//=======================================================================================

JABBER_FORM_INFO::~JABBER_FORM_INFO()
{
	xmlDestroyNode(xNode);
	mir_free(userdata);
}
