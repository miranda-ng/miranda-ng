#include "_globals.h"
#include "optionsctrlimpl.h"

#include "main.h"
#include "iconlib.h"

/*
 * OptionsCtrlImpl
 */

const TCHAR* OptionsCtrlImpl::m_ClassName = _T("HistoryStatsOptions");
HIMAGELIST OptionsCtrlImpl::m_hStateIcons = NULL;
int OptionsCtrlImpl::m_nStateIconsRef = 0;

LRESULT CALLBACK OptionsCtrlImpl::staticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OptionsCtrlImpl* pCtrl = reinterpret_cast<OptionsCtrlImpl*>(GetWindowLongPtr(hWnd, 0));

	switch (msg) {
	case WM_NCCREATE:
		{
			CREATESTRUCT* pCS = reinterpret_cast<CREATESTRUCT*>(lParam);

			pCS->style &= ~(WS_BORDER | WS_HSCROLL | WS_VSCROLL);
			pCS->style |= WS_CHILD;
			pCS->dwExStyle &= ~(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_WINDOWEDGE);

			pCtrl = new OptionsCtrlImpl(hWnd, reinterpret_cast<int>(pCS->hMenu));
			SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(pCtrl));
		}
		return pCtrl ? TRUE : FALSE;

	case WM_CREATE:
		return pCtrl->onWMCreate(reinterpret_cast<CREATESTRUCT*>(lParam));

	case WM_DESTROY:
		pCtrl->onWMDestroy();
		delete pCtrl;
		SetWindowLongPtr(hWnd, 0, 0);
		return 0;

	case WM_SETFOCUS:
		SetFocus(pCtrl->m_hTree);
		return 0;

	case WM_ENABLE:
		EnableWindow(pCtrl->m_hTree, wParam);
		return 0;

	case WM_GETFONT:
		return SendMessage(pCtrl->m_hTree, WM_GETFONT, wParam, lParam);

	case WM_SETFONT:
		return SendMessage(pCtrl->m_hTree, WM_SETFONT, wParam, lParam);

	case WM_WINDOWPOSCHANGED:
		{
			WINDOWPOS* pWP = reinterpret_cast<WINDOWPOS*>(lParam);
			SetWindowPos(pCtrl->m_hTree, NULL, 0, 0, pWP->cx, pWP->cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		return 0;

	case WM_SETREDRAW:
		return SendMessage(pCtrl->m_hTree, WM_SETREDRAW, wParam, lParam);

	case WM_NOTIFY:
		{
			NMHDR* pNM = reinterpret_cast<NMHDR*>(lParam);
			if (pNM->hwndFrom == pCtrl->m_hTree) {
				switch (pNM->code) {
				case NM_CLICK:
					pCtrl->onNMClick();
					return 0;

				case NM_CUSTOMDRAW:
					return pCtrl->onNMCustomDraw(reinterpret_cast<NMTVCUSTOMDRAW*>(pNM));

				case NM_KILLFOCUS:
					pCtrl->onNMKillFocus();
					return 0;

				case TVN_ITEMEXPANDING:
					return pCtrl->onTVNItemExpanding(reinterpret_cast<NMTREEVIEW*>(pNM));

				case TVN_DELETEITEM:
					pCtrl->onTVNDeleteItem(reinterpret_cast<NMTREEVIEW*>(pNM));
					return 0;

				case TVN_SELCHANGING:
					pCtrl->onTVNSelChanging(reinterpret_cast<NMTREEVIEW*>(pNM));
					return 0;

				case TVN_SELCHANGED:
					pCtrl->onTVNSelChanged(reinterpret_cast<NMTREEVIEW*>(pNM));
					return 0;

				case TVN_BEGINDRAG:
					pCtrl->onTVNBeginDrag(reinterpret_cast<NMTREEVIEW*>(pNM));
					return 0;
				}
			}
		}
		break;

	case WM_MOUSEMOVE:
		{
			POINTS pts = MAKEPOINTS(lParam);
			POINT pt = { pts.x, pts.y };
			pCtrl->onWMMouseMove(pt);
		}
		return 0;

	case WM_LBUTTONUP:
		{
			POINTS pts = MAKEPOINTS(lParam);
			POINT pt = { pts.x, pts.y };
			pCtrl->onWMLButtonUp(pt);
		}
		return 0;

	case OCM_INSERTGROUP:
		return reinterpret_cast<LRESULT>(pCtrl->onOCMInsertGroup(reinterpret_cast<HTREEITEM>(wParam), reinterpret_cast<OCGROUP*>(lParam)));

	case OCM_INSERTCHECK:
		return reinterpret_cast<LRESULT>(pCtrl->onOCMInsertCheck(reinterpret_cast<HTREEITEM>(wParam), reinterpret_cast<OCCHECK*>(lParam)));

	case OCM_INSERTRADIO:
		return reinterpret_cast<LRESULT>(pCtrl->onOCMInsertRadio(reinterpret_cast<HTREEITEM>(wParam), reinterpret_cast<OCRADIO*>(lParam)));

	case OCM_INSERTEDIT:
		return reinterpret_cast<LRESULT>(pCtrl->onOCMInsertEdit(reinterpret_cast<HTREEITEM>(wParam), reinterpret_cast<OCEDIT*>(lParam)));

	case OCM_INSERTCOMBO:
		return reinterpret_cast<LRESULT>(pCtrl->onOCMInsertCombo(reinterpret_cast<HTREEITEM>(wParam), reinterpret_cast<OCCOMBO*>(lParam)));

	case OCM_INSERTBUTTON:
		return reinterpret_cast<LRESULT>(pCtrl->onOCMInsertButton(reinterpret_cast<HTREEITEM>(wParam), reinterpret_cast<OCBUTTON*>(lParam)));

	case OCM_GETITEMLABEL:
		return reinterpret_cast<LRESULT>(pCtrl->onOCMGetItemLabel(reinterpret_cast<HTREEITEM>(wParam)));

	case OCM_SETITEMLABEL:
		pCtrl->onOCMSetItemLabel(reinterpret_cast<HTREEITEM>(wParam), reinterpret_cast<const TCHAR*>(lParam));
		return 0;

	case OCM_ISITEMENABLED:
		return BOOL_(pCtrl->onOCMIsItemEnabled(reinterpret_cast<HTREEITEM>(wParam)));

	case OCM_ENABLEITEM:
		pCtrl->onOCMEnableItem(reinterpret_cast<HTREEITEM>(wParam), bool_(lParam));
		return 0;

	case OCM_GETITEMDATA:
		return pCtrl->onOCMGetItemData(reinterpret_cast<HTREEITEM>(wParam));

	case OCM_SETITEMDATA:
		pCtrl->onOCMSetItemData(reinterpret_cast<HTREEITEM>(wParam), lParam);
		return 0;

	case OCM_ISITEMCHECKED:
		return BOOL_(pCtrl->onOCMIsItemChecked(reinterpret_cast<HTREEITEM>(wParam)));

	case OCM_CHECKITEM:
		pCtrl->onOCMCheckItem(reinterpret_cast<HTREEITEM>(wParam), bool_(lParam));
		return 0;

	case OCM_GETRADIOCHECKED:
		return pCtrl->onOCMGetRadioChecked(reinterpret_cast<HTREEITEM>(wParam));

	case OCM_SETRADIOCHECKED:
		pCtrl->onOCMSetRadioChecked(reinterpret_cast<HTREEITEM>(wParam), lParam);
		return 0;

	case OCM_GETEDITNUMBER:
		return pCtrl->onOCMGetEditNumber(reinterpret_cast<HTREEITEM>(wParam));

	case OCM_SETEDITNUMBER:
		pCtrl->onOCMSetEditNumber(reinterpret_cast<HTREEITEM>(wParam), lParam);
		return 0;

	case OCM_GETEDITSTRING:
		return reinterpret_cast<LPARAM>(pCtrl->onOCMGetEditString(reinterpret_cast<HTREEITEM>(wParam)));

	case OCM_SETEDITSTRING:
		pCtrl->onOCMSetEditString(reinterpret_cast<HTREEITEM>(wParam), reinterpret_cast<const TCHAR*>(lParam));
		return 0;

	case OCM_ADDCOMBOITEM:
		pCtrl->onOCMAddComboItem(reinterpret_cast<HTREEITEM>(wParam), reinterpret_cast<const TCHAR*>(lParam));
		return 0;

	case OCM_GETCOMBOSELECTED:
		return pCtrl->onOCMGetComboSelected(reinterpret_cast<HTREEITEM>(wParam));

	case OCM_SETCOMBOSELECTED:
		pCtrl->onOCMSetComboSelected(reinterpret_cast<HTREEITEM>(wParam), lParam);
		return 0;

	case OCM_ENSUREVISIBLE:
		pCtrl->onOCMEnsureVisible(reinterpret_cast<HTREEITEM>(wParam));
		return 0;

	case OCM_DELETEALLITEMS:
		pCtrl->onOCMDeleteAllItems();
		return 0;

	case OCM_GETSELECTION:
		return reinterpret_cast<LRESULT>(pCtrl->onOCMGetSelection());

	case OCM_SELECTITEM:
		pCtrl->onOCMSelectItem(reinterpret_cast<HTREEITEM>(wParam));
		return 0;

	case OCM_GETITEM:
		return reinterpret_cast<LRESULT>(pCtrl->onOCMGetItem(reinterpret_cast<HTREEITEM>(wParam), lParam));

	case OCM_DELETEITEM:
		pCtrl->onOCMDeleteItem(reinterpret_cast<HTREEITEM>(wParam));
		return 0;

	case OCM_MOVEITEM:
		pCtrl->onOCMMoveItem(*reinterpret_cast<HTREEITEM*>(wParam), reinterpret_cast<HTREEITEM>(lParam));
		return 0;

	case OCM_GETSCROLLPOS:
		return GetScrollPos(pCtrl->m_hTree, wParam);

	case OCM_SETSCROLLPOS:
		SetScrollPos(pCtrl->m_hTree, wParam, lParam, TRUE);
		return 0;

	case OCM_INSERTDATETIME:
		return reinterpret_cast<LRESULT>(pCtrl->onOCMInsertDateTime(reinterpret_cast<HTREEITEM>(wParam), reinterpret_cast<OCDATETIME*>(lParam)));

	case OCM_ISDATETIMENONE:
		return BOOL_(pCtrl->onOCMIsDateTimeNone(reinterpret_cast<HTREEITEM>(wParam)));

	case OCM_SETDATETIMENONE:
		pCtrl->onOCMSetDateTimeNone(reinterpret_cast<HTREEITEM>(wParam));
		return 0;

	case OCM_GETDATETIME:
		return pCtrl->onOCMGetDateTime(reinterpret_cast<HTREEITEM>(wParam), reinterpret_cast<BOOL*>(lParam));

	case OCM_SETDATETIME:
		pCtrl->onOCMSetDateTime(reinterpret_cast<HTREEITEM>(wParam), lParam);
		return 0;

	case OCM_INSERTCOLOR:
		return reinterpret_cast<LRESULT>(pCtrl->onOCMInsertColor(reinterpret_cast<HTREEITEM>(wParam), reinterpret_cast<OCCOLOR*>(lParam)));

	case OCM_GETITEMCOLOR:
		return pCtrl->onOCMGetItemColor(reinterpret_cast<HTREEITEM>(wParam));

	case OCM_SETITEMCOLOR:
		pCtrl->onOCMSetItemColor(reinterpret_cast<HTREEITEM>(wParam), lParam);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK OptionsCtrlImpl::staticTreeProc(HWND hTree, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OptionsCtrlImpl* pCtrl = reinterpret_cast<OptionsCtrlImpl*>(GetWindowLongPtr(GetParent(hTree), 0));

	switch (msg) {
	case WM_LBUTTONDOWN:
		pCtrl->onTreeWMLButtonDown(wParam, MAKEPOINTS(lParam));
		break;

	case WM_CHAR:
		pCtrl->onTreeWMChar(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_KEYDOWN:
		pCtrl->onTreeWMKeyDown(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_VSCROLL:
	case WM_HSCROLL:
	case WM_MOUSEWHEEL:
	case WM_WINDOWPOSCHANGED:
		pCtrl->onTreeXScroll(); // stops editing
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ccEdit:
			if (HIWORD(wParam) == EN_CHANGE)
				pCtrl->onENChange();
			else if (HIWORD(wParam) == EN_KILLFOCUS)
				pCtrl->onNMKillFocus();
			return 0;

		case ccCombo:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				pCtrl->onCBNSelChange();
			else if (HIWORD(wParam) == CBN_KILLFOCUS)
				pCtrl->onNMKillFocus();
			return 0;

		case ccButton:
			if (HIWORD(wParam) == BN_CLICKED)
				pCtrl->onBNClicked();
			else if (HIWORD(wParam) == BN_KILLFOCUS)
				pCtrl->onNMKillFocus();
			return 0;

		case ccColor:
			if (HIWORD(wParam) == CPN_COLOURCHANGED)
				pCtrl->onCPNColorChanged();
			return 0;
		}
		break;

	case WM_NOTIFY:
		NMHDR* pNM = reinterpret_cast<NMHDR*>(lParam);

		if (pNM->idFrom == ccDateTime) {
			if (pNM->code == DTN_DATETIMECHANGE)
				pCtrl->onDTNDateTimeChange();
			else if (pNM->code == NM_KILLFOCUS)
				pCtrl->onNMKillFocus();
			return 0;
		}
		break;
	}

	return CallWindowProc(pCtrl->m_pfnOldTreeProc, hTree, msg, wParam, lParam);
}

void OptionsCtrlImpl::staticInitStateImages()
{
	if (m_nStateIconsRef++ == 0 && !m_hStateIcons) {
		m_hStateIcons = ImageList_Create(OS::smIconCX(), OS::smIconCY(), OS::imageListColor() | ILC_MASK, 16, 0);

		staticUpdateStateImages(0);
		IconLib::registerCallback(staticUpdateStateImages, 0);
	}
}

void OptionsCtrlImpl::staticFreeStateImages()
{
	if (m_hStateIcons && --m_nStateIconsRef == 0) {
		IconLib::unregisterCallback(staticUpdateStateImages, 0);
		ImageList_Destroy(m_hStateIcons);

		m_hStateIcons = NULL;
	}
}

void OptionsCtrlImpl::staticUpdateStateImages(LPARAM lParam)
{
	static const IconLib::IconIndex StateIcons[18] = {
		IconLib::iiTreeCheck1,
		IconLib::iiTreeCheck2,
		IconLib::iiTreeCheck3,
		IconLib::iiTreeCheck4,
		IconLib::iiTreeRadio1,
		IconLib::iiTreeRadio2,
		IconLib::iiTreeRadio3,
		IconLib::iiTreeRadio4,
		IconLib::iiTreeEdit1,
		IconLib::iiTreeEdit2,
		IconLib::iiTreeCombo1,
		IconLib::iiTreeCombo2,
		IconLib::iiTreeFolder1,
		IconLib::iiTreeFolder2,
		IconLib::iiTreeButton1,
		IconLib::iiTreeButton2,
		IconLib::iiTreeDateTime1,
		IconLib::iiTreeDateTime2,
	};

	ImageList_RemoveAll(m_hStateIcons);

	array_each_(i, StateIcons)
	{
		ImageList_AddIcon(m_hStateIcons, IconLib::getIcon(StateIcons[i]));
	}
}

bool OptionsCtrlImpl::registerClass()
{
	const WNDCLASSEX wcx = {
		sizeof(wcx),				// cbSize
		0,							// style
		staticWndProc,				// lpfnWndProc
		0,							// cbClsExtra
		sizeof(OptionsCtrlImpl*),	// cbWndExtra
		g_hInst,					// hInstance
		NULL,						// hIcon
		NULL,						// hCursor
		NULL,						// hbrBackground
		NULL,						// lpszMenuName
		m_ClassName,				// lpszClassName
		NULL						// hIconSm
	};

	if (!RegisterClassEx(&wcx))
		return false;

	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_DATE_CLASSES;
	if (!InitCommonControlsEx(&icc))
		return false;

	return true;
}

void OptionsCtrlImpl::unregisterClass()
{
	UnregisterClass(m_ClassName, g_hInst);
}

OptionsCtrlImpl::OptionsCtrlImpl(HWND hWnd, int nOwnId) :
	m_hWnd(hWnd), m_nOwnId(nOwnId), m_hTree(NULL), m_pfnOldTreeProc(NULL), m_bModified(true), m_hDragItem(NULL)
{
}

OptionsCtrlImpl::~OptionsCtrlImpl()
{
}

LRESULT OptionsCtrlImpl::onWMCreate(CREATESTRUCT* pCS)
{
	DWORD dwStyle = 0;

	if (!(pCS->style & OCS_ALLOWDRAGDROP))
		dwStyle |= TVS_DISABLEDRAGDROP;

	m_hTree = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		WC_TREEVIEW,
		_T(""),
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASLINES | TVS_NOHSCROLL | TVS_SHOWSELALWAYS | dwStyle,
		0,
		0,
		pCS->cx,
		pCS->cy,
		m_hWnd,
		reinterpret_cast<HMENU>(ccTree),
		g_hInst,
		NULL);

	if (!m_hTree)
		return -1;

	// subclass tree view
	m_pfnOldTreeProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_hTree, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(staticTreeProc)));

	// create/set state icons
	staticInitStateImages();
	TreeView_SetImageList(m_hTree, m_hStateIcons, TVSIL_NORMAL);
	return 0;
}

