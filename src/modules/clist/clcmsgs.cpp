/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
#include "clc.h"

//processing of all the CLM_ messages incoming

LRESULT fnProcessExternalMessages(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case CLM_ADDCONTACT:
		cli.pfnAddContactToTree(hwnd, dat, wParam, 1, 0);
		cli.pfnRecalcScrollBar(hwnd, dat);
		cli.pfnSortCLC(hwnd, dat, 1);
		break;

	case CLM_ADDGROUP:
	{
		DWORD groupFlags;
		TCHAR *szName = cli.pfnGetGroupName(wParam, &groupFlags);
		if (szName == NULL)
			break;
		cli.pfnAddGroup(hwnd, dat, szName, groupFlags, wParam, 0);
		cli.pfnRecalcScrollBar(hwnd, dat);
		break;
	}

	case CLM_ADDINFOITEMA:
	case CLM_ADDINFOITEMW:
	{
		int i;
		ClcContact *groupContact;
		ClcGroup *group;
		CLCINFOITEM *cii = (CLCINFOITEM *)lParam;
		if (cii == NULL || cii->cbSize != sizeof(CLCINFOITEM))
			return NULL;
		if (cii->hParentGroup == NULL)
			group = &dat->list;
		else {
			if (!cli.pfnFindItem(hwnd, dat, int(cii->hParentGroup) | HCONTACT_ISGROUP, &groupContact, NULL, NULL))
				return NULL;
			group = groupContact->group;
		}
		if (msg == CLM_ADDINFOITEMA)
		{	WCHAR* wszText = mir_a2u((char*)cii->pszText);
			i = cli.pfnAddInfoItemToGroup(group, cii->flags, wszText);
			mir_free(wszText);
		}
		else i = cli.pfnAddInfoItemToGroup(group, cii->flags, cii->pszText);
		cli.pfnRecalcScrollBar(hwnd, dat);
		return (LRESULT)group->cl.items[i]->hContact | HCONTACT_ISINFO;
	}

	case CLM_AUTOREBUILD:
		KillTimer(hwnd, TIMERID_REBUILDAFTER);
		cli.pfnSaveStateAndRebuildList(hwnd, dat);
		cli.bAutoRebuild = false;
		break;

	case CLM_DELETEITEM:
		cli.pfnDeleteItemFromTree(hwnd, wParam);
		cli.pfnSortCLC(hwnd, dat, 1);
		cli.pfnRecalcScrollBar(hwnd, dat);
		break;

	case CLM_EDITLABEL:
		SendMessage(hwnd, CLM_SELECTITEM, wParam, 0);
		cli.pfnBeginRenameSelection(hwnd, dat);
		break;

	case CLM_ENDEDITLABELNOW:
		cli.pfnEndRename(hwnd, dat, wParam);
		break;

	case CLM_ENSUREVISIBLE:
	{
		ClcContact *contact;
		ClcGroup *group, *tgroup;
		if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, &group, NULL))
			break;
		for (tgroup = group; tgroup; tgroup = tgroup->parent)
			cli.pfnSetGroupExpand(hwnd, dat, tgroup, 1);
		cli.pfnEnsureVisible(hwnd, dat, cli.pfnGetRowsPriorTo(&dat->list, group, List_IndexOf((SortedList*)&group->cl, contact)), 0);
		break;
	}

	case CLM_EXPAND:
	{
		ClcContact *contact;
		if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
			break;
		if (contact->type != CLCIT_GROUP)
			break;
		cli.pfnSetGroupExpand(hwnd, dat, contact->group, lParam);
		break;
	}

	case CLM_FINDCONTACT:
		if (!cli.pfnFindItem(hwnd, dat, wParam, NULL, NULL, NULL))
			return NULL;
		return wParam;

	case CLM_FINDGROUP:
		if (!cli.pfnFindItem(hwnd, dat, wParam | HCONTACT_ISGROUP, NULL, NULL, NULL))
			return NULL;
		return wParam | HCONTACT_ISGROUP;

	case CLM_GETBKCOLOR:
		return dat->bkColour;

	case CLM_GETCHECKMARK:
	{
		ClcContact *contact;
		if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
			return 0;
		return (contact->flags & CONTACTF_CHECKED) != 0;
	}

	case CLM_GETCOUNT:
		return cli.pfnGetGroupContentsCount(&dat->list, 0);

	case CLM_GETEDITCONTROL:
		return (LRESULT)dat->hwndRenameEdit;

	case CLM_GETEXPAND:
	{
		ClcContact *contact;
		if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
			return CLE_INVALID;
		if (contact->type != CLCIT_GROUP)
			return CLE_INVALID;
		return contact->group->expanded;
	}

	case CLM_SETEXTRASPACE:
		dat->extraColumnSpacing = (int)wParam;
		cli.pfnInvalidateRect(hwnd,NULL,FALSE);
		return 0;

	case CLM_GETEXTRACOLUMNS:
		return dat->extraColumnsCount;

	case CLM_GETEXTRAIMAGE:
		if (LOWORD(lParam) < dat->extraColumnsCount) {
			ClcContact *contact;
			if (cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
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
		return db_get_b(NULL, "CLC", "HideOfflineRoot", 0);

	case CLM_GETINDENT:
		return dat->groupIndent;

	case CLM_GETISEARCHSTRING:
		mir_tstrcpy((TCHAR*) lParam, dat->szQuickSearch);
		return mir_tstrlen(dat->szQuickSearch);

	case CLM_GETITEMTEXT:
	{
		ClcContact *contact;
		if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
			return 0;
		mir_tstrcpy((TCHAR*) lParam, contact->szText);
		return mir_tstrlen(contact->szText);
	}

	case CLM_GETITEMTYPE:
	{
		ClcContact *contact;
		if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
			return CLCIT_INVALID;
		return contact->type;
	}

	case CLM_GETLEFTMARGIN:
		return dat->leftMargin;

	case CLM_GETNEXTITEM:
	{
		if (wParam == CLGN_ROOT) {
			if (dat->list.cl.count)
				return (LRESULT)cli.pfnContactToHItem(dat->list.cl.items[0]);
			return NULL;
		}

		ClcContact *contact;
		ClcGroup *group;
		if (!cli.pfnFindItem(hwnd, dat, lParam, &contact, &group, NULL))
			return NULL;

		int i = List_IndexOf((SortedList*)&group->cl, contact);
		switch (wParam) {
		case CLGN_CHILD:
			if (contact->type != CLCIT_GROUP)
				return NULL;
			group = contact->group;
			if (group->cl.count == 0)
				return NULL;
			return (LRESULT)cli.pfnContactToHItem(group->cl.items[0]);

		case CLGN_PARENT:
			return group->groupId | HCONTACT_ISGROUP;

		case CLGN_NEXT:
			do {
				if (++i >= group->cl.count)
					return NULL;
			}
				while (group->cl.items[i]->type == CLCIT_DIVIDER);
			return (LRESULT)cli.pfnContactToHItem(group->cl.items[i]);

		case CLGN_PREVIOUS:
			do {
				if (--i < 0)
					return NULL;
			}
				while (group->cl.items[i]->type == CLCIT_DIVIDER);
			return (LRESULT)cli.pfnContactToHItem(group->cl.items[i]);

		case CLGN_NEXTCONTACT:
			for (i++; i < group->cl.count; i++)
				if (group->cl.items[i]->type == CLCIT_CONTACT)
					break;
			if (i >= group->cl.count)
				return NULL;
			return (LRESULT)cli.pfnContactToHItem(group->cl.items[i]);

		case CLGN_PREVIOUSCONTACT:
			if (i >= group->cl.count)
				return NULL;
			for (i--; i >= 0; i--)
				if (group->cl.items[i]->type == CLCIT_CONTACT)
					break;
			if (i < 0)
				return NULL;
			return (LRESULT)cli.pfnContactToHItem(group->cl.items[i]);

		case CLGN_NEXTGROUP:
			for (i++; i < group->cl.count; i++)
				if (group->cl.items[i]->type == CLCIT_GROUP)
					break;
			if (i >= group->cl.count)
				return NULL;
			return (LRESULT)cli.pfnContactToHItem(group->cl.items[i]);

		case CLGN_PREVIOUSGROUP:
			if (i >= group->cl.count)
				return NULL;
			for (i--; i >= 0; i--)
				if (group->cl.items[i]->type == CLCIT_GROUP)
					break;
			if (i < 0)
				return NULL;
			return (LRESULT)cli.pfnContactToHItem(group->cl.items[i]);
		}
		return NULL;
	}

	case CLM_GETSCROLLTIME:
		return dat->scrollTime;

	case CLM_GETSELECTION:
	{
		ClcContact *contact;
		if (cli.pfnGetRowByIndex(dat, dat->selection, &contact, NULL) == -1)
			return NULL;
		return (LRESULT)cli.pfnContactToHItem(contact);
	}

	case CLM_GETTEXTCOLOR:
		if (wParam > FONTID_MAX)
			return 0;
		return (LRESULT)dat->fontInfo[wParam].colour;

	case CLM_HITTEST:
	{
		ClcContact *contact;
		DWORD hitFlags;
		int hit = cli.pfnHitTest(hwnd, dat, (short) LOWORD(lParam), (short) HIWORD(lParam), &contact, NULL, &hitFlags);
		if (wParam)
			*(PDWORD) wParam = hitFlags;
		if (hit == -1)
			return NULL;
		return (LRESULT)cli.pfnContactToHItem(contact);
	}

	case CLM_SELECTITEM:
	{
		ClcContact *contact;
		ClcGroup *group, *tgroup;
		if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, &group, NULL))
			break;
		for (tgroup = group; tgroup; tgroup = tgroup->parent)
			cli.pfnSetGroupExpand(hwnd, dat, tgroup, 1);
		dat->selection = cli.pfnGetRowsPriorTo(&dat->list, group, List_IndexOf((SortedList*)&group->cl, contact));
		cli.pfnEnsureVisible(hwnd, dat, dat->selection, 0);
		break;
	}

	case CLM_SETBKBITMAP:
		if (dat->hBmpBackground) {
			DeleteObject(dat->hBmpBackground);
			dat->hBmpBackground = NULL;
		}
		dat->hBmpBackground = (HBITMAP)lParam;
		dat->backgroundBmpUse = wParam;
		dat->bkChanged = 1;
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		break;

	case CLM_SETBKCOLOR:
		if (dat->hBmpBackground) {
			DeleteObject(dat->hBmpBackground);
			dat->hBmpBackground = NULL;
		}
		dat->bkColour = wParam;
		dat->bkChanged = 1;
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		break;

	case CLM_SETCHECKMARK:
	{
		ClcContact *contact;
		if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
			return 0;
		if (lParam)
			contact->flags |= CONTACTF_CHECKED;
		else
			contact->flags &= ~CONTACTF_CHECKED;
		cli.pfnRecalculateGroupCheckboxes(hwnd, dat);
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		break;
	}

	case CLM_SETEXTRACOLUMNS:
		if (wParam > EXTRA_ICON_COUNT)
			return 0;

		dat->extraColumnsCount = wParam;
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		break;

	case CLM_SETEXTRAIMAGE:
		if ( LOWORD(lParam) < dat->extraColumnsCount) {
			ClcContact *contact;
			if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
				return 0;

			contact->iExtraImage[LOWORD(lParam)] = HIWORD(lParam);
			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		}
		break;

	case CLM_SETEXTRAIMAGELIST:
		dat->himlExtraColumns = (HIMAGELIST) lParam;
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		break;

	case CLM_SETFONT:
		if (HIWORD(lParam) > FONTID_MAX)
			return 0;

		dat->fontInfo[HIWORD(lParam)].hFont = (HFONT) wParam;
		dat->fontInfo[HIWORD(lParam)].changed = 1;
		{
			SIZE fontSize;
			HDC hdc = GetDC(hwnd);
			SelectObject(hdc, (HFONT) wParam);
			GetTextExtentPoint32A(hdc, "x", 1, &fontSize);
			dat->fontInfo[HIWORD(lParam)].fontHeight = fontSize.cy;
			if (dat->rowHeight < fontSize.cy + 2)
				dat->rowHeight = fontSize.cy + 2;
			ReleaseDC(hwnd, hdc);
		}
		if (LOWORD(lParam))
			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		break;

	case CLM_SETGREYOUTFLAGS:
		dat->greyoutFlags = wParam;
		break;

	case CLM_SETHIDEEMPTYGROUPS:
		if (wParam)
			SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | CLS_HIDEEMPTYGROUPS);
		else
			SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) & ~CLS_HIDEEMPTYGROUPS);
		cli.pfnInitAutoRebuild(hwnd);
		break;

	case CLM_SETHIDEOFFLINEROOT:
		db_set_b(NULL, "CLC", "HideOfflineRoot", (BYTE) wParam);
		cli.pfnInitAutoRebuild(hwnd);
		break;

	case CLM_SETINDENT:
		dat->groupIndent = wParam;
		cli.pfnInitAutoRebuild(hwnd);
		break;

	case CLM_SETITEMTEXT:
	{
		ClcContact *contact;
		if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
			break;
		mir_tstrncpy(contact->szText, (TCHAR*)lParam, SIZEOF(contact->szText));
		cli.pfnSortCLC(hwnd, dat, 1);
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		break;
	}

	case CLM_SETLEFTMARGIN:
		dat->leftMargin = wParam;
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		break;

	case CLM_SETOFFLINEMODES:
		dat->offlineModes = wParam;
		cli.pfnInitAutoRebuild(hwnd);
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
		cli.pfnInitAutoRebuild(hwnd);
		break;
	}
	return 0;
}
