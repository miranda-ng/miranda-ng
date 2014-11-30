// Feel free to use this code in your own applications.
// The Author does not guarantee anything about this code.
// Author : Yves Maurer

// FormattedTextDraw.cpp : Implementation of CFormattedTextDraw

#include "headers.h"


const IID IID_ITextServices = { // 8d33f740-cf58-11ce-a89d-00aa006cadc5
    0x8d33f740,
    0xcf58,
    0x11ce,
    {0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
  };

const IID IID_ITextHost = { // c5bdd8d0-d26e-11ce-a89e-00aa006cadc5
    0xc5bdd8d0,
    0xd26e,
    0x11ce,
    {0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
  };

static const IID IID_ITextDocument = {
	0x8CC497C0,
	0xA1DF,
	0x11CE,
	{0x80,0x98,0x00,0xAA,0x00,0x47,0xBE,0x5D}
  };

/////////////////////////////////////////////////////////////////////////////
// CallBack functions
DWORD CALLBACK EditStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	PCOOKIE pCookie = (PCOOKIE) dwCookie;
	if (pCookie->isUnicode)
	{
		if ((pCookie->dwSize - pCookie->dwCount)*sizeof(WCHAR) < (DWORD)cb)
			*pcb = (pCookie->dwSize - pCookie->dwCount)*sizeof(WCHAR);
		else
			*pcb = cb&~1UL;
		CopyMemory(pbBuff, pCookie->unicode+pCookie->dwCount, *pcb);
		pCookie->dwCount += *pcb/sizeof(WCHAR);
	} else
	{
		if(pCookie->dwSize - pCookie->dwCount < (DWORD)cb)
			*pcb = pCookie->dwSize - pCookie->dwCount;
		else
			*pcb = cb;
		CopyMemory(pbBuff, pCookie->ansi+pCookie->dwCount, *pcb);
		pCookie->dwCount += *pcb;
	}

	return 0;	//	callback succeeded - no errors
}

/////////////////////////////////////////////////////////////////////////////
// CFormattedTextDraw
HRESULT CFormattedTextDraw::putRTFTextA(char *newVal)
{
	HRESULT hr;
	LRESULT lResult = 0;
	EDITSTREAM editStream;

	if (!m_spTextServices) 
		return S_FALSE;

	m_editCookie.isUnicode = false;
	m_editCookie.ansi = newVal;
	m_editCookie.dwSize = mir_strlen(m_editCookie.ansi);
	m_editCookie.dwCount = 0;
	editStream.dwCookie = (DWORD_PTR) &m_editCookie;
	editStream.dwError = 0;
	editStream.pfnCallback = (EDITSTREAMCALLBACK)EditStreamInCallback;
	hr = m_spTextServices->TxSendMessage(EM_STREAMIN, (WPARAM)(SF_RTF), (LPARAM)&editStream, &lResult);
	return S_OK;
}

HRESULT CFormattedTextDraw::putRTFTextW(WCHAR *newVal)
{

	HRESULT hr;
	LRESULT lResult = 0;
	EDITSTREAM editStream;

	if (!m_spTextServices) 
		return S_FALSE;

	m_editCookie.isUnicode = true;
	m_editCookie.unicode = newVal;
	m_editCookie.dwSize = mir_wstrlen(m_editCookie.unicode);
	m_editCookie.dwCount = 0;
	editStream.dwCookie = (DWORD_PTR) &m_editCookie;
	editStream.dwError = 0;
	editStream.pfnCallback = (EDITSTREAMCALLBACK)EditStreamInCallback;
	hr = m_spTextServices->TxSendMessage(EM_STREAMIN, (WPARAM)(SF_RTF|SF_UNICODE), (LPARAM)&editStream, &lResult);
	return S_OK;

}

HRESULT CFormattedTextDraw::putTextA(char *newVal)
{
	HRESULT hr;
	LRESULT lResult = 0;
	EDITSTREAM editStream;

	if (!m_spTextServices) 
		return S_FALSE;

	m_editCookie.isUnicode = false;
	m_editCookie.ansi = newVal;
	m_editCookie.dwSize = mir_strlen(m_editCookie.ansi);
	m_editCookie.dwCount = 0;
	editStream.dwCookie = (DWORD_PTR) &m_editCookie;
	editStream.dwError = 0;
	editStream.pfnCallback = (EDITSTREAMCALLBACK)EditStreamInCallback;
	hr = m_spTextServices->TxSendMessage(EM_STREAMIN, (WPARAM)(SF_TEXT), (LPARAM)&editStream, &lResult);
	
	CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE|CFM_BOLD;
	cf.dwEffects = 0;
	mir_sntprintf(cf.szFaceName, SIZEOF(cf.szFaceName), _T("MS Shell Dlg"));
	m_spTextServices->TxSendMessage(EM_SETCHARFORMAT, (WPARAM)(SCF_ALL), (LPARAM)&cf, &lResult);

	return S_OK;
}

HRESULT CFormattedTextDraw::putTextW(WCHAR *newVal)
{

	HRESULT hr;
	LRESULT lResult = 0;
	EDITSTREAM editStream;

	if (!m_spTextServices) 
		return S_FALSE;

	m_editCookie.isUnicode = true;
	m_editCookie.unicode = newVal;
	m_editCookie.dwSize = mir_wstrlen(m_editCookie.unicode);
	m_editCookie.dwCount = 0;
	editStream.dwCookie = (DWORD_PTR) &m_editCookie;
	editStream.dwError = 0;
	editStream.pfnCallback = (EDITSTREAMCALLBACK)EditStreamInCallback;
	hr = m_spTextServices->TxSendMessage(EM_STREAMIN, (WPARAM)(SF_TEXT|SF_UNICODE), (LPARAM)&editStream, &lResult);
	
	CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE|CFM_BOLD;
	cf.dwEffects = 0;
	mir_sntprintf(cf.szFaceName, SIZEOF(cf.szFaceName), _T("MS Shell Dlg"));
	m_spTextServices->TxSendMessage(EM_SETCHARFORMAT, (WPARAM)(SCF_ALL), (LPARAM)&cf, &lResult);

	return S_OK;

}

HRESULT CFormattedTextDraw::Draw(void *hdcDraw, RECT *prc)
{
	LOGFONT lf;
	GetObject(GetCurrentObject((HDC)hdcDraw, OBJ_FONT), sizeof(lf), &lf);

	LRESULT lResult;
	CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE/*|CFM_COLOR*/|CFM_CHARSET|CFM_SIZE|
		(lf.lfWeight>=FW_BOLD?CFM_BOLD:0)|
		(lf.lfItalic?CFM_ITALIC:0)|
		(lf.lfUnderline?CFM_UNDERLINE:0)|
		(lf.lfStrikeOut?CFM_STRIKEOUT:0);
	cf.dwEffects = CFE_BOLD|CFE_ITALIC|CFE_STRIKEOUT|CFE_UNDERLINE;
	cf.crTextColor = GetTextColor((HDC)hdcDraw);
	cf.bCharSet = lf.lfCharSet;
	cf.yHeight = 1440 * abs(lf.lfHeight) / GetDeviceCaps((HDC)hdcDraw, LOGPIXELSY);
	mir_sntprintf(cf.szFaceName, SIZEOF(cf.szFaceName), lf.lfFaceName);
	m_spTextServices->TxSendMessage(EM_SETCHARFORMAT, (WPARAM)(SCF_ALL), (LPARAM)&cf, &lResult);

	m_spTextServices->TxDraw(
	    DVASPECT_CONTENT,  		// Draw Aspect
		0,						// Lindex
		NULL,					// Info for drawing optimization
		NULL,					// target device information
		(HDC) hdcDraw,			// Draw device HDC
		NULL,			 	   	// Target device HDC
		(RECTL *) prc,			// Bounding client rectangle
		NULL,					// Clipping rectangle for metafiles
		(RECT *) NULL,			// Update rectangle
		NULL, 	   				// Call back function
		NULL,					// Call back parameter
		TXTVIEW_INACTIVE);		// What view of the object could be TXTVIEW_ACTIVE
	return S_OK;
}

HRESULT CFormattedTextDraw::Create()
{
	return CreateTextServicesObject();
}

HRESULT CFormattedTextDraw::get_NaturalSize(void *hdcDraw, long *Width, long *Height)
{
	LOGFONT lf;
	GetObject(GetCurrentObject((HDC)hdcDraw, OBJ_FONT), sizeof(lf), &lf);

	LRESULT lResult;
	CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE/*|CFM_COLOR*/|CFM_CHARSET|CFM_SIZE|
		(lf.lfWeight>=FW_BOLD?CFM_BOLD:0)|
		(lf.lfItalic?CFM_ITALIC:0)|
		(lf.lfUnderline?CFM_UNDERLINE:0)|
		(lf.lfStrikeOut?CFM_STRIKEOUT:0);
	cf.dwEffects = CFE_BOLD|CFE_ITALIC|CFE_STRIKEOUT|CFE_UNDERLINE;
	cf.crTextColor = GetTextColor((HDC)hdcDraw);
	cf.bCharSet = lf.lfCharSet;
	cf.yHeight = 1440 * abs(lf.lfHeight) / GetDeviceCaps((HDC)hdcDraw, LOGPIXELSY);
	mir_sntprintf(cf.szFaceName, SIZEOF(cf.szFaceName), lf.lfFaceName);

	if (!m_spTextServices)
		return S_FALSE;

	m_spTextServices->TxSendMessage(EM_SETCHARFORMAT, (WPARAM)(SCF_ALL), (LPARAM)&cf, &lResult);

	SIZEL szExtent;
//	HDC	hdcDraw;
//	hdcDraw = GetDC(NULL);
	*Height = 1;
	szExtent.cx = *Width;
	szExtent.cy = *Height;
	if (m_spTextServices->TxGetNaturalSize(DVASPECT_CONTENT, (HDC)hdcDraw, NULL, NULL, TXTNS_FITTOCONTENT, &szExtent, Width, Height) != S_OK)
	{
//		ReleaseDC(NULL, hdcDraw);
		return S_FALSE;
	}

//	ReleaseDC(NULL, hdcDraw);
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// ITextHost functions
HDC CFormattedTextDraw::TxGetDC()
{
	return NULL;
}

INT CFormattedTextDraw::TxReleaseDC(HDC hdc)
{
	return 1;
}

BOOL CFormattedTextDraw::TxShowScrollBar(INT fnBar, BOOL fShow)
{
	return FALSE;
}

BOOL CFormattedTextDraw::TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags)
{
	return FALSE;
}

BOOL CFormattedTextDraw::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
{
	return FALSE;
}

BOOL CFormattedTextDraw::TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw)
{
	return FALSE;
}

