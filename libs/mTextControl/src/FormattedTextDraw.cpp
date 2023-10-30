// Feel free to use this code in your own applications.
// The Author does not guarantee anything about this code.
// Author : Yves Maurer

// FormattedTextDraw.cpp : Implementation of CFormattedTextDraw

#include "stdafx.h"
#include "FormattedTextDraw.h"

const IID IID_ITextServices = { // 8d33f740-cf58-11ce-a89d-00aa006cadc5
	0x8d33f740,
	0xcf58,
	0x11ce,
	{ 0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5 }
};

const IID IID_ITextDocument = {
	0x8CC497C0,
	0xA1DF,
	0x11CE,
	{ 0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D }
};

/////////////////////////////////////////////////////////////////////////////
// CFormattedTextDraw

CFormattedTextDraw::CFormattedTextDraw()
{
	HDC hdcScreen;

	hdcScreen = GetDC(nullptr);
	nPixelsPerInchX = GetDeviceCaps(hdcScreen, LOGPIXELSX);
	nPixelsPerInchY = GetDeviceCaps(hdcScreen, LOGPIXELSY);
	ReleaseDC(nullptr, hdcScreen);

	SetRectEmpty(&m_rcClient);
	SetRectEmpty(&m_rcViewInset);

	InitDefaultCharFormat();
	InitDefaultParaFormat();
	m_spTextServices = nullptr;
	m_spTextDocument = nullptr;

	m_dwPropertyBits = TXTBIT_RICHTEXT | TXTBIT_MULTILINE | TXTBIT_WORDWRAP | TXTBIT_USECURRENTBKG;

	IUnknown *spUnk;
	HRESULT hr = MyCreateTextServices(nullptr, this, &spUnk);
	if (hr == S_OK) {
		hr = spUnk->QueryInterface(IID_ITextServices, (void **)&m_spTextServices);
		hr = spUnk->QueryInterface(IID_ITextDocument, (void **)&m_spTextDocument);
		spUnk->Release();
	}
}

CFormattedTextDraw::~CFormattedTextDraw()
{
	if (m_spTextServices != nullptr)
		m_spTextServices->Release();
	if (m_spTextDocument != nullptr)
		m_spTextDocument->Release();
}

/////////////////////////////////////////////////////////////////////////////////////////

struct STREAMDATATEXT
{
	bool isUnicode;
	union
	{
		char *ansi;
		wchar_t *unicode;
	};
	size_t cbSize, cbCount;
};

static DWORD CALLBACK EditStreamTextInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	auto *pCookie = (STREAMDATATEXT *)dwCookie;
	if (pCookie->isUnicode) {
		if ((pCookie->cbSize - pCookie->cbCount) * sizeof(wchar_t) < (size_t)cb)
			*pcb = LONG(pCookie->cbSize - pCookie->cbCount) * sizeof(wchar_t);
		else
			*pcb = cb & ~1UL;
		memcpy(pbBuff, pCookie->unicode + pCookie->cbCount, *pcb);
		pCookie->cbCount += *pcb / sizeof(wchar_t);
	}
	else {
		if (pCookie->cbSize - pCookie->cbCount < (uint32_t)cb)
			*pcb = LONG(pCookie->cbSize - pCookie->cbCount);
		else
			*pcb = cb;
		memcpy(pbBuff, pCookie->ansi + pCookie->cbCount, *pcb);
		pCookie->cbCount += *pcb;
	}

	return 0;	//	callback succeeded - no errors
}

HRESULT CFormattedTextDraw::putTextA(char *newVal)
{
	if (!m_spTextServices)
		return S_FALSE;

	STREAMDATATEXT streamData = {};
	streamData.isUnicode = false;
	streamData.ansi = newVal;
	streamData.cbSize = mir_strlen(newVal);

	EDITSTREAM editStream;
	editStream.dwCookie = (DWORD_PTR)&streamData;
	editStream.dwError = 0;
	editStream.pfnCallback = EditStreamTextInCallback;

	LRESULT lResult = 0;
	m_spTextServices->TxSendMessage(EM_STREAMIN, SF_TEXT, (LPARAM)&editStream, &lResult);

	CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE | CFM_BOLD;
	cf.dwEffects = 0;
	wcsncpy_s(cf.szFaceName, L"MS Shell Dlg", _TRUNCATE);
	m_spTextServices->TxSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf, &lResult);

	return S_OK;
}

