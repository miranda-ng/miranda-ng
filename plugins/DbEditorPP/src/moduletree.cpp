#include "stdafx.h"

HWND hwnd2Tree = 0;

volatile BOOL populating = 0;
volatile int Select = 0;

static ModuleTreeInfoStruct contacts_mtis = { CONTACT_ROOT_ITEM, 0 };
static ModuleTreeInfoStruct settings_mtis = { CONTACT, 0 };


void insertItem(MCONTACT hContact, const char *module, HTREEITEM hParent)
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
	TreeView_InsertItem(hwnd2Tree, &tvi);
}


int doContacts(HTREEITEM contactsRoot, ModuleSettingLL *modlist, MCONTACT hSelectedContact, const char *selectedModule, const char *selectedSetting)
{
	TVINSERTSTRUCT tvi;
	HTREEITEM contact;
	ModuleTreeInfoStruct *lParam;
	int itemscount = 0;
	int icon = 0;
	HTREEITEM hItem = 0;

	SetWindowText(hwnd2mainWindow, TranslateT("Loading contacts..."));

	tvi.hInsertAfter = TVI_SORT;
	tvi.item.cChildren = 1;
	
	char szProto[FLD_SIZE];
	TCHAR name[NAME_SIZE];

	for (MCONTACT hContact = db_find_first(); hContact && hwnd2mainWindow; hContact = db_find_next(hContact)) {
		
		if (ApplyProtoFilter(hContact)) continue;

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

		contact = TreeView_InsertItem(hwnd2Tree, &tvi);

		itemscount++;

		if (hSelectedContact == hContact) {

			for (ModSetLinkLinkItem *module = modlist->first; module && hwnd2mainWindow; module = module->next) {
				if (!module->name[0] || IsModuleEmpty(hContact, module->name))
					continue;
					insertItem(hContact, module->name, contact);
			}

			hItem = findItemInTree(hSelectedContact, selectedModule);
		}
	}

	if (hItem) {
		TreeView_SelectItem(hwnd2Tree, hItem);
		TreeView_Expand(hwnd2Tree, hItem, TVE_EXPAND);
		if (selectedSetting && selectedSetting[0])
			SelectSetting(selectedSetting);
	}

	return itemscount;
}


void doItems(ModuleSettingLL* modlist, int count)
{
	HWND hwnd = GetParent(hwnd2Tree); //!!! 

	TCHAR percent[128], title[96];
	mir_sntprintf(title, TranslateT("Loading modules..."));

	TVITEM item = { 0 };
	item.mask = TVIF_STATE | TVIF_PARAM;

	HTREEITEM contact = TreeView_GetChild(hwnd2Tree, TVI_ROOT);
	contact = TreeView_GetNextSibling(hwnd2Tree, contact);
	contact = TreeView_GetChild(hwnd2Tree, contact);

	MCONTACT hContact = 0;
	for (int i = 1; contact && hwnd2mainWindow; i++) {
		item.hItem = contact;
		contact = TreeView_GetNextSibling(hwnd2Tree, contact);

		if (TreeView_GetItem(hwnd2Tree, &item) && item.lParam) {
			ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)item.lParam;
			hContact = mtis->hContact;
			if (hContact == NULL || mtis->type != (CONTACT | EMPTY))
				continue;
			mtis->type = CONTACT;
		}
		else
			continue;

		// Caption
		mir_sntprintf(percent, _T("%s %d%%"), title, (int)(100 * i / count));
		SetWindowText(hwnd, percent);

		for (ModSetLinkLinkItem *module = modlist->first; module && hwnd2mainWindow; module = module->next) {
			if (!module->name[0] || IsModuleEmpty(hContact, module->name))
				continue;

			insertItem(hContact, module->name, item.hItem);
		}
	}

	SetWindowText(hwnd2mainWindow, TranslateT("Database Editor++"));
}

// the following code to go through the whole tree is nicked from codeguru..
// http://www.codeguru.com/Cpp/controls/treeview/treetraversal/comments.php/c683/?thread=7680

