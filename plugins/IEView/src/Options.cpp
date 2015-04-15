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

#include "ieview_common.h"

#define UM_CHECKSTATECHANGE (WM_USER+100)
HANDLE hHookOptionsChanged;
static INT_PTR CALLBACK IEViewGeneralOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK IEViewSRMMOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK IEViewGroupChatsOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK IEViewHistoryOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
#define OPTIONS_PAGES 4
static ProtocolSettings *srmmCurrentProtoItem = NULL;
static ProtocolSettings *chatCurrentProtoItem = NULL;
static ProtocolSettings *historyCurrentProtoItem = NULL;
static HIMAGELIST hProtocolImageList = NULL;
static HIMAGELIST hImageList = NULL;

struct
{
	DLGPROC dlgProc;
	DWORD dlgId;
	TCHAR *tabName;
}
static tabPages[] =
{
	{ IEViewGeneralOptDlgProc, IDD_GENERAL_OPTIONS, LPGENT("General") },
	{ IEViewSRMMOptDlgProc, IDD_SRMM_OPTIONS, LPGENT("Message Log") },
	{ IEViewGroupChatsOptDlgProc, IDD_SRMM_OPTIONS, LPGENT("Group chats") },
	{ IEViewHistoryOptDlgProc, IDD_SRMM_OPTIONS, LPGENT("History") }
};

static LPARAM GetItemParam(HWND hwndTreeView, HTREEITEM hItem)
{
	TVITEM tvi = { 0 };
	tvi.mask = TVIF_PARAM;
	tvi.hItem = hItem == NULL ? TreeView_GetSelection(hwndTreeView) : hItem;
	TreeView_GetItem(hwndTreeView, &tvi);
	return tvi.lParam;
}

static void SaveSRMMProtoSettings(HWND hwndDlg, ProtocolSettings *proto)
{
	if (proto != NULL) {
		char path[MAX_PATH];
		int i = Options::MODE_COMPATIBLE;
		if (IsDlgButtonChecked(hwndDlg, IDC_MODE_TEMPLATE))
			i = Options::MODE_TEMPLATE;
		else if (IsDlgButtonChecked(hwndDlg, IDC_MODE_CSS))
			i = Options::MODE_CSS;

		proto->setSRMMModeTemp(i);
		i = IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE) ? Options::LOG_IMAGE_ENABLED : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE) ? Options::LOG_IMAGE_SCROLL : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_NICKNAMES) ? Options::LOG_SHOW_NICKNAMES : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_TIME) ? Options::LOG_SHOW_TIME : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_DATE) ? Options::LOG_SHOW_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_SECONDS) ? Options::LOG_SHOW_SECONDS : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_LONG_DATE) ? Options::LOG_LONG_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_RELATIVE_DATE) ? Options::LOG_RELATIVE_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_GROUP_MESSAGES) ? Options::LOG_GROUP_MESSAGES : 0;
		proto->setSRMMFlagsTemp(i);
		GetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, SIZEOF(path));
		proto->setSRMMBackgroundFilenameTemp(path);
		GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, SIZEOF(path));
		proto->setSRMMCssFilenameTemp(path);
		GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path, SIZEOF(path));
		proto->setSRMMTemplateFilenameTemp(path);
	}
}

static void SaveChatProtoSettings(HWND hwndDlg, ProtocolSettings *proto)
{
	if (proto != NULL) {
		char path[MAX_PATH];
		int i = Options::MODE_COMPATIBLE;
		if (IsDlgButtonChecked(hwndDlg, IDC_MODE_TEMPLATE))
			i = Options::MODE_TEMPLATE;
		else if (IsDlgButtonChecked(hwndDlg, IDC_MODE_CSS))
			i = Options::MODE_CSS;

		proto->setChatModeTemp(i);
		i = IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE) ? Options::LOG_IMAGE_ENABLED : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE) ? Options::LOG_IMAGE_SCROLL : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_NICKNAMES) ? Options::LOG_SHOW_NICKNAMES : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_TIME) ? Options::LOG_SHOW_TIME : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_DATE) ? Options::LOG_SHOW_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_SECONDS) ? Options::LOG_SHOW_SECONDS : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_LONG_DATE) ? Options::LOG_LONG_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_RELATIVE_DATE) ? Options::LOG_RELATIVE_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_GROUP_MESSAGES) ? Options::LOG_GROUP_MESSAGES : 0;
		proto->setChatFlagsTemp(i);
		GetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, SIZEOF(path));
		proto->setChatBackgroundFilenameTemp(path);
		GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, SIZEOF(path));
		proto->setChatCssFilenameTemp(path);
		GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path, SIZEOF(path));
		proto->setChatTemplateFilenameTemp(path);
	}
}

static void SaveHistoryProtoSettings(HWND hwndDlg, ProtocolSettings *proto)
{
	if (proto != NULL) {
		char path[MAX_PATH];
		int i = Options::MODE_COMPATIBLE;
		if (IsDlgButtonChecked(hwndDlg, IDC_MODE_TEMPLATE))
			i = Options::MODE_TEMPLATE;
		else if (IsDlgButtonChecked(hwndDlg, IDC_MODE_CSS))
			i = Options::MODE_CSS;

		proto->setHistoryModeTemp(i);
		i = IsDlgButtonChecked(hwndDlg, IDC_BACKGROUND_IMAGE) ? Options::LOG_IMAGE_ENABLED : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE) ? Options::LOG_IMAGE_SCROLL : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_NICKNAMES) ? Options::LOG_SHOW_NICKNAMES : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_TIME) ? Options::LOG_SHOW_TIME : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_DATE) ? Options::LOG_SHOW_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_SHOW_SECONDS) ? Options::LOG_SHOW_SECONDS : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_LONG_DATE) ? Options::LOG_LONG_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_RELATIVE_DATE) ? Options::LOG_RELATIVE_DATE : 0;
		i |= IsDlgButtonChecked(hwndDlg, IDC_LOG_GROUP_MESSAGES) ? Options::LOG_GROUP_MESSAGES : 0;
		proto->setHistoryFlagsTemp(i);
		GetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path, SIZEOF(path));
		proto->setHistoryBackgroundFilenameTemp(path);
		GetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path, SIZEOF(path));
		proto->setHistoryCssFilenameTemp(path);
		GetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path, SIZEOF(path));
		proto->setHistoryTemplateFilenameTemp(path);
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

static void SetIcon(HWND hwnd, DWORD id, int index, bool condition)
{
	HICON hIcon;
	if (condition)
		hIcon = ImageList_GetIcon(hImageList, index + 1, ILD_NORMAL);
	else
		hIcon = ImageList_GetIcon(hImageList, index + 0, ILD_NORMAL);

	hIcon = (HICON)SendDlgItemMessage(hwnd, id, STM_SETICON, (WPARAM)hIcon, 0);
	if (hIcon != NULL)
		DestroyIcon(hIcon);
}

