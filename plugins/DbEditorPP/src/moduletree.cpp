#include "stdafx.h"

volatile BOOL populating = 0, skipEnter = 0;
volatile int Select = 0;

static ModuleTreeInfoStruct contacts_mtis = { CONTACT_ROOT_ITEM, 0 };
static ModuleTreeInfoStruct settings_mtis = { CONTACT, 0 };

void CMainDlg::insertItem(MCONTACT hContact, const char *module, HTREEITEM hParent)
{
	_A2T text(module);

	TVINSERTSTRUCT tvi;
	tvi.hParent = hParent;
	tvi.hInsertAfter = TVI_SORT;
	tvi.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvi.item.pszText = text;

	ModuleTreeInfoStruct *lParam = (ModuleTreeInfoStruct *)mir_calloc(sizeof(ModuleTreeInfoStruct));
	lParam->hContact = hContact;

	tvi.item.iImage = IMAGE_CLOSED;
	tvi.item.iSelectedImage = IMAGE_OPENED;
	lParam->type = MODULE;

	tvi.item.lParam = (LPARAM)lParam;
	m_modules.InsertItem(&tvi);
}

int CMainDlg::doContacts(HTREEITEM contactsRoot, ModuleSettingLL *modlist, MCONTACT hSelectedContact, const char *selectedModule, const char *selectedSetting)
{
	TVINSERTSTRUCT tvi;
	HTREEITEM contact;
	ModuleTreeInfoStruct *lParam;
	int itemscount = 0;
	int icon = 0;
	HTREEITEM hItem = nullptr;

	SetCaption(TranslateT("Loading contacts..."));

	tvi.hInsertAfter = TVI_SORT;
	tvi.item.cChildren = 1;

	char szProto[FLD_SIZE];
	wchar_t name[NAME_SIZE];

	for (auto &hContact : Contacts()) {
		if (ApplyProtoFilter(hContact))
			continue;

		// add the contact
		lParam = (ModuleTreeInfoStruct *)mir_calloc(sizeof(ModuleTreeInfoStruct));
		lParam->hContact = hContact;
		lParam->type = CONTACT;
		tvi.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.lParam = (LPARAM)lParam;
		tvi.hParent = contactsRoot;

		if (hSelectedContact != hContact)
			lParam->type |= EMPTY;

		if (db_get_static(hContact, "Protocol", "p", szProto, _countof(szProto)))
			szProto[0] = 0;

		icon = GetProtoIconIndex(szProto);

		GetContactName(hContact, szProto, name, _countof(name));

		tvi.item.pszText = name;
		tvi.item.iImage = icon;
		tvi.item.iSelectedImage = icon;
		contact = m_modules.InsertItem(&tvi);

		itemscount++;

		if (hSelectedContact == hContact) {
			for (ModSetLinkLinkItem *module = modlist->first; module && g_pMainWindow; module = module->next) {
				if (!module->name[0] || IsModuleEmpty(hContact, module->name))
					continue;
				insertItem(hContact, module->name, contact);
			}

			hItem = findItemInTree(hSelectedContact, selectedModule);
		}
	}

	if (hItem) {
		m_modules.SelectItem(hItem);
		m_modules.Expand(hItem, TVE_EXPAND);
		if (selectedSetting && selectedSetting[0])
			SelectSetting(selectedSetting);
	}

	return itemscount;
}

void CMainDlg::doItems(ModuleSettingLL *modlist, int count)
{
	wchar_t title[96];
	mir_snwprintf(title, TranslateT("Loading modules..."));

	TVITEMEX item = {};
	item.mask = TVIF_STATE | TVIF_PARAM;

	HTREEITEM contact = m_modules.GetChild(TVI_ROOT);
	contact = m_modules.GetNextSibling(contact);
	contact = m_modules.GetChild(contact);

	MCONTACT hContact = 0;
	for (int i = 1; contact && g_pMainWindow; i++) {
		item.hItem = contact;
		contact = m_modules.GetNextSibling(contact);

		if (m_modules.GetItem(&item) && item.lParam) {
			ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)item.lParam;
			hContact = mtis->hContact;
			if (hContact == NULL || mtis->type != (CONTACT | EMPTY))
				continue;
			mtis->type = CONTACT;
		}
		else
			continue;

		// Caption
		wchar_t percent[128];
		mir_snwprintf(percent, L"%s %d%%", title, (int)(100 * i / count));
		SetCaption(percent);

		for (ModSetLinkLinkItem *module = modlist->first; module && g_pMainWindow; module = module->next) {
			if (!module->name[0] || IsModuleEmpty(hContact, module->name))
				continue;

			insertItem(hContact, module->name, item.hItem);
		}
	}

	SetCaption(TranslateT("Database Editor++"));
}

