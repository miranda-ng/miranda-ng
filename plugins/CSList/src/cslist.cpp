/* ========================================================================

Custom Status List
__________________

Custom Status List plugin for Miranda-IM (www.miranda-im.org)
Follower of Custom Status History List by HANAX
Copyright © 2006-2008 HANAX
Copyright © 2007-2009 jarvis
Occasionally rewritten in 2012 by George Hazan

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
======================================================================== */

#include "stdafx.h"
#include "strpos.h"

CMPlugin g_plugin;

static LIST<CSWindow> arWindows(3, HandleKeySortT);

//====[ PLUGIN INFO ]========================================================

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {C8CC7414-6507-4AF6-925A-83C1D2F7BE8C}
	{ 0xc8cc7414, 0x6507, 0x4af6, { 0x92, 0x5a, 0x83, 0xc1, 0xd2, 0xf7, 0xbe, 0x8c } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODNAME, pluginInfoEx)
{
}

//====[ LOADER ]=============================================================

static int OnDbChanged(WPARAM hContact, LPARAM lparam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lparam;

	// if user changes his UIN or JID on any account
	if (hContact == NULL) {
		const char *szUniqueID = Proto_GetUniqueId(cws->szModule);
		if (!mir_strcmp(cws->szSetting, szUniqueID))
			Menu_ReloadProtoMenus();
	}
	return 0;
}

static int OnInitOptions(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 955000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc = CSOptionsProc;
	odp.szGroup.w = L"Status";
	odp.szTitle.w = MODULENAME;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	g_plugin.addOptions(wparam, &odp);
	return 0;
}

static int OnCreateMenuItems(WPARAM, LPARAM)
{
	for (auto &pa : Accounts())
		if (ProtoServiceExists(pa->szModuleName, PS_SETCUSTOMSTATUSEX))
			addProtoStatusMenuItem(pa->szModuleName);

	return 0;
}

static int OnPreshutdown(WPARAM, LPARAM)
{
	for (auto &it : arWindows)
		DestroyWindow(it->m_handle);
	return 0;
}

int CMPlugin::Load()
{
	// support for ComboBoxEx
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icc);

	// init icons
	wchar_t tszFile[MAX_PATH];
	GetModuleFileName(g_plugin.getInst(), tszFile, MAX_PATH);

	SKINICONDESC sid = {};
	sid.defaultFile.w = tszFile;
	sid.flags = SIDF_ALL_UNICODE;
	sid.section.w = MODULENAME;

	for (int i = 0; i < _countof(forms); i++) {
		char szSettingName[64];
		mir_snprintf(szSettingName, "%s_%s", MODNAME, forms[i].pszIconIcoLib);

		sid.pszName = szSettingName;
		sid.description.w = forms[i].ptszDescr;
		sid.iDefaultIndex = -forms[i].iconNoIcoLib;
		forms[i].hIcoLibItem = g_plugin.addIcon(&sid);
	}

	HookEvent(ME_OPT_INITIALISE, OnInitOptions);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnDbChanged);
	HookEvent(ME_CLIST_PREBUILDSTATUSMENU, OnCreateMenuItems);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreshutdown);

	// if we load a plugin dynamically, it will work, otherwise fail
	OnCreateMenuItems(0, 0);
	return 0;
}

//====[ FUN ]================================================================

void RegisterHotkeys(char buf[200], wchar_t* accName, int Number)
{
	HOTKEYDESC hotkey = {};
	hotkey.dwFlags = HKD_UNICODE;
	hotkey.pszName = buf;
	hotkey.szDescription.w = accName;
	hotkey.szSection.w = LPGENW("Custom Status List");
	hotkey.pszService = buf;
	hotkey.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, '0' + Number);
	g_plugin.addHotkey(&hotkey);
}

void SetStatus(uint16_t code, StatusItem* item, char *szAccName)
{
	if (code == IDCLOSE)
		return;

	PROTOACCOUNT* pdescr = Proto_GetAccount(szAccName);
	if (pdescr == nullptr)
		return;

	if (!ProtoServiceExists(szAccName, PS_SETCUSTOMSTATUSEX))
		return;

	int statusToSet;

	CUSTOM_STATUS ics = { sizeof(CUSTOM_STATUS) };
	ics.flags = CSSF_MASK_STATUS | CSSF_MASK_NAME | CSSF_MASK_MESSAGE | CSSF_UNICODE;

	if (code == IDC_CANCEL) {
		statusToSet = 0;
		ics.ptszName = L"";
		ics.ptszMessage = L"";
	}
	else if (code == IDOK && item != nullptr) {
		statusToSet = item->m_iIcon + 1;
		ics.ptszName = variables_parsedup(item->m_tszTitle, nullptr, NULL);
		ics.ptszMessage = variables_parsedup(item->m_tszMessage, nullptr, NULL);
	}
	else return;

	ics.status = &statusToSet;
	CallProtoService(szAccName, PS_SETCUSTOMSTATUSEX, 0, (LPARAM)&ics);
}