static void UpdateTemplateIcons(HWND hwnd, const char *path)
{
	TemplateMap *tmap = TemplateMap::loadTemplates(path, path, true);
	if (tmap != NULL) {
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
	if (proto != NULL) {
		HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
		TreeView_SetCheckState(hProtoList, TreeView_GetSelection(hProtoList), proto->isSRMMEnableTemp());
		CheckDlgButton(hwndDlg, IDC_MODE_TEMPLATE, proto->getSRMMModeTemp() == Options::MODE_TEMPLATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MODE_CSS, proto->getSRMMModeTemp() == Options::MODE_CSS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MODE_COMPATIBLE, proto->getSRMMModeTemp() == Options::MODE_COMPATIBLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_BACKGROUND_IMAGE, proto->getSRMMFlagsTemp() & Options::LOG_IMAGE_ENABLED ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE, proto->getSRMMFlagsTemp() & Options::LOG_IMAGE_SCROLL ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_NICKNAMES, proto->getSRMMFlagsTemp() & Options::LOG_SHOW_NICKNAMES ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_TIME, proto->getSRMMFlagsTemp() & Options::LOG_SHOW_TIME ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_DATE, proto->getSRMMFlagsTemp() & Options::LOG_SHOW_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_SECONDS, proto->getSRMMFlagsTemp() & Options::LOG_SHOW_SECONDS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_LONG_DATE, proto->getSRMMFlagsTemp() & Options::LOG_LONG_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_RELATIVE_DATE, proto->getSRMMFlagsTemp() & Options::LOG_RELATIVE_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_GROUP_MESSAGES, proto->getSRMMFlagsTemp() & Options::LOG_GROUP_MESSAGES ? BST_CHECKED : BST_UNCHECKED);
		if (proto->getSRMMBackgroundFilenameTemp() != NULL) {
			SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, proto->getSRMMBackgroundFilenameTemp());
		}
		else {
			SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, "");
		}
		if (proto->getSRMMCssFilename() != NULL) {
			SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, proto->getSRMMCssFilenameTemp());
		}
		else {
			SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, "");
		}
		if (proto->getSRMMTemplateFilenameTemp() != NULL) {
			SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, proto->getSRMMTemplateFilenameTemp());
		}
		else {
			SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, "");
		}
		UpdateTemplateIcons(hwndDlg, proto->getSRMMTemplateFilenameTemp());
		srmmCurrentProtoItem = proto;
		UpdateControlsState(hwndDlg);
	}
}

static void UpdateChatProtoInfo(HWND hwndDlg, ProtocolSettings *proto)
{
	if (proto != NULL) {
		HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
		TreeView_SetCheckState(hProtoList, TreeView_GetSelection(hProtoList), proto->isChatEnableTemp());
		CheckDlgButton(hwndDlg, IDC_MODE_TEMPLATE, proto->getChatModeTemp() == Options::MODE_TEMPLATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MODE_CSS, proto->getChatModeTemp() == Options::MODE_CSS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MODE_COMPATIBLE, proto->getChatModeTemp() == Options::MODE_COMPATIBLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_BACKGROUND_IMAGE, proto->getChatFlagsTemp() & Options::LOG_IMAGE_ENABLED ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE, proto->getChatFlagsTemp() & Options::LOG_IMAGE_SCROLL ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_NICKNAMES, proto->getChatFlagsTemp() & Options::LOG_SHOW_NICKNAMES ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_TIME, proto->getChatFlagsTemp() & Options::LOG_SHOW_TIME ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_DATE, proto->getChatFlagsTemp() & Options::LOG_SHOW_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_SECONDS, proto->getChatFlagsTemp() & Options::LOG_SHOW_SECONDS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_LONG_DATE, proto->getChatFlagsTemp() & Options::LOG_LONG_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_RELATIVE_DATE, proto->getChatFlagsTemp() & Options::LOG_RELATIVE_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_GROUP_MESSAGES, proto->getChatFlagsTemp() & Options::LOG_GROUP_MESSAGES ? BST_CHECKED : BST_UNCHECKED);
		if (proto->getChatBackgroundFilenameTemp() != NULL) {
			SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, proto->getChatBackgroundFilenameTemp());
		}
		else {
			SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, "");
		}
		if (proto->getChatCssFilename() != NULL) {
			SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, proto->getChatCssFilenameTemp());
		}
		else {
			SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, "");
		}
		if (proto->getChatTemplateFilenameTemp() != NULL) {
			SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, proto->getChatTemplateFilenameTemp());
		}
		else {
			SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, "");
		}
		UpdateTemplateIcons(hwndDlg, proto->getChatTemplateFilenameTemp());
		chatCurrentProtoItem = proto;
		UpdateControlsState(hwndDlg);
	}
}

static void UpdateHistoryProtoInfo(HWND hwndDlg, ProtocolSettings *proto)
{
	if (proto != NULL) {
		HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
		TreeView_SetCheckState(hProtoList, TreeView_GetSelection(hProtoList), proto->isHistoryEnableTemp());
		CheckDlgButton(hwndDlg, IDC_MODE_TEMPLATE, proto->getHistoryModeTemp() == Options::MODE_TEMPLATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MODE_CSS, proto->getHistoryModeTemp() == Options::MODE_CSS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MODE_COMPATIBLE, proto->getHistoryModeTemp() == Options::MODE_COMPATIBLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_BACKGROUND_IMAGE, proto->getHistoryFlagsTemp() & Options::LOG_IMAGE_ENABLED ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SCROLL_BACKGROUND_IMAGE, proto->getHistoryFlagsTemp() & Options::LOG_IMAGE_SCROLL ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_NICKNAMES, proto->getHistoryFlagsTemp() & Options::LOG_SHOW_NICKNAMES ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_TIME, proto->getHistoryFlagsTemp() & Options::LOG_SHOW_TIME ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_DATE, proto->getHistoryFlagsTemp() & Options::LOG_SHOW_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_SHOW_SECONDS, proto->getHistoryFlagsTemp() & Options::LOG_SHOW_SECONDS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_LONG_DATE, proto->getHistoryFlagsTemp() & Options::LOG_LONG_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_RELATIVE_DATE, proto->getHistoryFlagsTemp() & Options::LOG_RELATIVE_DATE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOG_GROUP_MESSAGES, proto->getHistoryFlagsTemp() & Options::LOG_GROUP_MESSAGES ? BST_CHECKED : BST_UNCHECKED);
		if (proto->getHistoryBackgroundFilenameTemp() != NULL) {
			SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, proto->getHistoryBackgroundFilenameTemp());
		}
		else {
			SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, "");
		}
		if (proto->getHistoryCssFilename() != NULL) {
			SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, proto->getHistoryCssFilenameTemp());
		}
		else {
			SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, "");
		}
		if (proto->getHistoryTemplateFilenameTemp() != NULL) {
			SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, proto->getHistoryTemplateFilenameTemp());
		}
		else {
			SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, "");
		}
		UpdateTemplateIcons(hwndDlg, proto->getHistoryTemplateFilenameTemp());
		historyCurrentProtoItem = proto;
		UpdateControlsState(hwndDlg);
	}
}

