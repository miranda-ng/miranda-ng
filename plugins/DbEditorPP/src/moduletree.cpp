#include "headers.h"

static BOOL populating = 0;
int Select = 0;
static ModuleTreeInfoStruct contacts_mtis = { CONTACT_ROOT_ITEM, 0 };
static ModuleTreeInfoStruct settings_mtis = { CONTACT, 0 };

int doContacts(HWND hwnd2Tree, HTREEITEM contactsRoot, ModuleSettingLL *modlist, MCONTACT hSelectedContact, char *SelectedModule, char *SelectedSetting)
{
	TVINSERTSTRUCT tvi;
	HTREEITEM contact;
	ModuleTreeInfoStruct *lParam;
	ModSetLinkLinkItem *module;
	int itemscount = 0;
	int loaded, i = 0, icon = 0;
	int hItem = -1;

	SetWindowText(hwnd2mainWindow, Translate("Loading contacts..."));

	tvi.hInsertAfter = TVI_SORT;
	tvi.item.cChildren = 1;

	for (MCONTACT hContact = db_find_first(); hContact && hwnd2mainWindow; hContact = db_find_next(hContact)) {
		char szProto[100];
		if (DBGetContactSettingStringStatic(hContact, "Protocol", "p", szProto, SIZEOF(szProto))) {
			icon = GetProtoIcon(szProto);
			loaded = (icon != DEF_ICON);
		}
		else {
			icon = DEF_ICON;
			loaded = 0;
		}

		i++;

		// filter
		if ((loaded && Mode == MODE_UNLOADED) || (!loaded && Mode == MODE_LOADED))
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

		// contacts name
		WCHAR nick[256];
		WCHAR protoW[256]; // unicode proto

		if (szProto)
			a2u(szProto, protoW, SIZEOF(protoW));
		else
			protoW[0] = 0;

		if (!szProto || !loaded) {
			tvi.item.iSelectedImage = (tvi.item.iImage = 4);

			if (protoW) {
				if (Order)
					mir_snwprintf(nick, SIZEOF(nick), L"(%s) %s %s", protoW, GetContactName(hContact, szProto, 1), L"(UNLOADED)");
				else
					mir_snwprintf(nick, SIZEOF(nick), L"%s (%s) %s", GetContactName(hContact, szProto, 1), protoW, L"(UNLOADED)");
			}
			else
				wcscpy(nick, nick_unknownW);
		}
		else {
			tvi.item.iSelectedImage = (tvi.item.iImage = icon); //GetProtoIcon(szProto, 7));
			if (Order)
				mir_snwprintf(nick, SIZEOF(nick), L"(%s) %s", protoW, GetContactName(hContact, szProto, 1));
			else
				mir_snwprintf(nick, SIZEOF(nick), L"%s (%s)", GetContactName(hContact, szProto, 1), protoW);
		}

		tvi.item.pszText = (char *)nick;
		contact = TreeView_InsertItemW(hwnd2Tree, &tvi);

		itemscount++;

		if (hSelectedContact == hContact) {
			module = modlist->first;
			while (module && hwnd2mainWindow) {
				if (module->name[0] && !IsModuleEmpty(hContact, module->name)) {
					tvi.hParent = contact;
					tvi.hInsertAfter = TVI_SORT;
					tvi.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
					tvi.item.pszText = module->name;

					lParam = (ModuleTreeInfoStruct *)mir_calloc(sizeof(ModuleTreeInfoStruct));
					lParam->hContact = hContact;

					tvi.item.iImage = 1;
					tvi.item.iSelectedImage = 2;
					lParam->type = KNOWN_MODULE;

					tvi.item.lParam = (LPARAM)lParam;
					TreeView_InsertItem(hwnd2Tree, &tvi);
				}
				module = (ModSetLinkLinkItem *)module->next;
			}

			hItem = findItemInTree(hwnd2Tree, hSelectedContact, SelectedModule);
		}
	}

	if (hItem != -1) {
		TreeView_SelectItem(hwnd2Tree, (HTREEITEM)hItem);
		TreeView_Expand(hwnd2Tree, hItem, TVE_EXPAND);
		if (SelectedSetting[0])
			SelectSetting(SelectedSetting);
	}

	return itemscount;
}

