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
	// lParam = iLast item
	// result = number of total selected items
	NSM_SELECTITEMS = NSM_FIRST,

	// add one or more events
	NSM_ADDEVENTS,
	NSM_ADDCHATEVENT,
	NSM_ADDRESULTS,

	// clear log
	NSM_CLEAR,

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
	NSM_DOWNLOAD,

	//
	NSM_GETCOUNT,
	NSM_GETARRAY,

	//
	NSM_SEEKEND,
	NSM_SEEKTIME,

	// 
	NSM_SET_SRMM,     // act inside SRMM dialog
	NSM_SET_CONTACT,  // set hContact
	NSM_SET_OPTIONS,  // options were changed

	NSM_LAST
};

struct NewstoryListData : public MZeroedObject
{
	NewstoryListData(HWND);

	HistoryArray items;

	int scrollTopItem; // topmost item
	int scrollTopPixel; // y coord of topmost item, this should be negative or zero
	int caret;
	int selStart = -1;
	int cachedWindowHeight;
	int cachedMaxTopItem; // the largest ID of top item to avoid empty space
	int cachedMaxTopPixel;
	int cachedWindowWidth = -1;
	int cachedMaxDrawnItem = -1;
	int cachedScrollbarPos = -1;
	unsigned int cachedScrollbarPage = -1;
	int totalCount;

	RECT rcLastPaint;

	bool bWasShift, bSortAscending, hasData;

	HWND hwnd;
	HWND hwndEditBox;

	CTimer redrawTimer;
	CSrmmBaseDialog *pMsgDlg = nullptr;

	void      OnContextMenu(int index, POINT pt);
	void      OnResize(int newWidth);
	void      OnTimer(CTimer *pTimer);

	void      AddSelection(int iFirst, int iLast);
	void      BeginEditItem(int index, bool bReadOnly);
	void      ClearSelection(int iFirst, int iLast);
	void      DeleteItems(void);
	void      EndEditItem(bool bAccept);
	void      EnsureVisible(int item);
	void      FixScrollPosition(bool bForce = false);
	ItemData* GetItem(int idx);
	int       GetItemFromPixel(int yPos);
	int       GetItemHeight(int index);
	void      LineUp();
	void      LineDown();
	ItemData* LoadItem(int idx);
	void      PageUp();
	void      PageDown();
	int       PaintItem(HDC hdc, int index, int top, int width);
	void      RecalcScrollBar();
	void      ScheduleDraw();
	void      ScrollTop();
	void      ScrollBottom();
	void      SetCaret(int idx, bool bEnsureVisible = true);
	void      SetPos(int pos);
	void      SetSelection(int iFirst, int iLast);
	void      ToggleBookmark();
	void      ToggleSelection(int iFirst, int iLast);
};

void InitNewstoryControl();

#endif // __history_control_h__
