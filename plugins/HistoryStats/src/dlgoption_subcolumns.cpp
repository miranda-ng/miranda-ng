#include "_globals.h"
#include "dlgoption.h"

#include "column.h"
#include "bandctrl.h"
#include "main.h"

/*
 * DlgOption::SubColumns
 */

INT_PTR CALLBACK DlgOption::SubColumns::staticAddProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hWndList = GetDlgItem(hDlg, IDC_COLUMN);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);

		SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_HISTORYSTATS))));

		utils::centerDialog(hDlg);

		upto_each_(i, Column::countColInfo())
		{
			int nIndex = SendMessage(hWndList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(Column::getColInfo(i).m_Title));
			SendMessage(hWndList, LB_SETITEMDATA, nIndex, static_cast<LPARAM>(i));
		}

		if (Column::countColInfo() > 0) {
			SendMessage(hWndList, LB_SETCURSEL, 0, 0);

			SetDlgItemText(hDlg, IDC_DESCRIPTION, Column::getColInfo(0).m_Description);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				int nIndex = SendMessage(hWndList, LB_GETCURSEL, 0, 0);
				if (nIndex != LB_ERR) {
					int nData = SendMessage(hWndList, LB_GETITEMDATA, nIndex, 0);

					EndDialog(hDlg, nIndex);
				}
				else EndDialog(hDlg, -1);
			}
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, -1);
			return TRUE;

		case IDC_COLUMN:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				int nIndex = SendMessage(hWndList, LB_GETCURSEL, 0, 0);
				if (nIndex != LB_ERR) {
					int nData = SendMessage(hWndList, LB_GETITEMDATA, nIndex, 0);
					SetDlgItemText(hDlg, IDC_DESCRIPTION, Column::getColInfo(nData).m_Description);
				}
				else SetDlgItemText(hDlg, IDC_DESCRIPTION, _T(""));
			}
			else if (HIWORD(wParam) == LBN_DBLCLK)
				SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDOK, 0), NULL);
			break;
		}
		break;
	}

	return FALSE;
}

DlgOption::SubColumns::SubColumns() :
	m_hColTitle(NULL),
	m_bShowInfo(true),
	m_nInfoHeight(0)
{
}

DlgOption::SubColumns::~SubColumns()
{
	g_pSettings->setShowColumnInfo(m_bShowInfo);
}

BOOL DlgOption::SubColumns::handleMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_WINDOWPOSCHANGED:
		rearrangeControls();
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_INFOLABEL && HIWORD(wParam) == STN_CLICKED) {
			m_bShowInfo = !m_bShowInfo;
			toggleInfo();
		}
		break;

	case WM_NOTIFY:
		switch (reinterpret_cast<NMHDR*>(lParam)->idFrom) {
		case IDC_BAND:
			{
				BandCtrl::NMBANDCTRL* pNM = reinterpret_cast<BandCtrl::NMBANDCTRL*>(lParam);
				if (pNM->hdr.code == BandCtrl::BCN_CLICKED)
					onBandClicked(pNM->hButton, pNM->dwData);
				else if (pNM->hdr.code == BandCtrl::BCN_DROPDOWN)
					onBandDropDown(pNM->hButton, pNM->dwData);
			}
			break;

		case IDC_COLUMNS:
			{
				OptionsCtrl::NMOPTIONSCTRL* pNM = reinterpret_cast<OptionsCtrl::NMOPTIONSCTRL*>(lParam);
				if (pNM->hdr.code == OptionsCtrl::OCN_MODIFIED) {
					getParent()->settingsChanged();
					getParent()->updateProblemInfo();
				}
				else if (pNM->hdr.code == OptionsCtrl::OCN_SELCHANGED) {
					onColSelChanged(pNM->hItem, pNM->dwData);
				}
				else if (pNM->hdr.code == OptionsCtrl::OCN_SELCHANGING) {
					onColSelChanging(pNM->hItem, pNM->dwData);
				}
				else if (pNM->hdr.code == OptionsCtrl::OCN_ITEMDROPPED) {
					OptionsCtrl::NMOPTIONSCTRLDROP* pNM2 = reinterpret_cast<OptionsCtrl::NMOPTIONSCTRLDROP*>(pNM);

					onColItemDropped(pNM2->hItem, pNM2->hDropTarget, pNM2->bAbove);
				}
			}
			break;

		case IDC_OPTIONS:
			{
				OptionsCtrl::NMOPTIONSCTRL * pNM = reinterpret_cast<OptionsCtrl::NMOPTIONSCTRL*>(lParam);

				if (pNM->hdr.code == OptionsCtrl::OCN_MODIFIED)
					getParent()->settingsChanged();
				else if (pNM->hdr.code == OptionsCtrl::OCN_CLICKED)
					onColumnButton(pNM->hItem, pNM->dwData);
			}
			break;

		case IDC_INFO:
			NMTREEVIEW* pNM = reinterpret_cast<NMTREEVIEW*>(lParam);
			if (pNM->hdr.code == TVN_ITEMEXPANDING) {
				if (pNM->action == TVE_COLLAPSE || pNM->action == TVE_COLLAPSERESET ||
					 (pNM->action == TVE_TOGGLE && pNM->itemNew.state & TVIS_EXPANDED)) {
					SetWindowLongPtr(getHWnd(), DWLP_MSGRESULT, TRUE);
					return TRUE;
				}
			}
			break;
		}
		break;
	}

	return FALSE;
}