static void RefreshProtoIcons()
{
	int i;
	ProtocolSettings *proto;
	if (hProtocolImageList != NULL) {
		ImageList_RemoveAll(hProtocolImageList);
	}
	else {
		for (i = 0, proto = Options::getProtocolSettings(); proto != NULL; proto = proto->getNext(), i++);
		hProtocolImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
			ILC_MASK | ILC_COLOR32, i, 0);
	}
	for (i = 0, proto = Options::getProtocolSettings(); proto != NULL; proto = proto->getNext(), i++) {
		HICON hIcon = NULL;
		if (i > 0) {
			hIcon = (HICON)CallProtoService(proto->getProtocolName(), PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
			if (hIcon == NULL)  {
				hIcon = (HICON)CallProtoService(proto->getProtocolName(), PS_LOADICON, PLI_PROTOCOL, 0);
			}
			ImageList_AddIcon(hProtocolImageList, hIcon);
			DestroyIcon(hIcon);
		}
		if (hIcon == NULL) {
			hIcon = (HICON)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
			ImageList_AddIcon(hProtocolImageList, hIcon);
			Skin_ReleaseIcon(hIcon);
		}
	}
}

static void RefreshIcons()
{
	if (hImageList != NULL) {
		ImageList_RemoveAll(hImageList);
	}
	else {
		hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK | ILC_COLOR32, 0, 0);
	}
	ImageList_AddIcon(hImageList, (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_GROUP_OFF), IMAGE_ICON, 0, 0, 0));
	ImageList_AddIcon(hImageList, (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_GROUP_ON), IMAGE_ICON, 0, 0, 0));
	ImageList_AddIcon(hImageList, (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_RTL_OFF), IMAGE_ICON, 0, 0, 0));
	ImageList_AddIcon(hImageList, (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_RTL_ON), IMAGE_ICON, 0, 0, 0));
}

static void RefreshProtoList(HWND hwndDlg, int mode, bool protoTemplates)
{
	int i;
	HTREEITEM hItem = NULL;
	HWND hProtoList = GetDlgItem(hwndDlg, IDC_PROTOLIST);
	TreeView_DeleteAllItems(hProtoList);
	TreeView_SetImageList(hProtoList, hProtocolImageList, TVSIL_NORMAL);
	ProtocolSettings *proto;
	for (i = 0, proto = Options::getProtocolSettings(); proto != NULL; proto = proto->getNext(), i++) {
		char protoName[128];
		TVINSERTSTRUCT tvi = { 0 };
		tvi.hParent = TVI_ROOT;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_STATE | TVIF_SELECTEDIMAGE;
		tvi.item.stateMask = TVIS_SELECTED | TVIS_STATEIMAGEMASK;
		if (i == 0) {
			strcpy(protoName, Translate("Default"));
		}
		else {
			CallProtoService(proto->getProtocolName(), PS_GETNAME, sizeof(protoName), (LPARAM)protoName);
			//			strcat(protoName, " ");
			//		strcat(protoName, Translate("protocol"));
		}
		tvi.item.pszText = mir_a2t(protoName);
		tvi.item.lParam = (LPARAM)proto;
		tvi.item.iImage = i;
		tvi.item.iSelectedImage = i;
		switch (mode) {
		case 0:
			tvi.item.state = INDEXTOSTATEIMAGEMASK(proto->isSRMMEnableTemp() ? 2 : 1);
			break;
		case 1:
			tvi.item.state = INDEXTOSTATEIMAGEMASK(proto->isChatEnableTemp() ? 2 : 1);
			break;
		case 2:
			tvi.item.state = INDEXTOSTATEIMAGEMASK(proto->isHistoryEnableTemp() ? 2 : 1);
			break;
		}
		if (i == 0) {
			hItem = TreeView_InsertItem(hProtoList, &tvi);
		}
		else {
			TreeView_InsertItem(hProtoList, &tvi);
		}
		if (!protoTemplates) break;
	}
	//	UpdateSRMMProtoInfo(hwndDlg, Options::getProtocolSettings());
	TreeView_SelectItem(hProtoList, hItem);
}

static bool BrowseFile(HWND hwndDlg, char *filter, char *defExt, char *path, int maxLen)
{
	OPENFILENAMEA ofn = { 0 };
	GetWindowTextA(hwndDlg, path, maxLen);
	ofn.lStructSize = sizeof(OPENFILENAME);//_SIZE_VERSION_400;
	ofn.hwndOwner = hwndDlg;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = filter;//"Templates (*.ivt)\0*.ivt\0All Files\0*.*\0\0";
	ofn.lpstrFile = path;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.nMaxFile = maxLen;
	ofn.nMaxFileTitle = maxLen;
	ofn.lpstrDefExt = defExt;//"ivt";
	if (GetOpenFileNameA(&ofn)) {
		SetWindowTextA(hwndDlg, path);
		return true;
	}
	return false;
}

int IEViewOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInstance;
	odp.ptszGroup = LPGENT("Message sessions");
	odp.ptszTitle = LPGENT("IEView");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.pszTemplate = MAKEINTRESOURCEA(tabPages[0].dlgId);
	odp.pfnDlgProc = tabPages[0].dlgProc;
	odp.ptszTab = tabPages[0].tabName;
	Options_AddPage(wParam, &odp);

	odp.ptszGroup = LPGENT("Skins");
	odp.ptszTitle = LPGENT("IEView");
	for (size_t i = 1; i < SIZEOF(tabPages); i++) {
		odp.pszTemplate = MAKEINTRESOURCEA(tabPages[i].dlgId);
		odp.pfnDlgProc = tabPages[i].dlgProc;
		odp.ptszTab = tabPages[i].tabName;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}

static int initialized = 0;
static int changed = 0;

static void MarkInitialized(int i)
{
	if (initialized == 0) {
		Options::resetProtocolSettings();
		RefreshProtoIcons();
		RefreshIcons();
	}
	initialized |= i;
}

