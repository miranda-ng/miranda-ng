/*

'File Association Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License,  or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (AssocMgr-License.txt); if not,  write to the Free Software
Foundation,  Inc.,  59 Temple Place - Suite 330,  Boston,  MA  02111-1307,  USA.
*/

#include "stdafx.h"

/************************* Assoc List *****************************/

static OBJLIST<ASSOCDATA> arAssocList(10, PtrKeySortT);
static mir_cs csAssocList;

/************************* Assoc Enabled **************************/

static BOOL IsAssocEnabled(const ASSOCDATA *assoc)
{
	char szSetting[MAXMODULELABELLENGTH];
	mir_snprintf(szSetting, "enabled_%s", assoc->pszClassName);
	return g_plugin.getByte(szSetting, (uint8_t)!(assoc->flags&FTDF_DEFAULTDISABLED)) != 0;
}

static void SetAssocEnabled(const ASSOCDATA *assoc, BOOL fEnabled)
{
	char szSetting[MAXMODULELABELLENGTH];
	wchar_t szDLL[MAX_PATH], szBuf[MAX_PATH];
	mir_snprintf(szSetting, "enabled_%s", assoc->pszClassName);
	g_plugin.setByte(szSetting, (uint8_t)fEnabled);
	// dll name for uninstall
	if (assoc->hInstance != nullptr && assoc->hInstance != g_plugin.getInst() && assoc->hInstance != GetModuleHandle(nullptr))
		if (GetModuleFileName(assoc->hInstance, szBuf, _countof(szBuf)))
			if (PathToRelativeW(szBuf, szDLL)) {
				mir_snprintf(szSetting, "module_%s", assoc->pszClassName);
				g_plugin.setWString(szSetting, szDLL);
			}
}

static void DeleteAssocEnabledSetting(const ASSOCDATA *assoc)
{
	char szSetting[MAXMODULELABELLENGTH];
	mir_snprintf(szSetting, "enabled_%s", assoc->pszClassName);
	g_plugin.delSetting(szSetting);
	// dll name for uninstall
	mir_snprintf(szSetting, "module_%s", assoc->pszClassName);
	g_plugin.delSetting(szSetting);
}

void CleanupAssocEnabledSettings(void)
{
	// delete old enabled_* settings if associated plugin no longer present
	int nSettingsCount;
	char **ppszSettings;
	if (EnumDbPrefixSettings(MODULENAME, "enabled_", &ppszSettings, &nSettingsCount)) {
		mir_cslock lck(csAssocList);
		for (int i = 0; i < nSettingsCount; ++i) {
			char *pszSuffix = &ppszSettings[i][8];
			char szSetting[MAXMODULELABELLENGTH];
			mir_snprintf(szSetting, "module_%s", pszSuffix);
			ptrW wszPath(g_plugin.getWStringA(szSetting));
			if (wszPath != nullptr) {
				wchar_t szDLL[MAX_PATH];
				if (PathToAbsoluteW(wszPath, szDLL)) {
					// file still exists?
					HANDLE hFile = CreateFile(szDLL, 0, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
					if (hFile == INVALID_HANDLE_VALUE) {
						g_plugin.delSetting(ppszSettings[i]);
						g_plugin.delSetting(szSetting);
					}
					else CloseHandle(hFile);
				}
			}
			mir_free(ppszSettings[i]);
		}
		mir_free(ppszSettings);
	}
}

/************************* Mime Reg *******************************/

static __inline void RememberMimeTypeAdded(const char *pszMimeType, const char *pszFileExt, uint8_t fAdded)
{
	char szSetting[MAXMODULELABELLENGTH];
	mir_snprintf(szSetting, "mime_%s", pszMimeType);
	if (fAdded)
		g_plugin.setString(szSetting, pszFileExt);
	else
		g_plugin.delSetting(szSetting);
}

static __inline BOOL WasMimeTypeAdded(const char *pszMimeType)
{
	char szSetting[MAXMODULELABELLENGTH];
	DBVARIANT dbv;
	BOOL fAdded = FALSE;
	mir_snprintf(szSetting, "mime_%s", pszMimeType);
	if (!db_get(0, MODULENAME, szSetting, &dbv))
		fAdded = TRUE;
	else
		db_free(&dbv);
	return fAdded;
}

void CleanupMimeTypeAddedSettings(void)
{
	// delete old mime_* settings and unregister the associated mime type
	int nSettingsCount;
	char **ppszSettings;
	if (EnumDbPrefixSettings(MODULENAME, "mime_", &ppszSettings, &nSettingsCount)) {
		mir_cslock lck(csAssocList);
		for (int i = 0; i < nSettingsCount; ++i) {
			char *pszSuffix = &ppszSettings[i][5];
			ASSOCDATA *p = nullptr;
			for (auto &it : arAssocList)
				if (!mir_strcmp(pszSuffix, it->pszMimeType)) {
					p = it;
					break; // mime type in current list
				}

			if (p == nullptr) { // mime type not in current list
				DBVARIANT dbv;
				if (!g_plugin.getString(ppszSettings[i], &dbv)) {
					RemoveRegMimeType(pszSuffix, dbv.pszVal);
					db_free(&dbv);
				}
				g_plugin.delSetting(ppszSettings[i]);
			}
			mir_free(ppszSettings[i]);
		}
		mir_free(ppszSettings);
	}
}

/************************* Shell Notify ***************************/

#define SHELLNOTIFY_DELAY  3000  // time for which assoc changes are buffered

static UINT nNotifyTimerID; // protected by csNotifyTimer
static mir_cs csNotifyTimer;

static void CALLBACK NotifyTimerProc(HWND hwnd, UINT, UINT_PTR nTimerID, DWORD)
{
	mir_cslock lck(csNotifyTimer);
	KillTimer(hwnd, nTimerID);
	if (nNotifyTimerID == nTimerID) // might be stopped previously
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, nullptr, nullptr);
	nNotifyTimerID = 0;
}

