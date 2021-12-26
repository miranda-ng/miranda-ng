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
// IFormatttedTextDraw
interface IFormattedTextDraw
{
public:
	virtual ~IFormattedTextDraw() {};
	virtual HRESULT get_NaturalSize(void *hdcDraw, long *Width, long *pVal) = 0;
	virtual HRESULT Create() = 0;
	virtual HRESULT Draw(void *hdcDraw, RECT *prc) = 0;
	virtual HRESULT putRTFTextA(char *newVal) = 0;
	virtual HRESULT putRTFTextW(wchar_t *newVal) = 0;
	virtual HRESULT putTextA(char *newVal) = 0;
	virtual HRESULT putTextW(wchar_t *newVal) = 0;

	virtual ITextServices *getTextService() = 0;
	virtual ITextDocument *getTextDocument() = 0;
	virtual void setParentWnd(HWND hwnd, RECT rect) = 0;

	// COM-like functions
	virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// CFormattedTextDraw
class CFormattedTextDraw :
	public ITextHost,
	public IFormattedTextDraw
{
public:
	CFormattedTextDraw()
	{
		HDC hdcScreen;

		hdcScreen = GetDC(nullptr);
		nPixelsPerInchX = GetDeviceCaps(hdcScreen, LOGPIXELSX);
		nPixelsPerInchY = GetDeviceCaps(hdcScreen, LOGPIXELSY);
		ReleaseDC(nullptr, hdcScreen);

		SetRectEmpty(&m_rcClient);
		SetRectEmpty(&m_rcViewInset);

		m_pCF = (CHARFORMAT2W*)malloc(sizeof(CHARFORMAT2W));

		InitDefaultCharFormat();
		InitDefaultParaFormat();
		m_spTextServices = nullptr;
		m_spTextDocument = nullptr;

		m_dwPropertyBits = TXTBIT_RICHTEXT | TXTBIT_MULTILINE | TXTBIT_WORDWRAP | TXTBIT_USECURRENTBKG;
		m_dwScrollbar = 0;
		m_dwMaxLength = INFINITE;
	}

	~CFormattedTextDraw()
	{
		free(m_pCF);
		if (m_spTextServices != nullptr)
			m_spTextServices->Release();
		if (m_spTextDocument != nullptr)
			m_spTextDocument->Release();
	}

	// Minimal COM functionality
	HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID,
		/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		*ppvObject = nullptr;
		return S_FALSE;
	}

	ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return 0;
	}

	ULONG STDMETHODCALLTYPE Release(void)
	{
		return 0;
	}

	// IFormattedTextDraw
public:
	HRESULT get_NaturalSize(void *hdcDraw, long *Width, long *pVal);
	HRESULT Create();
	HRESULT Draw(void *hdcDraw, RECT *prc);
	HRESULT putRTFTextA(char *newVal);
	HRESULT putRTFTextW(wchar_t *newVal);
	HRESULT putTextA(char *newVal);
	HRESULT putTextW(wchar_t *newVal);

	ITextServices *getTextService() { return m_spTextServices; };
	ITextDocument *getTextDocument() { return m_spTextDocument; };
	virtual void setParentWnd(HWND hwnd, RECT rect) { m_hwndParent = hwnd; m_rcClient = rect; }

	// ITextHost
	HDC TxGetDC();
	INT TxReleaseDC(HDC hdc);
	BOOL TxShowScrollBar(INT fnBar, BOOL fShow);
	BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags);
	BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw);
	BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw);
	void TxInvalidateRect(LPCRECT prc, BOOL fMode);
	void TxViewChange(BOOL fUpdate);
	BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);
	BOOL TxShowCaret(BOOL fShow);
	BOOL TxSetCaretPos(INT x, INT y);
	BOOL TxSetTimer(UINT idTimer, UINT uTimeout);
	void TxKillTimer(UINT idTimer);
	void TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll);
	void TxSetCapture(BOOL fCapture);
	void TxSetFocus();
	void TxSetCursor(HCURSOR hcur, BOOL fText);
	BOOL TxScreenToClient(LPPOINT lppt);
	BOOL TxClientToScreen(LPPOINT lppt);
	HRESULT	TxActivate(LONG * plOldState);
	HRESULT	TxDeactivate(LONG lNewState);
	HRESULT	TxGetClientRect(LPRECT prc);
	HRESULT	TxGetViewInset(LPRECT prc);
	HRESULT  TxGetCharFormat(const CHARFORMATW **ppCF);
	HRESULT	TxGetParaFormat(const PARAFORMAT **ppPF);
	COLORREF TxGetSysColor(int nIndex);
	HRESULT	TxGetBackStyle(TXTBACKSTYLE *pstyle);
	HRESULT	TxGetMaxLength(DWORD *plength);
	HRESULT	TxGetScrollBars(DWORD *pdwScrollBar);
	HRESULT	TxGetPasswordChar(wchar_t *pch);
	HRESULT	TxGetAcceleratorPos(LONG *pcp);
	HRESULT	TxGetExtent(LPSIZEL lpExtent);
	HRESULT  OnTxCharFormatChange(const CHARFORMATW * pcf);
	HRESULT	OnTxParaFormatChange(const PARAFORMAT * ppf);
	HRESULT	TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits);
	HRESULT	TxNotify(DWORD iNotify, void *pv);
	HIMC TxImmGetContext();
	void TxImmReleaseContext(HIMC himc);
	HRESULT	TxGetSelectionBarWidth(LONG *lSelBarWidth);

	// Custom functions
	HRESULT CharFormatFromHFONT(CHARFORMAT2W* pCF, HFONT hFont);
	HRESULT InitDefaultCharFormat();
	HRESULT InitDefaultParaFormat();
	HRESULT CreateTextServicesObject();

	// Variables
	HWND			m_hwndParent;
	RECT			m_rcClient;			// Client Rect
	RECT			m_rcViewInset;		// view rect inset
	SIZEL			m_sizelExtent;		// Extent array

	int				nPixelsPerInchX;    // Pixels per logical inch along width
	int				nPixelsPerInchY;    // Pixels per logical inch along height

	CHARFORMAT2W	*m_pCF;
	PARAFORMAT2		m_PF;
	uint32_t			m_dwScrollbar;		// Scroll bar style
	uint32_t			m_dwPropertyBits;	// Property bits
	uint32_t			m_dwMaxLength;
	COOKIE			m_editCookie;

	ITextServices	*m_spTextServices;
	ITextDocument	*m_spTextDocument;
};

void bbCodeParse(IFormattedTextDraw *ts);

#endif //__FORMATTEDTEXTDRAW_H_