INT_PTR showList(WPARAM, LPARAM, LPARAM param)
{
	char* szProto = (char*)param;
	for (auto &p : arWindows) {
		if (!mir_strcmp(szProto, p->m_protoName)) {
			ShowWindow(p->m_handle, SW_SHOW);
			SetForegroundWindow(p->m_handle);
			return 1;
		}
	}

	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CSLIST), nullptr, CSWindowProc, (LPARAM)new CSWindow(szProto));
	return 0;
}

void addProtoStatusMenuItem(char *protoName)
{
	PROTOACCOUNT *pdescr = Proto_GetAccount(protoName);
	if (pdescr == nullptr || pdescr->ppro == nullptr)
		return;

	HGENMENU hRoot = Menu_GetProtocolRoot(pdescr->ppro);
	if (hRoot == nullptr)
		return;

	char buf[200];
	mir_snprintf(buf, "CSList/ShowList/%s", protoName);
	if (!ServiceExists(buf))
		CreateServiceFunctionParam(buf, showList, (LPARAM)protoName);

	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = forms[0].hIcoLibItem;
	mi.name.w = MODULENAME;
	mi.position = 2000040000;
	mi.pszService = buf;
	mi.root = hRoot;
	Menu_AddStatusMenuItem(&mi);

	RegisterHotkeys(buf, pdescr->tszAccountName, pdescr->iOrder);
}

///////////////////////////////////////////////////////////////////////////////

void importCustomStatuses(CSWindow* csw, int result)
{
	DBVARIANT dbv;
	char bufTitle[32], bufMessage[32], *protoName = csw->m_protoName;

	for (int i = 0; i < csw->m_statusCount; i++) {
		StatusItem* si = new StatusItem();
		si->m_iIcon = i - 1;

		mir_snprintf(bufTitle, "XStatus%dName", i);
		if (!db_get_ws(0, protoName, bufTitle, &dbv)) {
			mir_wstrcpy(si->m_tszTitle, dbv.pwszVal);
			db_free(&dbv);
		}
		else si->m_tszTitle[0] = 0;

		mir_snprintf(bufMessage, "XStatus%dMsg", i);
		if (!db_get_ws(0, protoName, bufMessage, &dbv)) {
			mir_wstrcpy(si->m_tszMessage, dbv.pwszVal);
			db_free(&dbv);
		}
		else si->m_tszMessage[0] = 0;

		if (si->m_tszTitle[0] || si->m_tszMessage[0]) {
			csw->m_itemslist->m_list->add(si);
			csw->m_bSomethingChanged = TRUE;
		}
		else delete si;

		if (result == IDYES) {
			db_unset(0, protoName, bufTitle);
			db_unset(0, protoName, bufMessage);
		}
	}
	csw->m_listview->reinitItems(csw->m_itemslist->m_list->getListHead());
}

///////////////////////////////////////////////////////////////////////////////
// CSWindow class

CSWindow::CSWindow(char *protoName)
{
	m_protoName = protoName;
	m_handle = nullptr;
	m_bExtraIcons = g_plugin.getByte("AllowExtraIcons", DEFAULT_ALLOW_EXTRA_ICONS);
	m_itemslist = new CSItemsList(m_protoName);
	m_listview = nullptr;
	m_addModifyDlg = nullptr;
	m_bSomethingChanged = FALSE;
	m_filterString = nullptr;
}

void __fastcall SAFE_FREE(void** p)
{
	if (*p) {
		free(*p);
		*p = nullptr;
	}
}

CSWindow::~CSWindow()
{
	delete m_itemslist;
	SAFE_FREE((void**)&m_filterString);
}

void CSWindow::initIcons()
{
	PROTOACCOUNT *pdescr = Proto_GetAccount(m_protoName);
	if (pdescr == nullptr)
		return;

	const char *szUniqueID = Proto_GetUniqueId(m_protoName);
	if (szUniqueID == nullptr)
		return;

	DBVARIANT dbv;
	if (db_get(NULL, pdescr->szModuleName, szUniqueID, &dbv))
		return;
	db_free(&dbv);

	WPARAM iNumStatuses = 0;
	CUSTOM_STATUS cs = { sizeof(cs) };
	cs.flags = CSSF_STATUSES_COUNT;
	cs.wParam = &iNumStatuses;
	if (CallProtoService(pdescr->szModuleName, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&cs) != 0)
		return;

	m_statusCount = (int)iNumStatuses;
	if (nullptr == (m_icons = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, m_statusCount, 0)))
		return;

	for (int i = 1; i <= m_statusCount; i++) {
		HICON hIcon = (HICON)CallProtoService(pdescr->szModuleName, PS_GETCUSTOMSTATUSICON, i, 0);
		if (hIcon) {
			ImageList_AddIcon(m_icons, hIcon);
			DestroyIcon(hIcon);
		}
	}
}

