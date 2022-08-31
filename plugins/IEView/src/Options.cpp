/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

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

#include "stdafx.h"

#define UM_CHECKSTATECHANGE (WM_USER+100)
HANDLE hHookOptionsChanged;

static ProtocolSettings *srmmCurrentProtoItem = nullptr;
static ProtocolSettings *chatCurrentProtoItem = nullptr;
static ProtocolSettings *historyCurrentProtoItem = nullptr;
static HIMAGELIST hProtocolImageList = nullptr;
static HIMAGELIST hImageList = nullptr;

static OBJLIST<ProtocolSettings> arProtos(1), arTemp(1);

static LPARAM GetItemParam(HWND hwndTreeView, HTREEITEM hItem)
{
	TVITEM tvi = { 0 };
	tvi.mask = TVIF_PARAM;
	tvi.hItem = hItem == nullptr ? TreeView_GetSelection(hwndTreeView) : hItem;
	TreeView_GetItem(hwndTreeView, &tvi);
	return tvi.lParam;
}

static void SaveSRMMProtoSettings(HWND hwndDlg, ProtocolSettings *proto)
{
	if (proto != nullptr) {
		int i = Options::MODE_COMPATIBLE;
		if (IsDlgButtonChecked(hwndDlg, IDC_MODE_TEMPLATE))
			i = Options::MODE_TEMPLATE;
		else if (IsDlgButtonChecked(hwndDlg, IDC_MODE_CSS))
			i = Options::MODE_CSS;

		proto->setSRMMMode(i);
		i = IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE) ? Options::LOG_IMAGE_ENABLED : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE) ? Options::LOG_IMAGE_SCROLL : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_NICKNAMES) ? Options::LOG_SHOW_NICKNAMES : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_TIME) ? Options::LOG_SHOW_TIME : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_DATE) ? Options::LOG_SHOW_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_SECONDS) ? Options::LOG_SHOW_SECONDS : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_LONG_DATE) ? Options::LOG_LONG_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_RELATIVE_DATE) ? Options::LOG_RELATIVE_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_GROUP_MESSAGES) ? Options::LOG_GROUP_MESSAGES : 0;
		proto->setSRMMFlags(i);

		char path[MAX_PATH];
		GetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, _countof(path));
		proto->setSRMMBackgroundFilename(path);
		GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, _countof(path));
		proto->setSRMMCssFilename(path);
		GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path, _countof(path));
		proto->setSRMMTemplateFilename(path);
	}
}

static void SaveChatProtoSettings(HWND hwndDlg, ProtocolSettings *proto)
{
	if (proto != nullptr) {
		int i = Options::MODE_COMPATIBLE;
		if (IsDlgButtonChecked(hwndDlg, IDC_MODE_TEMPLATE))
			i = Options::MODE_TEMPLATE;
		else if (IsDlgButtonChecked(hwndDlg, IDC_MODE_CSS))
			i = Options::MODE_CSS;

		proto->setChatMode(i);
		i = IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE) ? Options::LOG_IMAGE_ENABLED : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE) ? Options::LOG_IMAGE_SCROLL : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_NICKNAMES) ? Options::LOG_SHOW_NICKNAMES : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_TIME) ? Options::LOG_SHOW_TIME : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_DATE) ? Options::LOG_SHOW_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_SECONDS) ? Options::LOG_SHOW_SECONDS : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_LONG_DATE) ? Options::LOG_LONG_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_RELATIVE_DATE) ? Options::LOG_RELATIVE_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_GROUP_MESSAGES) ? Options::LOG_GROUP_MESSAGES : 0;
		proto->setChatFlags(i);

		char path[MAX_PATH];
		GetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, _countof(path));
		proto->setChatBackgroundFilename(path);
		GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, _countof(path));
		proto->setChatCssFilename(path);
		GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path, _countof(path));
		proto->setChatTemplateFilename(path);
	}
}

static void SaveHistoryProtoSettings(HWND hwndDlg, ProtocolSettings *proto)
{
	if (proto != nullptr) {
		int i = Options::MODE_COMPATIBLE;
		if (IsDlgButtonChecked(hwndDlg, IDC_MODE_TEMPLATE))
			i = Options::MODE_TEMPLATE;
		else if (IsDlgButtonChecked(hwndDlg, IDC_MODE_CSS))
			i = Options::MODE_CSS;

		proto->setHistoryMode(i);
		i = IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE) ? Options::LOG_IMAGE_ENABLED : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE) ? Options::LOG_IMAGE_SCROLL : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_NICKNAMES) ? Options::LOG_SHOW_NICKNAMES : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_TIME) ? Options::LOG_SHOW_TIME : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_DATE) ? Options::LOG_SHOW_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_SECONDS) ? Options::LOG_SHOW_SECONDS : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_LONG_DATE) ? Options::LOG_LONG_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_RELATIVE_DATE) ? Options::LOG_RELATIVE_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_GROUP_MESSAGES) ? Options::LOG_GROUP_MESSAGES : 0;
		proto->setHistoryFlags(i);

		char path[MAX_PATH];
		GetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, _countof(path));
		proto->setHistoryBackgroundFilename(path);
		GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, _countof(path));
		proto->setHistoryCssFilename(path);
		GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path, _countof(path));
		proto->setHistoryTemplateFilename(path);
	}
}

static void UpdateControlsState(HWND hwndDlg)
{
	BOOL bChecked = IsDlgButtonChecked(hwndDlg, IDC_MODE_TEMPLATE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEMPLATES_FILENAME), bChecked);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_TEMPLATES), bChecked);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_NICKNAMES), bChecked);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_TIME), bChecked);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_DATE), bChecked);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_SHOW_SECONDS), bChecked);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_LONG_DATE), bChecked);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_RELATIVE_DATE), bChecked);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LOG_GROUP_MESSAGES), bChecked);

	bChecked = IsDlgButtonChecked(hwndDlg, IDC_MODE_CSS);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EXTERNALCSS_FILENAME), bChecked);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_EXTERNALCSS), bChecked);

	bChecked = IsDlgButtonChecked(hwndDlg, IDC_MODE_COMPATIBLE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE), bChecked);
	bChecked &= IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);
}

