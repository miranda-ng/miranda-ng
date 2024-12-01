/*
Miranda NG SmileyAdd Plugin
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)
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

/////////////////////////////////////////////////////////////////////////////////////////
// Service functions

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
					
					auto *p = g_SmileyCategories.GetSmileyCategory(categoryName);
					if (!p || p->GetFilename().IsEmpty())
						categoryName = protonam;
				}
				else categoryName = protonam;
			}
		}
	}

	if (categoryName.IsEmpty()) {
		if (proto == nullptr || proto[0] == 0)
			categoryName = L"Standard";
		else {
			categoryName = proto;
			if (opt.UseOneForAll) {
				SmileyCategoryType *smc = g_SmileyCategories.GetSmileyCategory(categoryName);
				if (smc == nullptr || smc->IsProto())
					categoryName = L"Standard";
			}
		}
	}

	return g_SmileyCategories.GetSmileyPack(categoryName);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR ReplaceSmileysCommand(WPARAM, LPARAM lParam)
{
	SMADD_RICHEDIT *smre = (SMADD_RICHEDIT*)lParam;
	if (smre == nullptr)
		return FALSE;

	SMADD_RICHEDIT smrec = {};
	memcpy(&smrec, smre, sizeof(smrec));

	static const CHARRANGE selection = { 0, LONG_MAX };
	if (smre->rangeToReplace == nullptr)
		smrec.rangeToReplace = (CHARRANGE*)&selection;
	else if (smrec.rangeToReplace->cpMax < 0)
		smrec.rangeToReplace->cpMax = LONG_MAX;

	SmileyPackCType *smcp = nullptr;
	SmileyPackType *SmileyPack = FindSmileyPack(smrec.Protocolname, smrec.hContact, (smrec.flags & SAFLRE_NOCUSTOM) ? nullptr : &smcp);

	ReplaceSmileys(smre->hwndRichEditControl, SmileyPack, smcp, *smrec.rangeToReplace,
		smrec.hContact == 0, false, false, (smre->flags & SAFLRE_FIREVIEW) ? true : false);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR ParseTextBatch(WPARAM, LPARAM lParam)
{
	SMADD_BATCHPARSE *smre = (SMADD_BATCHPARSE*)lParam;
	if (smre == nullptr)
		return FALSE;

	SmileyPackCType *smcp = nullptr;
	SmileysQueueType smllist;
	SmileyPackType *SmileyPack = FindSmileyPack(smre->Protocolname, smre->hContact, (smre->flag & (SAFL_OUTGOING | SAFL_NOCUSTOM)) ? nullptr : &smcp);

	if (smre->flag & SAFL_UNICODE)
		LookupAllSmileys(SmileyPack, smcp, smre->str.w, smllist, false);
	else
		LookupAllSmileys(SmileyPack, smcp, _A2T(smre->str.a), smllist, false);

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

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR FreeTextBatch(WPARAM, LPARAM lParam)
{
	delete[](SMADD_BATCHPARSERES*)lParam;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR RegisterPack(WPARAM, LPARAM lParam)
{
	SMADD_REGCAT *smre = (SMADD_REGCAT*)lParam;
	if (smre == nullptr)
		return FALSE;

	if (IsBadStringPtrA(smre->name, 50) || IsBadStringPtrA(smre->dispname, 50))
		return FALSE;

	g_SmileyCategories.AddAndLoad(_A2T(smre->name), _A2T(smre->dispname));
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static LIST<void> menuHandleArray(5);

static int RebuildContactMenu(WPARAM wParam, LPARAM)
{
	auto &smc = g_SmileyCategories.GetSmileyCategoryList();

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
			mi.name.w = (wchar_t *)smc[i].GetDisplayName().c_str();

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
		NotifyEventHooks(g_hevOptionsChanged, hContact, 0);
	}

	for (auto &it : menuHandleArray)
		Menu_RemoveItem((HGENMENU)it);
	menuHandleArray.destroy();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR ReloadPack(WPARAM, LPARAM lParam)
{
	if (lParam) {
		SmileyCategoryType *smc = g_SmileyCategories.GetSmileyCategory(_A2T((char *)lParam));
		if (smc != nullptr)
			smc->Load();
	}
	else {
		g_SmileyCategories.ClearAll();
		g_SmileyCategories.AddAllProtocolsAsCategory();
		g_SmileyCategories.ClearAndLoadAll();
	}

	NotifyEventHooks(g_hevOptionsChanged, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR LoadContactSmileys(WPARAM, LPARAM lParam)
{
	SMADD_CONT *cont = (SMADD_CONT *)lParam;

	switch (cont->type) {
	case SMADD_SMILEPACK:
		g_SmileyPackCStore.AddSmileyPack(cont->pszModule, cont->path);
		NotifyEventHooks(g_hevOptionsChanged, 0, (WPARAM)cont->pszModule);
		break;

	case SMADD_FILE:
		if (g_SmileyPackCStore.AddSmiley(cont->pszModule, cont->path))
			NotifyEventHooks(g_hevOptionsChanged, 0, (WPARAM)cont->pszModule);
		break;

	case SMADD_FOLDER:
		auto *p = wcsrchr(cont->path, '\\');
		CMStringW wszPath(cont->path, (p == nullptr) ? lstrlen(cont->path) : p - cont->path + 1);

		for (auto &it : MFilePath(cont->path).search())
			if (mir_wstrcmp(it.getPath(), L".") && mir_wstrcmp(it.getPath(), L".."))
				g_SmileyPackCStore.AddSmiley(cont->pszModule, wszPath + it.getPath());
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR SelectSmiley(WPARAM, LPARAM lParam)
{
	auto *pParam = (SMADD_SELECTSMILEY *)lParam;
	if (pParam == nullptr)
		return 1;

	auto *sml = (pParam->pszProto) ? FindSmileyPack(pParam->pszProto) : g_pEmoji;
	if (sml == nullptr)
		return 2;

	SmileyPackType *pPack;
	if (pParam->pszSmileys) {
		ptrW pText(mir_utf8decodeW(pParam->pszSmileys));
		pPack = new SmileyPackType();
		auto &pList = sml->GetSmileyList();

		for (auto *p = wcstok(pText, L" "); p; p = wcstok(0, L" ")) {
			for (auto &it : pList) {
				if (it->GetTriggerText() == p) {
					pPack->GetSmileyList().insert(new SmileyType(*it));
					break;
				}
			}
		}
	}
	else pPack = new SmileyPackType(*sml);

	SmileyToolWindowParam *stwp = new SmileyToolWindowParam;
	stwp->pSmileyPack = pPack;
	stwp->bOwnsPack = true;
	stwp->hWndParent = pParam->hWndParent;
	stwp->hWndTarget = pParam->hWndTarget;
	stwp->targetMessage = pParam->targetMessage;
	stwp->targetWParam = pParam->targetWParam;
	stwp->direction = pParam->direction;
	stwp->xPosition = pParam->xPosition;
	stwp->yPosition = pParam->yPosition;
	mir_forkThread<SmileyToolWindowParam>(SmileyToolThread, stwp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

void InitServices()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, RebuildContactMenu);

	CreateServiceFunction(MS_SMILEYADD_REPLACESMILEYS, ReplaceSmileysCommand);
	CreateServiceFunction(MS_SMILEYADD_REGISTERCATEGORY, RegisterPack);
	CreateServiceFunction(MS_SMILEYADD_BATCHPARSE, ParseTextBatch);
	CreateServiceFunction(MS_SMILEYADD_BATCHFREE, FreeTextBatch);
	CreateServiceFunction(MS_SMILEYADD_CUSTOMCATMENU, CustomCatMenu);
	CreateServiceFunction(MS_SMILEYADD_RELOAD, ReloadPack);
	CreateServiceFunction("SmileyAdd/LoadContactSmileys", LoadContactSmileys);
	CreateServiceFunction(MS_SMILEYADD_SELECTSMILEY, SelectSmiley);
}
