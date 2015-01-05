/*

Copyright 2000-12 Miranda IM, 2012-15 Miranda NG project,
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

#ifndef SRMM_CMDLIST_H
#define SRMM_CMDLIST_H

struct TMsgQueue
{
	int id;
	MCONTACT hContact;
	char *szMsg;
	int flags;
	unsigned ts;
};

void msgQueue_add(MCONTACT hContact, int id, char *szMsg, int flags);
void msgQueue_processack(MCONTACT hContact, int id, BOOL success, const char* szErr);
void msgQueue_destroy(void);

#endif