static void SetIcon(HWND hwnd, uint32_t id, int index, bool condition)
{
	HICON hIcon;
	if (condition)
		hIcon = ImageList_GetIcon(hImageList, index + 1, ILD_NORMAL);
	else
		hIcon = ImageList_GetIcon(hImageList, index + 0, ILD_NORMAL);

	hIcon = (HICON)SendDlgItemMessage(hwnd, id, STM_SETICON, (WPARAM)hIcon, 0);
	if (hIcon != nullptr)
		DestroyIcon(hIcon);
}

static void UpdateTemplateIcons(HWND hwnd, const char *path)
{
	TemplateMap *tmap = TemplateMap::loadTemplates(path, path, true);
	if (tmap != nullptr) {
		SetIcon(hwnd, IDC_GROUPSUPPORT, 0, tmap->isGrouping());
		SetIcon(hwnd, IDC_RTLSUPPORT, 2, tmap->isRTL());
		delete tmap;
	}
	else {
		SetIcon(hwnd, IDC_GROUPSUPPORT, 0, false);
		SetIcon(hwnd, IDC_RTLSUPPORT, 2, false);
	}
}

static void UpdateSRMMProtoInfo(HWND hwndDlg, ProtocolSettings *proto)
{
	if (proto != nullptr) {
		HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
		TreeView_SetCheckState(hProtoList, TreeView_GetSelection(hProtoList), proto->isSRMMEnable());
		CheckDlgButton(hwndDlg, IDC_MODE_TEMPLATE, proto->getSRMMMode() == Options::MODE_TEMPLATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MODE_CSS, proto->getSRMMMode() == Options::MODE_CSS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MODE_COMPATIBLE, proto->getSRMMMode() == Options::MODE_COMPATIBLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_BACKGROUND_IMAGE, proto->getSRMMFlags() & Options::LOG_IMAGE_ENABLED ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE, proto->getSRMMFlags() & Options::LOG_IMAGE_SCROLL ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_NICKNAMES, proto->getSRMMFlags() & Options::LOG_SHOW_NICKNAMES ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_TIME, proto->getSRMMFlags() & Options::LOG_SHOW_TIME ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_DATE, proto->getSRMMFlags() & Options::LOG_SHOW_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_SECONDS, proto->getSRMMFlags() & Options::LOG_SHOW_SECONDS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_LONG_DATE, proto->getSRMMFlags() & Options::LOG_LONG_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_RELATIVE_DATE, proto->getSRMMFlags() & Options::LOG_RELATIVE_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_GROUP_MESSAGES, proto->getSRMMFlags() & Options::LOG_GROUP_MESSAGES ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, proto->getSRMMBackgroundFilename());
		SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, proto->getSRMMCssFilename());
		SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, proto->getSRMMTemplateFilename());

		UpdateTemplateIcons(hwndDlg, proto->getSRMMTemplateFilename());
		srmmCurrentProtoItem = proto;
		UpdateControlsState(hwndDlg);
	}
}

static void UpdateChatProtoInfo(HWND hwndDlg, ProtocolSettings *proto)
{
	if (proto != nullptr) {
		HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
		TreeView_SetCheckState(hProtoList, TreeView_GetSelection(hProtoList), proto->isChatEnable());
		CheckDlgButton(hwndDlg, IDC_MODE_TEMPLATE, proto->getChatMode() == Options::MODE_TEMPLATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MODE_CSS, proto->getChatMode() == Options::MODE_CSS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MODE_COMPATIBLE, proto->getChatMode() == Options::MODE_COMPATIBLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_BACKGROUND_IMAGE, proto->getChatFlags() & Options::LOG_IMAGE_ENABLED ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE, proto->getChatFlags() & Options::LOG_IMAGE_SCROLL ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_NICKNAMES, proto->getChatFlags() & Options::LOG_SHOW_NICKNAMES ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_TIME, proto->getChatFlags() & Options::LOG_SHOW_TIME ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_DATE, proto->getChatFlags() & Options::LOG_SHOW_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_SECONDS, proto->getChatFlags() & Options::LOG_SHOW_SECONDS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_LONG_DATE, proto->getChatFlags() & Options::LOG_LONG_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_RELATIVE_DATE, proto->getChatFlags() & Options::LOG_RELATIVE_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_GROUP_MESSAGES, proto->getChatFlags() & Options::LOG_GROUP_MESSAGES ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, proto->getChatBackgroundFilename());
		SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, proto->getChatCssFilename());
		SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, proto->getChatTemplateFilename());

		UpdateTemplateIcons(hwndDlg, proto->getChatTemplateFilename());
		chatCurrentProtoItem = proto;
		UpdateControlsState(hwndDlg);
	}
}

static void UpdateHistoryProtoInfo(HWND hwndDlg, ProtocolSettings *proto)
{
	if (proto != nullptr) {
		HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
		TreeView_SetCheckState(hProtoList, TreeView_GetSelection(hProtoList), proto->isHistoryEnable());
		CheckDlgButton(hwndDlg, IDC_MODE_TEMPLATE, proto->getHistoryMode() == Options::MODE_TEMPLATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MODE_CSS, proto->getHistoryMode() == Options::MODE_CSS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MODE_COMPATIBLE, proto->getHistoryMode() == Options::MODE_COMPATIBLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_BACKGROUND_IMAGE, proto->getHistoryFlags() & Options::LOG_IMAGE_ENABLED ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE, proto->getHistoryFlags() & Options::LOG_IMAGE_SCROLL ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_NICKNAMES, proto->getHistoryFlags() & Options::LOG_SHOW_NICKNAMES ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_TIME, proto->getHistoryFlags() & Options::LOG_SHOW_TIME ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_DATE, proto->getHistoryFlags() & Options::LOG_SHOW_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_SECONDS, proto->getHistoryFlags() & Options::LOG_SHOW_SECONDS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_LONG_DATE, proto->getHistoryFlags() & Options::LOG_LONG_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_RELATIVE_DATE, proto->getHistoryFlags() & Options::LOG_RELATIVE_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_GROUP_MESSAGES, proto->getHistoryFlags() & Options::LOG_GROUP_MESSAGES ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, proto->getHistoryBackgroundFilename());
		SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, proto->getHistoryCssFilename());
		SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, proto->getHistoryTemplateFilename());

		UpdateTemplateIcons(hwndDlg, proto->getHistoryTemplateFilename());
		historyCurrentProtoItem = proto;
		UpdateControlsState(hwndDlg);
	}
}

