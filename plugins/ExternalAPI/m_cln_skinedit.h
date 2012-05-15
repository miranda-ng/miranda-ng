
/*
 * services
 */

#define MS_CLNSE_INVOKE "CLN_Skinedit/Invoke"
#define MS_CLNSE_FILLBYCURRENTSEL "CLN_Skinedit/FillByCurrentSel"

/*
 * data structs
 */

struct TWindowData;
class CImageItem;

struct ButtonItem {
    TCHAR   szName[40];
    HWND    hWnd;
    LONG    xOff, yOff;
    LONG    width, height;
    CImageItem *imgNormal, *imgPressed, *imgHover;
    LONG_PTR normalGlyphMetrics[4];
    LONG_PTR hoverGlyphMetrics[4];
    LONG_PTR pressedGlyphMetrics[4];
    DWORD   dwFlags, dwStockFlags;
    DWORD   uId;
    TCHAR   szTip[256];
    char    szService[256];
    char    szModule[256], szSetting[256];
    BYTE    bValuePush[256], bValueRelease[256];
    DWORD   type;
    void    (*pfnAction)(ButtonItem *item, HWND hwndDlg, TWindowData *dat, HWND hwndItem);
    void    (*pfnCallback)(ButtonItem *item, HWND hwndDlg, TWindowData *dat, HWND hwndItem);
    TCHAR   tszLabel[40];
    ButtonItem* nextItem;
	HANDLE  hContact;
	TWindowData *dat;
};

typedef struct _tagButtonSet {
    ButtonItem  *items;
    LONG        left, top, right, bottom;               // client area offsets, calculated from button layout
} ButtonSet;

struct CSkinItem {
    TCHAR szName[40];
    char szDBname[40];
    int statusID;

    BYTE GRADIENT;
    BYTE CORNER;

    DWORD COLOR;
    DWORD COLOR2;

    BYTE COLOR2_TRANSPARENT;

    DWORD TEXTCOLOR;

    int ALPHA;

    int MARGIN_LEFT;
    int MARGIN_TOP;
    int MARGIN_RIGHT;
    int MARGIN_BOTTOM;
    BYTE IGNORED;
    DWORD BORDERSTYLE;
    CImageItem *imageItem;
};

typedef struct _tagSkinDescription {
    DWORD           cbSize;
    CSkinItem   *StatusItems;
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
#define CORNER_ALL (CORNER_TL | CORNER_TR | CORNER_BR | CORNER_BL | CORNER_ACTIVE)

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

#define BUTTON_ISINTERNAL 1
#define BUTTON_ISTOGGLE 2
#define BUTTON_ISSERVICE 4
#define BUTTON_ISPROTOSERVICE 8
#define BUTTON_PASSHCONTACTW 16
#define BUTTON_PASSHCONTACTL 32
#define BUTTON_ISDBACTION    64
#define BUTTON_ISCONTACTDBACTION 128
#define BUTTON_DBACTIONONCONTACT 256
#define BUTTON_ISSIDEBAR 512
#define BUTTON_NORMALGLYPHISICON 1024
#define BUTTON_PRESSEDGLYPHISICON 2048
#define BUTTON_HOVERGLYPHISICON 4096
#define BUTTON_HASLABEL 8192

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