void CSWindow::deinitIcons()
{
	ImageList_Destroy(m_icons);
}

void CSWindow::initButtons()
{
	for (int i = 0; i < _countof(forms); i++) {
		if (forms[i].idc < 0)
			continue;

		SendDlgItemMessage(m_handle, forms[i].idc, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(forms[i].hIcoLibItem));
		SendDlgItemMessage(m_handle, forms[i].idc, BUTTONSETASFLATBTN, TRUE, 0); //maybe set as BUTTONSETDEFAULT?
		SendDlgItemMessage(m_handle, forms[i].idc, BUTTONADDTOOLTIP, (WPARAM)TranslateW(forms[i].ptszTitle), BATF_UNICODE);
	}
}

void CSWindow::loadWindowPosition()
{
	if (g_plugin.getByte("RememberWindowPosition", DEFAULT_REMEMBER_WINDOW_POSITION))
		Utils_RestoreWindowPosition(m_handle, NULL, MODNAME, "Position");
}

void CSWindow::toggleEmptyListMessage()
{
	HWND hwnd = GetDlgItem(m_handle, IDC_NO_ITEMS);
	ShowWindow(hwnd, (ListView_GetItemCount(m_listview->m_handle) > 0) ? FALSE : TRUE);
	SetForegroundWindow(hwnd);
	hwnd = GetDlgItem(m_handle, IDC_ADD_SAMPLE);
	ShowWindow(hwnd, (ListView_GetItemCount(m_listview->m_handle) > 0) ? FALSE : TRUE);
	SetForegroundWindow(hwnd);
}

BOOL CSWindow::itemPassedFilter(ListItem< StatusItem >* li)
{
	wchar_t filter[MAX_PATH];
	GetDlgItemText(m_handle, IDC_FILTER_FIELD, filter, _countof(filter));

	if (mir_wstrlen(filter)) {
		wchar_t title[EXTRASTATUS_TITLE_LIMIT], message[EXTRASTATUS_MESSAGE_LIMIT];
		mir_wstrcpy(title, li->m_item->m_tszTitle); mir_wstrcpy(message, li->m_item->m_tszMessage);
		if (strpos(wcslwr(title), wcslwr(filter)) == -1)
			if (strpos(wcslwr(message), wcslwr(filter)) == -1)
				return FALSE;
	}

	return TRUE;
}

void CSWindow::toggleFilter()
{
	HWND hFilter = GetDlgItem(m_handle, IDC_FILTER_FIELD);
	BOOL isEnabled = !IsWindowEnabled(hFilter) ? TRUE : FALSE; // ! = + isEnabled = !isEnabled in one
	EnableWindow(hFilter, isEnabled);
	ShowWindow(hFilter, isEnabled);
	CheckDlgButton(m_handle, IDC_FILTER, isEnabled ? BST_CHECKED : BST_UNCHECKED);
	SetForegroundWindow(hFilter);
	if (isEnabled)
		SetFocus(hFilter);
	else {
		wchar_t filterText[255];
		GetDlgItemText(m_handle, IDC_FILTER_FIELD, filterText, _countof(filterText));
		if (filterText[0] != 0)
			SetDlgItemText(m_handle, IDC_FILTER_FIELD, TEXT(""));
	}
}

BOOL CSWindow::toggleButtons()
{
	int selection = ListView_GetSelectedItemMacro(m_listview->m_handle);
	BOOL validSelection = (selection >= 0 && (unsigned int)selection < m_itemslist->m_list->getCount()) ? TRUE : FALSE;
	BOOL filterEnabled = IsWindowVisible(GetDlgItem(m_handle, IDC_FILTER_FIELD));
	BOOL somethingChanged = m_bSomethingChanged;

	EnableWindow(GetDlgItem(m_handle, IDC_ADD), !filterEnabled);
	EnableWindow(GetDlgItem(m_handle, IDC_MODIFY), validSelection && !filterEnabled);
	EnableWindow(GetDlgItem(m_handle, IDC_REMOVE), validSelection && !filterEnabled);
	EnableWindow(GetDlgItem(m_handle, IDC_FAVOURITE), validSelection && !filterEnabled);
	EnableWindow(GetDlgItem(m_handle, IDC_UNDO), somethingChanged && !filterEnabled);
	EnableWindow(GetDlgItem(m_handle, IDC_IMPORT), !filterEnabled);
	EnableWindow(GetDlgItem(m_handle, IDOK), validSelection);

	return validSelection;
}

