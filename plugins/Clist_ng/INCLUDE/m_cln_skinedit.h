
/*
 * services
 */

#define MS_CLNSE_INVOKE "CLN_Skinedit/Invoke"
#define MS_CLNSE_FILLBYCURRENTSEL "CLN_Skinedit/FillByCurrentSel"

/*
 * data structs
 */

struct TImageItem {
    char    szName[40];
    HBITMAP hbm;
    BYTE    bLeft, bRight, bTop, bBottom;      // sizing margins
    BYTE    alpha;
    DWORD   dwFlags;
    HDC     hdc;
    HBITMAP hbmOld;
    LONG    inner_height, inner_width;
    LONG    width, height;
    BLENDFUNCTION bf;
    BYTE    bStretch;
    HBRUSH  fillBrush;
    LONG    glyphMetrics[4];
    TImageItem *nextItem;
};

struct TButtonItem {
    char    szName[40];
    HWND    hWnd;
    LONG    xOff, yOff;
    //LONG	xOff_framed, yOff_framed;
    LONG    width, height;
    TImageItem *imgNormal, *imgPressed, *imgHover;
    LONG    normalGlyphMetrics[4];
    LONG    hoverGlyphMetrics[4];
    LONG    pressedGlyphMetrics[4];
    DWORD   dwFlags, dwStockFlags;
    DWORD   uId;
    TCHAR   szTip[256];
    char    szService[256];
    char    szModule[256], szSetting[256];
    BYTE    bValuePush[256], bValueRelease[256];
    DWORD   type;
    void    (*pfnAction)(TButtonItem *item, HWND hwndDlg, struct MessageWindowData *dat, HWND hwndItem);
    void    (*pfnCallback)(TButtonItem *item, HWND hwndDlg, struct MessageWindowData *dat, HWND hwndItem);
    TCHAR   tszLabel[40];
    TButtonItem *nextItem;
};

typedef struct _tagButtonSet {
    TButtonItem  *items;
    LONG        left, top, right, bottom;               // client area offsets, calculated from button layout
} ButtonSet;

typedef agg::span_gradient<agg::rgba8, agg::span_interpolator_linear<>, agg::gradient_x, agg::pod_auto_array<agg::rgba8, 256> > span_gradient_x_t;
typedef agg::span_gradient<agg::rgba8, agg::span_interpolator_linear<>, agg::gradient_y, agg::pod_auto_array<agg::rgba8, 256> > span_gradient_y_t;
typedef agg::span_allocator<agg::rgba8> span_allocator_t;

struct TStatusItem {
    char 	szName[40];
    int 	statusID;

    BYTE 	GRADIENT;
    BYTE 	CORNER;

    DWORD 	COLOR;
    DWORD 	COLOR2;
    BYTE 	ALPHA;
    BYTE 	ALPHA2;

    DWORD 	TEXTCOLOR;

    int 	MARGIN_LEFT;
    int 	MARGIN_TOP;
    int 	MARGIN_RIGHT;
    int 	MARGIN_BOTTOM;
    BYTE 	IGNORED;
    DWORD 	dwFlags;
    TImageItem *imageItem;

    agg::rasterizer_scanline_aa<>* 			agg_ras;
    agg::rounded_rect*						agg_rect;
	agg::pod_auto_array<agg::rgba8, 256>*	color_array;

	span_allocator_t*						span_allocator;
	span_gradient_x_t*						span_gradient_x;
	span_gradient_y_t*						span_gradient_y;
	agg::span_interpolator_linear<>*		span_interpolator;
	agg::gradient_x*						gradient_func_x;
	agg::gradient_y*						gradient_func_y;
	agg::trans_affine*						gradient_trans;
	agg::renderer_scanline_aa<agg::renderer_base<agg::pixfmt_bgra32>, span_allocator_t, span_gradient_x_t>* gradient_renderer_x;
	agg::renderer_scanline_aa<agg::renderer_base<agg::pixfmt_bgra32>, span_allocator_t, span_gradient_y_t>* gradient_renderer_y;
	agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> >* solid_renderer;

