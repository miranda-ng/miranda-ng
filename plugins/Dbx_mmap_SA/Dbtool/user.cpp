/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

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
#include "dbtool.h"

static DBContact user;
static int phase;
static DWORD ofsUser;

int WorkSettingsChain(DWORD ofsContact,DBContact *dbc,int firstTime);
int WorkEventChain(DWORD ofsContact,DBContact *dbc,int firstTime);

int WorkUser(int firstTime)
{
	int first=0;

	if(firstTime) {
		AddToStatus(STATUS_MESSAGE,TranslateT("Processing user data"));
		if(!SignatureValid(dbhdr.ofsUser,DBCONTACT_SIGNATURE)) {
			AddToStatus(STATUS_ERROR,TranslateT("User corrupted, this could cause major problems"));
			return ERROR_NO_MORE_ITEMS;
		}
		if(ReadSegment(dbhdr.ofsUser,&user,sizeof(DBContact))!=ERROR_SUCCESS)
			return ERROR_NO_MORE_ITEMS;
		if(user.ofsNext) {
			AddToStatus(STATUS_WARNING,TranslateT("More than one user contact: keeping only first"));
			user.ofsNext=0;
		}
		if((ofsUser=WriteSegment(WSOFS_END,&user,sizeof(DBContact)))==WS_ERROR)
			return ERROR_HANDLE_DISK_FULL;
		dbhdr.ofsUser=ofsUser;
		phase=0;
		first=1;
	}
	switch(phase) {
		int ret;

		case 0:
			ret=WorkSettingsChain(ofsUser,&user,first);
			if(ret==ERROR_NO_MORE_ITEMS) {
				phase++; first=1;
			}
			else if(ret) return ret;
			else break;
		case 1:
			ret=WorkEventChain(ofsUser,&user,first);
			if(ret==ERROR_NO_MORE_ITEMS) {
				if(WriteSegment(ofsUser,&user,sizeof(DBContact))==WS_ERROR)
					return ERROR_HANDLE_DISK_FULL;
				return ERROR_NO_MORE_ITEMS;
			}
			else if(ret) return ret;
			break;
	}
	return ERROR_SUCCESS;
}