void OptionsCtrlImpl::onWMDestroy()
{
	// empty tree just to be sure
	onOCMDeleteAllItems();

	// unset/free state icons
	if (TreeView_GetImageList(m_hTree, TVSIL_NORMAL))
		staticFreeStateImages();

	// undo subclassing of tree view
	SetWindowLongPtr(m_hTree, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_pfnOldTreeProc));
	m_pfnOldTreeProc = NULL;

	// destroy tree view before invalidating 'this'
	DestroyWindow(m_hTree);
	m_hTree = NULL;
}

void OptionsCtrlImpl::onNMClick()
{
	DWORD dwPoint = GetMessagePos();
	POINTS pts = MAKEPOINTS(dwPoint);
	TVHITTESTINFO hti = { { pts.x, pts.y } };

	ScreenToClient(m_hTree, &hti.pt);

	HTREEITEM hSelItem = TreeView_GetSelection(m_hTree);
	HTREEITEM hItem = TreeView_HitTest(m_hTree, &hti);

	if (hItem == hSelItem && hti.flags & TVHT_ONITEM)
		getItem(hItem)->onSelect();
}

LRESULT OptionsCtrlImpl::onNMCustomDraw(NMTVCUSTOMDRAW* pNMCustomDraw)
{
	switch (pNMCustomDraw->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT:
		Item* pItem = reinterpret_cast<Item*>(pNMCustomDraw->nmcd.lItemlParam);
		if (pItem)
			if (!pItem->m_bEnabled)
				pNMCustomDraw->clrText = GetSysColor(COLOR_GRAYTEXT);

		return CDRF_NEWFONT;
	}

	return 0;
}

