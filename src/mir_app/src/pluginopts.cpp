/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "stdafx.h"

#include "chat.h"
#include "plugins.h"

extern HANDLE hevLoadModule;

/////////////////////////////////////////////////////////////////////////////////////////
//   Plugins options page dialog

struct PluginListItemData
{
	wchar_t    fileName[MAX_PATH];
	HINSTANCE  hInst;
	int        flags, stdPlugin, iRow;
	bool       bRequiresRestart, bWasLoaded, bWasChecked;
	wchar_t   *author, *description, *copyright, *homepage;
	MUUID      uuid;

	void importIds(const MUUID *pIds)
	{
		const MUUID *piface = pIds;
		for (int i = 0; piface[i] != miid_last; i++) {
			int idx = getDefaultPluginIdx(piface[i]);
			if (idx != -1) {
				stdPlugin |= (1 << idx);
				break;
			}
		}
	}
};

static int sttSortPlugins(const PluginListItemData *p1, const PluginListItemData *p2)
{
	return mir_wstrcmp(p1->fileName, p2->fileName);
}

static LIST<PluginListItemData> arPluginList(10, sttSortPlugins);

void freePluginInstance(HINSTANCE hInst)
{
	for (auto &it : arPluginList)
		if (it->hInst == hInst) {
			it->hInst = nullptr;
			break;
		}
}

/////////////////////////////////////////////////////////////////////////////////////////

static wchar_t *sttUtf8auto(const char *src)
{
	if (src == nullptr)
		return mir_wstrdup(L"");

	char *p = NEWSTR_ALLOCA(src);
	wchar_t *pwszRes;
	return (mir_utf8decode(p, &pwszRes) != nullptr) ? pwszRes : mir_a2u_cp(src, 1250);
}

