/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project,
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
		SafeDestroyIcon(hIcon);
		SafeDestroyIcon(hIconDisabled);
		mir_free(tszTooltip);
	}

	MCONTACT hContact;
	HICON  hIcon, hIconDisabled;
	int    flags;
	TCHAR *tszTooltip;

	void SafeDestroyIcon(HICON hIcon)
	{
		if (hIcon == NULL)
			return;

		if (!IcoLib_IsManaged(hIcon))
			::DestroyIcon(hIcon);
	}
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

static HANDLE hHookIconsChanged;

INT_PTR ModifyStatusIcon(WPARAM hContact, LPARAM lParam)
{
	StatusIconData *sid = (StatusIconData *)lParam;
	if (sid == NULL || sid->cbSize != sizeof(StatusIconData))
		return 1;

	StatusIconMain *p = arIcons.find((StatusIconMain*)sid);
	if (p == NULL)
		return 1;

	if (hContact == NULL) {
		mir_free(p->sid.szModule);
		mir_free(p->sid.szTooltip);
		memcpy(&p->sid, sid, sizeof(p->sid));
		p->sid.szModule = mir_strdup(sid->szModule);
		p->sid.tszTooltip = (sid->flags & MBF_UNICODE) ? mir_u2t(sid->wszTooltip) : mir_a2t(sid->szTooltip);

		NotifyEventHooks(hHookIconsChanged, NULL, (LPARAM)p);
		return 0;
	}

	StatusIconChild *pc = p->arChildren.find((StatusIconChild*)&hContact);
	if (pc == NULL) {
		pc = new StatusIconChild();
		pc->hContact = hContact;
		p->arChildren.insert(pc);
	}
	else pc->SafeDestroyIcon(pc->hIcon);

	pc->flags = sid->flags;
	pc->hIcon = sid->hIcon;
	pc->hIconDisabled = sid->hIconDisabled;

	mir_free(pc->tszTooltip);
	pc->tszTooltip = (sid->flags & MBF_UNICODE) ? mir_u2t(sid->wszTooltip) : mir_a2t(sid->szTooltip);

	NotifyEventHooks(hHookIconsChanged, hContact, (LPARAM)p);
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
	p->hLangpack = (int)wParam;
	p->sid.szModule = mir_strdup(sid->szModule);
	if (sid->flags & MBF_UNICODE)
		p->sid.tszTooltip = mir_u2t(sid->wszTooltip);
	else
		p->sid.tszTooltip = mir_a2t(sid->szTooltip);
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
				if (pc->hIcon) res.hIcon = pc->hIcon;
				if (pc->hIconDisabled)
					res.hIconDisabled = pc->hIconDisabled;
				else if (pc->hIcon)
					res.hIconDisabled = pc->hIcon;
				if (pc->tszTooltip) res.tszTooltip = pc->tszTooltip;
				res.flags = pc->flags;
			}
			res.tszTooltip = TranslateTH(p.hLangpack, res.tszTooltip);
			return (INT_PTR)&res;
		}
		nVis++;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void KillModuleSrmmIcons(int hLangpack)
{
	for (int i=arIcons.getCount()-1; i >= 0; i--) {
		StatusIconMain &p = arIcons[i];
		if (p.hLangpack == hLangpack)
			arIcons.remove(i);
	}
}

int LoadSrmmModule()
{
	CreateServiceFunction("MessageAPI/AddIcon", AddStatusIcon);
	CreateServiceFunction(MS_MSG_REMOVEICON, RemoveStatusIcon);
	CreateServiceFunction(MS_MSG_MODIFYICON, ModifyStatusIcon);
	CreateServiceFunction("MessageAPI/GetNthIcon", GetNthIcon);

	hHookIconsChanged = CreateHookableEvent(ME_MSG_ICONSCHANGED);
	return 0;
}

void UnloadSrmmModule()
{
	arIcons.destroy();
	NotifyEventHooks(hHookIconsChanged, NULL, NULL);
	DestroyHookableEvent(hHookIconsChanged);
}