/////////////////////////////////////////////////////////////////////////////////////////
// the following code to go through the whole tree is nicked from codeguru..
// http://www.codeguru.com/Cpp/controls/treeview/treetraversal/comments.php/c683/?thread=7680

HTREEITEM CMainDlg::findItemInTree(MCONTACT hContact, const char *module)
{
	if (!m_modules.GetCount())
		return nullptr;

	_A2T szModule(module);

	wchar_t text[FLD_SIZE];
	TVITEMEX item;
	item.mask = TVIF_STATE | TVIF_PARAM | TVIF_TEXT;
	item.hItem = TVI_ROOT;
	item.pszText = text;
	item.cchTextMax = _countof(text);

	HTREEITEM lastItem;
	do {
		do {
			lastItem = item.hItem;
			if (lastItem != TVI_ROOT) {
				/* these next 2 lines are not from code guru..... */
				if (m_modules.GetItem(&item) && item.lParam) {
					if ((hContact == ((ModuleTreeInfoStruct *)item.lParam)->hContact) && (!module || !module[0] || !mir_wstrcmp(szModule, text))) {
						return item.hItem;

					}
				}
				/* back to coduguru's code*/
			}
		} while ((item.hItem = m_modules.GetChild(lastItem)));

		while ((!(item.hItem = m_modules.GetNextSibling(lastItem))) && (lastItem = item.hItem = m_modules.GetParent(lastItem)));

	} while (item.hItem);

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// the following code to go through the whole tree is nicked from codeguru..
// http://www.codeguru.com/Cpp/controls/treeview/treetraversal/comments.php/c683/?thread=7680

void CMainDlg::freeTree(MCONTACT hContact)
{
	if (!m_modules.GetCount())
		return;

	TVITEMEX item;
	item.mask = TVIF_STATE | TVIF_PARAM;
	item.hItem = TVI_ROOT;

	HTREEITEM lastItem;
	do {
		do {
			lastItem = item.hItem;
			if (lastItem != TVI_ROOT) {
				m_modules.GetItem(&item);
				/* these next 2 lines are not from code guru..... */
				if (item.lParam) {
					ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)item.lParam;

					if (!hContact || (hContact == mtis->hContact)) {
						if (hContact != NULL) {
							m_modules.DeleteItem(item.hItem);
							mir_free(mtis);
						}
						else
							mtis->type = STUB;
					}
				}
				/* back to coduguru's code*/
			}
		}
			while (item.hItem = m_modules.GetChild(lastItem));

		while (!(item.hItem = m_modules.GetNextSibling(lastItem)) && (lastItem = item.hItem = m_modules.GetParent(lastItem)))
			;
	}
		while (item.hItem);
}

/////////////////////////////////////////////////////////////////////////////////////////
// the following code to go through the whole tree is nicked from codeguru..
// http://www.codeguru.com/Cpp/controls/treeview/treetraversal/comments.php/c683/?thread=7680 