void DlgOption::SubColumns::onWMInitDialog()
{
	TranslateDialogDefault(getHWnd());

	// init column band
	m_Band << GetDlgItem(getHWnd(), IDC_BAND);

	static const struct
	{
		WORD iconId;
		TCHAR* szTooltip;
		bool bRight;
		bool bDropDown;
		bool bDisabled;
	}
	columnBand[] = {
		{ IDI_COL_ADD, LPGENT("Add column..."), false, true, false },
		{ IDI_COL_DEL, LPGENT("Delete column"), false, false, true },
		{ IDI_COL_DOWN, LPGENT("Move down"), true, false, true },
		{ IDI_COL_UP, LPGENT("Move up"), true, false, true },
	};

	array_each_(i, columnBand)
	{
		HICON hIcon = reinterpret_cast<HICON>(LoadImage(g_hInst, MAKEINTRESOURCE(columnBand[i].iconId), IMAGE_ICON, OS::smIconCX(), OS::smIconCY(), 0));
		DWORD dwFlags =
			(columnBand[i].bRight ? BandCtrl::BCF_RIGHT : 0) |
			(columnBand[i].bDropDown ? BandCtrl::BCF_DROPDOWN : 0) |
			(columnBand[i].bDisabled ? BandCtrl::BCF_DISABLED : 0);

		m_hActionButtons[i] = m_Band.addButton(dwFlags, hIcon, i, TranslateTS(columnBand[i].szTooltip));

		DestroyIcon(hIcon);
	}

	// init options controls
	m_Columns << GetDlgItem(getHWnd(), IDC_COLUMNS);

	// init option tree(s)
	m_Options << GetDlgItem(getHWnd(), IDC_OPTIONS);

	// int column info
	HWND hInfo = GetDlgItem(getHWnd(), IDC_INFO);

	SetWindowLongPtr(hInfo, GWL_STYLE, GetWindowLongPtr(hInfo, GWL_STYLE) | TVS_NOHSCROLL);

	m_bShowInfo = g_pSettings->getShowColumnInfo();
	toggleInfo();
}

void DlgOption::SubColumns::onWMDestroy()
{
	// avoid OCN_SELCHANGING messages when options are already destroyed
	m_Columns.deleteAllItems();
}

void DlgOption::SubColumns::loadSettings()
{
	// remeber currently selected item
	HANDLE hSelItem = m_Columns.getSelection();
	Column* pPrevCol = NULL;

	if (hSelItem) {
		pPrevCol = reinterpret_cast<Column*>(m_Columns.getItemData(hSelItem));
		hSelItem = NULL;
	}

	// remember scroll bar position
	int nScrollPosV = m_Columns.getScrollPos(SB_VERT);

	// refill column tree
	m_Columns.setRedraw(false);
	m_Columns.deleteAllItems();

	Settings& localS = getParent()->getLocalSettings();

	upto_each_(j, localS.countCol())
	{
		Column* pCol = localS.getCol(j);
		ext::string colTitle = pCol->getTitleForOptions();
		OptionsCtrl::Check hColCheck = m_Columns.insertCheck(NULL, colTitle.c_str(), 0, reinterpret_cast<DWORD>(pCol));

		m_Columns.checkItem(hColCheck, pCol->isEnabled());

		if (!hSelItem && pCol == pPrevCol)
			hSelItem = hColCheck;
	}

	m_Columns.ensureVisible(NULL);
	m_Columns.setRedraw(true);

	// restore scroll bar position
	m_Columns.setScrollPos(SB_VERT, nScrollPosV);

	// restore selected item
	m_Columns.selectItem(hSelItem);
}