void OptionsCtrlImpl::onNMKillFocus()
{
	HWND hWndFocused = GetFocus();
	if (hWndFocused != m_hTree && !IsChild(m_hTree, hWndFocused)) {
		HTREEITEM hSelected = TreeView_GetSelection(m_hTree);

		if (hSelected) {
			Item* pItem = getItem(hSelected);
			if (pItem->m_ItemType == itDateTime) {
				// MEMO: handle date/time picker (with possibly open month calendar) separately
				DateTime* pDateTime = reinterpret_cast<DateTime*>(pItem);

				if (!pDateTime->isMonthCalVisible())
					pDateTime->onDeselect();
			}
			else if (pItem->m_ItemType == itColor)
				; // MEMO: handle color picker separately
			else
				pItem->onDeselect();
		}
	}
}

LRESULT OptionsCtrlImpl::onTVNItemExpanding(NMTREEVIEW* pNMTreeView)
{
	return BOOL_(
		pNMTreeView->action == TVE_COLLAPSE || pNMTreeView->action == TVE_COLLAPSERESET ||
		(pNMTreeView->action == TVE_TOGGLE && pNMTreeView->itemNew.state & TVIS_EXPANDED));
}

void OptionsCtrlImpl::onTVNDeleteItem(NMTREEVIEW* pNMTreeView)
{
	if (pNMTreeView->itemOld.hItem) {
		// fake OCN_SELCHANGING message
		NMOPTIONSCTRL nmoc;

		nmoc.hdr.code = OCN_SELCHANGING;
		nmoc.hdr.hwndFrom = m_hWnd;
		nmoc.hdr.idFrom = m_nOwnId;
		nmoc.hItem = 0;
		nmoc.dwData = 0;

		SendMessage(GetParent(m_hWnd), WM_NOTIFY, nmoc.hdr.idFrom, reinterpret_cast<LPARAM>(&nmoc));

		// fake OCN_SELCHANGED message
		nmoc.hdr.code = OCN_SELCHANGED;

		SendMessage(GetParent(m_hWnd), WM_NOTIFY, nmoc.hdr.idFrom, reinterpret_cast<LPARAM>(&nmoc));

		// do actual delete
		Item* pItem = getItem(pNMTreeView->itemOld.hItem);
		setItem(pNMTreeView->itemOld.hItem, NULL);

		pItem->onDeselect();

		if (--pItem->m_nRef == 0)
			delete pItem;
	}
}

