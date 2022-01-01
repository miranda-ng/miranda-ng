/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

/////////////////////////////////////////////////////////////////////////////////////////
//processing of all the CLM_ messages incoming
//
LRESULT cli_ProcessExternalMessages(HWND hwnd, ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ClcContact *contact = nullptr;
	ClcGroup *group = nullptr;

	switch (msg) {
	case CLM_DELETEITEM:
		Clist_DeleteItemFromTree(hwnd, wParam);
		clcSetDelayTimer(TIMERID_DELAYEDRESORTCLC, hwnd, 1); //g_clistApi.pfnSortCLC(hwnd, dat, 1);
		clcSetDelayTimer(TIMERID_RECALCSCROLLBAR, hwnd, 2); //g_clistApi.pfnRecalcScrollBar(hwnd, dat);
		return 0;

	case CLM_AUTOREBUILD:
		if (dat->bForceInDialog) {
			Clist_SaveStateAndRebuildList(hwnd, dat);
			g_clistApi.bAutoRebuild = false;
		}
		else clcSetDelayTimer(TIMERID_REBUILDAFTER, hwnd);
		return 0;

	case CLM_SETFONT:
		if (HIWORD(lParam) > FONTID_MODERN_MAX) return 0;

		dat->fontModernInfo[HIWORD(lParam)].hFont = (HFONT)wParam;
		dat->fontModernInfo[HIWORD(lParam)].changed = 1;

		RowHeights_GetMaxRowHeight(dat, hwnd);

		if (LOWORD(lParam))
			cliInvalidateRect(hwnd, nullptr, FALSE);
		return 0;

	case CLM_SETHIDEEMPTYGROUPS:
		{
			BOOL old = ((GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_HIDEEMPTYGROUPS) != 0);
			if (wParam)
				SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | CLS_HIDEEMPTYGROUPS);
			else
				SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) &~CLS_HIDEEMPTYGROUPS);
			BOOL newval = ((GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_HIDEEMPTYGROUPS) != 0);
			if (newval != old)
				Clist_InitAutoRebuild(hwnd);
		}
		return 0;

	case CLM_SETTEXTCOLOR:
		if (wParam > FONTID_MODERN_MAX) break;

		dat->fontModernInfo[wParam].colour = lParam;
		dat->bForceInDialog = true;
		// Issue 40: option knows nothing about moderns colors
		// others who know have to set colors from lowest to highest
		switch (wParam) {
		case FONTID_CONTACTS:
			dat->fontModernInfo[FONTID_SECONDLINE].colour = lParam;
			dat->fontModernInfo[FONTID_THIRDLINE].colour = lParam;
			dat->fontModernInfo[FONTID_AWAY].colour = lParam;
			dat->fontModernInfo[FONTID_DND].colour = lParam;
			dat->fontModernInfo[FONTID_NA].colour = lParam;
			dat->fontModernInfo[FONTID_OCCUPIED].colour = lParam;
			dat->fontModernInfo[FONTID_CHAT].colour = lParam;
			dat->fontModernInfo[FONTID_INVISIBLE].colour = lParam;
			dat->fontModernInfo[FONTID_CONTACT_TIME].colour = lParam;
			break;
		case FONTID_OPENGROUPS:
			dat->fontModernInfo[FONTID_CLOSEDGROUPS].colour = lParam;
			break;
		case FONTID_OPENGROUPCOUNTS:
			dat->fontModernInfo[FONTID_CLOSEDGROUPCOUNTS].colour = lParam;
			break;
		}
		return 0;

	case CLM_GETNEXTITEM:
		{
			int i = 0;
			if (wParam != CLGN_ROOT) {
				if (!Clist_FindItem(hwnd, dat, lParam, &contact, &group))
					return 0;
				i = group->cl.indexOf(contact);
				if (i < 0)
					return 0;
			}
			switch (wParam) {
			case CLGN_ROOT:
				if (dat->list.cl.getCount())
					return Clist_ContactToHItem(dat->list.cl[0]);
				return 0;
			
			case CLGN_CHILD:
				if (contact->type != CLCIT_GROUP)
					return 0;
				group = contact->group;
				if (group->cl.getCount() == 0)
					return 0;
				return Clist_ContactToHItem(group->cl[0]);
			
			case CLGN_PARENT:
				return group->groupId | HCONTACT_ISGROUP;
			
			case CLGN_NEXT:
				do {
					if (++i >= group->cl.getCount())
						return 0;
				} while (group->cl[i]->type == CLCIT_DIVIDER);
				return Clist_ContactToHItem(group->cl[i]);
			
			case CLGN_PREVIOUS:
				do {
					if (--i < 0)
						return 0;
				} while (group->cl[i]->type == CLCIT_DIVIDER);
				return Clist_ContactToHItem(group->cl[i]);
			
			case CLGN_NEXTCONTACT:
				for (i++; i < group->cl.getCount(); i++)
					if (group->cl[i]->type == CLCIT_CONTACT)
						break;
				if (i >= group->cl.getCount())
					return 0;
				return Clist_ContactToHItem(group->cl[i]);
			
			case CLGN_PREVIOUSCONTACT:
				if (i >= group->cl.getCount())
					return 0;
				for (i--; i >= 0; i--)
					if (group->cl[i]->type == CLCIT_CONTACT)
						break;
				if (i < 0)
					return 0;
				return Clist_ContactToHItem(group->cl[i]);
			
			case CLGN_NEXTGROUP:
				for (i++; i < group->cl.getCount(); i++)
					if (group->cl[i]->type == CLCIT_GROUP)
						break;
				if (i >= group->cl.getCount())
					return 0;
				return Clist_ContactToHItem(group->cl[i]);
			
			case CLGN_PREVIOUSGROUP:
				if (i >= group->cl.getCount())
					return 0;
				for (i--; i >= 0; i--)
					if (group->cl[i]->type == CLCIT_GROUP)
						break;
				if (i < 0)
					return 0;
				return Clist_ContactToHItem(group->cl[i]);
			}
		}
		return 0;

	case CLM_SELECTITEM:
		ClcGroup *tgroup;
		{
			int index = -1;
			int mainindex = -1;
			if (!Clist_FindItem(hwnd, dat, wParam, &contact, &group))
				break;
			for (tgroup = group; tgroup; tgroup = tgroup->parent)
				g_clistApi.pfnSetGroupExpand(hwnd, dat, tgroup, 1);

			if (!contact->iSubNumber) {
				index = group->cl.indexOf(contact);
				mainindex = index;
			}
			else {
				index = group->cl.indexOf(contact->subcontacts);
				mainindex = index;
				index += contact->iSubNumber;
			}

			uint8_t k = db_get_b(0, "CLC", "MetaExpanding", SETTING_METAEXPANDING_DEFAULT);
			if (k) {
				for (int i = 0; i < mainindex; i++) {
					ClcContact *tempCont = group->cl[i];
					if (tempCont->type == CLCIT_CONTACT && tempCont->iSubAllocated && tempCont->bSubExpanded)
						index += tempCont->iSubAllocated;
				}
			}

			dat->selection = g_clistApi.pfnGetRowsPriorTo(&dat->list, group, index);
			Clist_EnsureVisible(hwnd, dat, dat->selection, 0);
		}
		return 0;
	}
	return corecli.pfnProcessExternalMessages(hwnd, dat, msg, wParam, lParam);
}