BOOL CMainDlg::findAndRemoveDuplicates(MCONTACT hContact, const char *module)
{
	TVITEMEX item;
	HTREEITEM lastItem, prelastItem;
	BOOL Result = 0;
	wchar_t text[FLD_SIZE];

	if (!m_modules.GetCount())
		return Result;

	_A2T szModule(module);

	item.mask = TVIF_STATE | TVIF_PARAM | TVIF_TEXT;
	item.hItem = TVI_ROOT;
	item.pszText = text;
	item.cchTextMax = _countof(text);
	prelastItem = item.hItem;

	do {
		do {
			lastItem = item.hItem;
			if (lastItem != TVI_ROOT) {
				m_modules.GetItem(&item);
				/* these next lines are not from code guru..... */
				if (item.lParam) {
					ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)item.lParam;
					if (hContact == mtis->hContact && !mir_wstrcmp(text, szModule)) {
						mir_free(mtis);
						m_modules.DeleteItem(item.hItem);
						lastItem = prelastItem;
						Result = 1;
					}
					else
						prelastItem = lastItem;
				}
				/* back to coduguru's code*/
			}
		} while (item.hItem = m_modules.GetChild(lastItem));

		while (!(item.hItem = m_modules.GetNextSibling(lastItem)) && (lastItem = item.hItem = m_modules.GetParent(lastItem))) {}

	} while (item.hItem);
	/*****************************************************************************/

	return Result;
}

void CMainDlg::replaceTreeItem(MCONTACT hContact, const char* module, const char* newModule)
{
	HTREEITEM hItem = findItemInTree(hContact, module);
	if (!hItem)
		return;

	TVITEMEX item;
	item.mask = TVIF_PARAM;
	item.hItem = hItem;

	HTREEITEM hParent = m_modules.GetParent(hItem);

	if (m_modules.GetItem(&item))
		mir_free((ModuleTreeInfoStruct *)item.lParam);

	m_modules.DeleteItem(item.hItem);

	if (!newModule)
		return;

	if (hParent) {
		replaceTreeItem(hContact, newModule, nullptr);
		insertItem(hContact, newModule, hParent);
	}
}

