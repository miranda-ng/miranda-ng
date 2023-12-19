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

#include "stdafx.h"

struct RichEditData : public MZeroedObject
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

static mir_cs csLists;
static LIST<RichEditData> g_RichEditList(10, HandleKeySortT);
static LIST<RichEditOwnerData> g_RichEditOwnerList(5, HandleKeySortT);

static void SetPosition(HWND hwnd)
{
	CComPtr<IRichEditOle> RichEditOle;
	if (SendMessage(hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle) == 0)
		return;

	CComPtr<ITextDocument> TextDocument;
	if (RichEditOle->QueryInterface(IID_ITextDocument, (void**)&TextDocument) != S_OK)
		return;

	// retrieve text range
	CComPtr<ITextRange> TextRange;
	if (TextDocument->Range(0, 0, &TextRange) != S_OK)
		return;

	int objectCount = RichEditOle->GetObjectCount();
	for (int i = objectCount - 1; i >= 0; i--) {
		REOBJECT reObj = {};
		reObj.cbStruct = sizeof(REOBJECT);

		HRESULT hr = RichEditOle->GetObject(i, &reObj, REO_GETOBJ_POLEOBJ);
		if (FAILED(hr))
			continue;

		CComPtr<ISmileyBase> igsc;
		if (reObj.clsid == CLSID_NULL)
			reObj.poleobj->QueryInterface(IID_ISmileyAddSmiley, (void**)&igsc);

		reObj.poleobj->Release();
		if (igsc == nullptr)
			continue;

		TextRange->SetRange(reObj.cp, reObj.cp);

		RECT rect;
		POINT pt;
		if (S_OK == TextRange->GetPoint(tomStart | TA_BOTTOM | TA_RIGHT, &pt.x, &pt.y)) {
			ScreenToClient(hwnd, &pt);
			rect.bottom = pt.y;
			rect.right = pt.x;
		}
		else rect.bottom = -1;

		if (S_OK == TextRange->GetPoint(tomStart | TA_TOP | TA_LEFT, &pt.x, &pt.y)) {
			ScreenToClient(hwnd, &pt);
			rect.top = pt.y;
			rect.left = pt.x;
		}
		else rect.top = -1;

		igsc->SetPosition(hwnd, &rect);
	}
}