HRESULT CFormattedTextDraw::putTextW(wchar_t *newVal)
{
	if (!m_spTextServices)
		return S_FALSE;

	STREAMDATATEXT streamData = {};
	streamData.isUnicode = true;
	streamData.unicode = newVal;
	streamData.cbSize = mir_wstrlen(newVal);

	EDITSTREAM editStream;
	editStream.dwCookie = (DWORD_PTR)&streamData;
	editStream.dwError = 0;
	editStream.pfnCallback = EditStreamTextInCallback;

	LRESULT lResult = 0;
	m_spTextServices->TxSendMessage(EM_STREAMIN, SF_TEXT | SF_UNICODE, (LPARAM)&editStream, &lResult);

	CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE | CFM_BOLD;
	cf.dwEffects = 0;
	wcsncpy_s(cf.szFaceName, L"MS Shell Dlg", _TRUNCATE);
	m_spTextServices->TxSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf, &lResult);
	return S_OK;
}

HRESULT CFormattedTextDraw::putRTFText(char *newVal)
{
	if (!m_spTextServices)
		return S_FALSE;

	m_bNative = true;

	STREAMDATATEXT streamData = {};
	streamData.isUnicode = false;
	streamData.ansi = newVal;
	streamData.cbSize = mir_strlen(newVal);

	EDITSTREAM editStream;
	editStream.dwCookie = (DWORD_PTR)&streamData;
	editStream.dwError = 0;
	editStream.pfnCallback = EditStreamTextInCallback;

	LRESULT lResult = 0;
	m_spTextServices->TxSendMessage(EM_STREAMIN, SF_RTF, (LPARAM)&editStream, &lResult);
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Other methods

HRESULT CFormattedTextDraw::Draw(HDC hdcDraw, RECT *prc)
{
	if (!m_bNative) {
		LOGFONT lf;
		GetObject(GetCurrentObject(hdcDraw, OBJ_FONT), sizeof(lf), &lf);

		LRESULT lResult;
		CHARFORMAT cf;
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_FACE/*|CFM_COLOR*/ | CFM_CHARSET | CFM_SIZE |
			(lf.lfWeight >= FW_BOLD ? CFM_BOLD : 0) |
			(lf.lfItalic ? CFM_ITALIC : 0) |
			(lf.lfUnderline ? CFM_UNDERLINE : 0) |
			(lf.lfStrikeOut ? CFM_STRIKEOUT : 0);
		cf.dwEffects = CFE_BOLD | CFE_ITALIC | CFE_STRIKEOUT | CFE_UNDERLINE;
		cf.crTextColor = GetTextColor(hdcDraw);
		cf.bCharSet = lf.lfCharSet;
		cf.yHeight = 1440 * abs(lf.lfHeight) / GetDeviceCaps(hdcDraw, LOGPIXELSY);
		wcsncpy_s(cf.szFaceName, lf.lfFaceName, _TRUNCATE);
		m_spTextServices->TxSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf, &lResult);
	}

	m_spTextServices->TxDraw(
		DVASPECT_CONTENT, // Draw Aspect
		0,                // Lindex
		nullptr,          // Info for drawing optimization
		nullptr,          // target device information
		hdcDraw,          // Draw device HDC
		nullptr,          // Target device HDC
		(RECTL *)prc,     // Bounding client rectangle
		nullptr,          // Clipping rectangle for metafiles
		(RECT *)nullptr,  // Update rectangle
		nullptr,          // Call back function
		NULL,             // Call back parameter
		TXTVIEW_INACTIVE); // What view of the object could be TXTVIEW_ACTIVE

	return S_OK;
}

HRESULT CFormattedTextDraw::get_NaturalSize(HDC hdcDraw, long *Width, long *Height)
{
	if (hdcDraw == 0)
		return S_FALSE;

	int iCaps = GetDeviceCaps(hdcDraw, LOGPIXELSY);
	if (iCaps == 0)
		return S_FALSE;

	if (!m_spTextServices)
		return S_FALSE;

	if (!m_bNative) {
		LOGFONT lf;
		GetObject(GetCurrentObject(hdcDraw, OBJ_FONT), sizeof(lf), &lf);

		LRESULT lResult;
		CHARFORMAT cf;
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_FACE/*|CFM_COLOR*/ | CFM_CHARSET | CFM_SIZE |
			(lf.lfWeight >= FW_BOLD ? CFM_BOLD : 0) |
			(lf.lfItalic ? CFM_ITALIC : 0) |
			(lf.lfUnderline ? CFM_UNDERLINE : 0) |
			(lf.lfStrikeOut ? CFM_STRIKEOUT : 0);
		cf.dwEffects = CFE_BOLD | CFE_ITALIC | CFE_STRIKEOUT | CFE_UNDERLINE;
		cf.crTextColor = GetTextColor(hdcDraw);
		cf.bCharSet = lf.lfCharSet;
		cf.yHeight = 1440 * abs(lf.lfHeight) / iCaps;
		wcsncpy_s(cf.szFaceName, lf.lfFaceName, _TRUNCATE);

		m_spTextServices->TxSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf, &lResult);
	}

	*Height = 1;

	SIZEL szExtent;
	szExtent.cx = *Width;
	szExtent.cy = *Height;
	if (m_spTextServices->TxGetNaturalSize(DVASPECT_CONTENT, hdcDraw, nullptr, nullptr, TXTNS_FITTOCONTENT, &szExtent, Width, Height) != S_OK)
		return S_FALSE;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// ITextHost functions

