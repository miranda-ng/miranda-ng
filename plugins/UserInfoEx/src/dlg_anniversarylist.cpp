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
#include "stdafx.h"
#include "svc_gender.h"
#include "svc_reminder.h"
#include "dlg_anniversarylist.h"

#include "m_message.h"

#define IsLeap(wYear)	(!(((wYear) % 4 != 0) || (((wYear) % 100 == 0) && ((wYear) % 400 != 0))))

class CAnnivList;

static CAnnivList *gpDlg = nullptr;

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
	uint8_t	_bRemindEnable;
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
		uint16_t	wDaysBefore = (uint16_t)-1;
		uint8_t	bFilterIndex = 0;
		LPSTR	pszProto = nullptr;
		LPTSTR pszAnniv = nullptr;
	} _filter;

	struct CItemData
	{
		MCONTACT	_hContact;
		MAnnivDate _pDate;
		uint16_t		_wDaysBefore;
		uint8_t		_wReminderState;

		CItemData(MCONTACT hContact, MAnnivDate &date) :
			_pDate(date)
		{
			_hContact = hContact;
			_wReminderState = date.RemindOption();
			_wDaysBefore = date.RemindOffset();
		}

		~CItemData()
		{
			// save changes
			if (_wReminderState != _pDate.RemindOption() || _wDaysBefore != _pDate.RemindOffset()) {
				_pDate.RemindOffset(_wDaysBefore);
				_pDate.RemindOption(_wReminderState);
				_pDate.DBWriteReminderOpts(_hContact);
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
		WINDOWPOS *_wndPos;
		HDWP _hdWnds;
		RECT _rcParent;

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

		void MoveCtrl(uint16_t idCtrl, int anchors)
		{
			if (!(_wndPos->flags & SWP_NOSIZE)) {
				HWND hCtrl = GetDlgItem(_wndPos->hwnd, idCtrl);
				if (nullptr != hCtrl) { /* Wine fix. */
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
		if (pDlg) {
			wchar_t szText1[MAX_PATH];
			wchar_t szText2[MAX_PATH];

			szText1[0] = szText2[0] = 0;
			switch (pDlg->_sortHeader) {
			case COLUMN_CONTACT:
			case COLUMN_PROTO:
			case COLUMN_DESC:
				ListView_GetItemText(pDlg->_hList, iItem1, pDlg->_sortHeader, szText1, _countof(szText1));
				ListView_GetItemText(pDlg->_hList, iItem2, pDlg->_sortHeader, szText2, _countof(szText2));
				return pDlg->_sortOrder * mir_wstrcmp(szText1, szText2);

			case COLUMN_AGE:
			case COLUMN_ETA:
				ListView_GetItemText(pDlg->_hList, iItem1, pDlg->_sortHeader, szText1, _countof(szText1));
				ListView_GetItemText(pDlg->_hList, iItem2, pDlg->_sortHeader, szText2, _countof(szText2));
				return pDlg->_sortOrder * (_wtoi(szText1) - _wtoi(szText2));

			case COLUMN_DATE: 
				CItemData *id1 = pDlg->ItemData(iItem1), *id2 = pDlg->ItemData(iItem2);
				if (PtrIsValid(id1) && PtrIsValid(id2))
					return pDlg->_sortOrder * id1->_pDate.Compare(id2->_pDate);
			}
		}
		
		return 0;
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
				hIcon = g_plugin.getIcon(IDI_ANNIVERSARY);
				SendDlgItemMessage(hDlg, IDC_HEADERBAR, WM_SETICON, 0, (LPARAM)hIcon);
				SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

				// insert columns into the listboxes
				ListView_SetExtendedListViewStyle(pDlg->_hList, LVS_EX_FULLROWSELECT);

				// add columns
				if (pDlg->AddColumn(CAnnivList::COLUMN_ETA, LPGENW("ETA"), 40)
					|| pDlg->AddColumn(CAnnivList::COLUMN_CONTACT, LPGENW("Contact"), 160)
					|| pDlg->AddColumn(CAnnivList::COLUMN_PROTO, LPGENW("Proto"), 50)
					|| pDlg->AddColumn(CAnnivList::COLUMN_AGE, LPGENW("Age/Nr."), 40)
					|| pDlg->AddColumn(CAnnivList::COLUMN_DESC, LPGENW("Anniversary"), 100)
					|| pDlg->AddColumn(CAnnivList::COLUMN_DATE, LPGENW("Date"), 80))
					break;

				TranslateDialogDefault(hDlg);
				
				// save minimal size
				GetWindowRect(hDlg, &rc);
				pDlg->_sizeMin.cx = rc.right - rc.left;
				pDlg->_sizeMin.cy = rc.bottom - rc.top;
				
				// restore position and size
				Utils_RestoreWindowPosition(hDlg, NULL, MODULENAME, "AnnivDlg_");

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
				pDlg->_bRemindEnable = g_plugin.iRemindState != REMIND_OFF;
				if (hCtrl = GetDlgItem(hDlg, CHECK_REMIND)) {
					Button_SetCheck(hCtrl, pDlg->_bRemindEnable ? BST_INDETERMINATE : BST_UNCHECKED);
					EnableWindow(hCtrl, pDlg->_bRemindEnable);
				}

				CheckDlgButton(hDlg, CHECK_POPUP, g_plugin.getByte(SET_ANNIVLIST_POPUP, FALSE) ? BST_CHECKED : BST_UNCHECKED);
				// set number of days to show contact in advance
				SetDlgItemInt(hDlg, EDIT_DAYS, pDlg->_filter.wDaysBefore, FALSE);
				if (hCtrl = GetDlgItem(hDlg, CHECK_DAYS)) {
					Button_SetCheck(hCtrl, g_plugin.getByte(SET_ANNIVLIST_FILTER_DAYSENABLED, FALSE));
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

				// show contact menu
				case NM_RCLICK:
					{
						CItemData *pid = pDlg->ItemData(pDlg->_curSel);
						if (pid) {
							HMENU hPopup = Menu_BuildContactMenu(pid->_hContact);
							if (hPopup) {
								POINT pt;
								GetCursorPos(&pt);
								TrackPopupMenu(hPopup, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hDlg, nullptr);
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
				if (pid && Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, pid->_hContact))
					break;

				switch (LOWORD(wParam)) {
				// enable/disable reminder checkbox is clicked
				case CHECK_REMIND:
					if (pDlg->_bRemindEnable && HIWORD(wParam) == BN_CLICKED) {
						uint8_t checkState = Button_GetCheck((HWND)lParam);

						EnableWindow(GetDlgItem(hDlg, EDIT_REMIND), checkState == BST_CHECKED);
						EnableWindow(GetDlgItem(hDlg, SPIN_REMIND), checkState == BST_CHECKED);
						if (pid && pid->_wReminderState != checkState)
							pid->_wReminderState = checkState;
					}
					break;

				// number of days to remind in advance is edited
				case EDIT_REMIND:
					if (pid && pDlg->_bRemindEnable && HIWORD(wParam) == EN_CHANGE) {
						uint16_t wDaysBefore = GetDlgItemInt(hDlg, LOWORD(wParam), nullptr, FALSE);
						if (pid->_wReminderState == BST_CHECKED && pid->_wDaysBefore != wDaysBefore) {
							pid->_wDaysBefore = wDaysBefore;
						}
					}
					break;

				// the filter to display only contacts which have an anniversary in a certain 
				// period of time is enabled/disabled
				case CHECK_DAYS:
					if (HIWORD(wParam) == BN_CLICKED) {
						uint8_t isChecked = Button_GetCheck((HWND)lParam);
						EnableWindow(GetDlgItem(hDlg, EDIT_DAYS), isChecked);
						EnableWindow(GetDlgItem(hDlg, TXT_DAYS), isChecked);
						pDlg->_filter.wDaysBefore = isChecked ? GetDlgItemInt(hDlg, EDIT_DAYS, nullptr, FALSE) : (uint16_t)-1;
						pDlg->RebuildList();
					}
					break;

				// the number of days a contact must have an anniversary in advance to be displayed is edited
				case EDIT_DAYS:
					if (HIWORD(wParam) == EN_CHANGE) {
						uint16_t wNewDays = GetDlgItemInt(hDlg, LOWORD(wParam), nullptr, FALSE);
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
			return Menu_DrawItem(lParam);

		case WM_MEASUREITEM:
			return Menu_MeasureItem(lParam);

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
	uint8_t AddColumn(int iSubItem, LPCTSTR pszText, int defaultWidth)
	{
		LVCOLUMN lvc;
		CHAR pszSetting[MAXSETTING];

		mir_snprintf(pszSetting, "AnnivDlg_Col%d", iSubItem);
		lvc.cx = g_plugin.getWord(pszSetting, defaultWidth);
		lvc.mask = LVCF_WIDTH | LVCF_TEXT;
		lvc.iSubItem = iSubItem;
		lvc.pszText = TranslateW(pszText);
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
	uint8_t AddSubItem(int iItem, int iSubItem, LPTSTR pszText)
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
	uint8_t AddItem(LPTSTR pszText, LPARAM lParam)
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
	uint8_t AddRow(MCONTACT hContact, LPCSTR pszProto, MAnnivDate &ad, MTime &mtNow, int wDaysBefore)
	{
		wchar_t szText[MAX_PATH];
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
				if (ad.RemindOffset() == (uint16_t)-1) {
					ad.RemindOffset(wDaysBefore);
					
					// create data object
					pdata = new CItemData(hContact, ad);
					if (!pdata)
						return FALSE;
					// add item
					iItem = AddItem(_itow(diff, szText, 10), (LPARAM)pdata);
					if (iItem == -1) {
						delete pdata;
						return FALSE;
					}

					// second column: contact name
					AddSubItem(iItem, COLUMN_CONTACT, Clist_GetContactDisplayName(hContact));

					// third column: protocol
					wchar_t *ptszProto = mir_a2u(pszProto);
					AddSubItem(iItem, COLUMN_PROTO, ptszProto);
					mir_free(ptszProto);

					// forth line: age
					if (ad.Age(&mtNow))
						AddSubItem(iItem, COLUMN_AGE, _itow(ad.Age(&mtNow), szText, 10));
					else
						AddSubItem(iItem, COLUMN_AGE, L"???");

					// fifth line: anniversary
					AddSubItem(iItem, COLUMN_DESC, (LPTSTR)ad.Description());

					// sixth line: date
					ad.DateFormatAlt(szText, _countof(szText));
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
		MAnnivDate ad;
		int i = 0;
		int totalAge = 0;
		int wDaysBefore = g_plugin.wRemindOffset;
		int numMale = 0;
		int numFemale = 0;
		int numContacts = 0;
		int numBirthContacts = 0;

		ShowWindow(_hList, SW_HIDE);
		DeleteAllItems();

		MTime mtNow;
		mtNow.GetLocalTime();

		// insert the items into the list
		for (auto &hContact : Contacts()) {
			// ignore metacontacts here, as they are not interesting.
			if (db_mc_isMeta(hContact))
				continue;

			// filter protocol
			char *pszProto = Proto_GetBaseAccountName(hContact);
			if (!pszProto)
				continue;

			numContacts++;
			switch (GenderOf(hContact, pszProto)) {
			case 'M':
				numMale++;
				break;
			case 'F':
				numFemale++;
			}

			if (!ad.DBGetBirthDate(hContact, pszProto)) {
				if (int age = ad.Age(&mtNow)) {
					totalAge += age;
					numBirthContacts++;
				}

				// add birthday
				if ((_filter.bFilterIndex != FILTER_ANNIV) && (!_filter.pszProto || !_strcmpi(pszProto, _filter.pszProto)))
					AddRow(hContact, pszProto, ad, mtNow, wDaysBefore);
			}

			// add anniversaries
			if (_filter.bFilterIndex != FILTER_BIRTHDAY && (!_filter.pszProto || !_strcmpi(pszProto, _filter.pszProto))) 
				for (i = 0; !ad.DBGetAnniversaryDate(hContact, i); i++)
					if (!_filter.pszAnniv || !mir_wstrcmpi(_filter.pszAnniv, ad.Description()))
						AddRow(hContact, pszProto, ad, mtNow, wDaysBefore);
		}

		ListView_SortItemsEx(_hList, (CMPPROC)cmpProc, this);
		ShowWindow(_hList, SW_SHOW);

		// display statistics
		SetDlgItemInt(_hDlg, TXT_NUMBIRTH, numBirthContacts, FALSE);
		SetDlgItemInt(_hDlg, TXT_NUMCONTACT, numContacts, FALSE);
		SetDlgItemInt(_hDlg, TXT_FEMALE, numFemale, FALSE);
		SetDlgItemInt(_hDlg, TXT_MALE, numMale, FALSE);
		SetDlgItemInt(_hDlg, TXT_AGE, numBirthContacts > 0 ? totalAge / numBirthContacts : 0, FALSE);
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
		return nullptr;
	}

	// This method loads all filter settings from db
	void LoadFilter()
	{
		_filter.wDaysBefore = g_plugin.getWord(SET_ANNIVLIST_FILTER_DAYS, 9);
		_filter.bFilterIndex = g_plugin.getByte(SET_ANNIVLIST_FILTER_INDEX, 0);
	}

	// This method saves all filter settings to db
	void SaveFilter()
	{
		if (_hDlg) {
			g_plugin.setWord(SET_ANNIVLIST_FILTER_DAYS, (uint16_t)GetDlgItemInt(_hDlg, EDIT_DAYS, nullptr, FALSE));
			g_plugin.setByte(SET_ANNIVLIST_FILTER_DAYSENABLED, (uint8_t)Button_GetCheck(GetDlgItem(_hDlg, CHECK_DAYS)));
			g_plugin.setByte(SET_ANNIVLIST_FILTER_INDEX, (uint8_t)ComboBox_GetCurSel(GetDlgItem(_hDlg, EDIT_DAYS)));
		}
	}

public:

	// This is the default constructor.
	CAnnivList()
	{
		_hList			= nullptr;
		_sortHeader		= 0;
		_sortOrder		= 1;
		_curSel			= -1;
		_numRows		= 0;
		_wmINIT			= true;
		_rcWin.left		= _rcWin.right = _rcWin.top = _rcWin.bottom = 0;
		LoadFilter();

		_hDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ANNIVERSARY_LIST), nullptr, DlgProc, (LPARAM)this);
		if (_hDlg)
			_mHookExit = HookEventMessage(ME_SYSTEM_PRESHUTDOWN, _hDlg, WM_CLOSE);
		else {
			_mHookExit = nullptr;
			delete this;
		}
	}

	// This is the default destructor.
	~CAnnivList()
	{
		// delete the shutdown hook
		if (_mHookExit) {
			UnhookEvent(_mHookExit);
			_mHookExit = nullptr;
		}

		// close window if required
		if (_hDlg) {
			// save list state
			if (_hList) {
				char pszSetting[MAXSETTING];
				int c, cc = Header_GetItemCount(ListView_GetHeader(_hList));

				for (c = 0; c < cc; c++) {
					mir_snprintf(pszSetting, "AnnivDlg_Col%d", c);
					g_plugin.setWord(pszSetting, (uint16_t)ListView_GetColumnWidth(_hList, c));
				}
				DeleteAllItems();
			}
			// remember popup setting
			g_plugin.setByte(SET_ANNIVLIST_POPUP, (uint8_t)IsDlgButtonChecked(_hDlg, CHECK_POPUP));
			// save window position, size and column widths
			Utils_SaveWindowPosition(_hDlg, NULL, MODULENAME, "AnnivDlg_");
			SaveFilter();

			// if the window did not yet retrieve a WM_DESTROY message, do it right now.
			if (PtrIsValid(GetUserData(_hDlg))) {
				SetUserData(_hDlg, NULL);
				DestroyWindow(_hDlg);
			}
		}	
		gpDlg = nullptr;
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
INT_PTR DlgAnniversaryListShow(WPARAM, LPARAM)
{
	if (!gpDlg) {
		myGlobals.WantAeroAdaption = g_plugin.bAero;
		gpDlg = new CAnnivList();
	} 
	else gpDlg->BringToFront();
	
	return 0;
}

/***********************************************************************************************************
 * loading and unloading module
 ***********************************************************************************************************/

#define MOD_MBIRTHDAY "mBirthday"

void DlgAnniversaryListLoadModule()
{
	CreateServiceFunction(MS_USERINFO_REMINDER_LIST, DlgAnniversaryListShow);

	HOTKEYDESC hk = {};
	hk.pszName = "AnniversaryList";
	hk.szSection.a = MODULENAME;
	hk.szDescription.a = LPGEN("Popup anniversary list");
	hk.pszService = MS_USERINFO_REMINDER_LIST;
	g_plugin.addHotkey(&hk);

	if (!g_plugin.getByte("Bug3001")) {
		// walk through all the contacts stored in the DB
		MAnnivDate mdb;
		for (auto &hContact : Contacts()) {
			if (!mdb.DBGetDate(hContact, MOD_MBIRTHDAY, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR)) {
				mdb.DBWriteDate(hContact, USERINFO, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR);
				mdb.DBDeleteDate(hContact, MOD_MBIRTHDAY, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR);
			}
		}

		g_plugin.setByte("Bug3001", 1);
	}
}
