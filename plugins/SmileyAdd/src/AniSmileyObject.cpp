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

class CAniSmileyObject;

static int CompareAniSmiley(const CAniSmileyObject* p1, const CAniSmileyObject* p2)
{
	return (int)((char*)p2 - (char*)p1);
}

static LIST<CAniSmileyObject> regAniSmileys(10, CompareAniSmiley);

static UINT_PTR timerId;
static void CALLBACK timerProc(HWND, UINT, UINT_PTR, DWORD);

static void CALLBACK sttMainThreadCallback( PVOID )
{
	if (timerId == 0xffffffff) 
		timerId = SetTimer(NULL, 0, 100, (TIMERPROC)timerProc);
}


class CAniSmileyObject : public ISmileyBase
{
private:
	typedef enum { animStdOle, animDrctRichEd, animHpp } AnimType; 

	POINTL      m_rectOrig;
	SIZEL       m_rectExt;

	COLORREF    m_bkg;

	SmileyType *m_sml;
	ImageBase  *m_img;
	unsigned	   m_nFramePosition;

	long        m_counter;
	unsigned    m_richFlags;
	long        m_lastObjNum; 

	AnimType    m_animtype;
	bool        m_allowAni;

public:
	CAniSmileyObject(SmileyType* sml, COLORREF clr, bool ishpp)
	{
		m_allowAni = false;
		m_animtype = ishpp ? animHpp : animStdOle;
		m_bkg      = clr;

		m_rectOrig.x = 0;
		m_rectOrig.y = 0;
		m_rectExt.cx = 0;
		m_rectExt.cy = 0;

		m_richFlags = 0;
		m_lastObjNum = 0;

		m_sml = sml;
		m_img = NULL;
		m_nFramePosition = 0;
		m_counter = 0;
	}

	~CAniSmileyObject(void)
	{
		UnloadSmiley();
	}

	void LoadSmiley(void)
	{
		if (m_img != NULL) return;

		m_img = m_sml->CreateCachedImage();
		if (m_img && m_img->IsAnimated() && opt.AnimateDlg) {
			m_nFramePosition = 0;
			m_img->SelectFrame(m_nFramePosition);
			long frtm = m_img->GetFrameDelay();
			m_counter = frtm / 10 + ((frtm % 10) >= 5);

			regAniSmileys.insert(this);
			if (timerId == 0) {
				timerId = 0xffffffff;
				CallFunctionAsync(sttMainThreadCallback, NULL);
			}
		}
		else m_nFramePosition = m_sml->GetStaticFrame();
	}

	void UnloadSmiley(void)
	{
		regAniSmileys.remove(this);

		if (timerId && (timerId+1) && regAniSmileys.getCount() == 0) {
			KillTimer(NULL, timerId);
			timerId = 0;
		}
		if (m_img) m_img->Release();
		m_img = NULL;
	}

