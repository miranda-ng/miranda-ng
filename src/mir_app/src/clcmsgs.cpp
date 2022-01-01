/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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
#include "clc.h"

// processing of all the CLM_ messages incoming

LRESULT fnProcessExternalMessages(HWND hwnd, ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ClcContact *contact;
	ClcGroup *group;
	int i;

	switch (msg) {
	case CLM_ADDCONTACT:
		g_clistApi.pfnAddContactToTree(hwnd, dat, wParam, 1, 0);
		g_clistApi.pfnRecalcScrollBar(hwnd, dat);
		g_clistApi.pfnSortCLC(hwnd, dat, 1);
		break;

	case CLM_ADDGROUP:
		{
			uint32_t groupFlags;
			wchar_t *szName = Clist_GroupGetName(wParam, &groupFlags);
			if (szName == nullptr)
				break;
			g_clistApi.pfnAddGroup(hwnd, dat, szName, groupFlags, wParam, 0);
			g_clistApi.pfnRecalcScrollBar(hwnd, dat);
		}
		break;

	case CLM_ADDINFOITEMA:
	case CLM_ADDINFOITEMW:
		{
			CLCINFOITEM *cii = (CLCINFOITEM *)lParam;
			if (cii == nullptr || cii->cbSize != sizeof(CLCINFOITEM))
				return 0;
			if (cii->hParentGroup == nullptr)
				group = &dat->list;
			else {
				if (!Clist_FindItem(hwnd, dat, INT_PTR(cii->hParentGroup) | HCONTACT_ISGROUP, &contact))
					return 0;
				group = contact->group;
			}
			
			ClcContact *cc;
			if (msg == CLM_ADDINFOITEMA) {
				wchar_t* wszText = mir_a2u((char*)cii->pszText);
				cc = g_clistApi.pfnAddInfoItemToGroup(group, cii->flags, wszText);
				mir_free(wszText);
			}
			else cc = g_clistApi.pfnAddInfoItemToGroup(group, cii->flags, cii->pszText);
			g_clistApi.pfnRecalcScrollBar(hwnd, dat);
			return (LRESULT)cc->hContact | HCONTACT_ISINFO;
		}

	case CLM_AUTOREBUILD:
		SetTimer(hwnd, TIMERID_REBUILDAFTER, 50, nullptr);
		break;

	case CLM_DELETEITEM:
		Clist_DeleteItemFromTree(hwnd, wParam);
		g_clistApi.pfnSortCLC(hwnd, dat, 1);
		g_clistApi.pfnRecalcScrollBar(hwnd, dat);
		break;

	case CLM_EDITLABEL:
		SendMessage(hwnd, CLM_SELECTITEM, wParam, 0);
		g_clistApi.pfnBeginRenameSelection(hwnd, dat);
		break;

	case CLM_ENDEDITLABELNOW:
		Clist_EndRename(dat, wParam);
		break;

	case CLM_ENSUREVISIBLE:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact, &group))
			break;

		for (ClcGroup *tgroup = group; tgroup; tgroup = tgroup->parent)
			g_clistApi.pfnSetGroupExpand(hwnd, dat, tgroup, 1);
		Clist_EnsureVisible(hwnd, dat, g_clistApi.pfnGetRowsPriorTo(&dat->list, group, group->cl.indexOf(contact)), 0);
		break;

	case CLM_EXPAND:
		if (Clist_FindItem(hwnd, dat, wParam, &contact))
			if (contact->type == CLCIT_GROUP)
				g_clistApi.pfnSetGroupExpand(hwnd, dat, contact->group, lParam);
		break;

	case CLM_FINDCONTACT:
		if (!Clist_FindItem(hwnd, dat, wParam, nullptr))
			return 0;
		return wParam;

	case CLM_FINDGROUP:
		if (!Clist_FindItem(hwnd, dat, wParam | HCONTACT_ISGROUP, nullptr))
			return 0;
		return wParam | HCONTACT_ISGROUP;

	case CLM_GETBKCOLOR:
		return dat->bkColour;

	case CLM_GETCHECKMARK:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			return 0;
		return (contact->flags & CONTACTF_CHECKED) != 0;

	case CLM_GETCOUNT:
		return g_clistApi.pfnGetGroupContentsCount(&dat->list, 0);

	case CLM_GETEDITCONTROL:
		return (LRESULT)dat->hwndRenameEdit;

	case CLM_GETEXPAND:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			return CLE_INVALID;
		if (contact->type != CLCIT_GROUP)
			return CLE_INVALID;
		return contact->group->expanded;

	case CLM_SETEXTRASPACE:
		dat->extraColumnSpacing = (int)wParam;
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		return 0;

	case CLM_GETEXTRACOLUMNS:
		return dat->extraColumnsCount;

	case CLM_GETEXTRAIMAGE:
		if (LOWORD(lParam) < dat->extraColumnsCount) {
			if (Clist_FindItem(hwnd, dat, wParam, &contact))
				return contact->iExtraImage[LOWORD(lParam)];
		}
		return EMPTY_EXTRA_ICON;

	case CLM_GETEXTRAIMAGELIST:
		return (LRESULT)dat->himlExtraColumns;

	case CLM_GETFONT:
		if (wParam > FONTID_MAX)
			return 0;
		return (LRESULT)dat->fontInfo[wParam].hFont;

	case CLM_GETHIDEOFFLINEROOT:
		return db_get_b(0, "CLC", "HideOfflineRoot", 0);

	case CLM_GETINDENT:
		return dat->groupIndent;

	case CLM_GETISEARCHSTRING:
		mir_wstrcpy((wchar_t*)lParam, dat->szQuickSearch);
		return mir_wstrlen(dat->szQuickSearch);

	case CLM_GETITEMTEXT:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			return 0;
		mir_wstrcpy((wchar_t*)lParam, contact->szText);
		return mir_wstrlen(contact->szText);

	case CLM_GETITEMTYPE:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			return CLCIT_INVALID;
		return contact->type;

	case CLM_GETEXSTYLE:
		return dat->exStyle;

	case CLM_SETEXSTYLE:
		dat->exStyle |= wParam;
		break;

	case CLM_GETNEXTITEM:
		if (wParam == CLGN_ROOT) {
			if (dat->list.cl.getCount())
				return Clist_ContactToHItem(dat->list.cl[0]);
			return 0;
		}

		if (!Clist_FindItem(hwnd, dat, lParam, &contact, &group))
			return 0;

		i = group->cl.indexOf(contact);
		switch (wParam) {
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
		return 0;

	case CLM_GETSCROLLTIME:
		return dat->scrollTime;

	case CLM_GETSELECTION:
		if (g_clistApi.pfnGetRowByIndex(dat, dat->selection, &contact, nullptr) == -1)
			return 0;
		return Clist_ContactToHItem(contact);

	case CLM_GETTEXTCOLOR:
		if (wParam > FONTID_MAX)
			return 0;
		return (LRESULT)dat->fontInfo[wParam].colour;

	case CLM_HITTEST:
		uint32_t hitFlags;
		{
			int hit = g_clistApi.pfnHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), &contact, nullptr, &hitFlags);
			if (wParam)
				*(PDWORD)wParam = hitFlags;
			if (hit == -1)
				return 0;
		}
		return Clist_ContactToHItem(contact);

	case CLM_SELECTITEM:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact, &group))
			break;

		for (ClcGroup *tgroup = group; tgroup; tgroup = tgroup->parent)
			g_clistApi.pfnSetGroupExpand(hwnd, dat, tgroup, 1);
		dat->selection = g_clistApi.pfnGetRowsPriorTo(&dat->list, group, group->cl.indexOf(contact));
		Clist_EnsureVisible(hwnd, dat, dat->selection, 0);
		break;

	case CLM_SETBKCOLOR:
		if (dat->hBmpBackground) {
			DeleteObject(dat->hBmpBackground);
			dat->hBmpBackground = nullptr;
		}
		dat->bkColour = wParam;
		dat->bkChanged = 1;
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case CLM_SETCHECKMARK:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			return 0;
		if (lParam)
			contact->flags |= CONTACTF_CHECKED;
		else
			contact->flags &= ~CONTACTF_CHECKED;
		Clist_RecalculateGroupCheckboxes(dat);
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case CLM_SETEXTRACOLUMNS:
		if (wParam > EXTRA_ICON_COUNT)
			return 0;

		dat->extraColumnsCount = wParam;
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case CLM_SETEXTRAIMAGE:
		if (LOWORD(lParam) < dat->extraColumnsCount) {
			int bVisible;
			if (Clist_FindItem(hwnd, dat, wParam, &contact, nullptr, &bVisible)) {
				contact->iExtraImage[LOWORD(lParam)] = HIWORD(lParam);
				if (bVisible)
					g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
			}
		}
		break;

	case CLM_SETEXTRAIMAGELIST:
		dat->himlExtraColumns = (HIMAGELIST)lParam;
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case CLM_SETFONT:
		if (HIWORD(lParam) > FONTID_MAX)
			return 0;

		dat->fontInfo[HIWORD(lParam)].hFont = (HFONT)wParam;
		dat->fontInfo[HIWORD(lParam)].changed = 1;
		{
			SIZE fontSize;
			HDC hdc = GetDC(hwnd);
			SelectObject(hdc, (HFONT)wParam);
			GetTextExtentPoint32A(hdc, "x", 1, &fontSize);
			dat->fontInfo[HIWORD(lParam)].fontHeight = fontSize.cy;
			if (dat->rowHeight < fontSize.cy + 2)
				dat->rowHeight = fontSize.cy + 2;
			ReleaseDC(hwnd, hdc);
		}
		if (LOWORD(lParam))
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case CLM_SETHIDEEMPTYGROUPS:
		if (wParam)
			SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | CLS_HIDEEMPTYGROUPS);
		else
			SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) & ~CLS_HIDEEMPTYGROUPS);
		Clist_InitAutoRebuild(hwnd);
		break;

	case CLM_SETHIDEOFFLINEROOT:
		db_set_b(0, "CLC", "HideOfflineRoot", (uint8_t)wParam);
		Clist_InitAutoRebuild(hwnd);
		break;

	case CLM_SETITEMTEXT:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			break;
		mir_wstrncpy(contact->szText, (wchar_t*)lParam, _countof(contact->szText));
		g_clistApi.pfnSortCLC(hwnd, dat, 1);
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case CLM_SETOFFLINEMODES:
		dat->offlineModes = wParam;
		Clist_InitAutoRebuild(hwnd);
		break;

	case CLM_SETSCROLLTIME:
		dat->scrollTime = wParam;
		break;

	case CLM_SETTEXTCOLOR:
		if (wParam > FONTID_MAX)
			break;
		dat->fontInfo[wParam].colour = lParam;
		break;

	case CLM_SETUSEGROUPS:
		if (wParam)
			SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | CLS_USEGROUPS);
		else
			SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) & ~CLS_USEGROUPS);
		Clist_InitAutoRebuild(hwnd);
		break;
	}
	return 0;
}