static BOOL dialogListPlugins(WIN32_FIND_DATA *fd, wchar_t *path, WPARAM, LPARAM lParam)
{
	wchar_t buf[MAX_PATH];
	mir_snwprintf(buf, L"%s\\Plugins\\%s", path, fd->cFileName);

	// try to check a plugin without loading it first
	HINSTANCE hInst = GetModuleHandle(buf);

	bool bIsPlugin = false, bNeedsFree = false;
	mir_ptr<MUUID> pIds(GetPluginInterfaces(buf, bIsPlugin));
	if (!bIsPlugin)
		return true;

	CMPluginBase *ppb;
	if (hInst == nullptr) {
		SetErrorMode(SEM_FAILCRITICALERRORS); // disable error messages
		HINSTANCE h = LoadLibraryW(buf);
		SetErrorMode(0);							  // reset the system default
		if (h == nullptr)
			return true;

		ppb = &GetPluginByInstance(h);
		if (ppb->getInst() != h) {
			FreeLibrary(h);
			return true;
		}

		bNeedsFree = true;
	}
	else ppb = &GetPluginByInstance(hInst);

	PluginListItemData *dat = (PluginListItemData*)mir_calloc(sizeof(PluginListItemData));
	dat->hInst = hInst;
	dat->flags = ppb->getInfo().flags;
	dat->bWasLoaded = hInst != nullptr;

	if (pIds != nullptr)
		dat->importIds(pIds);
	else {
		typedef MUUID* (__cdecl * Miranda_Plugin_Interfaces)(void);
		Miranda_Plugin_Interfaces pFunc = (Miranda_Plugin_Interfaces)GetProcAddress(ppb->getInst(), "MirandaPluginInterfaces");
		if (pFunc) {
			MUUID *pPascalIds = pFunc();
			if (pPascalIds != nullptr)
				dat->importIds(pPascalIds);
		}
	}

	CharLower(fd->cFileName);
	wcsncpy_s(dat->fileName, fd->cFileName, _TRUNCATE);

	CCtrlListView *pCtrl = (CCtrlListView*)lParam;
	bool bNoCheckbox = (dat->flags & STATIC_PLUGIN) != 0;
	if (bNoCheckbox || hasMuuid(pIds, MIID_CLIST))
		dat->bRequiresRestart = true;

	LVITEM it = { 0 };
	// column  1: Checkbox + Unicode/ANSI icon + filename
	it.iItem = dat->iRow = arPluginList.getCount();
	it.mask = LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT;
	it.iImage = (dat->flags & UNICODE_AWARE) ? 0 : 1;
	it.pszText = fd->cFileName;
	it.lParam = (LPARAM)dat;
	pCtrl->InsertItem(&it);

	if (!bNoCheckbox && IsPluginOnWhiteList(_T2A(fd->cFileName)))
		dat->bWasChecked = true;

	if (dat->iRow != -1) {
		// column 2: plugin short name
		const PLUGININFOEX &ppi = ppb->getInfo();
		dat->author = sttUtf8auto(ppi.author);
		dat->copyright = sttUtf8auto(ppi.copyright);
		dat->description = sttUtf8auto(ppi.description);
		dat->homepage = sttUtf8auto(ppi.homepage);
		if (ppi.cbSize == sizeof(PLUGININFOEX))
			dat->uuid = ppi.uuid;
		else
			memset(&dat->uuid, 0, sizeof(dat->uuid));

		wchar_t *shortNameT = mir_a2u(ppi.shortName);
		if (shortNameT) {
			pCtrl->SetItemText(dat->iRow, 1, shortNameT);
			mir_free(shortNameT);
		}

		// column 3: version number
		DWORD unused, verInfoSize = GetFileVersionInfoSize(buf, &unused);
		if (verInfoSize != 0) {
			UINT blockSize;
			VS_FIXEDFILEINFO *fi;
			void *pVerInfo = mir_alloc(verInfoSize);
			GetFileVersionInfo(buf, 0, verInfoSize, pVerInfo);
			VerQueryValue(pVerInfo, L"\\", (LPVOID*)&fi, &blockSize);
			mir_snwprintf(buf, L"%d.%d.%d.%d", HIWORD(fi->dwFileVersionMS), LOWORD(fi->dwFileVersionMS), HIWORD(fi->dwFileVersionLS), LOWORD(fi->dwFileVersionLS));
			mir_free(pVerInfo);
		}
		else mir_snwprintf(buf, L"%d.%d.%d.%d", HIBYTE(HIWORD(ppi.version)), LOBYTE(HIWORD(ppi.version)), HIBYTE(LOWORD(ppi.version)), LOBYTE(LOWORD(ppi.version)));

		pCtrl->SetItemText(dat->iRow, 2, buf);
		arPluginList.insert(dat);
	}
	else mir_free(dat);

	if (bNeedsFree)
		FreeLibrary(ppb->getInst());
	return true;
}

