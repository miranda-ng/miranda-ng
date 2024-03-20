#ifndef __history_control_h__
#define __history_control_h__

#define NEWSTORYLIST_CLASS "NewstoryList"

struct NewstoryListData;

class NSWebPage : public windows_container
{
	NewstoryListData &ctrl;
	cairo_images_cache m_images;

	cairo_surface_t *get_image(const std::string &url) override;

	void get_client_rect(litehtml::position &client) const override;
	void import_css(litehtml::string &text, const litehtml::string &url, litehtml::string &baseurl) override;
	void load_image(const char *src, const char *baseurl, bool redraw_on_ready) override;
	void on_anchor_click(const char *url, const litehtml::element::ptr &el) override;
	void set_base_url(const char *base_url) override;
	void set_caption(const char *caption) override;
	void set_clip(const litehtml::position &pos, const litehtml::border_radiuses &bdr_radius) override;
	void set_cursor(const char *cursor) override;

	void on_image_loaded(const wchar_t *file, const wchar_t *url, bool redraw_only);

public:
	NSWebPage(NewstoryListData &_1) :
		ctrl(_1)
	{}

	litehtml::uint_ptr getRC() const { return (litehtml::uint_ptr)m_temp_cr; }
};

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
	MCONTACT m_hContact = INVALID_CONTACT_ID;

	bool bWasShift, bSortAscending, bWasAtBottom;

	HWND m_hwnd;
	HWND hwndEditBox;

	CTimer redrawTimer;
	CSrmmBaseDialog *pMsgDlg = nullptr;

	NSWebPage webPage;
	simpledib::dib dib;

	void      OnContextMenu(int index, POINT pt);
	void      OnResize(int newWidth, int newHeight);
	
	void      onTimer_Draw(CTimer *pTimer);

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
	void      CopyPath();
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
	CMStringA GatherSelectedRtf();
	ItemData* GetItem(int idx) const;
	int       GetItemFromPixel(int yPos);
	int       GetItemHeight(int index);
	int       GetItemHeight(ItemData *pItem);
	bool      HasSelection() const;
	void      HitTotal(int yCurr, int yTotal);
	void      LineUp();
	void      LineDown();
	ItemData* LoadItem(int idx);
	void      MarkRead(ItemData *pItem);
	void      OpenFolder();
	void      PageUp();
	void      PageDown();
	void      Paint(simpledib::dib &dib, RECT *rcDraw);
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