void __cdecl CMainDlg::PopulateModuleTreeThreadFunc(void *param)
{
	char SelectedModule[FLD_SIZE] = "";
	char SelectedSetting[FLD_SIZE] = "";

	MCONTACT hSelectedContact = hRestore;
	
	hRestore = NULL;

	if (!g_pMainWindow)
		return;

	Select = 0;

	switch ((INT_PTR)param) {
	case 1: // restore after rebuild
		if (HTREEITEM item = g_pMainWindow->m_modules.GetSelection()) {
			wchar_t text[FLD_SIZE];
			TVITEMEX tvi = {};
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
			tvi.pszText = text;
			tvi.cchTextMax = _countof(text);
			tvi.hItem = item;

			g_pMainWindow->m_modules.GetItem(&tvi);
			if (tvi.lParam) {
				ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;
				hSelectedContact = mtis->hContact;
				if (mtis->type == CONTACT)
					SelectedModule[0] = 0;
				else
					mir_strncpy(SelectedModule, _T2A(text), _countof(SelectedModule));
				Select = 1;
			}
		}
		break;

	case 2: // restore saved
		if (GetValueA(0, MODULENAME, "LastModule", SelectedModule, _countof(SelectedModule))) {
			hSelectedContact = g_plugin.getDword("LastContact", INVALID_CONTACT_ID);
			if (hSelectedContact != INVALID_CONTACT_ID)
				Select = 1;
			GetValueA(0, MODULENAME, "LastSetting", SelectedSetting, _countof(SelectedSetting));
		}
		break;

	case 3: // restore from user menu
	case 4: // jump from user menu
		if (hSelectedContact && hSelectedContact != INVALID_CONTACT_ID)
			Select = 1;
		break;
	}

	if ((INT_PTR)param != 4) { // do not rebuild on just going to another setting
		ModuleSettingLL modlist;
		if (!EnumModules(&modlist))
			return;

		// remove all items (incase there are items there...
		g_pMainWindow->freeTree(0);
		g_pMainWindow->m_modules.DeleteAllItems();
		g_pMainWindow->m_modules.SelectItem(0);

		/// contact root item
		contacts_mtis.type = CONTACT_ROOT_ITEM;

		TVINSERTSTRUCT tvi;
		tvi.item.lParam = (LPARAM)&contacts_mtis;
		tvi.hParent = nullptr;
		tvi.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.state = TVIS_BOLD;
		tvi.item.stateMask = TVIS_BOLD;
		tvi.item.cChildren = 1;
		tvi.hInsertAfter = TVI_FIRST;

		tvi.item.pszText = TranslateT("Contacts");
		tvi.item.iImage = IMAGE_CONTACTS;
		tvi.item.iSelectedImage = IMAGE_CONTACTS;
		HTREEITEM contactsRoot = g_pMainWindow->m_modules.InsertItem(&tvi);

		// add the settings item
		settings_mtis.type = STUB;
		tvi.item.lParam = (LPARAM)&settings_mtis;
		tvi.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.cChildren = 1;
		tvi.hParent = nullptr;
		tvi.hInsertAfter = TVI_FIRST;
		tvi.item.pszText = TranslateT("Settings");
		tvi.item.iImage = IMAGE_SETTINGS;
		tvi.item.iSelectedImage = IMAGE_SETTINGS;
		HTREEITEM contact = g_pMainWindow->m_modules.InsertItem(&tvi);

		// to fix bug with CHANGE NOTIFY on window activation
		g_pMainWindow->m_modules.SelectItem(contact);
		settings_mtis.type = CONTACT;

		MCONTACT hContact = 0;
		for (ModSetLinkLinkItem *module = modlist.first; module && g_pMainWindow; module = module->next) {
			if (!module->name[0] || IsModuleEmpty(hContact, module->name))
				continue;
			g_pMainWindow->insertItem(hContact, module->name, contact);
		}

		if (g_plugin.bExpandSettingsOnOpen)
			g_pMainWindow->m_modules.Expand(contact, TVE_EXPAND);

		if (Select && hSelectedContact == NULL) {
			HTREEITEM hItem = g_pMainWindow->findItemInTree(hSelectedContact, SelectedModule);
			if (hItem) {
				g_pMainWindow->m_modules.SelectItem(hItem);
				g_pMainWindow->m_modules.Expand(hItem, TVE_EXPAND);
				if (SelectedSetting[0])
					g_pMainWindow->SelectSetting(SelectedSetting);
			}
			Select = 0;
		}

		int count = g_pMainWindow->doContacts(contactsRoot, &modlist, Select ? hSelectedContact : NULL, SelectedModule, SelectedSetting);
		Select = 0;
		g_pMainWindow->doItems(&modlist, count);

		FreeModuleSettingLL(&modlist);
	}

	if (Select) {
		HTREEITEM hItem = g_pMainWindow->findItemInTree(hSelectedContact, SelectedModule);
		if (hItem) {
			g_pMainWindow->m_modules.SelectItem(hItem);
			g_pMainWindow->m_modules.Expand(hItem, TVE_EXPAND);
			if (SelectedSetting[0])
				g_pMainWindow->SelectSetting(SelectedSetting);
		}
	}

	populating = 0;
}

void refreshTree(BOOL restore)
{
	if (populating)
		return;
	populating = 1;
	mir_forkthread(&CMainDlg::PopulateModuleTreeThreadFunc, (HWND)restore);
}