static void NotifyAssocChange(BOOL fNow)
{
	mir_cslock lck(csNotifyTimer);
	if (fNow) {
		nNotifyTimerID = 0; // stop previous timer
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSH, nullptr, nullptr);
	}
	else nNotifyTimerID = SetTimer(nullptr, nNotifyTimerID, SHELLNOTIFY_DELAY, NotifyTimerProc);
}

/************************* Assoc List Utils ***********************/

// this function assumes it has got the csAssocList mutex
static ASSOCDATA* FindAssocItem(const char *pszClassName)
{
	for (auto &it : arAssocList)
		if (!mir_strcmp(pszClassName, it->pszClassName))
			return it;

	return nullptr;
}

BOOL IsRegisteredAssocItem(const char *pszClassName)
{
	mir_cslock lck(csAssocList);
	return FindAssocItem(pszClassName) != nullptr;
}

// this function assumes it has got the csAssocList mutex
// this function assumes CoInitialize() has been called before
static int ReplaceImageListAssocIcon(HIMAGELIST himl, ASSOCDATA *assoc, int iPrevIndex)
{
	if (himl == nullptr)
		return -1;

	// load icon
	HICON hIcon = LoadRegClassSmallIcon(assoc);
	if (hIcon == nullptr) {
		SHFILEINFOA sfi;
		if (SHGetFileInfoA((assoc->pszFileExt != nullptr) ? assoc->pszFileExt : "", FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES))
			hIcon = sfi.hIcon; // WinXP: this icon is not updated until the process exits
	}
	// add icon
	if (hIcon == nullptr)
		return -1;
	int index = ImageList_ReplaceIcon(himl, iPrevIndex, hIcon);
	DestroyIcon(hIcon);
	return index;
}

// the return value does not need to be freed
// this function assumes it has got the csAssocList mutex
static wchar_t* GetAssocTypeDesc(const ASSOCDATA *assoc)
{
	static wchar_t szDesc[32];
	if (assoc->pszFileExt == nullptr)
		mir_snwprintf(szDesc, L"%hs:", assoc->pszClassName);
	else
		mir_snwprintf(szDesc, TranslateT("%hs files"), assoc->pszFileExt);
	return szDesc;
}

// this function assumes it has got the csAssocList mutex
static BOOL IsAssocRegistered(const ASSOCDATA *assoc)
{
	BOOL fIsUrl = (assoc->pszFileExt == nullptr);
	BOOL fUseMainCmdLine = (assoc->pszService == nullptr);

	// class
	BOOL fSuccess = FALSE;
	wchar_t *pszRunCmd = MakeRunCommand(fUseMainCmdLine, !fUseMainCmdLine);
	if (pszRunCmd != nullptr)
		fSuccess = IsRegClass(assoc->pszClassName, pszRunCmd);
	mir_free(pszRunCmd); // does NULL check
	// file ext
	if (!fIsUrl)
		fSuccess = IsRegFileExt(assoc->pszFileExt, assoc->pszClassName);

	return fSuccess;
}