static int uuidToString(const MUUID uuid, char *szStr, int cbLen)
{
	if (cbLen < 1 || !szStr)
		return 0;

	mir_snprintf(szStr, cbLen, "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		uuid.a, uuid.b, uuid.c, uuid.d[0], uuid.d[1], uuid.d[2], uuid.d[3], uuid.d[4], uuid.d[5], uuid.d[6], uuid.d[7]);
	return 1;
}

static bool LoadPluginDynamically(PluginListItemData *dat)
{
	wchar_t exe[MAX_PATH];
	GetModuleFileName(nullptr, exe, _countof(exe));
	wchar_t *p = wcsrchr(exe, '\\'); if (p) *p = 0;

	pluginEntry* pPlug = OpenPlugin(dat->fileName, L"Plugins", exe);
	if (pPlug == nullptr) {
	LBL_Error:
		Plugin_UnloadDyn(pPlug);
		return false;
	}

	if (!TryLoadPlugin(pPlug, true))
		goto LBL_Error;

	CMPluginBase *ppb = pPlug->m_pPlugin;
	if (CallPluginEventHook(ppb->getInst(), hModulesLoadedEvent, 0, 0) != 0)
		goto LBL_Error;

	// if dynamically loaded plugin contains protocols, initialize the corresponding accounts
	for (auto &pd : g_arProtos) {
		if (pd->hInst != ppb->getInst())
			continue;

		for (auto &pa : g_arAccounts) {
			if (mir_strcmp(pa->szProtoName, pd->szName))
				continue;

			if (pa->ppro == nullptr) {
				if (pa->bIsEnabled) {
					if (ActivateAccount(pa, true))
						NotifyEventHooks(hAccListChanged, PRAC_ADDED, (LPARAM)pa);
				}
				else pa->bDynDisabled = false;
			}
			else if (pa->bOldProto)
				NotifyEventHooks(hAccListChanged, PRAC_ADDED, (LPARAM)pa);
		}
	}

	dat->bWasLoaded = true;
	dat->hInst = ppb->getInst();
	NotifyFastHook(hevLoadModule, (WPARAM)ppb, (LPARAM)ppb->getInst());
	return true;
}

static bool UnloadPluginDynamically(PluginListItemData *dat)
{
	pluginEntry tmp;
	strncpy_s(tmp.pluginname, _T2A(dat->fileName), _TRUNCATE);

	pluginEntry *p = pluginList.find(&tmp);
	if (p) {
		if (!Plugin_UnloadDyn(p))
			return false;

		dat->bWasLoaded = false;
		dat->hInst = nullptr;
	}
	return true;
}

static int CALLBACK SortPlugins(LPARAM i1, LPARAM i2, LPARAM)
{
	PluginListItemData *p1 = (PluginListItemData*)i1, *p2 = (PluginListItemData*)i2;
	return mir_wstrcmp(p1->fileName, p2->fileName);
}

class CPluginOptDlg : public CDlgBase
{
	CTimer m_timer;
	CCtrlBase m_author, m_plugPid, m_plugInfo, m_copyright;
	CCtrlListView m_plugList;
	CCtrlHyperlink m_link, m_plugUrl;

	CMStringW m_szFilter;

	HANDLE m_hEvent1, m_hEvent2;
	bool needRestart = false;
	CMStringA szUrl;

public:
	CPluginOptDlg() :
		CDlgBase(g_plugin, IDD_OPT_PLUGINS),
		m_link(this, IDC_GETMOREPLUGINS, "https://miranda-ng.org/downloads/"),
		m_plugUrl(this, IDC_PLUGINURL),
		m_author(this, IDC_PLUGINAUTHOR),
		m_plugPid(this, IDC_PLUGINPID),
		m_plugInfo(this, IDC_PLUGINLONGINFO),
		m_copyright(this, IDC_PLUGINCPYR),
		m_plugList(this, IDC_PLUGLIST),
		m_timer(this, 1)
	{
		m_timer.OnEvent = Callback(this, &CPluginOptDlg::onTimer);

		m_plugList.OnItemChanged = Callback(this, &CPluginOptDlg::list_ItemChanged);
		m_plugList.OnKeyDown = Callback(this, &CPluginOptDlg::list_OnKeyDown);
	}

	bool OnInitDialog() override
	{
		HIMAGELIST hIml = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 4, 0);
		ImageList_AddSkinIcon(hIml, SKINICON_OTHER_UNICODE);
		ImageList_AddSkinIcon(hIml, SKINICON_OTHER_ANSI);
		m_plugList.SetImageList(hIml, LVSIL_SMALL);

		LVCOLUMN col;
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.pszText = TranslateT("Plugin");
		col.cx = 180;
		m_plugList.InsertColumn(1, &col);

		col.pszText = TranslateT("Name");
		col.cx = 180;
		m_plugList.InsertColumn(2, &col);

		col.pszText = TranslateT("Version");
		col.cx = 115;
		m_plugList.InsertColumn(3, &col);

		m_plugList.SetExtendedListViewStyleEx(0, LVS_EX_SUBITEMIMAGES | LVS_EX_CHECKBOXES | LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

		// scan the plugin dir for plugins
		arPluginList.destroy();
		m_szFilter.Empty();
		enumPlugins(dialogListPlugins, (WPARAM)m_hwnd, (LPARAM)&m_plugList);

		// some plugins could be just loaded by Plugin Updater, load them first
		for (auto &it : arPluginList)
			if (!it->bWasLoaded && !it->bRequiresRestart && it->bWasChecked && !it->hInst)
				LoadPluginDynamically(it);

		// set checkboxes for all loaded plugins
		for (auto &it : arPluginList) {
			if (!IsPluginOnWhiteList(_T2A(it->fileName)))
				continue;

			if (it->flags & STATIC_PLUGIN)
				m_plugList.SetItemState(it->iRow, 0x3000, LVIS_STATEIMAGEMASK);
			else if (it->hInst)
				m_plugList.SetItemState(it->iRow, 0x2000, LVIS_STATEIMAGEMASK);
		}

		// sort out the headers
		m_plugList.SetColumnWidth(0, LVSCW_AUTOSIZE); // dll name
		int w = m_plugList.GetColumnWidth(0);
		if (w > 180)
			m_plugList.SetColumnWidth(0, w = 180);

		int max = w < 180 ? 189 + 180 - w : 189;
		m_plugList.SetColumnWidth(2, LVSCW_AUTOSIZE); // short name
		w = m_plugList.GetColumnWidth(1);
		if (w > max)
			m_plugList.SetColumnWidth(1, max);

		m_plugList.SortItems(SortPlugins, (LPARAM)m_hwnd);
		return true;
	}

	bool OnApply() override
	{
		CMStringW bufRestart(TranslateT("Miranda NG must be restarted to apply changes for these plugins:"));
		bufRestart.AppendChar('\n');

		for (int iRow = 0; iRow != -1;) {
			wchar_t buf[1024];
			m_plugList.GetItemText(iRow, 0, buf, _countof(buf));
			int iState = m_plugList.GetItemState(iRow, LVIS_STATEIMAGEMASK);
			SetPluginOnWhiteList(_T2A(buf), (iState & 0x2000) != 0);

			if (iState != 0x3000) {
				LVITEM lvi = { 0 };
				lvi.mask = LVIF_IMAGE | LVIF_PARAM;
				lvi.stateMask = -1;
				lvi.iItem = iRow;
				lvi.iSubItem = 0;
				if (m_plugList.GetItem(&lvi)) {
					lvi.mask = LVIF_IMAGE;

					PluginListItemData *dat = (PluginListItemData*)lvi.lParam;
					if (iState == 0x2000) { // enabling plugin
						if (!dat->bWasLoaded) {
							if (!dat->bRequiresRestart)
								LoadPluginDynamically(dat);
							else {
								bufRestart.AppendFormat(L" - %s\n", buf);
								needRestart = true;
							}
						}
					}
					else { // disabling plugin
						if (dat->bWasLoaded) {
							if (!dat->bRequiresRestart)
								UnloadPluginDynamically(dat);
							else {
								bufRestart.AppendFormat(L" - %s\n", buf);
								needRestart = true;
							}
						}
					}
				}
			}

			iRow = m_plugList.GetNextItem(iRow, LVNI_ALL);
		}
		LoadStdPlugins();

		ShowWindow(GetDlgItem(m_hwnd, IDC_RESTART), needRestart);
		if (needRestart) {
			bufRestart.AppendChar('\n');
			bufRestart.Append(TranslateT("Do you want to restart it now?"));
			if (MessageBox(m_hwnd, bufRestart, L"Miranda NG", MB_ICONWARNING | MB_YESNO) == IDYES)
				CallService(MS_SYSTEM_RESTART, 1, 0);
		}
		return true;
	}

	void OnDestroy() override
	{
		UnhookEvent(m_hEvent1);
		UnhookEvent(m_hEvent2);

		arPluginList.destroy();

		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		for (lvi.iItem = 0; m_plugList.GetItem(&lvi); lvi.iItem++) {
			PluginListItemData *dat = (PluginListItemData*)lvi.lParam;
			mir_free(dat->author);
			mir_free(dat->copyright);
			mir_free(dat->description);
			mir_free(dat->homepage);
			mir_free(dat);
		}
	}

	void onTimer(CTimer*)
	{
		m_timer.Stop();
		m_szFilter.Empty();
	}

	void list_OnKeyDown(CCtrlListView::TEventInfo *evt)
	{
		if (evt->nmlvkey->wVKey == VK_BACK) {
			if (m_szFilter.GetLength() > 0)
				m_szFilter.Truncate(m_szFilter.GetLength() - 1);
			return;
		}

		if (evt->nmlvkey->wVKey < '0' || evt->nmlvkey->wVKey > 'Z')
			return;

		m_szFilter.AppendChar(evt->nmlvkey->wVKey);

		for (auto &p : arPluginList) {
			if (!wcsnicmp(m_szFilter, p->fileName, m_szFilter.GetLength())) {
				LVFINDINFO lvfi;
				lvfi.flags = LVFI_PARAM;
				lvfi.lParam = (LPARAM)p;
				int idx = m_plugList.FindItem(0, &lvfi);
				if (idx != -1) {
					m_plugList.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_plugList.EnsureVisible(idx, FALSE);
					m_timer.Start(1500);
					return;
				}
			}
		}

		m_szFilter.Truncate(m_szFilter.GetLength() - 1);
		MessageBeep((UINT)-1);
	}

	void list_ItemChanged(CCtrlListView::TEventInfo *evt)
	{
		auto hdr = evt->nmlv;
		if (!IsWindowVisible(hdr->hdr.hwndFrom))
			return;

		if (hdr->uOldState != 0 && (hdr->uNewState == 0x1000 || hdr->uNewState == 0x2000)) {
			LVITEM it;
			it.mask = LVIF_PARAM | LVIF_STATE;
			it.iItem = hdr->iItem;
			if (!m_plugList.GetItem(&it))
				return;

			PluginListItemData *dat = (PluginListItemData*)it.lParam;
			if (dat->flags & STATIC_PLUGIN) {
				m_plugList.SetItemState(hdr->iItem, 0x3000, LVIS_STATEIMAGEMASK);
				return;
			}
			
			// find all another standard plugins by mask and disable them
			if ((hdr->uNewState == 0x2000) && dat->stdPlugin != 0) {
				for (int iRow = 0; iRow != -1; iRow = m_plugList.GetNextItem(iRow, LVNI_ALL)) {
					if (iRow == hdr->iItem)  // skip the plugin we're standing on
						continue;

					LVITEM dt;
					dt.mask = LVIF_PARAM;
					dt.iItem = iRow;
					if (m_plugList.GetItem(&dt)) {
						PluginListItemData *dat2 = (PluginListItemData*)dt.lParam;
						if (dat2->stdPlugin & dat->stdPlugin)
							m_plugList.SetItemState(iRow, 0x1000, LVIS_STATEIMAGEMASK);
					}
				}
			}

			NotifyChange();
			return;
		}

		if (hdr->iItem != -1) {
			int sel = hdr->uNewState & LVIS_SELECTED;

			LVITEM lvi = { 0 };
			lvi.mask = LVIF_PARAM;
			lvi.iItem = hdr->iItem;
			if (m_plugList.GetItem(&lvi)) {
				PluginListItemData *dat = (PluginListItemData*)lvi.lParam;

				wchar_t buf[1024];
				m_plugList.GetItemText(hdr->iItem, 2, buf, _countof(buf));
				SetDlgItemText(m_hwnd, IDC_PLUGININFOFRAME, sel ? buf : L"");
				m_author.SetText(sel ? dat->author : L"");
				m_plugInfo.SetText(sel ? TranslateW_LP(dat->description, &GetPluginByInstance(dat->hInst)) : L"");
				m_copyright.SetText(sel ? dat->copyright : L"");

				szUrl = sel ? _T2A(dat->homepage) : "";
				m_plugUrl.SetUrl(szUrl);
				m_plugUrl.SetTextA(szUrl);

				if (dat->uuid != miid_last) {
					char szUID[128];
					uuidToString(dat->uuid, szUID, sizeof(szUID));
					m_plugPid.SetTextA(sel ? szUID : "");
				}
				else m_plugPid.SetText(sel ? TranslateT("<none>") : L"");
			}
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

int PluginOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pDialog = new CPluginOptDlg();
	odp.position = 1300000000;
	odp.szTitle.a = LPGEN("Plugins");
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
