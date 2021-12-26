#include "stdafx.h"

#define F_CASE	    0x0001
#define F_EXACT    0x0002
#define F_MODNAME  0x0004
#define F_SETNAME  0x0008
#define F_SETVAL   0x0010
#define F_ENTIRE   0x0020
#define F_NUMSRCH  0x0040
#define F_NUMREPL  0x0080
#define F_UNICODE  0x0100
#define F_REPLACED 0x0200
#define F_DELETED  0x0400

static CMOption<bool> g_bSearchModule(MODULENAME, "SearchModules", true);
static CMOption<bool> g_bSearchSetting(MODULENAME, "SearchSettings", true);
static CMOption<bool> g_bSearchValues(MODULENAME, "SearchValues", true);
static CMOption<bool> g_bReplaceAll(MODULENAME, "ReplaceAll", false);
static CMOption<bool> g_bExactMatch(MODULENAME, "ExactMatch", false);
static CMOption<bool> g_bCaseSensitive(MODULENAME, "CaseSensitive", false);

/////////////////////////////////////////////////////////////////////////////////////////

static int FindMatchA(const char *text, char *search, int options)
{
	if (!search[0] && (!(options & F_EXACT) || !text[0]))
		return 1;

	if (options & F_EXACT)
		return (options & F_CASE) ? !strcmp(text, search) : !stricmp(text, search);

	// on empty string strstr() returns full string while StrStrI() returns NULL 	
	return (options & F_CASE) ? (INT_PTR)strstr(text, search) : (INT_PTR)StrStrIA(text, search);
}


static int FindMatchW(const wchar_t *text, wchar_t *search, int options)
{
	if (!search[0] && (!(options & F_EXACT) || !text[0]))
		return 1;

	if (options & F_EXACT)
		return (options & F_CASE) ? !wcscmp(text, search) : !wcsicmp(text, search);

	// on empty string strstr() returns full string while StrStrI() returns NULL 	
	return (options & F_CASE) ? (INT_PTR)wcsstr(text, search) : (INT_PTR)StrStrIW(text, search);
}

/////////////////////////////////////////////////////////////////////////////////////////

static char* multiReplaceA(const char *value, const char *search, const char *replace, int cs)
{
	int slen = (int)mir_strlen(search);
	int rlen = (int)mir_strlen(replace);
	int vlen = (int)mir_strlen(value);
	int ci = slen ? cs : 1; // on empty string strstr() returns full string while StrStrI() returns NULL 
									// let's try to calculate maximum length for result string
	int newlen = (!slen) ? rlen + 1 : ((rlen <= slen) ? vlen + 1 : vlen * rlen / slen + 1);

	char *head;
	char *in = (char *)value;
	char *out = (char *)mir_alloc(newlen * sizeof(char));
	out[0] = 0;

	while (head = ci ? strstr(in, search) : StrStrIA(in, search)) {
		if (head != in)
			mir_strncat(out, in, head - in + 1);
		in = head + slen;
		mir_strcat(out, replace);
	}

	mir_strcat(out, in);
	return out;
}

static wchar_t* multiReplaceW(const wchar_t *value, const wchar_t *search, const wchar_t *replace, int cs)
{
	int slen = (int)mir_wstrlen(search);
	int rlen = (int)mir_wstrlen(replace);
	int vlen = (int)mir_wstrlen(value);
	int ci = slen ? cs : 1; // on empty string strstr() returns full string while StrStrI() returns NULL 
									// let's try to calculate maximum length for result string
	int newlen = (!slen) ? rlen + 1 : ((rlen <= slen) ? vlen + 1 : vlen * rlen / slen + 1);

	wchar_t *in = (wchar_t *)value;
	wchar_t *out = (wchar_t *)mir_alloc(newlen * sizeof(wchar_t));
	out[0] = 0;

	while (wchar_t *head = ci ? wcsstr(in, search) : StrStrIW(in, search)) {
		if (head != in)
			mir_wstrncat(out, in, head - in + 1);
		in = head + slen;
		mir_wstrcat(out, replace);
	}

	mir_wstrcat(out, in);
	return out;
}