static void SetTooltip(long x, long y, HWND hwnd, RichEditData *rdt)
{
	wchar_t *smltxt;
	int needtip = CheckForTip(x, y, hwnd, &smltxt);
	if (needtip == rdt->tipActive)
		return;

	TOOLINFO ti = { sizeof(ti) };
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.hwnd = hwnd;
	ti.uId = (UINT_PTR)ti.hwnd;

	if (needtip != -1) {
		if (rdt->tipActive == -1) {
			rdt->hToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, L"", TTS_NOPREFIX | WS_POPUP,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, nullptr, g_plugin.getInst(), nullptr);

			SendMessage(rdt->hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
		}

		ti.lpszText = smltxt;
		SendMessage(rdt->hToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
		SendMessage(rdt->hToolTip, TTM_ACTIVATE, TRUE, 0);
	}
	else if (rdt->tipActive != -1) {
		SendMessage(rdt->hToolTip, TTM_ACTIVATE, FALSE, 0);
		DestroyWindow(rdt->hToolTip);
		rdt->hToolTip = nullptr;
	}

	rdt->tipActive = needtip;
}

static const CHARRANGE allsel = { 0, LONG_MAX };

static void ReplaceContactSmileys(RichEditData *rdt, const CHARRANGE &sel, bool ignoreLast, bool unFreeze)
{
	if ((rdt->inputarea && !opt.InputSmileys) || rdt->dontReplace) return;
	SmileyPackCType *smcp = nullptr;
	SmileyPackType *SmileyPack = FindSmileyPack(Proto_GetBaseAccountName(rdt->hContact), rdt->hContact, rdt->inputarea ? nullptr : &smcp);
	ReplaceSmileys(rdt->hwnd, SmileyPack, smcp, sel, false, ignoreLast, unFreeze);
}

static void ReplaceContactSmileysWithText(RichEditData *rdt, CHARRANGE &sel, bool freeze)
{
	if ((rdt->inputarea && !opt.InputSmileys) || rdt->dontReplace) return;
	ReplaceSmileysWithText(rdt->hwnd, sel, freeze);
}

static void SmileyToTextCutPrep(RichEditData *rdt)
{
	if ((rdt->inputarea && !opt.InputSmileys) || rdt->dontReplace)
		return;

	SendMessage(rdt->hwnd, WM_SETREDRAW, FALSE, 0);
	CHARRANGE sel;
	SendMessage(rdt->hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
	ReplaceContactSmileysWithText(rdt, sel, true);
}

static void SmileyToTextCutRest(RichEditData *rdt)
{
	if ((rdt->inputarea && !opt.InputSmileys) || rdt->dontReplace)
		return;

	CHARRANGE sel;
	SendMessage(rdt->hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
	ReplaceContactSmileys(rdt, sel, false, true);
	SendMessage(rdt->hwnd, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(rdt->hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
}

static LRESULT CALLBACK RichEditSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RichEditData *rdt = g_RichEditList.find((RichEditData*)&hwnd);
	if (rdt == nullptr)
		return 0;

	CHARRANGE sel;

	switch (uMsg) {
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

	case WM_DESTROY:
		CloseRichCallback(hwnd);
		break;
	}

	LRESULT result = mir_callNextSubclass(hwnd, RichEditSubclass, uMsg, wParam, lParam);

	switch (uMsg) {
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
			bool hascont = rdt->hContact != 0;
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
	RichEditData *rdt = g_RichEditList.find((RichEditData*)&hwnd);
	if (rdt == nullptr) {
		CComPtr<IRichEditOle> RichEditOle;
		if (SendMessage(hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle) == 0)
			return false;

		rdt = new RichEditData;
		rdt->hwnd = hwnd;
		rdt->hContact = hContact;
		rdt->inputarea = (GetWindowLongPtr(hwnd, GWL_STYLE) & ES_READONLY) == 0;
		rdt->tipActive = -1;
		{
			mir_cslock lck(csLists);
			g_RichEditList.insert(rdt);
		}
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
	if (!hwnd)
		return;

	RichEditData *rdt;
	{
		mir_cslock lck(csLists);
		int ind = g_RichEditList.getIndex((RichEditData *)&hwnd);
		if (ind == -1)
			return;

		rdt = g_RichEditList[ind];
		g_RichEditList.remove(ind);
	}

	if (rdt->hToolTip)
		DestroyWindow(rdt->hToolTip);
	delete rdt;
	
	mir_unsubclassWindow(hwnd, RichEditSubclass);
}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK RichEditOwnerSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RichEditOwnerData *rdto = g_RichEditOwnerList.find((RichEditOwnerData*)&hwnd);
	if (rdto == nullptr)
		return 0;

	switch (uMsg) {
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == 1624) {
			RichEditData *rdt = g_RichEditList.find((RichEditData*)&rdto->hwndInput);
			if (rdt && (!rdt->inputarea || opt.InputSmileys)) {
				rdt->dontReplace = true;
				CHARRANGE sel = allsel;
				ReplaceSmileysWithText(rdt->hwnd, sel, false);
			}
		}
		break;

	case WM_DESTROY:
		RichEditData *rdt = g_RichEditList.find((RichEditData*)&rdto->hwndInput);
		if (rdt && (!rdt->inputarea || opt.InputSmileys)) {
			CHARRANGE sel = allsel;
			rdt->dontReplace = true;
			ReplaceSmileysWithText(rdt->hwnd, sel, false);
		}
		break;
	}

	LRESULT result = mir_callNextSubclass(hwnd, RichEditOwnerSubclass, uMsg, wParam, lParam);

	switch (uMsg) {
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == 1624) {
			RichEditData *rdt = g_RichEditList.find((RichEditData*)&rdto->hwndInput);
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
	RichEditOwnerData *rdto = g_RichEditOwnerList.find((RichEditOwnerData*)&hwnd);
	if (rdto == nullptr) {
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
		if (rdto->hwndInput == nullptr)
			rdto->hwndInput = hwndInput;
		if (rdto->hwndLog == nullptr)
			rdto->hwndLog = hwndLog;
	}
}

void CloseRichOwnerCallback(HWND hwnd)
{
	int ind = g_RichEditOwnerList.getIndex((RichEditOwnerData*)&hwnd);
	if (ind == -1)
		return;

	RichEditOwnerData *rdto = g_RichEditOwnerList[ind];
	CloseRichCallback(rdto->hwndInput);
	CloseRichCallback(rdto->hwndLog);
	delete rdto;

	g_RichEditOwnerList.remove(ind);

	mir_unsubclassWindow(hwnd, RichEditOwnerSubclass);
}

void ProcessAllInputAreas(bool restoreText)
{
	for (auto &rdt : g_RichEditList.rev_iter())
		if (rdt->inputarea) {
			if (restoreText) {
				CHARRANGE sel = allsel;
				ReplaceContactSmileysWithText(rdt, sel, false);
			}
			else ReplaceContactSmileys(rdt, allsel, false, false);
		}
}

void RichEditData_Destroy(void)
{
	for (auto &it : g_RichEditList.rev_iter())
		CloseRichCallback(it->hwnd);
	g_RichEditList.destroy();

	for (auto &it : g_RichEditOwnerList.rev_iter())
		CloseRichOwnerCallback(it->hwnd);
	g_RichEditOwnerList.destroy();
}
