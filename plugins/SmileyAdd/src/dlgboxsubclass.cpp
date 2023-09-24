/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2012 Boris Krasnovskiy All Rights Reserved
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

#include "stdafx.h"

static mir_cs csWndList;

// type definitions 

struct MsgWndData : public MZeroedObject
{
	CSrmmBaseDialog *pDlg;
	char *szProto;

	MsgWndData(CSrmmBaseDialog *p) :
		pDlg(p)
	{
		szProto = Proto_GetBaseAccountName(DecodeMetaContact(pDlg->m_hContact));
	}

	void CreateSmileyButton(void)
	{
		SmileyPackType *SmileyPack = FindSmileyPack(szProto, pDlg->m_hContact);
		bool doSmileyButton = SmileyPack && SmileyPack->VisibleSmileyCount() != 0;

		if (szProto) {
			INT_PTR cap = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
			doSmileyButton &= ((cap & (PF1_IMSEND | PF1_CHAT)) != 0);
		}

		BBButton bbd = {};
		bbd.pszModuleName = MODULENAME;
		if (!doSmileyButton)
			bbd.bbbFlags = BBBF_DISABLED;
		Srmm_SetButtonState(pDlg->m_hContact, &bbd);
	}
};

static OBJLIST<MsgWndData> g_MsgWndList(10, HandleKeySortT);

int UpdateSrmmDlg(WPARAM wParam, LPARAM)
{
	mir_cslock lck(csWndList);
	
	for (auto &it : g_MsgWndList) {
		if (wParam == 0 || it->pDlg->m_hContact == wParam) {
			SendMessage(it->pDlg->GetHwnd(), WM_SETREDRAW, FALSE, 0);
			it->pDlg->UpdateOptions();
			SendMessage(it->pDlg->GetHwnd(), WM_SETREDRAW, TRUE, 0);
		}
	}
	return 0;
}

// find the dialog info in the stored list
static MsgWndData* IsMsgWnd(HWND hwnd)
{
	mir_cslock lck(csWndList);
	for (auto &it : g_MsgWndList)
		if (it->pDlg->GetHwnd() == hwnd)
			return it;

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// toolbar button processing

int SmileyButtonCreate(WPARAM, LPARAM)
{
	// create a hotkey for the button first
	HOTKEYDESC desc = {};
	desc.pszName = "srmm_smileyadd";
	desc.szSection.a = BB_HK_SECTION;
	desc.szDescription.a = LPGEN("Smiley selector");
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_ALT, 'E');
	desc.lParam = LPARAM(g_plugin.getInst());
	g_plugin.addHotkey(&desc);

	BBButton bbd = {};
	bbd.pszModuleName = MODULENAME;
	bbd.pwszTooltip = LPGENW("Show smiley selection window");
	bbd.dwDefPos = 31;
	bbd.hIcon = IcoLib_GetIconHandle("SmileyAdd_ButtonSmiley");
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON;
	bbd.pszHotkey = desc.pszName;
	Srmm_AddButton(&bbd, &g_plugin);
	return 0;
}

int SmileyButtonPressed(WPARAM, LPARAM lParam)
{
	CustomButtonClickData *pcbc = (CustomButtonClickData*)lParam;
	if (mir_strcmp(pcbc->pszModule, MODULENAME))
		return 0;

	MsgWndData *dat = IsMsgWnd(pcbc->hwndFrom);
	if (dat == nullptr)
		return 0;

	SmileyToolWindowParam *stwp = new SmileyToolWindowParam;
	stwp->pSmileyPack = FindSmileyPack(dat->szProto, dat->pDlg->m_hContact);
	stwp->hWndParent = pcbc->hwndFrom;
	stwp->hWndTarget = dat->pDlg->GetInput();
	stwp->targetMessage = EM_REPLACESEL;
	stwp->targetWParam = TRUE;
	stwp->direction = 0;
	stwp->xPosition = pcbc->pt.x;
	stwp->yPosition = pcbc->pt.y;
	mir_forkThread<SmileyToolWindowParam>(SmileyToolThread, stwp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// window hook

static int MsgDlgHook(WPARAM uType, LPARAM lParam)
{
	auto *pDlg = (CSrmmBaseDialog *)lParam;
	auto hwndLog = pDlg->log()->GetHwnd();

	switch (uType) {
	case MSG_WINDOW_EVT_OPENING:
		{
			MsgWndData *msgwnd = new MsgWndData(pDlg);
			msgwnd->CreateSmileyButton();

			mir_cslock lck(csWndList);
			g_MsgWndList.insert(msgwnd);
		}

		SetRichOwnerCallback(pDlg->GetHwnd(), pDlg->GetInput(), hwndLog);

		if (hwndLog)
			SetRichCallback(hwndLog, pDlg->m_hContact, false, false);
		if (pDlg->GetInput())
			SetRichCallback(pDlg->GetInput(), pDlg->m_hContact, false, false);
		break;

	case MSG_WINDOW_EVT_OPEN:
		SetRichOwnerCallback(pDlg->GetHwnd(), pDlg->GetInput(), hwndLog);
		if (hwndLog)
			SetRichCallback(hwndLog, pDlg->m_hContact, true, true);
		if (pDlg->GetInput()) {
			SetRichCallback(pDlg->GetInput(), pDlg->m_hContact, true, true);
			SendMessage(pDlg->GetInput(), WM_REMAKERICH, 0, 0);
		}
		break;

	case MSG_WINDOW_EVT_CLOSE:
		if (hwndLog) {
			CloseRichCallback(hwndLog);
			CloseRichOwnerCallback(pDlg->GetHwnd());
		}

		mir_cslock lck(csWndList);
		g_MsgWndList.remove((MsgWndData *)&pDlg);
	}
	return 0;
}

void InstallDialogBoxHook(void)
{
	HookEvent(ME_MSG_WINDOWEVENT, MsgDlgHook);
}

void RemoveDialogBoxHook(void)
{
	mir_cslock lck(csWndList);
	for (auto &it : g_MsgWndList)
		delete it;
	g_MsgWndList.destroy();
}