/////////////////////////////////////////////////////////////////////////////////////////

static ColumnsSettings csResultList[] =
{
	{ LPGENW("Result"),  0, "Search0width", 100 },
	{ LPGENW("Contact"), 1, "Search1width", 100 },
	{ LPGENW("Module"),  2, "Search2width", 100 },
	{ LPGENW("Setting"), 3, "Search3width", 100 },
	{ LPGENW("Value"),   4, "Search4width", 150 },
	{ nullptr }
};

class CFindWindowDlg : public CDlgBase
{
	struct FindInfo
	{
		CFindWindowDlg *pDlg; // hwnd to item list
		int options; // or'd about items
		ptrW search; // text to find
		ptrW replace; // text to replace
	};

	bool bSearchActive = false, bFirstMatch = false;
	int lastColumn = -1;

	static void __cdecl FindSettings(LPVOID param)
	{
		FindInfo *fi = (FindInfo *)param;
		auto *pDlg = fi->pDlg;
		pDlg->bSearchActive = pDlg->bFirstMatch = true;

		ModuleSettingLL ModuleList, SettingList;
		ModSetLinkLinkItem *module, *setting;

		MCONTACT hContact;
		DBVARIANT dbv = {};

		int foundCount = 0, replaceCount = 0, deleteCount = 0;

		uint32_t numsearch = 0, numreplace = 0;
		int NULLContactDone = 0;

		if (!fi->search || !EnumModules(&ModuleList)) {
			delete fi;
			return;
		}

		_T2A search(fi->search);
		_T2A replace(fi->replace);

		// skip modules and setting names on unicode search or replace
		if (IsRealUnicode(fi->search) || IsRealUnicode(fi->replace)) {
			fi->options &= ~(F_SETNAME | F_MODNAME);
			fi->options |= F_UNICODE;
		}

		if (!(fi->options & F_UNICODE) && (fi->options & F_SETVAL)) {
			char val[16];
			numsearch = strtoul(search, nullptr, 10);
			_ultoa(numsearch, val, 10);
			if (!mir_strcmp(search, val)) {
				fi->options |= F_NUMSRCH;
				// replace numeric values only entirely
				if (replace && (fi->options & F_ENTIRE)) {
					numreplace = strtoul(replace, nullptr, 10);
					_ultoa(numreplace, val, 10);
					if (!replace[0] || !mir_strcmp(replace, val))
						fi->options |= F_NUMREPL;
				}
			}
		}

		pDlg->m_sb.SendMsg(SB_SETTEXT, 0, (LPARAM)TranslateT("Searching..."));

		hContact = 0;

		while (pDlg->bSearchActive) {

			if (!hContact) {
				if (NULLContactDone)
					break;
				else {
					NULLContactDone = 1;
					hContact = db_find_first();
				}
			}
			else hContact = db_find_next(hContact);

			for (module = ModuleList.first; module; module = module->next) {

				if (IsModuleEmpty(hContact, module->name))
					continue;

				if (fi->options & (F_SETVAL | F_SETNAME)) {

					if (!EnumSettings(hContact, module->name, &SettingList)) {
						delete fi;
						FreeModuleSettingLL(&ModuleList);
						return;
					}

					for (setting = SettingList.first; setting; setting = setting->next) {

						dbv.type = 0;
						if (db_get_s(hContact, module->name, setting->name, &dbv, 0))
							continue;

						// check in settings value				
						if (fi->options & F_SETVAL) {

							wchar_t *value = nullptr;

							switch (dbv.type) {
							case DBVT_BYTE:
							case DBVT_WORD:
							case DBVT_DWORD:
								if ((fi->options & F_NUMSRCH) && numsearch == getNumericValue(&dbv)) {
									wchar_t *val = fi->search;
									int flag = F_SETVAL;

									if (fi->options & F_NUMREPL) {
										if (replace[0]) {
											db_unset(hContact, module->name, setting->name);
											flag |= F_DELETED;
											deleteCount++;
										}
										else if (setNumericValue(hContact, module->name, setting->name, numreplace, dbv.type)) {
											val = fi->replace;
											flag |= F_REPLACED;
											replaceCount++;
										}
									}

									pDlg->ItemFound(hContact, module->name, setting->name, val, flag);
								}
								break;

							case DBVT_WCHAR:
								if (!value) value = mir_wstrdup(dbv.pwszVal);
								__fallthrough;
							case DBVT_UTF8:
								if (!value) value = mir_utf8decodeW(dbv.pszVal);
								__fallthrough;
							case DBVT_ASCIIZ:
								if (!value) value = mir_a2u(dbv.pszVal);

								if (FindMatchW(value, fi->search, fi->options)) {
									foundCount++;
									ptrW ptr;
									wchar_t *newValue = value;
									int flag = F_SETVAL;

									if (fi->replace) {
										newValue = (fi->options & F_ENTIRE) ? fi->replace : ptr = multiReplaceW(value, fi->search, fi->replace, fi->options & F_CASE);
										// !!!! delete or make empty ?
										if (!newValue[0]) {
											db_unset(hContact, module->name, setting->name);
											flag |= F_DELETED;
											newValue = value;
											deleteCount++;
										}
										else {
											// save as unicode if needed
											if (dbv.type != DBVT_ASCIIZ || IsRealUnicode(newValue))
												db_set_ws(hContact, module->name, setting->name, newValue);
											else
												db_set_s(hContact, module->name, setting->name, _T2A(newValue));
											flag |= F_REPLACED;
											replaceCount++;
										}
									}

									pDlg->ItemFound(hContact, module->name, setting->name, newValue, flag);
								}
								mir_free(value);
								break;
							} // switch
						}

						// check in setting name
						if ((fi->options & F_SETNAME) && FindMatchA(setting->name, search, fi->options)) {
							foundCount++;
							ptrA ptr;
							char *newSetting = setting->name;
							int flag = F_SETNAME;

							if (replace) {
								newSetting = (fi->options & F_ENTIRE) ? replace : ptr = multiReplaceA(setting->name, search, replace, fi->options & F_CASE);
								if (!newSetting[0]) {
									db_unset(hContact, module->name, setting->name);
									flag |= F_DELETED;
									newSetting = setting->name;
									deleteCount++;
								}
								else {
									DBVARIANT dbv2;
									// skip if exist
									if (!db_get_s(hContact, module->name, newSetting, &dbv2, 0))
										db_free(&dbv2);
									else if (!db_set(hContact, module->name, newSetting, &dbv)) {
										db_unset(hContact, module->name, setting->name);
										flag |= F_REPLACED;
										replaceCount++;
									}
								}
							}

							pDlg->ItemFound(hContact, module->name, newSetting, nullptr, flag);
						}

						db_free(&dbv);

					} // for(setting)

					FreeModuleSettingLL(&SettingList);
				}

				// check in module name
				if ((fi->options & F_MODNAME) && FindMatchA(module->name, search, fi->options)) {
					foundCount++;
					char *newModule = module->name;
					int flag = F_MODNAME;
					ptrA ptr;

					if (replace) {
						newModule = (fi->options & F_ENTIRE) ? replace : ptr = multiReplaceA(module->name, search, replace, fi->options & F_CASE);
						if (!newModule[0]) {
							deleteModule(g_pMainWindow->GetHwnd(), hContact, module->name, 0);
							g_pMainWindow->replaceTreeItem(hContact, module->name, nullptr);
							flag |= F_DELETED;
							newModule = module->name;
							deleteCount++;
						}
						else if (renameModule(hContact, module->name, newModule)) {
							g_pMainWindow->replaceTreeItem(hContact, module->name, nullptr);
							flag |= F_REPLACED;
							replaceCount++;
						}
					}

					pDlg->ItemFound(hContact, newModule, nullptr, nullptr, flag);
				}
			} // for(module)
		}

		wchar_t msg[MSG_SIZE];
		mir_snwprintf(msg, TranslateT("Finished. Items found: %d / replaced: %d / deleted: %d"), foundCount, replaceCount, deleteCount);
		pDlg->m_sb.SendMsg(SB_SETTEXT, 0, (LPARAM)msg);

		if (fi->replace) {
			pDlg->btnReplace.SetText(TranslateT("&Replace"));
			pDlg->btnSearch.Enable();
		}
		else {
			pDlg->btnSearch.SetText(TranslateT("&Search"));
			pDlg->btnReplace.Enable();
		}

		delete fi;
		FreeModuleSettingLL(&ModuleList);

		pDlg->bSearchActive = false;
		EnableWindow(GetDlgItem(pDlg->GetHwnd(), IDCANCEL), 1);
	}

