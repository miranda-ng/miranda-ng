/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/**
 * System Includes:
 **/
#include "commonheaders.h"
#include "svc_gender.h"
#include "svc_reminder.h"
#include "dlg_anniversarylist.h"

#include "m_message.h"

#define IsLeap(wYear)	(!(((wYear) % 4 != 0) || (((wYear) % 100 == 0) && ((wYear) % 400 != 0))))

class CAnnivList;

static CAnnivList *gpDlg = NULL;

/***********************************************************************************************************
 * class CAnnivList
 ***********************************************************************************************************/

class CAnnivList
{
	HWND	_hDlg;
	HWND	_hList;
	SIZE	_sizeMin;
	RECT	_rcWin;
	SHORT	_sortOrder;
	int		_sortHeader;
	int		_curSel;
	int		_numRows;
	BYTE	_bRemindEnable;
	HANDLE	_mHookExit;
	bool	_wmINIT;

	typedef int (CALLBACK* CMPPROC)(LPARAM, LPARAM LPARAM);

	enum EColumn
	{
		COLUMN_ETA = 0,
		COLUMN_CONTACT,
		COLUMN_PROTO,
		COLUMN_AGE,
		COLUMN_DESC,
		COLUMN_DATE,
	};

	enum EFilter
	{
		FILTER_ALL = 0,
		FILTER_BIRTHDAY,
		FILTER_ANNIV,
		FILTER_DISABLED_REMINDER
	};

	struct CFilter
	{
		WORD	wDaysBefore;
		BYTE	bFilterIndex;
		LPSTR	pszProto;
		LPTSTR	pszAnniv;

		CFilter()
		{
			wDaysBefore		= (WORD)-1;
			bFilterIndex	= 0;
			pszProto		= NULL;
			pszAnniv		= NULL;
		}
	} _filter;

	struct CItemData
	{
		MCONTACT	_hContact;
		MAnnivDate	*_pDate;
		WORD		_wDaysBefore;
		BYTE		_wReminderState;

		CItemData(MCONTACT hContact, MAnnivDate &date)
		{
			_hContact = hContact;
			_wReminderState = date.RemindOption();
			_wDaysBefore = date.RemindOffset();
			_pDate = new MAnnivDate(date);
		}

		~CItemData()
		{
			if (_pDate) {
				// save changes
				if (_wReminderState != _pDate->RemindOption() || _wDaysBefore != _pDate->RemindOffset()) {
					_pDate->RemindOffset(_wDaysBefore);
					_pDate->RemindOption(_wReminderState);
					_pDate->DBWriteReminderOpts(_hContact);
				}
				delete _pDate;
				_pDate = NULL;
			}
		}
	};

	// This class handles the movement of child controls on size change.
	class CAnchor
	{
	public:
		enum EAnchor
		{
			ANCHOR_LEFT		= 1,
			ANCHOR_RIGHT	= 2,
			ANCHOR_TOP		= 4,
			ANCHOR_BOTTOM	= 8,
			ANCHOR_ALL		= ANCHOR_LEFT | ANCHOR_RIGHT | ANCHOR_TOP | ANCHOR_BOTTOM
		};

	private:
		WINDOWPOS	*_wndPos;
		HDWP		_hdWnds;
		RECT		_rcParent;

		void _ScreenToClient(HWND hWnd, LPRECT rc)
		{
			POINT pt = { rc->left, rc->top };
			
			ScreenToClient(hWnd, &pt);
			rc->right	+= pt.x - rc->left;
			rc->bottom	+= pt.y - rc->top;
			rc->left	= pt.x;
			rc->top		= pt.y;
		}

		void _MoveWindow(HWND hWnd, int anchors)
		{
			if (!(_wndPos->flags & SWP_NOSIZE)) {
				RECT rcc = _CalcPos(hWnd, anchors);
				MoveWindow(hWnd, rcc.left, rcc.top, rcc.right - rcc.left, rcc.bottom - rcc.top, FALSE);
			}
		}