void OptionsCtrlImpl::onTVNSelChanging(NMTREEVIEW* pNMTreeView)
{
	NMOPTIONSCTRL nmoc;
	nmoc.hdr.code = OCN_SELCHANGING;
	nmoc.hdr.hwndFrom = m_hWnd;
	nmoc.hdr.idFrom = m_nOwnId;
	nmoc.hItem = reinterpret_cast<HANDLE>(pNMTreeView->itemOld.hItem);
	nmoc.dwData = pNMTreeView->itemOld.hItem ? reinterpret_cast<Item*>(pNMTreeView->itemOld.lParam)->m_dwData : 0;
	SendMessage(GetParent(m_hWnd), WM_NOTIFY, nmoc.hdr.idFrom, reinterpret_cast<LPARAM>(&nmoc));

	if (pNMTreeView->itemOld.hItem)
		reinterpret_cast<Item*>(pNMTreeView->itemOld.lParam)->onDeselect();
}

void OptionsCtrlImpl::onTVNSelChanged(NMTREEVIEW* pNMTreeView)
{
	if (pNMTreeView->itemNew.hItem)
		reinterpret_cast<Item*>(pNMTreeView->itemNew.lParam)->onSelect();

	NMOPTIONSCTRL nmoc;
	nmoc.hdr.code = OCN_SELCHANGED;
	nmoc.hdr.hwndFrom = m_hWnd;
	nmoc.hdr.idFrom = m_nOwnId;
	nmoc.hItem = reinterpret_cast<HANDLE>(pNMTreeView->itemNew.hItem);
	nmoc.dwData = pNMTreeView->itemNew.hItem ? reinterpret_cast<Item*>(pNMTreeView->itemNew.lParam)->m_dwData : 0;
	SendMessage(GetParent(m_hWnd), WM_NOTIFY, nmoc.hdr.idFrom, reinterpret_cast<LPARAM>(&nmoc));
}

