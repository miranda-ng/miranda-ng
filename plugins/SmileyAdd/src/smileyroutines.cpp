/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2011 Boris Krasnovskiy
Copyright (C) 2003 - 2004 Rein-Peter de Boer

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

static void LookupAllSmileysWorker(
	SmileyPackType *smileyPack,
	SmileyPackCType *smileyCPack,
	const wchar_t *lpstrText,
	SmileysQueueType &smllist,
	const bool firstOnly)
{
	if (lpstrText == nullptr || *lpstrText == 0) return;

	auto *sml  = smileyPack ? smileyPack->GetSmileyLookup() : nullptr;
	auto *smlc = smileyCPack ? &smileyCPack->GetSmileyLookup() : nullptr;

	// Precompute number of smileys
	int smlszo = sml  ? sml->getCount()  : 0;
	int smlszc = smlc ? smlc->getCount() : 0;
	int smlsz = smlszo + smlszc;

	if (smlsz == 0) return;

	// All possible smileys
	SmileyLookup::SmileyLocVecType *smileys = new SmileyLookup::SmileyLocVecType [smlsz];

	// Find all possible smileys
	CMStringW tmpstr(lpstrText);
	int i = 0;

	if (sml)
		for (auto &it : *sml) {
			it->Find(tmpstr, smileys[i], false);
			i++;
		}

	if (smlc)
		for (auto &it : *smlc) {
			it->Find(tmpstr, smileys[i], false);
			i++;
		}

	int *csmlit = (int*)alloca(smlsz * sizeof(int));
	memset(csmlit, 0, smlsz * sizeof(int));

	long numCharsSoFar = 0;
	size_t smloff = 0;

	while (true) {
		int firstSml = -1;
		int firstSmlRef = -1;
		SmileyLookup::SmileyLocVecType *smlf = nullptr;

		for (int csml=0; csml < smlsz; csml++) {
			SmileyLookup::SmileyLocVecType &smlv = smileys[csml];

			int tsml;
			for (tsml = csmlit[csml]; tsml < smlv.getCount(); tsml++) {
				if (smlv[tsml].pos >= smloff) {
					if (firstSmlRef == -1 || smlv[tsml].pos < (*smlf)[firstSmlRef].pos || 
						(smlv[tsml].pos == (*smlf)[firstSmlRef].pos && smlv[tsml].len > (*smlf)[firstSmlRef].len))
					{
						firstSmlRef = tsml;
						firstSml = csml;
						smlf = &smileys[csml];
					}
					break;
				}
			}
			csmlit[csml] = tsml;
		}

		// // Nothing to parse, exiting
		if (firstSml == -1)
			break;

		SmileyLookup::SmileyLocType &psmlf = (*smlf)[firstSmlRef];
		const wchar_t *textToSearch = lpstrText + smloff;
		const wchar_t *textSmlStart = lpstrText + psmlf.pos;
		const wchar_t *textSmlEnd   = textSmlStart + psmlf.len;

		ReplaceSmileyType *dat = new ReplaceSmileyType;

		// check if leading space exist
		const wchar_t *prech = _wcsdec(textToSearch, textSmlStart);
		dat->ldspace = prech != nullptr ? iswspace(*prech) != 0 : smloff == 0;

		// check if trailing space exist
		dat->trspace = *textSmlEnd == 0 || iswspace(*textSmlEnd);

		// compute text location in RichEdit 
		dat->loc.cpMin = (long)_wcsncnt(textToSearch, psmlf.pos - smloff) + numCharsSoFar;
		dat->loc.cpMax = numCharsSoFar = (long)_wcsncnt(textSmlStart, psmlf.len) + dat->loc.cpMin;

		if (!opt.EnforceSpaces || (dat->ldspace && dat->trspace)) {
			dat->ldspace |= !opt.SurroundSmileyWithSpaces;
			dat->trspace |= !opt.SurroundSmileyWithSpaces;

			if (firstSml < smlszo) {
				dat->sml = smileyPack->GetSmiley((*sml)[firstSml].GetIndex());
				dat->smlc = nullptr;
			}
			else {
				dat->smlc = smileyCPack->GetSmiley((*smlc)[firstSml-smlszo].GetIndex());
				dat->sml = nullptr;
			}

			if (dat->sml != nullptr || dat->smlc != nullptr) {
				// First smiley found record it
				smllist.insert(dat);
				if (firstOnly) break; 
			}
			else delete dat;
		}
		else delete dat;

		// Advance string pointer to search for the next smiley
		smloff = int(psmlf.pos + psmlf.len);
		csmlit[firstSml]++;
	}
	delete[] smileys;
}

