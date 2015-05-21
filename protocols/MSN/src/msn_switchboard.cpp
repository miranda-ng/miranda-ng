/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"
#ifdef OBSOLETE

int ThreadData::contactJoined(const char* email)
{
	for (int i = 0; i < mJoinedContactsWLID.getCount(); i++)
		if (_stricmp(mJoinedContactsWLID[i], email) == 0)
			return mJoinedContactsWLID.getCount();

	if (strchr(email, ';'))
		mJoinedIdentContactsWLID.insertn(email);
	else
		mJoinedContactsWLID.insertn(email);

	return mJoinedContactsWLID.getCount();
}

int ThreadData::contactLeft(const char* email)
{
	if (strchr(email, ';'))
		mJoinedIdentContactsWLID.remove(email);
	else
		mJoinedContactsWLID.remove(email);

	return mJoinedContactsWLID.getCount();
}
#endif

MCONTACT ThreadData::getContactHandle(void)
{
	return mJoinedContactsWLID.getCount() ? proto->MSN_HContactFromEmail(mJoinedContactsWLID[0]) : NULL;
}