// hwnd here is to the main window, NOT the treview
void CMainDlg::onContextMenu_Modules(CContextMenuPos *pos)
{
	wchar_t text[FLD_SIZE];
	TVITEMEX tvi = {};
	tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
	tvi.hItem = pos->hItem;
	tvi.pszText = text;
	tvi.cchTextMax = _countof(text);
	m_modules.GetItem(&tvi);
	if (!tvi.lParam)
		return;

	_T2A module(text);
	ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;

	MCONTACT hContact = mtis->hContact;

	HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXTMENU));
	TranslateMenu(hMenu);

	int menuNumber;
	if (mtis->type == CONTACT && hContact)
		menuNumber = 2;
	else if ((mtis->type == MODULE) && !hContact)
		menuNumber = 1;
	else if (mtis->type == CONTACT && !hContact)
		menuNumber = 3;
	else if (mtis->type == CONTACT_ROOT_ITEM && !hContact)
		menuNumber = 4;
	else if ((mtis->type == MODULE) && hContact)
		menuNumber = 5;
	else return;

	HMENU hSubMenu = GetSubMenu(hMenu, menuNumber);

	TranslateMenu(hSubMenu);
	switch (menuNumber) {
	case 1: // null module
	case 5: // contact module
		{
			// check if the setting is being watched and if it is then check the menu item
			int watchIdx = WatchedArrayIndex(hContact, module, nullptr, 1);
			if (watchIdx >= 0)
				CheckMenuItem(hSubMenu, MENU_WATCH_ITEM, MF_CHECKED | MF_BYCOMMAND);

			switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pos->pt.x, pos->pt.y, 0, m_hwnd, nullptr)) {
			case MENU_RENAME_MOD:
				skipEnter = true;
				m_modules.EditLabel(tvi.hItem);
				break;

			case MENU_DELETE_MOD:
				if (deleteModule(m_hwnd, hContact, module, 1)) {
					m_modules.DeleteItem(pos->hItem);
					mir_free(mtis);
				}
				break;

			case MENU_COPY_MOD:
				copyModuleMenuItem(m_hwnd, hContact, module);
				break;

				////////////////////////////////////////////////////////////////////// divider
			case MENU_WATCH_ITEM:
				if (watchIdx < 0)
					addSettingToWatchList(hContact, module, nullptr);
				else
					freeWatchListItem(watchIdx);
				PopulateWatchedWindow();
				break;

			case MENU_REFRESH:
				refreshTree(1);
				break;

			case MENU_EXPORTMODULE:
				exportDB(hContact, module);
				break;

			case MENU_EXPORTDB:
				exportDB(INVALID_CONTACT_ID, module);
				break;
			}
		}
		break;

	case 2: // contact
		switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pos->pt.x, pos->pt.y, 0, m_hwnd, nullptr)) {
		case MENU_DELETE_CONTACT:
			if (db_get_b(0, "CList", "ConfirmDelete", 1)) {
				wchar_t str[MSG_SIZE];
				mir_snwprintf(str, TranslateT("Are you sure you want to delete contact \"%s\"?"), text);
				if (g_pMainWindow->dlg(str, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
					break;
			}
			db_delete_contact(hContact);
			freeTree(hContact);
			m_modules.DeleteItem(tvi.hItem);
			break;

		case MENU_EXPORTCONTACT:
			exportDB(hContact, nullptr);
			break;

		case MENU_IMPORTFROMTEXT:
			ImportSettingsMenuItem(hContact);
			break;

		case MENU_IMPORTFROMFILE:
			ImportSettingsFromFileMenuItem(hContact, nullptr);
			break;

		case MENU_ADD_MODULE:
			addModuleDlg(hContact);
			break;

		case MENU_REFRESH:
			refreshTree(1);
			break;
		}
		break;

	case 3: // NULL contact
		switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pos->pt.x, pos->pt.y, 0, m_hwnd, nullptr)) {
		case MENU_ADD_MODULE:
			addModuleDlg(hContact);
			break;
		case MENU_EXPORTCONTACT:
			exportDB(NULL, nullptr);
			break;
		case MENU_IMPORTFROMTEXT:
			ImportSettingsMenuItem(NULL);
			break;
		case MENU_IMPORTFROMFILE:
			ImportSettingsFromFileMenuItem(NULL, nullptr);
			break;
		case MENU_REFRESH:
			refreshTree(1);
			break;
		}
		break;

	case 4: // Contacts root
		switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pos->pt.x, pos->pt.y, 0, m_hwnd, nullptr)) {
		case MENU_EXPORTCONTACT:
			exportDB(INVALID_CONTACT_ID, "");
			break;
		case MENU_IMPORTFROMTEXT:
			ImportSettingsMenuItem(NULL);
			break;
		case MENU_IMPORTFROMFILE:
			ImportSettingsFromFileMenuItem(NULL, nullptr);
			break;
		case MENU_REFRESH:
			refreshTree(1);
			break;
		}
		break;
	}
	DestroyMenu(hMenu);
}