void OptionsCtrlImpl::onTVNBeginDrag(NMTREEVIEW* pNMTreeView)
{
	SetCapture(m_hWnd);
	m_hDragItem = pNMTreeView->itemNew.hItem;
	TreeView_SelectItem(m_hTree, m_hDragItem);
}

void OptionsCtrlImpl::onWMMouseMove(const POINT& pt)
{
	if (!m_hDragItem)
		return;

	TVHITTESTINFO hti;
	hti.pt = pt;
	ClientToScreen(m_hWnd, &hti.pt);
	ScreenToClient(m_hTree, &hti.pt);
	TreeView_HitTest(m_hTree, &hti);

	if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
		RECT rItem;
		TreeView_GetItemRect(m_hTree, hti.hItem, &rItem, FALSE);
		TreeView_SetInsertMark(m_hTree, hti.hItem, (hti.pt.y >= (rItem.top + rItem.bottom) / 2) ? TRUE : FALSE);
	}
	else {
		if (hti.flags & TVHT_ABOVE)
			SendMessage(m_hTree, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
		else if (hti.flags & TVHT_BELOW)
			SendMessage(m_hTree, WM_VSCROLL, MAKELPARAM(SB_LINEDOWN, 0), 0);

		TreeView_SetInsertMark(m_hTree, NULL, FALSE);
	}
}

void OptionsCtrlImpl::onWMLButtonUp(const POINT& pt)
{
	if (!m_hDragItem)
		return;

	// revert to noraml state
	HTREEITEM hDragItem = m_hDragItem;

	TreeView_SetInsertMark(m_hTree, NULL, FALSE);
	m_hDragItem = NULL;
	ReleaseCapture();

	// check for drop target and handle
	TVHITTESTINFO hti;
	hti.pt = pt;
	ClientToScreen(m_hWnd, &hti.pt);
	ScreenToClient(m_hTree, &hti.pt);
	TreeView_HitTest(m_hTree, &hti);

	if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
		RECT rItem;
		TreeView_GetItemRect(m_hTree, hti.hItem, &rItem, FALSE);

		bool bAbove = (hti.pt.y >= (rItem.top + rItem.bottom) / 2);

		NMOPTIONSCTRLDROP nmocd;
		nmocd.hdr.code = OCN_ITEMDROPPED;
		nmocd.hdr.hwndFrom = m_hWnd;
		nmocd.hdr.idFrom = m_nOwnId;
		nmocd.hItem = reinterpret_cast<HANDLE>(hDragItem);
		nmocd.dwData = hDragItem ? getItem(hDragItem)->m_dwData : 0;
		nmocd.hDropTarget = reinterpret_cast<HANDLE>(hti.hItem);
		nmocd.dwDropTargetData = hti.hItem ? getItem(hti.hItem)->m_dwData : 0;
		nmocd.bAbove = BOOL_(bAbove);
		SendMessage(GetParent(m_hWnd), WM_NOTIFY, nmocd.hdr.idFrom, reinterpret_cast<LPARAM>(&nmocd));
	}
}

void OptionsCtrlImpl::onENChange()
{
	HTREEITEM hSelected = TreeView_GetSelection(m_hTree);
	if (hSelected)
		setModified(getItem(hSelected));
}

void OptionsCtrlImpl::onCBNSelChange()
{
	HTREEITEM hSelected = TreeView_GetSelection(m_hTree);

	if (hSelected) {
		Item* pItem = getItem(hSelected);

		assert(pItem->m_ItemType == itCombo);

		reinterpret_cast<Combo*>(pItem)->onSelChanged();
		setModified(pItem);
	}
}

void OptionsCtrlImpl::onBNClicked()
{
	HTREEITEM hSelected = TreeView_GetSelection(m_hTree);

	if (hSelected) {
		Item* pItem = getItem(hSelected);

		assert(pItem->m_ItemType == itButton);

		NMOPTIONSCTRL nmoc;
		nmoc.hdr.code = OCN_CLICKED;
		nmoc.hdr.hwndFrom = m_hWnd;
		nmoc.hdr.idFrom = m_nOwnId;
		nmoc.hItem = reinterpret_cast<HANDLE>(pItem->m_hItem);
		nmoc.dwData = pItem->m_dwData;
		SendMessage(GetParent(m_hWnd), WM_NOTIFY, nmoc.hdr.idFrom, reinterpret_cast<LPARAM>(&nmoc));
	}
}

void OptionsCtrlImpl::onDTNDateTimeChange()
{
	HTREEITEM hSelected = TreeView_GetSelection(m_hTree);

	if (hSelected) {
		Item* pItem = getItem(hSelected);

		assert(pItem->m_ItemType == itDateTime);

		reinterpret_cast<DateTime*>(pItem)->onDateTimeChange();
		setModified(pItem);
	}
}

void OptionsCtrlImpl::onCPNColorChanged()
{
	HTREEITEM hSelected = TreeView_GetSelection(m_hTree);

	if (hSelected) {
		Item* pItem = getItem(hSelected);

		assert(pItem->m_ItemType == itColor);

		reinterpret_cast<Color*>(pItem)->onColorChange();
		setModified(pItem);
	}
}