void doItems(HWND hwnd2Tree, ModuleSettingLL *modlist, int count)
{
	HWND hwnd = GetParent(hwnd2Tree);

	char percent[96], title[64];
	mir_snprintf(title, SIZEOF(title), Translate("Loading modules..."));

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
		mir_snprintf(percent, SIZEOF(percent), "%s %d%%", title, (int)(100 * i / count));
		SetWindowText(hwnd, percent);

		for (ModSetLinkLinkItem *module = modlist->first; module && hwnd2mainWindow; module = module->next) {
			if (!module->name[0] || IsModuleEmpty(hContact, module->name))
				continue;

			TVINSERTSTRUCT tvi;
			tvi.hParent = item.hItem;
			tvi.hInsertAfter = TVI_SORT;
			tvi.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
			tvi.item.pszText = module->name;

			ModuleTreeInfoStruct *lParam = (ModuleTreeInfoStruct *)mir_calloc(sizeof(ModuleTreeInfoStruct));
			lParam->hContact = hContact;

			tvi.item.iImage = 1;
			tvi.item.iSelectedImage = 2;
			lParam->type = KNOWN_MODULE;

			tvi.item.lParam = (LPARAM)lParam;
			TreeView_InsertItem(hwnd2Tree, &tvi);
		}
	}

	SetWindowText(hwnd, TranslateT("Database Editor++"));
}

// the following code to go through the whole tree is nicked from codeguru..
// http://www.codeguru.com/Cpp/controls/treeview/treetraversal/comments.php/c683/?thread=7680

int findItemInTree(HWND hwnd2Tree, MCONTACT hContact, char *module)
{
	TVITEM item;
	char text[265];
	HTREEITEM lastItem;
	if (!TreeView_GetCount(hwnd2Tree))
		return 0;

	item.mask = TVIF_STATE | TVIF_PARAM | TVIF_TEXT;
	item.hItem = TVI_ROOT;
	item.pszText = text;
	item.cchTextMax = SIZEOF(text);
	do {
		do {
			lastItem = item.hItem;
			if (lastItem != TVI_ROOT) {
				/* these next 2 lines are not from code guru..... */
				if (TreeView_GetItem(hwnd2Tree, &item) && item.lParam) {
					if ((hContact == ((ModuleTreeInfoStruct *)item.lParam)->hContact) && (!module[0] || !mir_strcmp(module, text))) {
						//TreeView_SelectItem(hwnd2Tree,item.hItem);
						return (int)item.hItem;
					}
				}
				/* back to coduguru's code*/
			}
		} while ((item.hItem = TreeView_GetChild(hwnd2Tree, lastItem)));

		while ((!(item.hItem = TreeView_GetNextSibling(hwnd2Tree, lastItem))) && (lastItem = item.hItem = TreeView_GetParent(hwnd2Tree, lastItem)));

	} while (item.hItem);
	return -1;
}

// the following code to go through the whole tree is nicked from codeguru..
// http://www.codeguru.com/Cpp/controls/treeview/treetraversal/comments.php/c683/?thread=7680

void freeTree(HWND hwnd2Tree, MCONTACT hContact)
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

