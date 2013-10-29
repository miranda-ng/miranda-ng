#ifndef _LCDINPUTTEXT_H_INCLUDED_ 
#define _LCDINPUTTEXT_H_INCLUDED_ 

#define MARKER_HOLD 0
#define MARKER_HORIZONTAL 1
#define MARKER_VERTICAL 2

#define KEYS_RETURN 0
#define KEYS_CTRL_RETURN 1

#include "CLCDTextObject.h"
#include "CLCDBar.h"

struct SMarker
{
	int iPosition;
	int iXLine;
	int iXWidth;
	int iLine;
};

struct SLineEntry
{
	int iOffset;
	int iWidth;
	bool bLineBreak;
};

class CLineEntry
{
public:
	tstring strLine;
	vector<int> vWrapLineOffsets;
};

class CLCDInput : public CLCDTextObject
{
public:
    CLCDInput();
    ~CLCDInput();

    bool Initialize();
    bool Shutdown();

    bool Draw(CLCDGfx *pGfx);
	bool Update();

	void ShowSymbols(bool bShow);
	void SetBreakKeys(int iKeys);
	void SetScrollbar(CLCDBar *pScrollbar);

	void ActivateInput();
	void DeactivateInput();
	bool IsInputActive();

	tstring GetText();
	void Reset();

	static LRESULT CALLBACK KeyboardHook(int Code, WPARAM wParam, LPARAM lParam);
	
	int GetLineCount();
	int GetLine();

	bool ScrollLine(bool bDown=true);
	void ScrollToMarker();
	long GetLastInputTime();

protected:
	void OnFontChanged();
	void OnSizeChanged(SIZE OldSize);

	LRESULT ProcessKeyEvent(int Code, WPARAM wParam, LPARAM lParam);
	
	void UpdateOffsets(int iModified);

	void UpdateMarker();
	void MoveMarker(int iDir,int iMove,bool bShift=false);
protected:
	bool m_bShowSymbols;
	bool m_iBreakKeys;

	int m_iLineCount;
	long m_lInputTime;
	bool m_bInsert;
	
	long m_lBlinkTimer;
	bool m_bShowMarker;

	int m_iLinePosition;
	SMarker m_Marker[2];

	// Text variables
	tstring m_strText;
	vector<SLineEntry> m_vLineOffsets;

	// Input variables
	static CLCDInput* m_pInput;
	HHOOK m_hKBHook;
	BYTE m_acKeyboardState[256];

	// Scrollbar
	CLCDBar *m_pScrollbar;
};


#endif // !_LCDTEXT_H_INCLUDED_ 

//** end of LCDText.h ****************************************************
