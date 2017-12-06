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

#ifndef event_h
#define event_h

///////////////////////////////////////////////////////////////////////////////

class ICQEvent
{
public:
    ICQEvent(unsigned short theCmd, unsigned short theSubCmd, unsigned int theSequence, unsigned int theUin, Socket *theSocket, Packet *thePacket, int theReply);
    ~ICQEvent();
    bool start();
    void stop();
    bool isEvent(SOCKET hSocket, unsigned long theSequence);
    void noAck();
    unsigned short cmd, subCmd;
    unsigned int uin, sequence;
    int reply;
    unsigned int hTimer;
    Packet *packet;
protected:
    unsigned short retries;
    Socket *socket;
};

///////////////////////////////////////////////////////////////////////////////

extern std::vector <ICQEvent *> icqEvents;

extern ICQEvent *getEvent(SOCKET hSocket, unsigned int sequence);

///////////////////////////////////////////////////////////////////////////////

#endif