// this function assumes it has got the csAssocList mutex
// call GetLastError() on error to get more error details
static BOOL EnsureAssocRegistered(const ASSOCDATA *assoc)
{
	BOOL fSuccess = FALSE, fIsUrl, fUseMainCmdLine;
	wchar_t *pszIconLoc, *pszRunCmd, *pszDdeCmd, *pszAppFileName;

	fIsUrl = (assoc->pszFileExt == nullptr);
	fUseMainCmdLine = (assoc->pszService == nullptr);

	pszRunCmd = MakeRunCommand(fUseMainCmdLine, !fUseMainCmdLine);
	if (pszRunCmd != nullptr) {
		fSuccess = TRUE; // tentatively
		// do not overwrite user customized settings
		if (!IsRegClass(assoc->pszClassName, pszRunCmd)) {
			// class icon
			if (!assoc->nIconResID && fIsUrl) pszIconLoc = MakeIconLocation(nullptr, 0); // miranda logo
			else if (!assoc->nIconResID) pszIconLoc = MakeIconLocation(g_plugin.getInst(), IDI_MIRANDAFILE); // generic file
			else pszIconLoc = MakeIconLocation(assoc->hInstance, assoc->nIconResID);
			// register class
			if (fUseMainCmdLine) pszDdeCmd = nullptr;
			else pszDdeCmd = fIsUrl ? DDEURLCMD : DDEFILECMD;
			fSuccess = AddRegClass(assoc->pszClassName, assoc->pszDescription, pszIconLoc, _A2W(MIRANDANAME), pszRunCmd, pszDdeCmd, DDEAPP, DDETOPIC, assoc->pszVerbDesc, assoc->flags&FTDF_BROWSERAUTOOPEN, fIsUrl, assoc->flags&FTDF_ISSHORTCUT);
			mir_free(pszIconLoc); // does NULL check
			// file type
			if (fSuccess && !fIsUrl) {
				// register mime type
				if (assoc->pszMimeType != nullptr)
					if (AddRegMimeType(assoc->pszMimeType, assoc->pszFileExt))
						RememberMimeTypeAdded(assoc->pszMimeType, assoc->pszFileExt, TRUE);
				// register file ext
				fSuccess = AddRegFileExt(assoc->pszFileExt, assoc->pszClassName, assoc->pszMimeType, assoc->flags&FTDF_ISTEXT);
				// register open-with
				pszAppFileName = MakeAppFileName(fUseMainCmdLine);
				if (pszAppFileName != nullptr)
					AddRegOpenWithExtEntry(pszAppFileName, assoc->pszFileExt, assoc->pszDescription);
				mir_free(pszAppFileName); // does NULL check
			}
		}
		mir_free(pszRunCmd);
	}
	else SetLastError(ERROR_OUTOFMEMORY);
	return fSuccess;
}

// this function assumes it has got the csAssocList mutex
// call GetLastError() on error to get more error details
static BOOL UnregisterAssoc(const ASSOCDATA *assoc)
{
	BOOL fIsUrl = (assoc->pszFileExt == nullptr);
	BOOL fUseMainCmdLine = (assoc->pszService == nullptr);

	// class might have been registered by another instance
	wchar_t *pszRunCmd = MakeRunCommand(fUseMainCmdLine, !fUseMainCmdLine);
	if (pszRunCmd != nullptr && !IsRegClass(assoc->pszClassName, pszRunCmd)) {
		mir_free(pszRunCmd);
		return TRUE; // succeed anyway
	}
	mir_free(pszRunCmd); // does NULL check

	// file type
	if (!fIsUrl) {
		// file extension
		RemoveRegFileExt(assoc->pszFileExt, assoc->pszClassName);
		// mime type
		if (assoc->pszMimeType != nullptr)
			if (WasMimeTypeAdded(assoc->pszMimeType)) {
				RemoveRegMimeType(assoc->pszMimeType, assoc->pszFileExt);
				RememberMimeTypeAdded(assoc->pszMimeType, assoc->pszFileExt, FALSE);
			}

		// open-with entry
		wchar_t *pszAppFileName = MakeAppFileName(fUseMainCmdLine);
		if (pszAppFileName != nullptr)
			RemoveRegOpenWithExtEntry(pszAppFileName, assoc->pszFileExt);
		mir_free(pszAppFileName); // does NULL check
	}
	return RemoveRegClass(assoc->pszClassName);
}

/************************* Assoc List Workers *********************/
// this structure represents the head of both
// * FILETYPEDESC and URLTYPEDESC structures.
// * the head is identical for both structures.

struct TYPEDESCHEAD
{
	int cbSize;  // either sizeof(FILETYPEDESC) or sizeof(URLTYPEDESC)
	const void *pszDescription;
	HINSTANCE hInstance;
	UINT nIconResID;
	const char *pszService;
	uint32_t flags;
};