HTREEITEM findItemInTree(MCONTACT hContact, const char* module)
{
	TVITEM item;
	HTREEITEM lastItem;
	TCHAR text[FLD_SIZE];

	if (!TreeView_GetCount(hwnd2Tree))
		return 0;

	_A2T szModule(module);

	item.mask = TVIF_STATE | TVIF_PARAM | TVIF_TEXT;
	item.hItem = TVI_ROOT;
	item.pszText = text;
	item.cchTextMax = _countof(text);
	do {
		do {
			lastItem = item.hItem;
			if (lastItem != TVI_ROOT) {
				/* these next 2 lines are not from code guru..... */
				if (TreeView_GetItem(hwnd2Tree, &item) && item.lParam) {
					if ((hContact == ((ModuleTreeInfoStruct *)item.lParam)->hContact) && (!module || !module[0] || !mir_tstrcmp(szModule, text))) {
						return item.hItem;

					}
				}
				/* back to coduguru's code*/
			}
		} while ((item.hItem = TreeView_GetChild(hwnd2Tree, lastItem)));

		while ((!(item.hItem = TreeView_GetNextSibling(hwnd2Tree, lastItem))) && (lastItem = item.hItem = TreeView_GetParent(hwnd2Tree, lastItem)));

	} while (item.hItem);

	return 0;
}

// the following code to go through the whole tree is nicked from codeguru..
// http://www.codeguru.com/Cpp/controls/treeview/treetraversal/comments.php/c683/?thread=7680

void freeTree(MCONTACT hContact)
{
	TVITEM item;
	HTREEITEM lastItem;
	if (!TreeView_GetCount(hwnd2Tree))
		return;

	item.mask = TVIF_STATE | TVIF_PARAM;
	item.hItem = TVI_ROOT;
	do {
		do {
			lastItem = item.hItem;
			if (lastItem != TVI_ROOT) {
				TreeView_GetItem(hwnd2Tree, &item);
				/* these next 2 lines are not from code guru..... */
				if (item.lParam) {
					ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)item.lParam;

					if (!hContact || (hContact == mtis->hContact)) {
						if (hContact != NULL) {
							TreeView_DeleteItem(hwnd2Tree, item.hItem);
							mir_free(mtis);
						}
						else
							mtis->type = STUB;
					}
				}
				/* back to coduguru's code*/
			}
		} while (item.hItem = TreeView_GetChild(hwnd2Tree, lastItem));

		while (!(item.hItem = TreeView_GetNextSibling(hwnd2Tree, lastItem)) && (lastItem = item.hItem = TreeView_GetParent(hwnd2Tree, lastItem))) {}

	} while (item.hItem);
}

BOOL findAndRemoveDuplicates(MCONTACT hContact, const char *module)
/* the following code to go through the whole tree is nicked from codeguru..
http://www.codeguru.com/Cpp/controls/treeview/treetraversal/comments.php/c683/?thread=7680 */
{
	TVITEM item;
	HTREEITEM lastItem, prelastItem;
	BOOL Result = 0;
	TCHAR text[FLD_SIZE];

	if (!TreeView_GetCount(hwnd2Tree))
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
				TreeView_GetItem(hwnd2Tree, &item);
				/* these next lines are not from code guru..... */
				if (item.lParam) {
					ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)item.lParam;
					if (hContact == mtis->hContact && !mir_tstrcmp(text, szModule)) {
						mir_free(mtis);
						TreeView_DeleteItem(hwnd2Tree, item.hItem);
						lastItem = prelastItem;
						Result = 1;
					}
					else
						prelastItem = lastItem;
				}
				/* back to coduguru's code*/
			}
		} while (item.hItem = TreeView_GetChild(hwnd2Tree, lastItem));

		while (!(item.hItem = TreeView_GetNextSibling(hwnd2Tree, lastItem)) && (lastItem = item.hItem = TreeView_GetParent(hwnd2Tree, lastItem))) {}

	} while (item.hItem);
	/*****************************************************************************/

	return Result;
}


void replaceTreeItem(MCONTACT hContact, const char* module, const char* newModule)
{
	HTREEITEM hItem = findItemInTree(hContact, module);

	if (!hItem)
		return;

	TVITEM item;
	item.mask = TVIF_PARAM;
	item.hItem = hItem;

	HTREEITEM hParent = TreeView_GetParent(hwnd2Tree, hItem);

	if (TreeView_GetItem(hwnd2Tree, &item))
		mir_free((ModuleTreeInfoStruct *)item.lParam);

	TreeView_DeleteItem(hwnd2Tree, item.hItem);

	if (!newModule)
		return;

	if (hParent) {
		replaceTreeItem(hContact, newModule, NULL);
		insertItem(hContact, newModule, hParent);
	}
}


