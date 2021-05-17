#include "stdafx.h"

static HIMAGELIST hImg = nullptr;

int g_Hex;
int g_Mode;
int g_Order;
int g_Inline;

CMainDlg *g_pMainWindow = nullptr;

extern volatile BOOL populating, skipEnter;
extern volatile int Select;

extern SettingListInfo info;

void EditFinish(int selected);
void EditLabel(int item, int subitem);

static LRESULT CALLBACK ModuleTreeSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_RBUTTONDOWN:
		{
			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(GetMessagePos());
			hti.pt.y = (short)HIWORD(GetMessagePos());
			ScreenToClient(hwnd, &hti.pt);

			if (TreeView_HitTest(hwnd, &hti)) {
				if (hti.flags&TVHT_ONITEM)
					TreeView_SelectItem(hwnd, hti.hItem);
			}
		}
		break;

	case WM_CHAR:
		if (GetKeyState(VK_CONTROL) & 0x8000 && wParam == 6)
			newFindWindow();
		break;

	case WM_KEYUP:
		if (wParam == VK_DELETE || wParam == VK_F2 || wParam == VK_F5 || wParam == VK_F3) {
			TVITEM tvi;
			wchar_t text[FLD_SIZE];
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
			tvi.hItem = TreeView_GetSelection(hwnd);
			tvi.pszText = text;
			tvi.cchTextMax = _countof(text);
			if (TreeView_GetItem(hwnd, &tvi) && tvi.lParam) {
				ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;
				MCONTACT hContact = mtis->hContact;
				if (wParam == VK_DELETE) {
					if ((mtis->type) & MODULE) {
						if (deleteModule(g_pMainWindow->GetHwnd(), hContact, _T2A(text), 1)) {
							mir_free(mtis);
							TreeView_DeleteItem(hwnd, tvi.hItem);
						}
					}
					else if ((mtis->type == CONTACT) && hContact) {
						if (db_get_b(0, "CList", "ConfirmDelete", 1)) {
							wchar_t str[MSG_SIZE];
							mir_snwprintf(str, TranslateT("Are you sure you want to delete contact \"%s\"?"), text);
							if (g_pMainWindow->dlg(str, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
								break;
						}
						db_delete_contact(hContact);
						g_pMainWindow->freeTree(mtis->hContact);
						TreeView_DeleteItem(hwnd, tvi.hItem);
					}
				}
				else if (wParam == VK_F2 && mtis->type == MODULE)
					TreeView_EditLabel(hwnd, tvi.hItem);
				else if (wParam == VK_F5) {
					refreshTree(1);
					break;
				}
				else if (wParam == VK_F3) {
					newFindWindow();
					break;
				}
			}
		}
		break;
	}
	return mir_callNextSubclass(hwnd, ModuleTreeSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK SettingListSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if (GetKeyState(VK_CONTROL) & 0x8000 && wParam == 6)
			newFindWindow();
		break;

	case WM_KEYDOWN:
		if (wParam == VK_F5)
			g_pMainWindow->PopulateSettings(info.hContact, info.module);
		else if (wParam == VK_F3)
			newFindWindow();
		else if (wParam == VK_DELETE || (wParam == VK_F2 && ListView_GetSelectedCount(hwnd) == 1)) {
			char setting[FLD_SIZE];
			int idx = ListView_GetSelectionMark(hwnd);
			if (idx == -1) return 0;
			ListView_GetItemTextA(hwnd, idx, 0, setting, _countof(setting));

			if (wParam == VK_F2)
				g_pMainWindow->editSetting(info.hContact, info.module, setting);
			else if (wParam == VK_DELETE)
				g_pMainWindow->DeleteSettingsFromList(info.hContact, info.module, setting);

			return 0;
		}
		break;
	}
	return mir_callNextSubclass(hwnd, SettingListSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK ModuleTreeLabelEditSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYUP:
		switch (wParam) {
		case VK_RETURN:
			if (skipEnter)
				skipEnter = 0;
			else
				TreeView_EndEditLabelNow(GetParent(hwnd), 0);
			return 0;
		case VK_ESCAPE:
			TreeView_EndEditLabelNow(GetParent(hwnd), 1);
			return 0;
		}
		break;
	}
	return mir_callNextSubclass(hwnd, ModuleTreeLabelEditSubClassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Main window dialog

CMainDlg::CMainDlg() :
	CDlgBase(g_plugin, IDD_MAIN),
	m_modules(this, IDC_MODULES),
	m_settings(this, IDC_SETTINGS),
	m_splitter(this, IDC_SPLITTER),
	splitterPos(g_plugin.getWord("Splitter", 200))
{
	SetMinSize(450, 300);

	m_autoClose = 0;

	m_splitter.OnChange = Callback(this, &CMainDlg::onChange_Splitter);

	m_modules.OnItemExpanding = Callback(this, &CMainDlg::onItemExpand_Modules);
	m_modules.OnSelChanged = Callback(this, &CMainDlg::onSelChanged_Modules);
	m_modules.OnBeginLabelEdit = Callback(this, &CMainDlg::onBeginLabelEdit_Modules);
	m_modules.OnEndLabelEdit = Callback(this, &CMainDlg::onEndLabelEdit_Modules);
	m_modules.OnBuildMenu = Callback(this, &CMainDlg::onContextMenu_Modules);

	m_settings.OnClick = Callback(this, &CMainDlg::onClick_Settings);
	m_settings.OnDoubleClick = Callback(this, &CMainDlg::onDblClick_Settings);
	m_settings.OnColumnClick = Callback(this, &CMainDlg::onColumnClick_Settings);
	m_settings.OnBuildMenu = Callback(this, &CMainDlg::onContextMenu_Settings);
}

static ColumnsSettings csSettingList[] =
{
	{ LPGENW("Name"), 0, "Column0width", 180 },
	{ LPGENW("Value"), 1, "Column1width", 250 },
	{ LPGENW("Type"), 2, "Column2width", 60 },
	{ LPGENW("Size"), 3, "Column3width", 80 },
	{ LPGENW("#"), 4, "Column4width", 30 },
	{ nullptr }
};

bool CMainDlg::OnInitDialog() 
{
	g_pMainWindow = this;

	LoadResidentSettings();

	// image list
	hImg = LoadIcons();

	// do the icon
	SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(ICO_REGEDIT)));
	SetWindowText(m_hwnd, TranslateT("Database Editor++"));

	// module tree
	mir_subclassWindow(m_modules.GetHwnd(), ModuleTreeSubclassProc);
	m_modules.SetImageList(hImg, TVSIL_NORMAL);

	//setting list
	mir_subclassWindow(m_settings.GetHwnd(), SettingListSubclassProc);
	m_settings.SetExtendedListViewStyle(32 | LVS_EX_SUBITEMIMAGES | LVS_EX_LABELTIP); //LVS_EX_GRIDLINES
	loadListSettings(m_settings.GetHwnd(), csSettingList);
	m_settings.SetImageList(hImg, LVSIL_SMALL);

	HMENU hMenu = GetMenu(m_hwnd);
	TranslateMenu(hMenu);
	for (int i = 0; i < 6; i++)
		TranslateMenu(GetSubMenu(hMenu, i));

	Utils_RestoreWindowPosition(m_hwnd, NULL, MODULENAME, "Main_");
	if (g_plugin.getByte("Maximized", 0))
		ShowWindow(m_hwnd, SW_SHOWMAXIMIZED);

	g_Inline = !g_plugin.getByte("DontAllowInLineEdit", 1);
	CheckMenuItem(GetSubMenu(hMenu, 5), MENU_INLINE_EDIT, MF_BYCOMMAND | (g_Inline ? MF_CHECKED : MF_UNCHECKED));

	g_Mode = MODE_ALL;
	CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_ALL, MF_BYCOMMAND | MF_CHECKED);

	g_Hex = g_plugin.getByte("HexMode", 0);
	CheckMenuItem(GetSubMenu(hMenu, 5), MENU_BYTE_HEX, MF_BYCOMMAND | ((g_Hex & HEX_BYTE) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(GetSubMenu(hMenu, 5), MENU_WORD_HEX, MF_BYCOMMAND | ((g_Hex & HEX_WORD) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(GetSubMenu(hMenu, 5), MENU_DWORD_HEX, MF_BYCOMMAND | ((g_Hex & HEX_DWORD) ? MF_CHECKED : MF_UNCHECKED));

	CheckMenuItem(GetSubMenu(GetMenu(m_hwnd), 5), MENU_SAVE_POSITION, MF_BYCOMMAND | (g_plugin.bRestoreOnOpen ? MF_CHECKED : MF_UNCHECKED));

	g_Order = g_plugin.getByte("SortMode", 1);
	CheckMenuItem(GetSubMenu(GetMenu(m_hwnd), 5), MENU_SORT_ORDER, MF_BYCOMMAND | (g_Order ? MF_CHECKED : MF_UNCHECKED));

	int restore;
	if (hRestore)
		restore = 3;
	else if (g_plugin.bRestoreOnOpen)
		restore = 2;
	else
		restore = 0;

	refreshTree(restore);
	return true;
}

void CMainDlg::OnDestroy()
{
	wchar_t text[256];

	if (g_plugin.bRestoreOnOpen) {
		HTREEITEM item;
		if (item = m_modules.GetSelection()) {
			int type = MODULE;
			TVITEMEX tvi = {};
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
			tvi.pszText = text;
			tvi.cchTextMax = _countof(text);
			tvi.hItem = item;
			if (m_modules.GetItem(&tvi)) {
				MCONTACT hContact = 0;
				if (tvi.lParam) {
					ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;
					hContact = mtis->hContact;
					type = mtis->type;
				}

				g_plugin.setDword("LastContact", hContact);

				if (type == CONTACT)
					g_plugin.setString("LastModule", "");
				else
					g_plugin.setString("LastModule", _T2A(text));
			}
			else {
				g_plugin.delSetting("LastContact");
				g_plugin.delSetting("LastModule");
			}

			int pos = m_settings.GetSelectionMark();
			if (pos != -1) {
				char data[FLD_SIZE];
				ListView_GetItemTextA(m_settings.GetHwnd(), pos, 0, data, _countof(data));
				g_plugin.setString("LastSetting", data);
			}
			else g_plugin.delSetting("LastSetting");
		}
	}

	g_plugin.setByte("HexMode", (byte)g_Hex);
	g_plugin.setByte("SortMode", (byte)g_Order);
	g_plugin.setByte("DontAllowInLineEdit", (byte)!g_Inline);

	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(m_hwnd, &wp);
	if (wp.flags == WPF_RESTORETOMAXIMIZED) {
		g_plugin.setByte("Maximized", 1);
		ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
	}
	else g_plugin.setByte("Maximized", 0);

	Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "Main_");
	ShowWindow(m_hwnd, SW_HIDE);

	saveListSettings(m_settings.GetHwnd(), csSettingList);
	ClearListView();

	freeTree(0);

	g_pMainWindow = nullptr;

	if (hImg) {
		ImageList_Destroy(hImg);
		hImg = nullptr;
	}

	FreeResidentSettings();

	if (g_bServiceMode)
		PostQuitMessage(0);
}

int CMainDlg::Resizer(UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_MODULES:
		urc->rcItem.right = splitterPos - 3;
		urc->rcItem.top = 0;
		urc->rcItem.left = 0;
		urc->rcItem.bottom = urc->dlgNewSize.cy;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_SPLITTER:
		urc->rcItem.top = 0;
		urc->rcItem.bottom = urc->dlgNewSize.cy;
		urc->rcItem.right = splitterPos;
		urc->rcItem.left = splitterPos - 3;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_SETTINGS:
		urc->rcItem.top = 0;
		urc->rcItem.bottom = urc->dlgNewSize.cy;
		urc->rcItem.left = splitterPos;
		urc->rcItem.right = urc->dlgNewSize.cx;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_VARS:
		urc->rcItem.top = 0;
		urc->rcItem.bottom = urc->dlgNewSize.cy;
		urc->rcItem.left = 0;
		urc->rcItem.right = urc->dlgNewSize.cx;
		return RD_ANCHORY_CUSTOM | RD_ANCHORX_CUSTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

void CMainDlg::onChange_Splitter(CSplitter *)
{
	RECT rc2;
	GetWindowRect(m_hwnd, &rc2);

	RECT rc;
	GetClientRect(m_hwnd, &rc);
	POINT pt = { m_splitter.GetPos(), 0 };
	ScreenToClient(m_hwnd, &pt);

	splitterPos = rc.left + pt.x + 1;
	if (splitterPos < 150)
		splitterPos = 150;
	if (splitterPos > rc2.right - rc2.left - 150)
		splitterPos = rc2.right - rc2.left - 150;
	SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_SPLITTER), GWLP_USERDATA, splitterPos);
	g_plugin.setWord("Splitter", (WORD)splitterPos);

	PostMessage(m_hwnd, WM_SIZE, 0, 0);
}

void CMainDlg::FindItem(int type, MCONTACT hContact, const char *szModule, const char *szSetting)
{
	if (HTREEITEM hItem = findItemInTree(hContact, szModule)) {
		m_modules.SelectItem(hItem);
		m_modules.Expand(hItem, TVE_EXPAND);
		if (type != FW_MODULE)
			SelectSetting(szSetting);
	}
}

LRESULT CMainDlg::OnCommand(UINT, WPARAM wParam, LPARAM)
{
	if (GetKeyState(VK_ESCAPE) & 0x8000)
		return TRUE; // this needs to be changed to c if htere is a label edit happening..

	wchar_t text[256];

	switch (LOWORD(wParam)) {
	case MENU_REFRESH_MODS:
		refreshTree(1);
		break;

	case MENU_REFRESH_SETS:
		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
		tvi.hItem = m_modules.GetSelection();
		tvi.pszText = text; // modulename
		tvi.cchTextMax = _countof(text);
		if (m_modules.GetItem(&tvi)) {
			ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;
			if (mtis && (mtis->type == MODULE))
				PopulateSettings(mtis->hContact, _T2A(text));
			else
				ClearListView();
		}
		break;

		// watches
	case MENU_VIEW_WATCHES:
		openWatchedVarWindow();
		break;
	case MENU_REMALL_WATCHES:
		freeAllWatches();
		break;
	case MENU_EXPORTDB: // all db
		exportDB(INVALID_CONTACT_ID, nullptr);
		break;
	case MENU_EXPORTCONTACT: // all contacts
		exportDB(INVALID_CONTACT_ID, "");
		break;
	case MENU_EXPORTMODULE: // all settings
		exportDB(NULL, nullptr);
		break;
	case MENU_IMPORTFROMFILE:
		ImportSettingsFromFileMenuItem(NULL, nullptr);
		break;
	case MENU_IMPORTFROMTEXT:
		ImportSettingsMenuItem(NULL);
		break;
	case MENU_EXIT:
	case IDCANCEL:
		DestroyWindow(m_hwnd);
		break;
	case MENU_DELETE:
		deleteModuleDlg();
		break;
	case MENU_FINDANDREPLACE:
		newFindWindow();
		break;
	case MENU_FIX_RESIDENT:
		if (g_pMainWindow->dlg(TranslateT("Delete resident settings from database?"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
			int cnt = fixResidentSettings();
			mir_snwprintf(text, TranslateT("%d items deleted."), cnt);
			g_pMainWindow->msg(text);
		}
		break;
	case MENU_FILTER_ALL:
		if (g_Mode != MODE_ALL) {
			HMENU hMenu = GetMenu(m_hwnd);
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_ALL, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_LOADED, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_UNLOADED, MF_BYCOMMAND | MF_UNCHECKED);
			g_Mode = MODE_ALL;
			refreshTree(1);
		}
		break;
	case MENU_FILTER_LOADED:
		if (g_Mode != MODE_LOADED) {
			HMENU hMenu = GetMenu(m_hwnd);
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_ALL, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_LOADED, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_UNLOADED, MF_BYCOMMAND | MF_UNCHECKED);
			g_Mode = MODE_LOADED;
			refreshTree(1);
		}
		break;
	case MENU_FILTER_UNLOADED:
		if (g_Mode != MODE_UNLOADED) {
			HMENU hMenu = GetMenu(m_hwnd);
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_ALL, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_LOADED, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetSubMenu(hMenu, 5), MENU_FILTER_UNLOADED, MF_BYCOMMAND | MF_CHECKED);
			g_Mode = MODE_UNLOADED;
			refreshTree(1);
		}
		break;
	case MENU_BYTE_HEX:
		g_Hex ^= HEX_BYTE;
		CheckMenuItem(GetSubMenu(GetMenu(m_hwnd), 5), MENU_BYTE_HEX, MF_BYCOMMAND | ((g_Hex & HEX_BYTE) ? MF_CHECKED : MF_UNCHECKED));
		break;
	case MENU_WORD_HEX:
		g_Hex ^= HEX_WORD;
		CheckMenuItem(GetSubMenu(GetMenu(m_hwnd), 5), MENU_WORD_HEX, MF_BYCOMMAND | ((g_Hex & HEX_WORD) ? MF_CHECKED : MF_UNCHECKED));
		break;
	case MENU_DWORD_HEX:
		g_Hex ^= HEX_DWORD;
		CheckMenuItem(GetSubMenu(GetMenu(m_hwnd), 5), MENU_DWORD_HEX, MF_BYCOMMAND | ((g_Hex & HEX_DWORD) ? MF_CHECKED : MF_UNCHECKED));
		break;
	case MENU_SAVE_POSITION:
		{
			bool bSave = !g_plugin.bRestoreOnOpen;
			CheckMenuItem(GetSubMenu(GetMenu(m_hwnd), 5), MENU_SAVE_POSITION, MF_BYCOMMAND | (bSave ? MF_CHECKED : MF_UNCHECKED));
			g_plugin.bRestoreOnOpen = bSave;
		}
		break;
	case MENU_INLINE_EDIT:
		g_Inline = !g_Inline;
		CheckMenuItem(GetSubMenu(GetMenu(m_hwnd), 5), MENU_INLINE_EDIT, MF_BYCOMMAND | (g_Inline ? MF_CHECKED : MF_UNCHECKED));
		break;
	case MENU_SORT_ORDER:
		g_Order = !g_Order;
		CheckMenuItem(GetSubMenu(GetMenu(m_hwnd), 5), MENU_SORT_ORDER, MF_BYCOMMAND | (g_Order ? MF_CHECKED : MF_UNCHECKED));
		refreshTree(1);
		break;
	case MENU_OPEN_OPTIONS:
		g_plugin.openOptions(L"Database", _A2W(modFullname));
		break;
	}
	return TRUE;
}