static void RefreshProtoIcons()
{
	hProtocolImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK | ILC_COLOR32, 0, 0);

	for (auto &it : arProtos) {
		HICON hIcon = nullptr;

		if (it->getProtocolName() != nullptr) {
			hIcon = (HICON)CallProtoService(it->getProtocolName(), PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
			if (hIcon == nullptr)
				hIcon = (HICON)CallProtoService(it->getProtocolName(), PS_LOADICON, PLI_PROTOCOL, 0);
		}

		if (hIcon != nullptr) {
			ImageList_AddIcon(hProtocolImageList, hIcon);
			DestroyIcon(hIcon);
		}
		else ImageList_AddSkinIcon(hProtocolImageList, SKINICON_OTHER_MIRANDA);
	}
}

static void RefreshIcons()
{
	hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK | ILC_COLOR32, 0, 0);
	ImageList_AddIcon(hImageList, IcoLib_GetIconByHandle(iconList[3].hIcolib));
	ImageList_AddIcon(hImageList, IcoLib_GetIconByHandle(iconList[2].hIcolib));
	ImageList_AddIcon(hImageList, IcoLib_GetIconByHandle(iconList[1].hIcolib));
	ImageList_AddIcon(hImageList, IcoLib_GetIconByHandle(iconList[0].hIcolib));
}

static void RefreshProtoList(HWND hwndDlg, int mode, bool protoTemplates)
{
	HTREEITEM hItem = nullptr;
	HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
	TreeView_DeleteAllItems(hProtoList);
	TreeView_SetImageList(hProtoList, hProtocolImageList, TVSIL_NORMAL);

	for (auto &it : arTemp) {
		TVINSERTSTRUCT tvi = {};
		tvi.hParent = TVI_ROOT;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_STATE | TVIF_SELECTEDIMAGE;
		tvi.item.stateMask = TVIS_SELECTED | TVIS_STATEIMAGEMASK;
		if (it->getProtocolName() == nullptr)
			tvi.item.pszText = TranslateT("Default");
		else {
			PROTOACCOUNT *pa = Proto_GetAccount(it->getProtocolName());
			tvi.item.pszText = (pa == nullptr) ? L"" : pa->tszAccountName;
		}

		tvi.item.lParam = (LPARAM)it;
		tvi.item.iImage = tvi.item.iSelectedImage = arTemp.indexOf(&it);
		switch (mode) {
		case 0:
			tvi.item.state = INDEXTOSTATEIMAGEMASK(it->isSRMMEnable() ? 2 : 1);
			break;
		case 1:
			tvi.item.state = INDEXTOSTATEIMAGEMASK(it->isChatEnable() ? 2 : 1);
			break;
		case 2:
			tvi.item.state = INDEXTOSTATEIMAGEMASK(it->isHistoryEnable() ? 2 : 1);
			break;
		}

		if (it->getProtocolName() == nullptr)
			hItem = TreeView_InsertItem(hProtoList, &tvi);
		else
			TreeView_InsertItem(hProtoList, &tvi);

		if (!protoTemplates)
			break;
	}

	TreeView_SelectItem(hProtoList, hItem);
}

static bool BrowseFile(HWND hwndDlg, char *filter, char *defExt, char *path, int maxLen)
{
	GetWindowTextA(hwndDlg, path, maxLen);
	
	OPENFILENAMEA ofn = {};
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwndDlg;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = path;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.nMaxFile = maxLen;
	ofn.nMaxFileTitle = maxLen;
	ofn.lpstrDefExt = defExt;
	if (GetOpenFileNameA(&ofn)) {
		SetWindowTextA(hwndDlg, path);
		return true;
	}
	return false;
}

static int initialized = 0;
static int changed = 0;

static void MarkInitialized(int i)
{
	if (initialized == 0) {
		for (auto &it : arProtos)
			arTemp.insert(new ProtocolSettings(*it));

		RefreshProtoIcons();
		RefreshIcons();
	}
	initialized |= i;
}

static void ApplyChanges(int i)
{
	changed &= ~i;
	if (changed == 0) {
		Options::saveProtocolSettings();
		NotifyEventHooks(hHookOptionsChanged, 0, 0);
	}
}

static void CleanDialog(int i)
{
	initialized &= ~i;
	if (initialized == 0) {
		arTemp.destroy();

		ImageList_Destroy(hImageList); hImageList = nullptr;
		ImageList_Destroy(hProtocolImageList); hProtocolImageList = nullptr;
	}
}

static void MarkChanges(int i, HWND hWnd)
{
	SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
	changed |= i;
}

