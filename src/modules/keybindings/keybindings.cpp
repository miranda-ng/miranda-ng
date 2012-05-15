/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"
#include "keybindings.h"
#include "m_keybindings.h"

static HANDLE hKeyBindings_Register;
static HANDLE hKeyBindings_Get;
static WNDPROC OldEditProc;
static TCHAR *keySeparator = _T(" + ");
static TreeItem *currentTreeItem = NULL;
static DWORD tempModifiers;
static DWORD modifiers;
static DWORD virtualKey;

static int addKeyBinding(KEYBINDINGDESC *desc) {
	int i, len;
	TCHAR *sectionName;
	TCHAR *actionName;
	KeyBindingItem *item = (KeyBindingItem *)mir_alloc(sizeof(KeyBindingItem));
	ZeroMemory(item, sizeof(KeyBindingItem));
	if (desc->flags & KBDF_UNICODE) {
		#ifdef _UNICODE
		sectionName = mir_tstrdup(desc->ptszSection);
		actionName = mir_tstrdup(desc->ptszActionName);
		#else
			sectionName = u2a(desc->pwszSection);
			actionName = u2a(desc->pwszActionName);
		#endif
	} else {
		#ifdef _UNICODE
			sectionName = a2u(desc->pszSection);
			actionName = a2u(desc->pszActionName);
		#else
		sectionName = mir_tstrdup(desc->ptszSection);
		actionName = mir_tstrdup(desc->ptszActionName);
		#endif
	}
	len = _tcslen(sectionName) + _tcslen(actionName) + 2;
	item->fullActionName  = (TCHAR *)mir_alloc(len * sizeof(TCHAR));
	_tcscpy(item->fullActionName, sectionName);
	_tcscat(item->fullActionName, _T("/"));
	_tcscat(item->fullActionName, actionName);
	item->actionName = actionName;
	mir_free(sectionName);
	item->actionGroupName = mir_strdup(desc->pszActionGroup);
	item->action = desc->action;
	for (i = 0; i < 5; i++) {
		item->defaultKey[i] = desc->key[i];
		item->key[i] = desc->key[i];
	}

	item->next = keyBindingList;
	keyBindingList = item;
	if (item->next != NULL)
		item->next->prev = item;
	{
		DBVARIANT dbv;
		char *paramName = mir_t2a(item->fullActionName);
		if ( !DBGetContactSettingString(NULL, "KeyBindings", paramName, &dbv )) {
			for (i = 0; i < 5; i++)
				item->key[i] = 0;
			sscanf(dbv.pszVal, "%X,%X,%X,%X,%X", &item->key[0],&item->key[1],&item->key[2],&item->key[3],&item->key[4]);
			DBFreeVariant(&dbv);
		}
		mir_free(paramName);
	}
	return 0;
}

static KeyBindingItem* findKeyBinding(char *actionGroup, DWORD key) 
{
	int i;
	KeyBindingItem *ptr = NULL;
	if (key != 0) {
		for (ptr = keyBindingList; ptr != NULL; ptr = ptr->next)
			if (strcmp(ptr->actionGroupName, actionGroup) == 0)
				for (i = 0; i < 5; i++) 
					if (ptr->key[i] == key) return ptr;
	}
	return ptr;
}

static KeyBindingItem* findTempKeyBinding(char *actionGroup, DWORD key) 
{
	int i;
	KeyBindingItem *ptr = NULL;
	if (key != 0) {
		for (ptr = keyBindingList; ptr != NULL; ptr = ptr->next)
			if (strcmp(ptr->actionGroupName, actionGroup) == 0)
				for (i = 0; i < 5; i++)
					if (ptr->tempKey[i] == key) return ptr;
	}
	return ptr;
}

static void removeTempKeyBinding(char *actionGroup, DWORD key) 
{
	KeyBindingItem *ptr = NULL;
	int i, j;
	if (key != 0) {
		for (ptr = keyBindingList; ptr != NULL; ptr = ptr->next)
			if (strcmp(ptr->actionGroupName, actionGroup) == 0)
				for (i = 0; i < 5; i++) 
					if (ptr->tempKey[i] == key) {
						for (j = i+1; j < 5; j++)
							ptr->tempKey[j-1] = ptr->tempKey[j];
						ptr->tempKey[4] = 0;
					}
	}
}

