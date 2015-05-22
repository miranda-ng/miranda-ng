// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2001-2004 Richard Hughes, Martin Öberg
// Copyright © 2004-2009 Joe Kucera, Bio
// Copyright © 2012-2014 Miranda NG Team
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
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  ChangeInfo Plugin stuff
// -----------------------------------------------------------------------------

#include "stdafx.h"

int CIcqProto::StringToListItemId(const char *szSetting,int def)
{
	int i;

	for(i=0;i<settingCount;i++)
		if (!mir_strcmp(szSetting,setting[i].szDbSetting))
			break;

	if (i == settingCount)
		return def;

	FieldNamesItem *list = (FieldNamesItem*)setting[i].pList;

	char *szValue = getSettingStringUtf(NULL, szSetting, NULL);
	if (!szValue)
		return def;

	for (i=0; list[i].text; i++)
		if (!mir_strcmp(list[i].text, szValue))
			break;

	SAFE_FREE(&szValue);
	if (!list[i].text) return def;

	return list[i].code;
}


int ChangeInfoData::UploadSettings(void)
{
	if (!ppro->icqOnline()) {
		MessageBox(hwndDlg, TranslateT("You are not currently connected to the ICQ network. You must be online in order to update your information on the server."), TranslateT("Change ICQ Details"), MB_OK);
		return 0;
	}

	hUpload[0] = (HANDLE)ppro->ChangeInfoEx(CIXT_FULL, 0);

	//password
	char* tmp = ppro->GetUserPassword(TRUE);
	if (tmp) {
		if (mir_strlen(Password) > 0 && mir_strcmp(Password, tmp)) {
			// update password in user info dialog (still open)
			strcpy(Password, tmp);
			// update password in protocol
			strcpy(ppro->m_szPassword, tmp);
			
			hUpload[1] = (HANDLE)ppro->icq_changeUserPasswordServ(tmp);
			char szPwd[PASSWORDMAXLEN] = {0};

			// password is stored in DB, update
			if (ppro->GetUserStoredPassword(szPwd, sizeof(szPwd)))
				ppro->setString("Password", tmp);
		}
	}

	return 1;
}
