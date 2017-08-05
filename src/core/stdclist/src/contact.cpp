/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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

int g_bSortByStatus, g_bSortByProto;

const struct
{
	int status, order;
}
static statusModeOrder[] = {
	{ ID_STATUS_OFFLINE, 500 },
	{ ID_STATUS_ONLINE, 10 },
	{ ID_STATUS_AWAY, 200 },
	{ ID_STATUS_DND, 110 },
	{ ID_STATUS_NA, 450 },
	{ ID_STATUS_OCCUPIED, 100 },
	{ ID_STATUS_FREECHAT, 0 },
	{ ID_STATUS_INVISIBLE, 20 },
	{ ID_STATUS_ONTHEPHONE, 150 },
	{ ID_STATUS_OUTTOLUNCH, 425 } };

int GetStatusModeOrdering(int statusMode)
{
	int i;
	for (i = 0; i < _countof(statusModeOrder); i++)
		if (statusModeOrder[i].status == statusMode)
			return statusModeOrder[i].order;
	return 1000;
}

int CompareContacts(const ClcContact* c1, const ClcContact* c2)
{
	MCONTACT a = c1->hContact, b = c2->hContact;
	wchar_t namea[128], *nameb;

	int statusa = db_get_w(a, c1->proto, "Status", ID_STATUS_OFFLINE);
	int statusb = db_get_w(b, c2->proto, "Status", ID_STATUS_OFFLINE);

	if (g_bSortByProto) {
		/* deal with statuses, online contacts have to go above offline */
		if ((statusa == ID_STATUS_OFFLINE) != (statusb == ID_STATUS_OFFLINE)) {
			return 2 * (statusa == ID_STATUS_OFFLINE) - 1;
		}
		/* both are online, now check protocols */
		if (c1->proto != NULL && c2->proto != NULL) {
			int rc = mir_strcmp(c1->proto, c2->proto);
			if (rc != 0)
				return rc;
		}
		/* protocols are the same, order by display name */
	}

	if (g_bSortByStatus) {
		int ordera = GetStatusModeOrdering(statusa);
		int orderb = GetStatusModeOrdering(statusb);
		if (ordera != orderb)
			return ordera - orderb;
	}
	else {
		//one is offline: offline goes below online
		if ((statusa == ID_STATUS_OFFLINE) != (statusb == ID_STATUS_OFFLINE))
			return 2 * (statusa == ID_STATUS_OFFLINE) - 1;
	}

	nameb = pcli->pfnGetContactDisplayName(a, 0);
	wcsncpy_s(namea, nameb, _TRUNCATE);
	namea[ _countof(namea)-1 ] = 0;
	nameb = pcli->pfnGetContactDisplayName(b, 0);

	//otherwise just compare names
	return mir_wstrcmpi(namea, nameb);
}