///////////////////////////////////////////////////////////////////////////////
// CSAMWindow class - add form window

CSAMWindow::CSAMWindow(uint16_t action, CSWindow* parent)
{
	m_action = action;
	m_parent = parent;
	m_bChanged = FALSE;
	m_hCombo = m_hMessage = nullptr;

	if (m_action == IDC_ADD)
		m_item = new StatusItem();
	else
		m_item = new StatusItem(*m_parent->m_itemslist->m_list->get(m_parent->m_listview->getPositionInList()));
}

CSAMWindow::~CSAMWindow()
{
	if (!m_bChanged)
		delete m_item;
}

void CSAMWindow::exec()
{
	DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ADDMODIFY), nullptr, CSAMWindowProc, (LPARAM)this);
}


void CSAMWindow::setCombo()
{
	PROTOACCOUNT *pdescr = Proto_GetAccount(m_parent->m_protoName);
	if (pdescr == nullptr)
		return;

	const char *szUniqueID = Proto_GetUniqueId(pdescr->szModuleName);
	if (szUniqueID == nullptr)
		return;

	DBVARIANT dbv;
	if (db_get(NULL, pdescr->szModuleName, szUniqueID, &dbv))
		return;
	db_free(&dbv);

	WPARAM iStatus;
	wchar_t tszName[100];
	CUSTOM_STATUS cs = { sizeof(cs) };
	cs.flags = CSSF_MASK_NAME | CSSF_DEFAULT_NAME | CSSF_UNICODE;
	cs.ptszName = tszName;
	cs.wParam = &iStatus;

	SendMessage(m_hCombo, CBEM_SETIMAGELIST, 0, (LPARAM)m_parent->m_icons);
	for (int i = 1; i <= m_parent->m_statusCount; i++) {
		iStatus = i;
		if (CallProtoService(pdescr->szModuleName, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&cs) != 0)
			continue;

		COMBOBOXEXITEM cbi = { 0 };
		cbi.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
		cbi.iItem = -1;
		cbi.iImage = cbi.iSelectedImage = i - 1;
		cbi.pszText = TranslateW(tszName);
		SendMessage(m_hCombo, CBEM_INSERTITEM, 0, (LPARAM)&cbi);
	}
	SendMessage(m_hCombo, CB_SETCURSEL, 0, 0); // first 0 sets selection to top
}

void CSAMWindow::fillDialog()
{
	if (m_action == IDC_ADD) {
		SetWindowText(m_handle, TranslateT("Add new item"));
		SetDlgItemText(m_handle, IDOK, TranslateT("Add"));
	}
	else {
		SetWindowText(m_handle, TranslateT("Modify item"));
		SetDlgItemText(m_handle, IDOK, TranslateT("Modify"));
	}

	SendMessage(m_hCombo, CB_SETCURSEL, m_item->m_iIcon, 0);
	SetDlgItemText(m_handle, IDC_MESSAGE, m_item->m_tszMessage);
}

void CSAMWindow::checkFieldLimit(uint16_t action, uint16_t item)
{
	BOOL type = (item == IDC_MESSAGE) ? TRUE : FALSE;
	unsigned int limit = type ? EXTRASTATUS_MESSAGE_LIMIT : EXTRASTATUS_TITLE_LIMIT;

	if (action == EN_CHANGE) {
		wchar_t* ptszInputText = (wchar_t*)mir_alloc((limit + 8) * sizeof(wchar_t));

		GetDlgItemText(m_handle, item, ptszInputText, limit + 8);

		if (mir_wstrlen(ptszInputText) > limit) {
			wchar_t tszPopupTip[MAX_PATH];
			EDITBALLOONTIP ebt = { 0 };
			ebt.cbStruct = sizeof(ebt);
			ebt.pszTitle = TranslateT("Warning");
			mir_snwprintf(tszPopupTip, TranslateT("This field doesn't accept string longer than %d characters. The string will be truncated."), limit);
			ebt.pszText = tszPopupTip;
			ebt.ttiIcon = TTI_WARNING;
			SendDlgItemMessage(m_handle, item, EM_SHOWBALLOONTIP, 0, (LPARAM)&ebt);

			wchar_t* ptszOutputText = (wchar_t*)mir_alloc((limit + 1) * sizeof(wchar_t));
			GetDlgItemText(m_handle, item, ptszOutputText, limit + 1);
			SetDlgItemText(m_handle, item, ptszOutputText);
			mir_free(ptszOutputText);
		}
		mir_free(ptszInputText);
	}
}