HDC CFormattedTextDraw::TxGetDC()
{
	return GetDC(m_hwndParent);
}

INT CFormattedTextDraw::TxReleaseDC(HDC hdc)
{
	ReleaseDC(m_hwndParent, hdc);
	return FALSE;
}

BOOL CFormattedTextDraw::TxShowScrollBar(INT, BOOL)
{
	return FALSE;
}

BOOL CFormattedTextDraw::TxEnableScrollBar(INT, INT)
{
	return FALSE;
}

BOOL CFormattedTextDraw::TxSetScrollRange(INT, LONG, INT, BOOL)
{
	return FALSE;
}

BOOL CFormattedTextDraw::TxSetScrollPos(INT, INT, BOOL)
{
	return FALSE;
}

void CFormattedTextDraw::TxInvalidateRect(LPCRECT, BOOL)
{}

void CFormattedTextDraw::TxViewChange(BOOL)
{}

BOOL CFormattedTextDraw::TxCreateCaret(HBITMAP, INT, INT)
{
	return FALSE;
}

BOOL CFormattedTextDraw::TxShowCaret(BOOL)
{
	return FALSE;
}

BOOL CFormattedTextDraw::TxSetCaretPos(INT, INT)
{
	return FALSE;
}

BOOL CFormattedTextDraw::TxSetTimer(UINT, UINT)
{
	return FALSE;
}

void CFormattedTextDraw::TxKillTimer(UINT)
{}

void CFormattedTextDraw::TxScrollWindowEx(INT, INT, LPCRECT, LPCRECT, HRGN, LPRECT, UINT)
{}

void CFormattedTextDraw::TxSetCapture(BOOL)
{}

void CFormattedTextDraw::TxSetFocus()
{}

void CFormattedTextDraw::TxSetCursor(HCURSOR hcur, BOOL fText)
{
	if (fText)
		SetCursor(LoadCursorW(nullptr, MAKEINTRESOURCEW(IDC_ARROW)));
	else
		SetCursor(hcur);
}

BOOL CFormattedTextDraw::TxScreenToClient(LPPOINT lppt)
{
	if (!m_hwndParent) return FALSE;
	return ScreenToClient(m_hwndParent, lppt);
}

BOOL CFormattedTextDraw::TxClientToScreen(LPPOINT lppt)
{
	if (!m_hwndParent) return FALSE;
	//	BOOL result = ;
	//	lppt->x -= m_rcClient.left;
	//	lppt->y -= m_rcClient.left;
	return ClientToScreen(m_hwndParent, lppt);
}

HRESULT CFormattedTextDraw::TxActivate(LONG *)
{
	return S_OK;
}

HRESULT CFormattedTextDraw::TxDeactivate(LONG)
{
	return S_OK;
}

HRESULT CFormattedTextDraw::TxGetClientRect(LPRECT prc)
{
	*prc = m_rcClient;
	return S_OK;
}

HRESULT CFormattedTextDraw::TxGetViewInset(LPRECT prc)
{
	*prc = m_rcViewInset;
	return S_OK;
}

HRESULT CFormattedTextDraw::TxGetCharFormat(const CHARFORMATW **ppCF)
{
	*ppCF = &m_CF;
	return S_OK;
}

HRESULT	CFormattedTextDraw::TxGetParaFormat(const PARAFORMAT **ppPF)
{
	*ppPF = &m_PF;
	return S_OK;
}

COLORREF CFormattedTextDraw::TxGetSysColor(int nIndex)
{
	return GetSysColor(nIndex);
}

HRESULT CFormattedTextDraw::TxGetBackStyle(TXTBACKSTYLE *pstyle)
{
	*pstyle = TXTBACK_TRANSPARENT;
	return S_OK;
}