	void GetDrawingProp(void)
	{
		if (m_hwnd == NULL) return;

		IRichEditOle* RichEditOle;
		if (SendMessage(m_hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle) == 0)
			return;

		REOBJECT reObj = {0};
		reObj.cbStruct  = sizeof(REOBJECT);

		HRESULT hr = RichEditOle->GetObject(m_lastObjNum, &reObj, REO_GETOBJ_NO_INTERFACES);
		if (hr == S_OK && reObj.dwUser == (DWORD)(ISmileyBase*)this && reObj.clsid == CLSID_NULL) 
			m_richFlags = reObj.dwFlags;
		else {
			long objectCount = RichEditOle->GetObjectCount();
			for (long i = objectCount; i--; ) {
				HRESULT hr = RichEditOle->GetObject(i, &reObj, REO_GETOBJ_NO_INTERFACES);
				if (FAILED(hr)) continue;

				if (reObj.dwUser == (DWORD)(ISmileyBase*)this && reObj.clsid == CLSID_NULL) {
					m_lastObjNum = i;
					m_richFlags = reObj.dwFlags;
					break;
				}
			}
		}
		RichEditOle->Release();

		if ((m_richFlags & REO_SELECTED) == 0) {
			CHARRANGE sel;
			SendMessage(m_hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			if (reObj.cp >= sel.cpMin && reObj.cp < sel.cpMax)
				m_richFlags |= REO_INVERTEDSELECT;
			else
				m_richFlags &= ~REO_INVERTEDSELECT;
		}
	}


	void DoDirectDraw(HDC hdc)
	{
		HBITMAP hBmp = CreateCompatibleBitmap(hdc, m_rectExt.cx, m_rectExt.cy);
		HDC hdcMem = CreateCompatibleDC(hdc);
		HANDLE hOld = SelectObject(hdcMem, hBmp);

		RECT rc;
		rc.left   = m_rectExt.cx - m_sizeExtent.cx;
		rc.top    = m_rectExt.cy - m_sizeExtent.cy;
		rc.right  = rc.left + m_sizeExtent.cx;
		rc.bottom = rc.top + m_sizeExtent.cy;

		HBRUSH hbr = CreateSolidBrush(m_bkg); 
		RECT frc = { 0, 0, m_rectExt.cx, m_rectExt.cy };
		FillRect(hdcMem, &frc, hbr);
		DeleteObject(hbr);

		m_img->DrawInternal(hdcMem, rc.left, rc.top, m_sizeExtent.cx - 1, m_sizeExtent.cy - 1); 

		if (m_richFlags & REO_SELECTED) {
			HBRUSH hbr = CreateSolidBrush(m_bkg ^ 0xFFFFFF); 
			FrameRect(hdcMem, &rc, hbr);
			DeleteObject(hbr);
		}

		if (m_richFlags & REO_INVERTEDSELECT)
			InvertRect(hdcMem, &rc);

		BitBlt(hdc, m_rectOrig.x, m_rectOrig.y, m_rectExt.cx, m_rectExt.cy, hdcMem, 0, 0, SRCCOPY);

		SelectObject(hdcMem, hOld);    
		DeleteObject(hBmp);	
		DeleteDC(hdcMem);
	}

	void DrawOnRichEdit(void)
	{ 
		HDC hdc = GetDC(m_hwnd);
		if (RectVisible(hdc, &m_orect)) {
			RECT crct;
			GetClientRect(m_hwnd, &crct);

			HRGN hrgnOld = CreateRectRgnIndirect(&crct);
			int res = GetClipRgn(hdc, hrgnOld);

			HRGN hrgn = CreateRectRgnIndirect(&crct); 
			SelectClipRgn(hdc, hrgn); 
			DeleteObject(hrgn);

			DoDirectDraw(hdc);

			SelectClipRgn(hdc, res < 1 ? NULL : hrgnOld); 
			DeleteObject(hrgnOld);
		}
		else {
			m_visible = false;
			m_allowAni = false;
			UnloadSmiley();
		}
		ReleaseDC(m_hwnd, hdc);
	}

	void DrawOnHPP(void)
	{
		FVCNDATA_NMHDR nmh = {0};
		nmh.code = NM_FIREVIEWCHANGE;
		nmh.hwndFrom = m_hwnd;

		nmh.cbSize = sizeof(nmh);
		nmh.bEvent = FVCN_PREFIRE;
		nmh.bAction = FVCA_DRAW;
		nmh.rcRect = m_orect;
		SendMessage(GetParent(m_hwnd), WM_NOTIFY, (WPARAM)m_hwnd, (LPARAM)&nmh);

		switch (nmh.bAction) {
		case FVCA_DRAW:
			// support for pseudo-edit mode and event details
			m_animtype = m_dirAniAllow ? animDrctRichEd : animStdOle;
			GetDrawingProp();
			DrawOnRichEdit();
			break;

		case FVCA_CUSTOMDRAW:
			m_rectExt.cy = nmh.rcRect.bottom - nmh.rcRect.top;
			m_rectExt.cx = nmh.rcRect.right - nmh.rcRect.left;
			m_rectOrig.x = nmh.rcRect.left; 
			m_rectOrig.y = nmh.rcRect.top;

			m_bkg = nmh.clrBackground;

			DoDirectDraw(nmh.hDC);

			nmh.bEvent = FVCN_POSTFIRE;
			SendMessage(GetParent(m_hwnd), WM_NOTIFY, (WPARAM)m_hwnd, (LPARAM)&nmh);
			break;

		case FVCA_SKIPDRAW:
			break;

		case FVCA_NONE:
			m_visible = false;
			break;
		}
	}

	void ProcessTimerTick(void)
	{
		if (m_visible && m_img && --m_counter <= 0) {
			m_nFramePosition = m_img->SelectNextFrame(m_nFramePosition);
			long frtm = m_img->GetFrameDelay();
			m_counter = frtm / 10 + ((frtm % 10) >= 5);

			switch (m_animtype) {
			case animStdOle:
				if (m_allowAni) SendOnViewChange();
				else {
					m_visible = false;
					UnloadSmiley();
				}
				m_allowAni = false;
				break;

			case animDrctRichEd:
				DrawOnRichEdit();
				break;

			case animHpp:
				DrawOnHPP();
				break;
			}
		}
	}

	void SetPosition(HWND hwnd, LPCRECT lpRect)
	{
		ISmileyBase::SetPosition(hwnd, lpRect);

		m_allowAni = m_visible;

		if (m_visible) LoadSmiley();
		else UnloadSmiley();

		if (lpRect == NULL) return;
		if (m_animtype == animStdOle) {
			m_animtype = animDrctRichEd;
			GetDrawingProp();
		}

		if (lpRect->top == -1) {
			m_rectOrig.x = lpRect->left;
			m_rectOrig.y = lpRect->bottom - m_sizeExtent.cy;
			m_rectExt.cy = m_sizeExtent.cy;
		}
		else if (lpRect->bottom == -1) {
			m_rectOrig.x = lpRect->left;
			m_rectOrig.y = lpRect->top;
		}
		else {
			m_rectOrig.x = lpRect->left;
			m_rectOrig.y = lpRect->top;
			m_rectExt.cy = lpRect->bottom - lpRect->top;
		}
	}

	STDMETHOD(Close)(DWORD dwSaveOption)
	{
		m_visible = false;
		UnloadSmiley();

		return ISmileyBase::Close(dwSaveOption);
	}

	STDMETHOD(Draw)(DWORD dwAspect, LONG, void*, DVTARGETDEVICE*, HDC, 
		HDC hdc, LPCRECTL pRectBounds, LPCRECTL /* pRectWBounds */,
		BOOL (__stdcall *)(ULONG_PTR), ULONG_PTR) 
	{
		if (dwAspect != DVASPECT_CONTENT) return DV_E_DVASPECT;
		if (pRectBounds == NULL) return E_INVALIDARG;

		LoadSmiley();

		if (m_img == NULL) return E_FAIL;

		m_sizeExtent.cx = pRectBounds->right  - pRectBounds->left;
		m_sizeExtent.cy = pRectBounds->bottom - pRectBounds->top;

		m_rectExt = m_sizeExtent;

		switch (m_animtype) {
		case animDrctRichEd: 
			{
				m_rectExt.cy = pRectBounds->bottom - m_rectOrig.y;
				RECT frc = { 0, 0, m_sizeExtent.cx - 1, m_sizeExtent.cy - 1 };

				HBITMAP hBmp = CreateCompatibleBitmap(hdc, frc.right, frc.bottom);
				HDC hdcMem = CreateCompatibleDC(hdc);
				HANDLE hOld = SelectObject(hdcMem, hBmp);

				HBRUSH hbr = CreateSolidBrush(m_bkg); 
				FillRect(hdcMem, &frc, hbr);
				DeleteObject(hbr);

				m_img->DrawInternal(hdcMem, 0, 0, frc.right, frc.bottom);

				BitBlt(hdc, pRectBounds->left, pRectBounds->top, frc.right, frc.bottom, hdcMem, 0, 0, SRCCOPY);

				SelectObject(hdcMem, hOld);    
				DeleteObject(hBmp);	
				DeleteDC(hdcMem);
			}
			GetDrawingProp();
			break;

		case animHpp:
			m_orect = *(LPRECT)pRectBounds;

		default:
			m_img->DrawInternal(hdc, pRectBounds->left, pRectBounds->top, 
				m_sizeExtent.cx - 1, m_sizeExtent.cy - 1);
			break;
		}

		m_allowAni  = true;
		m_visible = true;

		return S_OK;
	}

	STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL* psizel)
	{ 
		HRESULT hr = ISmileyBase::SetExtent(dwDrawAspect, psizel);
		if (hr == S_OK) m_rectExt = m_sizeExtent;
		return hr;
	}
};

ISmileyBase* CreateAniSmileyObject(SmileyType* sml, COLORREF clr, bool ishpp)
{
	if (!sml->IsValid()) return NULL;

	CAniSmileyObject *obj = new CAniSmileyObject(sml, clr, ishpp);
	return obj;
}

static void CALLBACK timerProc(HWND, UINT, UINT_PTR, DWORD) 
{
	for (int i=0; i < regAniSmileys.getCount(); i++)
		regAniSmileys[i]->ProcessTimerTick();
}

void DestroyAniSmileys(void)
{
	if (timerId && (timerId+1)) {
		KillTimer(NULL, timerId);
		timerId = 0;
	}

	for (int i=0; i < regAniSmileys.getCount(); i++)
		delete regAniSmileys[i];
}