// ownership of pszClassName,  pszFileExt,  pszVerbDesc and pszMimeType is transfered
// to the storage list on success
static bool AddNewAssocItem_Worker(char *pszClassName, const TYPEDESCHEAD *tdh, char *pszFileExt, wchar_t *pszVerbDesc, char *pszMimeType)
{
	// is already in list?
	mir_cslock lck(csAssocList);
	if (FindAssocItem(pszClassName) != nullptr)
		return false;

	// init new item
	ASSOCDATA *assoc = new ASSOCDATA();
	assoc->pszClassName = pszClassName; // no dup here
	assoc->pszDescription = s2t(tdh->pszDescription, tdh->flags & FTDF_UNICODE, TRUE); // does NULL check
	assoc->hInstance = tdh->hInstance; // hInstance is allowed to be NULL for miranda32.exe
	assoc->nIconResID = (uint16_t)tdh->nIconResID; // default icon selected later on
	assoc->pszService = mir_strdup(tdh->pszService); // does NULL check
	assoc->flags = (uint16_t)tdh->flags;
	assoc->pszFileExt = pszFileExt;
	assoc->pszMimeType = pszMimeType;
	assoc->pszVerbDesc = pszVerbDesc;

	// error check
	if (assoc->pszDescription == nullptr || (assoc->pszService == nullptr && tdh->pszService != nullptr)) {
		delete assoc;
		return false;
	}

	// add registry keys 
	if (IsAssocEnabled(assoc))
		EnsureAssocRegistered(assoc);

	arAssocList.insert(assoc);
	NotifyAssocChange(false);
	return true;
}

// ownership of pszClassName is *not* transferd to storage list
static bool RemoveAssocItem_Worker(const char *pszClassName)
{
	// find index
	mir_cslock lck(csAssocList);
	ASSOCDATA *assoc = FindAssocItem(pszClassName);
	if (assoc == nullptr)
		return false;

	// delete registry keys and db setting
	UnregisterAssoc(assoc);
	if (assoc->pszMimeType != nullptr)
		RememberMimeTypeAdded(assoc->pszMimeType, assoc->pszFileExt, FALSE);
	DeleteAssocEnabledSetting(assoc);

	// resize storage array
	arAssocList.remove(assoc);

	NotifyAssocChange(FALSE);
	return TRUE;
}

/************************* Services *******************************/

static INT_PTR ServiceAddNewFileType(WPARAM, LPARAM lParam)
{
	const FILETYPEDESC *ftd = (FILETYPEDESC*)lParam;
	if (ftd->cbSize < sizeof(FILETYPEDESC))
		return 1;
	if (ftd->pszFileExt == nullptr || ftd->pszFileExt[0] != '.')
		return 2;

	char *pszFileExt = mir_strdup(ftd->pszFileExt);
	char *pszClassName = MakeFileClassName(ftd->pszFileExt);
	if (pszFileExt != nullptr && pszClassName != nullptr) {
		wchar_t *pszVerbDesc = s2t(ftd->pwszVerbDesc, ftd->flags & FTDF_UNICODE, TRUE); // does NULL check
		char *pszMimeType = mir_strdup(ftd->pszMimeType); // does NULL check
		if (AddNewAssocItem_Worker(pszClassName, (TYPEDESCHEAD*)ftd, pszFileExt, pszVerbDesc, pszMimeType))
			// no need to free pszClassName,  pszFileExt, pszVerbDesc and pszMimeType, 
			// as their ownership got transfered to storage list
			return 0;
	}
	mir_free(pszClassName); // does NULL check
	mir_free(pszFileExt); // does NULL check
	return 3;
}

static INT_PTR ServiceRemoveFileType(WPARAM, LPARAM lParam)
{
	if ((char*)lParam == nullptr) return 2;
	char *pszClassName = MakeFileClassName((char*)lParam);
	if (pszClassName != nullptr)
		if (RemoveAssocItem_Worker(pszClassName)) {
			mir_free(pszClassName);
			return 0;
		}
	mir_free(pszClassName); // does NULL check
	return 3;
}

static INT_PTR ServiceAddNewUrlType(WPARAM, LPARAM lParam)
{
	const URLTYPEDESC *utd = (URLTYPEDESC*)lParam;
	if (utd->cbSize < sizeof(URLTYPEDESC))
		return 1;
	if (utd->pszService == nullptr)
		return 2;
	if (utd->pszProtoPrefix == nullptr || utd->pszProtoPrefix[mir_strlen(utd->pszProtoPrefix) - 1] != ':')
		return 2;

	char *pszClassName = MakeUrlClassName(utd->pszProtoPrefix);
	if (pszClassName != nullptr)
		if (AddNewAssocItem_Worker(pszClassName, (TYPEDESCHEAD*)utd, nullptr, nullptr, nullptr))
			// no need to free pszClassName, as its ownership got transferred to storage list
			return 0;
	mir_free(pszClassName); // does NULL check
	return 3;
}

static INT_PTR ServiceRemoveUrlType(WPARAM, LPARAM lParam)
{
	if ((char*)lParam == nullptr) return 2;
	char *pszClassName = MakeUrlClassName((char*)lParam);
	if (pszClassName != nullptr)
		if (RemoveAssocItem_Worker(pszClassName)) {
			mir_free(pszClassName);
			return 0;
		}
	mir_free(pszClassName); // does NULL check
	return 3;
}