BOOL findAndRemoveDuplicates(HWND hwnd2Tree, MCONTACT hContact, char *module)
/* the following code to go through the whole tree is nicked from codeguru..
http://www.codeguru.com/Cpp/controls/treeview/treetraversal/comments.php/c683/?thread=7680 */
{
	TVITEM item;
	HTREEITEM lastItem, prelastItem;
	BOOL Result = 0;
	char text[265];
	if (!TreeView_GetCount(hwnd2Tree))
		return Result;

	item.mask = TVIF_STATE | TVIF_PARAM | TVIF_TEXT;
	item.hItem = TVI_ROOT;
	item.pszText = text;
	item.cchTextMax = SIZEOF(text);
	prelastItem = item.hItem;

	do {
		do {
			lastItem = item.hItem;
			if (lastItem != TVI_ROOT) {
				TreeView_GetItem(hwnd2Tree, &item);
				/* these next lines are not from code guru..... */
				if (item.lParam) {
					ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)item.lParam;
					if (hContact == mtis->hContact && !mir_strcmp(text, module)) {
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


void replaceTreeItem(HWND hwnd, MCONTACT hContact, const char *module, const char *newModule)
{
	int hItem = findItemInTree(hwnd, hContact, (char *)module);
	HTREEITEM hParent;

	if (hItem == -1)
		return;

	hParent = TreeView_GetParent(hwnd, (HTREEITEM)hItem);

	{
		TVITEM item;
		item.mask = TVIF_PARAM;
		item.hItem = (HTREEITEM)hItem;

		if (TreeView_GetItem(hwnd, &item))
			mir_free((ModuleTreeInfoStruct *)item.lParam);
		TreeView_DeleteItem(hwnd, item.hItem);
	}

	if (hParent && newModule) {
		TVINSERTSTRUCT tvi = { 0 };
		ModuleTreeInfoStruct *lParam;

		tvi.hParent = hParent;
		tvi.hInsertAfter = TVI_SORT;
		tvi.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
		tvi.item.pszText = (char *)newModule;

		lParam = (ModuleTreeInfoStruct *)mir_calloc(sizeof(ModuleTreeInfoStruct));
		lParam->hContact = hContact;
		lParam->type = KNOWN_MODULE;
		tvi.item.iImage = 1;
		tvi.item.iSelectedImage = 2;

		tvi.item.lParam = (LPARAM)lParam;

		TreeView_InsertItem(hwnd, &tvi);
	}
}

void refreshTree(int restore)
{
	if (populating)
		return;
	populating = 1;
	forkthread(PopulateModuleTreeThreadFunc, 0, (HWND)restore);
}

void __cdecl PopulateModuleTreeThreadFunc(LPVOID di)
{
	TVINSERTSTRUCT tvi;
	HWND hwnd2Tree = GetDlgItem(hwnd2mainWindow, IDC_MODULES);
	char SelectedModule[256] = { 0 };
	char SelectedSetting[256] = { 0 };
	MCONTACT hSelectedContact = hRestore;
	MCONTACT hContact;
	HTREEITEM contact, contactsRoot;
	int count;

	// item lParams
	ModuleTreeInfoStruct *lParam;

	// module list
	ModSetLinkLinkItem *module;
	ModuleSettingLL modlist;

	hRestore = NULL;

	if (!hwnd2Tree) {
		msg(Translate("Module tree not found"), modFullname);
		return;
	}

	Select = 0;

	switch ((int)di) {
	case 1: // restore after rebuild
		if (HTREEITEM item = TreeView_GetSelection(hwnd2Tree)) {
			TVITEM tvi = { 0 };
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
			tvi.pszText = SelectedModule;
			tvi.cchTextMax = SIZEOF(SelectedModule);
			tvi.hItem = item;

			TreeView_GetItem(hwnd2Tree, &tvi);
			if (tvi.lParam) {
				ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)tvi.lParam;
				hSelectedContact = mtis->hContact;
				if (mtis->type == CONTACT)
					SelectedModule[0] = 0;
				Select = 1;
			}
		}
		break;

	case 2: // restore saved
		if (GetValue(NULL, modname, "LastModule", SelectedModule, SIZEOF(SelectedModule))) {
			hSelectedContact = db_get_dw(NULL, modname, "LastContact", INVALID_CONTACT_ID);
			if (hSelectedContact != INVALID_CONTACT_ID)
				Select = 1;
			GetValue(NULL, modname, "LastSetting", SelectedSetting, SIZEOF(SelectedSetting));
		}
		break;

	case 3: // restore from user menu
	case 4: // jump from user menu
		if (hSelectedContact && hSelectedContact != INVALID_CONTACT_ID)
			Select = 1;
		break;
	}

	if ((int)di != 4) { // do not rebuild on just going to another setting
		if (!EnumModules(&modlist)) {
			msg(Translate("Error loading module list"), modFullname);
			return;
		}

		// remove all items (incase there are items there...
		freeTree(hwnd2Tree, 0);
		TreeView_DeleteAllItems(hwnd2Tree);
		TreeView_SelectItem(hwnd2Tree, 0);

		//image list
		TreeView_SetImageList(hwnd2Tree, himl, TVSIL_NORMAL);

		/// contact root item
		contacts_mtis.type = CONTACT_ROOT_ITEM;
		tvi.item.lParam = (LPARAM)&contacts_mtis;
		tvi.hParent = NULL;
		tvi.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.state = TVIS_BOLD;
		tvi.item.stateMask = TVIS_BOLD;
		tvi.item.cChildren = 1;
		tvi.hInsertAfter = TVI_FIRST;

		tvi.item.pszText = Translate("Contacts");
		tvi.item.iImage = 3;
		tvi.item.iSelectedImage = 3;
		contactsRoot = TreeView_InsertItem(hwnd2Tree, &tvi);

		// add the settings item
		settings_mtis.type = STUB;
		tvi.item.lParam = (LPARAM)&settings_mtis;
		tvi.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.cChildren = 1;
		tvi.hParent = NULL;
		tvi.hInsertAfter = TVI_FIRST;
		tvi.item.pszText = Translate("Settings");
		tvi.item.iImage = 0;
		tvi.item.iSelectedImage = 0;
		contact = TreeView_InsertItem(hwnd2Tree, &tvi);

		// to fix bug with CHANGE NOTIFY on window activation
		TreeView_SelectItem(hwnd2Tree, contact);
		settings_mtis.type = CONTACT;

		hContact = 0;
		module = modlist.first;
		while (module) {
			if (!IsModuleEmpty(hContact, module->name)) {
				tvi.hParent = contact;
				tvi.hInsertAfter = TVI_SORT;
				tvi.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
				tvi.item.pszText = module->name;

				lParam = (ModuleTreeInfoStruct *)mir_calloc(sizeof(ModuleTreeInfoStruct));
				lParam->hContact = hContact;
				tvi.item.iImage = 1;
				tvi.item.iSelectedImage = 2;
				lParam->type = KNOWN_MODULE;

				tvi.item.lParam = (LPARAM)lParam;

				TreeView_InsertItem(hwnd2Tree, &tvi);
			}
			module = (ModSetLinkLinkItem *)module->next;
		}

		if (db_get_b(NULL, modname, "ExpandSettingsOnOpen", 0))
			TreeView_Expand(hwnd2Tree, contact, TVE_EXPAND);

		if (Select && hSelectedContact == NULL) {
			int hItem = findItemInTree(hwnd2Tree, hSelectedContact, SelectedModule);
			if (hItem != -1) {
				TreeView_SelectItem(hwnd2Tree, (HTREEITEM)hItem);
				TreeView_Expand(hwnd2Tree, hItem, TVE_EXPAND);
				if (SelectedSetting[0])
					SelectSetting(SelectedSetting);
			}
			Select = 0;
		}

		count = doContacts(hwnd2Tree, contactsRoot, &modlist, Select ? hSelectedContact : NULL, SelectedModule, SelectedSetting);
		Select = 0;
		doItems(hwnd2Tree, &modlist, count);

		FreeModuleSettingLL(&modlist);
	}

	if (Select) {
		int hItem = findItemInTree(hwnd2Tree, hSelectedContact, SelectedModule);
		if (hItem != -1) {
			TreeView_SelectItem(hwnd2Tree, (HTREEITEM)hItem);
			TreeView_Expand(hwnd2Tree, hItem, TVE_EXPAND);
			if (SelectedSetting[0])
				SelectSetting(SelectedSetting);
		}
	}

	populating = 0;

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
	case TVN_ITEMEXPANDINGA:
	case TVN_ITEMEXPANDINGW:
		if (populating && ((LPNMTREEVIEW)lParam)->action == TVE_EXPAND) {
			ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)((LPNMTREEVIEW)lParam)->itemNew.lParam;
			if (mtis && (mtis->type == (CONTACT | EMPTY))) {
				TVINSERTSTRUCT tvi;
				ModuleTreeInfoStruct *_lParam;
				HWND hwnd2Tree = GetDlgItem(hwnd2mainWindow, IDC_MODULES);
				ModSetLinkLinkItem *module;
				ModuleSettingLL modlist;
				MCONTACT hContact = mtis->hContact;

				mtis->type = CONTACT;

				if (!EnumModules(&modlist)) {
					msg(Translate("Error loading module list"), modFullname);
					break;
				}

				module = modlist.first;
				while (module && hwnd2mainWindow) {
					if (module->name[0] && !IsModuleEmpty(hContact, module->name)) {
						tvi.hParent = ((LPNMTREEVIEW)lParam)->itemNew.hItem;
						tvi.hInsertAfter = TVI_SORT;
						tvi.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
						tvi.item.pszText = module->name;

						_lParam = (ModuleTreeInfoStruct *)mir_calloc(sizeof(ModuleTreeInfoStruct));
						_lParam->hContact = hContact;

						tvi.item.iImage = 5;
						tvi.item.iSelectedImage = 6;
						_lParam->type = KNOWN_MODULE;

						tvi.item.lParam = (LPARAM)_lParam;
						TreeView_InsertItem(hwnd2Tree, &tvi);
					}
					module = (ModSetLinkLinkItem *)module->next;
				}

				FreeModuleSettingLL(&modlist);
			}
		}
		break;

	case TVN_SELCHANGEDA:
	case TVN_SELCHANGEDW:
		{
			ModuleTreeInfoStruct *mtis;
			LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
			TVITEM tvi = { 0 };
			char text[264];
			MCONTACT hContact;
			HWND hwnd2Settings = GetDlgItem(hwnd, IDC_SETTINGS);
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
			tvi.hItem = pnmtv->itemNew.hItem;
			tvi.pszText = text;
			tvi.cchTextMax = SIZEOF(text);
			TreeView_GetItem(pnmtv->hdr.hwndFrom, &tvi);

			if (tvi.lParam) {
				mtis = (ModuleTreeInfoStruct *)tvi.lParam;
				hContact = mtis->hContact;

				if (mtis->type == STUB)
					break;

				if (populating)
					Select = 0;

				if (mtis->type == MODULE) {
					SettingListInfo *info = (SettingListInfo *)GetWindowLongPtr(hwnd2Settings, GWLP_USERDATA);
					BOOL refresh = 1;

					if (info) {
						if (info->hContact == hContact && !mir_strcmp(info->module, text))
							refresh = 0;
					}

					if (refresh)
						PopulateSettings(hwnd2Settings, hContact, text);
				}
				else if (((mtis->type & CONTACT) == CONTACT && hContact) || (mtis->type == CONTACT_ROOT_ITEM && !hContact)) {
					char data[32], szProto[256];
					int index, loaded, multi = 0;
					LVITEM lvi = { 0 };
					lvi.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
					lvi.iImage = 6;

					ClearListview(hwnd2Settings);
					SetWindowLongPtr(hwnd2Settings, GWLP_USERDATA, 0);
					if (himl2)
						ListView_SetImageList(hwnd2Settings, himl2, LVSIL_SMALL);

					if (mtis->type == CONTACT_ROOT_ITEM && !hContact) {
						multi = 1;
						hContact = db_find_first();
					}

					while (hContact && hwnd2mainWindow) {
						if (multi) {
							// filter
							if (GetValue(hContact, "Protocol", "p", szProto, SIZEOF(szProto)))
								loaded = IsProtocolLoaded(szProto);
							else
								loaded = 0;

							if ((loaded && Mode == MODE_UNLOADED) || (!loaded && Mode == MODE_LOADED)) {
								hContact = db_find_next(hContact);
								continue;
							}
						}

						lvi.iItem = 0;
						lvi.pszText = (char *)GetContactName(hContact, NULL, FALSE);

						index = ListView_InsertItem(hwnd2Settings, &lvi);

						mir_snprintf(data, SIZEOF(data), "0x%08X (%ld)", hContact, hContact);

						ListView_SetItemText(hwnd2Settings, index, 1, data);
						ListView_SetItemText(hwnd2Settings, index, 2, Translate("HANDLE"));
						ListView_SetItemText(hwnd2Settings, index, 3, "0x0004 (4)");

						if (!multi)
							break;

						hContact = db_find_next(hContact);
					}
				}
				else
					ClearListview(hwnd2Settings);
			}
			else // clear any settings that may be there...
				ClearListview(hwnd2Settings);
		}
		break; //TVN_SELCHANGED:

	case NM_RCLICK:
		if (((NMHDR *)lParam)->code == NM_RCLICK)
			moduleListRightClick(hwnd, wParam, lParam);
		break;

	case TVN_BEGINLABELEDITA: // subclass it..
	case TVN_BEGINLABELEDITW:
		{
			LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)lParam;
			ModuleTreeInfoStruct *mtis = (ModuleTreeInfoStruct *)ptvdi->item.lParam;
			HWND hwnd2Edit = TreeView_GetEditControl(GetDlgItem(hwnd, IDC_MODULES));
			if (!mtis->type || (mtis->type == CONTACT)) {
				SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
				break;
			}
			mir_subclassWindow(hwnd2Edit, ModuleTreeLabelEditSubClassProc);
			SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
		}
		break;

	case TVN_ENDLABELEDITA:
	case TVN_ENDLABELEDITW:
		LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)lParam;
		TVITEM tvi = { 0 };
		char text[264];
		char *newtext;
		ModuleTreeInfoStruct *mtis;
		tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
		tvi.hItem = ptvdi->item.hItem;
		tvi.pszText = text;
		tvi.cchTextMax = SIZEOF(text);
		TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
		mtis = (ModuleTreeInfoStruct *)ptvdi->item.lParam;

		newtext = u2a((WCHAR *)ptvdi->item.pszText);

		if (!newtext // edit control failed
			|| !mtis->type // its a root item
			|| mtis->type == CONTACT // its a contact
			|| *newtext == 0)  // empty string
			SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
		else {
			if (mir_strcmp(tvi.pszText, newtext)) {
				renameModule(tvi.pszText, newtext, mtis->hContact);

				findAndRemoveDuplicates(((LPNMHDR)lParam)->hwndFrom, mtis->hContact, newtext);

				if (TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi)) {
					tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
					tvi.iImage = 1;
					tvi.iSelectedImage = 2;
					TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);

					PopulateSettings(GetDlgItem(hwnd, IDC_SETTINGS), mtis->hContact, newtext);
				}
			}
			SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
		}

		mir_free(newtext);
		break;
	}
}

