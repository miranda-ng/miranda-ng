/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of clist_nicer plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: clcmsgs.cpp 132 2010-09-29 07:49:20Z silvercircle $
 *
 */

#include <commonheaders.h>

//processing of all the CLM_ messages incoming

LRESULT CLC::ProcessExternalMessages(HWND hwnd, ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
        case CLM_SETSTICKY:
            {
                struct ClcContact *contact;
				struct ClcGroup *group;

                if (wParam == 0 || !findItem(hwnd, dat, (HANDLE) wParam, &contact, &group, NULL))
                    return 0;
				if (lParam)
                    contact->flags |= CONTACTF_STICKY;
				else
                    contact->flags &= ~CONTACTF_STICKY;
                break;
            }

	case CLM_SETEXTRAIMAGEINT:
		{
			struct ClcContact *contact = NULL;
			int index = -1;

			if (LOWORD(lParam) >= MAXEXTRACOLUMNS || wParam == 0)
				return 0;

			if (!findItem(hwnd, dat, (HANDLE) wParam, &contact, NULL, NULL))
				return 0;

			index = contact->extraCacheEntry;

			if(contact->type != CLCIT_CONTACT) // || contact->bIsMeta)
				return 0;

			if(index >= 0 && index < cfg::nextCacheEntry) {
				cfg::eCache[index].iExtraImage[LOWORD(lParam)] = (BYTE)HIWORD(lParam);
				cfg::eCache[index].iExtraValid = cfg::eCache[index].iExtraImage[LOWORD(lParam)] != (BYTE)0xff ? (cfg::eCache[index].iExtraValid | (1 << LOWORD(lParam))) : (cfg::eCache[index].iExtraValid & ~(1 << LOWORD(lParam)));
				PostMessage(hwnd, INTM_INVALIDATE, 0, (LPARAM)(contact ? contact->hContact : 0));
			}
		}
		return 0;
	case CLM_SETEXTRAIMAGEINTMETA:
		{
			MCONTACT hMasterContact = 0;
			int index = -1;

			if (LOWORD(lParam) >= MAXEXTRACOLUMNS)
				return 0;

            index = cfg::getCache(wParam, NULL);
            if(index >= 0 && index < cfg::nextCacheEntry) {
                cfg::eCache[index].iExtraImage[LOWORD(lParam)] = (BYTE)HIWORD(lParam);
                cfg::eCache[index].iExtraValid = cfg::eCache[index].iExtraImage[LOWORD(lParam)] != (BYTE)0xff ? (cfg::eCache[index].iExtraValid | (1 << LOWORD(lParam))) : (cfg::eCache[index].iExtraValid & ~(1 << LOWORD(lParam)));
            }

			hMasterContact = cfg::getDword(wParam, cfg::dat.szMetaName, "Handle", 0);

			index = cfg::getCache(hMasterContact, NULL);
			if(index >= 0 && index < cfg::nextCacheEntry) {
				cfg::eCache[index].iExtraImage[LOWORD(lParam)] = (BYTE)HIWORD(lParam);
				cfg::eCache[index].iExtraValid = cfg::eCache[index].iExtraImage[LOWORD(lParam)] != (BYTE)0xff ? (cfg::eCache[index].iExtraValid | (1 << LOWORD(lParam))) : (cfg::eCache[index].iExtraValid & ~(1 << LOWORD(lParam)));
				PostMessage(hwnd, INTM_INVALIDATE, 0, 0);
            }
        }
		return 0;

	case CLM_GETSTATUSMSG:
		{
			ClcContact *contact = NULL;

			if (wParam == 0)
				return 0;

			if (!findItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
				return 0;
			if(contact->type != CLCIT_CONTACT)
				return 0;
			if(contact->extraCacheEntry >= 0 && contact->extraCacheEntry <= cfg::nextCacheEntry) {
				if(cfg::eCache[contact->extraCacheEntry].bStatusMsgValid != STATUSMSG_NOTFOUND)
					return((INT_PTR)cfg::eCache[contact->extraCacheEntry].statusMsg);
		}	}
		return 0;

	case CLM_SETHIDESUBCONTACTS:
		dat->bHideSubcontacts = (BOOL)lParam;
		return 0;

	case CLM_TOGGLEPRIORITYCONTACT:
		{
			struct ClcContact *contact = NULL;

			if (wParam == 0)
				return 0;

			if (!findItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
				return 0;
			if(contact->type != CLCIT_CONTACT)
				return 0;
			contact->flags ^= CONTACTF_PRIORITY;
			cfg::writeByte(contact->hContact, "CList", "Priority", (BYTE)(contact->flags & CONTACTF_PRIORITY ? 1 : 0));
			pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
			return 0;
		}
	case CLM_QUERYPRIORITYCONTACT:
		{
			struct ClcContact *contact = NULL;

			if (wParam == 0)
				return 0;

			if (!findItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
				return 0;
			if(contact->type != CLCIT_CONTACT)
				return 0;
			return(contact->flags & CONTACTF_PRIORITY ? 1 : 0);
		}
	case CLM_TOGGLEFLOATINGCONTACT:
		{
			struct ClcContact *contact = NULL;
			BYTE state;
			int iEntry;

			if (wParam == 0)
				return 0;

			if (!findItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
				return 0;
			if(contact->type != CLCIT_CONTACT)
				return 0;

			iEntry = contact->extraCacheEntry;

			if(iEntry >= 0 && iEntry <= cfg::nextCacheEntry) {
				state = !cfg::getByte(contact->hContact, "CList", "floating", 0);
				if(state) {
					if(cfg::eCache[iEntry].floater == NULL)
						FLT_Create(iEntry);
					ShowWindow(cfg::eCache[contact->extraCacheEntry].floater->hwnd, SW_SHOW);
				}
				else {
					if(cfg::eCache[iEntry].floater && cfg::eCache[iEntry].floater->hwnd) {
						DestroyWindow(cfg::eCache[iEntry].floater->hwnd);
						cfg::eCache[iEntry].floater = 0;
					}
				}
				cfg::writeByte(contact->hContact, "CList", "floating", state);
			}
			return 0;
		}
	case CLM_QUERYFLOATINGCONTACT:
		{
			return(cfg::getByte(wParam, "CList", "floating", 0));
		}
	case CLM_SETEXTRAIMAGELIST:
		dat->himlExtraColumns = (HIMAGELIST) lParam;
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;

	case CLM_SETFONT:
		if(HIWORD(lParam)<0 || HIWORD(lParam)>FONTID_LAST)
			return 0;
		dat->fontInfo[HIWORD(lParam)].hFont = (HFONT)wParam;
		dat->fontInfo[HIWORD(lParam)].changed = 1;

		RowHeight::getMaxRowHeight(dat, hwnd);

		if(LOWORD(lParam))
			InvalidateRect(hwnd,NULL,FALSE);
		return 0;

	case CLM_ISMULTISELECT:
		return dat->isMultiSelect;
	}

	return coreCli.pfnProcessExternalMessages(hwnd, dat, msg, wParam, lParam);
}
