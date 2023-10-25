/*
Miranda SmileyAdd Plugin
Copyright (C) 2003 - 2004 Rein-Peter de Boer
Copyright (C) 2005 - 2011 Boris Krasnovskiy

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

#include <tchar.h>

ISmileyBase* CreateAniSmileyObject(SmileyType *sml, COLORREF clr, bool ishpp);

bool g_HiddenTextSupported = true;

// {8CC497C0-A1DF-11CE-8098-00AA0047BE5D}
const GUID IID_ITextDocument = 
{ 0x8CC497C0, 0xA1DF, 0x11CE, { 0x80,0x98,0x00,0xAA,0x00,0x47,0xBE,0x5D } };

static int CompareSmileys(const SmileyLookup::SmileyLocType *p1, const SmileyLookup::SmileyLocType *p2)
{
	// length is sorted in reverse order, the bigger one goes first
	if (p1->pos == p2->pos)
		return (int)p2->len - (int)p1->len;

	return (int)p1->pos - (int)p2->pos;
}

static bool HasOverlap(const CHARRANGE &loc, SmileysQueueType &smllist)
{
	for (auto &it : smllist)
		if (it->loc.cpMin <= loc.cpMin && it->loc.cpMax >= loc.cpMax)
			return true;

	return false;
}

static void LookupAllSmileysWorker(
	SmileyPackType *smileyPack,
	SmileyPackCType *smileyCPack,
	const wchar_t *lpstrText,
	SmileysQueueType &smllist,
	const bool firstOnly)
{
	if (lpstrText == nullptr || *lpstrText == 0)
		return;

	// All possible smileys
	OBJLIST<SmileyLookup::SmileyLocType> smileys(20, CompareSmileys);

	// Find all possible smileys
	CMStringW tmpstr(lpstrText);

	if (smileyPack)
		for (auto &it : smileyPack->GetSmileyLookup())
			it->Find(tmpstr, smileys, false);

	if (smileyCPack)
		for (auto &it : smileyCPack->GetSmileyLookup())
			it->Find(tmpstr, smileys, false);

	if (smileys.getCount() == 0)
		return;

	// remove overlapped smileys, if any
	for (int i = 0; i < smileys.getCount() - 1; ) {
		auto &p = smileys[i];
		if (p.pos + p.len > smileys[i + 1].pos)
			smileys.remove(i + 1);
		else
			i++;
	}

	long numCharsSoFar = 0;
	size_t smloff = 0;

	for (int i = 0; i < smileys.getCount(); i++) {
		auto &pCurr = smileys[i];
		if (pCurr.pos < smloff)
			continue;

		const wchar_t *textToSearch = lpstrText + smloff;
		const wchar_t *textSmlStart = lpstrText + pCurr.pos;
		const wchar_t *textSmlEnd = textSmlStart + pCurr.len;

		ReplaceSmileyType dat;

		// check if leading space exist
		const wchar_t *prech = _wcsdec(textToSearch, textSmlStart);
		dat.ldspace = prech != nullptr ? iswspace(*prech) != 0 : smloff == 0;
		if (i > 0 && smileys[i - 1].pos + smileys[i - 1].len == smloff)
			dat.ldspace = true;

		// check if trailing space exist
		dat.trspace = *textSmlEnd == 0 || iswspace(*textSmlEnd);
		if (i < smileys.getCount() - 1 && pCurr.pos + pCurr.len == smileys[i + 1].pos)
			dat.trspace = true;

		// compute text location in RichEdit 
		dat.loc.cpMin = (long)_wcsncnt(textToSearch, pCurr.pos - smloff) + numCharsSoFar;
		dat.loc.cpMax = numCharsSoFar = (long)_wcsncnt(textSmlStart, pCurr.len) + dat.loc.cpMin;
		if (!HasOverlap(dat.loc, smllist)) {
			if (!opt.EnforceSpaces || (dat.ldspace && dat.trspace)) {
				dat.ldspace |= !opt.SurroundSmileyWithSpaces;
				dat.trspace |= !opt.SurroundSmileyWithSpaces;

				if (smileyCPack && smileyCPack->GetSmileyLookup().find(pCurr.sml)) {
					dat.smlc = smileyCPack->GetSmiley(pCurr.sml->GetIndex());
					dat.sml = nullptr;
				}
				else {
					dat.sml = smileyPack->GetSmiley(pCurr.sml->GetIndex());
					dat.smlc = nullptr;
				}

				if (dat.sml != nullptr || dat.smlc != nullptr) {
					// First smiley found record it
					smllist.insert(new ReplaceSmileyType(dat));
					if (firstOnly)
						return;
				}
			}
		}

		// Advance string pointer to search for the next smiley
		smloff = int(pCurr.pos + pCurr.len);
	}
}

void LookupAllSmileys(SmileyPackType *smileyPack, SmileyPackCType *smileyCPack, const wchar_t *lpstrText, SmileysQueueType &smllist, bool firstOnly)
{
	LookupAllSmileysWorker(smileyPack, smileyCPack, lpstrText, smllist, firstOnly);

	if (g_pEmoji && smileyPack != g_pEmoji)
		LookupAllSmileysWorker(g_pEmoji, smileyCPack, lpstrText, smllist, firstOnly);
}

/////////////////////////////////////////////////////////////////////////////////////////

SmileyType* FindButtonSmiley(SmileyPackType *smp)
{
	SmileysQueueType smllist;
	LookupAllSmileys(smp, nullptr, smp->GetButtonSmiley(), smllist, true);
	return (smllist.getCount() == 0) ? nullptr : smllist[0].sml;
}

void UpdateSelection(CHARRANGE &sel, int pos, int dif)
{
	if (sel.cpMax == sel.cpMin) {
		if (sel.cpMax < LONG_MAX && sel.cpMax > pos) {
			sel.cpMax += dif;
			sel.cpMin += dif;
		}
	}
	else {
		if (sel.cpMax >= pos && sel.cpMax < LONG_MAX)
			sel.cpMax += dif;
		if (sel.cpMin > pos)
			sel.cpMin += dif;
	}
}

void ReplaceSmileys(HWND hwnd, SmileyPackType *smp, SmileyPackCType *smcp, const CHARRANGE &sel, bool useHidden, bool ignoreLast, bool unFreeze, bool fireView)
{
	CComPtr<IRichEditOle> RichEditOle;
	if (SendMessage(hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle) == 0)
		return;
	if (RichEditOle == nullptr)
		return;

	CComPtr<ITextDocument> TextDocument;
	if (RichEditOle->QueryInterface(IID_ITextDocument, (void **)&TextDocument) != S_OK)
		return;

	long cnt;
	if (smp == nullptr && smcp == nullptr) {
		if (unFreeze)
			TextDocument->Unfreeze(&cnt);
		return;
	}

	// retrieve text range
	CComPtr<ITextRange> TextRange;
	if (TextDocument->Range(sel.cpMin, sel.cpMax, &TextRange) != S_OK)
		return;

	// retrieve text to parse for smileys 
	BSTR btxt = nullptr;
	if (TextRange->GetText(&btxt) != S_OK)
		return;

	HDC hdc = GetDC(hwnd);
	if (hdc == nullptr)
		return;

	SmileysQueueType smllist;
	LookupAllSmileys(smp, smcp, btxt, smllist, false);

	SysFreeString(btxt);

	if (smllist.getCount() != 0) {
		// disable screen updates
		TextDocument->Freeze(&cnt);

		wchar_t classname[20];
		GetClassName(hwnd, classname, _countof(classname));
		bool ishpp = (wcsncmp(classname, L"THppRichEdit", 12) == 0) || fireView;

		SetRichCallback(hwnd, 0, false, true);

		bool rdo = (GetWindowLongPtr(hwnd, GWL_STYLE) & ES_READONLY) != 0;
		if (rdo)
			SendMessage(hwnd, EM_SETREADONLY, FALSE, 0);

		CComPtr<ITextSelection> TextSelection;
		TextDocument->GetSelection(&TextSelection);

		CComPtr<ITextFont> TextFont;
		TextSelection->GetFont(&TextFont);

		//save selection
		CHARRANGE oldSel;
		TextSelection->GetStart(&oldSel.cpMin);
		TextSelection->GetEnd(&oldSel.cpMax);

		CHARFORMAT2 chf;
		chf.cbSize = sizeof(chf);
		chf.dwMask = CFM_ALL2;

		// Determine background color
		// This logic trying to minimize number of background color changes
		COLORREF bkgColor = GetSysColor(COLOR_WINDOW);
		COLORREF bkgColorPv = (COLORREF)SendMessage(hwnd, EM_SETBKGNDCOLOR, 0, bkgColor);
		if (bkgColorPv != bkgColor) {
			bkgColor = bkgColorPv;
			SendMessage(hwnd, EM_SETBKGNDCOLOR, 0, bkgColor);
		}

		int sclX = GetDeviceCaps(hdc, LOGPIXELSX);
		int sclY = GetDeviceCaps(hdc, LOGPIXELSY);

		unsigned numBTBSm = 0;

		BSTR spaceb = SysAllocString(L" ");

		// Replace smileys specified in the list in RichEdit 
		for (int j = smllist.getCount() - 1; j >= 0; j--) {
			CHARRANGE &smlpos = smllist[j].loc;
			if (ignoreLast && oldSel.cpMax == smlpos.cpMax)
				continue;

			smlpos.cpMin += sel.cpMin;
			smlpos.cpMax += sel.cpMin;

			// Find all back to back smileys and for propper hidden text detection
			if (numBTBSm == 0) {
				CHARRANGE lastPos = smlpos;
				for (int jn = j; jn--; ) {
					if (jn != j && smllist[jn].loc.cpMax != lastPos.cpMin)
						break;

					++numBTBSm;
					lastPos.cpMin = smllist[jn].loc.cpMin;
				}
				TextSelection->SetRange(lastPos.cpMin, lastPos.cpMax);
				long hid;
				TextFont->GetHidden(&hid);
				if (hid == tomFalse) numBTBSm = 0;
			}
			if (numBTBSm != 0) {
				--numBTBSm;
				continue;
			}

			SmileyType *sml = smllist[j].sml;
			SmileyCType *smlc = smllist[j].smlc;
			if (sml == nullptr && smlc == nullptr)
				continue;

			// Select text analyze
			TextSelection->SetRange(smlpos.cpMin, smlpos.cpMax);

			BSTR bstrText = nullptr;

			if (smlc == nullptr && sml->IsText()) {
				bstrText = SysAllocString(sml->GetToolText().c_str());
				TextSelection->SetText(bstrText);
			}
			else {
				TextSelection->GetText(&bstrText);

				// Get font properties
				SendMessage(hwnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&chf);

				//do not look for smileys in hyperlinks
				if ((chf.dwEffects & (CFE_LINK | CFE_HIDDEN)) != 0)
					continue;

				SIZE osize;
				if (sml)
					sml->GetSize(osize);
				else
					smlc->GetSize(osize);

				if (osize.cx == 0 || osize.cy == 0)
					continue;

				int sizeX, sizeY;
				if (opt.ScaleToTextheight) {
					sizeY = CalculateTextHeight(hdc, &chf);
					sizeX = osize.cx * sizeY / osize.cy;

					int dx = osize.cx - sizeX;
					sizeX += dx & 1;

					int dy = osize.cy - sizeY;
					sizeY += dy & 1;
				}
				else {
					sizeX = osize.cx;
					sizeY = osize.cy;
				}

				if (smlc != nullptr && opt.MaxCustomSmileySize && (unsigned)sizeY > opt.MaxCustomSmileySize) {
					sizeY = opt.MaxCustomSmileySize;
					sizeX = osize.cx * sizeY / osize.cy;

					int dx = osize.cx - sizeX;
					sizeX += dx & 1;

					int dy = osize.cy - opt.MaxCustomSmileySize;
					sizeY += dy & 1;
				}

				if (opt.MinSmileySize && (unsigned)sizeY < opt.MinSmileySize) {
					sizeY = opt.MinSmileySize;
					sizeX = osize.cx * sizeY / osize.cy;

					int dx = osize.cx - sizeX;
					sizeX += dx & 1;

					int dy = osize.cy - opt.MinSmileySize;
					sizeY += dy & 1;
				}

				// Convert pixel to HIMETRIC
				SIZEL sizehm;
				sizehm.cx = (2540 * (sizeX + 1) + (sclX >> 1)) / sclX;
				sizehm.cy = (2540 * (sizeY + 1) + (sclY >> 1)) / sclY;

				// If font does not have designated background use control background
				if (chf.dwEffects & CFE_AUTOBACKCOLOR) chf.crBackColor = bkgColor;

				// insert space after
				if (!smllist[j].trspace && useHidden) {
					TextSelection->SetStart(smlpos.cpMax);
					TextSelection->TypeText(spaceb);
					UpdateSelection(oldSel, smlpos.cpMax, 1);

					// Restore selection
					TextSelection->SetRange(smlpos.cpMin, smlpos.cpMax);
				}

				if (g_HiddenTextSupported && useHidden) {
					TextFont->SetHidden(tomTrue);
					TextSelection->SetEnd(smlpos.cpMin);
					UpdateSelection(oldSel, smlpos.cpMin, 1);
				}
				else UpdateSelection(oldSel, smlpos.cpMin, -(int)SysStringLen(bstrText) + 1);

				ISmileyBase *smileyBase = CreateAniSmileyObject(smlc ? smlc : sml, chf.crBackColor, ishpp);
				if (smileyBase == nullptr)
					continue;

				smileyBase->SetExtent(DVASPECT_CONTENT, &sizehm);
				smileyBase->SetHint(bstrText);

				smileyBase->SetPosition(hwnd, nullptr);

				// Get the RichEdit container site
				IOleClientSite *pOleClientSite;
				RichEditOle->GetClientSite(&pOleClientSite);

				// Now Add the object to the RichEdit 
				REOBJECT reobject = { sizeof(REOBJECT) };
				reobject.cp = REO_CP_SELECTION;
				reobject.dvaspect = DVASPECT_CONTENT;
				reobject.poleobj = smileyBase;
				reobject.polesite = pOleClientSite;
				reobject.dwFlags = REO_BELOWBASELINE | REO_BLANK;
				reobject.dwUser = (DWORD_PTR)smileyBase;

				// Insert the bitmap at the current location in the richedit control
				RichEditOle->InsertObject(&reobject);

				smileyBase->Release();

				// insert space before
				if (!smllist[j].ldspace && useHidden) {
					TextSelection->SetRange(smlpos.cpMin, smlpos.cpMin);
					TextSelection->TypeText(spaceb);
					UpdateSelection(oldSel, smlpos.cpMin, 1);
				}
			}
			SysFreeString(bstrText);
		}
		SysFreeString(spaceb);

		TextSelection->SetRange(oldSel.cpMin, oldSel.cpMax);
		if (rdo)
			SendMessage(hwnd, EM_SETREADONLY, TRUE, 0);

		ReleaseDC(hwnd, hdc);

		TextDocument->Unfreeze(&cnt);
		if (cnt == 0)
			UpdateWindow(hwnd);
	}
	else ReleaseDC(hwnd, hdc);

	if (unFreeze) {
		TextDocument->Unfreeze(&cnt);
		if (cnt == 0)
			UpdateWindow(hwnd);
	}
}

void ReplaceSmileysWithText(HWND hwnd, CHARRANGE &sel, bool keepFrozen)
{
	CComPtr<IRichEditOle> RichEditOle = nullptr;
	if (SendMessage(hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle) == 0)
		return;
	if (RichEditOle == nullptr)
		return;

	CComPtr<ITextDocument> TextDocument;
	if (RichEditOle->QueryInterface(IID_ITextDocument, (void **)&TextDocument) != S_OK)
		return;

	// retrieve text range
	CComPtr<ITextRange> TextRange;
	if (TextDocument->Range(0, 0, &TextRange) != S_OK)
		return;

	long cnt;
	TextDocument->Freeze(&cnt);

	bool rdo = (GetWindowLongPtr(hwnd, GWL_STYLE) & ES_READONLY) != 0;
	if (rdo)
		SendMessage(hwnd, EM_SETREADONLY, FALSE, 0);

	CHARRANGE oldSel;
	SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&oldSel);

	int objectCount = RichEditOle->GetObjectCount();
	for (int i = objectCount - 1; i >= 0; i--) {
		REOBJECT reObj = { 0 };
		reObj.cbStruct = sizeof(REOBJECT);

		HRESULT hr = RichEditOle->GetObject(i, &reObj, REO_GETOBJ_POLEOBJ);
		if (FAILED(hr))
			continue;

		if (reObj.cp < sel.cpMin) {
			reObj.poleobj->Release();
			break;
		}

		ISmileyBase *igsc = nullptr;
		if (reObj.cp < sel.cpMax && reObj.clsid == CLSID_NULL)
			reObj.poleobj->QueryInterface(IID_ISmileyAddSmiley, (void **)&igsc);

		reObj.poleobj->Release();
		if (igsc == nullptr)
			continue;

		TextRange->SetRange(reObj.cp, reObj.cp + 1);

		BSTR bstr = nullptr;
		igsc->GetTooltip(&bstr);
		TextRange->SetText(bstr);

		unsigned int len = SysStringLen(bstr);
		UpdateSelection(oldSel, reObj.cp, len - 1);
		UpdateSelection(sel, reObj.cp, len - 1);

		SysFreeString(bstr);

		igsc->Release();
	}

	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&oldSel);
	if (rdo)
		SendMessage(hwnd, EM_SETREADONLY, TRUE, 0);
	if (!keepFrozen)
		TextDocument->Unfreeze(&cnt);
}