void OptionsCtrlImpl::onTreeWMLButtonDown(UINT nFlags, POINTS point)
{
	TVHITTESTINFO hti = { { point.x, point.y } };

	if (TreeView_HitTest(m_hTree, &hti) && hti.flags & TVHT_ONITEMICON)
		getItem(hti.hItem)->onToggle();
}

void OptionsCtrlImpl::onTreeWMChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_SPACE) {
		HTREEITEM hItem = TreeView_GetSelection(m_hTree);
		if (hItem)
			getItem(hItem)->onToggle();
	}
}

void OptionsCtrlImpl::onTreeWMKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_RIGHT) {
		HTREEITEM hItem = TreeView_GetSelection(m_hTree);
		if (hItem)
			getItem(hItem)->onActivate();
	}
}

void OptionsCtrlImpl::onTreeXScroll()
{
	HTREEITEM hSelected = TreeView_GetSelection(m_hTree);
	if (hSelected && isItemValid(hSelected))
		getItem(hSelected)->onDeselect();
}

HTREEITEM OptionsCtrlImpl::onOCMInsertGroup(HTREEITEM hParent, OCGROUP* pGroup)
{
	assert(pGroup);
	assert(pGroup->szLabel);

	return (new Group(this, hParent ? getItem(hParent) : NULL, pGroup->szLabel, pGroup->dwFlags, pGroup->dwData))->m_hItem;
}

HTREEITEM OptionsCtrlImpl::onOCMInsertCheck(HTREEITEM hParent, OCCHECK* pCheck)
{
	assert(pCheck);
	assert(pCheck->szLabel);

	return (new Check(this, hParent ? getItem(hParent) : NULL, pCheck->szLabel, pCheck->dwFlags, pCheck->dwData))->m_hItem;
}

HTREEITEM OptionsCtrlImpl::onOCMInsertRadio(HTREEITEM hParent, OCRADIO* pRadio)
{
	assert(pRadio);
	assert(pRadio->szLabel);

	Item* pSibling = pRadio->hSibling ? getItem(reinterpret_cast<HTREEITEM>(pRadio->hSibling)) : NULL;

	assert(!pSibling || pSibling->m_ItemType == itRadio);

	return (new Radio(this, hParent ? getItem(hParent) : NULL, reinterpret_cast<Radio*>(pSibling), pRadio->szLabel, pRadio->dwFlags, pRadio->dwData))->m_hItem;
}

HTREEITEM OptionsCtrlImpl::onOCMInsertEdit(HTREEITEM hParent, OCEDIT* pEdit)
{
	assert(pEdit);
	assert(pEdit->szLabel);
	assert(pEdit->szEdit);

	return (new Edit(this, hParent ? getItem(hParent) : NULL, pEdit->szLabel, pEdit->szEdit, pEdit->dwFlags, pEdit->dwData))->m_hItem;
}

HTREEITEM OptionsCtrlImpl::onOCMInsertCombo(HTREEITEM hParent, OCCOMBO* pCombo)
{
	assert(pCombo);
	assert(pCombo->szLabel);

	return (new Combo(this, hParent ? getItem(hParent) : NULL, pCombo->szLabel, pCombo->dwFlags, pCombo->dwData))->m_hItem;
}

HTREEITEM OptionsCtrlImpl::onOCMInsertButton(HTREEITEM hParent, OCBUTTON* pButton)
{
	assert(pButton);
	assert(pButton->szLabel);
	assert(pButton->szButton);

	return (new Button(this, hParent ? getItem(hParent) : NULL, pButton->szLabel, pButton->szButton, pButton->dwFlags, pButton->dwData))->m_hItem;
}

HTREEITEM OptionsCtrlImpl::onOCMInsertDateTime(HTREEITEM hParent, OCDATETIME* pDateTime)
{
	assert(pDateTime);
	assert(pDateTime->szLabel);
	assert(pDateTime->szFormat);

	return (new DateTime(this, hParent ? getItem(hParent) : NULL, pDateTime->szLabel, pDateTime->szFormat, pDateTime->dwDateTime, pDateTime->dwFlags, pDateTime->dwData))->m_hItem;
}

HTREEITEM OptionsCtrlImpl::onOCMInsertColor(HTREEITEM hParent, OCCOLOR* pColor)
{
	assert(pColor);
	assert(pColor->szLabel);

	return (new Color(this, hParent ? getItem(hParent) : NULL, pColor->szLabel, pColor->crColor, pColor->dwFlags, pColor->dwData))->m_hItem;
}

const TCHAR* OptionsCtrlImpl::onOCMGetItemLabel(HTREEITEM hItem)
{
	return getItem(hItem)->getLabel();
}

void OptionsCtrlImpl::onOCMSetItemLabel(HTREEITEM hItem, const TCHAR* szLabel)
{
	assert(szLabel);

	getItem(hItem)->setLabel(szLabel);
}

bool OptionsCtrlImpl::onOCMIsItemEnabled(HTREEITEM hItem)
{
	return getItem(hItem)->m_bEnabled;
}

void OptionsCtrlImpl::onOCMEnableItem(HTREEITEM hItem, bool bEnable)
{
	getItem(hItem)->setEnabled(bEnable);
}

DWORD OptionsCtrlImpl::onOCMGetItemData(HTREEITEM hItem)
{
	return getItem(hItem)->m_dwData;
}

void OptionsCtrlImpl::onOCMSetItemData(HTREEITEM hItem, INT_PTR dwData)
{
	getItem(hItem)->m_dwData = dwData;
}

bool OptionsCtrlImpl::onOCMIsItemChecked(HTREEITEM hItem)
{
	Item* pItem = getItem(hItem);

	assert(pItem->m_ItemType == itCheck || pItem->m_ItemType == itRadio);

	return (pItem->m_ItemType == itCheck) ? reinterpret_cast<Check*>(pItem)->isChecked() : reinterpret_cast<Radio*>(pItem)->isChecked();
}

