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

#include "corp.h"

std::vector <ICQEvent *> icqEvents;

///////////////////////////////////////////////////////////////////////////////

void WINAPI eventTimerProc(HWND hWnd, UINT Msg, UINT_PTR hTimer, DWORD Time)
{
    unsigned int i;

    KillTimer(NULL, hTimer);
    for (i=0; i<icqEvents.size(); i++) if (hTimer == icqEvents[i]->hTimer) icqEvents[i]->noAck();
}

///////////////////////////////////////////////////////////////////////////////

ICQEvent *getEvent(SOCKET hSocket, unsigned int sequence)
{
    unsigned int i;

    for (i=0; i<icqEvents.size(); i++) if (icqEvents[i]->isEvent(hSocket, sequence)) return icqEvents[i];
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

ICQEvent::ICQEvent(unsigned short theCmd, unsigned short theSubCmd, unsigned int theSequence,
                   unsigned int theUin, Socket *theSocket, Packet *thePacket, int theReply)
{
    cmd = theCmd;
    subCmd = theSubCmd;
    sequence = theSequence;
    uin = theUin;
    socket = theSocket;
    packet = new Packet(thePacket);
    reply = theReply;

    hTimer = NULL;
    retries = 0;
}

///////////////////////////////////////////////////////////////////////////////

ICQEvent::~ICQEvent()
{
    stop();
    delete packet;
}

///////////////////////////////////////////////////////////////////////////////

bool ICQEvent::start()
{
    // send the packet
    if (!socket->sendPacket(*packet)) return false;

    if (cmd != ICQ_CMDxTCP_START) hTimer = SetTimer(NULL, 0, MAX_WAIT_ACK, (TIMERPROC)eventTimerProc);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void ICQEvent::stop()
{
    if (hTimer)
    {
        KillTimer(NULL, hTimer);
        hTimer = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool ICQEvent::isEvent(SOCKET hSocket, unsigned long theSequence)
{
    return socket->handleVal == hSocket && sequence == theSequence;
}

///////////////////////////////////////////////////////////////////////////////

void ICQEvent::noAck()
{
    hTimer = NULL;
    if (socket->connected() && retries < MAX_SERVER_RETRIES && cmd != ICQ_CMDxTCP_START)
    {
        retries++;
        start();
    }
    else icq.doneEvent(false, socket->handleVal, sequence);
}

///////////////////////////////////////////////////////////////////////////////