void CMainDlg::onItemExpand_Modules(CCtrlTreeView::TEventInfo *ev)
{
	if (populating && ev->nmtv->action == TVE_EXPAND) {
		ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)ev->nmtv->itemNew.lParam;
		if (mtis && (mtis->type == (CONTACT | EMPTY))) {
			MCONTACT hContact = mtis->hContact;
			mtis->type = CONTACT;

			ModuleSettingLL modlist;
			if (!EnumModules(&modlist))
				return;

			ModSetLinkLinkItem *module = modlist.first;
			while (module && g_pMainWindow) {
				if (module->name[0] && !IsModuleEmpty(hContact, module->name))
					insertItem(hContact, module->name, ev->nmtv->itemNew.hItem);

				module = (ModSetLinkLinkItem *)module->next;
			}

			FreeModuleSettingLL(&modlist);
		}
	}
}

void CMainDlg::onSelChanged_Modules(CCtrlTreeView::TEventInfo *ev)
{
	wchar_t text[FLD_SIZE];
		
	TVITEMEX tvi = {};
	tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
	tvi.hItem = ev->nmtv->itemNew.hItem;
	tvi.pszText = text;
	tvi.cchTextMax = _countof(text);
	m_modules.GetItem(&tvi);

	ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;
	if (mtis) {
		MCONTACT hContact = mtis->hContact;
		if (mtis->type == STUB)
			return;

		if (populating)
			Select = 0;

		if (mtis->type == MODULE) {
			PopulateSettings(hContact, _T2A(text));
		}
		else if (((mtis->type & CONTACT) == CONTACT && hContact) || (mtis->type == CONTACT_ROOT_ITEM && !hContact)) {
			int multi = 0;

			ClearListView();

			if (mtis->type == CONTACT_ROOT_ITEM && !hContact) {
				multi = 1;
				hContact = db_find_first();
			}

			while (hContact && g_pMainWindow) {
				if (multi && ApplyProtoFilter(hContact)) {
					hContact = db_find_next(hContact);
					continue;
				}

				addListHandle(hContact);

				if (!multi)
					break;

				hContact = db_find_next(hContact);
			}
		}
		else ClearListView();
	}
	else ClearListView();
}

