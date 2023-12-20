#ifndef __history_control_h__
#define __history_control_h__

#define NEWSTORYLIST_CLASS "NewstoryList"

struct NewstoryListData : public MZeroedObject
{
	NewstoryListData(HWND);

	mir_cs m_csItems;
	HistoryArray items;

	int scrollTopItem; // topmost item
	int scrollTopPixel; // y coord of topmost item, this should be negative or zero
	int caret;
	int selStart = -1;
	int cachedMaxTopItem; // the largest ID of top item to avoid empty space
	int cachedMaxTopPixel;
	int cachedWindowWidth = -1, cachedWindowHeight = -1;
	int cachedMaxDrawnItem = -1;
	int cachedScrollbarPos = -1, cachedScrollbarMax = -1;
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
	void      BeginEditItem();
	void      CalcBottom();
	void      Clear();
	void      ClearSelection(int iFirst, int iLast);
	void      Copy(bool bTextOnly = false);
	void      CopyUrl();
	void      DeleteItems(void);
	void      Download(int iOptions);
	void      EndEditItem(bool bAccept);
	void      EnsureVisible(int item);
	void      EventUp();
	void      EventDown();
	void      EventPageUp();
	void      EventPageDown();
	int       FindNext(const wchar_t *pwszText);
	int       FindPrev(const wchar_t *pwszText);
	void      FixScrollPosition(bool bForce = false);
	CMStringW GatherSelected(bool bTextOnly);
	ItemData* GetItem(int idx) const;
	int       GetItemFromPixel(int yPos);
	int       GetItemHeight(int index);
	int       GetItemHeight(ItemData *pItem);
	bool      HasSelection() const;
	void      HitTotal(int yCurr, int yTotal);
	void      LineUp();
	void      LineDown();
	ItemData* LoadItem(int idx);
	void      OpenFolder();
	void      PageUp();
	void      PageDown();
	int       PaintItem(HDC hdc, ItemData* pItem, int top, int width, bool bDraw);
	void      Quote();
	void      RecalcScrollBar();
	void      Reply();
	void      ScheduleDraw();
	void      ScrollBottom();
	void      ScrollDown(int deltaY);
	void      ScrollTop();
	void      ScrollUp(int deltaY);
	void      SetCaret(int idx, bool bEnsureVisible = true);
	void      SetContact(MCONTACT hContact);
	void      SetDialog(CSrmmBaseDialog *pDialog);
	void      SetPos(int pos);
	void      SetSelection(int iFirst, int iLast);
	void      ToggleBookmark();
	void      ToggleSelection(int iFirst, int iLast);
	void      TryUp(int iCount);
};

void InitNewstoryControl();

#endif // __history_control_h__