void LookupAllSmileys(SmileyPackType *smileyPack, SmileyPackCType *smileyCPack, const wchar_t *lpstrText, SmileysQueueType &smllist, const bool firstOnly)
{
	LookupAllSmileysWorker(smileyPack, smileyCPack, lpstrText, smllist, firstOnly);

	if (g_pEmoji && smileyPack != g_pEmoji)
		LookupAllSmileys(g_pEmoji, smileyCPack, lpstrText, smllist, firstOnly);
}

void FindSmileyInText(SmileyPackType *smp, const wchar_t *str, unsigned &first, unsigned &size, SmileyType **sml)
{
	SmileysQueueType smllist;
	LookupAllSmileys(smp, nullptr, str, smllist, true);
	if (smllist.getCount() == 0) {
		size = 0;
		*sml = nullptr;
	}
	else {
		first = smllist[0].loc.cpMin;
		size  = smllist[0].loc.cpMax - smllist[0].loc.cpMin;
		*sml  = smllist[0].sml;
	}
}


SmileyType* FindButtonSmiley(SmileyPackType *smp)
{
	unsigned start, size;
	SmileyType *sml;
	FindSmileyInText(smp, smp->GetButtonSmiley(), start, size, &sml);
	return sml;
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

void ReplaceSmileys(HWND hwnd, SmileyPackType *smp, SmileyPackCType *smcp, const CHARRANGE &sel,  bool useHidden, bool ignoreLast, bool unFreeze, bool fireView)
{
	CComPtr<IRichEditOle> RichEditOle;
	if (SendMessage(hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle) == 0)
		return;
	if (RichEditOle == nullptr)
		return;

	CComPtr<ITextDocument> TextDocument;
	if (RichEditOle->QueryInterface(IID_ITextDocument, (void**)&TextDocument) != S_OK)
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
	if (RichEditOle->QueryInterface(IID_ITextDocument, (void**)&TextDocument) != S_OK)
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
	for (int i = objectCount-1; i >= 0; i--) {
		REOBJECT reObj = {0};
		reObj.cbStruct  = sizeof(REOBJECT);

		HRESULT hr = RichEditOle->GetObject(i, &reObj, REO_GETOBJ_POLEOBJ);
		if (FAILED(hr))
			continue;

		if (reObj.cp < sel.cpMin) {
			reObj.poleobj->Release();
			break;
		}

		ISmileyBase *igsc = nullptr;
		if (reObj.cp < sel.cpMax && reObj.clsid == CLSID_NULL) 
			reObj.poleobj->QueryInterface(IID_ISmileyAddSmiley, (void**) &igsc);

		reObj.poleobj->Release();
		if (igsc == nullptr)
			continue;

		TextRange->SetRange(reObj.cp, reObj.cp + 1);

		BSTR bstr = nullptr;
		igsc->GetTooltip(&bstr);
		TextRange->SetText(bstr);

		unsigned int len = SysStringLen(bstr);
		UpdateSelection(oldSel, reObj.cp, len-1);
		UpdateSelection(sel, reObj.cp, len-1);

		SysFreeString(bstr);

		igsc->Release();
	}

	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&oldSel);
	if (rdo)
		SendMessage(hwnd, EM_SETREADONLY, TRUE, 0);
	if (!keepFrozen)
		TextDocument->Unfreeze(&cnt);
}