void CSAMWindow::checkItemValidity()
{
	COMBOBOXEXITEM cbi = { 0 };
	cbi.mask = CBEIF_IMAGE;
	cbi.iItem = SendDlgItemMessage(m_handle, IDC_COMBO, CB_GETCURSEL, 0, 0);
	SendDlgItemMessage(m_handle, IDC_COMBO, CBEM_GETITEM, 0, (LPARAM)&cbi);

	if (m_item->m_iIcon != cbi.iImage)
		m_item->m_iIcon = cbi.iImage, m_bChanged = TRUE;

	wchar_t tszInputMessage[EXTRASTATUS_MESSAGE_LIMIT];

	GetDlgItemText(m_handle, IDC_MESSAGE, tszInputMessage, _countof(tszInputMessage));

	PROTOACCOUNT *pdescr = Proto_GetAccount(m_parent->m_protoName);
	if (pdescr == nullptr)
		return;

	WPARAM i = SendMessage(m_hCombo, CB_GETCURSEL, 0, 0) + 1;
	wchar_t tszTitle[100];

	CUSTOM_STATUS cs = { sizeof(cs) };
	cs.flags = CSSF_MASK_NAME | CSSF_DEFAULT_NAME | CSSF_UNICODE;
	cs.ptszName = tszTitle;
	cs.wParam = &i;
	if (CallProtoService(pdescr->szModuleName, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&cs) == 0)
		mir_wstrncpy(m_item->m_tszTitle, TranslateW(tszTitle), _countof(m_item->m_tszTitle));

	if (mir_wstrcmp(m_item->m_tszMessage, tszInputMessage))
		mir_wstrcpy(m_item->m_tszMessage, tszInputMessage), m_bChanged = true;
}

CSListView::CSListView(HWND hwnd, CSWindow* parent)
{
	m_handle = hwnd;
	m_parent = parent;

	LVGROUP lg = { 0 };
	lg.cbSize = sizeof(LVGROUP);
	lg.mask = LVGF_HEADER | LVGF_GROUPID;

	lg.pszHeader = TranslateT("Favorites");
	lg.iGroupId = 0;
	ListView_InsertGroup(m_handle, -1, &lg);

	lg.pszHeader = TranslateT("Regular statuses");
	lg.iGroupId = 1;
	ListView_InsertGroup(m_handle, -1, &lg);
	ListView_EnableGroupView(m_handle, TRUE);

	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 0x00;
	lvc.pszText = TEXT("");
	lvc.cx = 0x16;
	SendMessage(m_handle, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
	lvc.pszText = TranslateT("Title");
	lvc.cx = 0x64;
	SendMessage(m_handle, LVM_INSERTCOLUMN, 1, (LPARAM)&lvc);
	lvc.pszText = TranslateT("Message");
	lvc.cx = 0xa8;
	SendMessage(m_handle, LVM_INSERTCOLUMN, 2, (LPARAM)&lvc);

	ListView_SetExtendedListViewStyleEx(m_handle, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP);
	ListView_SetImageList(m_handle, m_parent->m_icons, LVSIL_SMALL);
}

void CSListView::addItem(StatusItem* item, int itemNumber)
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_IMAGE | LVIF_GROUPID | LVIF_PARAM;
	lvi.iItem = ListView_GetItemCount(m_handle);
	lvi.lParam = itemNumber;
	lvi.iGroupId = item->m_bFavourite ? 0 : 1;

	// first column
	lvi.iSubItem = 0;
	lvi.iImage = item->m_iIcon; // use selected xicon
	SendMessage(m_handle, LVM_INSERTITEM, 0, (LPARAM)&lvi);

	// second column
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = 1;
	lvi.pszText = item->m_tszTitle;
	SendMessage(m_handle, LVM_SETITEM, 0, (LPARAM)&lvi);

	// third column
	lvi.iSubItem = 2;
	lvi.pszText = item->m_tszMessage;
	SendMessage(m_handle, LVM_SETITEM, 0, (LPARAM)&lvi);
}

void CSListView::initItems(ListItem< StatusItem >* items)
{
	ListItem< StatusItem >* help = items;
	for (int i = 0; help != nullptr; help = help->m_next, i++)
		if (m_parent->itemPassedFilter(help))
			addItem(help->m_item, i);
}