/************************* Open Handler ***************************/

static BOOL InvokeHandler_Worker(const char *pszClassName, const wchar_t *pszParam, INT_PTR *res)
{
	// find it in list
	mir_cslock lck(csAssocList);
	ASSOCDATA *assoc = FindAssocItem(pszClassName);
	if (assoc == nullptr)
		return false;

	// no service specified? correct registry to use main commandline
	if (assoc->pszService == nullptr) {
		EnsureAssocRegistered(assoc);
		NotifyAssocChange(FALSE);
		// try main command line
		if ((INT_PTR)ShellExecute(nullptr, nullptr, pszParam, nullptr, nullptr, SW_SHOWNORMAL) >= 32)
			*res = 0; // success
		return TRUE;
	}

	// get params
	char *pszService = mir_strdup(assoc->pszService);
	void *pvParam = t2s(pszParam, assoc->flags & FTDF_UNICODE, FALSE);

	// call service
	if (pszService != nullptr && pvParam != nullptr)
		*res = CallService(pszService, 0, (LPARAM)pvParam);
	mir_free(pszService); // does NULL check
	mir_free(pvParam); // does NULL check
	return TRUE;
}

INT_PTR InvokeFileHandler(const wchar_t *pszFileName)
{
	INT_PTR res = CALLSERVICE_NOTFOUND;

	// find extension
	wchar_t *p = (wchar_t*)wcsrchr(pszFileName, '.');
	if (p != nullptr) {
		char *pszFileExt = mir_u2a(p);
		if (pszFileExt != nullptr) {
			// class name
			char *pszClassName = MakeFileClassName(pszFileExt);
			if (pszClassName != nullptr)
				if (!InvokeHandler_Worker(pszClassName, pszFileName, &res)) {
					// correct registry on error (no longer in list)
					RemoveRegFileExt(pszFileExt, pszClassName);
					RemoveRegClass(pszClassName);
				}
			mir_free(pszClassName); // does NULL check
			mir_free(pszFileExt);
		}
	}
	return res;
}

INT_PTR InvokeUrlHandler(const wchar_t *pszUrl)
{
	INT_PTR res = CALLSERVICE_NOTFOUND;

	// find prefix
	char *pszProtoPrefix = mir_u2a(pszUrl);
	if (pszProtoPrefix != nullptr) {
		char *p = strchr(pszProtoPrefix, ':');
		if (p != nullptr) {
			*(++p) = 0; // remove trailing :
			// class name
			char *pszClassName = MakeUrlClassName(pszProtoPrefix);
			if (pszClassName != nullptr)
				if (!InvokeHandler_Worker(pszClassName, pszUrl, &res))
					// correct registry on error (no longer in list)
					RemoveRegClass(pszClassName);
			mir_free(pszClassName); // does NULL check
		}
		mir_free(pszProtoPrefix);
	}
	return res;
}

/************************* Options ********************************/

static int CALLBACK ListViewSortDesc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int cmp;
	if (((ASSOCDATA*)lParam1)->pszFileExt != nullptr && ((ASSOCDATA*)lParam2)->pszFileExt != nullptr)
		cmp = CompareStringA((LCID)lParamSort, 0, ((ASSOCDATA*)lParam1)->pszFileExt, -1, ((ASSOCDATA*)lParam2)->pszFileExt, -1);
	else if (((ASSOCDATA*)lParam1)->pszFileExt.get() == ((ASSOCDATA*)lParam2)->pszFileExt.get()) // both NULL
		cmp = CompareStringA((LCID)lParamSort, 0, ((ASSOCDATA*)lParam1)->pszClassName, -1, ((ASSOCDATA*)lParam2)->pszClassName, -1);
	else // different types,  incomparable
		cmp = (((ASSOCDATA*)lParam1)->pszFileExt == nullptr) ? CSTR_LESS_THAN : CSTR_GREATER_THAN;
	if (cmp == CSTR_EQUAL)
		cmp = CompareString((LCID)lParamSort, 0, ((ASSOCDATA*)lParam1)->pszDescription, -1, ((ASSOCDATA*)lParam2)->pszDescription, -1);
	if (cmp != 0) cmp -= 2; // maintain CRT conventions
	return cmp;
}

