#ifndef __history_control_h__
#define __history_control_h__

#define NEWSTORYLIST_CLASS "NewstoryList"

struct NewstoryListData;

class NSWebPage : public document_container
{
	typedef std::map<std::wstring, uint_ptr> images_map;

	ULONG_PTR m_gdiplusToken;

	mir_cs m_csImages;
	images_map m_images;

	position::vector	m_clips;
	HRGN m_hClipRgn;
	std::set<std::wstring> m_installed_fonts;
	HDC m_tmp_hdc;

	NewstoryListData &ctrl;

	std::string resolve_color(const string &color) const;
	uint_ptr	get_image(LPCWSTR url_or_path, bool redraw_on_ready);
	void make_url(LPCWSTR url, LPCWSTR basepath, std::wstring &out);

	void get_client_rect(position &client) const override;
	void import_css(string &text, const string &url, string &baseurl) override;
	void on_anchor_click(const char *url, const element::ptr &el) override;
	void set_base_url(const char *base_url) override;
	void set_caption(const char *caption) override;
	void set_cursor(const char *cursor) override;

	void draw_image(uint_ptr hdc, const background_layer &layer, const std::string &url, const std::string &base_url) override;
	void get_img_size(uint_ptr img, size &sz);
	void free_image(uint_ptr img);

	// document_container members
	uint_ptr create_font(const char *faceName, int size, int weight, font_style italic, unsigned int decoration, font_metrics *fm) override;
	void delete_font(uint_ptr hFont) override;
	const char* get_default_font_name() const override;
	int  get_default_font_size() const override;
	
	void draw_text(uint_ptr hdc, const char *text, uint_ptr hFont, web_color color, const position &pos) override;
	int  text_width(const char *text, uint_ptr hFont) override;
	void transform_text(string &text, text_transform tt) override;

	void draw_borders(uint_ptr hdc, const borders &borders, const position &draw_pos, bool root) override;
	void draw_ellipse(HDC hdc, int x, int y, int width, int height, web_color color, int line_width);
	void draw_list_marker(uint_ptr hdc, const list_marker &marker) override;
	void draw_solid_fill(uint_ptr, const background_layer &, const web_color &) override;

	void draw_linear_gradient(uint_ptr, const background_layer &, const background_layer::linear_gradient &) override;
	void draw_radial_gradient(uint_ptr, const background_layer &, const background_layer::radial_gradient &) override;
	void draw_conic_gradient(uint_ptr, const background_layer &, const background_layer::conic_gradient &) override;

	void fill_ellipse(HDC hdc, int x, int y, int width, int height, web_color color);
	void fill_rect(HDC hdc, int x, int y, int width, int height, web_color color);

	int  pt_to_px(int pt) const override;

	void add_image(LPCWSTR url, uint_ptr img);
	void load_image(const char *src, const char *baseurl, bool redraw_on_ready) override;
	void get_image_size(const char *src, const char *baseurl, size &sz) override;

	element::ptr create_element(const char *tag_name, const string_map &attributes, const document::ptr &doc) override;
	void get_media_features(media_features &media) const override;
	void get_language(string &language, string &culture) const override;
	void link(const document::ptr &doc, const element::ptr &el) override;

	void apply_clip(HDC hdc);
	void del_clip() override;
	void release_clip(HDC hdc);
	void set_clip(const position &pos, const border_radiuses &bdr_radius) override;

	void make_url_utf8(const char *url, const char *basepath, std::wstring &out);

	void clear_images();

	static int CALLBACK EnumFontsProc(const LOGFONT *lplf, const TEXTMETRIC *lptm, DWORD dwType, LPARAM lpData);

public:
	NSWebPage(NewstoryListData &_1);
	~NSWebPage();

	COLORREF clText = -1, clBack = -1;
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

	bool bWasShift, bSortAscending, bWasAtBottom, bReadOnly;

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
	void      Paint(simpledib::dib &dib);
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
