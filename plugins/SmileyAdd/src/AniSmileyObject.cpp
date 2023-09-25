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

class CAniSmileyObject;

class CAniSmileyObject : public ISmileyBase, public MZeroedObject
{
	typedef enum { animStdOle, animDrctRichEd, animHpp } AnimType;

	POINTL      m_rectOrig;
	SIZEL       m_rectExt;

	COLORREF    m_bkg;

	SmileyType *m_sml;
	ImageBase  *m_img;

	unsigned    m_richFlags;
	long        m_lastObjNum;

	AnimType    m_animtype;
	bool        m_allowAni;
	bool        m_bRegistered;

public:
	CAniSmileyObject(SmileyType *sml, COLORREF clr, bool ishpp)
	{
		m_animtype = ishpp ? animHpp : animStdOle;
		m_bkg = clr;
		m_sml = sml;
	}

	virtual ~CAniSmileyObject()
	{
		UnloadSmiley();
	}

	virtual void OnClose(void)
	{
		UnloadSmiley();
	}

	void LoadSmiley()
	{
		if (m_img != nullptr) return;

		m_img = m_sml->CreateCachedImage();
		if (m_img && m_img->IsAnimated() && opt.AnimateDlg) {
			m_sml->AddObject(this);
			m_bRegistered = true;
		}
	}

	void UnloadSmiley()
	{
		if (m_bRegistered) {
			m_sml->RemoveObject(this);
			m_bRegistered = false;
		}

		if (m_img != nullptr) {
			m_img->Release();
			m_img = nullptr;
		}
	}

	void GetDrawingProp()
	{
		if (m_hwnd == nullptr)
			return;

		REOBJECT reObj = {};
		reObj.cbStruct = sizeof(reObj);
		{
			CComPtr<IRichEditOle> RichEditOle;
			if (SendMessage(m_hwnd, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle) == 0)
				return;

			HRESULT hr = RichEditOle->GetObject(m_lastObjNum, &reObj, REO_GETOBJ_NO_INTERFACES);
			if (SUCCEEDED(hr) && reObj.dwUser == (DWORD_PTR)this && reObj.clsid == CLSID_NULL)
				m_richFlags = reObj.dwFlags;
			else {
				long objectCount = RichEditOle->GetObjectCount();
				for (long i = objectCount; i--; ) {
					hr = RichEditOle->GetObject(i, &reObj, REO_GETOBJ_NO_INTERFACES);
					if (FAILED(hr)) continue;

					if (reObj.dwUser == (DWORD_PTR)this && reObj.clsid == CLSID_NULL) {
						m_lastObjNum = i;
						m_richFlags = reObj.dwFlags;
						break;
					}
				}
			}
		}

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
		rc.left = m_rectExt.cx - m_sizeExtent.cx;
		rc.top = m_rectExt.cy - m_sizeExtent.cy;
		rc.right = rc.left + m_sizeExtent.cx;
		rc.bottom = rc.top + m_sizeExtent.cy;
		{
			HBRUSH hbr = CreateSolidBrush(m_bkg);
			RECT frc = { 0, 0, m_rectExt.cx, m_rectExt.cy };
			FillRect(hdcMem, &frc, hbr);
			DeleteObject(hbr);
		}
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

	void DrawOnRichEdit()
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

			SelectClipRgn(hdc, res < 1 ? nullptr : hrgnOld);
			DeleteObject(hrgnOld);
		}
		else {
			m_visible = false;
			m_allowAni = false;
			UnloadSmiley();
		}
		ReleaseDC(m_hwnd, hdc);
	}

	void DrawOnHPP()
	{
		FVCNDATA_NMHDR nmh = {};
		nmh.code = NM_FIREVIEWCHANGE;
		nmh.hwndFrom = m_hwnd;

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

	virtual void Draw()
	{
		if (!m_visible || !m_img)
			return;

		switch (m_animtype) {
		case animStdOle:
			if (!m_allowAni) {
				m_visible = false;
				UnloadSmiley();
			}
			else {
				SendOnViewChange();
				m_allowAni = false;
			}
			break;

		case animDrctRichEd:
			DrawOnRichEdit();
			break;

		case animHpp:
			DrawOnHPP();
			break;
		}
	}

	virtual void SetPosition(HWND hwnd, LPCRECT lpRect)
	{
		ISmileyBase::SetPosition(hwnd, lpRect);

		m_allowAni = m_visible;

		if (m_visible) LoadSmiley();
		else UnloadSmiley();

		if (lpRect == nullptr) return;
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

	STDMETHOD(Draw)(DWORD dwAspect, LONG, void*, DVTARGETDEVICE*, HDC, HDC hdc, LPCRECTL pRectBounds, LPCRECTL, BOOL(__stdcall *)(ULONG_PTR), ULONG_PTR)
	{
		if (dwAspect != DVASPECT_CONTENT) return DV_E_DVASPECT;
		if (pRectBounds == nullptr) return E_INVALIDARG;

		LoadSmiley();

		if (m_img == nullptr) return E_FAIL;

		m_sizeExtent.cx = pRectBounds->right - pRectBounds->left;
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
			m_img->DrawInternal(hdc, pRectBounds->left, pRectBounds->top, m_sizeExtent.cx - 1, m_sizeExtent.cy - 1);
			break;
		}

		m_allowAni = true;
		m_visible = true;

		return S_OK;
	}

	STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL *psizel)
	{
		HRESULT hr = ISmileyBase::SetExtent(dwDrawAspect, psizel);
		if (hr == S_OK)
			m_rectExt = m_sizeExtent;
		return hr;
	}
};

ISmileyBase* CreateAniSmileyObject(SmileyType *sml, COLORREF clr, bool ishpp)
{
	if (!sml->IsValid())
		return nullptr;

	return new CAniSmileyObject(sml, clr, ishpp);
}
