/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project,
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

HCURSOR g_hCurHyperlinkHand;

void LoadSrmmToolbarModule();
void UnloadSrmmToolbarModule();

void SafeDestroyIcon(HICON hIcon)
{
	if (hIcon != nullptr)
		if (!IcoLib_IsManaged(hIcon))
			::DestroyIcon(hIcon);
}

struct StatusIconChild : public MZeroedObject
{
	~StatusIconChild()
	{
		SafeDestroyIcon(hIcon);
		SafeDestroyIcon(hIconDisabled);
		mir_free(tszTooltip);
	}

	MCONTACT hContact;
	HICON  hIcon, hIconDisabled;
	int    flags;
	wchar_t *tszTooltip;
};

struct StatusIconMain : public MZeroedObject
{
	StatusIconMain() :
		arChildren(3, NumericKeySortT)
		{}

	~StatusIconMain()
	{
		mir_free(sid.szModule);
		mir_free(sid.szTooltip);
	}

	StatusIconData sid;

	int hLangpack;
	OBJLIST<StatusIconChild> arChildren;
};

static int CompareIcons(const StatusIconMain *p1, const StatusIconMain *p2)
{
	int res = mir_strcmp(p1->sid.szModule, p2->sid.szModule);
	if (res)
		return res;
	
	return p1->sid.dwId - p2->sid.dwId;
}

static OBJLIST<StatusIconMain> arIcons(3, CompareIcons);

HANDLE hHookIconsChanged, hHookSrmmEvent;

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Srmm_AddIcon(StatusIconData *sid, int _hLangpack)
{
	if (sid == nullptr)
		return 1;

	StatusIconMain *p = arIcons.find((StatusIconMain*)sid);
	if (p != nullptr)
		return Srmm_ModifyIcon(0, sid);

	p = new StatusIconMain;
	memcpy(&p->sid, sid, sizeof(p->sid));
	p->hLangpack = _hLangpack;
	p->sid.szModule = mir_strdup(sid->szModule);
	if (sid->flags & MBF_UNICODE)
		p->sid.tszTooltip = mir_wstrdup(sid->wszTooltip);
	else
		p->sid.tszTooltip = mir_a2u(sid->szTooltip);
	arIcons.insert(p);

	NotifyEventHooks(hHookIconsChanged, 0, (LPARAM)p);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Srmm_ModifyIcon(MCONTACT hContact, StatusIconData *sid)
{
	if (sid == nullptr)
		return 1;

	StatusIconMain *p = arIcons.find((StatusIconMain*)sid);
	if (p == nullptr)
		return 1;

	if (hContact == 0) {
		mir_free(p->sid.szModule);
		mir_free(p->sid.szTooltip);
		memcpy(&p->sid, sid, sizeof(p->sid));
		p->sid.szModule = mir_strdup(sid->szModule);
		p->sid.tszTooltip = (sid->flags & MBF_UNICODE) ? mir_wstrdup(sid->wszTooltip) : mir_a2u(sid->szTooltip);

		NotifyEventHooks(hHookIconsChanged, 0, (LPARAM)p);
		return 0;
	}

	StatusIconChild *pc = p->arChildren.find((StatusIconChild*)&hContact);
	if (pc == nullptr) {
		pc = new StatusIconChild();
		pc->hContact = hContact;
		p->arChildren.insert(pc);
	}
	else SafeDestroyIcon(pc->hIcon);

	pc->flags = sid->flags;
	pc->hIcon = sid->hIcon;
	pc->hIconDisabled = sid->hIconDisabled;

	mir_free(pc->tszTooltip);
	pc->tszTooltip = (sid->flags & MBF_UNICODE) ? mir_wstrdup(sid->wszTooltip) : mir_a2u(sid->szTooltip);

	NotifyEventHooks(hHookIconsChanged, hContact, (LPARAM)p);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Srmm_RemoveIcon(const char *szProto, DWORD iconId)
{
	StatusIconData tmp = { (char*)szProto, iconId };
	StatusIconMain *p = arIcons.find((StatusIconMain*)&tmp);
	if (p != nullptr)
		arIcons.remove(p);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(StatusIconData*) Srmm_GetNthIcon(MCONTACT hContact, int index)
{
	static StatusIconData res;

	for (int i=arIcons.getCount()-1, nVis = 0; i >= 0; i--) {
		StatusIconMain &p = arIcons[i];

		StatusIconChild *pc = p.arChildren.find((StatusIconChild*)&hContact);
		if (pc) {
			if (pc->flags & MBF_HIDDEN)
				continue;
		}
		else if (p.sid.flags & MBF_HIDDEN)
			continue;

		if (nVis == index) {
			memcpy(&res, &p, sizeof(res));
			if (pc) {
				if (pc->hIcon) res.hIcon = pc->hIcon;
				if (pc->hIconDisabled)
					res.hIconDisabled = pc->hIconDisabled;
				else if (pc->hIcon)
					res.hIconDisabled = pc->hIcon;
				if (pc->tszTooltip) res.tszTooltip = pc->tszTooltip;
				res.flags = pc->flags;
			}
			res.tszTooltip = TranslateW_LP(res.tszTooltip, p.hLangpack);
			return &res;
		}
		nVis++;
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

void KillModuleSrmmIcons(int _hLang)
{
	for (int i = arIcons.getCount()-1; i >= 0; i--) {
		StatusIconMain &p = arIcons[i];
		if (p.hLangpack == _hLang)
			arIcons.remove(i);
	}
}

int LoadSrmmModule()
{
	g_hCurHyperlinkHand = LoadCursor(nullptr, IDC_HAND);

	LoadSrmmToolbarModule();
	
	hHookSrmmEvent = CreateHookableEvent(ME_MSG_WINDOWEVENT);
	hHookIconsChanged = CreateHookableEvent(ME_MSG_ICONSCHANGED);
	return 0;
}

void UnloadSrmmModule()
{
	arIcons.destroy();

	DestroyHookableEvent(hHookIconsChanged);
	DestroyHookableEvent(hHookSrmmEvent);

	DestroyCursor(g_hCurHyperlinkHand);

	UnloadSrmmToolbarModule();
}