static INT_PTR CALLBACK IEViewGeneralOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		MarkInitialized(1);
		TranslateDialogDefault(hwndDlg);
		if (Options::generalFlags & Options::GENERAL_ENABLE_BBCODES)
			CheckDlgButton(hwndDlg, IDC_ENABLE_BBCODES, BST_CHECKED);

		if (Options::generalFlags & Options::GENERAL_ENABLE_FLASH)
			CheckDlgButton(hwndDlg, IDC_ENABLE_FLASH, BST_CHECKED);

		if (Options::generalFlags & Options::GENERAL_SMILEYINNAMES)
			CheckDlgButton(hwndDlg, IDC_SMILEYS_IN_NAMES, BST_CHECKED);

		if (Options::generalFlags & Options::GENERAL_NO_BORDER)
			CheckDlgButton(hwndDlg, IDC_NO_BORDER, BST_CHECKED);

		if (Options::generalFlags & Options::GENERAL_ENABLE_EMBED)
			CheckDlgButton(hwndDlg, IDC_ENABLE_EMBED, BST_CHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_SMILEYS_IN_NAMES), Options::bHasSmileyAdd);
		EnableWindow(GetDlgItem(hwndDlg, IDC_EMBED_SIZE), IsDlgButtonChecked(hwndDlg, IDC_ENABLE_EMBED));
		{
			wchar_t* size[] = { L"320 x 205", L"480 x 385", L"560 x 349", L"640 x 390" };
			for (auto &it : size) {
				int item = SendDlgItemMessage(hwndDlg, IDC_EMBED_SIZE, CB_ADDSTRING, 0, (LPARAM)TranslateW(it));
				SendDlgItemMessage(hwndDlg, IDC_EMBED_SIZE, CB_SETITEMDATA, item, 0);
			}
			SendDlgItemMessage(hwndDlg, IDC_EMBED_SIZE, CB_SETCURSEL, Options::getEmbedSize(), 0);
		}
		return TRUE;

	case WM_DESTROY:
		CleanDialog(1);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ENABLE_BBCODES:
		case IDC_ENABLE_FLASH:
		case IDC_SMILEYS_IN_NAMES:
		case IDC_NO_BORDER:
		case IDC_EMBED_SIZE:
			MarkChanges(1, hwndDlg);
			break;
		case IDC_ENABLE_EMBED:
			MarkChanges(1, hwndDlg);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EMBED_SIZE), IsDlgButtonChecked(hwndDlg, IDC_ENABLE_EMBED));
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			uint32_t i = 0;
			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_BBCODES))
				i |= Options::GENERAL_ENABLE_BBCODES;

			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_FLASH))
				i |= Options::GENERAL_ENABLE_FLASH;

			if (IsDlgButtonChecked(hwndDlg, IDC_SMILEYS_IN_NAMES))
				i |= Options::GENERAL_SMILEYINNAMES;

			if (IsDlgButtonChecked(hwndDlg, IDC_NO_BORDER))
				i |= Options::GENERAL_NO_BORDER;

			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_EMBED))
				i |= Options::GENERAL_ENABLE_EMBED;

			Options::generalFlags = i;
			g_plugin.setDword(DBS_BASICFLAGS, i);

			ApplyChanges(1);
			Options::setEmbedSize(SendDlgItemMessage(hwndDlg, IDC_EMBED_SIZE, CB_GETCURSEL, 0, 0));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK IEViewSRMMOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL bChecked;
	char path[MAX_PATH], filter[MAX_PATH];

	switch (msg) {
	case WM_INITDIALOG:
		MarkInitialized(2);
		TranslateDialogDefault(hwndDlg);
		srmmCurrentProtoItem = nullptr;
		RefreshProtoList(hwndDlg, 0, true);
		return TRUE;

	case WM_DESTROY:
		CleanDialog(2);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACKGROUND_IMAGE_FILENAME:
		case IDC_EXTERNALCSS_FILENAME:
		case IDC_EXTERNALCSS_FILENAME_RTL:
		case IDC_TEMPLATES_FILENAME:
			if ((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE)
				MarkChanges(2, hwndDlg);
			break;
		case IDC_SCROLL_BACKGROUND_IMAGE:
		case IDC_LOG_SHOW_NICKNAMES:
		case IDC_LOG_SHOW_TIME:
		case IDC_LOG_SHOW_DATE:
		case IDC_LOG_SHOW_SECONDS:
		case IDC_LOG_LONG_DATE:
		case IDC_LOG_RELATIVE_DATE:
		case IDC_LOG_GROUP_MESSAGES:
			MarkChanges(2, hwndDlg);
			break;
		case IDC_BACKGROUND_IMAGE:
			bChecked = IsDlgButtonChecked(hwndDlg, IDC_MODE_COMPATIBLE) && IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
			MarkChanges(2, hwndDlg);
			break;
		case IDC_BROWSE_TEMPLATES:
			mir_snprintf(filter, "%s (*.ivt)%c*.ivt%c%s (*.*)%c*.*%c%c", Translate("Template"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "ivt", path, _countof(path))) {
				SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path);
				UpdateTemplateIcons(hwndDlg, path);
				MarkChanges(2, hwndDlg);
			}
			break;
		case IDC_BROWSE_BACKGROUND_IMAGE:
			mir_snprintf(filter, "%s (*.jpg,*.jpeg,*.gif,*.png,*.bmp)%c*.jpg;*.jpeg;*.gif;*.png;*.bmp%c%s (*.*)%c*.*%c%c", Translate("All Images"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "jpg", path, _countof(path))) {
				SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path);
				MarkChanges(2, hwndDlg);
			}
			break;
		case IDC_BROWSE_EXTERNALCSS:
			mir_snprintf(filter, "%s (*.css)%c*.css%c%s (*.*)%c*.*%c%c", Translate("Style Sheet"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "css", path, _countof(path))) {
				SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path);
				MarkChanges(2, hwndDlg);
			}
			break;
		case IDC_BROWSE_EXTERNALCSS_RTL:
			mir_snprintf(filter, "%s (*.css)%c*.css%c%s (*.*)%c*.*%c%c", Translate("Style Sheet"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "css", path, _countof(path))) {
				SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, path);
				MarkChanges(2, hwndDlg);
			}
			break;
		case IDC_MODE_COMPATIBLE:
		case IDC_MODE_CSS:
		case IDC_MODE_TEMPLATE:
			UpdateControlsState(hwndDlg);
			MarkChanges(2, hwndDlg);
			break;
		case IDC_GETTEMPLATES:
			Utils_OpenUrl("https://miranda-ng.org/tags/ieview-templates/");
			break;
		}
		break;

	case UM_CHECKSTATECHANGE:
		{
			ProtocolSettings *proto = (ProtocolSettings *)GetItemParam((HWND)wParam, (HTREEITEM)lParam);
			if (proto != nullptr)
				if (proto->getProtocolName() != nullptr)
					proto->setSRMMEnable(0 != TreeView_GetCheckState((HWND)wParam, (HTREEITEM)lParam));

			if ((HTREEITEM)lParam != TreeView_GetSelection((HWND)wParam))
				TreeView_SelectItem((HWND)wParam, (HTREEITEM)lParam);
			else
				UpdateSRMMProtoInfo(hwndDlg, proto);

			MarkChanges(2, hwndDlg);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_PROTOLIST) {
			switch (((LPNMHDR)lParam)->code) {
			case NM_CLICK:
				{
					TVHITTESTINFO ht = { 0 };
					uint32_t dwpos = GetMessagePos();
					POINTSTOPOINT(ht.pt, MAKEPOINTS(dwpos));
					MapWindowPoints(HWND_DESKTOP, ((LPNMHDR)lParam)->hwndFrom, &ht.pt, 1);
					TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &ht);
					if (TVHT_ONITEMSTATEICON & ht.flags) {
						PostMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom, (LPARAM)ht.hItem);
						return FALSE;
					}
				}
				break;

			case TVN_KEYDOWN:
				if (((LPNMTVKEYDOWN)lParam)->wVKey == VK_SPACE)
					PostMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom,
					(LPARAM)TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom));
				break;

			case TVN_SELCHANGED:
				ProtocolSettings *proto = (ProtocolSettings*)GetItemParam(GetDlgItem(hwndDlg, IDC_PROTOLIST), (HTREEITEM)nullptr);
				SaveSRMMProtoSettings(hwndDlg, srmmCurrentProtoItem);
				UpdateSRMMProtoInfo(hwndDlg, proto);
				break;
			}
			break;
		}
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			SaveSRMMProtoSettings(hwndDlg, srmmCurrentProtoItem);
			ApplyChanges(2);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK IEViewHistoryOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL bChecked = FALSE;
	char path[MAX_PATH], filter[MAX_PATH];
	switch (msg) {
	case WM_INITDIALOG:
		MarkInitialized(4);
		TranslateDialogDefault(hwndDlg);
		historyCurrentProtoItem = nullptr;
		RefreshProtoList(hwndDlg, 2, true);
		return TRUE;

	case WM_DESTROY:
		CleanDialog(4);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACKGROUND_IMAGE_FILENAME:
		case IDC_EXTERNALCSS_FILENAME:
		case IDC_EXTERNALCSS_FILENAME_RTL:
		case IDC_TEMPLATES_FILENAME:
			if ((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE)
				MarkChanges(4, hwndDlg);
			break;
		case IDC_SCROLL_BACKGROUND_IMAGE:
		case IDC_LOG_SHOW_NICKNAMES:
		case IDC_LOG_SHOW_TIME:
		case IDC_LOG_SHOW_DATE:
		case IDC_LOG_SHOW_SECONDS:
		case IDC_LOG_LONG_DATE:
		case IDC_LOG_RELATIVE_DATE:
		case IDC_LOG_GROUP_MESSAGES:
			MarkChanges(4, hwndDlg);
			break;
		case IDC_BACKGROUND_IMAGE:
			bChecked = IsDlgButtonChecked(hwndDlg, IDC_MODE_COMPATIBLE) && IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
			MarkChanges(4, hwndDlg);
			break;
		case IDC_BROWSE_TEMPLATES:
			mir_snprintf(filter, "%s (*.ivt)%c*.ivt%c%s%c*.*%c%c", Translate("Template"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "ivt", path, _countof(path))) {
				SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path);
				UpdateTemplateIcons(hwndDlg, path);
				MarkChanges(4, hwndDlg);
			}
			break;
		case IDC_BROWSE_BACKGROUND_IMAGE:
			mir_snprintf(filter, "%s (*.jpg,*.gif,*.png,*.bmp)%c*.ivt%c%s%c*.*%c%c", Translate("All Images"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "jpg", path, _countof(path))) {
				SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path);
				MarkChanges(4, hwndDlg);
			}
			break;
		case IDC_BROWSE_EXTERNALCSS:
			mir_snprintf(filter, "%s (*.css)%c*.ivt%c%s%c*.*%c%c", Translate("Style Sheet"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "css", path, _countof(path))) {
				SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path);
				MarkChanges(4, hwndDlg);
			}
			break;
		case IDC_BROWSE_EXTERNALCSS_RTL:
			mir_snprintf(filter, "%s (*.css)%c*.ivt%c%s%c*.*%c%c", Translate("Style Sheet"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "css", path, _countof(path))) {
				SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, path);
				MarkChanges(4, hwndDlg);
			}
			break;
		case IDC_MODE_COMPATIBLE:
		case IDC_MODE_CSS:
		case IDC_MODE_TEMPLATE:
			UpdateControlsState(hwndDlg);
			MarkChanges(4, hwndDlg);
			break;
		}
		break;

	case UM_CHECKSTATECHANGE:
		{
			ProtocolSettings *proto = (ProtocolSettings *)GetItemParam((HWND)wParam, (HTREEITEM)lParam);
			if (proto != nullptr)
				if (proto->getProtocolName() != nullptr)
					proto->setHistoryEnable(0 != TreeView_GetCheckState((HWND)wParam, (HTREEITEM)lParam));

			if ((HTREEITEM)lParam != TreeView_GetSelection((HWND)wParam))
				TreeView_SelectItem((HWND)wParam, (HTREEITEM)lParam);
			else
				UpdateHistoryProtoInfo(hwndDlg, proto);

			MarkChanges(4, hwndDlg);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_PROTOLIST) {
			switch (((LPNMHDR)lParam)->code) {
			case NM_CLICK:
				{
					TVHITTESTINFO ht = {};
					uint32_t dwpos = GetMessagePos();
					POINTSTOPOINT(ht.pt, MAKEPOINTS(dwpos));
					MapWindowPoints(HWND_DESKTOP, ((LPNMHDR)lParam)->hwndFrom, &ht.pt, 1);
					TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &ht);
					if (TVHT_ONITEMSTATEICON & ht.flags) {
						PostMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom, (LPARAM)ht.hItem);
						return FALSE;
					}
				}
				break;
			case TVN_KEYDOWN:
				if (((LPNMTVKEYDOWN)lParam)->wVKey == VK_SPACE)
					PostMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom,
					(LPARAM)TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom));
				break;

			case TVN_SELCHANGED:
				ProtocolSettings * proto = (ProtocolSettings *)GetItemParam(GetDlgItem(hwndDlg, IDC_PROTOLIST), (HTREEITEM)nullptr);
				SaveHistoryProtoSettings(hwndDlg, historyCurrentProtoItem);
				UpdateHistoryProtoInfo(hwndDlg, proto);
				break;
			}
			break;
		}
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			SaveHistoryProtoSettings(hwndDlg, historyCurrentProtoItem);
			ApplyChanges(4);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK IEViewGroupChatsOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL bChecked;
	char path[MAX_PATH], filter[MAX_PATH];
	switch (msg) {
	case WM_INITDIALOG:
		MarkInitialized(8);
		TranslateDialogDefault(hwndDlg);
		chatCurrentProtoItem = nullptr;
		RefreshProtoList(hwndDlg, 1, true);
		return TRUE;

	case WM_DESTROY:
		CleanDialog(8);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACKGROUND_IMAGE_FILENAME:
		case IDC_EXTERNALCSS_FILENAME:
		case IDC_EXTERNALCSS_FILENAME_RTL:
		case IDC_TEMPLATES_FILENAME:
			if ((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE)
				MarkChanges(8, hwndDlg);
			break;
		case IDC_SCROLL_BACKGROUND_IMAGE:
		case IDC_LOG_SHOW_NICKNAMES:
		case IDC_LOG_SHOW_TIME:
		case IDC_LOG_SHOW_DATE:
		case IDC_LOG_SHOW_SECONDS:
		case IDC_LOG_LONG_DATE:
		case IDC_LOG_RELATIVE_DATE:
		case IDC_LOG_GROUP_MESSAGES:
			MarkChanges(8, hwndDlg);
			break;
		case IDC_BACKGROUND_IMAGE:
			bChecked = IsDlgButtonChecked(hwndDlg, IDC_MODE_COMPATIBLE) && IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE_BACKGROUND_IMAGE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE), bChecked);
			MarkChanges(8, hwndDlg);
			break;
		case IDC_BROWSE_TEMPLATES:
			mir_snprintf(filter, "%s (*.ivt)%c*.ivt%c%s%c*.*%c%c", Translate("Template"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "ivt", path, _countof(path))) {
				SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path);
				UpdateTemplateIcons(hwndDlg, path);
				MarkChanges(8, hwndDlg);
			}
			break;
		case IDC_BROWSE_BACKGROUND_IMAGE:
			mir_snprintf(filter, "%s (*.jpg,*.gif,*.png,*.bmp)%c*.ivt%c%s%c*.*%c%c", Translate("All Images"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "jpg", path, _countof(path))) {
				SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path);
				MarkChanges(8, hwndDlg);
			}
			break;
		case IDC_BROWSE_EXTERNALCSS:
			mir_snprintf(filter, "%s (*.css)%c*.ivt%c%s%c*.*%c%c", Translate("Style Sheet"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "css", path, _countof(path))) {
				SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path);
				MarkChanges(8, hwndDlg);
			}
			break;
		case IDC_BROWSE_EXTERNALCSS_RTL:
			mir_snprintf(filter, "%s (*.css)%c*.ivt%c%s%c*.*%c%c", Translate("Style Sheet"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "css", path, _countof(path))) {
				SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME_RTL, path);
				MarkChanges(8, hwndDlg);
			}
			break;
		case IDC_MODE_COMPATIBLE:
		case IDC_MODE_CSS:
		case IDC_MODE_TEMPLATE:
			UpdateControlsState(hwndDlg);
			MarkChanges(8, hwndDlg);
			break;
		}
		break;

	case UM_CHECKSTATECHANGE:
		{
			ProtocolSettings *proto = (ProtocolSettings*)GetItemParam((HWND)wParam, (HTREEITEM)lParam);
			if (proto != nullptr)
				if (proto->getProtocolName() != nullptr)
					proto->setChatEnable(0 != TreeView_GetCheckState((HWND)wParam, (HTREEITEM)lParam));

			if ((HTREEITEM)lParam != TreeView_GetSelection((HWND)wParam)) {
				TreeView_SelectItem((HWND)wParam, (HTREEITEM)lParam);
			}
			else {
				UpdateChatProtoInfo(hwndDlg, proto);
			}
			MarkChanges(8, hwndDlg);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_PROTOLIST) {
			switch (((LPNMHDR)lParam)->code) {
			case NM_CLICK:
				{
					TVHITTESTINFO ht = {};
					uint32_t dwpos = GetMessagePos();
					POINTSTOPOINT(ht.pt, MAKEPOINTS(dwpos));
					MapWindowPoints(HWND_DESKTOP, ((LPNMHDR)lParam)->hwndFrom, &ht.pt, 1);
					TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &ht);
					if (TVHT_ONITEMSTATEICON & ht.flags) {
						PostMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom, (LPARAM)ht.hItem);
						return FALSE;
					}
				}
				break;
			case TVN_KEYDOWN:
				if (((LPNMTVKEYDOWN)lParam)->wVKey == VK_SPACE)
					PostMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom,
					(LPARAM)TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom));
				break;

			case TVN_SELCHANGED:
				ProtocolSettings * proto = (ProtocolSettings *)GetItemParam(GetDlgItem(hwndDlg, IDC_PROTOLIST), (HTREEITEM)nullptr);
				SaveChatProtoSettings(hwndDlg, chatCurrentProtoItem);
				UpdateChatProtoInfo(hwndDlg, proto);
				break;
			}
			break;
		}
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			SaveChatProtoSettings(hwndDlg, chatCurrentProtoItem);
			ApplyChanges(8);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int IEViewOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.w = LPGENW("Message sessions");
	odp.szTitle.w = LPGENW("IEView");
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_GENERAL_OPTIONS);
	odp.pfnDlgProc = IEViewGeneralOptDlgProc;
	odp.szTab.w = LPGENW("General");
	g_plugin.addOptions(wParam, &odp);

	odp.szGroup.w = LPGENW("Skins");
	odp.szTitle.w = LPGENW("IEView");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SRMM_OPTIONS);
	odp.pfnDlgProc = IEViewSRMMOptDlgProc;
	odp.szTab.w = LPGENW("Message Log");
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SRMM_OPTIONS);
	odp.pfnDlgProc = IEViewGroupChatsOptDlgProc;
	odp.szTab.w = LPGENW("Group chats");
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SRMM_OPTIONS);
	odp.pfnDlgProc = IEViewHistoryOptDlgProc;
	odp.szTab.w = LPGENW("History");
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool Options::isInited = false;
bool Options::bHasAvs = false;
bool Options::bHasSmileyAdd = false;
int  Options::generalFlags;

