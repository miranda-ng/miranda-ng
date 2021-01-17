/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team,
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
static mir_cs csIcons;

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
	StatusIconChild(MCONTACT _1) :
		hContact(_1)
	{}

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
	{
	}

	~StatusIconMain()
	{
		mir_free(szModule);
		mir_free(pwszTooltip);
	}

	char *szModule;
	DWORD dwId;
	HICON hIcon, hIconDisabled;
	int   flags;
	wchar_t *pwszTooltip;

	HANDLE hIcolibOn, hIcolibOff;
	HPLUGIN pPlugin;
	OBJLIST<StatusIconChild> arChildren;
};

static int CompareIcons(const StatusIconMain *p1, const StatusIconMain *p2)
{
	int res = mir_strcmp(p1->szModule, p2->szModule);
	if (res)
		return res;

	return p1->dwId - p2->dwId;
}

static OBJLIST<StatusIconMain> arIcons(10, CompareIcons);

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Srmm_AddIcon(StatusIconData *sid, HPLUGIN pPlugin)
{
	if (sid == nullptr)
		return 1;

	StatusIconMain *p = arIcons.find((StatusIconMain *)sid);
	if (p != nullptr)
		return 2;

	p = new StatusIconMain;
	p->szModule = mir_strdup(sid->szModule);
	p->dwId = sid->dwId;
	p->flags = sid->flags;
	p->pPlugin = pPlugin;

	if ((p->hIcolibOn = IcoLib_IsManaged(sid->hIcon)) == nullptr)
		p->hIcon = sid->hIcon;
	if ((p->hIcolibOff = IcoLib_IsManaged(sid->hIconDisabled)) == nullptr)
		p->hIconDisabled = sid->hIconDisabled;

	if (sid->flags & MBF_UNICODE)
		p->pwszTooltip = mir_wstrdup(sid->szTooltip.w);
	else
		p->pwszTooltip = mir_a2u(sid->szTooltip.a);
	arIcons.insert(p);

	NotifyEventHooks(hHookIconsChanged, 0, (LPARAM)p);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Srmm_ModifyIcon(MCONTACT hContact, const char *szModule, DWORD iconId, HICON hIcon, const wchar_t *pwszToolTip)
{
	StatusIconData sid;
	sid.szModule = szModule;
	sid.dwId = iconId;

	mir_cslockfull lck(csIcons);
	StatusIconMain *p = arIcons.find((StatusIconMain *)&sid);
	if (p == nullptr)
		return;

	if (hContact == 0) {
		if (hIcon)
			if ((p->hIcolibOn = IcoLib_IsManaged(hIcon)) == nullptr)
				p->hIcon = hIcon;

		replaceStrW(p->pwszTooltip, pwszToolTip);
	}
	else {
		StatusIconChild *pc = p->arChildren.find((StatusIconChild *)&hContact);
		if (pc == nullptr)
			p->arChildren.insert(pc = new StatusIconChild(hContact));

		if (hIcon)
			if ((pc->hIcolibOn = IcoLib_IsManaged(hIcon)) == nullptr)
				pc->hIcon = hIcon;

		replaceStrW(pc->pwszTooltip, pwszToolTip);
	}

	lck.unlock();
	NotifyEventHooks(hHookIconsChanged, hContact, (LPARAM)p);
}

MIR_APP_DLL(void) Srmm_SetIconFlags(MCONTACT hContact, const char *szModule, DWORD iconId, int flags)
{
	StatusIconData sid;
	sid.szModule = szModule;
	sid.dwId = iconId;

	mir_cslockfull lck(csIcons);
	StatusIconMain *p = arIcons.find((StatusIconMain *)&sid);
	if (p == nullptr)
		return;

	if (hContact == 0)
		p->flags = flags;
	else {
		StatusIconChild *pc = p->arChildren.find((StatusIconChild *)&hContact);
		if (pc == nullptr)
			p->arChildren.insert(pc = new StatusIconChild(hContact));

		pc->flags = flags;
	}

	lck.unlock();
	NotifyEventHooks(hHookIconsChanged, hContact, (LPARAM)p);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Srmm_RemoveIcon(const char *szProto, DWORD iconId)
{
	mir_cslock lck(csIcons);
	StatusIconData tmp = { (char *)szProto, iconId };
	StatusIconMain *p = arIcons.find((StatusIconMain *)&tmp);
	if (p != nullptr)
		arIcons.remove(p);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void tryIcolib(HANDLE hIcolib, HICON hIcon, HICON &dest)
{
	if (hIcolib != nullptr)
		hIcon = IcoLib_GetIconByHandle(hIcolib);
	if (hIcon != nullptr)
		dest = hIcon;
	if (hIcolib != nullptr)
		IcoLib_ReleaseIcon(hIcon);
}

MIR_APP_DLL(StatusIconData *) Srmm_GetNthIcon(MCONTACT hContact, int index)
{
	static StatusIconData res;

	int nVis = 0;
	for (auto &it : arIcons.rev_iter()) {
		StatusIconChild *pc = it->arChildren.find((StatusIconChild *)&hContact);
		if (pc) {
			if (pc->flags & MBF_HIDDEN)
				continue;
		}
		else if (it->flags & MBF_HIDDEN)
			continue;

		if (nVis == index) {
			memcpy(&res, it, sizeof(res));
			if (pc) {
				tryIcolib(pc->hIcolibOn, pc->hIcon, res.hIcon);
				tryIcolib(pc->hIcolibOff, pc->hIconDisabled, res.hIconDisabled);
				if (pc->pwszTooltip)
					res.szTooltip.w = pc->pwszTooltip;
				res.flags = pc->flags;
			}

			if (res.hIcon == nullptr)
				tryIcolib(it->hIcolibOn, it->hIcon, res.hIcon);
			if (res.hIconDisabled == nullptr)
				tryIcolib(it->hIcolibOff, it->hIconDisabled, res.hIconDisabled);

			if (res.hIconDisabled == nullptr)
				res.hIconDisabled = res.hIcon;
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
	for (auto &it : arIcons.rev_iter())
		if (it->pPlugin == pPlugin)
			arIcons.removeItem(&it);
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