void CMainDlg::onBeginLabelEdit_Modules(CCtrlTreeView::TEventInfo *ev)
{
	ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)ev->nmtvdi->item.lParam;
	if (!mtis->type || mtis->type == CONTACT) {
		SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
		return;
	}

	mir_subclassWindow(m_modules.GetEditControl(), ModuleTreeLabelEditSubClassProc);
	SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, FALSE);
}

void CMainDlg::onEndLabelEdit_Modules(CCtrlTreeView::TEventInfo *ev)
{
	TVITEMEX tvi = {};
	wchar_t text[FLD_SIZE];
	ModuleTreeInfoStruct *mtis;
	tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
	tvi.hItem = ev->nmtvdi->item.hItem;
	tvi.pszText = text;
	tvi.cchTextMax = _countof(text);
	m_modules.GetItem(&tvi);
	mtis = (ModuleTreeInfoStruct *)ev->nmtvdi->item.lParam;

	_T2A newtext(ev->nmtvdi->item.pszText);
	_T2A oldtext(tvi.pszText);

	if (!newtext // edit control failed
		|| !mtis->type // its a root item
		|| mtis->type == CONTACT // its a contact
		|| newtext[0] == 0)  // empty string
		SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, FALSE);
	else {
		if (mir_strcmp(oldtext, newtext)) {
			renameModule(mtis->hContact, oldtext, newtext);
			findAndRemoveDuplicates(mtis->hContact, newtext);
			if (m_modules.GetItem(&tvi))
				PopulateSettings(mtis->hContact, newtext);
		}
		SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
	}
}