void DlgOption::SubColumns::saveSettings()
{
	HANDLE hSelItem = m_Columns.getSelection();

	if (hSelItem) {
		// deselect and select current item to save its options to localS
		m_Columns.selectItem(NULL);
		m_Columns.selectItem(hSelItem);
	}

	HANDLE hItem = m_Columns.getFirstItem();
	while (hItem) {
		Column* pCol = reinterpret_cast<Column*>(m_Columns.getItemData(hItem));
		if (pCol)
			pCol->setEnabled(m_Columns.isItemChecked(hItem));

		hItem = m_Columns.getNextItem(hItem);
	}
}

void DlgOption::SubColumns::rearrangeControls()
{
	RECT rClient, rWnd;
	int offsetY;

	if (m_nInfoHeight == 0) {
		m_nInfoHeight = utils::getWindowRect(getHWnd(), IDC_INFO).bottom;
		m_nInfoHeight -= utils::getWindowRect(getHWnd(), IDC_INFOLABEL).bottom;
	}

	GetClientRect(getHWnd(), &rClient);

	// columns tree
	rWnd = utils::getWindowRect(getHWnd(), m_Columns);
	rWnd.bottom = rClient.bottom;
	utils::moveWindow(m_Columns, rWnd);

	// column info list
	rWnd = utils::getWindowRect(getHWnd(), IDC_INFO);
	offsetY = rClient.bottom + (m_bShowInfo ? 0 : m_nInfoHeight) - rWnd.bottom;
	OffsetRect(&rWnd, 0, offsetY);
	rWnd.right = rClient.right;
	utils::moveWindow(getHWnd(), IDC_INFO, rWnd);

	// column info list label
	rWnd = utils::getWindowRect(getHWnd(), IDC_INFOLABEL);
	OffsetRect(&rWnd, 0, offsetY);
	rWnd.right = rClient.right;
	utils::moveWindow(getHWnd(), IDC_INFOLABEL, rWnd);

	// options tree
	rWnd = utils::getWindowRect(getHWnd(), m_Options);
	rWnd.right = rClient.right;
	rWnd.bottom += offsetY;
	utils::moveWindow(m_Options, rWnd);
}

void DlgOption::SubColumns::toggleInfo()
{
	HWND hInfo = GetDlgItem(getHWnd(), IDC_INFO);
	const TCHAR* szInfoLabelText = m_bShowInfo ? LPGENT("Hide additional column info...") : LPGENT("Show additional column info...");

	SetDlgItemText(getHWnd(), IDC_INFOLABEL, TranslateTS(szInfoLabelText));
	ShowWindow(hInfo, m_bShowInfo ? SW_SHOW : SW_HIDE);
	EnableWindow(hInfo, BOOL_(m_bShowInfo));

	rearrangeControls();
}

void DlgOption::SubColumns::addCol(int nCol)
{
	if (nCol != -1) {
		Column* pCol = Column::fromUID(Column::getColInfo(nCol).m_UID);

		getParent()->getLocalSettings().addCol(pCol);

		OptionsCtrl::Check hColCheck = m_Columns.insertCheck(NULL, pCol->getTitle(), 0, reinterpret_cast<DWORD>(pCol));

		m_Columns.checkItem(hColCheck, pCol->isEnabled());

		getParent()->settingsChanged();
	}
}

void DlgOption::SubColumns::onColSelChanging(HANDLE hItem, INT_PTR dwData)
{
	if (hItem) {
		Column* pCol = reinterpret_cast<Column*>(m_Columns.getItemData(hItem));

		// general column settings
		if (m_hColTitle) {
			pCol->setCustomTitle(m_Options.getEditString(m_hColTitle));

			// adjust title in column tree
			ext::string colTitle = pCol->getTitleForOptions();

			m_Columns.setItemLabel(hItem, colTitle.c_str());

			m_hColTitle = NULL;
		}

		// column specific settings
		pCol->configFromUI(m_Options);

		// check for conflicts: PNG output vs. column options
		getParent()->updateProblemInfo();
	}

	// clear column settings
	m_Options.setRedraw(false);
	m_Options.deleteAllItems();
	m_Options.setRedraw(true);

	// clear output info
	HWND hInfo = GetDlgItem(getHWnd(), IDC_INFO);
	TreeView_DeleteAllItems(hInfo);
}