	void ItemFound(MCONTACT hContact, const char *module, const char *setting, wchar_t *value, int type)
	{
		if (bFirstMatch) {
			SetFocus(m_results.GetHwnd());
			bFirstMatch = false;
		}

		wchar_t *mode;
		if (type & F_REPLACED)
			mode = TranslateT("Replaced");
		else if (type & F_DELETED)
			mode = TranslateT("Deleted");
		else
			mode = TranslateT("Found");

		wchar_t name[NAME_SIZE];
		GetContactName(hContact, nullptr, name, _countof(name));

		LVITEM lvi = {};
		lvi.mask = LVIF_PARAM;
		lvi.lParam = (LPARAM)hContact;

		int index = m_results.InsertItem(&lvi);
		m_results.SetItemText(index, 0, mode);
		m_results.SetItemText(index, 1, name);
		m_results.SetItemText(index, 2, _A2T(module));

		int F_type = type & 0xFF;

		if (F_type == F_SETNAME || F_type == F_SETVAL)
			m_results.SetItemText(index, 3, _A2T(setting));

		if (F_type == F_SETVAL && value)
			m_results.SetItemText(index, 4, value);
	}

	void OpenSettings(int iItem)
	{
		MCONTACT hContact = (MCONTACT)m_results.GetItemData(iItem);
		if (hContact == INVALID_CONTACT_ID)
			return;

		char szModule[NAME_SIZE], szSetting[NAME_SIZE];
		ListView_GetItemTextA(m_results.GetHwnd(), iItem, 2, szModule, _countof(szModule));
		ListView_GetItemTextA(m_results.GetHwnd(), iItem, 3, szSetting, _countof(szSetting));

		g_pMainWindow->FindItem((szSetting[0]) ? FW_SETTINGNAME : FW_MODULE, hContact, szModule, szSetting);
	}

