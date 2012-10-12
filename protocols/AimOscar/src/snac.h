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
#ifndef SNAC_H
#define SNAC_H

#define SNAC_SIZE 10

class SNAC
{
private:
	unsigned short service_;
	unsigned short subgroup_;
	unsigned short length_;
	unsigned short flags_;
	unsigned short idh_;
	unsigned short id_;
	char* value_;
public:
	SNAC(char* buf, unsigned short length);
	int cmp(unsigned short service);
	int subcmp(unsigned short subgroup);
	unsigned short ushort(int pos=0);
	unsigned long ulong(int pos=0);
	unsigned char ubyte(int pos=0);
	char* part(int pos, int length);
	char* val(int pos=0) { return &value_[pos]; }
	unsigned short len(void) { return length_; }
	unsigned short flags(void) { return flags_; }
	unsigned short id(void) { return id_; }
	unsigned short idh(void) { return idh_; }
};

#endif
