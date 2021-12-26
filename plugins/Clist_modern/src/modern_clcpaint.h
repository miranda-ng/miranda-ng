#ifndef modern_clcpaint_h__
#define modern_clcpaint_h__

extern class CLCPaint g_clcPainter;
class CLCPaint
{

public:
	CLCPaint();
	~CLCPaint() {};

	CLINTERFACE void   cliPaintClc(HWND hwnd, ClcData *dat, HDC hdc, RECT *rcPaint);
	CLINTERFACE tPaintCallbackProc PaintCallbackProc(HWND hWnd, HDC hDC, RECT *rcPaint, HRGN rgn, uint32_t dFlags, void * CallBackData);

	BOOL   IsForegroundWindow(HWND hWnd);
	HFONT  ChangeToFont(HDC hdc, ClcData *dat, int id, int *fontHeight);
	int    GetBasicFontID(ClcContact *contact);
	void   GetTextSize(SIZE *text_size, HDC hdcMem, RECT free_row_rc, wchar_t *szText, SortedList *plText, UINT uTextFormat, int smiley_height);
	void   AddParam(MODERNMASK *mpModernMask, uint32_t dwParamHash, const char* szValue, uint32_t dwValueHash);
	BOOL   CheckMiniMode(ClcData *dat, BOOL selected);

private:
	static const int HORIZONTAL_SPACE;
	static const int EXTRA_CHECKBOX_SPACE;
	static const int EXTRA_SPACE;
	static const int SELECTION_BORDER;
	static const int MIN_TEXT_WIDTH;
	static const int BUF2SIZE;

	static const uint8_t GIM_SELECTED_AFFECT;
	static const uint8_t GIM_HOT_AFFECT;
	static const uint8_t GIM_TEMP_AFFECT;
	static const uint8_t GIM_IDLE_AFFECT;
	static const uint8_t GIM_EXTRAICON_AFFECT;
	static const uint8_t GIM_STATUSICON_AFFECT;
	static const uint8_t GIM_AVATAR_AFFECT;

	enum tagenumHASHINDEX
	{
		hi_Module = 0,
		hi_ID,
		hi_Type,
		hi_Open,
		hi_IsEmpty,
		hi_SubPos,
		hi_Protocol,
		hi_RootGroup,
		hi_Status,
		hi_HasAvatar,
		hi_GroupPos,
		hi_Selected,
		hi_Hot,
		hi_Odd,
		hi_Indent,
		hi_Index,
		hi_Name,
		hi_Group,
		hi_True,
		hi_False,
		hi_ONLINE,
		hi_AWAY,
		hi_DND,
		hi_NA,
		hi_OCCUPIED,
		hi_FREECHAT,
		hi_INVISIBLE,
		hi_IDLE,
		hi_OFFLINE,
		hi_Row,
		hi_CL,
		hi_SubContact,
		hi_MetaContact,
		hi_Contact,
		hi_Divider,
		hi_Info,
		hi_First_Single,
		hi_First,
		hi_Middle,
		hi_Mid,
		hi_Single,
		hi_Last,
		hi_Rate,
		hi_None,
		hi_Low,
		hi_Medium,
		hi_High,
		hi_State,
		hi_stActive,
		hi_stInactive,
		//ADD new item above here
		hi_LastItem
	} enumHASHINDEX;
	static const char* HASHTEXT[hi_LastItem];
	static uint32_t HASH[hi_LastItem];

	void _FillQuickHash();
	void _SetHotTrackColour(HDC hdc, ClcData *dat);
	RECT _GetRectangle(ClcData *dat, RECT *row_rc, RECT *free_row_rc, int *left_pos, int *right_pos, BOOL left, int real_width, int width, int height, int horizontal_space);
	void _DrawTextSmiley(HDC hdcMem, RECT *free_rc, SIZE * text_size, wchar_t *szText, int start, int len, SortedList *plText, UINT uTextFormat, BOOL ResizeSizeSmiley);
	void _AddParameter(MODERNMASK *mpModernMask, MASKPARAM * lpParam);
	void _AddParamShort(MODERNMASK *mpModernMask, uint32_t dwParamIndex, uint32_t dwValueIndex);
	void _FillParam(MASKPARAM * lpParam, uint32_t dwParamHash, const char* szValue, uint32_t dwValueHash);
	MODERNMASK* _GetCLCContactRowBackModernMask(ClcGroup *group, ClcContact *Drawing, int indent, int index, BOOL selected, BOOL hottrack, ClcData *dat);
	void _RTLRect(RECT *rect, int width);
	void _PaintRowItemsEx(HDC hdcMem, ClcData *dat, ClcContact *Drawing, RECT row_rc, RECT free_row_rc, int selected, int hottrack);
	void _DrawStatusIcon(ClcContact *Drawing, ClcData *dat, int iImage, HDC hdcMem, int x, int y, int cx, int cy, uint32_t colorbg, uint32_t colorfg, int mode);
	BOOL _DrawNonEnginedBackground(HDC hdcMem, RECT *rcPaint, RECT clRect, ClcData *dat);
	void _PaintClc(HWND hwnd, ClcData *dat, HDC hdc, RECT *rcPaint);
	void _StoreItemPos(ClcContact *contact, int ItemType, RECT *rc);
	void _CalcItemsPos(HDC hdcMem, ClcData *dat, ClcContact *Drawing, RECT *in_row_rc, RECT *in_free_row_rc, int left_pos, int right_pos, int selected);
	BOOL __IsVisible(RECT *firtRect, RECT *secondRect);
	void _GetBlendMode(IN ClcData *dat, IN ClcContact *Drawing, IN BOOL selected, IN BOOL hottrack, IN BOOL bFlag, OUT COLORREF * OutColourFg, OUT int * OutMode);
	void _DrawContactItems(HDC hdcMem, ClcData *dat, ClcContact *Drawing, RECT *row_rc, RECT *free_row_rc, int selected, int hottrack, RECT *rcPaint);
	void _PaintRowItems(HDC hdcMem, ClcData *dat, ClcContact *Drawing, RECT row_rc, RECT free_row_rc, int left_pos, int right_pos, int selected, int hottrack, RECT *rcPaint);