static void ApplyChanges(int i)
{
	changed &= ~i;
	initialized &= ~i;
	if (changed == 0) {
		Options::saveProtocolSettings();
		NotifyEventHooks(hHookOptionsChanged, 0, 0);
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
	{
		MarkInitialized(1);
		TranslateDialogDefault(hwndDlg);
		if (Options::getGeneralFlags() & Options::GENERAL_ENABLE_BBCODES) {
			CheckDlgButton(hwndDlg, IDC_ENABLE_BBCODES, BST_CHECKED);
		}
		if (Options::getGeneralFlags() & Options::GENERAL_ENABLE_FLASH) {
			CheckDlgButton(hwndDlg, IDC_ENABLE_FLASH, BST_CHECKED);
		}
		if (Options::getGeneralFlags() & Options::GENERAL_SMILEYINNAMES) {
			CheckDlgButton(hwndDlg, IDC_SMILEYS_IN_NAMES, BST_CHECKED);
		}
		if (Options::getGeneralFlags() & Options::GENERAL_NO_BORDER) {
			CheckDlgButton(hwndDlg, IDC_NO_BORDER, BST_CHECKED);
		}
		if (Options::getGeneralFlags() & Options::GENERAL_ENABLE_EMBED) {
			CheckDlgButton(hwndDlg, IDC_ENABLE_EMBED, BST_CHECKED);
		}
		EnableWindow(GetDlgItem(hwndDlg, IDC_SMILEYS_IN_NAMES), Options::isSmileyAdd());
		EnableWindow(GetDlgItem(hwndDlg, IDC_EMBED_SIZE), IsDlgButtonChecked(hwndDlg, IDC_ENABLE_EMBED));
		TCHAR* size[] = { _T("320 x 205"), _T("480 x 385"), _T("560 x 349"), _T("640 x 390") };
		for (int i = 0; i < SIZEOF(size); ++i){
			int item = SendDlgItemMessage(hwndDlg, IDC_EMBED_SIZE, CB_ADDSTRING, 0, (LPARAM)TranslateTS(size[i]));
			SendDlgItemMessage(hwndDlg, IDC_EMBED_SIZE, CB_SETITEMDATA, item, 0);
		}
		SendDlgItemMessage(hwndDlg, IDC_EMBED_SIZE, CB_SETCURSEL, Options::getEmbedsize(), 0);
		return TRUE;
	}
	case WM_COMMAND:
	{
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
	}
	break;
	case WM_NOTIFY:
	{
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			int i = 0;
			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_BBCODES)) {
				i |= Options::GENERAL_ENABLE_BBCODES;
			}
			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_FLASH)) {
				i |= Options::GENERAL_ENABLE_FLASH;
			}
			if (IsDlgButtonChecked(hwndDlg, IDC_SMILEYS_IN_NAMES)) {
				i |= Options::GENERAL_SMILEYINNAMES;
			}
			if (IsDlgButtonChecked(hwndDlg, IDC_NO_BORDER)) {
				i |= Options::GENERAL_NO_BORDER;
			}
			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_EMBED)) {
				i |= Options::GENERAL_ENABLE_EMBED;
			}
			Options::setGeneralFlags(i);
			ApplyChanges(1);
			Options::setEmbedsize(SendDlgItemMessage(hwndDlg, IDC_EMBED_SIZE, CB_GETCURSEL, 0, 0));
			return TRUE;
		}
	}
	break;
	case WM_DESTROY:
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
	{
		MarkInitialized(2);
		TranslateDialogDefault(hwndDlg);
		srmmCurrentProtoItem = NULL;
		RefreshProtoList(hwndDlg, 0, true);
		return TRUE;
	}
	case WM_COMMAND:
	{
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
			mir_snprintf(filter, SIZEOF(filter), "%s (*.ivt)%c*.ivt%c%s (*.*)%c*.*%c%c", Translate("Template"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "ivt", path, SIZEOF(path))) {
				SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path);
				UpdateTemplateIcons(hwndDlg, path);
				MarkChanges(2, hwndDlg);
			}
			break;
		case IDC_BROWSE_BACKGROUND_IMAGE:
			mir_snprintf(filter, SIZEOF(filter), "%s (*.jpg,*.jpeg,*.gif,*.png,*.bmp)%c*.jpg;*.jpeg;*.gif;*.png;*.bmp%c%s (*.*)%c*.*%c%c", Translate("All Images"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "jpg", path, SIZEOF(path))) {
				SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path);
				MarkChanges(2, hwndDlg);
			}
			break;
		case IDC_BROWSE_EXTERNALCSS:
			mir_snprintf(filter, SIZEOF(filter), "%s (*.css)%c*.css%c%s (*.*)%c*.*%c%c", Translate("Style Sheet"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "css", path, SIZEOF(path))) {
				SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path);
				MarkChanges(2, hwndDlg);
			}
			break;
		case IDC_BROWSE_EXTERNALCSS_RTL:
			mir_snprintf(filter, SIZEOF(filter), "%s (*.css)%c*.css%c%s (*.*)%c*.*%c%c", Translate("Style Sheet"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "css", path, SIZEOF(path))) {
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
		}
	}
	break;
	case UM_CHECKSTATECHANGE:
	{
		ProtocolSettings *proto = (ProtocolSettings *)GetItemParam((HWND)wParam, (HTREEITEM)lParam);
		if (proto != NULL)
			if (strcmpi(proto->getProtocolName(), "_default_"))
				proto->setSRMMEnableTemp(0 != TreeView_GetCheckState((HWND)wParam, (HTREEITEM)lParam));

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
				DWORD dwpos = GetMessagePos();
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
			case TVN_SELCHANGEDA:
			case TVN_SELCHANGEDW:
			{
				ProtocolSettings *proto = (ProtocolSettings *)GetItemParam(GetDlgItem(hwndDlg, IDC_PROTOLIST), (HTREEITEM)NULL);
				SaveSRMMProtoSettings(hwndDlg, srmmCurrentProtoItem);
				UpdateSRMMProtoInfo(hwndDlg, proto);
			}
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
		historyCurrentProtoItem = NULL;
		RefreshProtoList(hwndDlg, 2, true);
		return TRUE;

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
			mir_snprintf(filter, SIZEOF(filter), "%s (*.ivt)%c*.ivt%c%s%c*.*%c%c", Translate("Template"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "ivt", path, SIZEOF(path))) {
				SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path);
				UpdateTemplateIcons(hwndDlg, path);
				MarkChanges(4, hwndDlg);
			}
			break;
		case IDC_BROWSE_BACKGROUND_IMAGE:
			mir_snprintf(filter, SIZEOF(filter), "%s (*.jpg,*.gif,*.png,*.bmp)%c*.ivt%c%s%c*.*%c%c", Translate("All Images"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "jpg", path, SIZEOF(path))) {
				SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path);
				MarkChanges(4, hwndDlg);
			}
			break;
		case IDC_BROWSE_EXTERNALCSS:
			mir_snprintf(filter, SIZEOF(filter), "%s (*.css)%c*.ivt%c%s%c*.*%c%c", Translate("Style Sheet"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "css", path, SIZEOF(path))) {
				SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path);
				MarkChanges(4, hwndDlg);
			}
			break;
		case IDC_BROWSE_EXTERNALCSS_RTL:
			mir_snprintf(filter, SIZEOF(filter), "%s (*.css)%c*.ivt%c%s%c*.*%c%c", Translate("Style Sheet"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "css", path, SIZEOF(path))) {
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
		if (proto != NULL)
			if (strcmpi(proto->getProtocolName(), "_default_"))
				proto->setHistoryEnableTemp(0 != TreeView_GetCheckState((HWND)wParam, (HTREEITEM)lParam));

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
				TVHITTESTINFO ht = { 0 };
				DWORD dwpos = GetMessagePos();
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
			case TVN_SELCHANGEDA:
			case TVN_SELCHANGEDW:
			{
				ProtocolSettings *proto = (ProtocolSettings *)GetItemParam(GetDlgItem(hwndDlg, IDC_PROTOLIST), (HTREEITEM)NULL);
				SaveHistoryProtoSettings(hwndDlg, historyCurrentProtoItem);
				UpdateHistoryProtoInfo(hwndDlg, proto);
			}
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
		chatCurrentProtoItem = NULL;
		RefreshProtoList(hwndDlg, 1, true);
		return TRUE;

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
			mir_snprintf(filter, SIZEOF(filter), "%s (*.ivt)%c*.ivt%c%s%c*.*%c%c", Translate("Template"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "ivt", path, SIZEOF(path))) {
				SetDlgItemTextA(hwndDlg, IDC_TEMPLATES_FILENAME, path);
				UpdateTemplateIcons(hwndDlg, path);
				MarkChanges(8, hwndDlg);
			}
			break;
		case IDC_BROWSE_BACKGROUND_IMAGE:
			mir_snprintf(filter, SIZEOF(filter), "%s (*.jpg,*.gif,*.png,*.bmp)%c*.ivt%c%s%c*.*%c%c", Translate("All Images"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "jpg", path, SIZEOF(path))) {
				SetDlgItemTextA(hwndDlg, IDC_BACKGROUND_IMAGE_FILENAME, path);
				MarkChanges(8, hwndDlg);
			}
			break;
		case IDC_BROWSE_EXTERNALCSS:
			mir_snprintf(filter, SIZEOF(filter), "%s (*.css)%c*.ivt%c%s%c*.*%c%c", Translate("Style Sheet"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "css", path, SIZEOF(path))) {
				SetDlgItemTextA(hwndDlg, IDC_EXTERNALCSS_FILENAME, path);
				MarkChanges(8, hwndDlg);
			}
			break;
		case IDC_BROWSE_EXTERNALCSS_RTL:
			mir_snprintf(filter, SIZEOF(filter), "%s (*.css)%c*.ivt%c%s%c*.*%c%c", Translate("Style Sheet"), 0, 0, Translate("All Files"), 0, 0, 0);
			if (BrowseFile(hwndDlg, filter, "css", path, SIZEOF(path))) {
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
		ProtocolSettings *proto = (ProtocolSettings *)GetItemParam((HWND)wParam, (HTREEITEM)lParam);
		if (proto != NULL)
			if (strcmpi(proto->getProtocolName(), "_default_"))
				proto->setChatEnableTemp(0 != TreeView_GetCheckState((HWND)wParam, (HTREEITEM)lParam));

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
				TVHITTESTINFO ht = { 0 };
				DWORD dwpos = GetMessagePos();
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
			case TVN_SELCHANGEDA:
			case TVN_SELCHANGEDW:
			{
				ProtocolSettings *proto = (ProtocolSettings *)GetItemParam(GetDlgItem(hwndDlg, IDC_PROTOLIST), (HTREEITEM)NULL);
				SaveChatProtoSettings(hwndDlg, chatCurrentProtoItem);
				UpdateChatProtoInfo(hwndDlg, proto);
			}
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

bool Options::isInited = false;
bool Options::bSmileyAdd = false;
int  Options::avatarServiceFlags = 0;
int  Options::generalFlags;

ProtocolSettings *Options::protocolList = NULL;

ProtocolSettings::ProtocolSettings(const char *protocolName)
{
	this->protocolName = mir_strdup(protocolName);
	next = NULL;
	srmmEnable = false;
	srmmMode = Options::MODE_COMPATIBLE;
	srmmFlags = 0;
	srmmBackgroundFilename = mir_strdup("");
	srmmCssFilename = mir_strdup("");
	srmmTemplateFilename = mir_strdup("");

	srmmBackgroundFilenameTemp = mir_strdup("");
	srmmCssFilenameTemp = mir_strdup("");
	srmmTemplateFilenameTemp = mir_strdup("");

	chatEnable = false;
	chatMode = Options::MODE_COMPATIBLE;
	chatFlags = 0;
	chatBackgroundFilename = mir_strdup("");
	chatCssFilename = mir_strdup("");
	chatTemplateFilename = mir_strdup("");

	chatBackgroundFilenameTemp = mir_strdup("");
	chatCssFilenameTemp = mir_strdup("");
	chatTemplateFilenameTemp = mir_strdup("");

	historyEnable = false;
	historyMode = Options::MODE_COMPATIBLE;
	historyFlags = 0;
	historyBackgroundFilename = mir_strdup("");
	historyCssFilename = mir_strdup("");
	historyTemplateFilename = mir_strdup("");

	historyBackgroundFilenameTemp = mir_strdup("");
	historyCssFilenameTemp = mir_strdup("");
	historyTemplateFilenameTemp = mir_strdup("");

}

ProtocolSettings::~ProtocolSettings()
{
	mir_free(protocolName);
	mir_free(srmmBackgroundFilename);
	mir_free(srmmBackgroundFilenameTemp);
	mir_free(srmmCssFilename);
	mir_free(srmmCssFilenameTemp);
	mir_free(srmmTemplateFilename);
	mir_free(srmmTemplateFilenameTemp);
	mir_free(chatBackgroundFilename);
	mir_free(chatBackgroundFilenameTemp);
	mir_free(chatCssFilename);
	mir_free(chatCssFilenameTemp);
	mir_free(chatTemplateFilename);
	mir_free(chatTemplateFilenameTemp);

	mir_free(historyBackgroundFilename);
	mir_free(historyBackgroundFilenameTemp);
	mir_free(historyCssFilename);
	mir_free(historyCssFilenameTemp);
	mir_free(historyTemplateFilename);
	mir_free(historyTemplateFilenameTemp);
}

void ProtocolSettings::copyToTemp()
{
	setSRMMModeTemp(getSRMMMode());
	setSRMMFlagsTemp(getSRMMFlags());
	setSRMMBackgroundFilenameTemp(getSRMMBackgroundFilename());
	setSRMMCssFilenameTemp(getSRMMCssFilename());
	setSRMMTemplateFilenameTemp(getSRMMTemplateFilename());
	setSRMMEnableTemp(isSRMMEnable());

	setChatModeTemp(getChatMode());
	setChatFlagsTemp(getChatFlags());
	setChatBackgroundFilenameTemp(getChatBackgroundFilename());
	setChatCssFilenameTemp(getChatCssFilename());
	setChatTemplateFilenameTemp(getChatTemplateFilename());
	setChatEnableTemp(isChatEnable());

	setHistoryModeTemp(getHistoryMode());
	setHistoryFlagsTemp(getHistoryFlags());
	setHistoryBackgroundFilenameTemp(getHistoryBackgroundFilename());
	setHistoryCssFilenameTemp(getHistoryCssFilename());
	setHistoryTemplateFilenameTemp(getHistoryTemplateFilename());
	setHistoryEnableTemp(isHistoryEnable());
}

void ProtocolSettings::copyFromTemp()
{
	setSRMMMode(getSRMMModeTemp());
	setSRMMFlags(getSRMMFlagsTemp());
	setSRMMBackgroundFilename(getSRMMBackgroundFilenameTemp());
	setSRMMCssFilename(getSRMMCssFilenameTemp());
	setSRMMTemplateFilename(getSRMMTemplateFilenameTemp());
	setSRMMEnable(isSRMMEnableTemp());

	setChatMode(getChatModeTemp());
	setChatFlags(getChatFlagsTemp());
	setChatBackgroundFilename(getChatBackgroundFilenameTemp());
	setChatCssFilename(getChatCssFilenameTemp());
	setChatTemplateFilename(getChatTemplateFilenameTemp());
	setChatEnable(isChatEnableTemp());

	setHistoryMode(getHistoryModeTemp());
	setHistoryFlags(getHistoryFlagsTemp());
	setHistoryBackgroundFilename(getHistoryBackgroundFilenameTemp());
	setHistoryCssFilename(getHistoryCssFilenameTemp());
	setHistoryTemplateFilename(getHistoryTemplateFilenameTemp());
	setHistoryEnable(isHistoryEnableTemp());
}

void ProtocolSettings::setNext(ProtocolSettings *next)
{
	this->next = next;
}

const char *ProtocolSettings::getProtocolName()
{
	return protocolName;
}

ProtocolSettings * ProtocolSettings::getNext()
{
	return next;
}

void ProtocolSettings::setSRMMBackgroundFilename(const char *filename)
{
	replaceStr(srmmBackgroundFilename, filename);
}

void ProtocolSettings::setSRMMBackgroundFilenameTemp(const char *filename)
{
	replaceStr(srmmBackgroundFilenameTemp, filename);
}

void ProtocolSettings::setSRMMCssFilename(const char *filename)
{
	replaceStr(srmmCssFilename, filename);
}

void ProtocolSettings::setSRMMCssFilenameTemp(const char *filename)
{
	replaceStr(srmmCssFilenameTemp, filename);
}

void ProtocolSettings::setSRMMTemplateFilename(const char *filename)
{
	replaceStr(srmmTemplateFilename, filename);
	TemplateMap::loadTemplates(getSRMMTemplateFilename(), getSRMMTemplateFilename(), false);
}

void ProtocolSettings::setSRMMTemplateFilenameTemp(const char *filename)
{
	replaceStr(srmmTemplateFilenameTemp, filename);
}

const char *ProtocolSettings::getSRMMBackgroundFilename()
{
	return srmmBackgroundFilename;
}

const char *ProtocolSettings::getSRMMBackgroundFilenameTemp()
{
	return srmmBackgroundFilenameTemp;
}

const char *ProtocolSettings::getSRMMCssFilename()
{
	return srmmCssFilename;
}

const char *ProtocolSettings::getSRMMCssFilenameTemp()
{
	return srmmCssFilenameTemp;
}

const char *ProtocolSettings::getSRMMTemplateFilename()
{
	return srmmTemplateFilename;
}

const char *ProtocolSettings::getSRMMTemplateFilenameTemp()
{
	return srmmTemplateFilenameTemp;
}

void ProtocolSettings::setSRMMEnable(bool enable)
{
	this->srmmEnable = enable;
}

bool ProtocolSettings::isSRMMEnable()
{
	return srmmEnable;
}

void ProtocolSettings::setSRMMEnableTemp(bool enable)
{
	this->srmmEnableTemp = enable;
}

bool ProtocolSettings::isSRMMEnableTemp()
{
	return srmmEnableTemp;
}

void ProtocolSettings::setSRMMMode(int mode)
{
	this->srmmMode = mode;
}

int ProtocolSettings::getSRMMMode()
{
	return srmmMode;
}

void ProtocolSettings::setSRMMModeTemp(int mode)
{
	this->srmmModeTemp = mode;
}

int ProtocolSettings::getSRMMModeTemp()
{
	return srmmModeTemp;
}

void ProtocolSettings::setSRMMFlags(int flags)
{
	this->srmmFlags = flags;
}

int ProtocolSettings::getSRMMFlags()
{
	return srmmFlags;
}

void ProtocolSettings::setSRMMFlagsTemp(int flags)
{
	this->srmmFlagsTemp = flags;
}

int ProtocolSettings::getSRMMFlagsTemp()
{
	return srmmFlagsTemp;
}

/* */

void ProtocolSettings::setChatBackgroundFilename(const char *filename)
{
	replaceStr(chatBackgroundFilename, filename);
}

void ProtocolSettings::setChatBackgroundFilenameTemp(const char *filename)
{
	replaceStr(chatBackgroundFilenameTemp, filename);
}

void ProtocolSettings::setChatCssFilename(const char *filename)
{
	replaceStr(chatCssFilename, filename);
}

void ProtocolSettings::setChatCssFilenameTemp(const char *filename)
{
	replaceStr(chatCssFilenameTemp, filename);
}

void ProtocolSettings::setChatTemplateFilename(const char *filename)
{
	replaceStr(chatTemplateFilename, filename);
	TemplateMap::loadTemplates(getChatTemplateFilename(), getChatTemplateFilename(), false);
}

void ProtocolSettings::setChatTemplateFilenameTemp(const char *filename)
{
	replaceStr(chatTemplateFilenameTemp, filename);
}

const char *ProtocolSettings::getChatBackgroundFilename()
{
	return chatBackgroundFilename;
}

const char *ProtocolSettings::getChatBackgroundFilenameTemp()
{
	return chatBackgroundFilenameTemp;
}

const char *ProtocolSettings::getChatCssFilename()
{
	return chatCssFilename;
}

const char *ProtocolSettings::getChatCssFilenameTemp()
{
	return chatCssFilenameTemp;
}

const char *ProtocolSettings::getChatTemplateFilename()
{
	return chatTemplateFilename;
}

const char *ProtocolSettings::getChatTemplateFilenameTemp()
{
	return chatTemplateFilenameTemp;
}

void ProtocolSettings::setChatEnable(bool enable)
{
	this->chatEnable = enable;
}

bool ProtocolSettings::isChatEnable()
{
	return chatEnable;
}

void ProtocolSettings::setChatEnableTemp(bool enable)
{
	this->chatEnableTemp = enable;
}

bool ProtocolSettings::isChatEnableTemp()
{
	return chatEnableTemp;
}

void ProtocolSettings::setChatMode(int mode)
{
	this->chatMode = mode;
}

int ProtocolSettings::getChatMode()
{
	return chatMode;
}

void ProtocolSettings::setChatModeTemp(int mode)
{
	this->chatModeTemp = mode;
}

int ProtocolSettings::getChatModeTemp()
{
	return chatModeTemp;
}

void ProtocolSettings::setChatFlags(int flags)
{
	this->chatFlags = flags;
}

int ProtocolSettings::getChatFlags()
{
	return chatFlags;
}

void ProtocolSettings::setChatFlagsTemp(int flags)
{
	this->chatFlagsTemp = flags;
}

int ProtocolSettings::getChatFlagsTemp()
{
	return chatFlagsTemp;
}

/* */

void ProtocolSettings::setHistoryBackgroundFilename(const char *filename)
{
	replaceStr(historyBackgroundFilename, filename);
}

void ProtocolSettings::setHistoryBackgroundFilenameTemp(const char *filename)
{
	replaceStr(historyBackgroundFilenameTemp, filename);
}

void ProtocolSettings::setHistoryCssFilename(const char *filename)
{
	replaceStr(historyCssFilename, filename);
}

void ProtocolSettings::setHistoryCssFilenameTemp(const char *filename)
{
	replaceStr(historyCssFilenameTemp, filename);
}

void ProtocolSettings::setHistoryTemplateFilename(const char *filename)
{
	replaceStr(historyTemplateFilename, filename);
	TemplateMap::loadTemplates(getHistoryTemplateFilename(), getHistoryTemplateFilename(), false);
}

void ProtocolSettings::setHistoryTemplateFilenameTemp(const char *filename)
{
	replaceStr(historyTemplateFilenameTemp, filename);
}

const char *ProtocolSettings::getHistoryBackgroundFilename()
{
	return historyBackgroundFilename;
}

const char *ProtocolSettings::getHistoryBackgroundFilenameTemp()
{
	return historyBackgroundFilenameTemp;
}

const char *ProtocolSettings::getHistoryCssFilename()
{
	return historyCssFilename;
}

const char *ProtocolSettings::getHistoryCssFilenameTemp()
{
	return historyCssFilenameTemp;
}

const char *ProtocolSettings::getHistoryTemplateFilename()
{
	return historyTemplateFilename;
}

const char *ProtocolSettings::getHistoryTemplateFilenameTemp()
{
	return historyTemplateFilenameTemp;
}

void ProtocolSettings::setHistoryEnable(bool enable)
{
	this->historyEnable = enable;
}

bool ProtocolSettings::isHistoryEnable()
{
	return historyEnable;
}

void ProtocolSettings::setHistoryEnableTemp(bool enable)
{
	this->historyEnableTemp = enable;
}

bool ProtocolSettings::isHistoryEnableTemp()
{
	return historyEnableTemp;
}

void ProtocolSettings::setHistoryMode(int mode)
{
	this->historyMode = mode;
}

int ProtocolSettings::getHistoryMode()
{
	return historyMode;
}

void ProtocolSettings::setHistoryModeTemp(int mode)
{
	this->historyModeTemp = mode;
}

int ProtocolSettings::getHistoryModeTemp()
{
	return historyModeTemp;
}

void ProtocolSettings::setHistoryFlags(int flags)
{
	this->historyFlags = flags;
}

int ProtocolSettings::getHistoryFlags()
{
	return historyFlags;
}

void ProtocolSettings::setHistoryFlagsTemp(int flags)
{
	this->historyFlagsTemp = flags;
}

int ProtocolSettings::getHistoryFlagsTemp()
{
	return historyFlagsTemp;
}

void Options::init()
{
	if (isInited) return;
	isInited = true;
	DBVARIANT dbv;

	generalFlags = db_get_dw(NULL, ieviewModuleName, DBS_BASICFLAGS, 13);

	/* TODO: move to buildProtocolList method */
	int protoCount;
	PROTOACCOUNT **pProtos;
	ProtocolSettings *lastProto = NULL;
	ProtoEnumAccounts(&protoCount, &pProtos);
	for (int i = 0; i < protoCount + 1; i++) {
		ProtocolSettings *proto;
		char tmpPath[MAX_PATH];
		char dbsName[256];
		if (i == 0) {
			proto = new ProtocolSettings("_default_");
			proto->setSRMMEnable(true);
		}
		else if (strcmp(pProtos[i - 1]->szModuleName, META_PROTO)) {
			if ((CallProtoService(pProtos[i - 1]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM) == 0)
				continue;

			proto = new ProtocolSettings(pProtos[i - 1]->szModuleName);
		}
		else continue;

		/* SRMM settings */
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_SRMM_ENABLE);
		proto->setSRMMEnable(i == 0 ? true : 0 != db_get_b(NULL, ieviewModuleName, dbsName, FALSE));
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_SRMM_MODE);
		proto->setSRMMMode(db_get_b(NULL, ieviewModuleName, dbsName, FALSE));
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_SRMM_FLAGS);
		proto->setSRMMFlags(db_get_dw(NULL, ieviewModuleName, dbsName, 16128));
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_SRMM_BACKGROUND);
		if (!db_get(NULL, ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
			if (strncmp(tmpPath, "http://", 7))
				PathToAbsolute(dbv.pszVal, tmpPath);

			proto->setSRMMBackgroundFilename(tmpPath);
			db_free(&dbv);
		}
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_SRMM_CSS);
		if (!db_get(NULL, ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
			if (strncmp(tmpPath, "http://", 7))
				PathToAbsolute(dbv.pszVal, tmpPath);

			proto->setSRMMCssFilename(tmpPath);
			db_free(&dbv);
		}
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_SRMM_TEMPLATE);
		if (!db_get(NULL, ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
			PathToAbsolute(dbv.pszVal, tmpPath);
			proto->setSRMMTemplateFilename(tmpPath);
			db_free(&dbv);
		}

		/* Group chat settings */
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_CHAT_ENABLE);
		proto->setChatEnable(i == 0 ? true : 0 != db_get_b(NULL, ieviewModuleName, dbsName, FALSE));
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_CHAT_MODE);
		proto->setChatMode(db_get_b(NULL, ieviewModuleName, dbsName, FALSE));
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_CHAT_FLAGS);
		proto->setChatFlags(db_get_dw(NULL, ieviewModuleName, dbsName, 16128));
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_CHAT_BACKGROUND);
		if (!db_get(NULL, ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
			if (strncmp(tmpPath, "http://", 7))
				PathToAbsolute(dbv.pszVal, tmpPath);

			proto->setChatBackgroundFilename(tmpPath);
			db_free(&dbv);
		}
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_CHAT_CSS);
		if (!db_get(NULL, ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
			if (strncmp(tmpPath, "http://", 7))
				PathToAbsolute(dbv.pszVal, tmpPath);

			proto->setChatCssFilename(tmpPath);
			db_free(&dbv);
		}
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_CHAT_TEMPLATE);
		if (!db_get(NULL, ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
			PathToAbsolute(dbv.pszVal, tmpPath);
			proto->setChatTemplateFilename(tmpPath);
			db_free(&dbv);
		}

		/* History settings */
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_HISTORY_ENABLE);
		proto->setHistoryEnable(i == 0 ? true : 0 != db_get_b(NULL, ieviewModuleName, dbsName, FALSE));
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_HISTORY_MODE);
		proto->setHistoryMode(db_get_b(NULL, ieviewModuleName, dbsName, FALSE));
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_HISTORY_FLAGS);
		proto->setHistoryFlags(db_get_dw(NULL, ieviewModuleName, dbsName, 16128));
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_HISTORY_BACKGROUND);
		if (!db_get(NULL, ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
			if (strncmp(tmpPath, "http://", 7))
				PathToAbsolute(dbv.pszVal, tmpPath);

			proto->setHistoryBackgroundFilename(tmpPath);
			db_free(&dbv);
		}
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_HISTORY_CSS);
		if (!db_get(NULL, ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
			if (strncmp(tmpPath, "http://", 7))
				PathToAbsolute(dbv.pszVal, tmpPath);

			proto->setHistoryCssFilename(tmpPath);
			db_free(&dbv);
		}
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_HISTORY_TEMPLATE);
		if (!db_get(NULL, ieviewModuleName, dbsName, &dbv)) {
			strcpy(tmpPath, dbv.pszVal);
			PathToAbsolute(dbv.pszVal, tmpPath);
			proto->setHistoryTemplateFilename(tmpPath);
			db_free(&dbv);
		}
		proto->copyToTemp();
		if (lastProto != NULL) {
			lastProto->setNext(proto);
		}
		else {
			protocolList = proto;
		}
		lastProto = proto;
	}

	bSmileyAdd = 0 != ServiceExists(MS_SMILEYADD_BATCHPARSE);
	avatarServiceFlags = 0;
	if (ServiceExists(MS_AV_GETAVATARBITMAP))
		avatarServiceFlags = AVATARSERVICE_PRESENT;
}

