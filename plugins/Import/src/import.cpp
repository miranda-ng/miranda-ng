/*

Import plugin for Miranda NG

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

static bool CompareDb(DBVARIANT &dbv1, DBVARIANT &dbv2)
{
	if (dbv1.type == dbv2.type) {
		switch (dbv1.type) {
		case DBVT_DWORD:
			return dbv1.dVal == dbv2.dVal;

		case DBVT_ASCIIZ:
		case DBVT_UTF8:
			return 0 == mir_strcmp(dbv1.pszVal, dbv2.pszVal);
		}
	}
	return false;
}

static int CompareAccs(const AccountMap *p1, const AccountMap *p2)
{
	return mir_strcmpi(p1->szSrcAcc, p2->szSrcAcc);
}

static int CompareAccByIds(const AccountMap *p1, const AccountMap *p2)
{
	return p1->iOrder - p2->iOrder;
}

/////////////////////////////////////////////////////////////////////////////////////////

CImportBatch::CImportBatch() :
	m_accounts(5, CompareAccs),
	m_contacts(50, NumericKeySortT),
	m_metas(10)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

int CImportBatch::myGet(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	dbv->type = 0;
	return srcDb->GetContactSetting(hContact, szModule, szSetting, dbv);
}

int CImportBatch::myGetD(MCONTACT hContact, const char *szModule, const char *szSetting, int iDefault)
{
	DBVARIANT dbv = { DBVT_DWORD };
	return srcDb->GetContactSetting(hContact, szModule, szSetting, &dbv) ? iDefault : dbv.dVal;
}

BOOL CImportBatch::myGetS(MCONTACT hContact, const char *szModule, const char *szSetting, char *dest)
{
	DBVARIANT dbv = { DBVT_ASCIIZ };
	dbv.pszVal = dest; dbv.cchVal = 100;
	return srcDb->GetContactSettingStatic(hContact, szModule, szSetting, &dbv);
}

wchar_t* CImportBatch::myGetWs(MCONTACT hContact, const char *szModule, const char *szSetting)
{
	DBVARIANT dbv = { DBVT_WCHAR };
	return srcDb->GetContactSettingStr(hContact, szModule, szSetting, &dbv) ? nullptr : dbv.pwszVal;
}

/////////////////////////////////////////////////////////////////////////////////////////

MCONTACT CImportBatch::HContactFromID(const char *pszProtoName, const char *pszSetting, wchar_t *pwszID)
{
	for (MCONTACT hContact = dstDb->FindFirstContact(pszProtoName); hContact; hContact = dstDb->FindNextContact(hContact, pszProtoName)) {
		ptrW id(db_get_wsa(hContact, pszProtoName, pszSetting));
		if (!mir_wstrcmp(pwszID, id))
			return hContact;
	}
	return INVALID_CONTACT_ID;
}

MCONTACT CImportBatch::HContactFromChatID(const char *pszProtoName, const wchar_t *pszChatID)
{
	for (MCONTACT hContact = dstDb->FindFirstContact(pszProtoName); hContact; hContact = dstDb->FindNextContact(hContact, pszProtoName)) {
		if (!Contact::IsGroupChat(hContact, pszProtoName))
			continue;

		ptrW wszChatId(db_get_wsa(hContact, pszProtoName, "ChatRoomID"));
		if (!mir_wstrcmp(pszChatID, wszChatId))
			return hContact;
	}

	return INVALID_CONTACT_ID;
}

MCONTACT CImportBatch::HContactFromNumericID(const char *pszProtoName, const char *pszSetting, uint32_t dwID)
{
	for (MCONTACT hContact = dstDb->FindFirstContact(pszProtoName); hContact; hContact = dstDb->FindNextContact(hContact, pszProtoName))
		if (db_get_dw(hContact, pszProtoName, pszSetting, 0) == dwID)
			return hContact;

	return INVALID_CONTACT_ID;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int CopySettingsEnum(const char *szSetting, void *param)
{
	LIST<char> *pSettings = (LIST<char>*)param;
	pSettings->insert(mir_strdup(szSetting));
	return 0;
}

void CImportBatch::CopySettings(MCONTACT srcID, const char *szSrcModule, MCONTACT dstID, const char *szDstModule)
{
	LIST<char> arSettings(50);
	srcDb->EnumContactSettings(srcID, CopySettingsEnum, szSrcModule, &arSettings);

	for (auto &it : arSettings.rev_iter()) {
		DBVARIANT dbv = { 0 };
		if (!srcDb->GetContactSetting(srcID, szSrcModule, it, &dbv))
			db_set(dstID, szDstModule, it, &dbv);
		mir_free(it);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// accounts matcher dialog

static HWND hwndList, hwndCombo;
static int iPrevIndex = -1;

static void SetAccountName(int idx, PROTOACCOUNT *pa)
{
	ListView_SetItemText(hwndList, idx, 1, (pa == nullptr) ? TranslateT("<New account>") : pa->tszAccountName);
}

static LRESULT CALLBACK ComboWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KILLFOCUS && LPARAM(hwnd) == lParam) {
		if (iPrevIndex != -1) {
			AccountMap *pMap = (AccountMap*)SendMessage(hwnd, CB_GETITEMDATA, 0, 0);

			int idx = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
			if (idx == 0)
				pMap->pa = nullptr;
			else
				pMap->pa = (PROTOACCOUNT*)SendMessage(hwnd, CB_GETITEMDATA, idx, 0);

			SetAccountName(iPrevIndex, pMap->pa);
			iPrevIndex = -1;
		}

		DestroyWindow(hwnd);
		hwndCombo = nullptr;
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

		wchar_t tszText[100];
		ListView_GetItemText(hwnd, hit.iItem, 1, tszText, _countof(tszText));

		LVITEM lvitem;
		lvitem.iItem = hit.iItem;
		lvitem.iSubItem = 0;
		lvitem.mask = LVIF_PARAM;
		ListView_GetItem(hwnd, &lvitem);

		if (hwndCombo != nullptr)
			SendMessage(hwndCombo, WM_KILLFOCUS, 0, (LPARAM)hwndCombo);

		hwndCombo = CreateWindowEx(WS_EX_CLIENTEDGE, WC_COMBOBOX, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
			r.left + 3, r.top, r.right - r.left - 3, r.bottom - r.top, hwnd, nullptr, g_plugin.getInst(), nullptr);

		// copy a font from listview
		HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
		SendMessage(hwndCombo, WM_SETFONT, (WPARAM)hFont, 0);

		SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("<New account>"));
		SendMessage(hwndCombo, CB_SETITEMDATA, 0, lvitem.lParam);

		int iSel = 0;
		for (auto &it : Accounts()) {
			int idx = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)it->tszAccountName);
			SendMessage(hwndCombo, CB_SETITEMDATA, idx, (LPARAM)it);

			if (!mir_wstrcmpi(it->tszAccountName, tszText))
				iSel = idx;
		}

		SendMessage(hwndCombo, CB_SETCURSEL, iSel, 0);

		SetFocus(hwndCombo);
		mir_subclassWindow(hwndCombo, ComboWndProc);

		iPrevIndex = hit.iItem;
	}

	return mir_callNextSubclass(hwnd, ListWndProc, uMsg, wParam, lParam);
}

static INT_PTR CALLBACK AccountsMatcherProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		g_hwndAccMerge = hwndDlg;
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

			auto &accs = g_pBatch->AccountsMap();

			LVITEM lvi = { 0 };
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			for (int i = 0; i < accs.getCount(); i++) {
				AccountMap &p = accs[i];
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
			if (hwndCombo != nullptr)
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
		g_hwndAccMerge = nullptr;
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static char* newStr(const char *s)
{
	return (s == nullptr) ? nullptr : mir_strcpy(new char[mir_strlen(s) + 1], s);
}

bool CImportBatch::FindDestAccount(const char *szProto)
{
	for (auto &pam : m_accounts) {
		if (pam->pa == nullptr)
			continue;

		if (!mir_strcmp(pam->pa->szModuleName, szProto))
			return true;
	}

	return false;
}

PROTOACCOUNT* CImportBatch::FindMyAccount(const char *szProto, const char *szBaseProto, const wchar_t *ptszName, bool bStrict)
{
	PROTOACCOUNT *pProto = nullptr;
	for (auto &pa : Accounts()) {
		// already used? skip
		if (FindDestAccount(pa->szModuleName))
			continue;

		// different base protocotol type -> skip
		if (mir_strcmp(pa->szProtoName, szBaseProto))
			continue;

		// these protocols have no accounts, and their name match -> success
		if (pa->bOldProto || pa->bIsVirtual || pa->bDynDisabled)
			return pa;

		if (ptszName && !mir_wstrcmp(pa->tszAccountName, ptszName))
			return pa;

		const char *pszUniqueSetting = Proto_GetUniqueId(pa->szModuleName);
		if (!pszUniqueSetting) {
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
	return (bStrict) ? nullptr : pProto;
}

bool CImportBatch::ImportAccounts(OBJLIST<char> &arSkippedModules)
{
	bool bNeedManualMerge = false;
	if (m_pPattern == nullptr) {
		int protoCount = myGetD(NULL, "Protocols", "ProtoCount", 0);

		for (int i = 0; i < protoCount; i++) {
			char szSetting[100], szProto[100];
			itoa(i, szSetting, 10);
			if (myGetS(NULL, "Protocols", szSetting, szProto))
				continue;

			itoa(800 + i, szSetting, 10);
			ptrW tszName(myGetWs(NULL, "Protocols", szSetting));
			AccountMap* pNew = new AccountMap(szProto, i, tszName);
			m_accounts.insert(pNew);

			itoa(200 + i, szSetting, 10);
			pNew->iOrder = myGetD(NULL, "Protocols", szSetting, 0);

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
	}
	else {
		AccountMap *pNew = new AccountMap("Pattern", 0, m_pPattern->wszName);
		if (m_hContact)
			pNew->pa = Proto_GetAccount(Proto_GetBaseAccountName(m_hContact));
		m_accounts.insert(pNew);

		bNeedManualMerge = pNew->pa == nullptr;
	}

	// all accounts to be converted automatically, no need to raise a dialog
	if (bNeedManualMerge)
		if (DialogBox(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ACCMERGE), nullptr, AccountsMatcherProc) != IDOK)
			return false;

	bool bImportSysAll = (m_iOptions & IOPT_SYS_SETTINGS) != 0;

	LIST<AccountMap> arIndexedMap(m_accounts.getCount(), CompareAccByIds);
	for (auto &it : m_accounts)
		arIndexedMap.insert(it);

	for (auto &p : arIndexedMap) {
		if (p->szBaseProto == NULL || !mir_strcmp(p->szSrcAcc, META_PROTO))
			continue;

		if (!Proto_IsProtocolLoaded(p->szBaseProto)) {
			AddMessage(LPGENW("Protocol %S is not loaded, skipping account %s creation"), p->szBaseProto, p->tszSrcName);
			continue;
		}

		if (p->pa == nullptr) {
			char *szNewInternalName = nullptr; // create a new internal name by default
			if (!Proto_GetAccount(p->szSrcAcc))
				szNewInternalName = p->szSrcAcc; // but if the original internal name is available, keep it

			CopySettings(NULL, p->szSrcAcc, NULL, szNewInternalName);

			p->pa = Proto_CreateAccount(szNewInternalName, p->szBaseProto, p->tszSrcName);
			if (p->pa == nullptr) {
				AddMessage(LPGENW("Unable to create an account %s of protocol %S"), p->tszSrcName, p->szBaseProto);
				continue;
			}
		}
		else CopySettings(NULL, p->szSrcAcc, NULL, p->pa->szModuleName);

		char szSetting[100];
		itoa(400 + p->iSrcIndex, szSetting, 10);
		int iVal = myGetD(NULL, "Protocols", szSetting, 1);
		itoa(400 + p->pa->iOrder, szSetting, 10);
		db_set_dw(0, "Protocols", szSetting, iVal);
		p->pa->bIsVisible = iVal != 0;

		itoa(600 + p->iSrcIndex, szSetting, 10);
		iVal = myGetD(NULL, "Protocols", szSetting, 1);
		itoa(600 + p->pa->iOrder, szSetting, 10);
		db_set_dw(0, "Protocols", szSetting, iVal);
		p->pa->bIsEnabled = iVal != 0;

		if (p->tszSrcName == NULL) {
			p->pa->tszAccountName = mir_a2u(p->pa->szModuleName);
			itoa(800 + p->pa->iOrder, szSetting, 10);
			db_set_ws(0, "Protocols", szSetting, p->pa->tszAccountName);
		}

		if (bImportSysAll)
			arSkippedModules.insert(newStr(p->szSrcAcc));
	}

	CopySettings(NULL, META_PROTO, NULL, META_PROTO);
	if (bImportSysAll) {
		arSkippedModules.insert(newStr(META_PROTO));
		arSkippedModules.insert(newStr("Protocols"));
		arSkippedModules.insert(newStr("CryptoEngine"));
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

MCONTACT CImportBatch::MapContact(MCONTACT hSrc)
{
	if (m_hContact != 0 && hSrc == 1)
		return m_hContact;

	ContactMap *pDestContact = m_contacts.find((ContactMap*)&hSrc);
	return (pDestContact == nullptr) ? INVALID_CONTACT_ID : pDestContact->dstID;
}

struct ImportContactData
{
	MCONTACT from, to;
	const char *szSrcProto, *szDstProto;
	bool bSkipProto;
};

int ModulesEnumProc(const char *szModuleName, void *pParam)
{
	if (!mir_strcmp(szModuleName, "Protocol"))
		return 0;

	ImportContactData *icd = (ImportContactData*)pParam;
	if (!mir_strcmp(icd->szSrcProto, szModuleName)) {
		if (!icd->bSkipProto)
			g_pBatch->CopySettings(icd->from, szModuleName, icd->to, icd->szDstProto);
	}
	else g_pBatch->CopySettings(icd->from, szModuleName, icd->to, szModuleName);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct MImportGroup
{
	MImportGroup(int _n, wchar_t *_nm) :
		wszName(_nm),
		iNumber(_n)
	{}

	int iNumber;
	ptrW wszName;
};

static int ImportGroup(const char* szSettingName, void *param)
{
	OBJLIST<MImportGroup> *pArray = (OBJLIST<MImportGroup>*)param;
	wchar_t *wszGroupName = g_pBatch->myGetWs(NULL, "CListGroups", szSettingName);
	if (wszGroupName != nullptr)
		pArray->insert(new MImportGroup(atoi(szSettingName), wszGroupName));
	return 0;
}

int CImportBatch::ImportGroups()
{
	OBJLIST<MImportGroup> arGroups(10, NumericKeySortT);
	srcDb->EnumContactSettings(NULL, ImportGroup, "CListGroups", &arGroups);

	for (auto &it : arGroups) {
		MGROUP group_id = Clist_GroupCreate(0, it->wszName.get() + 1);
		if (group_id <= 0)
			continue;

		Clist_GroupSetExpanded(group_id, (it->wszName[0] & GROUPF_EXPANDED));
	}
	return arGroups.getCount();
}

/////////////////////////////////////////////////////////////////////////////////////////

DBCachedContact* CImportBatch::FindDestMeta(DBCachedContact *ccSrc)
{
	for (MCONTACT hMeta = dstDb->FindFirstContact(META_PROTO); hMeta != 0; hMeta = dstDb->FindNextContact(hMeta, META_PROTO)) {
		DBCachedContact *cc = dstDb->getCache()->GetCachedContact(hMeta);
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

	return nullptr;
}

MCONTACT CImportBatch::FindExistingMeta(DBCachedContact *ccSrc)
{
	MCONTACT hResult = INVALID_CONTACT_ID;

	// subs of source meta must belong to the only dest meta
	for (int i = 0; i < ccSrc->nSubs; i++) {
		MCONTACT hDestSub = MapContact(ccSrc->pSubs[i]);
		if (hDestSub == INVALID_CONTACT_ID)
			continue;

		DBCachedContact *cc = dstDb->getCache()->GetCachedContact(hDestSub);
		if (cc == nullptr || !cc->IsSub()) // check if it's a sub
			continue;

		if (hResult == INVALID_CONTACT_ID)
			hResult = cc->parentID;
		else if (hResult != cc->parentID) {
			return 0;
		}
	}
	return hResult;
}

void CImportBatch::ImportMeta(DBCachedContact *ccSrc)
{
	if (!ccSrc->IsMeta() || !ccSrc->nSubs || !ccSrc->pSubs)
		return;

	// check first that the precise copy of metacontact exists
	DBCachedContact *ccDst = FindDestMeta(ccSrc);
	if (ccDst == nullptr) {
		MCONTACT hDest = FindExistingMeta(ccSrc);
		if (hDest == 0) {
			AddMessage(LPGENW("Metacontact cannot be imported due to its ambiguity."));
			return;
		}

		ptrW tszGroup(myGetWs(ccSrc->contactID, "CList", "Group")), tszNick(myGetWs(ccSrc->contactID, "CList", "MyHandle"));
		if (tszNick == NULL)
			tszNick = myGetWs(ccSrc->contactID, ccSrc->szProto, "Nick");

		// do we need to add a new metacontact?
		if (hDest == INVALID_CONTACT_ID) {
			hDest = db_add_contact();
			db_set_s(hDest, "Protocol", "p", META_PROTO);
			CopySettings(ccSrc->contactID, META_PROTO, hDest, META_PROTO);

			ccDst = dstDb->getCache()->GetCachedContact(hDest);
			if (ccDst == nullptr) // normally it shouldn't happen
				return;

			// simply copy the whole metacontact structure
			ccDst->nDefault = ccSrc->nDefault;
			if (ccDst->nSubs = ccSrc->nSubs) {
				ccDst->pSubs = (MCONTACT*)mir_alloc(sizeof(MCONTACT)*ccSrc->nSubs);
				for (int i = 0; i < ccSrc->nSubs; i++) {
					MCONTACT hSub = MapContact(ccSrc->pSubs[i]);
					if (hSub == INVALID_CONTACT_ID) {
						hSub = db_add_contact();

						DBCachedContact *ccSub = srcDb->getCache()->GetCachedContact(ccSrc->pSubs[i]);
						if (ccSub && ccSub->szProto) {
							Proto_AddToContact(hDest, ccSub->szProto);
							CopySettings(ccSrc->contactID, ccSub->szProto, hSub, ccSub->szProto);
						}
					}

					ccDst->pSubs[i] = hSub;

					char szSettingName[100];
					mir_snprintf(szSettingName, "Handle%d", i);
					db_set_dw(hDest, META_PROTO, szSettingName, hSub);

					db_set_b(hSub, META_PROTO, "IsSubcontact", 1);
					db_set_dw(hSub, META_PROTO, "ParentMeta", hDest);

					DBCachedContact *ccSub = dstDb->getCache()->GetCachedContact(hSub);
					if (ccSub)
						ccSub->parentID = hDest;
				}
			}
		}
		else { // add missing subs
			ccDst = dstDb->getCache()->GetCachedContact(hDest);
			if (ccDst == nullptr) // normally it shouldn't happen
				return;

			for (int i = 0; i < ccSrc->nSubs; i++) {
				MCONTACT hDstSub = MapContact(ccSrc->pSubs[i]);
				if (db_mc_getMeta(hDstSub) == NULL) // add a sub if needed
					db_mc_addToMeta(hDstSub, hDest);
			}
		}

		// ok, now transfer the common data
		CreateGroup(tszGroup, hDest);

		if (tszNick && *tszNick) {
			db_set_ws(hDest, "CList", "MyHandle", tszNick);
			AddMessage(LPGENW("Added metacontact '%s'"), tszNick.get());
		}
		else AddMessage(LPGENW("Added metacontact"));
	}

	ImportContactData icd = { ccSrc->contactID, ccDst->contactID, META_PROTO, META_PROTO, true };
	db_enum_modules(ModulesEnumProc, &icd);

	m_contacts.insert(new ContactMap(ccSrc->contactID, ccDst->contactID));
}

/////////////////////////////////////////////////////////////////////////////////////////

MCONTACT CImportBatch::ImportContact(MCONTACT hSrc)
{
	// Check what protocol this contact belongs to
	DBCachedContact *cc = srcDb->getCache()->GetCachedContact(hSrc);
	if (cc == nullptr || cc->szProto == nullptr) {
		AddMessage(LPGENW("Skipping contact with no protocol"));
		return 0;
	}

	if (cc->IsMeta()) {
		m_metas.insert(cc);
		return 0;
	}

	const char *szSrcModuleName, *szDstModuleName;
	{
		AccountMap *pda = m_accounts.find((AccountMap*)&cc->szProto);
		if (pda == nullptr || pda->pa == nullptr) {
			// it might be a virtual protocol not included into the general account map
			PROTOACCOUNT *pa = Proto_GetAccount(cc->szProto);
			if (pa == nullptr) {
				AddMessage(LPGENW("Skipping contact, account %S cannot be mapped."), cc->szProto);
				return 0;
			}

			// virtual protocols have no accounts and cannot change module
			szSrcModuleName = szDstModuleName = pa->szModuleName;
		}
		else {
			szSrcModuleName = pda->szSrcAcc;
			szDstModuleName = pda->pa->szModuleName;
		}
	}

	if (!Proto_GetAccount(szDstModuleName)) {
		AddMessage(LPGENW("Skipping contact, %S not installed."), cc->szProto);
		return 0;
	}

	// group chat?
	const char *pszUniqueSetting;
	bool bIsChat = myGetD(hSrc, cc->szProto, "ChatRoom", 0) != 0;
	if (bIsChat)
		pszUniqueSetting = "ChatRoomID";
	else {
		// Skip protocols with no unique id setting (some non IM protocols return 0)
		pszUniqueSetting = Proto_GetUniqueId(szDstModuleName);
		if (!pszUniqueSetting) {
			AddMessage(LPGENW("Skipping non-IM contact (%S)"), cc->szProto);
			return 0;
		}
	}

	wchar_t *pszUniqueID = L"<Unknown>";
	DBVARIANT dbv = {};
	if (!myGet(hSrc, cc->szProto, (m_pPattern != nullptr) ? "ID" : pszUniqueSetting, &dbv)) {
		// Does the contact already exist?
		MCONTACT hDst;
		wchar_t id[40];
		switch (dbv.type) {
		case DBVT_DWORD:
			pszUniqueID = _ltow(dbv.dVal, id, 10);
			hDst = HContactFromNumericID(szDstModuleName, pszUniqueSetting, dbv.dVal);
			break;

		case DBVT_ASCIIZ:
		case DBVT_UTF8:
			pszUniqueID = NEWWSTR_ALLOCA(_A2T(dbv.pszVal));
			if (bIsChat)
				hDst = HContactFromChatID(szDstModuleName, pszUniqueID);
			else
				hDst = HContactFromID(szDstModuleName, pszUniqueSetting, pszUniqueID);
			break;

		case DBVT_WCHAR:
			pszUniqueID = NEWWSTR_ALLOCA(dbv.pwszVal);
			if (bIsChat)
				hDst = HContactFromChatID(szDstModuleName, pszUniqueID);
			else
				hDst = HContactFromID(szDstModuleName, pszUniqueSetting, pszUniqueID);
			break;

		default:
			hDst = INVALID_CONTACT_ID;
		}

		if (hDst != INVALID_CONTACT_ID) {
			AddMessage(LPGENW("Skipping duplicate %S contact %s"), cc->szProto, pszUniqueID);
			srcDb->FreeVariant(&dbv);
			m_contacts.insert(new ContactMap(hSrc, hDst));
			return 0;
		}
	}

	ptrW tszGroup(myGetWs(hSrc, "CList", "Group")), tszNick(myGetWs(hSrc, "CList", "MyHandle"));
	if (tszNick == NULL)
		tszNick = myGetWs(hSrc, cc->szProto, "Nick");

	// adding missing contact
	MCONTACT hDst = db_add_contact();
	if (hDst == INVALID_CONTACT_ID) {
		nSkippedContacts++;
		AddMessage(LPGENW("Failed to create contact %s (%s)"), cc->szProto, pszUniqueID);
		return 0;
	}

	if (dbv.type == 0)
		srcDb->GetContactSetting(hSrc, cc->szProto, "ID", &dbv);

	if (dbv.type != 0)
		db_set(hDst, szDstModuleName, pszUniqueSetting, &dbv);

	if (Proto_AddToContact(hDst, szDstModuleName) != 0) {
		db_delete_contact(hDst);
		AddMessage(LPGENW("Failed to add %S contact %s"), szDstModuleName, pszUniqueID);
		return INVALID_CONTACT_ID;
	}

	CreateGroup(tszGroup, hDst);

	if (tszNick && *tszNick) {
		db_set_ws(hDst, "CList", "MyHandle", tszNick);
		AddMessage(LPGENW("Added %S contact %s, '%s'"), szDstModuleName, pszUniqueID, tszNick.get());
	}
	else AddMessage(LPGENW("Added %S contact %s"), szDstModuleName, pszUniqueID);

	srcDb->FreeVariant(&dbv);

	if (bIsChat)
		db_set_b(hDst, szDstModuleName, "ChatRoom", 1);

	m_contacts.insert(new ContactMap(hSrc, hDst));

	// also copy settings
	ImportContactData icd = { hSrc, hDst, szSrcModuleName, szDstModuleName, false };
	srcDb->EnumModuleNames(ModulesEnumProc, &icd);
	return hDst;
}

/////////////////////////////////////////////////////////////////////////////////////////
// copying system settings

static int CopySystemSettings(const char *szModuleName, void *param)
{
	LIST<char> *arSkippedAccs = (LIST<char>*)param;
	if (!arSkippedAccs->find((char*)szModuleName))
		g_pBatch->CopySettings(NULL, szModuleName, NULL, szModuleName);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// contact's history import

void CImportBatch::ImportHistory(MCONTACT hContact, PROTOACCOUNT **protocol, int protoCount)
{
	MCONTACT hDst;
	bool bIsMeta = false;
	bool bIsRelational = srcDb->IsRelational() != 0;
	char *szProto = nullptr;

	// Is it contact's history import?
	if (hContact) {
		// we ignore history import for metacontacts
		// the metahistory will be generated automatically by gathering subs' histories
		DBCachedContact *cc = srcDb->getCache()->GetCachedContact(hContact);
		if (cc == nullptr)
			return;

		// for k/v databases we read history for subs only
		if (bIsRelational) {
			if (cc->IsMeta())
				return;
		}
		// for mmap we read history for metas only
		else {
			if (cc->IsSub())
				return;

			bIsMeta = cc->IsMeta();
		}

		if ((hDst = MapContact(hContact)) == INVALID_CONTACT_ID) {
			nSkippedContacts++;
			return;
		}

		cc = dstDb->getCache()->GetCachedContact(hDst);
		if (cc != nullptr)
			szProto = cc->szProto;
	}
	else hDst = NULL;

	bool bSkipAll = false;
	uint32_t cbAlloc = 4096;
	uint8_t *eventBuf = (uint8_t*)mir_alloc(cbAlloc);

	// Get the start of the event chain
	int i = 0;
	DB::ECPTR pCursor(srcDb->EventCursor(hContact, 0));
	while (MEVENT hEvent = pCursor.FetchNext()) {
		i++;

		// Copy the event and import it
		DBEVENTINFO dbei = {};
		dbei.cbBlob = srcDb->GetBlobSize(hEvent);
		if (dbei.cbBlob > cbAlloc) {
			cbAlloc = dbei.cbBlob + 4096 - dbei.cbBlob % 4096;
			eventBuf = (uint8_t*)mir_realloc(eventBuf, cbAlloc);
		}
		dbei.pBlob = eventBuf;

		bool bSkipThis = false;
		if (!srcDb->GetEvent(hEvent, &dbei)) {
			if (dbei.szModule == nullptr)
				dbei.szModule = szProto;

			// check protocols during system history import
			if (hDst == NULL) {
				bSkipAll = true;
				for (int k = 0; k < protoCount; k++) {
					if (!mir_strcmp(dbei.szModule, protocol[k]->szModuleName)) {
						bSkipAll = false;
						break;
					}
				}
				bSkipThis = bSkipAll;
			}

			// custom filtering
			if (!bSkipThis) {
				bool bIsSent = (dbei.flags & DBEF_SENT) != 0;

				if (dbei.timestamp < (uint32_t)m_dwSinceDate)
					bSkipThis = true;

				if (!bSkipThis) {
					if (hDst) {
						bSkipThis = 1;
						switch (dbei.eventType) {
						case EVENTTYPE_MESSAGE:
							if ((bIsSent ? IOPT_MSGSENT : IOPT_MSGRECV) & m_iOptions)
								bSkipThis = false;
							break;
						case EVENTTYPE_FILE:
							if ((bIsSent ? IOPT_FILESENT : IOPT_FILERECV) & m_iOptions)
								bSkipThis = false;
							break;
						default:
							if ((bIsSent ? IOPT_OTHERSENT : IOPT_OTHERRECV) & m_iOptions)
								bSkipThis = false;
							break;
						}
					}
					else if (!(m_iOptions & IOPT_SYSTEM))
						bSkipThis = true;
				}

				if (bSkipThis)
					nSkippedEvents++;
			}

			if (bSkipThis)
				continue;

			// check for duplicate entries
			if ((m_iOptions & IOPT_CHECKDUPS) != 0 && DB::IsDuplicateEvent(hDst, dbei)) {
				nDupes++;
				continue;
			}

			// no need to display all these dialogs again
			if (dbei.eventType == EVENTTYPE_AUTHREQUEST || dbei.eventType == EVENTTYPE_ADDED)
				dbei.flags |= DBEF_READ;

			// add dbevent
			MCONTACT hOwner = (bIsMeta) ? MapContact(srcDb->GetEventContact(hEvent)) : hDst;
			if (hOwner != INVALID_CONTACT_ID) {
				// add dbevent 
				if (dstDb->AddEvent(hOwner, &dbei) != NULL)
					nMessagesCount++;
				else
					AddMessage(LPGENW("Failed to add message"));
			}
		}

		if (!(i % 10)) {
			MSG msg;
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
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
{
	return mir_strcmpi(p1, p2);
}

void CImportBatch::DoImport()
{
	if ((dstDb = (MDatabaseCommon*)db_get_current()) == nullptr) {
		AddMessage(LPGENW("Error retrieving current profile, exiting."));
		return;
	}

	DATABASELINK *dblink;
	if (m_pPattern == nullptr) {
		dblink = FindDatabasePlugin(m_wszFileName);
		if (dblink == nullptr) {
			AddMessage(LPGENW("There's no database driver to open the input file, exiting."));
			return;
		}
	}
	else dblink = &g_patternDbLink;

	if ((srcDb = dblink->Load(m_wszFileName, TRUE)) == nullptr) {
		AddMessage(LPGENW("Error loading source file, exiting."));
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
	AddMessage(LPGENW("Number of contacts in database: %d"), nNumberOfContacts);
	AddMessage(L"");

	// Configure database for fast writing
	dstDb->SetCacheSafetyMode(FALSE);

	// Start benchmark timer
	uint32_t dwTimer = time(0);

	OBJLIST<char> arSkippedAccs(1, CompareModules);
	arSkippedAccs.insert(newStr("CListGroups"));
	if (!ImportAccounts(arSkippedAccs)) {
		AddMessage(LPGENW("Error mapping accounts, exiting."));
		return;
	}

	// copy system settings if needed
	if (m_iOptions & IOPT_SYS_SETTINGS)
		srcDb->EnumModuleNames(CopySystemSettings, &arSkippedAccs);
	arSkippedAccs.destroy();

	// Import Groups
	if (m_iOptions & IOPT_GROUPS) {
		AddMessage(LPGENW("Importing groups."));
		nGroupsCount = ImportGroups();
		if (nGroupsCount == -1)
			AddMessage(LPGENW("Group import failed."));

		AddMessage(L"");
	}
	dstDb->Flush();
	// End of Import Groups

	// Import Contacts
	if (m_iOptions & IOPT_CONTACTS) {
		AddMessage(LPGENW("Importing contacts."));
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
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			// Get next contact in chain
			hContact = srcDb->FindNextContact(hContact);
		}

		for (auto &it : m_metas)
			ImportMeta(it);
	}
	else AddMessage(LPGENW("Skipping new contacts import."));
	AddMessage(L"");
	dstDb->Flush();
	// End of Import Contacts

	// Import NULL contact message chain
	if (m_iOptions & IOPT_SYSTEM) {
		AddMessage(LPGENW("Importing system history."));

		int protoCount;
		PROTOACCOUNT **accs;
		Proto_EnumAccounts(&protoCount, &accs);

		if (protoCount > 0)
			ImportHistory(NULL, accs, protoCount);
	}
	else AddMessage(LPGENW("Skipping system history import."));
	AddMessage(L"");
	dstDb->Flush();

	// Import other contact messages
	if (m_iOptions & IOPT_HISTORY) {
		AddMessage(LPGENW("Importing history."));
		MCONTACT hContact = srcDb->FindFirstContact();
		for (int i = 1; hContact != NULL; i++) {
			ImportHistory(hContact, nullptr, NULL);
			dstDb->Flush();

			SetProgress(100 * i / nNumberOfContacts);
			hContact = srcDb->FindNextContact(hContact);
		}
	}
	else AddMessage(LPGENW("Skipping history import."));
	AddMessage(L"");
	dstDb->Flush();

	// Restore database writing mode
	dstDb->SetCacheSafetyMode(TRUE);
	db_setCurrent(dstDb);

	// Clean up before exit
	delete srcDb;

	// Stop timer
	dwTimer = time(0) - dwTimer;

	// Print statistics
	AddMessage(LPGENW("Import completed in %d seconds."), dwTimer);
	SetProgress(100);
	AddMessage(nMessagesCount == 0 ?
		LPGENW("Added %d contacts and %d groups.") : LPGENW("Added %d contacts, %d groups and %d events."),
		nContactsCount, nGroupsCount, nMessagesCount);

	if (nSkippedContacts)
		AddMessage(LPGENW("Skipped %d contacts."), nSkippedContacts);

	if (nDupes || nSkippedEvents)
		AddMessage(LPGENW("Skipped %d duplicates and %d filtered events."), nDupes, nSkippedEvents);
}
