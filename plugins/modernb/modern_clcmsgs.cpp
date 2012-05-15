/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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
#include "m_clc.h"
#include "hdr/modern_clc.h"
#include "hdr/modern_commonprototypes.h"


//processing of all the CLM_ messages incoming

LRESULT cli_ProcessExternalMessages(HWND hwnd,struct ClcData *dat,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) 
	{

	case CLM_DELETEITEM:
		{
			pcli->pfnDeleteItemFromTree(hwnd, (HANDLE) wParam);
			clcSetDelayTimer( TIMERID_DELAYEDRESORTCLC, hwnd, 1 ); //pcli->pfnSortCLC(hwnd, dat, 1);
			clcSetDelayTimer( TIMERID_RECALCSCROLLBAR,  hwnd, 2 ); //pcli->pfnRecalcScrollBar(hwnd, dat);
		}
		return 0;
	case CLM_AUTOREBUILD:
		if (dat->force_in_dialog)
		{
			pcli->pfnSaveStateAndRebuildList(hwnd, dat);
		}
		else
		{
			clcSetDelayTimer( TIMERID_REBUILDAFTER, hwnd );
			CLM_AUTOREBUILD_WAS_POSTED=FALSE;
		}
		return 0;

	case CLM_SETEXTRACOLUMNSSPACE:
		dat->extraColumnSpacing=(int)wParam;
		CLUI__cliInvalidateRect(hwnd,NULL,FALSE);
		return 0;

	case CLM_SETFONT:
		if(HIWORD(lParam)<0 || HIWORD(lParam)>FONTID_MODERN_MAX) return 0;

		dat->fontModernInfo[HIWORD(lParam)].hFont=(HFONT)wParam;
		dat->fontModernInfo[HIWORD(lParam)].changed=1;

		RowHeights_GetMaxRowHeight(dat, hwnd);

		if(LOWORD(lParam))
			CLUI__cliInvalidateRect(hwnd,NULL,FALSE);
		return 0;


	case CLM_SETHIDEEMPTYGROUPS:
		{
			BOOL old=((GetWindowLong(hwnd,GWL_STYLE)&CLS_HIDEEMPTYGROUPS)!=0);
			BOOL newval=old;
			if(wParam) SetWindowLong(hwnd,GWL_STYLE,GetWindowLong(hwnd,GWL_STYLE)|CLS_HIDEEMPTYGROUPS);
			else SetWindowLong(hwnd,GWL_STYLE,GetWindowLong(hwnd,GWL_STYLE)&~CLS_HIDEEMPTYGROUPS);
			newval=((GetWindowLong(hwnd,GWL_STYLE)&CLS_HIDEEMPTYGROUPS)!=0);
			if (newval!=old)
				SendMessage(hwnd,CLM_AUTOREBUILD,0,0);
		}
		return 0;
	
	case CLM_SETTEXTCOLOR:
		if(wParam<0 || wParam>FONTID_MODERN_MAX) break;

		dat->fontModernInfo[wParam].colour=lParam;
		dat->force_in_dialog=TRUE;
		// Issue 40: option knows nothing about moderns colors
		// others who know have to set colors from lowest to highest
		switch ( wParam )  
		{
			case FONTID_CONTACTS:
				dat->fontModernInfo[FONTID_SECONDLINE].colour=lParam;
				dat->fontModernInfo[FONTID_THIRDLINE].colour=lParam;
				dat->fontModernInfo[FONTID_AWAY].colour=lParam;
				dat->fontModernInfo[FONTID_DND].colour=lParam;
				dat->fontModernInfo[FONTID_NA].colour=lParam;
				dat->fontModernInfo[FONTID_OCCUPIED].colour=lParam;
				dat->fontModernInfo[FONTID_CHAT].colour=lParam;
				dat->fontModernInfo[FONTID_INVISIBLE].colour=lParam;
				dat->fontModernInfo[FONTID_PHONE].colour=lParam;
				dat->fontModernInfo[FONTID_LUNCH].colour=lParam;
				dat->fontModernInfo[FONTID_CONTACT_TIME].colour=lParam;
				break;
			case FONTID_OPENGROUPS:
				dat->fontModernInfo[FONTID_CLOSEDGROUPS].colour=lParam;				
				break;
			case FONTID_OPENGROUPCOUNTS:
				dat->fontModernInfo[FONTID_CLOSEDGROUPCOUNTS].colour=lParam;				
				break;
		}
		return 0;

	case CLM_GETNEXTITEM:
		{
			struct ClcContact *contact;
			struct ClcGroup *group;
			int i;

			if (wParam != CLGN_ROOT) {
				if (!pcli->pfnFindItem(hwnd, dat, (HANDLE) lParam, &contact, &group, NULL))
					return (LRESULT) (HANDLE) NULL;
				i = li.List_IndexOf((SortedList*)&group->cl,contact);
				if (i<0) return 0;
			}
			switch (wParam) 
			{
			case CLGN_ROOT:
				if (dat->list.cl.count)
					return (LRESULT) pcli->pfnContactToHItem(dat->list.cl.items[0]);
				else
					return (LRESULT) (HANDLE) NULL;
			case CLGN_CHILD:
				if (contact->type != CLCIT_GROUP)
					return (LRESULT) (HANDLE) NULL;
				group = contact->group;
				if (group->cl.count == 0)
					return (LRESULT) (HANDLE) NULL;
				return (LRESULT) pcli->pfnContactToHItem(group->cl.items[0]);
			case CLGN_PARENT:
				return group->groupId | HCONTACT_ISGROUP;
			case CLGN_NEXT:
                do {
			        if (++i >= group->cl.count)
				        return NULL;
                }
                while (group->cl.items[i]->type == CLCIT_DIVIDER);
				return (LRESULT) pcli->pfnContactToHItem(group->cl.items[i]);
			case CLGN_PREVIOUS:
                do {
			        if (--i < 0)
				        return NULL;
                }
                while (group->cl.items[i]->type == CLCIT_DIVIDER);
				return (LRESULT) pcli->pfnContactToHItem(group->cl.items[i]);
			case CLGN_NEXTCONTACT:
				for (i++; i < group->cl.count; i++)
					if (group->cl.items[i]->type == CLCIT_CONTACT)
						break;
				if (i >= group->cl.count)
					return (LRESULT) (HANDLE) NULL;
				return (LRESULT) pcli->pfnContactToHItem(group->cl.items[i]);
			case CLGN_PREVIOUSCONTACT:
				if (i >= group->cl.count)
					return (LRESULT) (HANDLE) NULL;
				for (i--; i >= 0; i--)
					if (group->cl.items[i]->type == CLCIT_CONTACT)
						break;
				if (i < 0)
					return (LRESULT) (HANDLE) NULL;
				return (LRESULT) pcli->pfnContactToHItem(group->cl.items[i]);
			case CLGN_NEXTGROUP:
				for (i++; i < group->cl.count; i++)
					if (group->cl.items[i]->type == CLCIT_GROUP)
						break;
				if (i >= group->cl.count)
					return (LRESULT) (HANDLE) NULL;
				return (LRESULT) pcli->pfnContactToHItem(group->cl.items[i]);
			case CLGN_PREVIOUSGROUP:
				if (i >= group->cl.count)
					return (LRESULT) (HANDLE) NULL;
				for (i--; i >= 0; i--)
					if (group->cl.items[i]->type == CLCIT_GROUP)
						break;
				if (i < 0)
					return (LRESULT) (HANDLE) NULL;
				return (LRESULT) pcli->pfnContactToHItem(group->cl.items[i]);
			}
			return (LRESULT) (HANDLE) NULL;
		}
		return 0;
	case CLM_SELECTITEM:
		{
			struct ClcContact *contact;
			struct ClcGroup *group, *tgroup;
			int index=-1;
			int mainindex=-1;
			if (!pcli->pfnFindItem(hwnd, dat, (HANDLE) wParam, &contact, &group, NULL))
				break;
			for (tgroup = group; tgroup; tgroup = tgroup->parent)
				pcli->pfnSetGroupExpand(hwnd, dat, tgroup, 1);

			if (!contact->isSubcontact)
			{
				index=li.List_IndexOf((SortedList*)&group->cl,contact);
				mainindex=index;
			}
			else
			{
				index=li.List_IndexOf((SortedList*)&group->cl,contact->subcontacts);
				mainindex=index;
				index+=contact->isSubcontact;				
			}			
			{	
				BYTE k=ModernGetSettingByte(NULL,"CLC","MetaExpanding",SETTING_METAEXPANDING_DEFAULT);
				if (k)
				{
					int subcontactscount=0;
					int i;
					for ( i=0; i<mainindex; i++)
					{
						struct ClcContact *tempCont=group->cl.items[i];
						if (tempCont->type==CLCIT_CONTACT && tempCont->SubAllocated && tempCont->SubExpanded)
							index+=tempCont->SubAllocated;				
					}
				}					
			}
			dat->selection = pcli->pfnGetRowsPriorTo(&dat->list, group, index);
			pcli->pfnEnsureVisible(hwnd, dat, dat->selection, 0);			
		}
		return 0;

	case CLM_SETEXTRAIMAGE:
		{
			struct ClcContact *contact;
			if (LOWORD(lParam) >= dat->extraColumnsCount)
				return 0;
			if (!pcli->pfnFindItem(hwnd, dat, (HANDLE) wParam, &contact, NULL, NULL))
				return 0;
			contact->iExtraImage[LOWORD(lParam)] = (BYTE)  HIWORD(lParam); //set oldstyle icon
			contact->iWideExtraImage[LOWORD(lParam)] = (WORD) 0xFFFF; //reset wide icon
			pcli->pfnInvalidateRect(hwnd, NULL, FALSE);
			return 0;
		}

	case CLM_SETWIDEEXTRAIMAGE:
		{
				struct ClcContact *contact;
				if (LOWORD(lParam) >= dat->extraColumnsCount)
					return 0;
				if (!pcli->pfnFindItem(hwnd, dat, (HANDLE) wParam, &contact, NULL, NULL))
					return 0;
				contact->iExtraImage[LOWORD(lParam)] = (BYTE) 0xFF; //reset oldstyle icon
				contact->iWideExtraImage[LOWORD(lParam)] = (WORD) HIWORD(lParam); //set wide icon
				pcli->pfnInvalidateRect(hwnd, NULL, FALSE);
				return 0;
			}

	case CLM_SETEXTRAIMAGELIST:
		{
			dat->himlExtraColumns = (HIMAGELIST) lParam;
			dat->himlWideExtraColumns = (HIMAGELIST) wParam;
			pcli->pfnInvalidateRect(hwnd, NULL, FALSE);
			return 0;
		}

	case CLM_GETWIDEEXTRAIMAGE:
		{
			struct ClcContact *contact;
			if (LOWORD(lParam) >= dat->extraColumnsCount)
				return 0xFFFF;
			if (!pcli->pfnFindItem(hwnd, dat, (HANDLE) wParam, &contact, NULL, NULL))
				return 0xFFFF;
			return contact->iWideExtraImage[LOWORD(lParam)];
		}
	}
	return corecli.pfnProcessExternalMessages(hwnd, dat, msg, wParam, lParam);
}
