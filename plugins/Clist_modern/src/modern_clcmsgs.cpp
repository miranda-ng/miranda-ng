/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_clc.h"
#include "hdr/modern_commonprototypes.h"

//processing of all the CLM_ messages incoming

LRESULT cli_ProcessExternalMessages(HWND hwnd, ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ClcContact *contact = NULL;
	ClcGroup *group = NULL;

	switch (msg) {
	case CLM_DELETEITEM:
		pcli->pfnDeleteItemFromTree(hwnd, wParam);
		clcSetDelayTimer(TIMERID_DELAYEDRESORTCLC, hwnd, 1); //pcli->pfnSortCLC(hwnd, dat, 1);
		clcSetDelayTimer(TIMERID_RECALCSCROLLBAR, hwnd, 2); //pcli->pfnRecalcScrollBar(hwnd, dat);
		return 0;

	case CLM_AUTOREBUILD:
		if (dat->force_in_dialog)
			pcli->pfnSaveStateAndRebuildList(hwnd, dat);
		else
			clcSetDelayTimer(TIMERID_REBUILDAFTER, hwnd);

		pcli->bAutoRebuild = false;
		return 0;

	case CLM_SETFONT:
		if (HIWORD(lParam) > FONTID_MODERN_MAX) return 0;

		dat->fontModernInfo[HIWORD(lParam)].hFont = (HFONT)wParam;
		dat->fontModernInfo[HIWORD(lParam)].changed = 1;

		RowHeights_GetMaxRowHeight(dat, hwnd);

		if (LOWORD(lParam))
			CLUI__cliInvalidateRect(hwnd, NULL, FALSE);
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
			pcli->pfnInitAutoRebuild(hwnd);
	}
	return 0;

	case CLM_SETTEXTCOLOR:
		if (wParam > FONTID_MODERN_MAX) break;

		dat->fontModernInfo[wParam].colour = lParam;
		dat->force_in_dialog = TRUE;
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
			dat->fontModernInfo[FONTID_PHONE].colour = lParam;
			dat->fontModernInfo[FONTID_LUNCH].colour = lParam;
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
			if (!pcli->pfnFindItem(hwnd, dat, lParam, &contact, &group, NULL))
				return NULL;
			i = List_IndexOf((SortedList*)&group->cl, contact);
			if (i < 0) return 0;
		}
		switch (wParam) {
		case CLGN_ROOT:
			if (dat->list.cl.count)
				return (LRESULT)pcli->pfnContactToHItem(dat->list.cl.items[0]);
			else
				return NULL;
		case CLGN_CHILD:
			if (contact->type != CLCIT_GROUP)
				return NULL;
			group = contact->group;
			if (group->cl.count == 0)
				return NULL;
			return (LRESULT)pcli->pfnContactToHItem(group->cl.items[0]);
		case CLGN_PARENT:
			return group->groupId | HCONTACT_ISGROUP;
		case CLGN_NEXT:
			do {
				if (++i >= group->cl.count)
					return NULL;
			} while (group->cl.items[i]->type == CLCIT_DIVIDER);
			return (LRESULT)pcli->pfnContactToHItem(group->cl.items[i]);
		case CLGN_PREVIOUS:
			do {
				if (--i < 0)
					return NULL;
			} while (group->cl.items[i]->type == CLCIT_DIVIDER);
			return (LRESULT)pcli->pfnContactToHItem(group->cl.items[i]);
		case CLGN_NEXTCONTACT:
			for (i++; i < group->cl.count; i++)
				if (group->cl.items[i]->type == CLCIT_CONTACT)
					break;
			if (i >= group->cl.count)
				return NULL;
			return (LRESULT)pcli->pfnContactToHItem(group->cl.items[i]);
		case CLGN_PREVIOUSCONTACT:
			if (i >= group->cl.count)
				return NULL;
			for (i--; i >= 0; i--)
				if (group->cl.items[i]->type == CLCIT_CONTACT)
					break;
			if (i < 0)
				return NULL;
			return (LRESULT)pcli->pfnContactToHItem(group->cl.items[i]);
		case CLGN_NEXTGROUP:
			for (i++; i < group->cl.count; i++)
				if (group->cl.items[i]->type == CLCIT_GROUP)
					break;
			if (i >= group->cl.count)
				return NULL;
			return (LRESULT)pcli->pfnContactToHItem(group->cl.items[i]);
		case CLGN_PREVIOUSGROUP:
			if (i >= group->cl.count)
				return NULL;
			for (i--; i >= 0; i--)
				if (group->cl.items[i]->type == CLCIT_GROUP)
					break;
			if (i < 0)
				return NULL;
			return (LRESULT)pcli->pfnContactToHItem(group->cl.items[i]);
		}
		return NULL;
	}
	case CLM_SELECTITEM:
	{
		ClcGroup *tgroup;
		int index = -1;
		int mainindex = -1;
		if (!pcli->pfnFindItem(hwnd, dat, wParam, &contact, &group, NULL))
			break;
		for (tgroup = group; tgroup; tgroup = tgroup->parent)
			pcli->pfnSetGroupExpand(hwnd, dat, tgroup, 1);

		if (!contact->isSubcontact) {
			index = List_IndexOf((SortedList*)&group->cl, contact);
			mainindex = index;
		}
		else {
			index = List_IndexOf((SortedList*)&group->cl, contact->subcontacts);
			mainindex = index;
			index += contact->isSubcontact;
		}

		BYTE k = db_get_b(NULL, "CLC", "MetaExpanding", SETTING_METAEXPANDING_DEFAULT);
		if (k) {
			for (int i = 0; i < mainindex; i++)
			{
				ClcContact *tempCont = group->cl.items[i];
				if (tempCont->type == CLCIT_CONTACT && tempCont->SubAllocated && tempCont->SubExpanded)
					index += tempCont->SubAllocated;
			}
		}

		dat->selection = pcli->pfnGetRowsPriorTo(&dat->list, group, index);
		pcli->pfnEnsureVisible(hwnd, dat, dat->selection, 0);
	}
	return 0;

	case CLM_SETEXTRAIMAGE:
		if (LOWORD(lParam) >= dat->extraColumnsCount)
			return 0;

		if (!pcli->pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
			return 0;

		contact->iExtraImage[LOWORD(lParam)] = HIWORD(lParam);
		pcli->pfnInvalidateRect(hwnd, NULL, FALSE);
		return 0;
	}
	return corecli.pfnProcessExternalMessages(hwnd, dat, msg, wParam, lParam);
}