class COptionsDialog : public CDlgBase
{
	CCtrlListView m_lvAssocList;
	CCtrlCheck m_chkAutoStart;
	CCtrlCheck m_chkOnlyRun;

public:
	COptionsDialog() :
		CDlgBase(g_plugin, IDD_OPT_ASSOCLIST),
		m_lvAssocList(this, IDC_ASSOCLIST),
		m_chkAutoStart(this, IDC_AUTOSTART),
		m_chkOnlyRun(this, IDC_ONLYWHILERUNNING)
	{
		// only while running
		CreateLink(m_chkOnlyRun, "OnlyWhileRunning", DBVT_BYTE, SETTING_ONLYWHILERUNNING_DEFAULT);
		// autostart
		wchar_t *pszRunCmd = MakeRunCommand(TRUE, TRUE);
		if (pszRunCmd != nullptr) {
			m_chkAutoStart.SetState(IsRegRunEntry(L"MirandaNG", pszRunCmd));
			mir_free(pszRunCmd);
		}

		m_lvAssocList.OnDeleteItem = Callback(this, &COptionsDialog::OnAssocListItemDeleted);
		m_lvAssocList.OnItemChanged = Callback(this, &COptionsDialog::OnAssocListItemChanged);
		m_lvAssocList.OnKeyDown = Callback(this, &COptionsDialog::OnAssocListKeyDown);
	}