		RECT _CalcPos(HWND hWnd, int anchors)
		{
			RECT rcc;

			GetWindowRect(hWnd, &rcc);
			_ScreenToClient(_wndPos->hwnd, &rcc);
			if (!(_wndPos->flags & SWP_NOSIZE)) {
				// calculate difference between new and old size
				const int cx = _wndPos->cx - _rcParent.right + _rcParent.left;
				const int cy = _wndPos->cy - _rcParent.bottom + _rcParent.top;

				if (cx != 0 || cy != 0) {
					// move client rect points to the desired new position
					if (!(anchors & ANCHOR_LEFT) || (anchors & ANCHOR_RIGHT))
						rcc.right += cx;
					if (!(anchors & ANCHOR_TOP) || (anchors & ANCHOR_BOTTOM))
						rcc.bottom += cy;
					if ((anchors & ANCHOR_RIGHT) && (!(anchors & ANCHOR_LEFT)))
						rcc.left += cx;
					if ((anchors & ANCHOR_BOTTOM) && (!(anchors & ANCHOR_TOP)))
						rcc.top += cy;
				}
			}
			return rcc;
		}

	public:
		CAnchor(WINDOWPOS *wndPos, SIZE minSize)
		{
			GetWindowRect(wndPos->hwnd, &_rcParent);
			if (wndPos->cx < minSize.cx)
				wndPos->cx = minSize.cx;
			if (wndPos->cy < minSize.cy)
				wndPos->cy = minSize.cy;
			_wndPos = wndPos;
			_hdWnds = BeginDeferWindowPos(2);
		}

		~CAnchor()
		{
			EndDeferWindowPos(_hdWnds);
		}

		void MoveCtrl(WORD idCtrl, int anchors)
		{
			if (!(_wndPos->flags & SWP_NOSIZE)) {
				HWND hCtrl = GetDlgItem(_wndPos->hwnd, idCtrl);
				RECT rcc = _CalcPos(hCtrl, anchors);
				_hdWnds = DeferWindowPos(
						_hdWnds,					//HDWP hWinPosInfo
						hCtrl,						//HWND hWnd
						HWND_NOTOPMOST,				//hWndInsertAfter
						rcc.left,					//int x
						rcc.top,					//int y
						rcc.right - rcc.left,
						rcc.bottom - rcc.top,
						SWP_NOZORDER				//UINT uFlags
						);
			}
		}
	};

	/**
	 * This compare function is used by ListView_SortItemsEx to sort the listview.
	 *
	 * @param		iItem1	- index of the first item
	 * @param		iItem2	- index of the second item
	 * @param		pDlg	- pointer to the class' object
	 *
	 * @return	This function returns a number indicating comparison result.
	 **/
	static int CALLBACK cmpProc(int iItem1, int iItem2, CAnnivList *pDlg)
	{
		int result;

		if (pDlg) {
			TCHAR szText1[MAX_PATH];
			TCHAR szText2[MAX_PATH];

			szText1[0] = szText2[0] = 0;
			switch (pDlg->_sortHeader) {
			case COLUMN_CONTACT:
			case COLUMN_PROTO:
			case COLUMN_DESC:
				ListView_GetItemText(pDlg->_hList, iItem1, pDlg->_sortHeader, szText1, MAX_PATH);
				ListView_GetItemText(pDlg->_hList, iItem2, pDlg->_sortHeader, szText2, MAX_PATH);
				result = pDlg->_sortOrder * mir_tstrcmp(szText1, szText2);
				break;

			case COLUMN_AGE:
			case COLUMN_ETA:
				ListView_GetItemText(pDlg->_hList, iItem1, pDlg->_sortHeader, szText1, MAX_PATH);
				ListView_GetItemText(pDlg->_hList, iItem2, pDlg->_sortHeader, szText2, MAX_PATH);
				result = pDlg->_sortOrder * (_ttoi(szText1) - _ttoi(szText2));
				break;

			case COLUMN_DATE: 
				{
					CItemData *id1 = pDlg->ItemData(iItem1), *id2 = pDlg->ItemData(iItem2);

					if (PtrIsValid(id1) && PtrIsValid(id2)) {
						result = pDlg->_sortOrder * id1->_pDate->Compare(*id2->_pDate);
						break;
					}
				}
			default:
				result = 0;
			}
		}
		else
			result = 0;
		return result;
	}

	/**
	 * This static method is the window procedure for the dialog.
	 *
	 * @param		hDlg	- handle of the dialog window
	 * @param		uMsg	- message to handle
	 * @param		wParam	- message dependend parameter
	 * @param		lParam	- message dependend parameter
	 *
	 * @return	depends on message
	 **/
	static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		CAnnivList *pDlg = (CAnnivList *)GetUserData(hDlg);
		