void CSListView::reinitItems(ListItem< StatusItem >* items)
{
	EnableWindow(m_handle, FALSE);
	removeItems();
	initItems(items);
	EnableWindow(m_handle, TRUE);
}

void CSListView::removeItems()
{
	ListView_DeleteAllItems(m_handle);
}

int CSListView::getPositionInList()
{
	LVITEM lvi = { 0 };
	lvi.iItem = ListView_GetSelectedItemMacro(m_parent->m_listview->m_handle);
	lvi.iSubItem = 0;
	lvi.mask = LVIF_PARAM;
	ListView_GetItem(m_parent->m_listview->m_handle, &lvi);
	return lvi.lParam;
}

void CSListView::setFullFocusedSelection(int selection)
{
	ListView_SetItemState(m_handle, -1, 0, LVIS_SELECTED);
	ListView_EnsureVisible(m_handle, selection, FALSE);
	ListView_SetItemState(m_handle, selection, LVIS_SELECTED, LVIS_SELECTED);
	ListView_SetItemState(m_handle, selection, LVIS_FOCUSED, LVIS_FOCUSED);
	SetFocus(m_handle);
}

//====[ LIST MANAGEMENT ]====================================================

CSItemsList::CSItemsList(char *protoName)
{
	m_list = new List< StatusItem >(compareItems);
	loadItems(protoName);
}

CSItemsList::~CSItemsList()
{
	delete m_list;
}

int CSItemsList::compareItems(const StatusItem* p1, const StatusItem* p2)
{
	int favRes = 0, icoRes = 0, ttlRes = 0, msgRes = 0;

	if (p1->m_bFavourite < p2->m_bFavourite)
		favRes = 1;
	else if (p1->m_bFavourite > p2->m_bFavourite)
		favRes = -1;

	int result;
	if (p1->m_iIcon > p2->m_iIcon)
		icoRes = 1;
	else if (p1->m_iIcon < p2->m_iIcon)
		icoRes = -1;

	result = mir_wstrcmp(p1->m_tszTitle, p2->m_tszTitle);
	ttlRes = result;

	result = mir_wstrcmp(p1->m_tszMessage, p2->m_tszMessage);
	msgRes = result;

	if (!icoRes && !ttlRes && !msgRes)
		return 0;

	if (favRes != 0)
		return favRes;
	if (icoRes != 0)
		return icoRes;
	if (ttlRes != 0)
		return ttlRes;
	if (msgRes != 0)
		return msgRes;

	return 0;
}

void CSItemsList::loadItems(char *protoName)
{
	char dbSetting[32];
	mir_snprintf(dbSetting, "%s_ItemsCount", protoName);
	unsigned int itemsCount = g_plugin.getWord(dbSetting, DEFAULT_ITEMS_COUNT);

	for (unsigned int i = 1; i <= itemsCount; i++) {
		StatusItem* item = new StatusItem();
		DBVARIANT dbv;
		mir_snprintf(dbSetting, "%s_Item%dIcon", protoName, i);
		item->m_iIcon = g_plugin.getByte(dbSetting, DEFAULT_ITEM_ICON);

		mir_snprintf(dbSetting, "%s_Item%dTitle", protoName, i);
		if (!g_plugin.getWString(dbSetting, &dbv)) {
			mir_wstrcpy(item->m_tszTitle, dbv.pwszVal);
			db_free(&dbv);
		}
		else item->m_tszTitle[0] = 0;

		mir_snprintf(dbSetting, "%s_Item%dMessage", protoName, i);
		if (!g_plugin.getWString(dbSetting, &dbv)) {
			mir_wstrcpy(item->m_tszMessage, dbv.pwszVal);
			db_free(&dbv);
		}
		else item->m_tszMessage[0] = 0;

		mir_snprintf(dbSetting, "%s_Item%dFavourite", protoName, i);
		item->m_bFavourite = (BOOL)g_plugin.getByte(dbSetting, DEFAULT_ITEM_IS_FAVOURITE);

		m_list->add(item);
	}
}

