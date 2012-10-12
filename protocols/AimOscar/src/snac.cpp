/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2009 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

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
#include "aim.h"
#include "snac.h"
#include "packets.h"

SNAC::SNAC(char* buf,unsigned short length)
{
	service_=_htons((*(unsigned short*)&buf[0]));
	subgroup_=_htons((*(unsigned short*)&buf[2]));
	flags_=_htons((*(unsigned short*)&buf[4]));
	idh_=_htons((*(unsigned short*)&buf[6]));
	id_=_htons((*(unsigned short*)&buf[8]));
	value_=&buf[SNAC_SIZE];
	length_=length;
}
int SNAC::cmp(unsigned short service)
{
	if(service_==service)
		return 1;
	else 
		return 0;
}
int SNAC::subcmp(unsigned short subgroup)
{
	if(subgroup_==subgroup)
		return 1;
	else 
		return 0;
}
unsigned short SNAC::ushort(int pos)
{
	return _htons(*(unsigned short*)&value_[pos]);
}
unsigned long SNAC::ulong(int pos)
{
	return _htonl(*(unsigned long*)&value_[pos]);
}
unsigned char SNAC::ubyte(int pos)
{
	return value_[pos];
}
char* SNAC::part(int pos, int length)
{
	char* value = (char*)mir_alloc(length+1);
	memcpy(value, &value_[pos], length);
	value[length] = '\0';
	return value;
}