void OptionsCtrlImpl::onOCMCheckItem(HTREEITEM hItem, bool bCheck)
{
	Item* pItem = getItem(hItem);

	assert(pItem->m_ItemType == itCheck || pItem->m_ItemType == itRadio);

	if (pItem->m_ItemType == itCheck)
		reinterpret_cast<Check*>(pItem)->setChecked(bCheck);
	else
		reinterpret_cast<Radio*>(pItem)->setChecked();
}

int OptionsCtrlImpl::onOCMGetRadioChecked(HTREEITEM hRadio)
{
	Item* pRadio = getItem(hRadio);

	assert(pRadio->m_ItemType == itRadio);

	return reinterpret_cast<Radio*>(pRadio)->m_pSiblings->getChecked();
}

void OptionsCtrlImpl::onOCMSetRadioChecked(HTREEITEM hRadio, int nCheck)
{
	Item* pRadio = getItem(hRadio);

	assert(pRadio->m_ItemType == itRadio);

	reinterpret_cast<Radio*>(pRadio)->m_pSiblings->setChecked(nCheck);
}

int OptionsCtrlImpl::onOCMGetEditNumber(HTREEITEM hEdit)
{
	Item* pEdit = getItem(hEdit);

	assert(pEdit->m_ItemType == itEdit);

	return reinterpret_cast<Edit*>(pEdit)->getNumber();
}

void OptionsCtrlImpl::onOCMSetEditNumber(HTREEITEM hEdit, int nNumber)
{
	Item* pEdit = getItem(hEdit);

	assert(pEdit->m_ItemType == itEdit);

	reinterpret_cast<Edit*>(pEdit)->setNumber(nNumber);
}

const TCHAR* OptionsCtrlImpl::onOCMGetEditString(HTREEITEM hEdit)
{
	Item* pEdit = getItem(hEdit);

	assert(pEdit->m_ItemType == itEdit);

	return reinterpret_cast<Edit*>(pEdit)->getString();
}

void OptionsCtrlImpl::onOCMSetEditString(HTREEITEM hEdit, const TCHAR *szString)
{
	assert(szString);

	Item* pEdit = getItem(hEdit);

	assert(pEdit->m_ItemType == itEdit);

	reinterpret_cast<Edit*>(pEdit)->setString(szString);
}

void OptionsCtrlImpl::onOCMAddComboItem(HTREEITEM hCombo, const TCHAR* szItem)
{
	assert(szItem);

	Item* pCombo = getItem(hCombo);

	assert(pCombo->m_ItemType == itCombo);

	reinterpret_cast<Combo*>(pCombo)->addItem(szItem);
}

int OptionsCtrlImpl::onOCMGetComboSelected(HTREEITEM hCombo)
{
	Item* pCombo = getItem(hCombo);

	assert(pCombo->m_ItemType == itCombo);

	return reinterpret_cast<Combo*>(pCombo)->getSelected();
}

void OptionsCtrlImpl::onOCMSetComboSelected(HTREEITEM hCombo, int nSelect)
{
	Item* pCombo = getItem(hCombo);

	assert(pCombo->m_ItemType == itCombo);

	reinterpret_cast<Combo*>(pCombo)->setSelected(nSelect);
}

void OptionsCtrlImpl::onOCMEnsureVisible(HTREEITEM hItem)
{
	if (!hItem)
		hItem = TreeView_GetChild(m_hTree, TVI_ROOT);

	if (hItem)
		TreeView_EnsureVisible(m_hTree, hItem);
}

void OptionsCtrlImpl::onOCMDeleteAllItems()
{
	TreeView_SelectItem(m_hTree, NULL);
	TreeView_DeleteAllItems(m_hTree);
}

HTREEITEM OptionsCtrlImpl::onOCMGetSelection()
{
	return TreeView_GetSelection(m_hTree);
}

void OptionsCtrlImpl::onOCMSelectItem(HTREEITEM hItem)
{
	TreeView_SelectItem(m_hTree, hItem);
}

HTREEITEM OptionsCtrlImpl::onOCMGetItem(HTREEITEM hItem, DWORD dwFlag)
{
	switch (dwFlag) {
	case OCGI_FIRST:
		return TreeView_GetRoot(m_hTree);

	case OCGI_NEXT:
		return TreeView_GetNextSibling(m_hTree, hItem);

	case OCGI_PREV:
		return TreeView_GetPrevSibling(m_hTree, hItem);

	default:
		return NULL;
	}
}

void OptionsCtrlImpl::onOCMDeleteItem(HTREEITEM hItem)
{
	TreeView_DeleteItem(m_hTree, hItem);
}

void OptionsCtrlImpl::onOCMMoveItem(HTREEITEM& hItem, HTREEITEM hInsertAfter)
{
	assert(hItem);

	TVINSERTSTRUCT tvis;
	tvis.hParent = TreeView_GetParent(m_hTree, hItem);
	tvis.hInsertAfter = hInsertAfter ? hInsertAfter : TVI_FIRST;
	tvis.item.hItem = hItem;
	tvis.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_PARAM;
	tvis.item.stateMask = TVIS_BOLD | TVIS_EXPANDED;

	if (TreeView_GetItem(m_hTree, &tvis.item)) {
		Item* pItem = reinterpret_cast<Item*>(tvis.item.lParam);

		++pItem->m_nRef;

		TreeView_DeleteItem(m_hTree, hItem);

		tvis.item.mask |= TVIF_TEXT;
		tvis.item.pszText = _T("");

		hItem = TreeView_InsertItem(m_hTree, &tvis);

		ext::string strLabel = pItem->getLabel();

		pItem->m_hItem = hItem;
		pItem->setLabel(strLabel.c_str());
	}
}