		switch (uMsg) {
		case WM_INITDIALOG:
			{
				int i = 0;
				HWND hCtrl;
				HICON hIcon;
				RECT rc;

				// link the class to the window handle
				pDlg = (CAnnivList *)lParam;
				if (!pDlg)
					break;
				SetUserData(hDlg, lParam);
				pDlg->_hDlg = hDlg;

				// init pointer listview control
				pDlg->_hList = GetDlgItem(hDlg, EDIT_ANNIVERSARY_DATE);
				if (!pDlg->_hList)
					break;

				// set icons
				hIcon = Skin_GetIcon(ICO_DLG_ANNIVERSARY);
				SendDlgItemMessage(hDlg, IDC_HEADERBAR, WM_SETICON, 0, (LPARAM)hIcon);
				SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

				// insert columns into the listboxes
				ListView_SetExtendedListViewStyle(pDlg->_hList, LVS_EX_FULLROWSELECT);

				// add columns
				if (pDlg->AddColumn(CAnnivList::COLUMN_ETA, LPGENT("ETA"), 40)
					|| pDlg->AddColumn(CAnnivList::COLUMN_CONTACT, LPGENT("Contact"), 160)
					|| pDlg->AddColumn(CAnnivList::COLUMN_PROTO, LPGENT("Proto"), 50)
					|| pDlg->AddColumn(CAnnivList::COLUMN_AGE, LPGENT("Age/Nr."), 40)
					|| pDlg->AddColumn(CAnnivList::COLUMN_DESC, LPGENT("Anniversary"), 100)
					|| pDlg->AddColumn(CAnnivList::COLUMN_DATE, LPGENT("Date"), 80))
					break;

				TranslateDialogDefault(hDlg);
				
				// save minimal size
				GetWindowRect(hDlg, &rc);
				pDlg->_sizeMin.cx = rc.right - rc.left;
				pDlg->_sizeMin.cy = rc.bottom - rc.top;
				
				// restore position and size
				Utils_RestoreWindowPosition(hDlg, NULL, MODNAME, "AnnivDlg_");

				//save win pos
				GetWindowRect(hDlg, &pDlg->_rcWin);

				// add filter strings
				if (hCtrl = GetDlgItem(hDlg, COMBO_VIEW)) {
					ComboBox_AddString(hCtrl, TranslateT("All contacts"));
					ComboBox_AddString(hCtrl, TranslateT("Birthdays only"));
					ComboBox_AddString(hCtrl, TranslateT("Anniversaries only"));
					ComboBox_AddString(hCtrl, TranslateT("Disabled reminder"));
					ComboBox_SetCurSel(hCtrl, pDlg->_filter.bFilterIndex);
				}

				// init reminder groups
				pDlg->_bRemindEnable = db_get_b(NULL, MODNAME, SET_REMIND_ENABLED, DEFVAL_REMIND_ENABLED) != REMIND_OFF;
				if (hCtrl = GetDlgItem(hDlg, CHECK_REMIND)) {
					Button_SetCheck(hCtrl, pDlg->_bRemindEnable ? BST_INDETERMINATE : BST_UNCHECKED);
					EnableWindow(hCtrl, pDlg->_bRemindEnable);
				}

				CheckDlgButton(hDlg, CHECK_POPUP, db_get_b(NULL, MODNAME, SET_ANNIVLIST_POPUP, FALSE));
				// set number of days to show contact in advance
				SetDlgItemInt(hDlg, EDIT_DAYS, pDlg->_filter.wDaysBefore, FALSE);
				if (hCtrl = GetDlgItem(hDlg, CHECK_DAYS)) {
					Button_SetCheck(hCtrl, db_get_b(NULL, MODNAME, SET_ANNIVLIST_FILTER_DAYSENABLED, FALSE));
					DlgProc(hDlg, WM_COMMAND, MAKEWPARAM(CHECK_DAYS, BN_CLICKED), (LPARAM)hCtrl);
				}

				pDlg->_wmINIT = false;
			}
			return TRUE;

		// set propertysheet page's background white in aero mode
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORDLG:
			if (IsAeroMode())
				return (INT_PTR)GetStockBrush(WHITE_BRUSH);
			break;

		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->idFrom) {
			case EDIT_ANNIVERSARY_DATE:
				switch (((LPNMHDR)lParam)->code) {
				// handle changed selection
				case LVN_ITEMCHANGED:
					{
						CItemData *pid;
						HWND hCheck;

						pDlg->_curSel = ((LPNMLISTVIEW)lParam)->iItem;
						pid = pDlg->ItemData(pDlg->_curSel);
						if (pid && pDlg->_bRemindEnable && (hCheck = GetDlgItem(hDlg, CHECK_REMIND))) {
							SetDlgItemInt(hDlg, EDIT_REMIND, pid->_wDaysBefore, FALSE);
							Button_SetCheck(hCheck, pid->_wReminderState);
							DlgProc(hDlg, WM_COMMAND, MAKEWPARAM(CHECK_REMIND, BN_CLICKED), (LPARAM)hCheck);
						}
					}
					break;

				// resort the list
				case LVN_COLUMNCLICK:
					{
						LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;

						if (pDlg->_sortHeader == pnmv->iSubItem)
							pDlg->_sortOrder *= -1;
						else {
							pDlg->_sortOrder = 1;
							pDlg->_sortHeader = pnmv->iSubItem;
						}
						ListView_SortItemsEx(pDlg->_hList, (CMPPROC)cmpProc, pDlg);
					}
					break;

				// show contact menu
				case NM_RCLICK:
					{
						CItemData *pid = pDlg->ItemData(pDlg->_curSel);
						if (pid) {
							HMENU hPopup = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)pid->_hContact, 0);
							if (hPopup) {
								POINT pt;
								GetCursorPos(&pt);
								TrackPopupMenu(hPopup, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hDlg, NULL);
								DestroyMenu(hPopup);
							}
						}
					}
					break;

