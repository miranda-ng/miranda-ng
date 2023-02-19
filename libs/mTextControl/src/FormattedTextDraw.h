// Feel free to use this code in your own applications.
// The Author does not guarantee anything about this code.
// Author : Yves Maurer
// FormattedTextDraw.h : Declaration of the CFormattedTextDraw

#ifndef __FORMATTEDTEXTDRAW_H_
#define __FORMATTEDTEXTDRAW_H_

#ifndef LY_PER_INCH
#define LY_PER_INCH   1440
#define HOST_BORDER 0
#endif

struct COOKIE
{
	bool isUnicode;
	union
	{
		char *ansi;
		wchar_t *unicode;
	};
	size_t cbSize, cbCount;
};

/////////////////////////////////////////////////////////////////////////////
// CFormattedTextDraw

class CFormattedTextDraw : public ITextHost, public MZeroedObject
{
	HWND           m_hwndParent;
	RECT           m_rcClient;			// Client Rect
	RECT           m_rcViewInset;		// view rect inset
	SIZEL          m_sizelExtent;		// Extent array

	int            nPixelsPerInchX;    // Pixels per logical inch along width
	int            nPixelsPerInchY;    // Pixels per logical inch along height

	CHARFORMAT2W  *m_pCF;
	PARAFORMAT2    m_PF;
	uint32_t       m_dwScrollbar;		// Scroll bar style
	uint32_t       m_dwPropertyBits;	// Property bits
	uint32_t       m_dwMaxLength;
	COOKIE         m_editCookie;

	ITextServices *m_spTextServices;
	ITextDocument *m_spTextDocument;

public:
	CFormattedTextDraw();
	~CFormattedTextDraw();

	HRESULT get_NaturalSize(HDC hdcDraw, long *Width, long *pVal);
	HRESULT Draw(HDC hdcDraw, RECT *prc);
	HRESULT putRTFTextA(char *newVal);
	HRESULT putRTFTextW(wchar_t *newVal);
	HRESULT putTextA(char *newVal);
	HRESULT putTextW(wchar_t *newVal);

	__forceinline ITextServices *getTextService() { return m_spTextServices; };
	__forceinline ITextDocument *getTextDocument() { return m_spTextDocument; };

	__forceinline void setParentWnd(HWND hwnd, RECT rect) { m_hwndParent = hwnd; m_rcClient = rect; }

	// IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void __RPC_FAR *__RPC_FAR *ppvObject) override
	{	*ppvObject = nullptr;
		return S_FALSE;
	}

	ULONG STDMETHODCALLTYPE AddRef(void) override
	{	return 0;
	}

	ULONG STDMETHODCALLTYPE Release(void) override
	{	return 0;
	}

	// ITextHost
	HDC      TxGetDC() override;
	INT      TxReleaseDC(HDC hdc) override;
	BOOL     TxShowScrollBar(INT fnBar, BOOL fShow) override;
	BOOL     TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags) override;
	BOOL     TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw) override;
	BOOL     TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw) override;
	void     TxInvalidateRect(LPCRECT prc, BOOL fMode) override;
	void     TxViewChange(BOOL fUpdate) override;
	BOOL     TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight) override;
	BOOL     TxShowCaret(BOOL fShow) override;
	BOOL     TxSetCaretPos(INT x, INT y) override;
	BOOL     TxSetTimer(UINT idTimer, UINT uTimeout) override;
	void     TxKillTimer(UINT idTimer) override;
	void     TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll) override;
	void     TxSetCapture(BOOL fCapture) override;
	void     TxSetFocus() override;
	void     TxSetCursor(HCURSOR hcur, BOOL fText) override;
	BOOL     TxScreenToClient(LPPOINT lppt) override;
	BOOL     TxClientToScreen(LPPOINT lppt) override;
	HRESULT	TxActivate(LONG * plOldState) override;
	HRESULT	TxDeactivate(LONG lNewState) override;
	HRESULT	TxGetClientRect(LPRECT prc) override;
	HRESULT	TxGetViewInset(LPRECT prc) override;
	HRESULT  TxGetCharFormat(const CHARFORMATW **ppCF) override;
	HRESULT	TxGetParaFormat(const PARAFORMAT **ppPF) override;
	COLORREF TxGetSysColor(int nIndex) override;
	HRESULT	TxGetBackStyle(TXTBACKSTYLE *pstyle) override;
	HRESULT	TxGetMaxLength(DWORD *plength) override;
	HRESULT	TxGetScrollBars(DWORD *pdwScrollBar) override;
	HRESULT	TxGetPasswordChar(wchar_t *pch) override;
	HRESULT	TxGetAcceleratorPos(LONG *pcp) override;
	HRESULT	TxGetExtent(LPSIZEL lpExtent) override;
	HRESULT  OnTxCharFormatChange(const CHARFORMATW * pcf) override;
	HRESULT	OnTxParaFormatChange(const PARAFORMAT * ppf) override;
	HRESULT	TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits) override;
	HRESULT	TxNotify(DWORD iNotify, void *pv) override;
	HIMC     TxImmGetContext() override;
	void     TxImmReleaseContext(HIMC himc) override;
	HRESULT	TxGetSelectionBarWidth(LONG *lSelBarWidth) override;

	// Custom functions
	HRESULT  CharFormatFromHFONT(CHARFORMAT2W* pCF, HFONT hFont);
	HRESULT  InitDefaultCharFormat();
	HRESULT  InitDefaultParaFormat();
};

void bbCodeParse(CFormattedTextDraw *ts);

#endif //__FORMATTEDTEXTDRAW_H_