	agg::renderer_base<agg::pixfmt_bgra32>*	rbase;
	agg::pixfmt_bgra32*						pixfmt;
	agg::rounded_rect*						rect;
};

typedef struct {
    BOOL bGRADIENT;
    BOOL bCORNER;
    BOOL bCOLOR;
    BOOL bCOLOR2;
    BOOL bTEXTCOLOR;
    BOOL bALPHA;
    BOOL bMARGIN_LEFT;
    BOOL bMARGIN_TOP;
    BOOL bMARGIN_RIGHT;
    BOOL bMARGIN_BOTTOM;
    BOOL bIGNORED;
    BOOL bALPHA2;
    BOOL bFlags;
} ChangedSItems_t;

typedef struct _tagSkinDescription {
    DWORD           cbSize;
    TStatusItem   *StatusItems;
    int             lastItem;
    int             firstItem;
    char            szModule[100];
    HWND            hWndParent, hWndTab;
    HWND            hwndCLUI;
    HWND            hwndSkinEdit;                       /* out param */
    HWND            hwndImageEdit;                      /* out param */
    HMENU           hMenuItems;
    void            (*pfnSaveCompleteStruct)(void);
    void            (*pfnClcOptionsChanged )(void);
    void*           (*pfnMalloc)(unsigned int);
    void            (*pfnFree)(void);
    void*           (*pfnRealloc)(void *, unsigned int);
    void*           reserved[20];
} SKINDESCRIPTION;

// defines

// FLAGS
#define CORNER_NONE 0
#define CORNER_ACTIVE 1
#define CORNER_TL 2
#define CORNER_TR 4
#define CORNER_BR 8
#define CORNER_BL 16

#define GRADIENT_NONE 0
#define GRADIENT_ACTIVE 1
#define GRADIENT_LR 2
#define GRADIENT_RL 4
#define GRADIENT_TB 8
#define GRADIENT_BT 16

#define IMAGE_PERPIXEL_ALPHA 1
#define IMAGE_FLAG_DIVIDED 2
#define IMAGE_FILLSOLID 4
#define IMAGE_GLYPH 8

#define IMAGE_STRETCH_V 1
#define IMAGE_STRETCH_H 2
#define IMAGE_STRETCH_B 4

#define BUTTON_ISINTERNAL 			0x1
#define BUTTON_ISTOGGLE 			0x2
#define BUTTON_ISSERVICE 			0x4
#define BUTTON_ISPROTOSERVICE 		0x8
#define BUTTON_PASSHCONTACTW 		0x10
#define BUTTON_PASSHCONTACTL 		0x20
#define BUTTON_ISDBACTION    		0x40
#define BUTTON_ISCONTACTDBACTION 	0x80
#define BUTTON_DBACTIONONCONTACT 	0x100
#define BUTTON_ISSIDEBAR 			0x200
#define BUTTON_NORMALGLYPHISICON 	0x400
#define BUTTON_PRESSEDGLYPHISICON 	0x800
#define BUTTON_HOVERGLYPHISICON 	0x1000
#define BUTTON_HASLABEL 			0x2000
#define BUTTON_HALIGN_R 			0x4000
#define BUTTON_VALIGN_B 			0x8000
#define BUTTON_FRAMELESS_ONLY 		0x10000
#define BUTTON_FAKE_CAPTIONBUTTON 	0x20000

#define CLCDEFAULT_GRADIENT 0
#define CLCDEFAULT_CORNER 0

#define CLCDEFAULT_COLOR 0xd0d0d0
#define CLCDEFAULT_COLOR2 0xd0d0d0

#define CLCDEFAULT_TEXTCOLOR 0x000000

#define CLCDEFAULT_COLOR2_TRANSPARENT 1

#define CLCDEFAULT_ALPHA 100
#define CLCDEFAULT_MRGN_LEFT 0
#define CLCDEFAULT_MRGN_TOP 0
#define CLCDEFAULT_MRGN_RIGHT 0
#define CLCDEFAULT_MRGN_BOTTOM 0
#define CLCDEFAULT_IGNORE 1

HWND SkinEdit_Invoke(WPARAM wParam, LPARAM lParam);