void DlgOption::SubColumns::onColSelChanged(HANDLE hItem, INT_PTR dwData)
{
	m_Options.setRedraw(false);
	m_Options.deleteAllItems();

	if (hItem) {
		Column* pCol = reinterpret_cast<Column*>(m_Columns.getItemData(hItem));

		// general column settings
		OptionsCtrl::Group hGeneral = m_Options.insertGroup(NULL, TranslateT("General column settings"), OptionsCtrl::OCF_ROOTGROUP);

		m_hColTitle = m_Options.insertEdit(hGeneral, TranslateT("Title (default if empty)"), pCol->getCustomTitle().c_str());

		// column specific settings
		if (pCol->getFeatures() & Column::cfHasConfig) {
			OptionsCtrl::Group hSpecific = m_Options.insertGroup(NULL, TranslateT("Column specific settings"), OptionsCtrl::OCF_ROOTGROUP);

			pCol->configToUI(m_Options, hSpecific);
		}

		m_Options.ensureVisible(NULL);

		// show output info for current column
		HWND hInfo = GetDlgItem(getHWnd(), IDC_INFO);
		TVINSERTSTRUCT tvi;
		bool bPNGOutput = getParent()->isPNGOutput();
		int nPNGMode = getParent()->getPNGMode();
		int restrictions = pCol->configGetRestrictions(NULL);

		SendMessage(hInfo, WM_SETREDRAW, FALSE, 0);
		TreeView_DeleteAllItems(hInfo);

		tvi.hParent = TVI_ROOT;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.mask = TVIF_TEXT | TVIF_STATE;
		tvi.item.state = TVIS_EXPANDED;
		tvi.item.stateMask = TVIS_EXPANDED;

		tvi.item.pszText = const_cast<TCHAR*>(TranslateT("For this config the selected column..."));
		tvi.hParent = TreeView_InsertItem(hInfo, &tvi);

		// show capabilities of column
		ext::string msg = TranslateT("...can be output as: ");

		if (restrictions & Column::crHTMLMask) {
			// MEMO: don't distinguish between full/partial since not yet supported
			msg += TranslateT("HTML");
		}

		if (restrictions & Column::crPNGMask) {
			if (restrictions & Column::crHTMLMask)
				msg += _T(", ");

			msg += ((restrictions & Column::crPNGMask) == Column::crPNGPartial) ? TranslateT("PNG (partial)") : TranslateT("PNG");
		}

		tvi.item.pszText = const_cast<TCHAR*>(msg.c_str());
		TreeView_InsertItem(hInfo, &tvi);

		// show effect of current config
		msg = TranslateT("...will be output as: ");

		/*
		 * the logic (output mode -> restrictions -> effect):
		 *
		 *   HTML           -> HTML-full               -> HTML
		 *                  -> !HTML-full              -> nothing (column will be ignored)
		 *
		 *   PNG-fallback   -> HTML-full               -> HTML
		 *                  -> HTML-full | PNG-partial -> HTML as fallback
		 *                  -> (HTML-full |) PNG-full  -> PNG
		 *
		 *   PNG-enforce    -> HTML-full               -> HTML
		 *                  -> HTML-full | PNG-partial -> PNG, ignoring some settings
		 *                  -> (HTML-full |) PNG-full  -> PNG
		 *
		 *   PNG-preferHTML -> HTML-full (| PNG-...)   -> HTML
		 *                  -> PNG-full                -> PNG
		 */

		if (!bPNGOutput) {
			msg += ((restrictions & Column::crHTMLMask) == Column::crHTMLFull) ? TranslateT("HTML") : TranslateT("Nothing (column will be skipped)");
		}
		else if (nPNGMode != Settings::pmPreferHTML) // && bPNGOutput
		{
			if (restrictions == (Column::crHTMLFull | Column::crPNGPartial))
				msg += (nPNGMode == Settings::pmHTMLFallBack) ? TranslateT("HTML as fallback") : TranslateT("PNG, ignoring some settings");
			else // !(html-full | png-partial)
				msg += ((restrictions & Column::crPNGMask) == Column::crPNGFull) ? TranslateT("PNG") : TranslateT("HTML");
		}
		else // bPNGOutput && nPNGMode == Settings::pmPreferHTML
		{
			msg += ((restrictions & Column::crHTMLMask) == Column::crHTMLFull) ? TranslateT("HTML") : TranslateT("PNG");
		}

		tvi.item.pszText = const_cast<TCHAR*>(msg.c_str());
		TreeView_InsertItem(hInfo, &tvi);

		SendMessage(hInfo, WM_SETREDRAW, TRUE, 0);
	}

	// (de)activate band controls
	m_Band.enableButton(m_hActionButtons[caDel], bool_(hItem));
	m_Band.enableButton(m_hActionButtons[caMoveUp], hItem && m_Columns.getPrevItem(hItem));
	m_Band.enableButton(m_hActionButtons[caMoveDown], hItem && m_Columns.getNextItem(hItem));

	m_Options.setRedraw(true);
}

