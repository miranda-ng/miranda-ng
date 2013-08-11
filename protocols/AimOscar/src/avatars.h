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
#ifndef AVATARS_H
#define AVATARS_H

struct avatar_req
{
    char* sn;
    char* hash;
	unsigned char type;

    avatar_req(char* sn, char* hash, unsigned char type)
		: sn(sn), hash(mir_strdup(hash)), type(type) {} 

    ~avatar_req()
    { mir_free(sn); mir_free(hash); }
};

struct avatar_up_req
{
	char* data1;
	char* data2;
	unsigned short size1;
	unsigned short size2;

    avatar_up_req(char* data1, unsigned short size1, char* data2, unsigned short size2)
		: data1(data1), size1(size1), data2(data2), size2(size2) {} 

    ~avatar_up_req()
    { mir_free(data1); mir_free(data2); }
};

bool get_avatar_hash(const TCHAR* file, char* hash, char** data, unsigned short &size);
void rescale_image(char *data, unsigned short size, char *&data1, unsigned short &size1);

#endif