	bool OnInitDialog() override
	{
		CDlgBase::OnInitDialog();
		m_lvAssocList.SetUnicodeFormat(true);
		m_lvAssocList.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
		// columns
		LVCOLUMN lvc;
		lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		lvc.pszText = TranslateT("Type");
		lvc.cx = 170;
		m_lvAssocList.InsertColumn(lvc.iSubItem = 0, &lvc);
		lvc.pszText = TranslateT("Description");
		m_lvAssocList.InsertColumn(lvc.iSubItem = 1, &lvc);
		// create image storage
		HIMAGELIST himl;
		mir_cslock lck(csAssocList);
		{
			HDC hdc = GetDC(m_lvAssocList.GetHwnd());
			if (hdc != nullptr) { // BITSPIXEL is compatible with ILC_COLOR flags
				himl = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), GetDeviceCaps(hdc, BITSPIXEL) | ILC_MASK, arAssocList.getCount(), 0);
				ReleaseDC(m_lvAssocList.GetHwnd(), hdc);
			}
			else
				himl = nullptr;
		}
		m_lvAssocList.SetImageList(himl, LVSIL_SMALL);

		// enum assoc list
		LVITEM lvi;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
		for (auto &it : arAssocList) {
			lvi.iItem = 0;
			lvi.lParam = (LPARAM)new ASSOCDATA(*it);
			lvi.pszText = GetAssocTypeDesc(it);
			lvi.iImage = ReplaceImageListAssocIcon(himl, it, -1);
			lvi.iItem = m_lvAssocList.InsertItem(&lvi);
			if (lvi.iItem != -1) {
				m_lvAssocList.SetItemText(lvi.iItem, 1, it->pszDescription);
				m_lvAssocList.SetCheckState(lvi.iItem, IsAssocEnabled(it) && IsAssocRegistered(it));
			}
		}
		// sort items (before moving to groups)
		m_lvAssocList.SortItems(ListViewSortDesc, Langpack_GetDefaultLocale());
		// groups
		if (m_lvAssocList.EnableGroupView(TRUE) == 1) { // returns 0 on pre WinXP or if commctls6 are disabled
			LVGROUP lvg;
			int iItem;
			// dummy item for group
			lvi.iItem = m_lvAssocList.GetItemCount() - 1;
			lvi.iSubItem = 0;
			lvi.mask = LVIF_PARAM | LVIF_IMAGE;
			lvi.iImage = -1;
			lvi.lParam = 0;
			// insert groups
			lvg.cbSize = sizeof(lvg);
			lvg.mask = LVGF_HEADER | LVGF_GROUPID;
			lvg.iGroupId = 2;
			lvg.pszHeader = TranslateT("URLs on websites");
			lvi.iItem = m_lvAssocList.InsertItem(&lvi);
			if (lvi.iItem != -1) {
				m_lvAssocList.InsertGroup(lvi.iItem, &lvg);
				lvg.iGroupId = 1;
				lvg.pszHeader = TranslateT("File types");
				iItem = lvi.iItem = m_lvAssocList.InsertItem(&lvi);
				if (lvi.iItem != -1)
					m_lvAssocList.InsertGroup(lvi.iItem, &lvg);
				else
					m_lvAssocList.DeleteItem(iItem);
			}
			// move to group
			lvi.iSubItem = 0;
			lvi.mask = LVIF_PARAM | LVIF_GROUPID;
			for (lvi.iItem = 0; m_lvAssocList.GetItem(&lvi); ++lvi.iItem) {
				ASSOCDATA *assoc = (ASSOCDATA*)lvi.lParam;
				if (assoc == nullptr)
					continue; // groups
				lvi.iGroupId = (assoc->pszFileExt == nullptr) + 1;
				m_lvAssocList.SetItem(&lvi);
			}
		}
		lvi.iItem = m_lvAssocList.GetTopIndex();
		m_lvAssocList.SetItemState(lvi.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_lvAssocList.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER); // size to fit window
		return true;
	}

	void OnDestroy() override
	{
		LVITEM lvi;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM;
		mir_cslock lck(csAssocList);
		for (lvi.iItem = 0; m_lvAssocList.GetItem(&lvi); ++lvi.iItem) {
			ASSOCDATA *assoc = (ASSOCDATA*)lvi.lParam;
			delete assoc;
		}
	}

	void RefreshIcons()
	{
		HIMAGELIST himl = ListView_GetImageList(m_lvAssocList.GetHwnd(), LVSIL_SMALL);
		// enum items
		LVITEM lvi;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM | LVIF_IMAGE;
		for (lvi.iItem = 0; m_lvAssocList.GetItem(&lvi); ++lvi.iItem) {
			ASSOCDATA *assoc = (ASSOCDATA*)lvi.lParam;
			if (assoc == nullptr)
				continue; // groups
			lvi.iImage = ReplaceImageListAssocIcon(himl, assoc, lvi.iImage);
			m_lvAssocList.SetItem(&lvi);
		}
		if (lvi.iItem) { // ListView_Update() blinks
			m_lvAssocList.RedrawItems(0, lvi.iItem - 1);
			UpdateWindow(m_lvAssocList.GetHwnd());
		}
	}

	void OnAssocListItemDeleted(CCtrlListView::TEventInfo *evt)
	{
		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.iItem = evt->nmlv->iItem;
		// free memory
		if (m_lvAssocList.GetItem(&lvi))
			mir_free((ASSOCDATA*)lvi.lParam); // does NULL check
	}

	void OnAssocListItemChanged(CCtrlListView::TEventInfo *)
	{
		// enable apply (not while loading)
		if (IsWindowVisible(m_lvAssocList.GetHwnd()))
			NotifyChange();
	}

	void OnAssocListKeyDown(CCtrlListView::TEventInfo *evt)
	{
		// workaround for WinXP (ListView with groups):
		// eat keyboard navigation that goes beyond the first item in list
		// as it would scroll out of scope in this case
		// bug should not be present using WinVista and higher
		switch (evt->nmlvkey->wVKey) {
			LVITEM lvi;
		case VK_UP:
			lvi.iSubItem = 0;
			lvi.mask = LVIF_PARAM;
			lvi.iItem = m_lvAssocList.GetNextItem(-1, LVNI_FOCUSED);
			lvi.iItem = m_lvAssocList.GetNextItem(lvi.iItem, LVNI_ABOVE);
			if (lvi.iItem != -1)
				if (m_lvAssocList.GetItem(&lvi))
					if ((ASSOCDATA*)lvi.lParam == nullptr) // groups
						lvi.iItem = -1;
			/*if (lvi.iItem == -1) {
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE); // eat it
				return TRUE;
			}*/
			break;

		case VK_PRIOR:
			lvi.iSubItem = 0;
			lvi.mask = LVIF_PARAM;
			lvi.iItem = m_lvAssocList.GetNextItem(-1, LVNI_FOCUSED);
			lvi.iItem -= m_lvAssocList.GetCountPerPage();
			if (lvi.iItem >= 0)
				if (m_lvAssocList.GetItem(&lvi))
					if ((ASSOCDATA*)lvi.lParam == nullptr) // groups
						lvi.iItem = -1;
			if (lvi.iItem < 0) {
				m_lvAssocList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				//SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE); // eat it
				return;
			}
			break;
		}
	}

	bool OnApply() override
	{
		BOOL fEnabled, fRegFailed = FALSE;

		// only while running
		g_plugin.setByte("OnlyWhileRunning", m_chkOnlyRun.GetState() != 0);

		// save enabled assoc items
		LVITEM lvi;
		lvi.iSubItem = 0;
		lvi.mask = LVIF_PARAM;
		mir_cslock lck(csAssocList);
		for (lvi.iItem = 0; m_lvAssocList.GetItem(&lvi); ++lvi.iItem) {
			ASSOCDATA *assoc = (ASSOCDATA*)lvi.lParam;
			if (assoc == nullptr)
				continue; // groups
			fEnabled = m_lvAssocList.GetCheckState(lvi.iItem);
			SetAssocEnabled(assoc, fEnabled);

			// re-register registery keys
			if (fEnabled ? !EnsureAssocRegistered(assoc) : !UnregisterAssoc(assoc)) {
				char *pszErr = GetWinErrorDescription(GetLastError());
				ShowInfoMessage(NIIF_ERROR, Translate("File association error"), Translate("There was an error writing to the registry to modify the file/url associations.\nReason: %s"), (pszErr != nullptr) ? pszErr : Translate("Unknown"));
				mir_free(pszErr); // does NULL check
				fRegFailed = TRUE; // just show one time
			}
		}
		NotifyAssocChange(TRUE);
		RefreshIcons();

		// autostart
		wchar_t *pszRunCmd = MakeRunCommand(TRUE, TRUE);
		fRegFailed = FALSE;
		if (pszRunCmd != nullptr) {
			fEnabled = m_chkAutoStart.GetState();
			if (fEnabled ? !AddRegRunEntry(L"MirandaNG", pszRunCmd) : !RemoveRegRunEntry(L"MirandaNG", pszRunCmd)) {
				char *pszErr;
				pszErr = GetWinErrorDescription(GetLastError());
				ShowInfoMessage(NIIF_ERROR, Translate("Autostart error"), Translate("There was an error writing to the registry to modify the autostart list.\n\nReason: %s"), (pszErr != nullptr) ? pszErr : Translate("Unknown"));
				mir_free(pszErr); // does NULL check
				fRegFailed = TRUE; // just show one time
			}
			mir_free(pszRunCmd);
		}
		return true;
	}
};

