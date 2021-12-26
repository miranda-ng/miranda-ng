// Tooltip.h: interface for the CTooltip class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

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
	void set_Translucency(uint8_t bAlpha);
	void set_Text(const wchar_t* szText);
	void set_Font(const LOGFONT& Font) { m_lfFont = Font; }
	void set_TextColor(uint32_t TextColor) { m_dwTextColor = TextColor; }
	void set_BgColor(uint32_t BgColor) { m_dwBgColor = BgColor; }

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
	uint32_t m_dwTextColor;
	uint32_t m_dwBgColor;
	LOGFONT m_lfFont;
	wchar_t *m_szText;
	uint8_t m_bAlpha;
	BOOL m_bTranspInput;
	uint8_t m_bLDblClick;
	
	CTooltipNotify *m_pTooltipNotify;	
	static const wchar_t *s_szTooltipClass;
};
