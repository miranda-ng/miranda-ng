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
	VOID Hide();
	VOID Show();	
	VOID Validate();
	VOID set_Position(INT x, INT y);
	VOID get_Rect(RECT *Rect) const;
	VOID set_TransparentInput(BOOL bOnOff);
	VOID set_Translucency(BYTE bAlpha);
	VOID set_Text(const TCHAR* szText);
	VOID set_Font(const LOGFONT& Font) { m_lfFont = Font; }
	VOID set_TextColor(DWORD TextColor) { m_dwTextColor = TextColor; }
	VOID set_BgColor(DWORD BgColor) { m_dwBgColor = BgColor; }

	static void Initialize(HMODULE hInstance);
	static void Deinitialize(HMODULE hInstance);

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
