/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-13 Miranda NG project,
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

#include "..\..\core\commonheaders.h"

struct StatusIconChild : public MZeroedObject
{
	~StatusIconChild()
	{
		mir_free(szTooltip);
	}

	HANDLE hContact;
	HICON hIcon, hIconDisabled;
	int flags;
	char *szTooltip;
};

struct StatusIconMain : public MZeroedObject
{
	StatusIconMain() :
		arChildren(3, HandleKeySortT)
		{}

	~StatusIconMain()
	{
		mir_free(sid.szModule);
		mir_free(sid.szTooltip);
	}

	StatusIconData sid;
	OBJLIST<StatusIconChild> arChildren;
};

static int CompareIcons(const StatusIconMain *p1, const StatusIconMain *p2)
{
	int res = strcmp(p1->sid.szModule, p2->sid.szModule);
	if (res)
		return res;
	
	return p1->sid.dwId - p2->sid.dwId;
}

static OBJLIST<StatusIconMain> arIcons(3, CompareIcons);

static HANDLE hHookIconsChanged;

INT_PTR ModifyStatusIcon(WPARAM wParam, LPARAM lParam)
{
	StatusIconData *sid = (StatusIconData *)lParam;
	if (sid == NULL || sid->cbSize != sizeof(StatusIconData))
		return 1;

	StatusIconMain *p = arIcons.find((StatusIconMain*)sid);
	if (p == NULL)
		return 1;

	HANDLE hContact = (HANDLE)wParam;
	if (hContact == NULL) {
		memcpy(&p->sid, sid, sizeof(p->sid));
		replaceStr(p->sid.szModule, sid->szModule);
		replaceStr(p->sid.szTooltip, sid->szTooltip);
		NotifyEventHooks(hHookIconsChanged, NULL, (LPARAM)p);
		return 0;
	}

	StatusIconChild *pc = p->arChildren.find((StatusIconChild*)&hContact);
	if (pc == NULL) {
		pc = new StatusIconChild();
		pc->hContact = hContact;
		p->arChildren.insert(pc);
	}

	pc->flags = sid->flags;
	pc->hIcon = sid->hIcon;
	pc->hIconDisabled = sid->hIconDisabled;
	replaceStr(pc->szTooltip, sid->szTooltip);
	NotifyEventHooks(hHookIconsChanged, wParam, (LPARAM)p);
	return 0;
}

static INT_PTR AddStatusIcon(WPARAM wParam, LPARAM lParam)
{
	StatusIconData *sid = (StatusIconData *)lParam;
	if (sid == NULL || sid->cbSize != sizeof(StatusIconData))
		return 1;

	StatusIconMain *p = arIcons.find((StatusIconMain*)sid);
	if (p != NULL)
		return ModifyStatusIcon(0, lParam);

	p = new StatusIconMain;
	memcpy(&p->sid, sid, sizeof(p->sid));
	p->sid.szModule = mir_strdup(sid->szModule);
	p->sid.szTooltip = mir_strdup(sid->szTooltip);
	arIcons.insert(p);

	NotifyEventHooks(hHookIconsChanged, NULL, (LPARAM)p);
	return 0;
}

static INT_PTR RemoveStatusIcon(WPARAM wParam, LPARAM lParam)
{
	StatusIconData *sid = (StatusIconData *)lParam;
	if (sid == NULL || sid->cbSize != sizeof(StatusIconData))
		return 1;

	StatusIconMain *p = arIcons.find((StatusIconMain*)sid);
	if (p == NULL)
		return 1;

	arIcons.remove(p);
	return 0;
}

static INT_PTR GetNthIcon(WPARAM wParam, LPARAM lParam)
{
	static StatusIconData res;

	for (int i=arIcons.getCount()-1, nVis = 0; i >= 0; i--) {
		StatusIconMain &p = arIcons[i];

		StatusIconChild *pc = p.arChildren.find((StatusIconChild*)&wParam);
		if (pc) {
			if (pc->flags & MBF_HIDDEN)
				continue;
		}
		else if (p.sid.flags & MBF_HIDDEN)
			continue;

		if (nVis == (int)lParam) {
			memcpy(&res, &p, sizeof(res));
			if (pc) {
				res.hIcon = pc->hIcon;
				res.hIconDisabled = pc->hIconDisabled;
				res.flags = pc->flags;
				res.szTooltip = pc->szTooltip;
			}
			return (INT_PTR)&res;
		}
		nVis++;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *evt = (MessageWindowEventData*)lParam;
	if (evt->uType == MSG_WINDOW_EVT_CLOSE) {
		for (int i=0; i < arIcons.getCount(); i++) {
			StatusIconMain &p = arIcons[i];
			p.arChildren.remove((StatusIconChild*)&evt->hContact);
		}
	}

	return 0;			
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_MSG_WINDOWEVENT, OnWindowEvent);
	return 0;
}

int LoadSrmmModule()
{
	CreateServiceFunction(MS_MSG_ADDICON,    AddStatusIcon);
	CreateServiceFunction(MS_MSG_REMOVEICON, RemoveStatusIcon);
	CreateServiceFunction(MS_MSG_MODIFYICON, ModifyStatusIcon);
	CreateServiceFunction(MS_MSG_GETNTHICON, GetNthIcon);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	hHookIconsChanged = CreateHookableEvent(ME_MSG_ICONSCHANGED);
	return 0;
}

void UnloadSrmmModule()
{
	arIcons.destroy();
	NotifyEventHooks(hHookIconsChanged, NULL, NULL);
	DestroyHookableEvent(hHookIconsChanged);
}