static void loadTempKeyBinding(KeyBindingItem *item) 
{
	int i;
	for (i = 0; i < 5; i ++)
		item->tempKey[i] = item->key[i];
}

static void loadTempKeyBindings() 
{
	KeyBindingItem *ptr;
	for (ptr = keyBindingList; ptr != NULL; ptr = ptr->next)
		loadTempKeyBinding(ptr);
}

static void saveKeyBinding(KeyBindingItem *item) 
{
	BOOL save = FALSE;
	int i;
	char buff[128];
	for (i = 0; i < 5; i ++) {
		if (item->key[i] != item->tempKey[i]) {
			item->key[i] = item->tempKey[i];
			save = TRUE;
		}
	}
	if (save) {
		char *paramName = mir_t2a(item->fullActionName);
		mir_snprintf(buff, sizeof(buff), "%X,%X,%X,%X,%X", item->key[0],item->key[1],item->key[2],item->key[3],item->key[4]);
		DBWriteContactSettingString(NULL, "KeyBindings", paramName, buff);
		mir_free(paramName);
	}
}

static void saveKeyBindings() 
{
	KeyBindingItem *ptr;
	for (ptr = keyBindingList; ptr != NULL; ptr = ptr->next)
		saveKeyBinding(ptr);
}

static HTREEITEM findNamedTreeItemAt(HWND hwndTree, HTREEITEM hItem, const TCHAR *name)
{
	TVITEM tvi = {0};
	TCHAR str[MAX_PATH];

	if (hItem)
		tvi.hItem = TreeView_GetChild(hwndTree, hItem);
	else
		tvi.hItem = TreeView_GetRoot(hwndTree);

	if (!name)
		return tvi.hItem;

	tvi.mask = TVIF_TEXT;
	tvi.pszText = str;
	tvi.cchTextMax = MAX_PATH;

	while (tvi.hItem)
	{
		TreeView_GetItem(hwndTree, &tvi);
		if (!lstrcmp(tvi.pszText, name))
			return tvi.hItem;

		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return NULL;
}

static void createSettingsTreeNode(HWND hwndTree, KeyBindingItem *keyBindingItem)
{
	TCHAR itemName[1024];
	TCHAR* sectionName;
	int sectionLevel = 0;

	HTREEITEM hSection = NULL;
	lstrcpy(itemName, keyBindingItem->fullActionName);
	sectionName = itemName;

	while (sectionName) {
		HTREEITEM hItem;
		TCHAR* pTranslatedItemName;
		TCHAR* pItemName = sectionName;

		if (sectionName = _tcschr(sectionName, '/')) {
			*sectionName = 0;
		}
		pTranslatedItemName = TranslateTS( pItemName );
		hItem = findNamedTreeItemAt(hwndTree, hSection, pTranslatedItemName);
		if (!sectionName || !hItem) {
			if (!hItem) {
				TVINSERTSTRUCT tvis = {0};
				TreeItem *treeItem = (TreeItem *)mir_alloc(sizeof(TreeItem));
				treeItem->keyBinding = !sectionName ? keyBindingItem : NULL;
				treeItem->paramName = mir_t2a(itemName);
				tvis.hParent = hSection;
				tvis.hInsertAfter = TVI_SORT; //!sectionName ? TVI_LAST : TVI_SORT;
				tvis.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_STATE;
				tvis.item.pszText = pTranslatedItemName;
				tvis.item.lParam = (LPARAM)treeItem;
				tvis.item.state = tvis.item.stateMask = DBGetContactSettingByte(NULL, "KeyBindingsUI", treeItem->paramName, TVIS_EXPANDED );
				hItem = TreeView_InsertItem(hwndTree, &tvis);
			}
		}
		if (sectionName) {
			*sectionName = '/';
			sectionName++;
		}
		sectionLevel++;
		hSection = hItem;
	}
}

static void saveCollapseState( HWND hwndTree )
{
	HTREEITEM hti;
	TVITEM tvi;
	hti = TreeView_GetRoot( hwndTree );
	while( hti != NULL ) {
		HTREEITEM ht;
		tvi.mask = TVIF_STATE | TVIF_HANDLE | TVIF_CHILDREN | TVIF_PARAM;
		tvi.hItem = hti;
		tvi.stateMask = (DWORD)-1;
		TreeView_GetItem( hwndTree, &tvi );
		if( tvi.cChildren > 0 ) {
			TreeItem *treeItem = (TreeItem *)tvi.lParam;
			if ( tvi.state & TVIS_EXPANDED )
				DBWriteContactSettingByte(NULL, "KeyBindingsUI", treeItem->paramName, TVIS_EXPANDED );
			else
				DBWriteContactSettingByte(NULL, "KeyBindingsUI", treeItem->paramName, 0 );
		}
		ht = TreeView_GetChild( hwndTree, hti );
		if( ht == NULL ) {
			ht = TreeView_GetNextSibling( hwndTree, hti );
			while( ht == NULL ) {
				hti = TreeView_GetParent( hwndTree, hti );
				if( hti == NULL ) break;
				ht = TreeView_GetNextSibling( hwndTree, hti );
		}	}
		hti = ht;
}	}


static const TCHAR* getKeyName(DWORD key) {
	static TCHAR keyName[64];
	int nameLen = 0;
	ZeroMemory(keyName, sizeof(keyName));
	if (key & KB_CTRL_FLAG) {
		GetKeyNameText(MAKELPARAM(0, MapVirtualKey(VK_CONTROL, 0)), keyName, 64);
		_tcscat(keyName, keySeparator);
		nameLen = _tcslen(keyName);
	}
	if (key & KB_SHIFT_FLAG) {
		GetKeyNameText(MAKELPARAM(0, MapVirtualKey(VK_SHIFT, 0)), &keyName[nameLen], 64 - nameLen);
		_tcscat(keyName, keySeparator);
		nameLen = _tcslen(keyName);
	}
	if (key & KB_ALT_FLAG) {
		GetKeyNameText(MAKELPARAM(0, MapVirtualKey(VK_MENU, 0)), &keyName[nameLen], 64 - nameLen);
		_tcscat(keyName, keySeparator);
		nameLen = _tcslen(keyName);
	}
	if ((key & 0xFFFF) != 0) {
		DWORD scanCode = MapVirtualKey(key & 0xFFFF, 0);
		switch(key & 0xFFFF) {
		case VK_INSERT:
		case VK_DELETE:
		case VK_HOME:
		case VK_END:
		case VK_NEXT:
		case VK_PRIOR:
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			scanCode |= 0x100; // Add extended bit
		}
		GetKeyNameText(MAKELPARAM(0, scanCode), &keyName[nameLen], 64 - nameLen);
		nameLen = _tcslen(keyName);
	}
	return keyName;
}

static refreshPreview(HWND hwnd) 
{
	TCHAR warning[1024];
	ZeroMemory(warning, sizeof(warning));
	SetWindowText(hwnd, getKeyName(virtualKey | modifiers));
	if (currentTreeItem != NULL && currentTreeItem->keyBinding != NULL) {
		KeyBindingItem *item = findTempKeyBinding(currentTreeItem->keyBinding->actionGroupName, virtualKey | modifiers);
		if (item != NULL)
			mir_sntprintf(warning, 1024, TranslateT("Shortcut already assigned to \"%s\" action.\nIf you click \"Add\" the shortcut will be reassigned."), item->actionName);
	}
	SetDlgItemText(GetParent(hwnd), IDC_MESSAGE, warning);
	EnableWindow(GetDlgItem(GetParent(hwnd), IDC_ADD), virtualKey != 0);
}

static LRESULT CALLBACK KeyBindingsEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_CREATE:
			virtualKey = 0;
			break;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			if (virtualKey != 0) {
				virtualKey = 0;
			}
		    switch (wParam)
			{
				case VK_SHIFT:
					tempModifiers |= KB_SHIFT_FLAG;
					break;
				case VK_CONTROL:
					tempModifiers |= KB_CTRL_FLAG;
					break;
				case VK_MENU:
					tempModifiers |= KB_ALT_FLAG;
					break;
				default:
					virtualKey = wParam;
					break;
			}
			modifiers = tempModifiers;
			refreshPreview(hwnd);
			return 0;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			switch (wParam)
			{
				case VK_SHIFT:
					tempModifiers &= ~KB_SHIFT_FLAG;
					break;
				case VK_CONTROL:
					tempModifiers &= ~KB_CTRL_FLAG;
					break;
				case VK_MENU:
					tempModifiers &= ~KB_ALT_FLAG;
					break;
				default:
					break;
			}
			if (virtualKey == 0) {
				modifiers = tempModifiers;
				refreshPreview(hwnd);
			}
		case WM_CHAR:
		case WM_PASTE:
			return 0;
		case WM_SETFOCUS:
			modifiers = 0;
			tempModifiers = 0;
			virtualKey = 0;
			refreshPreview(hwnd);
			break;
		case WM_GETDLGCODE:
			return DLGC_WANTARROWS|DLGC_WANTALLKEYS| DLGC_WANTTAB;
	}
	return CallWindowProc(OldEditProc, hwnd, msg, wParam, lParam);
}