				// handle double click on contact: show message dialog
				case NM_DBLCLK:
					{
						CItemData *pid = pDlg->ItemData(((LPNMITEMACTIVATE)lParam)->iItem);
						if (pid)
							CallService(MS_MSG_SENDMESSAGE,(WPARAM)pid->_hContact, NULL);
					}
				}
			}
			break;

		case WM_COMMAND:
			if (PtrIsValid(pDlg)) {
				CItemData* pid = pDlg->ItemData(pDlg->_curSel);

				// process contact menu command
				if (pid && CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)pid->_hContact))
					break;

				switch (LOWORD(wParam)) {
				// enable/disable reminder checkbox is clicked
				case CHECK_REMIND:
					if (pDlg->_bRemindEnable && HIWORD(wParam) == BN_CLICKED) {
						BYTE checkState = Button_GetCheck((HWND)lParam);

						EnableWindow(GetDlgItem(hDlg, EDIT_REMIND), checkState == BST_CHECKED);
						EnableWindow(GetDlgItem(hDlg, SPIN_REMIND), checkState == BST_CHECKED);
						if (pid && pid->_wReminderState != checkState)
							pid->_wReminderState = checkState;
					}
					break;

				// number of days to remind in advance is edited
				case EDIT_REMIND:
					if (pid && pDlg->_bRemindEnable && HIWORD(wParam) == EN_CHANGE) {
						WORD wDaysBefore = GetDlgItemInt(hDlg, LOWORD(wParam), NULL, FALSE);
						if (pid->_wReminderState == BST_CHECKED && pid->_wDaysBefore != wDaysBefore) {
							pid->_wDaysBefore = wDaysBefore;
						}
					}
					break;

				// the filter to display only contacts which have an anniversary in a certain 
				// period of time is enabled/disabled
				case CHECK_DAYS:
					if (HIWORD(wParam) == BN_CLICKED) {
						BYTE isChecked = Button_GetCheck((HWND)lParam);
						EnableWindow(GetDlgItem(hDlg, EDIT_DAYS), isChecked);
						EnableWindow(GetDlgItem(hDlg, TXT_DAYS), isChecked);
						pDlg->_filter.wDaysBefore = isChecked ? GetDlgItemInt(hDlg, EDIT_DAYS, NULL, FALSE) : (WORD)-1;
						pDlg->RebuildList();
					}
					break;

				// the number of days a contact must have an anniversary in advance to be displayed is edited
				case EDIT_DAYS:
					if (HIWORD(wParam) == EN_CHANGE) {
						WORD wNewDays = GetDlgItemInt(hDlg, LOWORD(wParam), NULL, FALSE);
						if (wNewDays != pDlg->_filter.wDaysBefore) {
							pDlg->_filter.wDaysBefore = wNewDays;
							pDlg->RebuildList();
						}
					}
					break;

				// the filter selection of the filter combobox has changed
				case COMBO_VIEW:
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						pDlg->_filter.bFilterIndex = ComboBox_GetCurSel((HWND)lParam);
						pDlg->RebuildList();
					}
				}
			}
			break;

		case WM_DRAWITEM:
			return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

		case WM_MEASUREITEM:
			return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

		case WM_WINDOWPOSCHANGING:
			if (PtrIsValid(pDlg)) {
				WINDOWPOS *wndPos = (WINDOWPOS *)lParam;
				if (!pDlg->_wmINIT && (wndPos->cx != 0 || wndPos->cy != 0)) {
					//win pos change
					if ((wndPos->cx == pDlg->_rcWin.right - pDlg->_rcWin.left) && (wndPos->cy == pDlg->_rcWin.bottom - pDlg->_rcWin.top))
						//win pos change (store new pos)
						GetWindowRect(hDlg, &pDlg->_rcWin);
					//win size change
					else {
						// l change
						if ((wndPos->cx < pDlg->_sizeMin.cx) && (wndPos->x > pDlg->_rcWin.left)) {
							wndPos->x  = wndPos->x + wndPos->cx - pDlg->_sizeMin.cx;
							wndPos->cx = pDlg->_sizeMin.cx;
						}
						// r change
						else if (wndPos->cx < pDlg->_sizeMin.cx)
							wndPos->cx = pDlg->_sizeMin.cx;

						// t change
						if ((wndPos->cy < pDlg->_sizeMin.cy) && (wndPos->y > pDlg->_rcWin.top)) {
							wndPos->y  = wndPos->y + wndPos->cy - pDlg->_sizeMin.cy;
							wndPos->cy = pDlg->_sizeMin.cy;
						}
						// b change
						else if (wndPos->cy < pDlg->_sizeMin.cy)
							wndPos->cy = pDlg->_sizeMin.cy;

						pDlg->_rcWin.left = wndPos->x;
						pDlg->_rcWin.right = wndPos->x + wndPos->cx;
						pDlg->_rcWin.top = wndPos->y;
						pDlg->_rcWin.bottom = wndPos->y + wndPos->cy;
					}
				}

				CAnchor anchor(wndPos, pDlg->_sizeMin);
				int anchorPos = CAnchor::ANCHOR_LEFT | CAnchor::ANCHOR_RIGHT | CAnchor::ANCHOR_TOP;

				anchor.MoveCtrl(IDC_HEADERBAR, anchorPos);
				anchor.MoveCtrl(GROUP_STATS, anchorPos);

				// birthday list
				anchor.MoveCtrl(EDIT_ANNIVERSARY_DATE, CAnchor::ANCHOR_ALL);

				anchorPos = CAnchor::ANCHOR_RIGHT | CAnchor::ANCHOR_BOTTOM;

				// filter group
				anchor.MoveCtrl(GROUP_FILTER, anchorPos);
				anchor.MoveCtrl(COMBO_VIEW, anchorPos);
				anchor.MoveCtrl(CHECK_DAYS, anchorPos);
				anchor.MoveCtrl(EDIT_DAYS, anchorPos);
				anchor.MoveCtrl(TXT_DAYS, anchorPos);

				// filter group
				anchor.MoveCtrl(GROUP_REMINDER, anchorPos);
				anchor.MoveCtrl(CHECK_REMIND, anchorPos);
				anchor.MoveCtrl(EDIT_REMIND, anchorPos);
				anchor.MoveCtrl(SPIN_REMIND, anchorPos);
				anchor.MoveCtrl(TXT_REMIND6, anchorPos);
				anchor.MoveCtrl(CHECK_POPUP, anchorPos);
			}
			break;

		// This message is sent if eighter the user clicked on the close button or
		// Miranda fires the ME_SYSTEM_SHUTDOWN event.
		case WM_CLOSE:
			DestroyWindow(hDlg);
			break;

		// If the anniversary list is destroyed somehow, the data class must be deleted, too.
		case WM_DESTROY:
			if (PtrIsValid(pDlg)) {
				SetUserData(hDlg, NULL);
				delete pDlg;
			}
			break;
		}
		return FALSE;
	}

	/**
	 * This method adds a column to the listview.
	 *
	 * @param	iSubItem		- desired column index
	 * @param	pszText			- the header text
	 * @param	defaultWidth	- the default witdth
	 *
	 * @retval	0 if successful
	 * @retval	1 if failed
	 **/
	BYTE AddColumn(int iSubItem, LPCTSTR pszText, int defaultWidth)
	{
		LVCOLUMN lvc;
		CHAR pszSetting[MAXSETTING];

		mir_snprintf(pszSetting, SIZEOF(pszSetting), "AnnivDlg_Col%d", iSubItem);
		lvc.cx = db_get_w(NULL, MODNAME, pszSetting, defaultWidth);
		lvc.mask = LVCF_WIDTH | LVCF_TEXT;
		lvc.iSubItem = iSubItem;
		lvc.pszText = TranslateTS(pszText);
		return ListView_InsertColumn(_hList, lvc.iSubItem++, &lvc) == -1;
	}

	/**
	 * This method sets the subitem text for the current contact
	 *
	 * @param	iItem			- index of the current row
	 * @param	iSubItem		- column to set text for
	 * @param	pszText			- text to insert
	 *
	 * @retval	TRUE if successful
	 * @retval	FALSE if failed
	 **/
	BYTE AddSubItem(int iItem, int iSubItem, LPTSTR pszText)
	{
		LVITEM lvi;
		if (iSubItem > 0) {
			lvi.iItem = iItem;
			lvi.iSubItem = iSubItem;
			lvi.pszText = pszText;
			lvi.mask = LVIF_TEXT;
			return ListView_SetItem(_hList, &lvi);
		}
		return FALSE;
	}

	/**
	 * This method adds a row to the listview.
	 *
	 * @param	pszText			- text to insert
	 * @param	lParam			- pointer to the rows data
	 *
	 * @retval	TRUE if successful
	 * @retval	FALSE if failed
	 **/	
	BYTE AddItem(LPTSTR pszText, LPARAM lParam)
	{
		LVITEM lvi;

		if (!pszText) {
			return FALSE;
		}
		lvi.iItem = 0;
		lvi.iSubItem = 0;
		lvi.pszText = pszText;
		lvi.mask = LVIF_TEXT | TVIF_PARAM;
		lvi.lParam = lParam;
		return ListView_InsertItem(_hList, &lvi);
	}

	/**
	 * This method adds a row to the listview.
	 *
	 * @param	hContact		- contact to add the line for
	 * @param	pszProto		- contact's protocol
	 * @param	ad				- anniversary to add
	 * @param	mtNow			- current time
	 * @param	wDaysBefore		- number of days in advance to remind the user of the anniversary
	 *
	 * @retval	TRUE if successful
	 * @retval	FALSE if failed
	 **/
	BYTE AddRow(MCONTACT hContact, LPCSTR pszProto, MAnnivDate &ad, MTime &mtNow, WORD wDaysBefore) 
	{
		TCHAR szText[MAX_PATH];
		int diff, iItem = -1;
		CItemData *pdata;
	
		// first column: ETA
		diff = ad.CompareDays(mtNow);
		if (diff < 0)
			diff += IsLeap(mtNow.Year() + 1) ? 366 : 365;
		// is filtered
		if (diff <= _filter.wDaysBefore) {
			// read reminder options for the contact
			ad.DBGetReminderOpts(hContact);
			if ((_filter.bFilterIndex != FILTER_DISABLED_REMINDER) || (ad.RemindOption() == BST_UNCHECKED)) {
				// set default offset if required
				if (ad.RemindOffset() == (WORD)-1) {
					ad.RemindOffset(wDaysBefore);
					
					// create data object
					pdata = new CItemData(hContact, ad);
					if (!pdata)
						return FALSE;
					// add item
					iItem = AddItem(_itot(diff, szText, 10), (LPARAM)pdata);
					if (iItem == -1) {
						delete pdata;
						return FALSE;
					}

					// second column: contact name
					AddSubItem(iItem, COLUMN_CONTACT, DB::Contact::DisplayName(hContact));

					// third column: protocol
					TCHAR *ptszProto = mir_a2t(pszProto);
					AddSubItem(iItem, COLUMN_PROTO, ptszProto);
					mir_free(ptszProto);

					// forth line: age
					AddSubItem(iItem, COLUMN_AGE, _itot(ad.Age(&mtNow), szText, 10));

					// fifth line: anniversary
					AddSubItem(iItem, COLUMN_DESC, (LPTSTR)ad.Description());

					// sixth line: date
					ad.DateFormat(szText, SIZEOF(szText));
					AddSubItem(iItem, COLUMN_DATE, szText);
					
					_numRows++;
				}
			}
		}
		return TRUE;
	}

	// This method clears the list and adds contacts again, according to the current filter settings.
	void RebuildList()
	{
		LPSTR pszProto;
		MTime mtNow;
		MAnnivDate ad;
		int i = 0;
		DWORD age = 0;
		WORD wDaysBefore = db_get_w(NULL, MODNAME, SET_REMIND_OFFSET, DEFVAL_REMIND_OFFSET);
		WORD numMale = 0;
		WORD numFemale = 0;
		WORD numContacts = 0;
		WORD numBirthContacts = 0;

		ShowWindow(_hList, SW_HIDE);
		DeleteAllItems();
		mtNow.GetLocalTime();

		// insert the items into the list
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			// ignore meta subcontacts here, as they are not interesting.
			if (!db_mc_isSub(hContact)) {
				// filter protocol
				pszProto = DB::Contact::Proto(hContact);
				if (pszProto) {
					numContacts++;
					switch (GenderOf(hContact, pszProto)) {
					case 'M':
						numMale++;
						break;
					case 'F':
						numFemale++;
					}

					if (!ad.DBGetBirthDate(hContact, pszProto)) {
						age += ad.Age(&mtNow);
						numBirthContacts++;

						// add birthday
						if ((_filter.bFilterIndex != FILTER_ANNIV) && (!_filter.pszProto || !_strcmpi(pszProto, _filter.pszProto)))
							AddRow(hContact, pszProto, ad, mtNow, wDaysBefore);
					}

					// add anniversaries
					if (_filter.bFilterIndex != FILTER_BIRTHDAY && (!_filter.pszProto || !_strcmpi(pszProto, _filter.pszProto))) 
						for (i = 0; !ad.DBGetAnniversaryDate(hContact, i); i++)
							if (!_filter.pszAnniv || !_tcsicmp(_filter.pszAnniv, ad.Description()))
								AddRow(hContact, pszProto, ad, mtNow, wDaysBefore);
				}
			}
		}
		ListView_SortItemsEx(_hList, (CMPPROC)cmpProc, this);
		ShowWindow(_hList, SW_SHOW);

		// display statistics
		SetDlgItemInt(_hDlg, TXT_NUMBIRTH, numBirthContacts, FALSE);
		SetDlgItemInt(_hDlg, TXT_NUMCONTACT, numContacts, FALSE);
		SetDlgItemInt(_hDlg, TXT_FEMALE, numFemale, FALSE);
		SetDlgItemInt(_hDlg, TXT_MALE, numMale, FALSE);
		SetDlgItemInt(_hDlg, TXT_AGE, numBirthContacts > 0 ? max(0, (age - (age % numBirthContacts)) / numBirthContacts) : 0, FALSE);
	}

	// This method deletes all items from the listview
	void DeleteAllItems()
	{
		CItemData *pid;
		
		for (int i = 0; i < _numRows; i++) {
			pid = ItemData(i);
			if (pid)
				delete pid;
		}
		ListView_DeleteAllItems(_hList);
		_numRows = 0;
	}

	/**
	 * This method returns the data structure accociated with a list item.
	 *
	 * @param	iItem			- index of the desired item
	 *
	 * @return	pointer to the data strucutre on success or NULL otherwise.
	 **/
	CItemData *ItemData(int iItem)
	{
		if (_hList && iItem >= 0 && iItem < _numRows) {
			LVITEM lvi;
		
			lvi.mask = LVIF_PARAM;
			lvi.iItem = iItem;
			lvi.iSubItem = 0;
			if (ListView_GetItem(_hList, &lvi) && PtrIsValid(lvi.lParam))
				return (CItemData *)lvi.lParam;
		}
		return NULL;
	}

	// This method loads all filter settings from db
	void LoadFilter()
	{
		_filter.wDaysBefore = db_get_w(NULL, MODNAME, SET_ANNIVLIST_FILTER_DAYS, 9);
		_filter.bFilterIndex = db_get_b(NULL, MODNAME, SET_ANNIVLIST_FILTER_INDEX, 0);
	}

	// This method saves all filter settings to db
	void SaveFilter()
	{
		if (_hDlg) {
			db_set_w(NULL, MODNAME, SET_ANNIVLIST_FILTER_DAYS, (WORD)GetDlgItemInt(_hDlg, EDIT_DAYS, NULL, FALSE));
			db_set_b(NULL, MODNAME, SET_ANNIVLIST_FILTER_DAYSENABLED, (BYTE)Button_GetCheck(GetDlgItem(_hDlg, CHECK_DAYS)));
			db_set_b(NULL, MODNAME, SET_ANNIVLIST_FILTER_INDEX, (BYTE)ComboBox_GetCurSel(GetDlgItem(_hDlg, EDIT_DAYS)));
		}
	}

