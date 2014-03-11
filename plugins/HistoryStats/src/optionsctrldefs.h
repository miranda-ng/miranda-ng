#if !defined(HISTORYSTATS_GUARD_OPTIONSCTRLDEFS_H)
#define HISTORYSTATS_GUARD_OPTIONSCTRLDEFS_H

#include "_globals.h"

/*
 * OptionsCtrlDefs
 */

class OptionsCtrlDefs
{
public:
	enum Message {
		OCM_INSERTGROUP      = WM_USER +  0, // (HANDLE hParent, OCGROUP* pGroup)       -> HANDLE hGroup           [Group]
		OCM_INSERTCHECK      = WM_USER +  1, // (HANDLE hParent, OCCHECK* pCheck)       -> HANDLE hCheck           [Check]
		OCM_INSERTRADIO      = WM_USER +  2, // (HANDLE hParent, OCRADIO* pRadio)       -> HANDLE hRadio           [Radio]
		OCM_INSERTEDIT       = WM_USER +  3, // (HANDLE hParent, OCEDIT* pEdit)         -> HANDLE hEdit            [Edit]
		OCM_INSERTCOMBO      = WM_USER +  4, // (HANDLE hParent, OCCOMBO* pCombo)       -> HANDLE hCombo           [Combo]
		OCM_INSERTBUTTON     = WM_USER +  5, // (HANDLE hParent, OCBUTTON* pButton)     -> HANDLE hButton          [Button]
		OCM_GETITEMLABEL     = WM_USER +  6, // (HANDLE hItem, #)                       -> const TCHAR* szLabel
		OCM_SETITEMLABEL     = WM_USER +  7, // (HANDLE hItem, const TCHAR* szLabel)  -> #
		OCM_ISITEMENABLED    = WM_USER +  8, // (HANDLE hItem, #)                       -> BOOL bEnabled
		OCM_ENABLEITEM       = WM_USER +  9, // (HANDLE hItem, BOOL bEnable)            -> #
		OCM_GETITEMDATA      = WM_USER + 10, // (HANDLE hItem, #)                       -> INT_PTR dwData
		OCM_SETITEMDATA      = WM_USER + 11, // (HANDLE hItem, INT_PTR dwData)            -> #
		OCM_ISITEMCHECKED    = WM_USER + 12, // (HANDLE hItem, #)                       -> BOOL bChecked           [Check/Radio]
		OCM_CHECKITEM        = WM_USER + 13, // (HANDLE hItem, BOOL bCheck)             -> #                       [Check/Radio (ignores bCheck)]
		OCM_GETRADIOCHECKED  = WM_USER + 14, // (HANDLE hRadio, #)                      -> int nChecked            [Radio]
		OCM_SETRADIOCHECKED  = WM_USER + 15, // (HANDLE hRadio, int nCheck)             -> #                       [Radio]
		OCM_GETEDITNUMBER    = WM_USER + 16, // (HANDLE hEdit, #)                       -> int nNumber             [Edit]
		OCM_SETEDITNUMBER    = WM_USER + 17, // (HANDLE hEdit, int nNumber)             -> #                       [Edit]
		OCM_GETEDITSTRING    = WM_USER + 18, // (HANDLE hEdit, #)                       -> const TCHAR* szString [Edit]
		OCM_SETEDITSTRING    = WM_USER + 19, // (HANDLE hEdit, const TCHAR* szString) -> #                       [Edit]
		OCM_ADDCOMBOITEM     = WM_USER + 20, // (HANDLE hCombo, const TCHAR* szItem)  -> #                       [Combo]
		OCM_GETCOMBOSELECTED = WM_USER + 21, // (HANDLE hCombo, #)                      -> int nSelected           [Combo]
		OCM_SETCOMBOSELECTED = WM_USER + 22, // (HANDLE hCombo, int nSelect)            -> #                       [Combo]
		OCM_ENSUREVISIBLE    = WM_USER + 23, // (HANDLE hItem, #)                       -> #
		OCM_DELETEALLITEMS   = WM_USER + 24, // (#, #)                                  -> #
		OCM_GETSELECTION     = WM_USER + 25, // (#, #)                                  -> HANDLE hItem
		OCM_SELECTITEM       = WM_USER + 26, // (HANDLE hItem, #)                       -> #
		OCM_GETITEM          = WM_USER + 27, // (HANDLE hItem, DWORD dwFlag = OCGI_*)   -> HANDLE hItem
		OCM_DELETEITEM       = WM_USER + 28, // (HANDLE hItem, #)                       -> #
		OCM_MOVEITEM         = WM_USER + 29, // (HANDLE* phItem, HANDLE hInsertAfter)   -> #
		OCM_GETSCROLLPOS     = WM_USER + 30, // (int nBar, #)                           -> int nPos
		OCM_SETSCROLLPOS     = WM_USER + 31, // (int nBar, int nPos)                    -> #
		OCM_INSERTDATETIME   = WM_USER + 32, // (HANDLE hParent, OCDATETIME* pDateTime) -> HANDLE hDateTime        [DateTime]
		OCM_ISDATETIMENONE   = WM_USER + 33, // (HANDLE hDateTime, #)                   -> BOOL bNone              [DateTime]
		OCM_SETDATETIMENONE  = WM_USER + 34, // (HANDLE hDateTime, #)                   -> #                       [DateTime]
		OCM_GETDATETIME      = WM_USER + 35, // (HANDLE hDateTime, BOOL* pbNone)        -> DWORD dwDateTime        [DateTime]
		OCM_SETDATETIME      = WM_USER + 36, // (HANDLE hDateTime, DWORD dwDateTime)    -> #                       [DateTime]
		OCM_INSERTCOLOR      = WM_USER + 37, // (HANDLE hParent, OCCOLOR* pColor)       -> HANDLE hColor           [Color]
		OCM_GETITEMCOLOR     = WM_USER + 38, // (HANDLE hColor, #)                      -> COLORREF crColor        [Color]
		OCM_SETITEMCOLOR     = WM_USER + 39, // (HANDLE hColor, COLORREF crColor)       -> #                       [Color]
	};