void DlgOption::SubColumns::onColItemDropped(HANDLE hItem, HANDLE hDropTarget, BOOL bAbove)
{
	// check if dropped before or after hItem and abort
	if (hItem == hDropTarget)
		return;

	// convert dropped below to dropped above
	if (!bAbove)
		hDropTarget = m_Columns.getPrevItem(hDropTarget);

	// check if dropped before or after hItem and abort (part 2)
	if (hItem == hDropTarget || (hDropTarget && m_Columns.getNextItem(hDropTarget) == hItem))
		return;

	// perform move
	Column* pItem = reinterpret_cast<Column*>(m_Columns.getItemData(hItem));
	Column* pDropTaregt = hDropTarget ? reinterpret_cast<Column*>(m_Columns.getItemData(hDropTarget)) : NULL;

	m_Columns.selectItem(NULL);

	getParent()->getLocalSettings().moveCol(pItem, pDropTaregt);
	m_Columns.moveItem(hItem, hDropTarget);

	m_Columns.selectItem(hItem);

	getParent()->settingsChanged();
}

void DlgOption::SubColumns::onBandClicked(HANDLE hButton, INT_PTR dwData)
{
	switch (dwData) {
	case caAdd:
		onAdd();
		break;

	case caDel:
		onDel();
		break;

	case caMoveUp:
		onMoveUp();
		break;

	case caMoveDown:
		onMoveDown();
		break;
	}
}

void DlgOption::SubColumns::onBandDropDown(HANDLE hButton, INT_PTR dwData)
{
	if (dwData == caAdd) {
		// determine position for popup menu
		RECT rItem = m_Band.getButtonRect(hButton);
		POINT ptMenu = { rItem.left, rItem.bottom };

		ClientToScreen(m_Band, &ptMenu);

		// create and display popup menu
		HMENU hPopup = CreatePopupMenu();

		upto_each_(i, Column::countColInfo())
		{
			AppendMenu(hPopup, MF_STRING, i + 1, Column::getColInfo(i).m_Title);
		}

		int nCol = -1 + TrackPopupMenu(
			hPopup,
			TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON,
			ptMenu.x,
			ptMenu.y,
			0,
			getHWnd(),
			NULL);

		DestroyMenu(hPopup);

		addCol(nCol);
	}
}

void DlgOption::SubColumns::onAdd()
{
	int nCol = DialogBox(g_hInst, MAKEINTRESOURCE(IDD_COLADD), getHWnd(), staticAddProc);

	addCol(nCol);
}

void DlgOption::SubColumns::onDel()
{
	HANDLE hSelItem = m_Columns.getSelection();

	if (hSelItem) {
		Column* pCol = reinterpret_cast<Column*>(m_Columns.getItemData(hSelItem));

		if (pCol) {
			getParent()->getLocalSettings().delCol(pCol);
			m_Columns.deleteItem(hSelItem);

			getParent()->settingsChanged();
		}
	}
}