public:

	// This is the default constructor.
	CAnnivList()
	{
		_hList			= NULL;
		_sortHeader		= 0;
		_sortOrder		= 1;
		_curSel			= -1;
		_numRows		= 0;
		_wmINIT			= true;
		_rcWin.left		= _rcWin.right = _rcWin.top = _rcWin.bottom = 0;
		LoadFilter();

		_hDlg = CreateDialogParam(ghInst, MAKEINTRESOURCE(IDD_ANNIVERSARY_LIST), NULL, DlgProc, (LPARAM)this);
		if (_hDlg)
			_mHookExit = HookEventMessage(ME_SYSTEM_PRESHUTDOWN, _hDlg, WM_CLOSE);
		else {
			_mHookExit = NULL;
			delete this;
		}
	}

	// This is the default destructor.
	~CAnnivList()
	{
		// delete the shutdown hook
		if (_mHookExit) {
			UnhookEvent(_mHookExit);
			_mHookExit = NULL;
		}

		// close window if required
		if (_hDlg) {
			// save list state
			if (_hList) {
				CHAR pszSetting[MAXSETTING];
				int c, cc = Header_GetItemCount(ListView_GetHeader(_hList));

				for (c = 0; c < cc; c++) {
					mir_snprintf(pszSetting, MAXSETTING, "AnnivDlg_Col%d", c);
					db_set_w(NULL, MODNAME, pszSetting, (WORD)ListView_GetColumnWidth(_hList, c));
				}
				DeleteAllItems();
			}
			// remember popup setting
			db_set_b(NULL, MODNAME, SET_ANNIVLIST_POPUP, (BYTE)IsDlgButtonChecked(_hDlg, CHECK_POPUP));
			// save window position, size and column widths
			Utils_SaveWindowPosition(_hDlg, NULL, MODNAME, "AnnivDlg_");
			SaveFilter();

			// if the window did not yet retrieve a WM_DESTROY message, do it right now.
			if (PtrIsValid(GetUserData(_hDlg))) {
				SetUserData(_hDlg, NULL);
				DestroyWindow(_hDlg);
			}
		}	
		gpDlg = NULL;
	}

	/**
	 * name:	BringToFront
	 * class:	CAnnivList
	 * desc:	brings anniversary list to the top
	 * param:	none
	 * return:	nothing
	 **/
	void BringToFront()
	{
		ShowWindow(_hDlg, SW_RESTORE);
		SetForegroundWindow(_hDlg);
		SetFocus(_hDlg);
	}

}; // class CAnnivList