	CCtrlBase m_sb;
	CCtrlCheck chkModules, chkSettings, chkValues, chkReplaceAll, chkExactMatch, chkCaseSensitive;
	CCtrlButton btnSearch, btnReplace;
	CCtrlListView m_results;

public: 
	CFindWindowDlg() : 
		CDlgBase(g_plugin, IDD_FIND),
		m_sb(this, IDC_SBAR),
		m_results(this, IDC_LIST),
		btnSearch(this, IDOK),
		btnReplace(this, IDC_BTNREPLACE),
		chkModules(this, IDC_MODNAME),
		chkSettings(this, IDC_SETTINGNAME),
		chkValues(this, IDC_SETTINGVALUE),
		chkExactMatch(this, IDC_EXACT),
		chkReplaceAll(this, IDC_ENTIRELY),
		chkCaseSensitive(this, IDC_CASESENSITIVE)
	{
		SetParent(g_pMainWindow->GetHwnd());
		SetMinSize(610, 300);

		CreateLink(chkModules, g_bSearchModule);
		CreateLink(chkSettings, g_bSearchSetting);
		CreateLink(chkValues, g_bSearchValues);
		CreateLink(chkReplaceAll, g_bReplaceAll);
		CreateLink(chkExactMatch, g_bExactMatch);
		CreateLink(chkCaseSensitive, g_bCaseSensitive);

		btnReplace.OnClick = Callback(this, &CFindWindowDlg::onClick_Search);

		m_results.OnColumnClick = Callback(this, &CFindWindowDlg::onColumnClick_List);
		m_results.OnDoubleClick = Callback(this, &CFindWindowDlg::onDblClick_List);
	}

