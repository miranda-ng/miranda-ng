/*

Import plugin for Miranda NG

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)

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

#include "import.h"

#define SetProgress(n)  SendMessage(hdlgProgress,PROGM_SETPROGRESS,n,0)

struct AccountMap
{
	AccountMap(const char *_src, int _origIdx, const TCHAR *_srcName) :
		szSrcAcc(mir_strdup(_src)),
		iSrcIndex(_origIdx),
		tszSrcName(mir_tstrdup(_srcName)),
		pa(NULL)
	{}

	~AccountMap() {}

	ptrA szSrcAcc, szBaseProto;
	ptrT tszSrcName;
	int iSrcIndex;
	PROTOACCOUNT *pa;
};

static int CompareAccs(const AccountMap *p1, const AccountMap *p2)
{	return stricmp(p1->szSrcAcc, p2->szSrcAcc);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct ContactMap
{
	ContactMap(MCONTACT _src, MCONTACT _dst) :
		srcID(_src),
		dstID(_dst)
	{}

	MCONTACT srcID, dstID;
};

/////////////////////////////////////////////////////////////////////////////////////////

static OBJLIST<AccountMap> arAccountMap(5, CompareAccs);
static OBJLIST<ContactMap> arContactMap(50, NumericKeySortT);
static LIST<DBCachedContact> arMetas(10);

/////////////////////////////////////////////////////////////////////////////////////////
// local data

static HWND hdlgProgress;
static DWORD nDupes, nContactsCount, nMessagesCount, nGroupsCount, nSkippedEvents, nSkippedContacts;
static MIDatabase *srcDb, *dstDb;

/////////////////////////////////////////////////////////////////////////////////////////

void AddMessage(const TCHAR* fmt, ...)
{
	va_list args;
	TCHAR msgBuf[4096];
	va_start(args, fmt);
	mir_vsntprintf(msgBuf, SIZEOF(msgBuf), TranslateTS(fmt), args);

	SendMessage(hdlgProgress, PROGM_ADDMESSAGE, 0, (LPARAM)msgBuf);
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool CompareDb(DBVARIANT &dbv1, DBVARIANT &dbv2)
{
	if (dbv1.type == dbv2.type) {
		switch (dbv1.type) {
		case DBVT_DWORD:
			return dbv1.dVal == dbv2.dVal;

		case DBVT_ASCIIZ:
		case DBVT_UTF8:
			return 0 == strcmp(dbv1.pszVal, dbv2.pszVal);
		}
	}
	return false;
}

static int myGet(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	dbv->type = 0;
	return srcDb->GetContactSetting(hContact, szModule, szSetting, dbv);
}

static int myGetD(MCONTACT hContact, const char *szModule, const char *szSetting, int iDefault)
{
	DBVARIANT dbv = { DBVT_DWORD };
	return srcDb->GetContactSetting(hContact, szModule, szSetting, &dbv) ? iDefault : dbv.dVal;
}

static TCHAR* myGetWs(MCONTACT hContact, const char *szModule, const char *szSetting)
{
	DBVARIANT dbv = { DBVT_TCHAR };
	return srcDb->GetContactSettingStr(hContact, szModule, szSetting, &dbv) ? NULL : dbv.ptszVal;
}

static BOOL myGetS(MCONTACT hContact, const char *szModule, const char *szSetting, char *dest)
{
	DBVARIANT dbv = { DBVT_ASCIIZ };
	dbv.pszVal = dest; dbv.cchVal = 100;
	return srcDb->GetContactSettingStatic(hContact, szModule, szSetting, &dbv);
}

/////////////////////////////////////////////////////////////////////////////////////////

static MCONTACT HContactFromNumericID(char *pszProtoName, char *pszSetting, DWORD dwID)
{
	for (MCONTACT hContact = dstDb->FindFirstContact(); hContact; hContact = dstDb->FindNextContact(hContact)) {
		if (db_get_dw(hContact, pszProtoName, pszSetting, 0) == dwID) {
			char* szProto = GetContactProto(hContact);
			if (szProto != NULL && !mir_strcmp(szProto, pszProtoName))
				return hContact;
		}
	}
	return INVALID_CONTACT_ID;
}

static MCONTACT HContactFromID(char *pszProtoName, char *pszSetting, TCHAR *pwszID)
{
	for (MCONTACT hContact = dstDb->FindFirstContact(); hContact; hContact = dstDb->FindNextContact(hContact)) {
		char *szProto = GetContactProto(hContact);
		if (!mir_strcmp(szProto, pszProtoName)) {
			ptrW id(db_get_tsa(hContact, pszProtoName, pszSetting));
			if (!mir_tstrcmp(pwszID, id))
				return hContact;
		}
	}
	return INVALID_CONTACT_ID;
}

static MCONTACT HistoryImportFindContact(HWND hdlgProgress, char *szModuleName, DWORD uin, int addUnknown)
{
	MCONTACT hContact = HContactFromNumericID(szModuleName, "UIN", uin);
	if (hContact == NULL) {
		AddMessage(LPGENT("Ignored event from/to self"));
		return INVALID_CONTACT_ID;
	}

	if (hContact != INVALID_CONTACT_ID)
		return hContact;

	if (!addUnknown)
		return INVALID_CONTACT_ID;

	hContact = CallService(MS_DB_CONTACT_ADD, 0, 0);
	CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)szModuleName);
	db_set_dw(hContact, szModuleName, "UIN", uin);
	AddMessage(LPGENT("Added contact %u (found in history)"), uin);
	return hContact;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int CopySettingsEnum(const char *szSetting, LPARAM lParam)
{
	LIST<char> *pSettings = (LIST<char>*)lParam;
	pSettings->insert(mir_strdup(szSetting));
	return 0;
}

void CopySettings(MCONTACT srcID, const char *szSrcModule, MCONTACT dstID, const char *szDstModule)
{
	LIST<char> arSettings(50);

	DBCONTACTENUMSETTINGS dbces = { 0 };
	dbces.szModule = szSrcModule;
	dbces.pfnEnumProc = CopySettingsEnum;
	dbces.lParam = (LPARAM)&arSettings;
	srcDb->EnumContactSettings(srcID, &dbces);

	for (int i = arSettings.getCount() - 1; i >= 0; i--) {
		DBVARIANT dbv = { 0 };
		if (!srcDb->GetContactSetting(srcID, szSrcModule, arSettings[i], &dbv))
			db_set(dstID, szDstModule, arSettings[i], &dbv);
		mir_free(arSettings[i]);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// accounts matcher dialog

static HWND hwndList, hwndCombo;
static int iPrevIndex = -1;

static void SetAccountName(int idx, PROTOACCOUNT *pa)
{
	ListView_SetItemText(hwndList, idx, 1, (pa == NULL) ? TranslateT("<New account>") : pa->tszAccountName);
}

static LRESULT CALLBACK ComboWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KILLFOCUS && LPARAM(hwnd) == lParam) {
		if (iPrevIndex != -1) {
			AccountMap *pMap = (AccountMap*)SendMessage(hwnd, CB_GETITEMDATA, 0, 0);

			int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
			if (idx == 0)
				pMap->pa = NULL;
			else
				pMap->pa = (PROTOACCOUNT*)SendMessage(hwnd, CB_GETITEMDATA, idx, 0);

			SetAccountName(iPrevIndex, pMap->pa);
			iPrevIndex = -1;
		}

		DestroyWindow(hwnd);
		hwndCombo = 0;
	}
	return mir_callNextSubclass(hwnd, ComboWndProc, uMsg, wParam, lParam);
}

static LRESULT CALLBACK ListWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_LBUTTONDOWN:
		long x = (long)LOWORD(lParam), y = (long)HIWORD(lParam);

		LVHITTESTINFO hit;
		hit.pt.x = x;
		hit.pt.y = y;
		int lResult = ListView_SubItemHitTest(hwnd, &hit);
		if (lResult == -1 || hit.iSubItem != 1) {
			SendMessage(hwndCombo, WM_KILLFOCUS, 0, (LPARAM)hwndCombo);
			break;
		}

		RECT r;
		ListView_GetSubItemRect(hwnd, hit.iItem, 1, LVIR_BOUNDS, &r);
		r.top--; r.bottom--;

		TCHAR tszText[100];
		ListView_GetItemText(hwnd, hit.iItem, 1, tszText, SIZEOF(tszText));

		LVITEM lvitem;
		lvitem.iItem = hit.iItem;
		lvitem.iSubItem = 0;
		lvitem.mask = LVIF_PARAM;
		ListView_GetItem(hwnd, &lvitem);

		if (hwndCombo != NULL)
			SendMessage(hwndCombo, WM_KILLFOCUS, 0, (LPARAM)hwndCombo);

		hwndCombo = CreateWindowEx(WS_EX_CLIENTEDGE, WC_COMBOBOX, _T(""), WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			r.left+3, r.top, r.right - r.left - 3, r.bottom - r.top, hwnd, 0, hInst, NULL);

		// copy a font from listview
		HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
		SendMessage(hwndCombo, WM_SETFONT, (WPARAM)hFont, 0);

		SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("<New account>"));
		SendMessage(hwndCombo, CB_SETITEMDATA, 0, lvitem.lParam);

		int protoCount, iSel = 0;
		PROTOACCOUNT **accs;
		ProtoEnumAccounts(&protoCount, &accs);
		for (int i = 0; i < protoCount; i++) {
			int idx = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)accs[i]->tszAccountName);
			SendMessage(hwndCombo, CB_SETITEMDATA, idx, (LPARAM)accs[i]);

			if (!mir_tstrcmpi(accs[i]->tszAccountName, tszText))
				iSel = idx;
		}

		SendMessage(hwndCombo, CB_SETCURSEL, iSel, 0);

		SetFocus(hwndCombo);
		mir_subclassWindow(hwndCombo, ComboWndProc);

		iPrevIndex = hit.iItem;
	}

	return mir_callNextSubclass(hwnd, ListWndProc, uMsg, wParam, lParam);
}

static INT_PTR CALLBACK AccountsMatcherProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		hwndAccMerge = hwndDlg;
		hwndList = GetDlgItem(hwndDlg, IDC_LIST);
		{
			LVCOLUMN col = { 0 };
			col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
			col.fmt = LVCFMT_LEFT;
			col.cx = 155;
			col.pszText = TranslateT("Old account");
			ListView_InsertColumn(hwndList, 0, &col);

			col.iSubItem = 1;
			col.pszText = TranslateT("New account");
			ListView_InsertColumn(hwndList, 1, &col);

			LVITEM lvi = { 0 };
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			for (int i = 0; i < arAccountMap.getCount(); i++) {
				AccountMap &p = arAccountMap[i];
				lvi.iItem = i;
				lvi.iSubItem = 0;
				lvi.pszText = p.tszSrcName;
				lvi.lParam = (LPARAM)&p;
				ListView_InsertItem(hwndList, &lvi);

				SetAccountName(i, p.pa);
			}
			mir_subclassWindow(hwndList, ListWndProc);
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) != BN_CLICKED)
			break;	// Only clicks of buttons are relevant, let other COMMANDs through

		switch (LOWORD(wParam)) {
		case IDOK:
			if (hwndCombo != NULL)
				SendMessage(hwndCombo, WM_KILLFOCUS, 0, (LPARAM)hwndCombo);
			EndDialog(hwndDlg, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		hwndAccMerge = NULL;
		break;

	case WM_NOTIFY:
		LPNMHDR hdr = (LPNMHDR)lParam;
		if (hdr->idFrom != IDC_LIST)
			break;

		switch (hdr->code) {
		case LVN_ITEMCHANGED:
		case LVN_ITEMACTIVATE:
			;
		}
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static char* newStr(const char *s)
{
	return (s == NULL) ? NULL : strcpy(new char[strlen(s) + 1], s);
}

static bool FindDestAccount(const char *szProto)
{
	for (int i = 0; i < arAccountMap.getCount(); i++) {
		AccountMap &pam = arAccountMap[i];
		if (pam.pa == NULL)
			continue;

		if (!strcmp(pam.pa->szModuleName, szProto))
			return true;
	}

	return false;
}

static PROTOACCOUNT* FindMyAccount(const char *szProto, const char *szBaseProto, const TCHAR *ptszName, bool bStrict)
{
	int destProtoCount;
	PROTOACCOUNT **destAccs;
	CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&destProtoCount, (LPARAM)&destAccs);

	PROTOACCOUNT *pProto = NULL;
	for (int i = 0; i < destProtoCount; i++) {
		PROTOACCOUNT *pa = destAccs[i];
		// already used? skip
		if (FindDestAccount(pa->szModuleName))
			continue;

		// different base protocotol type -> skip
		if (mir_strcmp(pa->szProtoName, szBaseProto))
			continue;

		// these protocols have no accounts, and their name match -> success
		if (pa->bOldProto || pa->bIsVirtual || pa->bDynDisabled)
			return pa;

		if (ptszName && !_tcscmp(pa->tszAccountName, ptszName))
			return pa;

		char *pszUniqueSetting = (char*)CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
		if (!pszUniqueSetting || INT_PTR(pszUniqueSetting) == CALLSERVICE_NOTFOUND) {
			pProto = pa;
			continue;
		}

		DBVARIANT dbSrc, dbDst;
		if (dstDb->GetContactSetting(NULL, pa->szModuleName, pszUniqueSetting, &dbDst)) {
			pProto = pa;
			continue;
		}

		bool bEqual = false;
		if (!myGet(NULL, szProto, pszUniqueSetting, &dbSrc)) {
			bEqual = CompareDb(dbSrc, dbDst);
			srcDb->FreeVariant(&dbSrc);
		}
		dstDb->FreeVariant(&dbDst);

		if (bEqual)
			return pa;
	}
	return (bStrict) ? NULL : pProto;
}

bool ImportAccounts(OBJLIST<char> &arSkippedModules)
{
	int protoCount = myGetD(NULL, "Protocols", "ProtoCount", 0);
	bool bNeedManualMerge = false;

	for (int i = 0; i < protoCount; i++) {
		char szSetting[100], szProto[100];
		itoa(i, szSetting, 10);
		if (myGetS(NULL, "Protocols", szSetting, szProto))
			continue;

		itoa(800 + i, szSetting, 10);
		ptrT tszName(myGetWs(NULL, "Protocols", szSetting));
		AccountMap *pNew = new AccountMap(szProto, i, tszName);
		arAccountMap.insert(pNew);

		// check if it's an account-based proto or an old style proto
		char szBaseProto[100];
		if (myGetS(NULL, szProto, "AM_BaseProto", szBaseProto))
			continue;

		// save base proto for the future use
		pNew->szBaseProto = mir_strdup(szBaseProto);

		// try the precise match first
		PROTOACCOUNT *pa = FindMyAccount(szProto, szBaseProto, tszName, true);
		if (pa) {
			pNew->pa = pa;
			continue;
		}

		// if fail, try to found an account by its name
		if (pa = FindMyAccount(szProto, szBaseProto, tszName, false)) {
			pNew->pa = pa;
			bNeedManualMerge = true;
		}
	}

	// all accounts to be converted automatically, no need to raise a dialog
	if (bNeedManualMerge)
		if (DialogBox(hInst, MAKEINTRESOURCE(IDD_ACCMERGE), NULL, AccountsMatcherProc) != IDOK)
			return false;

	bool bImportSysAll = (nImportOptions & IOPT_SYS_SETTINGS) != 0;

	for (int i = 0; i < arAccountMap.getCount(); i++) {
		AccountMap &p = arAccountMap[i];
		if (p.pa != NULL || p.szBaseProto == NULL || !mir_strcmp(p.szSrcAcc, META_PROTO))
			continue;

		if (!Proto_IsProtocolLoaded(p.szBaseProto)) {
			AddMessage(LPGENT("Protocol %S is not loaded, skipping account %s creation"), p.szBaseProto, p.tszSrcName);
			continue;
		}

		ACC_CREATE newacc;
		newacc.pszBaseProto = p.szBaseProto;
		newacc.pszInternal = NULL;
		newacc.ptszAccountName = p.tszSrcName;

		p.pa = ProtoCreateAccount(&newacc);
		if (p.pa == NULL) {
			AddMessage(LPGENT("Unable to create an account %s of protocol %S"), p.tszSrcName, p.szBaseProto);
			return false;
		}

		char szSetting[100];
		itoa(400 + p.iSrcIndex, szSetting, 10);
		int iVal = myGetD(NULL, "Protocols", szSetting, 1);
		itoa(400 + p.pa->iOrder, szSetting, 10);
		db_set_dw(NULL, "Protocols", szSetting, iVal);
		p.pa->bIsVisible = iVal != 0;

		itoa(600 + p.iSrcIndex, szSetting, 10);
		iVal = myGetD(NULL, "Protocols", szSetting, 1);
		itoa(600 + p.pa->iOrder, szSetting, 10);
		db_set_dw(NULL, "Protocols", szSetting, iVal);
		p.pa->bIsEnabled = iVal != 0;

		if (p.tszSrcName == NULL) {
			p.pa->tszAccountName = mir_a2t(p.pa->szModuleName);
			itoa(800 + p.pa->iOrder, szSetting, 10);
			db_set_ts(NULL, "Protocols", szSetting, p.pa->tszAccountName);
		}

		CopySettings(NULL, p.szSrcAcc, NULL, p.pa->szModuleName);
		if (bImportSysAll)
			arSkippedModules.insert(newStr(p.szSrcAcc));
	}

	CopySettings(NULL, META_PROTO, NULL, META_PROTO);
	if (bImportSysAll) {
		arSkippedModules.insert(newStr(META_PROTO));
		arSkippedModules.insert(newStr("Protocols"));
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

static MCONTACT MapContact(MCONTACT hSrc)
{
	ContactMap *pDestContact = arContactMap.find((ContactMap*)&hSrc);
	return (pDestContact == NULL) ? INVALID_CONTACT_ID : pDestContact->dstID;
}

static MCONTACT AddContact(HWND hdlgProgress, char* szProto, char* pszUniqueSetting, DBVARIANT* id, const TCHAR* pszUserID, TCHAR *nick, TCHAR *group)
{
	MCONTACT hContact = CallService(MS_DB_CONTACT_ADD, 0, 0);
	if (CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)szProto) != 0) {
		CallService(MS_DB_CONTACT_DELETE, hContact, 0);
		AddMessage(LPGENT("Failed to add %S contact %s"), szProto, pszUserID);
		return INVALID_CONTACT_ID;
	}

	db_set(hContact, szProto, pszUniqueSetting, id);

	CreateGroup(group, hContact);

	if (nick && *nick) {
		db_set_ws(hContact, "CList", "MyHandle", nick);
		AddMessage(LPGENT("Added %S contact %s, '%s'"), szProto, pszUserID, nick);
	}
	else AddMessage(LPGENT("Added %S contact %s"), szProto, pszUserID);

	srcDb->FreeVariant(id);
	return hContact;
}

void ImportContactSettings(AccountMap *pda, MCONTACT hSrc, MCONTACT hDst)
{
	if (pda->pa == NULL)
		return;

	char *szDstAcc = pda->pa->szModuleName;

	// Hidden?
	DBVARIANT dbv;
	if (!myGet(hSrc, "CList", "Hidden", &dbv)) {
		db_set(hDst, "CList", "Hidden", &dbv);
		srcDb->FreeVariant(&dbv);
	}

	// Ignore settings
	if (!myGet(hSrc, "Ignore", "Mask1", &dbv)) {
		db_set(hDst, "Ignore", "Mask1", &dbv);
		srcDb->FreeVariant(&dbv);
	}

	// Apparent mode
	if (!myGet(hSrc, pda->szSrcAcc, "ApparentMode", &dbv)) {
		db_set(hDst, szDstAcc, "ApparentMode", &dbv);
		srcDb->FreeVariant(&dbv);
	}

	// Nick
	if (!myGet(hSrc, pda->szSrcAcc, "Nick", &dbv)) {
		db_set(hDst, szDstAcc, "Nick", &dbv);
		srcDb->FreeVariant(&dbv);
	}

	// Myhandle
	if (!myGet(hSrc, pda->szSrcAcc, "MyHandle", &dbv)) {
		db_set(hDst, szDstAcc, "MyHandle", &dbv);
		srcDb->FreeVariant(&dbv);
	}

	// First name
	if (!myGet(hSrc, pda->szSrcAcc, "FirstName", &dbv)) {
		db_set(hDst, szDstAcc, "FirstName", &dbv);
		srcDb->FreeVariant(&dbv);
	}

	// Last name
	if (!myGet(hSrc, pda->szSrcAcc, "LastName", &dbv)) {
		db_set(hDst, szDstAcc, "LastName", &dbv);
		srcDb->FreeVariant(&dbv);
	}

	// About
	if (!myGet(hSrc, pda->szSrcAcc, "About", &dbv)) {
		db_set(hDst, szDstAcc, "About", &dbv);
		srcDb->FreeVariant(&dbv);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static int ImportGroup(const char* szSettingName, LPARAM lParam)
{
	int *pnGroups = (int*)lParam;

	TCHAR* tszGroup = myGetWs(NULL, "CListGroups", szSettingName);
	if (tszGroup != NULL) {
		if (CreateGroup(tszGroup + 1, NULL))
			pnGroups[0]++;
		mir_free(tszGroup);
	}
	return 0;
}

static int ImportGroups()
{
	int nGroups = 0;

	DBCONTACTENUMSETTINGS param = { 0 };
	param.szModule = "CListGroups";
	param.pfnEnumProc = ImportGroup;
	param.lParam = (LPARAM)&nGroups;
	srcDb->EnumContactSettings(NULL, &param);
	return nGroups;
}

/////////////////////////////////////////////////////////////////////////////////////////

DBCachedContact* FindDestMeta(DBCachedContact *ccSrc)
{
	for (MCONTACT hMeta = dstDb->FindFirstContact(META_PROTO); hMeta != 0; hMeta = dstDb->FindNextContact(hMeta, META_PROTO)) {
		DBCachedContact *cc = dstDb->m_cache->GetCachedContact(hMeta);
		if (cc->nSubs != ccSrc->nSubs)
			continue;

		int i;
		for (i = 0; i < ccSrc->nSubs; i++) {
			MCONTACT hDest = MapContact(ccSrc->pSubs[i]);
			if (hDest == INVALID_CONTACT_ID || cc->pSubs[i] != hDest)
				break;
		}

		if (i == ccSrc->nSubs)
			return cc;
	}

	return NULL;
}

MCONTACT FindExistingMeta(DBCachedContact *ccSrc)
{
	MCONTACT hResult = INVALID_CONTACT_ID;

	// subs of source meta must belong to the only dest meta
	for (int i = 0; i < ccSrc->nSubs; i++) {
		MCONTACT hDestSub = MapContact(ccSrc->pSubs[i]);
		if (hDestSub == INVALID_CONTACT_ID)
			continue;

		DBCachedContact *cc = dstDb->m_cache->GetCachedContact(hDestSub);
		if (cc == NULL || !cc->IsSub()) // check if it's a sub
			continue;

		if (hResult == INVALID_CONTACT_ID)
			hResult = cc->parentID;
		else if (hResult != cc->parentID) {
			return 0;
		}
	}
	return hResult;
}

void ImportMeta(DBCachedContact *ccSrc)
{
	if (!ccSrc->IsMeta() || !ccSrc->nSubs || !ccSrc->pSubs)
		return;

	// check first that the precise copy of metacontact exists
	DBCachedContact *ccDst = FindDestMeta(ccSrc);
	if (ccDst == NULL) {
		MCONTACT hDest = FindExistingMeta(ccSrc);
		if (hDest == 0) {
			AddMessage(LPGENT("Metacontact cannot be imported due to its ambiguity."));
			return;
		}

		ptrT tszGroup(myGetWs(ccSrc->contactID, "CList", "Group")), tszNick(myGetWs(ccSrc->contactID, "CList", "MyHandle"));
		if (tszNick == NULL)
			tszNick = myGetWs(ccSrc->contactID, ccSrc->szProto, "Nick");

		// do we need to add a new metacontact?
		if (hDest == INVALID_CONTACT_ID) {
			hDest = CallService(MS_DB_CONTACT_ADD, 0, 0);
			CallService(MS_PROTO_ADDTOCONTACT, hDest, LPARAM(META_PROTO));
			CopySettings(ccSrc->contactID, META_PROTO, hDest, META_PROTO);

			ccDst = dstDb->m_cache->GetCachedContact(hDest);
			if (ccDst == NULL) // normally it shouldn't happen
				return;

			// simply copy the whole metacontact structure
			ccDst->nDefault = ccSrc->nDefault;
			if (ccDst->nSubs = ccSrc->nSubs) {
				ccDst->pSubs = (MCONTACT*)mir_alloc(sizeof(MCONTACT)*ccSrc->nSubs);
				for (int i = 0; i < ccSrc->nSubs; i++) {
					ccDst->pSubs[i] = MapContact(ccSrc->pSubs[i]);

					char szSettingName[100];
					mir_snprintf(szSettingName, SIZEOF(szSettingName), "Handle%d", i);
					db_set_dw(hDest, META_PROTO, szSettingName, ccDst->pSubs[i]);

					db_set_b(ccDst->pSubs[i], META_PROTO, "IsSubcontact", 1);
					db_set_dw(ccDst->pSubs[i], META_PROTO, "ParentMeta", hDest);

					DBCachedContact *ccSub = dstDb->m_cache->GetCachedContact(ccDst->pSubs[i]);
					if (ccSub)
						ccSub->parentID = hDest;
				}
			}
		}
		else { // add missing subs
			ccDst = dstDb->m_cache->GetCachedContact(hDest);
			if (ccDst == NULL) // normally it shouldn't happen
				return;

			for (int i = 0; i < ccSrc->nSubs; i++) {
				MCONTACT hDstSub = MapContact(ccSrc->pSubs[i]);
				if (db_mc_getMeta(hDstSub) == NULL) // add a sub if needed
					CallService(MS_MC_ADDTOMETA, hDstSub, hDest);
			}
		}

		// ok, now transfer the common data
		CreateGroup(tszGroup, hDest);

		if (tszNick && *tszNick) {
			db_set_ws(hDest, "CList", "MyHandle", tszNick);
			AddMessage(LPGENT("Added metacontact '%s'"), tszNick);
		}
		else AddMessage(LPGENT("Added metacontact"));
	}

	PROTOACCOUNT *pa = ProtoGetAccount(META_PROTO);
	if (pa) {
		AccountMap pda(META_PROTO, 0, _T(META_PROTO));
		ImportContactSettings(&pda, ccSrc->contactID, ccDst->contactID);
	}

	arContactMap.insert(new ContactMap(ccSrc->contactID, ccDst->contactID));
}

/////////////////////////////////////////////////////////////////////////////////////////

static MCONTACT ImportContact(MCONTACT hSrc)
{
	// Check what protocol this contact belongs to
	DBCachedContact *cc = srcDb->m_cache->GetCachedContact(hSrc);
	if (cc == NULL || cc->szProto == NULL) {
		AddMessage(LPGENT("Skipping contact with no protocol"));
		return NULL;
	}

	if (cc->IsMeta()) {
		arMetas.insert(cc);
		return NULL;
	}

	AccountMap *pda = arAccountMap.find((AccountMap*)&cc->szProto);
	if (pda == NULL || pda->pa == NULL) {
		AddMessage(LPGENT("Skipping contact, account %S cannot be mapped."), cc->szProto);
		return NULL;
	}

	if (!ProtoGetAccount(pda->pa->szModuleName)) {
		AddMessage(LPGENT("Skipping contact, %S not installed."), cc->szProto);
		return NULL;
	}

	// Skip protocols with no unique id setting (some non IM protocols return NULL)
	char *pszUniqueSetting = (char*)CallProtoService(pda->pa->szModuleName, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
	if (!pszUniqueSetting || (INT_PTR)pszUniqueSetting == CALLSERVICE_NOTFOUND) {
		AddMessage(LPGENT("Skipping non-IM contact (%S)"), cc->szProto);
		return NULL;
	}

	DBVARIANT dbv;
	if (myGet(hSrc, cc->szProto, pszUniqueSetting, &dbv)) {
		AddMessage(LPGENT("Skipping %S contact, ID not found"), cc->szProto);
		return NULL;
	}

	// Does the contact already exist?
	TCHAR id[40], *pszUniqueID;
	MCONTACT hDst;
	switch (dbv.type) {
	case DBVT_DWORD:
		pszUniqueID = _ltot(dbv.dVal, id, 10);
		hDst = HContactFromNumericID(pda->pa->szModuleName, pszUniqueSetting, dbv.dVal);
		break;

	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		pszUniqueID = NEWTSTR_ALLOCA(_A2T(dbv.pszVal));
		hDst = HContactFromID(pda->pa->szModuleName, pszUniqueSetting, pszUniqueID);
		break;

	default:
		hDst = INVALID_CONTACT_ID;
		pszUniqueID = NULL;
	}

	if (hDst != INVALID_CONTACT_ID) {
		AddMessage(LPGENT("Skipping duplicate %S contact %s"), cc->szProto, pszUniqueID);
		srcDb->FreeVariant(&dbv);
		arContactMap.insert(new ContactMap(hSrc, hDst));
		return NULL;
	}

	ptrT tszGroup(myGetWs(hSrc, "CList", "Group")), tszNick(myGetWs(hSrc, "CList", "MyHandle"));
	if (tszNick == NULL)
		tszNick = myGetWs(hSrc, cc->szProto, "Nick");

	hDst = AddContact(hdlgProgress, pda->pa->szModuleName, pszUniqueSetting, &dbv, pszUniqueID, tszNick, tszGroup);
	if (hDst == INVALID_CONTACT_ID) {
		AddMessage(LPGENT("Unknown error while adding %S contact %s"), pda->pa->szModuleName, pszUniqueID);
		return INVALID_CONTACT_ID;
	}

	arContactMap.insert(new ContactMap(hSrc, hDst));
	ImportContactSettings(pda, hSrc, hDst);
	return hDst;
}

/////////////////////////////////////////////////////////////////////////////////////////
// copying system settings

static int CopySystemSettings(const char *szModuleName, DWORD, LPARAM param)
{
	LIST<char> *arSkippedAccs = (LIST<char>*)param;
	if (!arSkippedAccs->find((char*)szModuleName))
		CopySettings(NULL, szModuleName, NULL, szModuleName);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// contact's history import

static void ImportHistory(MCONTACT hContact, PROTOACCOUNT **protocol, int protoCount)
{
	MCONTACT hDst;

	// Is it contact's history import?
	if (hContact) {
		// we ignore history import for metacontacts
		// the metahistory will be generated automatically by gathering subs' histories
		DBCachedContact *cc = srcDb->m_cache->GetCachedContact(hContact);
		if (cc == NULL || cc->IsMeta())
			return;

		if ((hDst = MapContact(hContact)) == INVALID_CONTACT_ID) {
			nSkippedContacts++;
			return;
		}
	}
	else hDst = NULL;

	bool bSkipAll = false;
	DWORD cbAlloc = 4096;
	BYTE *eventBuf = (PBYTE)mir_alloc(cbAlloc);

	// Get the start of the event chain
	MEVENT hEvent = srcDb->FindFirstEvent(hContact);
	for (int i = 0; hEvent; i++, hEvent = srcDb->FindNextEvent(hContact, hEvent)) {
		// Copy the event and import it
		DBEVENTINFO dbei = { sizeof(DBEVENTINFO) };
		dbei.cbBlob = srcDb->GetBlobSize(hEvent);
		if (dbei.cbBlob > cbAlloc) {
			cbAlloc = dbei.cbBlob + 4096 - dbei.cbBlob % 4096;
			eventBuf = (PBYTE)mir_realloc(eventBuf, cbAlloc);
		}
		dbei.pBlob = eventBuf;

		bool bSkipThis = false;
		if (!srcDb->GetEvent(hEvent, &dbei)) {
			// check protocols during system history import
			if (hDst == NULL) {
				bSkipAll = true;
				for (int i = 0; i < protoCount; i++) {
					if (!strcmp(dbei.szModule, protocol[i]->szModuleName)) {
						bSkipAll = false;
						break;
					}
				}
				bSkipThis = bSkipAll;
			}

			// custom filtering
			if (!bSkipThis) {
				BOOL sent = (dbei.flags & DBEF_SENT);

				if (dbei.timestamp < (DWORD)dwSinceDate)
					bSkipThis = 1;

				if (!bSkipThis) {
					if (hDst) {
						bSkipThis = 1;
						switch (dbei.eventType) {
						case EVENTTYPE_MESSAGE:
							if ((sent ? IOPT_MSGSENT : IOPT_MSGRECV) & nImportOptions)
								bSkipThis = 0;
							break;
						case EVENTTYPE_FILE:
							if ((sent ? IOPT_FILESENT : IOPT_FILERECV) & nImportOptions)
								bSkipThis = 0;
							break;
						case EVENTTYPE_URL:
							if ((sent ? IOPT_URLSENT : IOPT_URLRECV) & nImportOptions)
								bSkipThis = 0;
							break;
						default:
							if ((sent ? IOPT_OTHERSENT : IOPT_OTHERRECV) & nImportOptions)
								bSkipThis = 0;
							break;
						}
					}
					else if (!(nImportOptions & IOPT_SYSTEM))
						bSkipThis = 1;
				}

				if (bSkipThis)
					nSkippedEvents++;
			}

			if (!bSkipThis) {
				// check for duplicate entries
				if (!IsDuplicateEvent(hDst, dbei)) {
					// no need to display all these dialogs again
					if (dbei.eventType == EVENTTYPE_AUTHREQUEST || dbei.eventType == EVENTTYPE_ADDED)
						dbei.flags |= DBEF_READ;

					// add dbevent
					if (dstDb->AddEvent(hDst, &dbei) != NULL)
						nMessagesCount++;
					else
						AddMessage(LPGENT("Failed to add message"));
				}
				else nDupes++;
			}
		}

		if (!(i % 10)) {
			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		// bSkipThis this chain if needed
		if (bSkipAll)
			break;
	}
	mir_free(eventBuf);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int CompareModules(const char *p1, const char *p2)
{	return stricmp(p1, p2);
}

void MirandaImport(HWND hdlg)
{
	hdlgProgress = hdlg;

	if ((dstDb = GetCurrentDatabase()) == NULL) {
		AddMessage(LPGENT("Error retrieving current profile, exiting."));
		return;
	}

	DATABASELINK *dblink = FindDatabasePlugin(importFile);
	if (dblink == NULL) {
		AddMessage(LPGENT("There's no database driver to open the input file, exiting."));
		return;
	}

	if ((srcDb = dblink->Load(importFile, TRUE)) == NULL) {
		AddMessage(LPGENT("Error loading source file, exiting."));
		return;
	}

	// Reset statistics
	nSkippedEvents = 0;
	nDupes = 0;
	nContactsCount = 0;
	nMessagesCount = 0;
	nGroupsCount = 0;
	nSkippedContacts = 0;
	SetProgress(0);

	// Get number of contacts
	int nNumberOfContacts = srcDb->GetContactCount();
	AddMessage(LPGENT("Number of contacts in database: %d"), nNumberOfContacts);
	AddMessage(_T(""));

	// Configure database for fast writing
	dstDb->SetCacheSafetyMode(FALSE);

	// Start benchmark timer
	DWORD dwTimer = time(NULL);

	OBJLIST<char> arSkippedAccs(1, CompareModules);
	if (!ImportAccounts(arSkippedAccs)) {
		AddMessage(LPGENT("Error mapping accounts, exiting."));
		return;
	}

	// copy system settings if needed
	if (nImportOptions & IOPT_SYS_SETTINGS)
		srcDb->EnumModuleNames(CopySystemSettings, &arSkippedAccs);
	arSkippedAccs.destroy();

	// Import Groups
	if (nImportOptions & IOPT_GROUPS) {
		AddMessage(LPGENT("Importing groups."));
		nGroupsCount = ImportGroups();
		if (nGroupsCount == -1)
			AddMessage(LPGENT("Group import failed."));

		AddMessage(_T(""));
	}
	// End of Import Groups

	// Import Contacts
	if (nImportOptions & IOPT_CONTACTS) {
		AddMessage(LPGENT("Importing contacts."));
		int i = 1;
		MCONTACT hContact = srcDb->FindFirstContact();
		while (hContact != NULL) {
			if (ImportContact(hContact))
				nContactsCount++;

			// Update progress bar
			SetProgress(100 * i / nNumberOfContacts);
			i++;

			// Process queued messages
			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			// Get next contact in chain
			hContact = srcDb->FindNextContact(hContact);
		}

		for (i = 0; i < arMetas.getCount(); i++)
			ImportMeta(arMetas[i]);
	}
	else AddMessage(LPGENT("Skipping new contacts import."));
	AddMessage(_T(""));
	// End of Import Contacts

	// Import NULL contact message chain
	if (nImportOptions & IOPT_SYSTEM) {
		AddMessage(LPGENT("Importing system history."));

		int protoCount;
		PROTOACCOUNT **accs;
		CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&protoCount, (LPARAM)&accs);

		if (protoCount > 0)
			ImportHistory(NULL, accs, protoCount);
	}
	else AddMessage(LPGENT("Skipping system history import."));
	AddMessage(_T(""));

	// Import other contact messages
	if (nImportOptions & IOPT_HISTORY) {
		AddMessage(LPGENT("Importing history."));
		MCONTACT hContact = srcDb->FindFirstContact();
		for (int i = 1; hContact != NULL; i++) {
			ImportHistory(hContact, NULL, NULL);

			SetProgress(100 * i / nNumberOfContacts);
			hContact = srcDb->FindNextContact(hContact);
		}
	}
	else AddMessage(LPGENT("Skipping history import."));
	AddMessage(_T(""));

	// Restore database writing mode
	dstDb->SetCacheSafetyMode(TRUE);

	// Clean up before exit
	dblink->Unload(srcDb);

	// Stop timer
	dwTimer = time(NULL) - dwTimer;

	// Print statistics
	AddMessage(LPGENT("Import completed in %d seconds."), dwTimer);
	SetProgress(100);
	AddMessage(nMessagesCount == 0 ?
		LPGENT("Added %d contacts and %d groups.") : LPGENT("Added %d contacts, %d groups and %d events."),
		nContactsCount, nGroupsCount, nMessagesCount);

	if (nSkippedContacts)
		AddMessage(LPGENT("Skipped %d contacts."), nSkippedContacts);

	if (nDupes || nSkippedEvents)
		AddMessage(LPGENT("Skipped %d duplicates and %d filtered events."), nDupes, nSkippedEvents);

	arMetas.destroy();
	arAccountMap.destroy();
	arContactMap.destroy();
}