/***********************************************************************************************************
 * service handlers
 ***********************************************************************************************************/

/**
 * This is the service function that is called list all anniversaries.
 *
 * @param	wParam	- not used
 * @param	lParam	- not used
 *
 * @return	always 0
 **/
INT_PTR DlgAnniversaryListShow(WPARAM wParam, LPARAM lParam)
{
	if (!gpDlg) {
		try {
			myGlobals.WantAeroAdaption = db_get_b(NULL, MODNAME, SET_PROPSHEET_AEROADAPTION, TRUE);
			gpDlg = new CAnnivList();
		}
		catch(...) {
			delete gpDlg;
			gpDlg = NULL;
		}
	} 
	else
		gpDlg->BringToFront();
	return 0;
}

/***********************************************************************************************************
 * loading and unloading module
 ***********************************************************************************************************/

#define TBB_IDBTN		"AnnivList"
#define TBB_ICONAME		TOOLBARBUTTON_ICONIDPREFIX TBB_IDBTN TOOLBARBUTTON_ICONIDPRIMARYSUFFIX

/**
 * This function is called by the ME_TTB_MODULELOADED event.
 * It adds a set of buttons to the TopToolbar plugin.
 *
 * @param	wParam	- none
 *
 * @return	nothing
 **/
void DlgAnniversaryListOnTopToolBarLoaded()
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = MS_USERINFO_REMINDER_LIST;
	ttb.hIconHandleUp = Skin_GetIconHandle(ICO_COMMON_ANNIVERSARY);
	ttb.name = ttb.pszTooltipUp = LPGEN("Anniversary list");
	TopToolbar_AddButton(&ttb);
}

/**
 * This function initially loads all required stuff for the anniversary list.
 *
 * @param	none
 *
 * @return	nothing
 **/
void DlgAnniversaryListLoadModule()
{
	CreateServiceFunction(MS_USERINFO_REMINDER_LIST, DlgAnniversaryListShow);

	HOTKEYDESC hk = { 0 };
	hk.cbSize = sizeof(HOTKEYDESC);
	hk.pszSection = MODNAME;
	hk.pszName = "AnniversaryList";
	hk.pszDescription = LPGEN("Popup anniversary list");
	hk.pszService = MS_USERINFO_REMINDER_LIST;
	Hotkey_Register(&hk);
}
