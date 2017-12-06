/*
    ICQ Corporate protocol plugin for Miranda IM.
    Copyright (C) 2003-2005 Eugene Tarasenko <zlyden13@inbox.ru>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef user_h
#define user_h

#include "socket.h"


///////////////////////////////////////////////////////////////////////////////

class ICQUser
{
public:
    unsigned int uin;
	MCONTACT hContact;
    TCPSocket socket;
    unsigned short statusVal;

    ICQUser();
    void setStatus(unsigned short newStatus);
    void setInfo(char *name, unsigned int data);
    void setInfo(char *name, unsigned short data);
    void setInfo(char *name, unsigned char data);
    void setInfo(char *name, char *data);
};

///////////////////////////////////////////////////////////////////////////////

extern std::vector <ICQUser *> icqUsers;

extern int icqUserInfoInitialise(WPARAM wParam, LPARAM lParam);

///////////////////////////////////////////////////////////////////////////////

#endif