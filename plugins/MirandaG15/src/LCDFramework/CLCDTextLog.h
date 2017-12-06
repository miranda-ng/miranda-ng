#ifndef _CLCDTEXTLOG_H_
#define _CLCDTEXTLOG_H_

#include "CLCDTextObject.h"
#include "CLCDBar.h"


enum EScrollMode {SCROLL_NONE, SCROLL_MESSAGE,SCROLL_LINE };
enum EExpandMode { EXPAND_SCROLL, EXPAND_UP,EXPAND_DOWN };

class CLCDTextLog : public CLCDTextObject
{
public:
	

	// Constructor
	CLCDTextLog();
	// Destructor
	~CLCDTextLog();

	// Initializes the log
	bool Initialize();
	// Deinitializes the log
	bool Shutdown();

	// updates the log
	bool Update();
	// draws the log
	bool Draw(CLCDGfx *pGfx);

	// sets the logs text
	void SetText(tstring strText);
	// adds some text to the log
	void AddText(tstring strText,bool bForceAutoscroll=false);
	// sets the maximum number of log entrys
	void SetLogSize(int iSize);
	// clears the log
	void ClearLog();

	// scrolls one line up
	bool ScrollUp();
	// scrolls one line down
	bool ScrollDown();
	// scrolls to the specified line
	bool ScrollTo(int iIndex);

	// associates a scrollbar with the log
	void SetScrollbar(CLCDBar *pScrollbar);

	// sets the autoscrolling mode
	void SetAutoscrollMode(EScrollMode eMode);
	// sets the expand mode
	void SetExpandMode(EExpandMode eMode);

protected:
	// called when the logs font has changed
	void OnFontChanged();
	// called when the logs size has changed
	void OnSizeChanged(SIZE OldSize);

	// rewraps all textlines
	void RefreshLines();
private:
	// the log entry class
	class CLogEntry
	{
	public:
		tstring strString;
		int iLines;
		vector<tstring> vLines;
	};

	// wraps the specified log entry
	void WrapMessage(CLogEntry *pEntry);

	EScrollMode m_eAutoScroll;
	EExpandMode m_eExpandMode;

	int		m_iLogSize;
	int		m_iPosition;
	int		m_iTextLines;
	int		m_iLastScrollDirection;
	
	DWORD	m_dwLastScroll;

	list<CLogEntry*> m_Entrys;
	CLCDBar			*m_pScrollbar;
};

#endif