void CMainDlg::onClick_Settings(CCtrlListView::TEventInfo *ev)
{
	LVHITTESTINFO hti;
	hti.pt = ev->nmlv->ptAction;
	if (m_settings.SubItemHitTest(&hti) > -1) {
		if (g_Inline && hti.iSubItem <= 1 && hti.flags != LVHT_ONITEMICON && info.selectedItem == hti.iItem)
			EditLabel(hti.iItem, hti.iSubItem);
		else
			EditFinish(hti.iItem);
	}
	else EditFinish(0);
}

void CMainDlg::onDblClick_Settings(CCtrlListView::TEventInfo *ev)
{
	LVHITTESTINFO hti;
	hti.pt = ev->nmlv->ptAction;
	if (m_settings.SubItemHitTest(&hti) == -1)
		return;

	if (!info.module[0]) { // contact
		LVITEM lvi = {};
		lvi.mask = LVIF_PARAM;
		lvi.iItem = hti.iItem;
		if (m_settings.GetItem(&lvi))
			FindItem(FW_MODULE, (MCONTACT)lvi.lParam, "", "");
		return;
	}

	if (!g_Inline || hti.iSubItem > 1 || hti.flags == LVHT_ONITEMICON) {
		char setting[FLD_SIZE];
		EditFinish(hti.iItem);
		if (ListView_GetItemTextA(m_settings.GetHwnd(), hti.iItem, 0, setting, _countof(setting)))
			editSetting(info.hContact, info.module, setting);
	}
	else EditLabel(hti.iItem, hti.iSubItem);
}

void CMainDlg::onColumnClick_Settings(CCtrlListView::TEventInfo *ev)
{
	ColumnsSortParams params = {};
	params.hList = m_settings.GetHwnd();
	params.column = ev->nmlv->iSubItem;
	params.last = lastColumn;
	m_settings.SortItemsEx(ColumnsCompare, (LPARAM)&params);
	lastColumn = (params.column == lastColumn) ? -1 : params.column;
}