void __cdecl PopulateModuleTreeThreadFunc(LPVOID param)
{
	TVINSERTSTRUCT tvi;
	char SelectedModule[FLD_SIZE] = "";
	char SelectedSetting[FLD_SIZE] = "";

	MCONTACT hSelectedContact = hRestore;
	MCONTACT hContact;
	HTREEITEM contact, contactsRoot;
	int count;

	// module list
	ModuleSettingLL modlist;

	hRestore = NULL;

	if (!hwnd2Tree)
		return;

	Select = 0;

	switch ((int)param) {
	case 1: // restore after rebuild
		if (HTREEITEM item = TreeView_GetSelection(hwnd2Tree)) {
			TCHAR text[FLD_SIZE];
			TVITEM tvi = { 0 };
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
			tvi.pszText = text;
			tvi.cchTextMax = _countof(text);
			tvi.hItem = item;

			TreeView_GetItem(hwnd2Tree, &tvi);
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
		if (GetValueA(NULL, modname, "LastModule", SelectedModule, _countof(SelectedModule))) {
			hSelectedContact = db_get_dw(NULL, modname, "LastContact", INVALID_CONTACT_ID);
			if (hSelectedContact != INVALID_CONTACT_ID)
				Select = 1;
			GetValueA(NULL, modname, "LastSetting", SelectedSetting, _countof(SelectedSetting));
		}
		break;

	case 3: // restore from user menu
	case 4: // jump from user menu
		if (hSelectedContact && hSelectedContact != INVALID_CONTACT_ID)
			Select = 1;
		break;
	}

	if ((int)param != 4) { // do not rebuild on just going to another setting
		if (!EnumModules(&modlist))
			return;

		// remove all items (incase there are items there...
		freeTree(0);
		TreeView_DeleteAllItems(hwnd2Tree);
		TreeView_SelectItem(hwnd2Tree, 0);

		/// contact root item
		contacts_mtis.type = CONTACT_ROOT_ITEM;
		tvi.item.lParam = (LPARAM)&contacts_mtis;
		tvi.hParent = NULL;
		tvi.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.state = TVIS_BOLD;
		tvi.item.stateMask = TVIS_BOLD;
		tvi.item.cChildren = 1;
		tvi.hInsertAfter = TVI_FIRST;

		tvi.item.pszText = TranslateT("Contacts");
		tvi.item.iImage = IMAGE_CONTACTS;
		tvi.item.iSelectedImage = IMAGE_CONTACTS;
		contactsRoot = TreeView_InsertItem(hwnd2Tree, &tvi);

		// add the settings item
		settings_mtis.type = STUB;
		tvi.item.lParam = (LPARAM)&settings_mtis;
		tvi.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.cChildren = 1;
		tvi.hParent = NULL;
		tvi.hInsertAfter = TVI_FIRST;
		tvi.item.pszText = TranslateT("Settings");
		tvi.item.iImage = IMAGE_SETTINGS;
		tvi.item.iSelectedImage = IMAGE_SETTINGS;
		contact = TreeView_InsertItem(hwnd2Tree, &tvi);

		// to fix bug with CHANGE NOTIFY on window activation
		TreeView_SelectItem(hwnd2Tree, contact);
		settings_mtis.type = CONTACT;

		hContact = 0;
		for (ModSetLinkLinkItem *module = modlist.first; module && hwnd2mainWindow; module = module->next) {
			if (!module->name[0] || IsModuleEmpty(hContact, module->name))
				continue;
				insertItem(hContact, module->name, contact);
		}

		if (db_get_b(NULL, modname, "ExpandSettingsOnOpen", 0))
			TreeView_Expand(hwnd2Tree, contact, TVE_EXPAND);

		if (Select && hSelectedContact == NULL) {
			HTREEITEM hItem = findItemInTree(hSelectedContact, SelectedModule);
			if (hItem) {
				TreeView_SelectItem(hwnd2Tree, hItem);
				TreeView_Expand(hwnd2Tree, hItem, TVE_EXPAND);
				if (SelectedSetting[0])
					SelectSetting(SelectedSetting);
			}
			Select = 0;
		}

		count = doContacts(contactsRoot, &modlist, Select ? hSelectedContact : NULL, SelectedModule, SelectedSetting);
		Select = 0;
		doItems(&modlist, count);

		FreeModuleSettingLL(&modlist);
	}

	if (Select) {
		HTREEITEM hItem = findItemInTree(hSelectedContact, SelectedModule);
		if (hItem) {
			TreeView_SelectItem(hwnd2Tree, hItem);
			TreeView_Expand(hwnd2Tree, hItem, TVE_EXPAND);
			if (SelectedSetting[0])
				SelectSetting(SelectedSetting);
		}
	}

	populating = 0;

}

void refreshTree(int restore)
{
	if (populating)
		return;
	populating = 1;
	mir_forkthread(PopulateModuleTreeThreadFunc, (HWND)restore);
}


static LRESULT CALLBACK ModuleTreeLabelEditSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYUP:
		switch (wParam) {
		case VK_RETURN:
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

void moduleListRightClick(HWND hwnd, WPARAM wParam, LPARAM lParam);

void moduleListWM_NOTIFY(HWND hwnd, UINT, WPARAM wParam, LPARAM lParam)// hwnd here is to the main window, NOT the treview
{
	switch (((NMHDR *)lParam)->code) {
	case TVN_ITEMEXPANDING:
		if (populating && ((LPNMTREEVIEW)lParam)->action == TVE_EXPAND) {
			ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)((LPNMTREEVIEW)lParam)->itemNew.lParam;
			if (mtis && (mtis->type == (CONTACT | EMPTY))) {
				MCONTACT hContact = mtis->hContact;
				mtis->type = CONTACT;

				ModuleSettingLL modlist;
				if (!EnumModules(&modlist))
					break;

				ModSetLinkLinkItem *module = modlist.first;
				while (module && hwnd2mainWindow) {
					if (module->name[0] && !IsModuleEmpty(hContact, module->name)) {
						insertItem(hContact, module->name, ((LPNMTREEVIEW)lParam)->itemNew.hItem);
					}
					module = (ModSetLinkLinkItem *)module->next;
				}

				FreeModuleSettingLL(&modlist);
			}
		}
		break;

	case TVN_SELCHANGED:
		{
			LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
			TVITEM tvi = { 0 };
			TCHAR text[FLD_SIZE];
			MCONTACT hContact;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
			tvi.hItem = pnmtv->itemNew.hItem;
			tvi.pszText = text;
			tvi.cchTextMax = _countof(text);
			TreeView_GetItem(pnmtv->hdr.hwndFrom, &tvi);
			
			ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;

			if (mtis) {
				
				hContact = mtis->hContact;

				if (mtis->type == STUB)
					break;

				if (populating)
					Select = 0;

				if (mtis->type == MODULE) {
					_T2A module(text);
					PopulateSettings(hContact, module);
				}
				else 
				if (((mtis->type & CONTACT) == CONTACT && hContact) || (mtis->type == CONTACT_ROOT_ITEM && !hContact)) {
					int multi = 0;

					ClearListView();

					if (mtis->type == CONTACT_ROOT_ITEM && !hContact) {
						multi = 1;
						hContact = db_find_first();
					}

					while (hContact && hwnd2mainWindow) {

						if (multi && ApplyProtoFilter(hContact)) {
							hContact = db_find_next(hContact);
							continue;
						}

						addListHandle(hContact);

						if (!multi) {
							break;
						}

						hContact = db_find_next(hContact);
					}
				}
				else
					ClearListView();
			}
			else 
				ClearListView();
		}
		break; //TVN_SELCHANGED:

	case NM_RCLICK:
		if (((NMHDR *)lParam)->code == NM_RCLICK)
			moduleListRightClick(hwnd, wParam, lParam);
		break;

	case TVN_BEGINLABELEDIT: // subclass it..
		{
			LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)lParam;
			ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)ptvdi->item.lParam;
			HWND hwnd2Edit = TreeView_GetEditControl(hwnd2Tree);
			if (!mtis->type || (mtis->type == CONTACT)) {
				SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
				break;
			}
			mir_subclassWindow(hwnd2Edit, ModuleTreeLabelEditSubClassProc);
			SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
		}
		break;

	case TVN_ENDLABELEDIT:
		LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)lParam;
		TVITEM tvi = { 0 };
		TCHAR text[FLD_SIZE];
		ModuleTreeInfoStruct *mtis;
		tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
		tvi.hItem = ptvdi->item.hItem;
		tvi.pszText = text;
		tvi.cchTextMax = _countof(text);
		TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
		mtis = (ModuleTreeInfoStruct *)ptvdi->item.lParam;

		_T2A newtext(ptvdi->item.pszText);
		_T2A oldtext(tvi.pszText);

		if (!newtext // edit control failed
			|| !mtis->type // its a root item
			|| mtis->type == CONTACT // its a contact
			|| newtext[0] == 0)  // empty string
			SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
		else {
			if (mir_strcmp(oldtext, newtext)) {
				renameModule(mtis->hContact, oldtext, newtext);
				findAndRemoveDuplicates(mtis->hContact, newtext);
				if (TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi))
					PopulateSettings(mtis->hContact, newtext);
			}
			SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
		}
		break;
	}
}

