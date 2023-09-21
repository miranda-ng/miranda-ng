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
	HWND hwnd, hwndLog, hwndInput;
	int  idxLastChar;
	bool doSmileyReplace;
	MCONTACT hContact;
	char ProtocolName[52];

	void CreateSmileyButton(void)
	{
		SmileyPackType *SmileyPack = FindSmileyPack(ProtocolName, hContact);
		bool doSmileyButton = SmileyPack != nullptr && SmileyPack->VisibleSmileyCount() != 0;

		doSmileyReplace = true;

		if (ProtocolName[0] != 0) {
			INT_PTR cap = CallProtoService(ProtocolName, PS_GETCAPS, PFLAGNUM_1, 0);
			doSmileyButton &= ((cap & (PF1_IMSEND | PF1_CHAT)) != 0);
			doSmileyReplace &= ((cap & (PF1_IMRECV | PF1_CHAT)) != 0);
		}

		BBButton bbd = {};
		bbd.pszModuleName = MODULENAME;
		if (!doSmileyButton)
			bbd.bbbFlags = BBBF_DISABLED;
		Srmm_SetButtonState(hContact, &bbd);
	}
};

static LIST<MsgWndData> g_MsgWndList(10, HandleKeySortT);

int UpdateSrmmDlg(WPARAM wParam, LPARAM)
{
	mir_cslock lck(csWndList);
	
	for (auto &it : g_MsgWndList) {
		if (wParam == 0 || it->hContact == wParam) {
			SendMessage(it->hwnd, WM_SETREDRAW, FALSE, 0);
			SendMessage(it->hwnd, DM_OPTIONSAPPLIED, 0, 0);
			SendMessage(it->hwnd, WM_SETREDRAW, TRUE, 0);
		}
	}
	return 0;
}

// find the dialog info in the stored list
static MsgWndData* IsMsgWnd(HWND hwnd)
{
	mir_cslock lck(csWndList);
	return g_MsgWndList.find((MsgWndData*)&hwnd);
}

// global subclass function for all dialogs
static LRESULT CALLBACK MessageDlgSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MsgWndData *dat = IsMsgWnd(hwnd);
	if (dat == nullptr)
		return 0;

	if (uMsg == DM_OPTIONSAPPLIED)
		dat->CreateSmileyButton();

	LRESULT result = mir_callNextSubclass(hwnd, MessageDlgSubclass, uMsg, wParam, lParam);

	if (uMsg == WM_DESTROY) {
		mir_cslock lck(csWndList);
		int ind = g_MsgWndList.getIndex((MsgWndData*)&hwnd);
		if (ind != -1) {
			delete g_MsgWndList[ind];
			g_MsgWndList.remove(ind);
		}
	}

	return result;
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
	stwp->pSmileyPack = FindSmileyPack(dat->ProtocolName, dat->hContact);
	stwp->hWndParent = pcbc->hwndFrom;
	stwp->hWndTarget = dat->hwndInput;
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
			MsgWndData *msgwnd = new MsgWndData();
			msgwnd->hwnd = pDlg->GetHwnd();
			msgwnd->hContact = pDlg->m_hContact;
			msgwnd->hwndLog = hwndLog;
			msgwnd->hwndInput = pDlg->GetInput();

			// Get the protocol for this contact to display correct smileys.
			char *protonam = Proto_GetBaseAccountName(DecodeMetaContact(msgwnd->hContact));
			if (protonam)
				strncpy_s(msgwnd->ProtocolName, protonam, _TRUNCATE);

			mir_subclassWindow(msgwnd->hwnd, MessageDlgSubclass);
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
		mir_unsubclassWindow(pDlg->GetHwnd(), MessageDlgSubclass);
		break;
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