	void _DrawContactAvatar(HDC hdcMem, ClcData *dat, ClcContact *Drawing, RECT *row_rc, int& selected, int& hottrack, RECT *prcItem);
	void _DrawContactIcon(HDC hdcMem, ClcData *dat, ClcContact *Drawing, int& selected, int& hottrack, RECT *prcItem);
	void _DrawContactText(HDC hdcMem, ClcData *dat, ClcContact *Drawing, int& selected, int& hottrack, RECT& text_rc, RECT *prcItem, UINT uTextFormat);
	void _DrawContactSubText(HDC hdcMem, ClcData *dat, ClcContact *Drawing, int& selected, int& hottrack, RECT& text_rc, RECT *prcItem, UINT uTextFormat, uint8_t itemType);
	void _DrawContactTime(HDC hdcMem, ClcData *dat, ClcContact *Drawing, RECT *prcItem);
	void _DrawContactExtraIcon(HDC hdcMem, ClcData *dat, ClcContact *Drawing, int& selected, int& hottrack, RECT *rc, int iImage);
	void _DrawContactSelection(HDC hdcMem, ClcData *dat, int& selected, int& hottrack, RECT *rcPaint, RECT *prcItem);
	void _DrawContactLine(HDC hdcMem, ClcContact *Drawing, RECT *free_row_rc, RECT *rcPaint, RECT& text_rc);

	int _rcWidth(RECT *rc) { return rc->right - rc->left; }
	int _rcHeight(RECT *rc) { return rc->bottom - rc->top; }

private:
	enum enDrawMode
	{
		DM_LAYERED = 1,        // Draw normal skin on 32 bit with alpha layer
		DM_NON_LAYERED = 2,        // Draw skinnable, but does not take care about alpha       
		DM_CLASSIC = 4,        // Old style draw for classic
		DM_CONTROL = 8,        // Draw as control according to windows color scheme
		DM_FLOAT = 16,       // Float mode 
		DM_GRAY = 32,       // Grayed mode
		DM_GREYALTERNATE = 64,       // Gray odd lines
		DM_DRAW_OFFSCREEN = DM_FLOAT | DM_CONTROL | DM_NON_LAYERED | DM_CLASSIC,
	};

	inline int _DetermineDrawMode(HWND hWnd, ClcData *dat);

	struct _PaintContext
	{
		enum
		{
			release_none = 0,
			release_hdcmem2 = 1,
			release_hdcmem = 2
		};
		HDC hdcMem;
		HDC hdcMem2;

		HBITMAP hBmpOsb2;
		HBITMAP oldbmp2;

		HBITMAP hBmpOsb;
		HBITMAP oldbmp;

		HBRUSH   hBrushAlternateGrey;
		COLORREF tmpbkcolour;
		COLORREF tmpforecolour;

		uint32_t fRelease;
		_PaintContext(HDC _hdcMem = nullptr) :
			hdcMem(_hdcMem), hdcMem2(nullptr),
			hBmpOsb2(nullptr), oldbmp2(nullptr),
			hBmpOsb(nullptr), oldbmp(nullptr),
			hBrushAlternateGrey(nullptr),
			tmpbkcolour(0), tmpforecolour(0),
			fRelease(release_none) {};
	};

	inline void _PreparePaintContext(ClcData *dat, HDC hdc, int paintMode, RECT& clRect, _PaintContext& pc);
	inline void _DrawBackground(HWND hWnd, ClcData *dat, int paintMode, RECT* rcPaint, RECT& clRect, _PaintContext& pc);
	inline void _DrawLines(HWND hWnd, ClcData *dat, int paintMode, RECT* rcPaint, RECT& clRect, _PaintContext& pc);
	inline void _DrawInsertionMark(ClcData *dat, RECT& clRect, _PaintContext& pc);
	inline void _CopyPaintToDest(HDC hdc, int paintMode, RECT* rcPaint, _PaintContext& pc);
	inline void _FreePaintContext(_PaintContext& pc);
};


#endif // modern_clcpaint_h__
