/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-19 Miranda NG team,
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
HANDLE hHookSrmmEvent;

static HANDLE hHookIconsChanged, hHookIconPressedEvt;

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
		mir_free(pwszTooltip);
	}

	MCONTACT hContact;
	HICON hIcon, hIconDisabled;
	HANDLE hIcolibOn, hIcolibOff;
	int    flags;
	wchar_t *pwszTooltip;
};

struct StatusIconMain : public MZeroedObject
{
	StatusIconMain() :
		arChildren(3, NumericKeySortT)
		{}

	~StatusIconMain()
	{
		mir_free((void*)sid.szModule);
		mir_free((void*)sid.szTooltip.w);
	}

	StatusIconData sid;

	HANDLE hIcolibOn, hIcolibOff;
	HPLUGIN pPlugin;
	OBJLIST<StatusIconChild> arChildren;
};

static int CompareIcons(const StatusIconMain *p1, const StatusIconMain *p2)
{
	int res = mir_strcmp(p1->sid.szModule, p2->sid.szModule);
	if (res)
		return res;
	
	return p1->sid.dwId - p2->sid.dwId;
}

static OBJLIST<StatusIconMain> arIcons(10, CompareIcons);

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Srmm_AddIcon(StatusIconData *sid, HPLUGIN pPlugin)
{
	if (sid == nullptr)
		return 1;

	StatusIconMain *p = arIcons.find((StatusIconMain*)sid);
	if (p != nullptr)
		return Srmm_ModifyIcon(0, sid);

	p = new StatusIconMain;
	memcpy(&p->sid, sid, sizeof(p->sid));
	p->pPlugin = pPlugin;
	p->sid.szModule = mir_strdup(sid->szModule);
	p->hIcolibOn = IcoLib_IsManaged(sid->hIcon);
	p->hIcolibOff = IcoLib_IsManaged(sid->hIconDisabled);
	if (sid->flags & MBF_UNICODE)
		p->sid.szTooltip.w = mir_wstrdup(sid->szTooltip.w);
	else
		p->sid.szTooltip.w = mir_a2u(sid->szTooltip.a);
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
		mir_free((void*)p->sid.szModule);
		mir_free((void*)p->sid.szTooltip.w);
		memcpy(&p->sid, sid, sizeof(p->sid));
		p->hIcolibOn = IcoLib_IsManaged(sid->hIcon);
		p->hIcolibOff = IcoLib_IsManaged(sid->hIconDisabled);
		p->sid.szModule = mir_strdup(sid->szModule);
		p->sid.szTooltip.w = (sid->flags & MBF_UNICODE) ? mir_wstrdup(sid->szTooltip.w) : mir_a2u(sid->szTooltip.a);

		NotifyEventHooks(hHookIconsChanged, 0, (LPARAM)p);
		return 0;
	}

	StatusIconChild *pc = p->arChildren.find((StatusIconChild*)&hContact);
	if (pc == nullptr) {
		pc = new StatusIconChild();
		pc->hContact = hContact;
		p->arChildren.insert(pc);
	}

	pc->flags = sid->flags;
	pc->hIcon = sid->hIcon;
	pc->hIconDisabled = sid->hIconDisabled;
	pc->hIcolibOn = IcoLib_IsManaged(sid->hIcon);
	pc->hIcolibOff = IcoLib_IsManaged(sid->hIconDisabled);
	replaceStrW(pc->pwszTooltip, (sid->flags & MBF_UNICODE) ? mir_wstrdup(sid->szTooltip.w) : mir_a2u(sid->szTooltip.a));

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

	int nVis = 0;
	for (auto &it : arIcons.rev_iter()) {
		StatusIconChild *pc = it->arChildren.find((StatusIconChild*)&hContact);
		if (pc) {
			if (pc->flags & MBF_HIDDEN)
				continue;
		}
		else if (it->sid.flags & MBF_HIDDEN)
			continue;

		if (nVis == index) {
			memcpy(&res, it, sizeof(res));
			if (pc) {
				if (pc->hIcon)
					res.hIcon = pc->hIcon;
				if (pc->hIconDisabled)
					res.hIconDisabled = pc->hIconDisabled;
				else if (pc->hIcon)
					res.hIconDisabled = pc->hIcon;
				if (pc->pwszTooltip)
					res.szTooltip.w = pc->pwszTooltip;
				res.flags = pc->flags;
			}
			res.szTooltip.w = TranslateW_LP(res.szTooltip.w, it->pPlugin);
			return &res;
		}
		nVis++;
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Srmm_ClickStatusIcon(MCONTACT hContact, const StatusIconClickData *sid)
{
	NotifyEventHooks(hHookIconPressedEvt, hContact, (LPARAM)sid);
}

/////////////////////////////////////////////////////////////////////////////////////////

void KillModuleSrmmIcons(HPLUGIN pPlugin)
{
	auto T = arIcons.rev_iter();
	for (auto &it : T)
		if (it->pPlugin == pPlugin)
			arIcons.remove(T.indexOf(&it));
}

int LoadSrmmModule()
{
	g_hCurHyperlinkHand = LoadCursor(nullptr, IDC_HAND);

	LoadSrmmToolbarModule();

	hHookSrmmEvent = CreateHookableEvent(ME_MSG_WINDOWEVENT);
	hHookIconsChanged = CreateHookableEvent(ME_MSG_ICONSCHANGED);
	hHookIconPressedEvt = CreateHookableEvent(ME_MSG_ICONPRESSED);
	return 0;
}

void UnloadSrmmModule()
{
	arIcons.destroy();

	DestroyHookableEvent(hHookIconsChanged);
	DestroyHookableEvent(hHookSrmmEvent);
	DestroyHookableEvent(hHookIconPressedEvt);

	DestroyCursor(g_hCurHyperlinkHand);

	UnloadSrmmToolbarModule();
}
