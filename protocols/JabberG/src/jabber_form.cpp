/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2012-14  Miranda NG project

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
#include "jabber_caps.h"

static LRESULT CALLBACK JabberFormMultiLineWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	//case WM_GETDLGCODE:
	//	return DLGC_WANTARROWS|DLGC_WANTCHARS|DLGC_HASSETSEL|DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		if (wParam == VK_TAB) {
			SetFocus(GetNextDlgTabItem(GetParent(GetParent(hwnd)), hwnd, GetKeyState(VK_SHIFT)<0?TRUE:FALSE));
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
	int minX;
	GetWindowRect(hwndStatic,&rcWindow);
	minX=rcWindow.right;
	HWND oldChild=NULL;
	HWND hWndChild=GetWindow(hwndStatic,GW_CHILD);
	while (hWndChild!=oldChild && hWndChild != NULL)
	{
	   DWORD style=GetWindowLongPtr(hWndChild, GWL_STYLE);
	   RECT rc;
	   GetWindowRect(hWndChild,&rc);
	   if ((style&SS_RIGHT) && !(style&WS_TABSTOP))
	   {
		  TCHAR * text;
		  RECT calcRect=rc;
		  int len=GetWindowTextLength(hWndChild);
		  text=(TCHAR*)malloc(sizeof(TCHAR)*(len+1));
		  GetWindowText(hWndChild,text,len+1);
		  HDC hdc=GetDC(hWndChild);
		  HFONT hfntSave = (HFONT)SelectObject(hdc, (HFONT)SendMessage(hWndChild, WM_GETFONT, 0, 0));
		  DrawText(hdc,text,-1,&calcRect,DT_CALCRECT|DT_WORDBREAK);
		  minX=min(minX, rc.right-(calcRect.right-calcRect.left));
		  SelectObject(hdc, hfntSave);
		  ReleaseDC(hWndChild,hdc);
	   }
	   else
	   {
		  minX=min(minX,rc.left);
	   }
	   oldChild=hWndChild;
	   hWndChild=GetWindow(hWndChild,GW_HWNDNEXT);
	}
	if (minX>rcWindow.left+5)
	{
		int dx=(minX-rcWindow.left)/2;
		oldChild=NULL;
		hWndChild=GetWindow(hwndStatic,GW_CHILD);
		while (hWndChild!=oldChild && hWndChild != NULL)
		{
			DWORD style=GetWindowLongPtr(hWndChild, GWL_STYLE);
			RECT rc;
			GetWindowRect(hWndChild,&rc);
			if ((style&SS_RIGHT) && !(style&WS_TABSTOP))
				MoveWindow(hWndChild,rc.left-rcWindow.left,rc.top-rcWindow.top, rc.right-rc.left-dx, rc.bottom-rc.top, TRUE);
			else
				MoveWindow(hWndChild,rc.left-dx-rcWindow.left,rc.top-rcWindow.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);
			oldChild=hWndChild;
			hWndChild=GetWindow(hWndChild,GW_HWNDNEXT);
		}
	}
}