	enum Notification {
		OCN_MODIFIED    = NM_LAST - 1, // -> NMOPTIONSCTRL     (hItem/dwData -> item that caused the modification     ) [Check/Radio/Edit/Combo/DateTime/Color]
		OCN_CLICKED     = NM_LAST - 2, // -> NMOPTIONSCTRL     (hItem/dwData -> item that was clicked                 ) [Button]
		OCN_SELCHANGING = NM_LAST - 3, // -> NMOPTIONSCTRL     (hItem/dwData -> item that gets unselected, may be NULL)
		OCN_SELCHANGED  = NM_LAST - 4, // -> NMOPTIONSCTRL     (hItem/dwData -> item that got selected, may be NULL   )
		OCN_ITEMDROPPED = NM_LAST - 5, // -> NMOPTIONSCTRLDROP (hItem/dwData/hDropTarget/dwDropTargetData/bAbove      )
	};

	enum StyleFlags {
		OCS_ALLOWDRAGDROP = 0x0001,
	};

	enum ItemFlags {
		OCF_BOLD                   = 0x01,
		OCF_DISABLED               = 0x02,
		OCF_NODISABLECHILDS        = 0x04,
		OCF_CHECKED                = 0x08, // [Check/Radio]
		OCF_NONE                   = 0x08, // [DateTime]
		OCF_DISABLECHILDSONUNCHECK = 0x10, // [Check/Radio]
		OCF_DISABLECHILDSONINDEX0  = 0x10, // [Combo]
		OCF_DRAWLINE               = 0x10, // [Group]
		OCF_NUMBER                 = 0x10, // [Edit]
		OCF_DISABLECHILDSONNONE    = 0x10, // [DateTime]
		OCF_ALLOWNONE              = 0x20, // [DateTime]
		// combined flags
		OCF_ROOTGROUP              = OCF_BOLD | OCF_DRAWLINE, // [Group]
	};

	enum GetItemFlag {
		OCGI_FIRST = 0, // hItem is ignored
		OCGI_NEXT  = 1, // next sibling relative to hItem
		OCGI_PREV  = 2, // previous sibling relative to hItem
	};

	struct OCGROUP {
		DWORD dwFlags;
		TCHAR* szLabel;
		INT_PTR dwData;
	};

	struct OCCHECK {
		DWORD dwFlags;
		TCHAR* szLabel;
		INT_PTR dwData;
	};

	struct OCRADIO {
		DWORD dwFlags;
		TCHAR* szLabel;
		INT_PTR dwData;
		HANDLE hSibling;
	};

	struct OCEDIT {
		DWORD dwFlags;
		TCHAR* szLabel;
		INT_PTR dwData;
		TCHAR* szEdit;
	};

	struct OCCOMBO {
		DWORD dwFlags;
		TCHAR* szLabel;
		INT_PTR dwData;
	};

	struct OCBUTTON {
		DWORD dwFlags;
		TCHAR* szLabel;
		INT_PTR dwData;
		TCHAR* szButton;
	};

	struct OCDATETIME {
		DWORD dwFlags;
		TCHAR* szLabel;
		INT_PTR dwData;
		TCHAR* szFormat;
		DWORD dwDateTime;
	};

	struct OCCOLOR {
		DWORD dwFlags;
		TCHAR* szLabel;
		INT_PTR dwData;
		COLORREF crColor;
	};

	struct NMOPTIONSCTRL {
		NMHDR hdr;
		HANDLE hItem;
		INT_PTR dwData;
	};

	struct NMOPTIONSCTRLDROP {
		NMHDR hdr;
		HANDLE hItem;
		INT_PTR dwData;
		HANDLE hDropTarget;
		DWORD dwDropTargetData;
		BOOL bAbove;
	};
};

#endif // HISTORYSTATS_GUARD_OPTIONSCTRLDEFS_H