static void buildTree(HWND hwnd) {
	KeyBindingItem *ptr;
	for (ptr = keyBindingList; ptr != NULL; ptr=ptr->next) {
		createSettingsTreeNode(hwnd, ptr);
	}
}

static void refreshListBox(HWND hwnd) {
	int count = 0;
	BOOL nonDefault = FALSE;
	BOOL canUndo = FALSE;
	SendDlgItemMessage(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
	if (currentTreeItem->keyBinding != NULL) {
		int i;
		for (i=0; i<5; i++) {
			if (currentTreeItem->keyBinding->tempKey[i] != currentTreeItem->keyBinding->defaultKey[i]) nonDefault = TRUE;
			if (currentTreeItem->keyBinding->tempKey[i] != currentTreeItem->keyBinding->key[i]) canUndo = TRUE;
			if (currentTreeItem->keyBinding->tempKey[i] != 0) {
				SendDlgItemMessage(hwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)getKeyName(currentTreeItem->keyBinding->tempKey[i]));
				count++;
			}
		}
	}
	EnableWindow(GetDlgItem(hwnd, IDC_PREVIEW), currentTreeItem->keyBinding != NULL && count < 5);
	SetDlgItemText(hwnd, IDC_PREVIEW, _T(""));
	SetDlgItemText(hwnd, IDC_MESSAGE, _T(""));
	EnableWindow(GetDlgItem(hwnd, IDC_ADD), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_DELETE), FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_BTN_RESET), nonDefault);
	EnableWindow(GetDlgItem(hwnd, IDC_BTN_UNDO), canUndo);
}

