/*
Miranda NG SmileyAdd Plugin
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (C) 2005-11 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003-04 Rein-Peter de Boer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

LIST<void> menuHandleArray(5);

//implementation of service functions

SmileyPackType* FindSmileyPack(const char *proto, MCONTACT hContact, SmileyPackCType **smlc)
{
	hContact = DecodeMetaContact(hContact);
	if (smlc)
		*smlc = g_SmileyPackCStore.GetSmileyPack(proto);

	if (proto != nullptr && IsBadStringPtrA(proto, 10))
		return nullptr;

	CMStringW categoryName;
	if (hContact != 0) {
		opt.ReadContactCategory(hContact, categoryName);
		if (categoryName == L"<None>") return nullptr;
		if (!categoryName.IsEmpty() && g_SmileyCategories.GetSmileyCategory(categoryName) == nullptr) {
			categoryName.Empty();
			opt.WriteContactCategory(hContact, categoryName);
		}

		if (categoryName.IsEmpty() && !opt.UseOneForAll) {
			char *protonam = Proto_GetBaseAccountName(hContact);
			if (protonam != nullptr) {
				DBVARIANT dbv;
				if (db_get_ws(hContact, protonam, "Transport", &dbv) == 0) {
					categoryName = dbv.pwszVal;
					db_free(&dbv);
				}
				else if (opt.UsePhysProto && db_get_ws(0, protonam, "AM_BaseProto", &dbv) == 0) {
					categoryName = L"AllProto";
					categoryName += dbv.pwszVal;
					db_free(&dbv);
					CMStringW categoryFileName = g_SmileyCategories.GetSmileyCategory(categoryName) ? g_SmileyCategories.GetSmileyCategory(categoryName)->GetFilename() : L"";
					if (categoryFileName.IsEmpty())
						categoryName = _A2T(protonam);
				}
				else
					categoryName = _A2T(protonam);
			}
		}
	}

	if (categoryName.IsEmpty()) {
		if (proto == nullptr || proto[0] == 0)
			categoryName = L"Standard";
		else {
			categoryName = _A2T(proto);
			if (opt.UseOneForAll) {
				SmileyCategoryType *smc = g_SmileyCategories.GetSmileyCategory(categoryName);
				if (smc == nullptr || smc->IsProto())
					categoryName = L"Standard";
			}
		}
	}

	return g_SmileyCategories.GetSmileyPack(categoryName);
}

INT_PTR ReplaceSmileysCommand(WPARAM, LPARAM lParam)
{
	SMADD_RICHEDIT3 *smre = (SMADD_RICHEDIT3*)lParam;
	if (smre == nullptr)
		return FALSE;

	SMADD_RICHEDIT3 smrec = {};
	memcpy(&smrec, smre, min(smre->cbSize, sizeof(smrec)));

	static const CHARRANGE selection = { 0, LONG_MAX };
	if (smre->rangeToReplace == nullptr) smrec.rangeToReplace = (CHARRANGE*)&selection;
	else if (smrec.rangeToReplace->cpMax < 0) smrec.rangeToReplace->cpMax = LONG_MAX;

	SmileyPackCType *smcp = nullptr;
	SmileyPackType *SmileyPack = FindSmileyPack(smrec.Protocolname, smrec.hContact, (smrec.flags & SAFLRE_NOCUSTOM) ? nullptr : &smcp);

	ReplaceSmileys(smre->hwndRichEditControl, SmileyPack, smcp, *smrec.rangeToReplace,
		smrec.hContact == 0, false, false, (smre->flags & SAFLRE_FIREVIEW) ? true : false);

	return TRUE;
}

static int GetInfoCommandE(SMADD_INFO2 *smre, bool retDup)
{
	if (smre == nullptr) return FALSE;

	SmileyPackType *SmileyPack = FindSmileyPack(smre->Protocolname);
	if (SmileyPack == nullptr || SmileyPack->SmileyCount() == 0) {
		smre->ButtonIcon = nullptr;
		smre->NumberOfSmileys = 0;
		smre->NumberOfVisibleSmileys = 0;
		return FALSE;
	}

	SmileyType *sml = FindButtonSmiley(SmileyPack);

	if (sml != nullptr)
		smre->ButtonIcon = retDup ? sml->GetIconDup() : sml->GetIcon();
	else
		smre->ButtonIcon = GetDefaultIcon(retDup);

	smre->NumberOfSmileys = SmileyPack->SmileyCount();
	smre->NumberOfVisibleSmileys = SmileyPack->VisibleSmileyCount();

	return TRUE;
}

INT_PTR GetInfoCommand(WPARAM, LPARAM lParam)
{
	return GetInfoCommandE((SMADD_INFO2*)lParam, false);
}

INT_PTR GetInfoCommand2(WPARAM, LPARAM lParam)
{
	return GetInfoCommandE((SMADD_INFO2*)lParam, true);
}



INT_PTR ParseTextBatch(WPARAM, LPARAM lParam)
{
	SMADD_BATCHPARSE2 *smre = (SMADD_BATCHPARSE2*)lParam;
	if (smre == nullptr)
		return FALSE;

	SmileyPackCType *smcp = nullptr;
	SmileysQueueType smllist;
	SmileyPackType *SmileyPack = FindSmileyPack(smre->Protocolname, smre->hContact, (smre->flag & (SAFL_OUTGOING | SAFL_NOCUSTOM)) ? nullptr : &smcp);

	if (smre->flag & SAFL_UNICODE)
		LookupAllSmileys(SmileyPack, smcp, smre->wstr, smllist, false);
	else
		LookupAllSmileys(SmileyPack, smcp, _A2T(smre->astr), smllist, false);

	if (smllist.getCount() == 0)
		return 0;

	SMADD_BATCHPARSERES *res = new SMADD_BATCHPARSERES[smllist.getCount()];
	SMADD_BATCHPARSERES *cres = res;
	for (auto &it : smllist) {
		cres->startChar = it->loc.cpMin;
		cres->size = it->loc.cpMax - it->loc.cpMin;
		if (it->sml) {
			if (smre->flag & SAFL_PATH)
				cres->filepath = it->sml->GetFilePath().c_str();
			else
				cres->hIcon = it->sml->GetIconDup();
		}
		else {
			if (smre->flag & SAFL_PATH)
				cres->filepath = it->smlc->GetFilePath().c_str();
			else
				cres->hIcon = it->smlc->GetIcon();
		}

		cres++;
	}

	smre->numSmileys = smllist.getCount();

	smre->oflag = smre->flag | SAFL_UNICODE;

	return (INT_PTR)res;
}

INT_PTR FreeTextBatch(WPARAM, LPARAM lParam)
{
	delete[](SMADD_BATCHPARSERES*)lParam;
	return TRUE;
}

INT_PTR RegisterPack(WPARAM, LPARAM lParam)
{
	SMADD_REGCAT *smre = (SMADD_REGCAT*)lParam;

	if (smre == nullptr || smre->cbSize < sizeof(SMADD_REGCAT)) return FALSE;
	if (IsBadStringPtrA(smre->name, 50) || IsBadStringPtrA(smre->dispname, 50)) return FALSE;


	CMStringW nmd(_A2T(smre->dispname));
	CMStringW nm(_A2T(smre->name));
	g_SmileyCategories.AddAndLoad(nm, nmd);

	return TRUE;
}

INT_PTR CustomCatMenu(WPARAM hContact, LPARAM lParam)
{
	if (lParam != 0) {
		SmileyCategoryType *smct = g_SmileyCategories.GetSmileyCategory((unsigned)lParam - 3);
		if (smct != nullptr)
			opt.WriteContactCategory(hContact, smct->GetName());
		else {
			CMStringW empty;
			if (lParam == 1) empty = L"<None>";
			opt.WriteContactCategory(hContact, empty);
		}
		NotifyEventHooks(hEvent1, hContact, 0);
	}

	for (auto &it : menuHandleArray)
		Menu_RemoveItem((HGENMENU)it);
	menuHandleArray.destroy();

	return TRUE;
}


int RebuildContactMenu(WPARAM wParam, LPARAM)
{
	auto &smc = *g_SmileyCategories.GetSmileyCategoryList();

	char *protnam = Proto_GetBaseAccountName(wParam);
	bool haveMenu = IsSmileyProto(protnam);
	if (haveMenu && opt.UseOneForAll) {
		unsigned cnt = 0;
		for (auto &it : smc)
			cnt += it->IsCustom();
		haveMenu = cnt != 0;
	}

	Menu_ShowItem(hContactMenuItem, haveMenu);

	for (auto &it : menuHandleArray)
		Menu_RemoveItem((HGENMENU)it);
	menuHandleArray.destroy();

	if (haveMenu) {
		CMStringW cat;
		opt.ReadContactCategory(wParam, cat);

		CMenuItem mi(&g_plugin);
		mi.root = hContactMenuItem;
		mi.flags = CMIF_UNICODE | CMIF_SYSTEM;
		mi.pszService = MS_SMILEYADD_CUSTOMCATMENU;

		bool nonecheck = true;
		HGENMENU hMenu;

		for (int i = 0; i < smc.getCount(); i++) {
			if (smc[i].IsExt() || (smc[i].IsProto() && opt.UseOneForAll) || smc[i].GetFilename().IsEmpty())
				continue;

			const int ind = i + 3;
			mi.position = ind;
			mi.name.w = (wchar_t*)smc[i].GetDisplayName().c_str();

			if (cat == smc[i].GetName()) {
				mi.flags |= CMIF_CHECKED;
				nonecheck = false;
			}

			hMenu = Menu_AddContactMenuItem(&mi);
			Menu_ConfigureItem(hMenu, MCI_OPT_EXECPARAM, ind);
			menuHandleArray.insert(hMenu);
			mi.flags &= ~CMIF_CHECKED;
		}

		mi.position = 1;
		mi.name.w = L"<None>";
		if (cat == L"<None>") {
			mi.flags |= CMIF_CHECKED;
			nonecheck = false;
		}

		hMenu = Menu_AddContactMenuItem(&mi);
		Menu_ConfigureItem(hMenu, MCI_OPT_EXECPARAM, 1);
		menuHandleArray.insert(hMenu);

		mi.position = 2;
		mi.name.w = LPGENW("Protocol specific");
		if (nonecheck) mi.flags |= CMIF_CHECKED; else mi.flags &= ~CMIF_CHECKED;

		hMenu = Menu_AddContactMenuItem(&mi);
		Menu_ConfigureItem(hMenu, MCI_OPT_EXECPARAM, 2);
		menuHandleArray.insert(hMenu);
	}

	return 0;
}

INT_PTR ReloadPack(WPARAM, LPARAM lParam)
{
	if (lParam) {
		CMStringW categoryName = _A2T((char*)lParam);
		SmileyCategoryType *smc = g_SmileyCategories.GetSmileyCategory(categoryName);
		if (smc != nullptr)
			smc->Load();
	}
	else {
		g_SmileyCategories.ClearAll();
		g_SmileyCategories.AddAllProtocolsAsCategory();
		g_SmileyCategories.ClearAndLoadAll();
	}

	NotifyEventHooks(hEvent1, 0, 0);
	return 0;
}

INT_PTR LoadContactSmileys(WPARAM, LPARAM lParam)
{
	SMADD_CONT *cont = (SMADD_CONT*)lParam;

	switch (cont->type) {
	case 0:
		g_SmileyPackCStore.AddSmileyPack(cont->pszModule, cont->path);
		NotifyEventHooks(hEvent1, (WPARAM)cont->pszModule, 0);
		break;

	case 1:
		g_SmileyPackCStore.AddSmiley(cont->pszModule, cont->path);
		NotifyEventHooks(hEvent1, (WPARAM)cont->pszModule, 0);
		break;

	case 2:
		WIN32_FIND_DATAW findData;
		CMStringW wszPath(cont->path);
		HANDLE hFind = FindFirstFileW(wszPath, &findData);
		if (hFind != INVALID_HANDLE_VALUE) {
			int idx = wszPath.ReverseFind('\\');
			if (idx != -1)
				wszPath.Truncate(idx+1);

			do {
				CMStringW wszFileName = wszPath + findData.cFileName;
				g_SmileyPackCStore.AddSmiley(cont->pszModule, wszFileName);
			} while (FindNextFileW(hFind, &findData));
		}
	}
	return 0;
}

int AccountListChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *acc = (PROTOACCOUNT*)lParam;

	switch (wParam) {
	case PRAC_ADDED:
		if (acc != nullptr) {
			CMStringW catname(L"Standard");
			const CMStringW &defaultFile = g_SmileyCategories.GetSmileyCategory(catname)->GetFilename();
			g_SmileyCategories.AddAccountAsCategory(acc, defaultFile);
		}
		break;

	case PRAC_CHANGED:
		if (acc != nullptr && acc->szModuleName != nullptr) {
			CMStringW name(_A2T(acc->szModuleName));
			SmileyCategoryType *smc = g_SmileyCategories.GetSmileyCategory(name);
			if (smc != nullptr) {
				if (acc->tszAccountName) name = acc->tszAccountName;
				smc->SetDisplayName(name);
			}
		}
		break;

	case PRAC_REMOVED:
		g_SmileyCategories.DeleteAccountAsCategory(acc);
		break;

	case PRAC_CHECKED:
		if (acc != nullptr) {
			if (acc->bIsEnabled) {
				CMStringW catname(L"Standard");
				const CMStringW &defaultFile = g_SmileyCategories.GetSmileyCategory(catname)->GetFilename();
				g_SmileyCategories.AddAccountAsCategory(acc, defaultFile);
			}
			else g_SmileyCategories.DeleteAccountAsCategory(acc);
		}
		break;
	}
	return 0;
}

int DbSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (cws->value.type == DBVT_DELETED)
		return 0;

	if (strcmp(cws->szSetting, "Transport") == 0) {
		CMStringW catname(L"Standard");
		SmileyCategoryType *smc = g_SmileyCategories.GetSmileyCategory(catname);
		if (smc != nullptr)
			g_SmileyCategories.AddContactTransportAsCategory(hContact, smc->GetFilename());
	}
	return 0;
}


int ReloadColour(WPARAM, LPARAM)
{
	opt.SelWndBkgClr = db_get_dw(0, "SmileyAdd", "SelWndBkgClr", GetSysColor(COLOR_WINDOW));
	return 0;
}