void CSItemsList::saveItems(char *protoName)
{
	unsigned int i;
	char dbSetting[32];
	mir_snprintf(dbSetting, "%s_ItemsCount", protoName);
	unsigned int oldItemsCount = g_plugin.getWord(dbSetting, DEFAULT_ITEMS_COUNT);

	for (i = 1; i <= m_list->getCount(); i++) {
		StatusItem *item = m_list->get(i - 1);
		mir_snprintf(dbSetting, "%s_Item%dIcon", protoName, i);
		g_plugin.setByte(dbSetting, item->m_iIcon);
		mir_snprintf(dbSetting, "%s_Item%dTitle", protoName, i);
		g_plugin.setWString(dbSetting, item->m_tszTitle);
		mir_snprintf(dbSetting, "%s_Item%dMessage", protoName, i);
		g_plugin.setWString(dbSetting, item->m_tszMessage);
		mir_snprintf(dbSetting, "%s_Item%dFavourite", protoName, i);
		g_plugin.setByte(dbSetting, item->m_bFavourite);
	}

	mir_snprintf(dbSetting, "%s_ItemsCount", protoName);
	g_plugin.setWord(dbSetting, m_list->getCount());

	for (; i <= oldItemsCount; i++) {
		mir_snprintf(dbSetting, "%s_Item%dIcon", protoName, i);
		g_plugin.delSetting(dbSetting);
		mir_snprintf(dbSetting, "%s_Item%dTitle", protoName, i);
		g_plugin.delSetting(dbSetting);
		mir_snprintf(dbSetting, "%s_Item%dMessage", protoName, i);
		g_plugin.delSetting(dbSetting);
		mir_snprintf(dbSetting, "%s_Item%dFavourite", protoName, i);
		g_plugin.delSetting(dbSetting);
	}
}


//====[ PROCEDURES ]=========================================================

INT_PTR CALLBACK CSWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	CSWindow* csw = (CSWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		csw = (CSWindow*)lparam;
		arWindows.insert(csw);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);

		csw->m_handle = hwnd;
		csw->initIcons();
		csw->initButtons();
		csw->m_listview = new CSListView(GetDlgItem(hwnd, IDC_CSLIST), csw);
		csw->m_listview->initItems(csw->m_itemslist->m_list->getListHead());
		csw->toggleButtons();
		csw->toggleEmptyListMessage();
		csw->loadWindowPosition();
		SetWindowText(hwnd, MODULENAME);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDC_MODIFY:
		case IDC_ADD:
			csw->m_addModifyDlg = new CSAMWindow(LOWORD(wparam), csw);
			csw->m_addModifyDlg->exec();
			if (csw->m_addModifyDlg->m_bChanged) {
				if (LOWORD(wparam) == IDC_MODIFY)
					csw->m_itemslist->m_list->remove(csw->m_listview->getPositionInList());

				int selection = csw->m_itemslist->m_list->add(csw->m_addModifyDlg->m_item);
				csw->m_bSomethingChanged = TRUE;
				csw->m_listview->reinitItems(csw->m_itemslist->m_list->getListHead());
				csw->m_listview->setFullFocusedSelection(selection);
				csw->toggleButtons();
				csw->toggleEmptyListMessage();
			}
			delete csw->m_addModifyDlg;
			break;

		case IDC_REMOVE:
			if (g_plugin.getByte("ConfirmDeletion", DEFAULT_PLUGIN_CONFIRM_ITEMS_DELETION))
				if (MessageBox(hwnd, TranslateT("Do you really want to delete selected item?"), TranslateW(MODULENAME), MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDNO)
					break;

			csw->m_itemslist->m_list->remove(csw->m_listview->getPositionInList());
			csw->m_bSomethingChanged = TRUE;
			csw->m_listview->reinitItems(csw->m_itemslist->m_list->getListHead());
			csw->toggleButtons();
			csw->toggleEmptyListMessage();
			break;

		case IDC_FAVOURITE:
			{
				int selection = csw->m_listview->getPositionInList();
				StatusItem* f = new StatusItem(*csw->m_itemslist->m_list->get(selection));
				f->m_bFavourite = !f->m_bFavourite;
				csw->m_itemslist->m_list->remove(selection);
				selection = csw->m_itemslist->m_list->add(f);
				csw->m_bSomethingChanged = TRUE;
				csw->m_listview->reinitItems(csw->m_itemslist->m_list->getListHead());
				csw->m_listview->setFullFocusedSelection(selection);
				csw->toggleButtons();
			}
			break;

		case IDC_UNDO:
			csw->m_itemslist->m_list->destroy();
			csw->m_itemslist->loadItems(csw->m_protoName);
			csw->m_bSomethingChanged = FALSE;
			csw->m_listview->reinitItems(csw->m_itemslist->m_list->getListHead());
			csw->toggleButtons();
			csw->toggleEmptyListMessage();
			break;

		case IDC_IMPORT:
			{
				int result = g_plugin.getByte("DeleteAfterImport", DEFAULT_PLUGIN_DELETE_AFTER_IMPORT);
				if (result == TRUE)
					result = IDYES;
				else {
					result = MessageBox(hwnd,
						TranslateT("Do you want old database entries to be deleted after Import?"),
						TranslateW(MODULENAME), MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONQUESTION);
					if (result == IDCANCEL)
						break;
				}

				importCustomStatuses(csw, result);
				csw->m_bSomethingChanged = TRUE;
				csw->toggleButtons();
				csw->toggleEmptyListMessage();
			}
			break;

		case IDC_FILTER:
			csw->toggleFilter();
			csw->toggleButtons();
			break;

		case IDC_FILTER_FIELD:
			if (HIWORD(wparam) == EN_CHANGE)
				csw->m_listview->reinitItems(csw->m_itemslist->m_list->getListHead());
			break;

		case IDCLOSE:    // close and save, no custom status
		case IDCANCEL:   // close and save, no custom status
		case IDC_CANCEL:   // close and save, cancel custom status
		case IDOK:       // close and save, set selected custom status
			if (LOWORD(wparam) == IDOK && csw->toggleButtons())
				SetStatus(IDOK, csw->m_itemslist->m_list->get(csw->m_listview->getPositionInList()), csw->m_protoName);
			if (LOWORD(wparam) == IDC_CANCEL)
				SetStatus(IDC_CANCEL, nullptr, csw->m_protoName);
			if (csw->m_bSomethingChanged)
				csw->m_itemslist->saveItems(csw->m_protoName);
			csw->saveWindowPosition(csw->m_handle);
			EndDialog(hwnd, FALSE);
			break;
		}
		return FALSE;

	case WM_NOTIFY:
		if (wparam == IDC_CSLIST) {
			NMHDR* pnmh = (NMHDR*)lparam;
			switch (pnmh->code) {
			case NM_DBLCLK:
				PostMessage(hwnd, WM_COMMAND, IDOK, 0L);
				break;

			case LVN_ITEMCHANGED:
			case NM_CLICK:
				csw->toggleButtons();
				break;
			}
		}
		return FALSE;

	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wparam, RGB(174, 174, 174));
		if (((HWND)lparam == GetDlgItem(hwnd, IDC_NO_ITEMS)) || ((HWND)lparam == GetDlgItem(hwnd, IDC_ADD_SAMPLE)))
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
		return NULL;

	case WM_DESTROY:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		arWindows.remove(csw);
		delete csw->m_listview;
		csw->deinitIcons();
		delete csw;
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK CSAMWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	CSAMWindow *csamw = (CSAMWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (message) {
	case WM_INITDIALOG:
		csamw = (CSAMWindow*)lparam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);
		csamw->m_handle = hwnd;
		EnableWindow(csamw->m_parent->m_handle, FALSE);
		csamw->m_hCombo = GetDlgItem(hwnd, IDC_COMBO);
		csamw->m_hMessage = GetDlgItem(hwnd, IDC_MESSAGE);
		csamw->setCombo();
		csamw->fillDialog();
		TranslateDialogDefault(hwnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDC_MESSAGE:
			csamw->checkFieldLimit(HIWORD(wparam), LOWORD(wparam));
			break;

		case IDOK:
			csamw->checkItemValidity();

		case IDCANCEL:
			EnableWindow(csamw->m_parent->m_handle, TRUE);
			EndDialog(hwnd, LOWORD(wparam));
			break;
		}
		break;
	}
	return FALSE;
}


