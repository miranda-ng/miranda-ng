/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#include <commonheaders.h>

//processing of all the CLM_ messages incoming

LRESULT ProcessExternalMessages(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case CLM_SETSTICKY:
		{
			ClcContact *contact;
			ClcGroup *group;
			if (wParam == 0 || !FindItem(hwnd, dat, (HANDLE)wParam, &contact, &group, NULL))
				return 0;

			if (lParam)
				contact->flags |= CONTACTF_STICKY;
			else
				contact->flags &= ~CONTACTF_STICKY;
			break;
		}

	case CLM_GETSTATUSMSG:
		if (wParam) {
			ClcContact *contact = NULL;
			if (!FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
				return 0;

			if (contact->type != CLCIT_CONTACT)
				return 0;

			if (contact->pExtra)
				if (contact->pExtra->bStatusMsgValid != STATUSMSG_NOTFOUND)
					return((INT_PTR)contact->pExtra->statusMsg);
		}
		return 0;

	case CLM_SETHIDESUBCONTACTS:
		dat->bHideSubcontacts = (BOOL)lParam;
		return 0;

	case CLM_TOGGLEPRIORITYCONTACT:
		if (wParam) {
			ClcContact *contact = NULL;
			if (!FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
				return 0;
			if (contact->type != CLCIT_CONTACT)
				return 0;
			contact->flags ^= CONTACTF_PRIORITY;
			cfg::writeByte(contact->hContact, "CList", "Priority", (BYTE)(contact->flags & CONTACTF_PRIORITY ? 1 : 0));
			pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
		}
		return 0;

	case CLM_QUERYPRIORITYCONTACT:
		if (wParam) {
			ClcContact *contact = NULL;
			if (!FindItem(hwnd, dat, (HANDLE)wParam, &contact, NULL, NULL))
				return 0;
			if (contact->type != CLCIT_CONTACT)
				return 0;
			return(contact->flags & CONTACTF_PRIORITY ? 1 : 0);
		}
		return 0;

	case CLM_SETFONT:
		if (HIWORD(lParam)>FONTID_LAST)
			return 0;
		dat->fontInfo[HIWORD(lParam)].hFont = (HFONT)wParam;
		dat->fontInfo[HIWORD(lParam)].changed = 1;

		RowHeight::getMaxRowHeight(dat, hwnd);

		if (LOWORD(lParam))
			InvalidateRect(hwnd,NULL,FALSE);
		return 0;

	case CLM_ISMULTISELECT:
		return dat->isMultiSelect;
	}

	return coreCli.pfnProcessExternalMessages(hwnd, dat, msg, wParam, lParam);
}