void DlgOption::SubColumns::onMoveUp()
{
	HANDLE hSel, hPrev2;

	if (!(hSel = m_Columns.getSelection()))
		return;

	if (!(hPrev2 = m_Columns.getPrevItem(hSel)))
		return;

	hPrev2 = m_Columns.getPrevItem(hPrev2);

	Column* pSelCol = reinterpret_cast<Column*>(m_Columns.getItemData(hSel));
	Column* pPrev2Col = hPrev2 ? reinterpret_cast<Column*>(m_Columns.getItemData(hPrev2)) : NULL;

	m_Columns.selectItem(NULL);

	getParent()->getLocalSettings().moveCol(pSelCol, pPrev2Col);
	m_Columns.moveItem(hSel, hPrev2);

	m_Columns.selectItem(hSel);

	getParent()->settingsChanged();
}

void DlgOption::SubColumns::onMoveDown()
{
	HANDLE hSel, hNext;

	if (!(hSel = m_Columns.getSelection()))
		return;

	if (!(hNext = m_Columns.getNextItem(hSel)))
		return;

	Column* pSelCol = reinterpret_cast<Column*>(m_Columns.getItemData(hSel));
	Column* pNextCol = reinterpret_cast<Column*>(m_Columns.getItemData(hNext));

	m_Columns.selectItem(NULL);

	getParent()->getLocalSettings().moveCol(pSelCol, pNextCol);
	m_Columns.moveItem(hSel, hNext);

	m_Columns.selectItem(hSel);

	getParent()->settingsChanged();
}

void DlgOption::SubColumns::onColumnButton(HANDLE hButton, INT_PTR dwData)
{
	if (dwData == Settings::biFilterWords) {
		HANDLE hSel = m_Columns.getSelection();
		Column* pCol = reinterpret_cast<Column*>(m_Columns.getItemData(hSel));

		if (hSel && pCol) {
			if (getParent()->getLocalSettings().manageFilterWords(getHWnd(), pCol)) {
				getParent()->settingsChanged();
			}
		}
	}
}

bool DlgOption::SubColumns::configHasConflicts(HelpVec* pHelp)
{
	bool bPNGOutput = getParent()->isPNGOutput();
	int nPNGMode = getParent()->getPNGMode();
	HANDLE hItem = m_Columns.getFirstItem();

	int nConflicts = 0;
	ext::string curDetails;

	while (hItem) {
		if (m_Columns.isItemChecked(hItem)) {
			Column* pCol = reinterpret_cast<Column*>(m_Columns.getItemData(hItem));

			if (pCol) {
				int restrictions = pCol->configGetRestrictions(pHelp ? &curDetails : NULL);

				// sanity check: either HTML or PNG has to be fully supported
				if ((restrictions & Column::crHTMLMask) != Column::crHTMLFull &&
					 (restrictions & Column::crPNGMask) != Column::crPNGFull) {
					MessageBox(NULL,
						TranslateT("An internal column configuration error occured. Please contact the author of this plugin."),
						TranslateT("HistoryStats - Error"), MB_ICONERROR | MB_OK);
				}

				do {
					// HTML-only output but column doesn't support it
					if (!bPNGOutput && !(restrictions & Column::crHTMLPartial)) {
						if (pHelp) {
							pHelp->push_back(HelpPair());

							pHelp->back().first = pCol->getTitle();
							pHelp->back().first += _T(": ");
							pHelp->back().first += TranslateT("HTML output unsupported.");

							pHelp->back().second = _T("");
						}

						++nConflicts;
						break;
					}

					// PNG output but only partial PNG support (enforce mode) -or-
					// PNG output with alternative full HTML-only support (fallback mode)
					if (bPNGOutput &&
						 (restrictions & Column::crPNGMask) == Column::crPNGPartial &&
						 (restrictions & Column::crHTMLMask) == Column::crHTMLFull &&
						 nPNGMode != Settings::pmPreferHTML) {
						if (pHelp) {
							pHelp->push_back(HelpPair());

							pHelp->back().first = pCol->getTitle();
							pHelp->back().first += _T(": ");

							if (nPNGMode == Settings::pmHTMLFallBack)
								pHelp->back().first += TranslateT("Fallback to HTML due to setting.");
							else
								pHelp->back().first += TranslateT("Setting ignored due to PNG output.");

							pHelp->back().second = curDetails;
						}

						++nConflicts;
						break;
					}
				}
				while (false);
			}
		}

		hItem = m_Columns.getNextItem(hItem);
	}

	return (nConflicts > 0);
}
