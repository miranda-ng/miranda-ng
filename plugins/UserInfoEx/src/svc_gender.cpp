/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

uint8_t GenderOf(MCONTACT hContact, LPCSTR pszProto)
{
	DBVARIANT dbv;
	if (DB::Setting::GetAsIsEx(hContact, USERINFO, pszProto, SET_CONTACT_GENDER, &dbv) == 0) {
		// gender must be byte and either M or F
		if (dbv.type == DBVT_BYTE && (dbv.bVal == 'M' || dbv.bVal == 'F'))
			return dbv.bVal;	

		db_free(&dbv);
	}
	return 0;
}

/**
* This function gets the gender of the contact from the database.
*
* @param	hContact		- handle to contact to read email from
*
* @retval	F	- contact is female
* @retval	M	- contact is male
* @retval	0	- contact does not provide its gender
**/

uint8_t GenderOf(MCONTACT hContact)
{
	return GenderOf(hContact, Proto_GetBaseAccountName(hContact));
}