void moduleListRightClick(HWND hwnd, WPARAM, LPARAM lParam) // hwnd here is to the main window, NOT the treview
{
	TVHITTESTINFO hti;
	hti.pt.x = (short)LOWORD(GetMessagePos());
	hti.pt.y = (short)HIWORD(GetMessagePos());
	ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);

	if (!TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti) || !(hti.flags & TVHT_ONITEM)) return; 

	TVITEM tvi = { 0 };
	HMENU hMenu, hSubMenu;
	int menuNumber;
	TCHAR text[FLD_SIZE];

	tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
	tvi.hItem = hti.hItem;
	tvi.pszText = text;
	tvi.cchTextMax = _countof(text);
	TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);

	if (!tvi.lParam) return;

	_T2A module(text);
	ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;

	MCONTACT hContact = mtis->hContact;
	GetCursorPos(&hti.pt);

	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXTMENU));
	TranslateMenu(hMenu);

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

	hSubMenu = GetSubMenu(hMenu, menuNumber);

	TranslateMenu(hSubMenu);
	switch (menuNumber) {
	case 1: // null module
	case 5: // contact module
	{
		// check if the setting is being watched and if it is then check the menu item
		int watchIdx = WatchedArrayIndex(hContact, module, NULL, 1);
		if (watchIdx >= 0) 
			CheckMenuItem(hSubMenu, MENU_WATCH_ITEM, MF_CHECKED | MF_BYCOMMAND);

		switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, hti.pt.x, hti.pt.y, 0, hwnd, NULL)) {
		case MENU_RENAME_MOD:
			TreeView_EditLabel(hwnd2Tree, tvi.hItem);
			break;

		case MENU_DELETE_MOD:
			if (deleteModule(hContact, module, 1)) {
				TreeView_DeleteItem(((LPNMHDR)lParam)->hwndFrom, hti.hItem);
				mir_free(mtis);
			}
			break;

		case MENU_COPY_MOD:
			copyModuleMenuItem(hContact, module);
			break;

			////////////////////////////////////////////////////////////////////// divider
		case MENU_WATCH_ITEM:
			if (watchIdx < 0)
				addSettingToWatchList(hContact, module, 0);
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
		switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, hti.pt.x, hti.pt.y, 0, hwnd, NULL)) {
		case MENU_CLONE_CONTACT:
			if (CloneContact(hContact))
				refreshTree(1);
			break;

		case MENU_DELETE_CONTACT:
			if (db_get_b(NULL, "CList", "ConfirmDelete", 1)) {
				TCHAR str[MSG_SIZE];
				mir_sntprintf(str, TranslateT("Are you sure you want to delete contact \"%s\"?"), text);
				if (dlg(str, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
					break;
			}
			CallService(MS_DB_CONTACT_DELETE, hContact, 0);
			freeTree(hContact);
			TreeView_DeleteItem(hwnd2Tree, tvi.hItem);
			break;

			////////////////////////////////////////////////////////////////////// divider
		case MENU_EXPORTCONTACT:
			exportDB(hContact, 0);
			break;
		case MENU_IMPORTFROMTEXT:
			ImportSettingsMenuItem(hContact);
			break;
		case MENU_IMPORTFROMFILE:
			ImportSettingsFromFileMenuItem(hContact, NULL);
			break;

			////////////////////////////////////////////////////////////////////// divider
		case MENU_ADD_MODULE:
			addModuleDlg(hContact);
			break;

		case MENU_REFRESH:
			refreshTree(1);			
			break;

		}
		break;

	case 3: // NULL contact
		switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, hti.pt.x, hti.pt.y, 0, hwnd, NULL)) {
		case MENU_ADD_MODULE:
			addModuleDlg(hContact);
			break;
		case MENU_EXPORTCONTACT:
			exportDB(NULL, 0);
			break;
		case MENU_IMPORTFROMTEXT:
			ImportSettingsMenuItem(NULL);
			break;
		case MENU_IMPORTFROMFILE:
			ImportSettingsFromFileMenuItem(NULL, NULL);
			break;
		case MENU_REFRESH:
			refreshTree(1);			
			break;
		}
		break;

	case 4: // Contacts root
		switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, hti.pt.x, hti.pt.y, 0, hwnd, NULL)) {
		case MENU_EXPORTCONTACT:
			exportDB(INVALID_CONTACT_ID, 0);
			break;
		case MENU_IMPORTFROMTEXT:
			ImportSettingsMenuItem(NULL);
			break;
		case MENU_IMPORTFROMFILE:
			ImportSettingsFromFileMenuItem(NULL, NULL);
			break;
		case MENU_REFRESH:
			refreshTree(1);			
			break;
		}
		break;
	}
	DestroyMenu(hMenu);

}
