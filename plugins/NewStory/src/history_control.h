#ifndef __history_control_h__
#define __history_control_h__

#define NEWSTORYLIST_CLASS "NewstoryList"

struct ADDEVENTS
{
	MCONTACT hContact;
	MEVENT hFirstEVent;
	int eventCount;
};

enum
{
	NSM_FIRST = WM_USER + 100,

	// wParam = fist item
	// lParam = last item
	// result = number of total selected items
	NSM_SELECTITEMS = NSM_FIRST,

	// wParam = fist item
	// lParam = last item
	// result = number of total selected items
	NSM_TOGGLEITEMS,

	// wParam = fist item
	// lParam = last item
	// result = number of total selected items
	// select items wParam - lParam and deselect all other
	NSM_SELECTITEMS2,

	// wParam = fist item
	// lParam = last item
	// result = number of total selected items
	NSM_DESELECTITEMS,

	// wParam = item id
	NSM_ENSUREVISIBLE,

	// wParam = x in control
	// lParam = y in control
	// result = id
	NSM_GETITEMFROMPIXEL,

	// add one or more events
	NSM_ADDEVENTS,
	NSM_ADDCHATEVENT,

	// clear log
	NSM_CLEAR,

	// wParam = id
	NSM_SETCARET,

	// result = id
	NSM_GETCARET,

	// wParam = text
	NSM_FINDNEXT,
	NSM_FINDPREV,

	// wParam = wtext
	NSM_FINDNEXTW,
	NSM_FINDPREVW,

	//
	NSM_COPY,
	NSM_EXPORT,

	//
	NSM_GETCOUNT,
	NSM_GETARRAY,

	//
	NSM_SEEKEND,
	NSM_SEEKTIME,

	// 
	NSM_SET_SRMM,     // act inside SRMM dialog
	NSM_SET_CONTACT,

	NSM_LAST
};

struct NewstoryListData : public MZeroedObject
{
	NewstoryListData(HWND _1) :
		hwnd(_1),
		redrawTimer(Miranda_GetSystemWindow(), (LPARAM)this)
	{
		redrawTimer.OnEvent = Callback(this, &NewstoryListData::OnTimer);
	}

	HistoryArray items;

	int scrollTopItem; // topmost item
	int scrollTopPixel; // y coord of topmost item, this should be negative or zero
	int caret;
	int selStart = -1;
	int cachedWindowHeight;
	int cachedMaxTopItem; // the largest ID of top item to avoid empty space
	int cachedMaxTopPixel;

	RECT rcLastPaint;

	bool bWasShift;

	HWND hwnd;
	HWND hwndEditBox;

	CTimer redrawTimer;
	CSrmmBaseDialog *pMsgDlg = nullptr;

	void OnContextMenu(int index, POINT pt);
	void OnTimer(CTimer *pTimer);
	void BeginEditItem(int index, bool bReadOnly);
	void DeleteItems(void);
	void EndEditItem(bool bAccept);
	void EnsureVisible(int item);
	void FixScrollPosition();
	int  GetItemFromPixel(int yPos);
	int  LayoutItem(int index);
	int  PaintItem(HDC hdc, int index, int top, int width);
	void RecalcScrollBar();
	void ScheduleDraw();
	void ScrollListBy(int scrollItems, int scrollPixels);
	void SetPos(int pos);
};

void InitNewstoryControl();

#endif // __history_control_h__
