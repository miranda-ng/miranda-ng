/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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

#include "commonheaders.h"

//VERY VERY VERY BASIC ENCRYPTION FUNCTION


void Encrypt(char*msg,BOOL up)
{
	int i;
	int jump;
	if (up)
	{
		jump=5;
	}
	else
	{
		jump=-5;
	}

	for (i=0;msg[i];i++)
	{
			msg[i]=msg[i]+jump;
	}

}

static INT_PTR EncodeString(WPARAM wParam,LPARAM lParam)
{
	Encrypt((char*)lParam,TRUE);
	return 0;
}

static INT_PTR DecodeString(WPARAM wParam,LPARAM lParam)
{
	Encrypt((char*)lParam,FALSE);
	return 0;
}

int InitCrypt(void)
{
	CreateServiceFunction(MS_DB_CRYPT_ENCODESTRING,EncodeString);
	CreateServiceFunction(MS_DB_CRYPT_DECODESTRING,DecodeString);
	return 0;
}