void JabberFormSetInstruction(HWND hwndForm, const TCHAR *text)
{
	if (!text) text = _T("");

	int len = mir_tstrlen(text);
	int fixedLen = len;
	for (int i = 1; i < len; i++)
		if ((text[i - 1] == _T('\n')) && (text[i] != _T('\r')))
			++fixedLen;
	TCHAR *fixedText = NULL;
	if (fixedLen != len) {
		fixedText = (TCHAR *)mir_alloc(sizeof(TCHAR) * (fixedLen+1));
		TCHAR *p = fixedText;
		for (int i=0; i < len; i++) {
			*p = text[i];
			if (i && (text[i] == _T('\n')) && (text[i] != _T('\r'))) {
				*p++ = _T('\r');
				*p = _T('\n');
			}
			++p;
		}
		*p = 0;
		text = fixedText;
	}

	SetDlgItemText(hwndForm, IDC_INSTRUCTION, text);

	RECT rcText;
	GetWindowRect(GetDlgItem(hwndForm, IDC_INSTRUCTION), &rcText);
	int oldWidth = rcText.right-rcText.left;
	int deltaHeight = -(rcText.bottom-rcText.top);

	SetRect(&rcText, 0, 0, rcText.right-rcText.left, 0);
	HDC hdcEdit = GetDC(GetDlgItem(hwndForm, IDC_INSTRUCTION));
	HFONT hfntSave = (HFONT)SelectObject(hdcEdit, (HFONT)SendDlgItemMessage(hwndForm, IDC_INSTRUCTION, WM_GETFONT, 0, 0));
	DrawTextEx(hdcEdit, (TCHAR *)text, mir_tstrlen(text), &rcText,
		DT_CALCRECT|DT_EDITCONTROL|DT_TOP|DT_WORDBREAK, NULL);
	SelectObject(hdcEdit, hfntSave);
	ReleaseDC(GetDlgItem(hwndForm, IDC_INSTRUCTION), hdcEdit);

	RECT rcWindow; GetClientRect(hwndForm, &rcWindow);
	if (rcText.bottom-rcText.top > (rcWindow.bottom-rcWindow.top)/5) {
		HWND hwndEdit = GetDlgItem(hwndForm, IDC_INSTRUCTION);
		SetWindowLongPtr(hwndEdit, GWL_STYLE, WS_VSCROLL | GetWindowLongPtr(hwndEdit, GWL_STYLE));
		rcText.bottom = rcText.top + (rcWindow.bottom-rcWindow.top)/5;
	}
	else {
		HWND hwndEdit = GetDlgItem(hwndForm, IDC_INSTRUCTION);
		SetWindowLongPtr(hwndEdit, GWL_STYLE, ~WS_VSCROLL & GetWindowLongPtr(hwndEdit, GWL_STYLE));
	}
	deltaHeight += rcText.bottom-rcText.top;

	SetWindowPos(GetDlgItem(hwndForm, IDC_INSTRUCTION), 0, 0, 0,
		oldWidth,
		rcText.bottom-rcText.top,
		SWP_NOMOVE|SWP_NOZORDER);

	GetWindowRect(GetDlgItem(hwndForm, IDC_WHITERECT), &rcText);
	MapWindowPoints(NULL, hwndForm, (LPPOINT)&rcText, 2);
	rcText.bottom += deltaHeight;
	SetWindowPos(GetDlgItem(hwndForm, IDC_WHITERECT), 0, 0, 0,
		rcText.right-rcText.left,
		rcText.bottom-rcText.top,
		SWP_NOMOVE|SWP_NOZORDER);

	GetWindowRect(GetDlgItem(hwndForm, IDC_FRAME1), &rcText);
	MapWindowPoints(NULL, hwndForm, (LPPOINT)&rcText, 2);
	rcText.top += deltaHeight;
	SetWindowPos(GetDlgItem(hwndForm, IDC_FRAME1), 0,
		rcText.left,
		rcText.top,
		0, 0,
		SWP_NOSIZE|SWP_NOZORDER);

	GetWindowRect(GetDlgItem(hwndForm, IDC_FRAME), &rcText);
	MapWindowPoints(NULL, hwndForm, (LPPOINT)&rcText, 2);
	rcText.top += deltaHeight;
	SetWindowPos(GetDlgItem(hwndForm, IDC_FRAME), 0,
		rcText.left,
		rcText.top,
		rcText.right-rcText.left,
		rcText.bottom-rcText.top,
		SWP_NOZORDER);

	GetWindowRect(GetDlgItem(hwndForm, IDC_VSCROLL), &rcText);
	MapWindowPoints(NULL, hwndForm, (LPPOINT)&rcText, 2);
	rcText.top += deltaHeight;
	SetWindowPos(GetDlgItem(hwndForm, IDC_VSCROLL), 0,
		rcText.left,
		rcText.top,
		rcText.right-rcText.left,
		rcText.bottom-rcText.top,
		SWP_NOZORDER);

	if (fixedText) mir_free(fixedText);
}