BOOL CALLBACK DlgProcKeyBindingsOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			OldEditProc = (WNDPROC) SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_PREVIEW), GWLP_WNDPROC, (LONG_PTR) KeyBindingsEditProc);
			currentTreeItem = NULL;
			loadTempKeyBindings();
			buildTree(GetDlgItem(hwndDlg, IDC_CATEGORYLIST));
			EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
		}
		return TRUE;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		SendDlgItemMessage(hwndDlg, IDC_PREVIEW, msg, wParam, lParam);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ADD:
			if (currentTreeItem->keyBinding != NULL) {
				int i;
				removeTempKeyBinding(currentTreeItem->keyBinding->actionGroupName, modifiers | virtualKey);
				for (i=0; i<5; i++) {
					if (currentTreeItem->keyBinding->tempKey[i] == 0) {
						currentTreeItem->keyBinding->tempKey[i] = modifiers | virtualKey;
						break;
					}
				}
			}
			refreshListBox(hwndDlg);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_DELETE:
			if (currentTreeItem->keyBinding != NULL) {
				int index = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETCURSEL, 0, 0);
				if (index != LB_ERR && index <5) {
					if (currentTreeItem->keyBinding->tempKey[index] != 0) {
						int i;
						for (i = index + 1; i < 5; i++) {
							currentTreeItem->keyBinding->tempKey[i-1] = currentTreeItem->keyBinding->tempKey[i];
						}
						currentTreeItem->keyBinding->tempKey[4] = 0;
					}
				}
			}
			refreshListBox(hwndDlg);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_BTN_RESET:
			if (currentTreeItem->keyBinding != NULL) {
				int i;
				for (i = 0; i < 5; i++) {
					removeTempKeyBinding(currentTreeItem->keyBinding->actionGroupName, currentTreeItem->keyBinding->defaultKey[i]);
					currentTreeItem->keyBinding->tempKey[i] = currentTreeItem->keyBinding->defaultKey[i];
				}
			}
			refreshListBox(hwndDlg);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_BTN_UNDO:
			if (currentTreeItem->keyBinding != NULL) {
				int i;
				for (i = 0; i < 5; i++) {
					removeTempKeyBinding(currentTreeItem->keyBinding->actionGroupName, currentTreeItem->keyBinding->key[i]);
					currentTreeItem->keyBinding->tempKey[i] = currentTreeItem->keyBinding->key[i];
				}
			}
			refreshListBox(hwndDlg);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_LIST:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
			}
			break;
		}
		break;
	case WM_NOTIFY:
		if (((LPNMHDR) lParam)->idFrom == IDC_CATEGORYLIST)
		{
			switch(((NMHDR*)lParam)->code) {
			case TVN_SELCHANGEDA:
			case TVN_SELCHANGEDW:
				{
					TVITEM tvi = {0};
					tvi.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_CATEGORYLIST));
					tvi.mask = TVIF_HANDLE|TVIF_PARAM;
					TreeView_GetItem(GetDlgItem(hwndDlg, IDC_CATEGORYLIST), &tvi);
					currentTreeItem = (TreeItem *) tvi.lParam;
					refreshListBox(hwndDlg);
					break;
				}
			case TVN_DELETEITEMA:
			case TVN_DELETEITEMW:
				{
					TreeItem *treeItem = (TreeItem *)(((LPNMTREEVIEW)lParam)->itemOld.lParam);
					if (treeItem) {
						mir_free(treeItem->paramName);
						mir_free(treeItem);
					}
					break;
				}
			}
		}
		if (((LPNMHDR) lParam)->idFrom == 0 && ((LPNMHDR) lParam)->code == PSN_APPLY ) {
			saveKeyBindings();
		}
		break;
	case WM_DESTROY:
		saveCollapseState(GetDlgItem(hwndDlg, IDC_CATEGORYLIST));

	}
	return FALSE;
}