ProtocolSettings::ProtocolSettings(const char *protocolName)
{
	this->protocolName = mir_strdup(protocolName);

	srmmMode = Options::MODE_COMPATIBLE;
	chatMode = Options::MODE_COMPATIBLE;
	historyMode = Options::MODE_COMPATIBLE;
}

ProtocolSettings::~ProtocolSettings()
{
}

/////////////////////////////////////////////////////////////////////////////////////////

void ProtocolSettings::readFromDb()
{
	/* SRMM settings */
	const char *szProto = protocolName;
	if (szProto == nullptr)
		szProto = "_default_";

	char dbsName[256];
	mir_snprintf(dbsName, "%s.%s", szProto, DBS_SRMM_ENABLE);
	setSRMMEnable(protocolName == nullptr ? true : 0 != g_plugin.getByte(dbsName, FALSE));

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_SRMM_MODE);
	setSRMMMode(g_plugin.getByte(dbsName, FALSE));

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_SRMM_FLAGS);
	setSRMMFlags(g_plugin.getDword(dbsName, 16128));

	DBVARIANT dbv;
	char tmpPath[MAX_PATH];
	mir_snprintf(dbsName, "%s.%s", szProto, DBS_SRMM_BACKGROUND);
	if (!g_plugin.getString(dbsName, &dbv)) {
		PathToAbsolute(dbv.pszVal, tmpPath);
		setSRMMBackgroundFilename(tmpPath);
		db_free(&dbv);
	}

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_SRMM_CSS);
	if (!g_plugin.getString(dbsName, &dbv)) {
		PathToAbsolute(dbv.pszVal, tmpPath);
		setSRMMCssFilename(tmpPath);
		db_free(&dbv);
	}

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_SRMM_TEMPLATE);
	if (!g_plugin.getString(dbsName, &dbv)) {
		PathToAbsolute(dbv.pszVal, tmpPath);
		setSRMMTemplateFilename(tmpPath);
		db_free(&dbv);
	}

	/* Group chat settings */
	mir_snprintf(dbsName, "%s.%s", szProto, DBS_CHAT_ENABLE);
	setChatEnable(protocolName == nullptr ? true : 0 != g_plugin.getByte(dbsName, FALSE));

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_CHAT_MODE);
	setChatMode(g_plugin.getByte(dbsName, FALSE));

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_CHAT_FLAGS);
	setChatFlags(g_plugin.getDword(dbsName, 16128));

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_CHAT_BACKGROUND);
	if (!g_plugin.getString(dbsName, &dbv)) {
		if (strncmp(tmpPath, "http://", 7))
			PathToAbsolute(dbv.pszVal, tmpPath);

		setChatBackgroundFilename(tmpPath);
		db_free(&dbv);
	}

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_CHAT_CSS);
	if (!g_plugin.getString(dbsName, &dbv)) {
		if (strncmp(tmpPath, "http://", 7))
			PathToAbsolute(dbv.pszVal, tmpPath);

		setChatCssFilename(tmpPath);
		db_free(&dbv);
	}

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_CHAT_TEMPLATE);
	if (!g_plugin.getString(dbsName, &dbv)) {
		PathToAbsolute(dbv.pszVal, tmpPath);
		setChatTemplateFilename(tmpPath);
		db_free(&dbv);
	}

	/* History settings */
	mir_snprintf(dbsName, "%s.%s", szProto, DBS_HISTORY_ENABLE);
	setHistoryEnable(protocolName == nullptr ? true : 0 != g_plugin.getByte(dbsName, FALSE));

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_HISTORY_MODE);
	setHistoryMode(g_plugin.getByte(dbsName, FALSE));

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_HISTORY_FLAGS);
	setHistoryFlags(g_plugin.getDword(dbsName, 16128));

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_HISTORY_BACKGROUND);
	if (!g_plugin.getString(dbsName, &dbv)) {
		if (strncmp(tmpPath, "http://", 7))
			PathToAbsolute(dbv.pszVal, tmpPath);

		setHistoryBackgroundFilename(tmpPath);
		db_free(&dbv);
	}

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_HISTORY_CSS);
	if (!g_plugin.getString(dbsName, &dbv)) {
		if (strncmp(tmpPath, "http://", 7))
			PathToAbsolute(dbv.pszVal, tmpPath);

		setHistoryCssFilename(tmpPath);
		db_free(&dbv);
	}

	mir_snprintf(dbsName, "%s.%s", szProto, DBS_HISTORY_TEMPLATE);
	if (!g_plugin.getString(dbsName, &dbv)) {
		PathToAbsolute(dbv.pszVal, tmpPath);
		setHistoryTemplateFilename(tmpPath);
		db_free(&dbv);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void ProtocolSettings::setSRMMTemplateFilename(const char *filename)
{
	srmmTemplateFilename = filename;
	TemplateMap::loadTemplates(getSRMMTemplateFilename(), getSRMMTemplateFilename(), false);
}

void ProtocolSettings::setChatTemplateFilename(const char *filename)
{
	chatTemplateFilename = filename;
	TemplateMap::loadTemplates(getChatTemplateFilename(), getChatTemplateFilename(), false);
}

void ProtocolSettings::setHistoryTemplateFilename(const char *filename)
{
	historyTemplateFilename = filename;
	TemplateMap::loadTemplates(getHistoryTemplateFilename(), getHistoryTemplateFilename(), false);
}

/////////////////////////////////////////////////////////////////////////////////////////

void Options::init()
{
	if (isInited) return;
	isInited = true;

	generalFlags = g_plugin.getDword(DBS_BASICFLAGS, 13);

	auto *proto = new ProtocolSettings(nullptr);
	proto->readFromDb();
	arProtos.insert(proto);

	for (auto &it : Accounts()) {

		if (!mir_strcmp(it->szModuleName, META_PROTO))
			continue;
		if ((CallProtoService(it->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM) == 0)
			continue;

		proto = new ProtocolSettings(it->szModuleName);
		proto->readFromDb();
		arProtos.insert(proto);
	}

	bHasAvs = ServiceExists(MS_AV_GETAVATARBITMAP) != 0;
	bHasSmileyAdd = ServiceExists(MS_SMILEYADD_BATCHPARSE) != 0;
}

void Options::uninit()
{
	arProtos.destroy();

	TemplateMap::dropTemplates();
}

void Options::setEmbedSize(int size)
{
	g_plugin.setDword("Embedsize", (uint32_t)size);
}

int Options::getEmbedSize()
{
	return g_plugin.getDword("Embedsize", 0);
}

ProtocolSettings* Options::getDefaultSettings()
{
	return &arProtos[0];
}

ProtocolSettings* Options::getProtocolSettings(const char *protocolName)
{
	for (auto &it : arProtos)
		if (!mir_strcmpi(it->getProtocolName(), protocolName))
			return it;

	return nullptr;
}

void Options::saveProtocolSettings()
{
	for (auto &it : arTemp) {
		const char *szProto = it->getProtocolName();
		if (szProto == nullptr)
			szProto = "_default_";

		/* SRMM settings */
		char dbsName[256], tmpPath[MAX_PATH];
		mir_snprintf(dbsName, "%s.%s", szProto, DBS_SRMM_ENABLE);
		g_plugin.setByte(dbsName, it->isSRMMEnable());
		
		mir_snprintf(dbsName, "%s.%s", szProto, DBS_SRMM_MODE);
		g_plugin.setByte(dbsName, it->getSRMMMode());
		
		mir_snprintf(dbsName, "%s.%s", szProto, DBS_SRMM_FLAGS);
		g_plugin.setDword(dbsName, it->getSRMMFlags());
		
		mir_snprintf(dbsName, "%s.%s", szProto, DBS_SRMM_BACKGROUND);
		PathToRelative(it->getSRMMBackgroundFilename(), tmpPath);
		g_plugin.setString(dbsName, tmpPath);

		mir_snprintf(dbsName, "%s.%s", szProto, DBS_SRMM_CSS);
		PathToRelative(it->getSRMMCssFilename(), tmpPath);
		g_plugin.setString(dbsName, tmpPath);

		mir_snprintf(dbsName, "%s.%s", szProto, DBS_SRMM_TEMPLATE);
		PathToRelative(it->getSRMMTemplateFilename(), tmpPath);
		g_plugin.setString(dbsName, tmpPath);

		/* Group Chat settings */
		mir_snprintf(dbsName, "%s.%s", szProto, DBS_CHAT_ENABLE);
		g_plugin.setByte(dbsName, it->isChatEnable());
		
		mir_snprintf(dbsName, "%s.%s", szProto, DBS_CHAT_MODE);
		g_plugin.setByte(dbsName, it->getChatMode());
		
		mir_snprintf(dbsName, "%s.%s", szProto, DBS_CHAT_FLAGS);
		g_plugin.setDword(dbsName, it->getChatFlags());
		
		mir_snprintf(dbsName, "%s.%s", szProto, DBS_CHAT_BACKGROUND);
		PathToRelative(it->getChatBackgroundFilename(), tmpPath);
		g_plugin.setString(dbsName, tmpPath);

		mir_snprintf(dbsName, "%s.%s", szProto, DBS_CHAT_CSS);
		PathToRelative(it->getChatCssFilename(), tmpPath);
		g_plugin.setString(dbsName, tmpPath);

		mir_snprintf(dbsName, "%s.%s", szProto, DBS_CHAT_TEMPLATE);
		PathToRelative(it->getChatTemplateFilename(), tmpPath);
		g_plugin.setString(dbsName, tmpPath);

		/* History settings */
		mir_snprintf(dbsName, "%s.%s", szProto, DBS_HISTORY_ENABLE);
		g_plugin.setByte(dbsName, it->isHistoryEnable());

		mir_snprintf(dbsName, "%s.%s", szProto, DBS_HISTORY_MODE);
		g_plugin.setByte(dbsName, it->getHistoryMode());

		mir_snprintf(dbsName, "%s.%s", szProto, DBS_HISTORY_FLAGS);
		g_plugin.setDword(dbsName, it->getHistoryFlags());

		mir_snprintf(dbsName, "%s.%s", szProto, DBS_HISTORY_BACKGROUND);
		PathToRelative(it->getHistoryBackgroundFilename(), tmpPath);
		g_plugin.setString(dbsName, tmpPath);

		mir_snprintf(dbsName, "%s.%s", szProto, DBS_HISTORY_CSS);
		PathToRelative(it->getHistoryCssFilename(), tmpPath);
		g_plugin.setString(dbsName, tmpPath);

		mir_snprintf(dbsName, "%s.%s", szProto, DBS_HISTORY_TEMPLATE);
		PathToRelative(it->getHistoryTemplateFilename(), tmpPath);
		g_plugin.setString(dbsName, tmpPath);
	}

	reload();
}

void Options::reload()
{
	arProtos.destroy();
	isInited = false;
	init();
}
