/*
Miranda SmileyAdd Plugin
Copyright (C) 2008 - 2011 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "general.h"

struct RichEditData
{
	HWND hwnd;
	MCONTACT hContact;
	HWND hToolTip;
	int tipActive;
	bool inputarea;
	bool dontReplace;
};

struct RichEditOwnerData
{
	HWND hwnd;
	HWND hwndInput;
	HWND hwndLog;
};

static int CompareRichEditData(const RichEditData* p1, const RichEditData* p2)
{
	return (int)((INT_PTR)p1->hwnd - (INT_PTR)p2->hwnd);
}

static LIST<RichEditData> g_RichEditList(10, CompareRichEditData);

static int CompareRichEditData(const RichEditOwnerData* p1, const RichEditOwnerData* p2)
{
	return (int)((INT_PTR)p1->hwnd - (INT_PTR)p2->hwnd);
}

static LIST<RichEditOwnerData> g_RichEditOwnerList(5, CompareRichEditData);

static void SetPosition(HWND hwnd)
{
	IRichEditOle* RichEditOle;
	if (SendMessage(hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle) == 0)
		return;

	ITextDocument* TextDocument;
	if (RichEditOle->QueryInterface(IID_ITextDocument, (void**)&TextDocument) != S_OK) {
		RichEditOle->Release();
		return;
	}

	// retrieve text range
	ITextRange* TextRange;
	if (TextDocument->Range(0, 0, &TextRange) != S_OK) {
		TextDocument->Release();
		RichEditOle->Release();
		return;
	}
	TextDocument->Release();

	int objectCount = RichEditOle->GetObjectCount();
	for (int i = objectCount - 1; i >= 0; i--) {
		REOBJECT reObj = {0};
		reObj.cbStruct  = sizeof(REOBJECT);

		HRESULT hr = RichEditOle->GetObject(i, &reObj, REO_GETOBJ_POLEOBJ);
		if (FAILED(hr)) continue;

		ISmileyBase *igsc = NULL;
		if (reObj.clsid == CLSID_NULL) 
			reObj.poleobj->QueryInterface(IID_ISmileyAddSmiley, (void**) &igsc);

		reObj.poleobj->Release();
		if (igsc == NULL) continue;

		TextRange->SetRange(reObj.cp, reObj.cp);

		BOOL res;
		POINT pt;
		RECT rect;
		hr = TextRange->GetPoint(tomStart | TA_BOTTOM | TA_LEFT, &pt.x, &pt.y);
		if (hr == S_OK) {
			res = ScreenToClient(hwnd, &pt);
			rect.bottom = pt.y;
			rect.left = pt.x;
		}
		else rect.bottom = -1;

		hr = TextRange->GetPoint(tomStart | TA_TOP | TA_LEFT, &pt.x, &pt.y);
		if (hr == S_OK) {
			res = ScreenToClient(hwnd, &pt);
			rect.top = pt.y;
			rect.left = pt.x;
		}
		else rect.top = -1;

		igsc->SetPosition(hwnd, &rect);
		igsc->Release();
	}
	TextRange->Release();
	RichEditOle->Release();
}

static void SetTooltip(long x, long y, HWND hwnd, RichEditData* rdt)
{
	TCHAR* smltxt;
	int needtip = CheckForTip(x, y, hwnd, &smltxt);
	if (needtip == rdt->tipActive)
		return;

	TOOLINFO ti = { sizeof(ti) };
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.hwnd = hwnd;
	ti.uId = (UINT_PTR)ti.hwnd;

	if (needtip != -1) {
		if (rdt->tipActive == -1) {
			rdt->hToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, _T(""), TTS_NOPREFIX | WS_POPUP, 
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, g_hInst, NULL);

			SendMessage(rdt->hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
		}

		ti.lpszText = smltxt; 
		SendMessage(rdt->hToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
		SendMessage(rdt->hToolTip, TTM_ACTIVATE, TRUE, 0);
	}
	else if (rdt->tipActive != -1) {
		SendMessage(rdt->hToolTip, TTM_ACTIVATE, FALSE, 0);
		DestroyWindow(rdt->hToolTip);
		rdt->hToolTip = NULL;
	}

	rdt->tipActive = needtip;
}

static const CHARRANGE allsel = { 0, LONG_MAX };

static void ReplaceContactSmileys(RichEditData *rdt, const CHARRANGE &sel, bool ignoreLast, bool unFreeze)
{
	if ((rdt->inputarea && !opt.InputSmileys) || rdt->dontReplace) return;
	SmileyPackCType *smcp = NULL;
	SmileyPackType* SmileyPack = GetSmileyPack(NULL, rdt->hContact, rdt->inputarea ? NULL : &smcp);
	ReplaceSmileys(rdt->hwnd, SmileyPack, smcp, sel, false, ignoreLast, unFreeze);
}

static void ReplaceContactSmileysWithText(RichEditData *rdt, CHARRANGE &sel, bool freeze)
{
	if ((rdt->inputarea && !opt.InputSmileys) || rdt->dontReplace) return;
	ReplaceSmileysWithText(rdt->hwnd, sel, freeze);
}

static void SmileyToTextCutPrep(RichEditData* rdt)
{
	if ((rdt->inputarea && !opt.InputSmileys) || rdt->dontReplace)
		return;

	SendMessage(rdt->hwnd, WM_SETREDRAW, FALSE, 0);
	CHARRANGE sel;
	SendMessage(rdt->hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
	ReplaceContactSmileysWithText(rdt, sel, true);
}

static void SmileyToTextCutRest(RichEditData* rdt)
{
	if ((rdt->inputarea && !opt.InputSmileys) || rdt->dontReplace)
		return;

	CHARRANGE sel;
	SendMessage(rdt->hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
	ReplaceContactSmileys(rdt, sel, false, true);
	SendMessage(rdt->hwnd, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(rdt->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

static LRESULT CALLBACK RichEditSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RichEditData* rdt = g_RichEditList.find((RichEditData*)&hwnd);
	if (rdt == NULL)
		return 0;

	CHARRANGE sel;

	switch(uMsg) {
	case WM_COPY:
	case WM_CUT:
		SmileyToTextCutPrep(rdt);
		break;

	case WM_PAINT:
		SetPosition(hwnd);
		break;

	case EM_STREAMOUT:
		if (wParam & SFF_SELECTION)
			SmileyToTextCutPrep(rdt);
		else {
			sel = allsel;
			ReplaceContactSmileysWithText(rdt, sel, true);
		}
		break;

	case WM_KEYDOWN:
		if ((wParam == 'C' || wParam == VK_INSERT) && (GetKeyState(VK_CONTROL) & 0x8000))
			SmileyToTextCutPrep(rdt);
		else if ((wParam == 'X' && (GetKeyState(VK_CONTROL) & 0x8000)) || (wParam == VK_DELETE && (GetKeyState(VK_SHIFT) & 0x8000)))
			SmileyToTextCutPrep(rdt);
		else if (wParam == VK_TAB && ((GetKeyState(VK_CONTROL) | GetKeyState(VK_SHIFT)) & 0x8000) == 0) {
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			sel.cpMin = max(sel.cpMin - 20, 0);

			ReplaceContactSmileysWithText(rdt, sel, true);
		}
		break;
	}

	LRESULT result = mir_callNextSubclass(hwnd, RichEditSubclass, uMsg, wParam, lParam); 

	switch(uMsg) {
	case WM_MOUSEMOVE:
		SetTooltip(LOWORD(lParam), HIWORD(lParam), hwnd, rdt);
		break;

	case WM_PAINT:
	case WM_HSCROLL:
	case WM_VSCROLL:
		SetPosition(hwnd);
		break;

	case WM_COPY:
	case WM_CUT:
		SmileyToTextCutRest(rdt);
		break;

	case EM_STREAMOUT:
		if (wParam & SFF_SELECTION)
			SmileyToTextCutRest(rdt);
		else
			ReplaceContactSmileys(rdt, allsel, false, true);
		break;

	case WM_KEYDOWN:
		if ((wParam == 'C' || wParam == VK_INSERT) && (GetKeyState(VK_CONTROL) & 0x8000))
			SmileyToTextCutRest(rdt);

		else if ((wParam == 'X' && (GetKeyState(VK_CONTROL) & 0x8000)) || (wParam == VK_DELETE && (GetKeyState(VK_SHIFT) & 0x8000)))
			SmileyToTextCutRest(rdt);

		else if (wParam == VK_TAB && ((GetKeyState(VK_CONTROL) | GetKeyState(VK_SHIFT)) & 0x8000) == 0) {
			sel.cpMax = LONG_MAX;
			bool hascont = rdt->hContact != NULL; 
			ReplaceContactSmileys(rdt, sel, false, hascont);
		}
		break;

	case WM_CHAR:
		if (!rdt->inputarea || (rdt->inputarea && !opt.InputSmileys)) 
			break;

		if (lParam & (1 << 28))	// ALT key
			break;

		if ((lParam & 0xFF) > 2)	// Repeat rate
			break;

		if (wParam > ' ' && opt.EnforceSpaces)
			break;

		if (wParam == 0x16) {
			ReplaceContactSmileys(rdt, allsel, false, false);
			break;
		}

		if (opt.DCursorSmiley)
			ReplaceContactSmileys(rdt, allsel, true, true);
		else if (wParam >= ' ' || wParam == '\n' || wParam == '\r') {
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			sel.cpMin = max(sel.cpMin - 20, 0);
			sel.cpMax += 20;

			ReplaceContactSmileysWithText(rdt, sel, true);
			ReplaceContactSmileys(rdt, sel, false, true);
		}
		break;

	case EM_PASTESPECIAL:
	case WM_PASTE:
	case EM_REPLACESEL:
	case WM_SETTEXT:
	case EM_SETTEXTEX:
		if (rdt->inputarea)
			ReplaceContactSmileys(rdt, allsel, false, false);
		break;

	case WM_REMAKERICH:
		ReplaceContactSmileys(rdt, allsel, false, false);
		break;
	}

	return result;
}

bool SetRichCallback(HWND hwnd, MCONTACT hContact, bool subany, bool subnew)
{
	RichEditData* rdt = g_RichEditList.find((RichEditData*)&hwnd);
	if (rdt == NULL) {
		IRichEditOle* RichEditOle;
		if (SendMessage(hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle) == 0)
			return false;
		RichEditOle->Release();

		rdt = new RichEditData;

		rdt->hwnd = hwnd;
		rdt->hContact = hContact;
		rdt->inputarea = (GetWindowLongPtr(hwnd, GWL_STYLE) & ES_READONLY) == 0;
		rdt->dontReplace = false;
		rdt->tipActive = -1;
		rdt->hToolTip = NULL;
		g_RichEditList.insert(rdt);

		if (subnew)
			mir_subclassWindow(hwnd, RichEditSubclass);
	}
	else {
		if (hContact && !rdt->hContact)
			rdt->hContact = hContact; 

		if (subany) 
			mir_subclassWindow(hwnd, RichEditSubclass);
	}
	return true;
}

void CloseRichCallback(HWND hwnd)
{
	int ind = g_RichEditList.getIndex((RichEditData*)&hwnd);
	if (ind == -1 )
		return;

	RichEditData* rdt = g_RichEditList[ind];
	if (rdt->hToolTip)
		DestroyWindow(rdt->hToolTip);
	delete rdt;
	g_RichEditList.remove(ind);
	mir_unsubclassWindow(hwnd, RichEditSubclass);
}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK RichEditOwnerSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RichEditOwnerData* rdto = g_RichEditOwnerList.find((RichEditOwnerData*)&hwnd);
	if (rdto == NULL)
		return 0;

	switch(uMsg) {
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == 1624) {
			RichEditData* rdt = g_RichEditList.find((RichEditData*)&rdto->hwndInput);
			if (rdt && (!rdt->inputarea || opt.InputSmileys)) {
				rdt->dontReplace = true;
				CHARRANGE sel = allsel;
				ReplaceSmileysWithText(rdt->hwnd, sel, false);
			}
		}
		break;

	case WM_DESTROY:
		RichEditData* rdt = g_RichEditList.find((RichEditData*)&rdto->hwndInput);
		if (rdt && (!rdt->inputarea || opt.InputSmileys)) {
			CHARRANGE sel = allsel;
			rdt->dontReplace = true;
			ReplaceSmileysWithText(rdt->hwnd, sel, false);
		}
		break;
	}

	LRESULT result = mir_callNextSubclass(hwnd, RichEditOwnerSubclass, uMsg, wParam, lParam); 

	switch(uMsg) {
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == 1624) {
			RichEditData* rdt = g_RichEditList.find((RichEditData*)&rdto->hwndInput);
			if (rdt) {
				CHARRANGE sel = allsel;
				if (!result) ReplaceContactSmileys(rdt, sel, false, false);
				rdt->dontReplace = false;
			}
		}
		break;
	}
	return result;
}

void SetRichOwnerCallback(HWND hwnd, HWND hwndInput, HWND hwndLog)
{
	RichEditOwnerData* rdto = g_RichEditOwnerList.find((RichEditOwnerData*)&hwnd);
	if (rdto == NULL) {
		rdto = new RichEditOwnerData;
		rdto->hwnd = hwnd;
		rdto->hwndInput = hwndInput;
		rdto->hwndLog = hwndLog;
		g_RichEditOwnerList.insert(rdto);

		if (hwndLog)
			SendMessage(hwndLog, EM_SETUNDOLIMIT, 0, 0);

		mir_subclassWindow(hwnd, RichEditOwnerSubclass);
	}
	else {
		if (rdto->hwndInput == NULL) rdto->hwndInput = hwndInput;
		if (rdto->hwndLog == NULL) rdto->hwndLog = hwndLog;
	}
}

void CloseRichOwnerCallback(HWND hwnd)
{
	int ind = g_RichEditOwnerList.getIndex((RichEditOwnerData*)&hwnd);
	if (ind == -1)
		return;

	RichEditOwnerData* rdto = g_RichEditOwnerList[ind];
	CloseRichCallback(rdto->hwndInput);
	CloseRichCallback(rdto->hwndLog);
	delete rdto;

	g_RichEditOwnerList.remove(ind);

	mir_unsubclassWindow(hwnd, RichEditOwnerSubclass);
}

void ProcessAllInputAreas(bool restoreText)
{
	for (int i=g_RichEditList.getCount()-1; i >= 0; i--) {
		RichEditData* rdt = g_RichEditList[i];
		if (rdt->inputarea) {
			if (restoreText) {
				CHARRANGE sel = allsel;
				ReplaceContactSmileysWithText(rdt, sel, false);
			}
			else ReplaceContactSmileys(rdt, allsel, false, false);
		}
	}
}

void  RichEditData_Destroy(void)
{
	int i;
	for (i=g_RichEditList.getCount()-1; i >= 0; i--) 
		CloseRichCallback(g_RichEditList[i]->hwnd);
	g_RichEditList.destroy();

	for (i=g_RichEditOwnerList.getCount()-1; i >= 0; i--) 
		CloseRichOwnerCallback(g_RichEditOwnerList[i]->hwnd);
	g_RichEditOwnerList.destroy();
}