	bool OnInitDialog() override
	{
		m_sb.SendMsg(SB_SETTEXT, 0, (LPARAM)TranslateT("Enter a string to search the database for"));
		CheckDlgButton(m_hwnd, IDC_FOUND, BST_CHECKED);
		SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(ICO_REGEDIT)));
		SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_APPWINDOW); // taskbar icon
		m_results.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP); // LVS_EX_GRIDLINES
		loadListSettings(m_results.GetHwnd(), csResultList);
		Utils_RestoreWindowPositionNoMove(m_hwnd, NULL, MODULENAME, "Search_");
		return true;
	}

	bool OnApply() override
	{
		if (::GetFocus() == m_results.GetHwnd())
			OpenSettings(m_results.GetSelectionMark());
		else
			onClick_Search(&btnSearch);
		return false;
	}

	void OnDestroy() override
	{
		m_results.DeleteAllItems();
		saveListSettings(m_results.GetHwnd(), csResultList);
		Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "Search_");
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_LIST:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
		case IDC_SBAR:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}

	void onClick_Search(CCtrlButton *pButton)
	{
		// stop the search, if it's active
		if (bSearchActive) {
			bSearchActive = false;
			return;
		}

		chkModules.OnApply(); chkSettings.OnApply(); chkValues.OnApply();
		chkReplaceAll.OnApply(); chkExactMatch.OnApply(); chkCaseSensitive.OnApply();

		wchar_t text[FLD_SIZE];
		if (!GetDlgItemText(m_hwnd, IDC_TEXT, text, _countof(text)) && !g_bExactMatch)
			return;

		// empty replace is done only for exact match or entire replace
		wchar_t replace[FLD_SIZE] = {};
		if (pButton == &btnReplace && !GetDlgItemText(m_hwnd, IDC_REPLACE, replace, _countof(replace)) && !g_bReplaceAll && !g_bExactMatch)
			return;

		if (!g_bSearchModule && !g_bSearchSetting && !g_bSearchValues)
			return;

		FindInfo *fi = new FindInfo();
		fi->pDlg = this;
		fi->options = (g_bCaseSensitive ? F_CASE : 0) | (g_bExactMatch ? F_EXACT : 0) | (g_bSearchModule ? F_MODNAME : 0) | (g_bSearchSetting ? F_SETNAME : 0) | (g_bSearchValues ? F_SETVAL : 0);

		if (pButton == &btnReplace) {
			if (g_bReplaceAll)
				fi->options |= F_ENTIRE;

			fi->replace = mir_wstrdup(replace);

			btnReplace.SetText(TranslateT("Stop"));
			btnSearch.Disable();

		}
		else {
			btnSearch.SetText(TranslateT("Stop"));
			btnReplace.Disable();
		}

		fi->search = mir_wstrdup(text);

		m_results.DeleteAllItems();

		EnableWindow(GetDlgItem(m_hwnd, IDCANCEL), 0);
		mir_forkthread(FindSettings, fi);
	}

	void onDblClick_List(CCtrlListView::TEventInfo *ev)
	{
		LVHITTESTINFO hti;
		hti.pt = ev->nmlv->ptAction;
		if (m_results.SubItemHitTest(&hti) == -1)
			return;

		if (hti.flags & LVHT_ONITEM)
			OpenSettings(hti.iItem);
	}

	void onColumnClick_List(CCtrlListView::TEventInfo *ev)
	{
		ColumnsSortParams params;
		params.hList = m_results.GetHwnd();
		params.column = ev->nmlv->iSubItem;
		params.last = lastColumn;
		m_results.SortItemsEx(ColumnsCompare, (LPARAM)&params);
		lastColumn = (params.column == lastColumn) ? -1 : params.column;
	}
};

void newFindWindow()
{
	(new CFindWindowDlg())->Create();
}