static TJabberFormControlType JabberFormTypeNameToId(const TCHAR *type)
{
	if (!_tcscmp(type, _T("text-private")))
		return JFORM_CTYPE_TEXT_PRIVATE;
	if (!_tcscmp(type, _T("text-multi")) || !_tcscmp(type, _T("jid-multi")))
		return JFORM_CTYPE_TEXT_MULTI;
	if (!_tcscmp(type, _T("boolean")))
		return JFORM_CTYPE_BOOLEAN;
	if (!_tcscmp(type, _T("list-single")))
		return JFORM_CTYPE_LIST_SINGLE;
	if (!_tcscmp(type, _T("list-multi")))
		return JFORM_CTYPE_LIST_MULTI;
	if (!_tcscmp(type, _T("fixed")))
		return JFORM_CTYPE_FIXED;
	if (!_tcscmp(type, _T("hidden")))
		return JFORM_CTYPE_HIDDEN;

	return JFORM_CTYPE_TEXT_SINGLE;
}

void JabberFormLayoutSingleControl(TJabberFormControlInfo *item, TJabberFormLayoutInfo *layout_info, const TCHAR *labelStr, const TCHAR *valueStr)
{
	RECT rcLabel = {0}, rcCtrl = {0};
	if (item->hLabel)
	{
		SetRect(&rcLabel, 0, 0, layout_info->width, 0);
		HDC hdc = GetDC(item->hLabel);
		HFONT hfntSave = (HFONT)SelectObject(hdc, (HFONT)SendMessage(item->hLabel, WM_GETFONT, 0, 0));
		DrawText(hdc, labelStr, -1, &rcLabel, DT_CALCRECT|DT_WORDBREAK);
		SelectObject(hdc, hfntSave);
		ReleaseDC(item->hLabel, hdc);
	}

	int indent = layout_info->compact ? 10 : 20;

	if ((layout_info->compact && (item->type != JFORM_CTYPE_BOOLEAN) && (item->type != JFORM_CTYPE_FIXED))||
		(rcLabel.right >= layout_info->maxLabelWidth) ||
		(rcLabel.bottom > layout_info->ctrlHeight) ||
		(item->type == JFORM_CTYPE_LIST_MULTI) ||
		(item->type == JFORM_CTYPE_TEXT_MULTI))
	{
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
	CreateWindow(_T("static"), labelStr, WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE, \
		0, 0, 0, 0, hwndStatic, (HMENU)-1, hInst, NULL)

TJabberFormControlInfo *JabberFormAppendControl(HWND hwndStatic, TJabberFormLayoutInfo *layout_info, TJabberFormControlType type, const TCHAR *labelStr, const TCHAR *valueStr)
{
	TJabberFormControlList *controls = (TJabberFormControlList *)GetWindowLongPtr(hwndStatic, GWLP_USERDATA);
	if (!controls) {
		controls = new TJabberFormControlList(5);
		SetWindowLongPtr(hwndStatic, GWLP_USERDATA, (LONG_PTR)controls);
	}

	TJabberFormControlInfo *item = (TJabberFormControlInfo *)mir_alloc(sizeof(TJabberFormControlInfo));
	item->type = type;
	item->hLabel = item->hCtrl = NULL;

	switch (type) {
	case JFORM_CTYPE_TEXT_PRIVATE:
		item->hLabel = JabberFormCreateLabel();
		item->hCtrl = CreateWindowEx(WS_EX_CLIENTEDGE, _T("edit"), valueStr,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | ES_PASSWORD,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, hInst, NULL);
		++layout_info->id;
		break;

	case JFORM_CTYPE_TEXT_MULTI:
		item->hLabel = JabberFormCreateLabel();
		item->hCtrl = CreateWindowEx(WS_EX_CLIENTEDGE, _T("edit"), valueStr,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, hInst, NULL);
		mir_subclassWindow(item->hCtrl, JabberFormMultiLineWndProc);
		++layout_info->id;
		break;

	case JFORM_CTYPE_BOOLEAN:
		item->hCtrl = CreateWindowEx(0, _T("button"), labelStr,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX | BS_MULTILINE,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, hInst, NULL);
		if (valueStr && !_tcscmp(valueStr, _T("1")))
			SendMessage(item->hCtrl, BM_SETCHECK, 1, 0);
		++layout_info->id;
		break;

	case JFORM_CTYPE_LIST_SINGLE:
		item->hLabel = JabberFormCreateLabel();
		item->hCtrl = CreateWindowExA(WS_EX_CLIENTEDGE, "combobox", NULL,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, hInst, NULL);
		++layout_info->id;
		break;

	case JFORM_CTYPE_LIST_MULTI:
		item->hLabel = JabberFormCreateLabel();
		item->hCtrl = CreateWindowExA(WS_EX_CLIENTEDGE, "listbox",
			NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | LBS_MULTIPLESEL,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, hInst, NULL);
		++layout_info->id;
		break;

	case JFORM_CTYPE_FIXED:
		item->hCtrl = CreateWindow(_T("edit"), valueStr,
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL,
			0, 0, 0, 0,
			hwndStatic, (HMENU)-1, hInst, NULL);
		break;

	case JFORM_CTYPE_HIDDEN:
		break;

	case JFORM_CTYPE_TEXT_SINGLE:
		item->hLabel = labelStr ? (JabberFormCreateLabel()) : NULL;
		item->hCtrl = CreateWindowEx(WS_EX_CLIENTEDGE, _T("edit"), valueStr,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,
			0, 0, 0, 0,
			hwndStatic, (HMENU)layout_info->id, hInst, NULL);
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

void JabberFormAddListItem(TJabberFormControlInfo *item, const TCHAR *text, bool selected)
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

	const TCHAR *label, *typeName, *varStr, *str, *valueText;
	TCHAR *labelStr, *valueStr;
	RECT frameRect;

	if (xNode == NULL || xmlGetName(xNode) == NULL || mir_tstrcmp(xmlGetName(xNode), _T("x")) || hwndStatic == NULL)
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
		HXML n = xmlGetChild(xNode, i);
		if (!n)
			break;

		if (mir_tstrcmp(xmlGetName(n), _T("field")))
			continue;
				
		varStr = xmlGetAttrValue(n, _T("var"));
		if ((typeName = xmlGetAttrValue(n, _T("type"))) == NULL)
			continue;

		if ((label = xmlGetAttrValue(n, _T("label"))) != NULL)
			labelStr = mir_tstrdup(label);
		else
			labelStr = mir_tstrdup(varStr);

		TJabberFormControlType type = JabberFormTypeNameToId(typeName);

		if ((v = xmlGetChild(n, "value")) != NULL) {
			valueText = xmlGetText(v);
			if (type != JFORM_CTYPE_TEXT_MULTI)
				valueStr = mir_tstrdup(valueText);
			else {
				size_t size = 1;
				for (int j = 0;; j++) {
					v = xmlGetChild(n, j);
					if (!v)
						break;
					if (xmlGetName(v) && !mir_tstrcmp(xmlGetName(v), _T("value")) && xmlGetText(v))
						size += _tcslen(xmlGetText(v)) + 2;
				}
				valueStr = (TCHAR*)mir_alloc(sizeof(TCHAR)*size);
				valueStr[0] = '\0';
				for (int j = 0;; j++) {
					v = xmlGetChild(n, j);
					if (!v)
						break;
					if (xmlGetName(v) && !mir_tstrcmp(xmlGetName(v), _T("value")) && xmlGetText(v)) {
						if (valueStr[0])
							_tcscat(valueStr, _T("\r\n"));
						_tcscat(valueStr, xmlGetText(v));
					}
				}
			}
		}
		else valueText = valueStr = NULL;

		TJabberFormControlInfo *item = JabberFormAppendControl(hwndStatic, &layout_info, type, labelStr, valueStr);

		mir_free(labelStr);
		mir_free(valueStr);

		if (type == JFORM_CTYPE_LIST_SINGLE) {
			for (int j = 0;; j++) {
				HXML o = xmlGetChild(n, j);
				if (o == NULL)
					break;
				
				if (mir_tstrcmp(xmlGetName(o), _T("option")))
					continue;
				if ((v = xmlGetChild(o, "value")) == NULL || xmlGetText(v) == NULL)
					continue;
				if ((str = xmlGetAttrValue(o, _T("label"))) == NULL)
					str = xmlGetText(v);
				if (str == NULL)
					continue;

				bool selected = !mir_tstrcmp(valueText, xmlGetText(v));
				JabberFormAddListItem(item, str, selected);
			}
		}
		else if (type == JFORM_CTYPE_LIST_MULTI) {
			for (int j = 0;; j++) {
				HXML o = xmlGetChild(n, j);
				if (o == NULL)
					break;

				if (mir_tstrcmp(xmlGetName(o), _T("option")))
					continue;
				if ((v = xmlGetChild(o, "value")) == NULL || xmlGetText(v) == NULL)
					continue;
				if ((str = xmlGetAttrValue(o, _T("label"))) == NULL)
					str = xmlGetText(v);
				if (str == NULL)
					continue;

				bool selected = false;
				for (int k = 0;; k++) {
					vs = xmlGetChild(n, k);
					if (!vs)
						break;
					if (!mir_tstrcmp(xmlGetName(vs), _T("value")) && !mir_tstrcmp(xmlGetText(vs), xmlGetText(v))) {
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
		for (int i=0; i < controls->getCount(); i++)
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
	const TCHAR *varName, *type, *fieldStr, *labelText, *str2;
	TCHAR *p, *q, *str;

	if (xNode == NULL || xmlGetName(xNode) == NULL || mir_tstrcmp(xmlGetName(xNode), _T("x")) || hwndStatic == NULL)
		return NULL;

	hFrame = hwndStatic;
	id = 0;
	XmlNode x(_T("x"));
	x << XATTR(_T("xmlns"), JABBER_FEAT_DATA_FORMS) << XATTR(_T("type"), _T("submit"));

	for (int i=0; ; i++) {
		n = xmlGetChild(xNode ,i);
		if (!n)
			break;

		fieldStr = NULL;
		if (mir_tstrcmp(xmlGetName(n), _T("field")))
			continue;

		if ((varName = xmlGetAttrValue(n, _T("var"))) == NULL || (type = xmlGetAttrValue(n, _T("type"))) == NULL)
			continue;

		hCtrl = GetDlgItem(hFrame, id);
		HXML field = x << XCHILD(_T("field")) << XATTR(_T("var"), varName);

		if (!_tcscmp(type, _T("text-multi")) || !_tcscmp(type, _T("jid-multi"))) {
			len = GetWindowTextLength(GetDlgItem(hFrame, id));
			str = (TCHAR*)mir_alloc(sizeof(TCHAR)*(len+1));
			GetDlgItemText(hFrame, id, str, len+1);
			p = str;
			while (p != NULL) {
				if ((q = _tcsstr(p, _T("\r\n"))) != NULL)
					*q = '\0';
				field << XCHILD(_T("value"), p);
				p = q ? q+2 : NULL;
			}
			mir_free(str);
			id++;
		}
		else if (!_tcscmp(type, _T("boolean"))) {
			TCHAR buf[10];
			_itot(IsDlgButtonChecked(hFrame, id) == BST_CHECKED ? 1 : 0, buf, 10);
			field << XCHILD(_T("value"), buf);
			id++;
		}
		else if (!_tcscmp(type, _T("list-single"))) {
			len = GetWindowTextLength(GetDlgItem(hFrame, id));
			str = (TCHAR*)mir_alloc(sizeof(TCHAR)*(len + 1));
			GetDlgItemText(hFrame, id, str, len+1);
			v = NULL;
			for (j=0; ; j++) {
				o = xmlGetChild(n,j);
				if (!o)
					break;

				if (!mir_tstrcmp(xmlGetName(o), _T("option"))) {
					if ((v = xmlGetChild(o , "value")) != NULL && xmlGetText(v)) {
						if ((str2 = xmlGetAttrValue(o, _T("label"))) == NULL)
							str2 = xmlGetText(v);
						if (!mir_tstrcmp(str2, str))
							break;
			}	}	}

			if (o)
				field << XCHILD(_T("value"), xmlGetText(v));

			mir_free(str);
			id++;
		}
		else if (!_tcscmp(type, _T("list-multi"))) {
			int count = SendMessage(hCtrl, LB_GETCOUNT, 0, 0);
			for (j=0; j<count; j++) {
				if (SendMessage(hCtrl, LB_GETSEL, j, 0) > 0) {
					// an entry is selected
					len = SendMessage(hCtrl, LB_GETTEXTLEN, j, 0);
					if ((str = (TCHAR*)mir_alloc((len+1)*sizeof(TCHAR))) != NULL) {
						SendMessage(hCtrl, LB_GETTEXT, j, (LPARAM)str);
						for (k=0; ; k++) {
							o = xmlGetChild(n,k);
							if (!o)
								break;

							if (xmlGetName(o) && !mir_tstrcmp(xmlGetName(o), _T("option"))) {
								if ((v = xmlGetChild(o , "value")) != NULL && xmlGetText(v)) {
									if ((labelText = xmlGetAttrValue(o, _T("label"))) == NULL)
										labelText = xmlGetText(v);

									if (!mir_tstrcmp(labelText, str))
										field << XCHILD(_T("value"), xmlGetText(v));
						}	}	}
						mir_free(str);
			}	}	}
			id++;
		}
		else if (!_tcscmp(type, _T("fixed")) || !_tcscmp(type, _T("hidden"))) {
			v = xmlGetChild(n, "value");
			if (v != NULL && xmlGetText(v) != NULL)
				field << XCHILD(_T("value"), xmlGetText(v));
		}
		else { // everything else is considered "text-single" or "text-private"
			len = GetWindowTextLength(GetDlgItem(hFrame, id));
			str = (TCHAR*)mir_alloc(sizeof(TCHAR)*(len+1));
			GetDlgItemText(hFrame, id, str, len+1);
			field << XCHILD(_T("value"), str);
			mir_free(str);
			id++;
	}	}

	return xi.copyNode(x);
}

struct JABBER_FORM_INFO
{
	~JABBER_FORM_INFO();

	CJabberProto *ppro;
	HXML xNode;
	TCHAR defTitle[128];	// Default title if no <title/> in xNode
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
		{
			HXML n;
			LONG frameExStyle;

			// lParam is (JABBER_FORM_INFO *)
			TranslateDialogDefault(hwndDlg);
			ShowWindow(GetDlgItem(hwndDlg, IDC_FRAME_TEXT), SW_HIDE);
			jfi = (JABBER_FORM_INFO*)lParam;
			if (jfi != NULL) {
				// Set dialog title
				if (jfi->xNode != NULL && (n = xmlGetChild(jfi->xNode , "title")) != NULL && xmlGetText(n) != NULL)
					SetWindowText(hwndDlg, xmlGetText(n));
				else if (jfi->defTitle != NULL)
					SetWindowText(hwndDlg, TranslateTS(jfi->defTitle));
				// Set instruction field
				if (jfi->xNode != NULL && (n = xmlGetChild(jfi->xNode , "instructions")) != NULL && xmlGetText(n) != NULL)
					JabberFormSetInstruction(hwndDlg, xmlGetText(n));
				else {
					if (jfi->xNode != NULL && (n = xmlGetChild(jfi->xNode, "title")) != NULL && xmlGetText(n) != NULL)
						JabberFormSetInstruction(hwndDlg, xmlGetText(n));
					else if (jfi->defTitle != NULL)
						JabberFormSetInstruction(hwndDlg, TranslateTS(jfi->defTitle));
				}

				// Create form
				if (jfi->xNode != NULL) {
					RECT rect;
					GetClientRect(GetDlgItem(hwndDlg, IDC_FRAME), &(jfi->frameRect));
					GetClientRect(GetDlgItem(hwndDlg, IDC_VSCROLL), &rect);
					jfi->frameRect.right -= (rect.right - rect.left);
					GetClientRect(GetDlgItem(hwndDlg, IDC_FRAME), &rect);
					jfi->frameHeight = rect.bottom - rect.top;
					JabberFormCreateUI(GetDlgItem(hwndDlg, IDC_FRAME), jfi->xNode, &(jfi->formHeight));
				}
			}

			if (jfi->formHeight > jfi->frameHeight) {
				HWND hwndScroll;

				hwndScroll = GetDlgItem(hwndDlg, IDC_VSCROLL);
				EnableWindow(hwndScroll, TRUE);
				SetScrollRange(hwndScroll, SB_CTL, 0, jfi->formHeight - jfi->frameHeight, FALSE);
				jfi->curPos = 0;
			}

			// Enable WS_EX_CONTROLPARENT on IDC_FRAME (so tab stop goes through all its children)
			frameExStyle = GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FRAME), GWL_EXSTYLE);
			frameExStyle |= WS_EX_CONTROLPARENT;
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FRAME), GWL_EXSTYLE, frameExStyle);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) jfi);
			if (jfi->pfnSubmit != NULL)
				EnableWindow(GetDlgItem(hwndDlg, IDC_SUBMIT), TRUE);
		}
		return TRUE;

	case WM_CTLCOLORSTATIC:
		if ((GetWindowLongPtr((HWND)lParam, GWL_ID) == IDC_WHITERECT) ||
			(GetWindowLongPtr((HWND)lParam, GWL_ID) == IDC_INSTRUCTION) ||
			(GetWindowLongPtr((HWND)lParam, GWL_ID) == IDC_TITLE))
		{
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
		}

		return NULL;

	case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (zDelta) {
				int nScrollLines=0;
				SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, (void*)&nScrollLines, 0);
				for (int i=0; i < (nScrollLines + 1) / 2; i++)
					SendMessage(hwndDlg, WM_VSCROLL, (zDelta < 0) ? SB_LINEDOWN : SB_LINEUP, 0);
			}
		}
		break;

	case WM_VSCROLL:
		if (jfi != NULL) {
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
				ScrollWindow(GetDlgItem(hwndDlg, IDC_FRAME), 0, jfi->curPos - pos, NULL, &(jfi->frameRect));
				SetScrollPos(GetDlgItem(hwndDlg, IDC_VSCROLL), SB_CTL, pos, TRUE);
				jfi->curPos = pos;
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SUBMIT:
			if (jfi != NULL) {
				HXML n = JabberFormGetData(GetDlgItem(hwndDlg, IDC_FRAME), jfi->xNode);
				(jfi->ppro->*(jfi->pfnSubmit))(n, jfi->userdata);
				xi.destroyNode(n);
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
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FORM), NULL, JabberFormDlgProc, (LPARAM)param);
}

void CJabberProto::FormCreateDialog(HXML xNode, TCHAR* defTitle, JABBER_FORM_SUBMIT_FUNC pfnSubmit, void *userdata)
{
	JABBER_FORM_INFO *jfi = new JABBER_FORM_INFO;
	memset(jfi, 0, sizeof(JABBER_FORM_INFO));
	jfi->ppro = this;
	jfi->xNode = xi.copyNode(xNode);
	if (defTitle)
		_tcsncpy_s(jfi->defTitle, defTitle, _TRUNCATE);
	jfi->pfnSubmit = pfnSubmit;
	jfi->userdata = userdata;

	CallFunctionAsync(JabberFormCreateDialogApcProc, jfi);
}

//=======================================================================================

JABBER_FORM_INFO::~JABBER_FORM_INFO()
{
	xi.destroyNode(xNode);
	mir_free(userdata);
}