void Options::uninit()
{
	ProtocolSettings *p, *p1;
	for (p = protocolList; p != NULL; p = p1) {
		p1 = p->getNext();
		delete p;
	}
	TemplateMap::dropTemplates();
	if (hImageList != NULL)
		ImageList_Destroy(hImageList);
	if (hProtocolImageList != NULL)
		ImageList_Destroy(hProtocolImageList);
}

void Options::setGeneralFlags(int flags)
{
	generalFlags = flags;
	db_set_dw(NULL, ieviewModuleName, DBS_BASICFLAGS, (DWORD)flags);
}

int Options::getGeneralFlags()
{
	return generalFlags;
}

void Options::setEmbedsize(int size)
{
	db_set_dw(NULL, ieviewModuleName, "Embedsize", (DWORD)size);
}

int Options::getEmbedsize()
{
	return db_get_dw(NULL, ieviewModuleName, "Embedsize", 0);
}

bool Options::isSmileyAdd()
{
	return bSmileyAdd;
}

int Options::getAvatarServiceFlags()
{
	return avatarServiceFlags;
}

ProtocolSettings * Options::getProtocolSettings()
{
	return protocolList;
}

ProtocolSettings * Options::getProtocolSettings(const char *protocolName)
{
	for (ProtocolSettings *proto = protocolList; proto != NULL; proto = proto->getNext())
		if (!strcmpi(proto->getProtocolName(), protocolName))
			return proto;

	return NULL;
}