bool OptionsCtrlImpl::onOCMIsDateTimeNone(HTREEITEM hDateTime)
{
	Item* pDateTime = getItem(hDateTime);

	assert(pDateTime->m_ItemType == itDateTime);

	return reinterpret_cast<DateTime*>(pDateTime)->isNone();
}

void OptionsCtrlImpl::onOCMSetDateTimeNone(HTREEITEM hDateTime)
{
	Item* pDateTime = getItem(hDateTime);

	assert(pDateTime->m_ItemType == itDateTime);

	reinterpret_cast<DateTime*>(pDateTime)->setNone();
}

DWORD OptionsCtrlImpl::onOCMGetDateTime(HTREEITEM hDateTime, BOOL* pbNone)
{
	Item* pDateTime = getItem(hDateTime);

	assert(pDateTime->m_ItemType == itDateTime);
	assert(pbNone);

	*pbNone = reinterpret_cast<DateTime*>(pDateTime)->isNone();

	return reinterpret_cast<DateTime*>(pDateTime)->getTimestamp();
}

void OptionsCtrlImpl::onOCMSetDateTime(HTREEITEM hDateTime, DWORD dwTimestamp)
{
	Item* pDateTime = getItem(hDateTime);

	assert(pDateTime->m_ItemType == itDateTime);

	reinterpret_cast<DateTime*>(pDateTime)->setTimestamp(dwTimestamp);
}

COLORREF OptionsCtrlImpl::onOCMGetItemColor(HTREEITEM hColor)
{
	Item* pColor = getItem(hColor);

	assert(pColor->m_ItemType == itColor);

	return reinterpret_cast<Color*>(pColor)->getColor();
}

void OptionsCtrlImpl::onOCMSetItemColor(HTREEITEM hColor, COLORREF crColor)
{
	Item* pColor = getItem(hColor);

	assert(pColor->m_ItemType == itColor);

	reinterpret_cast<Color*>(pColor)->setColor(crColor);
}

void OptionsCtrlImpl::insertItem(Item* pParent, Item* pItem, const TCHAR* szNodeText, DWORD dwFlags, int iImage)
{
	assert(!pParent || pParent->m_hItem);
	assert(pItem);
	assert(szNodeText);

	TVINSERTSTRUCT tvis;
	tvis.hParent = pParent ? pParent->m_hItem : TVI_ROOT;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_PARAM;
	tvis.item.pszText = const_cast<TCHAR*>(szNodeText);
	tvis.item.iImage = iImage;
	tvis.item.iSelectedImage = iImage;
	tvis.item.stateMask = TVIS_EXPANDED | ((dwFlags & OCF_BOLD) ? TVIS_BOLD : 0);
	tvis.item.state = tvis.item.stateMask;
	tvis.item.lParam = reinterpret_cast<LPARAM>(pItem);
	pItem->m_hItem = TreeView_InsertItem(m_hTree, &tvis);
}

void OptionsCtrlImpl::setModified(Item* pItem, bool bModified /* = true */)
{
	m_bModified = bModified;

	if (bModified) {
		NMOPTIONSCTRL nmoc;
		nmoc.hdr.code = OCN_MODIFIED;
		nmoc.hdr.hwndFrom = m_hWnd;
		nmoc.hdr.idFrom = m_nOwnId;
		nmoc.hItem = reinterpret_cast<HANDLE>(pItem->m_hItem);
		nmoc.dwData = pItem->m_dwData;
		SendMessage(GetParent(m_hWnd), WM_NOTIFY, nmoc.hdr.idFrom, reinterpret_cast<LPARAM>(&nmoc));
	}
}

bool OptionsCtrlImpl::isItemValid(HTREEITEM hItem)
{
	assert(hItem);

	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = hItem;
	Item* pItem = TreeView_GetItem(m_hTree, &tvi) ? reinterpret_cast<Item*>(tvi.lParam) : NULL;

	return bool_(pItem);
}

OptionsCtrlImpl::Item* OptionsCtrlImpl::getItem(HTREEITEM hItem)
{
	assert(hItem);

	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = hItem;
	Item* pItem = TreeView_GetItem(m_hTree, &tvi) ? reinterpret_cast<Item*>(tvi.lParam) : NULL;

	assert(pItem);

	return pItem;
}

void OptionsCtrlImpl::setItem(HTREEITEM hItem, Item* pItem)
{
	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = hItem;
	tvi.lParam = reinterpret_cast<LPARAM>(pItem);
	TreeView_SetItem(m_hTree, &tvi);
}

void OptionsCtrlImpl::setStateImage(HTREEITEM hItem, int iImage)
{
	assert(hItem);

	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvi.hItem = hItem;
	tvi.iImage = iImage;
	tvi.iSelectedImage = iImage;
	TreeView_SetItem(m_hTree, &tvi);
}

void OptionsCtrlImpl::setNodeText(HTREEITEM hItem, const TCHAR* szNodeText)
{
	assert(hItem);
	assert(szNodeText);

	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_TEXT;
	tvi.hItem = hItem;
	tvi.pszText = const_cast<TCHAR*>(szNodeText);
	TreeView_SetItem(m_hTree, &tvi);
}

bool OptionsCtrlImpl::getItemFreeRect(HTREEITEM hItem, RECT& outRect)
{
	RECT rLine, rText;

	if (TreeView_GetItemRect(m_hTree, hItem, &rLine, FALSE) && TreeView_GetItemRect(m_hTree, hItem, &rText, TRUE)) {
		outRect.left = rText.right + 2;
		outRect.top = rText.top;
		outRect.right = rLine.right - 2;
		outRect.bottom = rText.bottom;
		return true;
	}
	else return false;
}
