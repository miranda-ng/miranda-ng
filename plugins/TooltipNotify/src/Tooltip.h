// Tooltip.h: interface for the CTooltip class.
//
//////////////////////////////////////////////////////////////////////

class CTooltipNotify;

class CTooltip
{
public:

	CTooltip(CTooltipNotify *pTooltipNotify);
	virtual ~CTooltip();

	HWND GetHandle() const { return m_hWnd; }
	void Hide();
	void Show();	
	void Validate();
	void set_Position(INT x, INT y);
	void get_Rect(RECT *Rect) const;
	void set_TransparentInput(BOOL bOnOff);
	void set_Translucency(BYTE bAlpha);
	void set_Text(const TCHAR* szText);
	void set_Font(const LOGFONT& Font) { m_lfFont = Font; }
	void set_TextColor(DWORD TextColor) { m_dwTextColor = TextColor; }
	void set_BgColor(DWORD BgColor) { m_dwBgColor = BgColor; }

	static void Initialize();
	static void Deinitialize();

private:
	// prohibit copying
	CTooltip(const CTooltip& rhs);
	CTooltip& operator= (const CTooltip& rhs);

private:
	static LRESULT CALLBACK WindowProcWrapper(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HWND m_hWnd;
	HFONT m_hFont;

	// tooltip parameters
	DWORD m_dwTextColor;
	DWORD m_dwBgColor;
	LOGFONT m_lfFont;
	TCHAR *m_szText;
	BYTE m_bAlpha;
	BOOL m_bTranspInput;
	BYTE m_bLDblClick;
	
	CTooltipNotify *m_pTooltipNotify;	
	static const TCHAR *s_szTooltipClass;
};