INT_PTR CALLBACK CSOptionsProc(HWND hwnd, UINT message, WPARAM, LPARAM lparam)
{
	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		CheckDlgButton(hwnd, IDC_CONFIRM_DELETION,
			g_plugin.getByte("ConfirmDeletion", DEFAULT_PLUGIN_CONFIRM_ITEMS_DELETION) ?
			BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwnd, IDC_DELETE_AFTER_IMPORT,
			g_plugin.getByte("DeleteAfterImport", DEFAULT_PLUGIN_DELETE_AFTER_IMPORT) ?
			BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwnd, IDC_REMEMBER_POSITION,
			g_plugin.getByte("RememberWindowPosition", DEFAULT_REMEMBER_WINDOW_POSITION) ?
			BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lparam)->code) {
		case PSN_APPLY:
			g_plugin.setByte("ConfirmDeletion", IsDlgButtonChecked(hwnd, IDC_CONFIRM_DELETION) ? 1 : 0);
			g_plugin.setByte("DeleteAfterImport", IsDlgButtonChecked(hwnd, IDC_DELETE_AFTER_IMPORT) ? 1 : 0);
			g_plugin.setByte("RememberWindowPosition", IsDlgButtonChecked(hwnd, IDC_REMEMBER_POSITION) ? 1 : 0);

			Menu_ReloadProtoMenus();
			break;
		}
		return TRUE;
	}
	return FALSE;
}