static INT_PTR KBRegister( WPARAM wParam, LPARAM lParam )
{
	return (int)addKeyBinding(( KEYBINDINGDESC* )lParam );
}

static INT_PTR KBGet( WPARAM wParam, LPARAM lParam )
{
	KEYBINDINGDESC* desc = ( KEYBINDINGDESC* )lParam;
	KeyBindingItem* item = (KeyBindingItem*)findKeyBinding(desc->pszActionGroup, desc->key[0]);
	if (item != NULL) {
		desc->action = item->action;
		return 0;
	}
	return 1;
}

static void InitKeyBinding() 
{
	keyBindingList = NULL;
	hKeyBindings_Register  = CreateServiceFunction(MS_KEYBINDINGS_REGISTER, KBRegister);
	hKeyBindings_Get = CreateServiceFunction(MS_KEYBINDINGS_GET, KBGet);
}

static void UninitKeyBinding() 
{
	KeyBindingItem *ptr, *ptr2;
	ptr = keyBindingList;
	keyBindingList = NULL;
	for (; ptr != NULL; ptr = ptr2) {
		ptr2 = ptr->next;
		mir_free(ptr->actionName);
		mir_free(ptr->fullActionName);
		mir_free(ptr->actionGroupName);
		mir_free(ptr);
	}
	DestroyServiceFunction(hKeyBindings_Register);
	DestroyServiceFunction(hKeyBindings_Get);
}

static int KeyBindingsOptionsInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.hInstance = GetModuleHandle(NULL);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.position = -180000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_KEYBINDINGS);
	odp.ptszTitle = TranslateT("Key Bindings");
	odp.ptszGroup = TranslateT("Customize");
	odp.pfnDlgProc = DlgProcKeyBindingsOpts;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	return 0;
}

static int KeyBindingsSystemModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	HookEvent(ME_OPT_INITIALISE, KeyBindingsOptionsInit);
	return 0;
}

static int OnPreShutdown(WPARAM wParam, LPARAM lParam)
{
	UninitKeyBinding();
	return 0;
}

int LoadKeyBindingsModule( void )
{
	HookEvent(ME_SYSTEM_MODULESLOADED, KeyBindingsSystemModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN,   OnPreShutdown);
	InitKeyBinding();
	return 0;
}
