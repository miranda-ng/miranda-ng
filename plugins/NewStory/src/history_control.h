#ifndef __history_control_h__
#define __history_control_h__

#define NEWSTORYLIST_CLASS "NewstoryList"

enum
{
	NSM_FIRST = WM_USER + 100,

	// wParam = fist item
	// lParam = iLast item
	// result = number of total selected items
	NSM_SELECTITEMS = NSM_FIRST,

	//
	NSM_SEEKTIME,

	// 
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
	int cachedMaxTopItem; // the largest ID of top item to avoid empty space
	int cachedMaxTopPixel;
	int cachedWindowWidth = -1, cachedWindowHeight = -1;
	int cachedMaxDrawnItem = -1;
	int cachedScrollbarPos = -1;
	unsigned int cachedScrollbarPage = -1;
	int totalCount;

	RECT rcLastPaint;

	bool bWasShift, bSortAscending, bWasAtBottom;

	HWND m_hwnd;
	HWND hwndEditBox;

	CTimer redrawTimer;
	CSrmmBaseDialog *pMsgDlg = nullptr;

	void      OnContextMenu(int index, POINT pt);
	void      OnResize(int newWidth, int newHeight);
	void      OnTimer(CTimer *pTimer);

	void      AddChatEvent(SESSION_INFO *si, const LOGINFO *lin);
	void      AddEvent(MCONTACT hContact, MEVENT hFirstEvent, int iCount);
	void      AddResults(const OBJLIST<SearchResult> &results);
	void      AddSelection(int iFirst, int iLast);
	bool      AtBottom(void) const;
	bool      AtTop(void) const;
	void      BeginEditItem(bool bReadOnly);
	void      Clear();
	void      ClearSelection(int iFirst, int iLast);
	void      Copy(bool bTextOnly = false);
	void      DeleteItems(void);
	void      Download(int iOptions);
	void      EndEditItem(bool bAccept);
	void      EnsureVisible(int item);
	int       FindNext(const wchar_t *pwszText);
	int       FindPrev(const wchar_t *pwszText);
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
	void      Quote();
	void      RecalcScrollBar();
	void      ScheduleDraw();
	void      ScrollTop();
	void      ScrollBottom();
	void      SetCaret(int idx, bool bEnsureVisible = true);
	void      SetContact(MCONTACT hContact);
	void      SetDialog(CSrmmBaseDialog *pDialog);
	void      SetPos(int pos);
	void      SetSelection(int iFirst, int iLast);
	void      ToggleBookmark();
	void      ToggleSelection(int iFirst, int iLast);
};

void InitNewstoryControl();

#endif // __history_control_h__