HRESULT	CFormattedTextDraw::TxGetMaxLength(DWORD *plength)
{
	*plength = INFINITE;
	return S_OK;
}

HRESULT CFormattedTextDraw::TxGetScrollBars(DWORD *pdwScrollBar)
{
	*pdwScrollBar = 0;
	return S_OK;
}

HRESULT CFormattedTextDraw::TxGetPasswordChar(wchar_t *)
{
	return S_FALSE;
}

HRESULT CFormattedTextDraw::TxGetAcceleratorPos(LONG *pcp)
{
	*pcp = -1;
	return S_OK;
}

HRESULT CFormattedTextDraw::TxGetExtent(LPSIZEL)
{
	return E_NOTIMPL;
}

HRESULT CFormattedTextDraw::OnTxCharFormatChange(const CHARFORMATW *pcf)
{
	memcpy(&m_CF, pcf, pcf->cbSize);
	return S_OK;
}

HRESULT	CFormattedTextDraw::OnTxParaFormatChange(const PARAFORMAT *ppf)
{
	memcpy(&m_PF, ppf, ppf->cbSize);
	return S_OK;
}

HRESULT CFormattedTextDraw::TxGetPropertyBits(DWORD, DWORD *pdwBits)
{
	*pdwBits = m_dwPropertyBits;
	return S_OK;
}

HRESULT CFormattedTextDraw::TxNotify(DWORD, void *)
{
	return S_OK;
}

HIMC CFormattedTextDraw::TxImmGetContext()
{
	return nullptr;
}

void CFormattedTextDraw::TxImmReleaseContext(HIMC)
{}

HRESULT CFormattedTextDraw::TxGetSelectionBarWidth(LONG *lSelBarWidth)
{
	*lSelBarWidth = 100;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Takes an HFONT and fills in a CHARFORMAT2W structure with the corresponding info

HRESULT CFormattedTextDraw::CharFormatFromHFONT(CHARFORMAT2W *pCF, HFONT hFont)
{
	// Get LOGFONT for default font
	if (!hFont)
		hFont = (HFONT)GetStockObject(SYSTEM_FONT);

	// Get LOGFONT for passed hfont
	LOGFONTW lf;
	if (!GetObjectW(hFont, sizeof(LOGFONT), &lf))
		return E_FAIL;

	// Set CHARFORMAT structure
	memset(pCF, 0, sizeof(CHARFORMAT2W));
	pCF->cbSize = sizeof(CHARFORMAT2W);
	pCF->yHeight = -lf.lfHeight;

	if (HWND hWnd = GetDesktopWindow()) {
		if (HDC hDC = GetDC(hWnd)) {
			LONG yPixPerInch = GetDeviceCaps(hDC, LOGPIXELSY);
			pCF->yHeight = -lf.lfHeight * LY_PER_INCH / yPixPerInch;
			ReleaseDC(hWnd, hDC);
		}
	}

	pCF->yOffset = 0;
	pCF->crTextColor = 0;

	pCF->dwEffects = CFM_EFFECTS | CFE_AUTOBACKCOLOR;
	pCF->dwEffects &= ~(CFE_PROTECTED | CFE_LINK | CFE_AUTOCOLOR);

	if (lf.lfWeight < FW_BOLD)
		pCF->dwEffects &= ~CFE_BOLD;

	if (!lf.lfItalic)
		pCF->dwEffects &= ~CFE_ITALIC;

	if (!lf.lfUnderline)
		pCF->dwEffects &= ~CFE_UNDERLINE;

	if (!lf.lfStrikeOut)
		pCF->dwEffects &= ~CFE_STRIKEOUT;

	pCF->dwMask = CFM_ALL | CFM_BACKCOLOR | CFM_STYLE;
	pCF->bCharSet = lf.lfCharSet;
	pCF->bPitchAndFamily = lf.lfPitchAndFamily;

	mir_wstrcpy(pCF->szFaceName, lf.lfFaceName);
	return S_OK;
}

HRESULT CFormattedTextDraw::InitDefaultCharFormat()
{
	return CharFormatFromHFONT(&m_CF, nullptr);
}

HRESULT CFormattedTextDraw::InitDefaultParaFormat()
{
	memset(&m_PF, 0, sizeof(PARAFORMAT2));
	m_PF.cbSize = sizeof(PARAFORMAT2);
	m_PF.dwMask = PFM_ALL;
	m_PF.wAlignment = PFA_LEFT;
	m_PF.cTabCount = 1;
	m_PF.rgxTabs[0] = lDefaultTab;
	return S_OK;
}