static int AssocListOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.a = LPGEN("Services"); // autotranslated
	odp.szTitle.a = LPGEN("Associations"); // autotranslated
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new COptionsDialog;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/************************* Misc ***********************************/

void InitAssocList(void)
{
	// Options
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icc);

	HookEvent(ME_OPT_INITIALISE, AssocListOptInit);

	// Services
	CreateServiceFunction(MS_ASSOCMGR_ADDNEWFILETYPE, ServiceAddNewFileType);
	CreateServiceFunction(MS_ASSOCMGR_REMOVEFILETYPE, ServiceRemoveFileType);
	CreateServiceFunction(MS_ASSOCMGR_ADDNEWURLTYPE, ServiceAddNewUrlType);
	CreateServiceFunction(MS_ASSOCMGR_REMOVEURLTYPE, ServiceRemoveUrlType);

	// Notify Shell
	nNotifyTimerID = 0;

	// register open-with app
	{
		wchar_t *pszAppFileName, *pszIconLoc, *pszRunCmd;
		pszIconLoc = MakeIconLocation(nullptr, 0);

		// miranda32.exe
		pszAppFileName = MakeAppFileName(TRUE);
		pszRunCmd = MakeRunCommand(TRUE, FALSE);
		if (pszAppFileName != nullptr && pszRunCmd != nullptr)
			AddRegOpenWith(pszAppFileName, FALSE, _A2W(MIRANDANAME), pszIconLoc, pszRunCmd, nullptr, nullptr, nullptr);
		mir_free(pszRunCmd); // does NULL check
		mir_free(pszAppFileName); // does NULL check
		// assocmgr.dll
		pszAppFileName = MakeAppFileName(FALSE);
		pszRunCmd = MakeRunCommand(FALSE, TRUE);
		if (pszAppFileName != nullptr && pszRunCmd != nullptr)
			AddRegOpenWith(pszAppFileName, TRUE, _A2W(MIRANDANAME), pszIconLoc, pszRunCmd, DDEFILECMD, DDEAPP, DDETOPIC);
		mir_free(pszRunCmd); // does NULL check
		mir_free(pszAppFileName); // does NULL check

		mir_free(pszIconLoc); // does NULL check
	}

	// default items
	{
		FILETYPEDESC ftd;
		ftd.cbSize = sizeof(FILETYPEDESC);
		ftd.pszFileExt = ".dat";
		ftd.pszMimeType = nullptr;
		ftd.pwszDescription = TranslateT("Miranda NG database");
		ftd.hInstance = g_plugin.getInst();
		ftd.nIconResID = IDI_MIRANDAFILE;
		ftd.pwszVerbDesc = nullptr;
		ftd.pszService = nullptr;
		ftd.flags = FTDF_DEFAULTDISABLED | FTDF_UNICODE;
		ServiceAddNewFileType(0, (LPARAM)&ftd);
	}
}

void UninitAssocList(void)
{
	// Assoc List
	uint8_t fOnlyWhileRunning = g_plugin.getByte("OnlyWhileRunning", SETTING_ONLYWHILERUNNING_DEFAULT);
	for (auto &it : arAssocList)
		if (fOnlyWhileRunning)
			UnregisterAssoc(it); // remove registry keys

	// Notify Shell
	if (fOnlyWhileRunning && arAssocList.getCount())
		NotifyAssocChange(TRUE);
	arAssocList.destroy();

	// unregister open-with app
	if (fOnlyWhileRunning) {
		// miranda32.exe
		ptrW pszAppFileName(MakeAppFileName(TRUE));
		if (pszAppFileName != NULL)
			RemoveRegOpenWith(pszAppFileName);

		// assocmgr.dll
		pszAppFileName = MakeAppFileName(FALSE);
		if (pszAppFileName != NULL)
			RemoveRegOpenWith(pszAppFileName);
	}
}