void CFormattedTextDraw::TxInvalidateRect(LPCRECT prc, BOOL fMode)
{
}

void CFormattedTextDraw::TxViewChange(BOOL fUpdate)
{
}

BOOL CFormattedTextDraw::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
{
	return FALSE;
}

BOOL CFormattedTextDraw::TxShowCaret(BOOL fShow)
{
	return FALSE;
}

BOOL CFormattedTextDraw::TxSetCaretPos(INT x, INT y)
{
	return FALSE;
}

BOOL CFormattedTextDraw::TxSetTimer(UINT idTimer, UINT uTimeout)
{
	return FALSE;
}

void CFormattedTextDraw::TxKillTimer(UINT idTimer)
{
}

void CFormattedTextDraw::TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll)
{
}

void CFormattedTextDraw::TxSetCapture(BOOL fCapture)
{
}

void CFormattedTextDraw::TxSetFocus()
{
}


void CFormattedTextDraw::TxSetCursor(HCURSOR hcur, BOOL fText)
{
	if (fText)
	{
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
	} else
	{
		SetCursor(hcur);
	}
}

BOOL CFormattedTextDraw::TxScreenToClient(LPPOINT lppt)
{
	if (!m_hwndParent) return FALSE;
//	BOOL result = ScreenToClient(m_hwndParent, lppt);
//	lppt->x -= m_rcClient.left;
//	lppt->y -= m_rcClient.left;
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

HRESULT	CFormattedTextDraw::TxActivate(LONG * plOldState)
{
	return S_OK;
}

HRESULT	CFormattedTextDraw::TxDeactivate(LONG lNewState)
{
	return S_OK;
}

HRESULT	CFormattedTextDraw::TxGetClientRect(LPRECT prc)
{
	*prc = m_rcClient;
	return S_OK;
}

HRESULT	CFormattedTextDraw::TxGetViewInset(LPRECT prc)
{
	*prc = m_rcViewInset;
	return S_OK;
}

HRESULT CFormattedTextDraw::TxGetCharFormat(const CHARFORMATW **ppCF)
{
	*ppCF = m_pCF;
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

HRESULT	CFormattedTextDraw::TxGetBackStyle(TXTBACKSTYLE *pstyle)
{
	*pstyle = TXTBACK_TRANSPARENT;
	return S_OK;
}

HRESULT	CFormattedTextDraw::TxGetMaxLength(DWORD *plength)
{
	*plength = m_dwMaxLength;
	return S_OK;
}

HRESULT	CFormattedTextDraw::TxGetScrollBars(DWORD *pdwScrollBar)
{
	*pdwScrollBar = m_dwScrollbar;
	return S_OK;
}

HRESULT	CFormattedTextDraw::TxGetPasswordChar(TCHAR *pch)
{
	return S_FALSE;
}

HRESULT	CFormattedTextDraw::TxGetAcceleratorPos(LONG *pcp)
{
	*pcp = -1;
	return S_OK;
}

HRESULT	CFormattedTextDraw::TxGetExtent(LPSIZEL lpExtent)
{
	return E_NOTIMPL;
}

HRESULT CFormattedTextDraw::OnTxCharFormatChange(const CHARFORMATW * pcf)
{
	memcpy(m_pCF, pcf, pcf->cbSize);
	return S_OK;
}

HRESULT	CFormattedTextDraw::OnTxParaFormatChange(const PARAFORMAT * ppf)
{
	memcpy(&m_PF, ppf, ppf->cbSize);
	return S_OK;
}

HRESULT	CFormattedTextDraw::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits)
{
	*pdwBits = m_dwPropertyBits;
	return S_OK;
}

HRESULT	CFormattedTextDraw::TxNotify(DWORD iNotify, void *pv)
{
	return S_OK;
}

HIMC CFormattedTextDraw::TxImmGetContext()
{
	return NULL;
}

void CFormattedTextDraw::TxImmReleaseContext(HIMC himc)
{
}

HRESULT	CFormattedTextDraw::TxGetSelectionBarWidth(LONG *lSelBarWidth)
{
	*lSelBarWidth = 100;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// custom functions

HRESULT CFormattedTextDraw::CharFormatFromHFONT(CHARFORMAT2W* pCF, HFONT hFont)
// Takes an HFONT and fills in a CHARFORMAT2W structure with the corresponding info
{

	HWND hWnd;
	LOGFONT lf;
	HDC hDC;
	LONG yPixPerInch;

	// Get LOGFONT for default font
	if (!hFont)
		hFont = (HFONT) GetStockObject(SYSTEM_FONT);

	// Get LOGFONT for passed hfont
	if (!GetObject(hFont, sizeof(LOGFONT), &lf))
		return E_FAIL;

	// Set CHARFORMAT structure
	memset(pCF, 0, sizeof(CHARFORMAT2W));
	pCF->cbSize = sizeof(CHARFORMAT2W);

	hWnd = GetDesktopWindow();
	hDC = GetDC(hWnd);
	yPixPerInch = GetDeviceCaps(hDC, LOGPIXELSY);
	pCF->yHeight = -lf.lfHeight * LY_PER_INCH / yPixPerInch;
	ReleaseDC(hWnd, hDC);

	pCF->yOffset = 0;
	pCF->crTextColor = 0;

	pCF->dwEffects = CFM_EFFECTS | CFE_AUTOBACKCOLOR;
	pCF->dwEffects &= ~(CFE_PROTECTED | CFE_LINK | CFE_AUTOCOLOR);

	if(lf.lfWeight < FW_BOLD)
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
	return CharFormatFromHFONT(m_pCF, NULL);
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

HRESULT CFormattedTextDraw::CreateTextServicesObject()
{
	HRESULT hr;
	IUnknown *spUnk;

	hr = MyCreateTextServices(NULL, static_cast<ITextHost*>(this), &spUnk);
	if (hr == S_OK) {
		hr = spUnk->QueryInterface(IID_ITextServices, (void**)&m_spTextServices);
		hr = spUnk->QueryInterface(IID_ITextDocument, (void**)&m_spTextDocument);
		spUnk->Release();
	}
	return hr;
}

