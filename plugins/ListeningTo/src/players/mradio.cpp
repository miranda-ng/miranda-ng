/* 
Copyright (C) 2008 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "..\commons.h"

MRadio::MRadio()
{
	name = _T("mRadio");
	needPoll = TRUE;
}

void MRadio::EnableDisable()
{
	if (!ServiceExists(MS_RADIO_COMMAND))
		enabled = FALSE;
}


int MRadio::GetData()
{
	for (MCONTACT hContact = db_find_first("mRadio"); hContact; hContact = db_find_next(hContact, "mRadio")) {
		WORD status = db_get_w(hContact, "mRadio", "Status", ID_STATUS_OFFLINE); 
		if (status != ID_STATUS_ONLINE)
			continue;

		DBVARIANT dbv;
		if (!db_get_s(hContact, "mRadio", "Nick", &dbv)) {	
			listening_info.cbSize = sizeof(listening_info);
			listening_info.dwFlags = LTI_TCHAR;
			listening_info.ptszArtist = mir_tstrdup(_T("Radio"));
			listening_info.ptszType = mir_tstrdup(_T("Radio"));
			listening_info.ptszTitle = mir_a2t(dbv.pszVal);

			db_free(&dbv);
			return 1;
		}
	}

	return 0;
}


BOOL MRadio::GetListeningInfo(LISTENINGTOINFO *lti)
{
	FreeData();	

	if (enabled) 
	{
		if (CallService(MS_RADIO_COMMAND, MRC_STATUS, RD_STATUS_GET) == RD_STATUS_PLAYING)
		{
			if (!GetData())
				FreeData();
		}
	}

	return Player::GetListeningInfo(lti);
}