void moduleListRightClick(HWND hwnd, WPARAM, LPARAM lParam) // hwnd here is to the main window, NOT the treview
{
	TVHITTESTINFO hti;
	hti.pt.x = (short)LOWORD(GetMessagePos());
	hti.pt.y = (short)HIWORD(GetMessagePos());
	ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);

	if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti)) {
		if (hti.flags & TVHT_ONITEM) {
			TVITEM tvi = { 0 };
			HMENU hMenu, hSubMenu;
			int menuNumber;
			char module[256];
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
			tvi.hItem = hti.hItem;
			tvi.pszText = module;
			tvi.cchTextMax = SIZEOF(module);
			TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
			if (tvi.lParam) {
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
					// check if we r already watching the module
					int i = 0, watching = 0;
					// check if the setting is being watched and if it is then check the menu item
					if (WatchListArray.item)
						for (i = 0; i < WatchListArray.count; i++)
							if (WatchListArray.item[i].module && (hContact == WatchListArray.item[i].hContact)) {
								if (!mir_strcmp(module, WatchListArray.item[i].module) && !WatchListArray.item[i].setting) {
									// yes so uncheck it
									CheckMenuItem(hSubMenu, MENU_WATCH_ITEM, MF_CHECKED | MF_BYCOMMAND);
									watching = 1;
									break;
								}
							}

					switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, hti.pt.x, hti.pt.y, 0, hwnd, NULL)) {
					case MENU_RENAME_MOD:
						TreeView_EditLabel(GetDlgItem(hwnd, IDC_MODULES), tvi.hItem);
						break;

					case MENU_DELETE_MOD:
						if (deleteModule(module, hContact, 0)) {
							TreeView_DeleteItem(((LPNMHDR)lParam)->hwndFrom, hti.hItem);
							mir_free(mtis);
						}
						break;

					case MENU_COPY_MOD:
						copyModuleMenuItem(module, hContact);
						break;

						////////////////////////////////////////////////////////////////////// divider
					case MENU_WATCH_ITEM:
						if (!watching)
							addSettingToWatchList(hContact, module, 0);
						else
							freeWatchListItem(i);
						if (hwnd2watchedVarsWindow)
							PopulateWatchedWindow(GetDlgItem(hwnd2watchedVarsWindow, IDC_VARS));
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
							char msg[1024];
							mir_snprintf(msg, SIZEOF(msg), Translate("Are you sure you want to delete contact \"%s\"?"), module);
							if (MessageBox(0, msg, Translate("Confirm contact delete"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
								CallService(MS_DB_CONTACT_DELETE, hContact, 0);
								freeTree(((LPNMHDR)lParam)->hwndFrom, hContact);
								TreeView_DeleteItem(((LPNMHDR)lParam)->hwndFrom, tvi.hItem);
							}
						}
						else {
							CallService(MS_DB_CONTACT_DELETE, hContact, 0);
							freeTree(((LPNMHDR)lParam)->hwndFrom, hContact);
							TreeView_DeleteItem(((LPNMHDR)lParam)->hwndFrom, tvi.hItem);
						}
						break;

						////////////////////////////////////////////////////////////////////// divider
					case MENU_EXPORTCONTACT:
						exportDB(hContact, 0);
						break;
					case MENU_IMPORTFROMTEXT:
						ImportSettingsMenuItem(hContact);
						break;
					case MENU_IMPORTFROMFILE:
						ImportSettingsFromFileMenuItem(hContact, "");
						break;

						////////////////////////////////////////////////////////////////////// divider
					case MENU_ADD_MODULE:
					{
						HWND AddModhwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADD_MODULE), hwnd, AddModDlgProc, hContact);
						char msg[1024];
						mir_snprintf(msg, SIZEOF(msg), Translate("Add module to contact \"%s\""), module);
						SetWindowText(AddModhwnd, module);
					}
					break;
					}
					break;

				case 3: // NULL contact
					switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, hti.pt.x, hti.pt.y, 0, hwnd, NULL)) {
					case MENU_ADD_MODULE:
					{
						HWND AddModhwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADD_MODULE), hwnd, AddModDlgProc, hContact);
						char msg[1024];
						mir_snprintf(msg, SIZEOF(msg), Translate("Add module to contact \"%s\""), module);
						SetWindowText(AddModhwnd, module);
					}
					break;
					case MENU_EXPORTCONTACT:
						exportDB(NULL, 0);
						break;
					case MENU_IMPORTFROMTEXT:
						ImportSettingsMenuItem(NULL);
						break;
					case MENU_IMPORTFROMFILE:
						ImportSettingsFromFileMenuItem(NULL, "");
						break;
					}
					break;

				case 4: // Contacts root
					switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, hti.pt.x, hti.pt.y, 0, hwnd, NULL)) {
					case MENU_EXPORTCONTACT:
						exportDB(INVALID_CONTACT_ID, "");
						break;
					case MENU_IMPORTFROMTEXT:
						ImportSettingsMenuItem(NULL);
						break;
					case MENU_IMPORTFROMFILE:
						ImportSettingsFromFileMenuItem(NULL, "");
						break;
					}
					break;
				}
				DestroyMenu(hMenu);
			}
		}
	}
}
