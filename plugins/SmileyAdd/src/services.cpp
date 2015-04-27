/*
Miranda NG SmileyAdd Plugin
Copyright (C) 2012 - 2015 Miranda NG project (http://miranda-ng.org)
Copyright (C) 2005 - 2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003 - 2004 Rein-Peter de Boer

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

#include "general.h"

LIST<void> menuHandleArray(5);

//implementation of service functions

SmileyPackType* GetSmileyPack(const char* proto, MCONTACT hContact, SmileyPackCType** smlc)
{
	hContact = DecodeMetaContact(hContact);
	if (smlc)
		*smlc = opt.DisableCustom ? NULL : g_SmileyPackCStore.GetSmileyPack(hContact);

	if (proto != NULL && IsBadStringPtrA(proto, 10))
		return NULL;

	CMString categoryName;
	if (hContact != NULL) {
		opt.ReadContactCategory(hContact, categoryName);
		if (categoryName == _T("<None>")) return NULL;
		if (!categoryName.IsEmpty() && g_SmileyCategories.GetSmileyCategory(categoryName) == NULL) {
			categoryName.Empty();
			opt.WriteContactCategory(hContact, categoryName);
		}

		if (categoryName.IsEmpty() && !opt.UseOneForAll) {
			char *protonam = GetContactProto(hContact);
			if (protonam != NULL) {
				DBVARIANT dbv;
				if (db_get_ts(hContact, protonam, "Transport", &dbv) == 0) {
					categoryName = dbv.ptszVal;
					db_free(&dbv);
				}
				else if (opt.UsePhysProto && db_get_ts(NULL, protonam, "AM_BaseProto", &dbv) == 0){
					categoryName = _T("AllProto");
					categoryName += dbv.ptszVal;
					db_free(&dbv);					
					if (g_SmileyCategories.GetSmileyCategory(categoryName)->GetFilename().IsEmpty())
						categoryName = A2T_SM(protonam);
				}
				else 
					categoryName = A2T_SM(protonam);
			}
		}
	}

	if (categoryName.IsEmpty()) {
		if (proto == NULL || proto[0] == 0)
			categoryName = _T("Standard");
		else {
			categoryName = A2T_SM(proto);
			if (opt.UseOneForAll) {
				SmileyCategoryType *smc = g_SmileyCategories.GetSmileyCategory(categoryName);
				if (smc == NULL || smc->IsProto()) 
					categoryName = _T("Standard");
			}
		}
	}

	return g_SmileyCategories.GetSmileyPack(categoryName);
}


INT_PTR ReplaceSmileysCommand(WPARAM, LPARAM lParam)
{
	SMADD_RICHEDIT3* smre = (SMADD_RICHEDIT3*) lParam;
	if (smre == NULL)
		return FALSE;

	SMADD_RICHEDIT3 smrec = {0};
	memcpy(&smrec, smre, min(smre->cbSize, sizeof(smrec)));

	static const CHARRANGE selection = { 0, LONG_MAX };
	if (smre->rangeToReplace == NULL) smrec.rangeToReplace = (CHARRANGE*)&selection;
	else if (smrec.rangeToReplace->cpMax < 0) smrec.rangeToReplace->cpMax = LONG_MAX; 

	SmileyPackCType* smcp = NULL;
	SmileyPackType* SmileyPack = GetSmileyPack(smrec.Protocolname, smrec.hContact, 
		(smrec.flags & (SAFLRE_OUTGOING | SAFLRE_NOCUSTOM)) ? NULL : &smcp);

	ReplaceSmileys(smre->hwndRichEditControl, SmileyPack, smcp, *smrec.rangeToReplace, 
		smrec.hContact == NULL, false, false, (smre->flags & SAFLRE_FIREVIEW)?true:false);

	return TRUE;
}


INT_PTR ShowSmileySelectionCommand(WPARAM, LPARAM lParam)
{
	SMADD_SHOWSEL3* smaddInfo = (SMADD_SHOWSEL3*) lParam;

	if (smaddInfo == NULL) return FALSE;
	HWND parent = smaddInfo->hwndParent;
	MCONTACT hContact = smaddInfo->hContact;

	SmileyToolWindowParam *stwp = new SmileyToolWindowParam;
	stwp->pSmileyPack = GetSmileyPack(smaddInfo->Protocolname, hContact);
	stwp->hContact = hContact;

	stwp->hWndParent = parent;
	stwp->hWndTarget = smaddInfo->hwndTarget;
	stwp->targetMessage = smaddInfo->targetMessage;
	stwp->targetWParam = smaddInfo->targetWParam;
	stwp->xPosition = smaddInfo->xPosition;
	stwp->yPosition = smaddInfo->yPosition;
	stwp->direction = smaddInfo->Direction;

	mir_forkthread(SmileyToolThread, stwp);

	return TRUE;
}


static int GetInfoCommandE(SMADD_INFO2* smre, bool retDup)
{
	if (smre == NULL) return FALSE;
	MCONTACT hContact = smre->hContact;

	SmileyPackType* SmileyPack = GetSmileyPack(smre->Protocolname, hContact);

	if (SmileyPack == NULL || SmileyPack->SmileyCount() == 0)
	{
		smre->ButtonIcon = NULL;
		smre->NumberOfSmileys = 0;
		smre->NumberOfVisibleSmileys = 0;
		return FALSE;
	}

	SmileyType* sml = FindButtonSmiley(SmileyPack);

	if (sml != NULL)
		smre->ButtonIcon = retDup ? sml->GetIconDup() : sml->GetIcon();
	else
		smre->ButtonIcon = GetDefaultIcon(retDup);

	smre->NumberOfSmileys = SmileyPack->SmileyCount();
	smre->NumberOfVisibleSmileys = SmileyPack->VisibleSmileyCount();

	return TRUE;
}


INT_PTR GetInfoCommand(WPARAM, LPARAM lParam)
{
	return GetInfoCommandE((SMADD_INFO2*) lParam, false);
}


INT_PTR GetInfoCommand2(WPARAM, LPARAM lParam)
{
	return GetInfoCommandE((SMADD_INFO2*) lParam, true);
}



INT_PTR ParseTextBatch(WPARAM, LPARAM lParam)
{
	SMADD_BATCHPARSE2* smre = (SMADD_BATCHPARSE2*) lParam;

	if (smre == NULL) return FALSE;
	MCONTACT hContact = smre->hContact;

	SmileyPackCType* smcp = NULL;
	SmileyPackType* SmileyPack = GetSmileyPack(smre->Protocolname, hContact, 
		(smre->flag & (SAFL_OUTGOING | SAFL_NOCUSTOM)) ? NULL : &smcp);

	SmileysQueueType smllist;

	if (smre->flag & SAFL_UNICODE)
		LookupAllSmileys(SmileyPack, smcp, W2T_SM(smre->wstr), smllist, false);
	else
		LookupAllSmileys(SmileyPack, smcp, A2T_SM(smre->astr), smllist, false);

	if (smllist.getCount() == 0)
		return 0;

	SMADD_BATCHPARSERES *res = new SMADD_BATCHPARSERES[smllist.getCount()]; 
	SMADD_BATCHPARSERES* cres = res;
	for (int j=0; j < smllist.getCount(); j++) {
		cres->startChar = smllist[j].loc.cpMin;
		cres->size = smllist[j].loc.cpMax - smllist[j].loc.cpMin;
		if (smllist[j].sml) {
			if (smre->flag & SAFL_PATH)
				cres->filepath = smllist[j].sml->GetFilePath().c_str();
			else
				cres->hIcon = smllist[j].sml->GetIconDup();
		}
		else {
			if (smre->flag & SAFL_PATH)
				cres->filepath = smllist[j].smlc->GetFilePath().c_str();
			else
				cres->hIcon = smllist[j].smlc->GetIcon();
		}

		cres++;
	}

	smre->numSmileys = smllist.getCount();

	smre->oflag = smre->flag | SAFL_UNICODE;

	return (INT_PTR)res;
}

INT_PTR FreeTextBatch(WPARAM, LPARAM lParam)
{
	delete[] (SMADD_BATCHPARSERES*)lParam;
	return TRUE;
}

INT_PTR RegisterPack(WPARAM, LPARAM lParam)
{
	SMADD_REGCAT* smre = (SMADD_REGCAT*) lParam;

	if (smre == NULL || smre->cbSize < sizeof(SMADD_REGCAT)) return FALSE;
	if (IsBadStringPtrA(smre->name, 50) || IsBadStringPtrA(smre->dispname, 50)) return FALSE;


	unsigned lpcp = (unsigned)CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	if (lpcp == CALLSERVICE_NOTFOUND) lpcp = CP_ACP;



	CMString nmd(A2W_SM(smre->dispname, lpcp));


	CMString nm(A2T_SM(smre->name));
	g_SmileyCategories.AddAndLoad(nm, nmd);

	return TRUE;
}

INT_PTR CustomCatMenu(WPARAM hContact, LPARAM lParam)
{
	if (lParam != 0) {
		SmileyCategoryType* smct = g_SmileyCategories.GetSmileyCategory((unsigned)lParam - 3);
		if (smct != NULL) 
			opt.WriteContactCategory(hContact, smct->GetName());
		else {
			CMString empty;
			if (lParam == 1) empty = _T("<None>");
			opt.WriteContactCategory(hContact, empty);
		}
		NotifyEventHooks(hEvent1, hContact, 0);
	}

	for (int i=0; i < menuHandleArray.getCount(); i++)
		CallService(MO_REMOVEMENUITEM, (WPARAM)menuHandleArray[i], 0);
	menuHandleArray.destroy();

	return TRUE;
}


int RebuildContactMenu(WPARAM wParam, LPARAM)
{
	SmileyCategoryListType::SmileyCategoryVectorType& smc = *g_SmileyCategories.GetSmileyCategoryList();

	char* protnam = GetContactProto(wParam);
	bool haveMenu = IsSmileyProto(protnam);
	if (haveMenu && opt.UseOneForAll) {
		unsigned cnt = 0;
		for (int i=0; i < smc.getCount(); i++)
			cnt += smc[i].IsCustom();
		haveMenu = cnt != 0;
	}

	Menu_ShowItem(hContactMenuItem, haveMenu);

	for (int i=0; i < menuHandleArray.getCount(); i++)
		CallService(MO_REMOVEMENUITEM, (WPARAM)menuHandleArray[i], 0);
	menuHandleArray.destroy();

	if (haveMenu) {
		CMString cat;
		opt.ReadContactCategory(wParam, cat);

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.hParentMenu = hContactMenuItem;
		mi.flags       = CMIF_CHILDPOPUP | CMIM_FLAGS | CMIF_TCHAR;
		mi.pszService  = MS_SMILEYADD_CUSTOMCATMENU;

		bool nonecheck = true;
		HGENMENU hMenu;

		for (int i=0; i < smc.getCount(); i++) {
			if (smc[i].IsExt() || (smc[i].IsProto() && opt.UseOneForAll))
				continue;

			const int ind = i + 3;

			mi.position      = ind;
			mi.popupPosition = ind;
			mi.ptszName      = (TCHAR*)smc[i].GetDisplayName().c_str();

			if (cat == smc[i].GetName()) {
				mi.flags |= CMIF_CHECKED; 
				nonecheck = false;
			}

			hMenu = Menu_AddContactMenuItem(&mi);
			menuHandleArray.insert(hMenu);
			mi.flags &= ~CMIF_CHECKED;
		}

		mi.position      = 1;
		mi.popupPosition = 1;
		mi.ptszName      = _T("<None>");
		if (cat == _T("<None>")) {
			mi.flags |= CMIF_CHECKED; 
			nonecheck = false;
		}

		hMenu = Menu_AddContactMenuItem(&mi);
		menuHandleArray.insert(hMenu);

		mi.position      = 2;
		mi.popupPosition = 2;
		mi.ptszName      = LPGENT("Protocol specific");
		if (nonecheck) mi.flags |= CMIF_CHECKED; else mi.flags &= ~CMIF_CHECKED;

		hMenu = Menu_AddContactMenuItem(&mi);
		menuHandleArray.insert(hMenu);
	}

	return 0;
}

INT_PTR ReloadPack(WPARAM, LPARAM lParam)
{
	if (lParam)
	{
		CMString categoryName = A2T_SM((char*)lParam);
		SmileyCategoryType *smc = g_SmileyCategories.GetSmileyCategory(categoryName);
		if (smc != NULL) smc->Load();
	}
	else
		g_SmileyCategories.ClearAndLoadAll();

	NotifyEventHooks(hEvent1, 0, 0);
	return 0;
}

INT_PTR LoadContactSmileys(WPARAM, LPARAM lParam)
{
	if  (opt.DisableCustom) return 0;

	SMADD_CONT* cont = (SMADD_CONT*)lParam;

	switch (cont->type) {
	case 0:
		g_SmileyPackCStore.AddSmileyPack(cont->hContact, cont->path);
		NotifyEventHooks(hEvent1, (WPARAM)cont->hContact, 0);
		break;

	case 1:
		g_SmileyPackCStore.AddSmiley(cont->hContact, cont->path);
		NotifyEventHooks(hEvent1, (WPARAM)cont->hContact, 0);
		break;
	}
	return 0;
}

int AccountListChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT* acc = (PROTOACCOUNT*)lParam;

	switch (wParam)
	{
	case PRAC_ADDED:
		if (acc != NULL)
		{
			CMString catname(_T("Standard"));
			const CMString& defaultFile = g_SmileyCategories.GetSmileyCategory(catname)->GetFilename();
			g_SmileyCategories.AddAccountAsCategory(acc, defaultFile);
		}
		break;

	case PRAC_CHANGED:
		if (acc != NULL && acc->szModuleName != NULL)
		{
			CMString name(A2T_SM(acc->szModuleName));
			SmileyCategoryType* smc = g_SmileyCategories.GetSmileyCategory(name);
			if (smc != NULL)
			{
				if (acc->tszAccountName) name = acc->tszAccountName;
				smc->SetDisplayName(name);
			}
		}
		break;

	case PRAC_REMOVED:
		g_SmileyCategories.DeleteAccountAsCategory(acc);
		break;

	case PRAC_CHECKED:
		if (acc != NULL)
		{
			if (acc->bIsEnabled)
			{
				CMString catname(_T("Standard"));
				const CMString& defaultFile = g_SmileyCategories.GetSmileyCategory(catname)->GetFilename();
				g_SmileyCategories.AddAccountAsCategory(acc, defaultFile);
			}
			else
			{
				g_SmileyCategories.DeleteAccountAsCategory(acc);
			}
		}
		break;
	}
	return 0;
}

int DbSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;

	DBCONTACTWRITESETTING* cws = (DBCONTACTWRITESETTING*)lParam;
	if (cws->value.type == DBVT_DELETED)
		return 0; 

	if (strcmp(cws->szSetting, "Transport") == 0) {
		CMString catname(_T("Standard"));
		SmileyCategoryType *smc = g_SmileyCategories.GetSmileyCategory(catname);
		if (smc != NULL)
			g_SmileyCategories.AddContactTransportAsCategory(hContact, smc->GetFilename());
	}
	return 0;
}