void Options::resetProtocolSettings()
{
	for (ProtocolSettings *proto = Options::getProtocolSettings(); proto != NULL; proto = proto->getNext())
		proto->copyToTemp();
}

void Options::saveProtocolSettings()
{
	ProtocolSettings *proto = Options::getProtocolSettings();
	for (int i = 0; proto != NULL; proto = proto->getNext(), i++) {
		char dbsName[256];
		char tmpPath[MAX_PATH];
		proto->copyFromTemp();
		/* SRMM settings */
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_SRMM_ENABLE);
		db_set_b(NULL, ieviewModuleName, dbsName, proto->isSRMMEnable());
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_SRMM_MODE);
		db_set_b(NULL, ieviewModuleName, dbsName, proto->getSRMMMode());
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_SRMM_FLAGS);
		db_set_dw(NULL, ieviewModuleName, dbsName, proto->getSRMMFlags());
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_SRMM_BACKGROUND);
		strcpy(tmpPath, proto->getSRMMBackgroundFilename());
		PathToRelative(proto->getSRMMBackgroundFilename(), tmpPath);
		db_set_s(NULL, ieviewModuleName, dbsName, tmpPath);

		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_SRMM_CSS);
		strcpy(tmpPath, proto->getSRMMCssFilename());
		PathToRelative(proto->getSRMMCssFilename(), tmpPath);
		db_set_s(NULL, ieviewModuleName, dbsName, tmpPath);

		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_SRMM_TEMPLATE);
		strcpy(tmpPath, proto->getSRMMTemplateFilename());
		PathToRelative(proto->getSRMMTemplateFilename(), tmpPath);
		db_set_s(NULL, ieviewModuleName, dbsName, tmpPath);

		/* Group Chat settings */
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_CHAT_ENABLE);
		db_set_b(NULL, ieviewModuleName, dbsName, proto->isChatEnable());
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_CHAT_MODE);
		db_set_b(NULL, ieviewModuleName, dbsName, proto->getChatMode());
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_CHAT_FLAGS);
		db_set_dw(NULL, ieviewModuleName, dbsName, proto->getChatFlags());
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_CHAT_BACKGROUND);
		strcpy(tmpPath, proto->getChatBackgroundFilename());
		PathToRelative(proto->getChatBackgroundFilename(), tmpPath);
		db_set_s(NULL, ieviewModuleName, dbsName, tmpPath);

		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_CHAT_CSS);
		strcpy(tmpPath, proto->getChatCssFilename());
		PathToRelative(proto->getChatCssFilename(), tmpPath);
		db_set_s(NULL, ieviewModuleName, dbsName, tmpPath);

		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_CHAT_TEMPLATE);
		strcpy(tmpPath, proto->getChatTemplateFilename());
		PathToRelative(proto->getChatTemplateFilename(), tmpPath);
		db_set_s(NULL, ieviewModuleName, dbsName, tmpPath);

		/* History settings */
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_HISTORY_ENABLE);
		db_set_b(NULL, ieviewModuleName, dbsName, proto->isHistoryEnable());
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_HISTORY_MODE);
		db_set_b(NULL, ieviewModuleName, dbsName, proto->getHistoryMode());
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_HISTORY_FLAGS);
		db_set_dw(NULL, ieviewModuleName, dbsName, proto->getHistoryFlags());
		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_HISTORY_BACKGROUND);
		strcpy(tmpPath, proto->getHistoryBackgroundFilename());
		PathToRelative(proto->getHistoryBackgroundFilename(), tmpPath);
		db_set_s(NULL, ieviewModuleName, dbsName, tmpPath);

		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_HISTORY_CSS);
		strcpy(tmpPath, proto->getHistoryCssFilename());
		PathToRelative(proto->getHistoryCssFilename(), tmpPath);
		db_set_s(NULL, ieviewModuleName, dbsName, tmpPath);

		mir_snprintf(dbsName, SIZEOF(dbsName), "%s.%s", proto->getProtocolName(), DBS_HISTORY_TEMPLATE);
		strcpy(tmpPath, proto->getHistoryTemplateFilename());
		PathToRelative(proto->getHistoryTemplateFilename(), tmpPath);
		db_set_s(NULL, ieviewModuleName, dbsName, tmpPath);
	}
}
