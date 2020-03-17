// Copyright © 2010-20 SecureIM developers (baloo and others), sss
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "stdafx.h"

void setSrmmIcon(MCONTACT hContact);

int __cdecl onWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *mwd = (MessageWindowEventData *)lParam;
	if (mwd->uType == MSG_WINDOW_EVT_OPEN || mwd->uType == MSG_WINDOW_EVT_OPENING)
		setSrmmIcon(mwd->hContact);
	return 0;
}

int __cdecl onIconPressed(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	MCONTACT hMeta = NULL;
	if (db_mc_isMeta(hContact)) {
		hMeta = hContact;
		hContact = metaGetMostOnline(hContact); // возьмем тот, через который пойдет сообщение
	}
	else if (db_mc_isSub(hContact))
		hMeta = db_mc_getMeta(hContact);

	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if (mir_strcmp(sicd->szModule, MODULENAME))
		return 0; // not our event

	int enc = g_plugin.getByte(hContact, "GPGEncryption", 0);
	if (enc) {
		g_plugin.setByte(hContact, "GPGEncryption", 0);
		if (hMeta)
			g_plugin.setByte(hMeta, "GPGEncryption", 0);
		setSrmmIcon(hContact);
		setClistIcon(hContact);
	}
	else if (!enc) {
		if (!isContactHaveKey(hContact))
			ShowLoadPublicKeyDialog(hContact, false);
		else {
			g_plugin.setByte(hContact, "GPGEncryption", 1);
			if (hMeta)
				g_plugin.setByte(hMeta, "GPGEncryption", 1);
			setSrmmIcon(hContact);
			setClistIcon(hContact);
			return 0;
		}

		if (isContactHaveKey(hContact)) {
			g_plugin.setByte(hContact, "GPGEncryption", 1);
			if (hMeta)
				g_plugin.setByte(hMeta, "GPGEncryption", 1);
			setSrmmIcon(hContact);
			setClistIcon(hContact);
		}
	}
	return 0;
}
