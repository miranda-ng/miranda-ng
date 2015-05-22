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

ICQ icq;

///////////////////////////////////////////////////////////////////////////////

unsigned short toIcqStatus(unsigned short status)
{
/*
    unsigned short icqStatus[] = { ICQ_STATUS_OFFLINE, ICQ_STATUS_ONLINE, ICQ_STATUS_AWAY, ICQ_STATUS_DND,
        ICQ_STATUS_NA, ICQ_STATUS_OCCUPIED, ICQ_STATUS_FREEFORCHAT, ICQ_STATUS_PRIVATE };

    return icqStatus[status - ID_STATUS_OFFLINE];
*/
    switch (status)
    {
    case ID_STATUS_OFFLINE: return ICQ_STATUS_OFFLINE;
    case ID_STATUS_ONLINE: return ICQ_STATUS_ONLINE;
    case ID_STATUS_AWAY: return ICQ_STATUS_AWAY;
    case ID_STATUS_DND: return ICQ_STATUS_DND;
    case ID_STATUS_NA: return ICQ_STATUS_NA;
    case ID_STATUS_OCCUPIED: return ICQ_STATUS_OCCUPIED;
    case ID_STATUS_FREECHAT: return ICQ_STATUS_FREECHAT;
    case ID_STATUS_INVISIBLE: return ICQ_STATUS_PRIVATE;
    }
    return ICQ_STATUS_ONLINE;
}

///////////////////////////////////////////////////////////////////////////////

unsigned short toIdStatus(unsigned short status)
{
    switch (status)
    {
    case ICQ_STATUS_OFFLINE: return ID_STATUS_OFFLINE;
    case ICQ_STATUS_ONLINE: return ID_STATUS_ONLINE;
    case ICQ_STATUS_AWAY: return ID_STATUS_AWAY;
    case ICQ_STATUS_DND: return ID_STATUS_DND;
    case ICQ_STATUS_NA: return ID_STATUS_NA;
    case ICQ_STATUS_OCCUPIED: return ID_STATUS_OCCUPIED;
    case ICQ_STATUS_FREECHAT: return ID_STATUS_FREECHAT;
    case ICQ_STATUS_PRIVATE: return ID_STATUS_INVISIBLE;
    }
    return ID_STATUS_ONLINE;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT WINAPI messageWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SOCKET hSocket = (SOCKET)wParam;
    unsigned short netEvents = LOWORD(lParam);
    unsigned long result;

    switch (msg)
    {
    case WM_NETEVENT_SERVER:
        if (hSocket == icq.udpSocket.handleVal)
        {
            if (netEvents & FD_READ) icq.recvUDP(0);
            break;
        }
        break;

    case WM_NETEVENT_CONNECTION:
        if (hSocket == icq.tcpSocket.handleVal)
        {
            if (netEvents & FD_ACCEPT) icq.recvNewTCP(0);
            break;
        }
        break;

    case WM_NETEVENT_USER:
        if (netEvents & FD_READ)
        {
            ioctlsocket(hSocket, FIONREAD, &result);
            if (result > 0) icq.recvTCP(hSocket);
        }
        if (netEvents & FD_CLOSE)
        {
            unsigned int i;
            for (i=0; i<icqUsers.size(); i++)
            {
                if (hSocket == icqUsers[i]->socket.handleVal)
                {
                    T("[tcp] user %d is aborted connection\n", icqUsers[i]->uin);
                    icqUsers[i]->socket.closeConnection();
                    break;
                }
            }
        }
        break;

    case WM_NETEVENT_TRANSFER:
        if (netEvents & FD_READ)
        {
            ioctlsocket(hSocket, FIONREAD, &result);
            if (result > 0) icq.recvTransferTCP(hSocket);
        }
        if (netEvents & FD_CLOSE)
        {
            unsigned int i;
            for (i=0; i<icqTransfers.size(); i++)
            {
                if (hSocket == icqTransfers[i]->socket.handleVal)
                {
                    T("[tcp] user %d is aborted file connection\n", icqTransfers[i]->uin);
                    ProtoBroadcastAck(protoName, icqTransfers[i]->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, icqTransfers[i], 0);
                    delete icqTransfers[i];
                    icqTransfers[i] = icqTransfers[icqTransfers.size() - 1];
                    icqTransfers.pop_back();
                    break;
                }
            }
        }
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void WINAPI pingTimerProc(HWND hWnd, UINT Msg, UINT_PTR hTimer, DWORD Time)
{
    icq.ping();
}

///////////////////////////////////////////////////////////////////////////////
//
//  ICQ
//
///////////////////////////////////////////////////////////////////////////////

ICQ::ICQ() :
    tcpSocket(WM_NETEVENT_CONNECTION),
    udpSocket(WM_NETEVENT_SERVER)
{
}

///////////////////////////////////////////////////////////////////////////////

bool ICQ::load()
{
    WSADATA data;

    if (WSAStartup(MAKEWORD(2,2), &data))
    {
        MessageBox(NULL, Translate("ICQ Corporate plugin used only WinSock v2.2 or later."), protoName, MB_ICONWARNING|MB_OK);
        return false;
    }

    statusVal = ID_STATUS_OFFLINE;
    searchSequenceVal = 0;
    tcpSequenceVal = 0xFFFFFFFE;

    awayMessage = new char[1];
    awayMessage[0] = 0;

    WNDCLASS wc = { 0, messageWndProc, 0, 0, hInstance, NULL, NULL, NULL, NULL, protoName };
    if (!RegisterClass(&wc)) return false;

    hWnd = CreateWindowEx(0, protoName, NULL, 0, 0, 0, 0, 0, (unsigned short)GetVersion() >= 5 ? HWND_MESSAGE : 0, NULL, hInstance, NULL);
    if (hWnd == NULL) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::unload()
{
    if (statusVal != ID_STATUS_OFFLINE) logoff(false);

    KillTimer(NULL, pingTimer);
    pingTimer = NULL;

    delete [] awayMessage;

    WSACleanup();

    DestroyWindow(hWnd);
    UnregisterClass(protoName, hInstance);
}

///////////////////////////////////////////////////////////////////////////////

bool ICQ::logon(unsigned short logonStatus)
{
    DBVARIANT dbv;
    char str[128];

    if(!db_get(NULL, protoName, "Server", &dbv))
    {
        lstrcpy(str, dbv.pszVal);
        db_free(&dbv);
    }
    else
    {
        MessageBox(NULL, Translate("You need specify ICQ Corporate login server."), protoName, MB_ICONWARNING|MB_OK);
        return false;
    }

    if (!tcpSocket.connected() && !tcpSocket.startServer()) return false;
    if (!udpSocket.connected())
    {
        if (!udpSocket.setDestination(str, db_get_w(NULL, protoName, "Port", 4000))) return false;
        udpSocket.openConnection();
    }

    if (pingTimer == NULL) pingTimer = SetTimer(NULL, 0, PING_FREQUENCY, (TIMERPROC)pingTimerProc);

    updateContactList();

    uin = db_get_dw(NULL, protoName, "UIN", 0);
    if (!db_get(NULL, protoName, "Password", &dbv))
    {
        lstrcpy(str, dbv.pszVal);
        db_free(&dbv);
    }


    timeStampLastMessage = 0;
    sequenceVal = 1;

    Packet loginPacket;
    loginPacket << ICQ_VERSION
                << ICQ_CMDxSND_LOGON
                << sequenceVal
                << sequenceVal
                << uin
                << (unsigned int)0x00
                << tcpSocket.localPortVal
                << str
                << (unsigned short)0x7A
                << (unsigned short)0x02
//                << LOCALHOST
                << udpSocket.localIPVal
                << (unsigned char)0x04
                << (unsigned int)toIcqStatus(logonStatus)
                << (unsigned int)0x02
                << (unsigned int)0x00
                << (unsigned short)0x13
                << (unsigned short)0x7A;

    T("[udp] requesting logon (%d)...\n", sequenceVal);
    sendICQ(udpSocket, loginPacket, ICQ_CMDxSND_LOGON, sequenceVal);

    desiredStatus = logonStatus;
    statusVal = ID_STATUS_CONNECTING;
    ProtoBroadcastAck(protoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_OFFLINE, statusVal);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::logoff(bool reconnect)
{
    unsigned int i;

    // if not connected then don't both logging off
    if (udpSocket.connected())
    {
        Packet logoffPacket;
        logoffPacket << ICQ_VERSION
                     << ICQ_CMDxSND_LOGOFF
                     << (unsigned int)0x00
                     << uin
                     << (unsigned int)0x00
                     << "B_USER_DISCONNECTED"
                     << (unsigned short)0x0005;

        T("[udp] logging off.\n");
        udpSocket.sendPacket(logoffPacket);
//        udpSocket.closeConnection();

        // close all open events
        for (i=0; i<icqEvents.size(); i++) delete icqEvents[i];
        icqEvents.clear();
    }

    statusVal = ID_STATUS_OFFLINE;

    if (reconnect) logon(desiredStatus);
    else
    {
        udpSocket.closeConnection();
        tcpSocket.closeConnection();

        updateContactList();
        ProtoBroadcastAck(protoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_ONLINE, statusVal);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::ping()
{
    if (statusVal > ID_STATUS_OFFLINE)
    {
        Packet pingPacket;
        pingPacket << ICQ_VERSION
                   << ICQ_CMDxSND_PING
                   << sequenceVal
                   << (unsigned short)0x00
                   << uin
                   << (unsigned int)0x00;

        T("[udp] keep alive (%d)\n", sequenceVal);
        sendICQ(udpSocket, pingPacket, ICQ_CMDxSND_PING, sequenceVal);
    }

    if (statusVal == ID_STATUS_OFFLINE && desiredStatus != ID_STATUS_OFFLINE) logoff(true);
}

///////////////////////////////////////////////////////////////////////////////

ICQEvent *ICQ::sendICQ(Socket &socket, Packet &packet, unsigned short cmd, unsigned long sequence,
                       unsigned long uin, unsigned short subCmd, int reply)
{
    ICQEvent *result;

    if (!socket.connected()) return NULL;

    if (cmd != ICQ_CMDxTCP_START) sequenceVal++;

    icqEvents.push_back(result = new ICQEvent(cmd, subCmd, sequence, uin, &socket, &packet, reply));
    if (!result->start())
    {
        cancelEvent(result);
        return NULL;
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::doneEvent(bool gotAck, int hSocket, int sequence)
{
    unsigned int i;
    ICQEvent *e;

    for (i=0; i<icqEvents.size(); i++)
    {
        e = icqEvents[i];
        if (e->isEvent(hSocket, sequence)) break;
    }
    if (i == icqEvents.size()) return;

    e->stop();
    if (!gotAck || e->reply == 0)
    {
        icqEvents[i] = icqEvents[icqEvents.size() - 1];
        icqEvents.pop_back();
    }

    if (!gotAck) T("[   ] sending failed (%d)\n", sequence);

    switch (e->cmd)
    {
    case ICQ_CMDxTCP_START:
        doneUserFcn(gotAck, e);
        break;
    case ICQ_CMDxSND_THRUxSERVER:
        doneUserFcn(gotAck, e);
        break;
    case ICQ_CMDxSND_USERxGETxINFO:
        //emit doneUserInfo(true, e->uin);
        break;
    case ICQ_CMDxSND_SETxSTATUS:
        if (gotAck)
        {
            int oldStatus = statusVal;
            statusVal = desiredStatus;
            ProtoBroadcastAck(protoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, statusVal);
        }
        break;
    case ICQ_CMDxSND_PING:
        //if (!gotAck) emit doneOwnerFcn(false, cmd);
        break;
    case ICQ_CMDxSND_USERxADD:
        //if (!gotAck) emit doneOwnerFcn(false, cmd);
        break;
    case ICQ_CMDxSND_AUTHORIZE:
        //emit doneOwnerFcn(gotAck, cmd);
        break;
    case ICQ_CMDxSND_LOGON:
        if (!gotAck)
        {
            logoff(false);
         //emit doneOwnerFcn(false, cmd);
        }
        break;
    case ICQ_CMDxSND_USERxLIST:
        //if (!gotAck) emit doneOwnerFcn(false, cmd);
        break;
    case ICQ_CMDxSND_VISxLIST:
        //if (!gotAck) emit doneOwnerFcn(false, cmd);
        break;
    case ICQ_CMDxSND_SYSxMSGxREQ:
        //if (!gotAck) emit doneOwnerFcn(false, cmd);
        break;
    case ICQ_CMDxSND_SYSxMSGxDONExACK:
        //if (!gotAck) emit doneOwnerFcn(false, cmd);
        break;
    }

    if (!gotAck && e->cmd != ICQ_CMDxTCP_START && e->cmd != ICQ_CMDxSND_LOGON) logoff(true);


    if (!gotAck || e->reply == 0) delete e;

    else e->reply--;

}

///////////////////////////////////////////////////////////////////////////////

void ICQ::cancelEvent(ICQEvent *&e)
{
    unsigned int i;

    for (i=0; i<icqEvents.size(); i++) if (icqEvents[i] == e) break;
    if (i == icqEvents.size()) return;

    e->stop();

    icqEvents[i] = icqEvents[icqEvents.size() - 1];
    icqEvents.pop_back();

    delete e;
    e = NULL;
}

///////////////////////////////////////////////////////////////////////////////

unsigned short ICQ::processUdpPacket(Packet &packet)
{
    unsigned short version, command, newCommand, theSequence, theSequence1, searchSequence, junkShort;
    unsigned int checkUin, userIP, realIP, junkl, newStatus, userPort, timedataStamp;
    unsigned char junkChar;
    char *message = NULL;
    ICQUser *u;
    ICQEvent *e;

    // read in the standard UDP header info
    packet >> version
           >> command
           >> theSequence
           >> theSequence1
           >> checkUin
           >> junkl;

    if (version != ICQ_VERSION)
    {
        T("[udp] bad version number %d\n", version);
        return 0xFFFF;
    }
/*	on offline multi packet field checkUin not equal uin :(

    if (checkUin != uin)
    {
        T("[udp] bad uin number %d\n", checkUin);
        return 0xFFFF;
    }
*/
    switch (command)
    {
    case ICQ_CMDxRCV_LOGIN_ERR:
        T("[udp] error loging to server.\n");
        ackUDP(theSequence);

        packet >> message;

        T("%s\n", message);
        MessageBox(NULL, message, protoName, MB_ICONERROR|MB_OK);
        delete [] message;
        break;

    case ICQ_CMDxRCV_USERxONLINE: // initial user status packet
        packet >> checkUin;

        T("[udp] user %d is online\n", checkUin);
        ackUDP(theSequence);

        if ((u = getUserByUIN(checkUin, false)) == NULL) break;

        packet >> userIP
               >> userPort
               >> realIP
               >> junkChar
               >> newStatus;

        u->socket.closeConnection();
        u->socket.setDestination(userIP, userPort);
        u->setStatus(toIdStatus(newStatus));
        u->setInfo("IP", (unsigned int)ntohl(userIP));
        u->setInfo("Port", (unsigned short)userPort);
        u->setInfo("RealIP", (unsigned int)ntohl(realIP));
        break;

    case ICQ_CMDxRCV_USERxOFFLINE: // user just went offline packet
        packet >> checkUin;

        T("[udp] user %d is offline\n", checkUin);
        ackUDP(theSequence);

        if ((u = getUserByUIN(checkUin, false)) == NULL) break;

        u->setStatus(ID_STATUS_OFFLINE);
        u->socket.closeConnection();
        break;

    case ICQ_CMDxRCV_USERxBASICxINFO:
    case ICQ_CMDxRCV_USERxINFO:
    case ICQ_CMDxRCV_USERxWORKxINFO:
    case ICQ_CMDxRCV_USERxWORKxPAGE:
    case ICQ_CMDxRCV_USERxHOMExINFO:
    case ICQ_CMDxRCV_USERxHOMExPAGE:
        T("[udp] user information packet (%d)\n", theSequence);
        ackUDP(theSequence);

        if ((e = getEvent(udpSocket.handleVal, theSequence1)) == NULL) break;
        checkUin = e->uin;
        if ((u = getUserByUIN(checkUin, false)) == NULL) break;

        char *buffer;
        buffer = new char[1024];

        switch (command)
        {
        case ICQ_CMDxRCV_USERxBASICxINFO:
        case ICQ_CMDxRCV_USERxINFO:
            packet >> buffer;
            u->setInfo("Nick", buffer);
            packet >> buffer;
            u->setInfo("FirstName", buffer);
            packet >> buffer;
            u->setInfo("LastName", buffer);
            packet >> buffer;
            u->setInfo("e-mail", buffer);
            break;

        case ICQ_CMDxRCV_USERxWORKxINFO:
            packet >> buffer;
            u->setInfo("CompanyStreet", buffer);
            packet >> buffer;
            u->setInfo("CompanyCity", buffer);
            packet >> buffer;
            u->setInfo("CompanyState", buffer);
            packet >> junkShort;
            u->setInfo("CompanyCountry", junkShort);
            packet >> buffer;
            u->setInfo("Company", buffer);
            packet >> buffer;
            u->setInfo("CompanyPosition", buffer);
            packet >> junkl;
            packet >> buffer;
            u->setInfo("CompanyPhone", buffer);
            packet >> buffer;
            u->setInfo("CompanyFax", buffer);
            packet >> buffer;
            packet >> junkl;
            if (junkl && junkl != 0xFFFFFFFF) _itoa(junkl, buffer, 10);
            else buffer[0] = 0;
            u->setInfo("CompanyZIP", buffer);
            break;

        case ICQ_CMDxRCV_USERxWORKxPAGE:
            packet >> buffer;
            u->setInfo("CompanyHomepage", buffer);
            break;

        case ICQ_CMDxRCV_USERxHOMExINFO:
            packet >> buffer;
            u->setInfo("Street", buffer);
            packet >> buffer;
            u->setInfo("City", buffer);
            packet >> buffer;
            u->setInfo("State", buffer);
            packet >> junkShort;
            u->setInfo("Country", junkShort);
            packet >> buffer;
            u->setInfo("Phone", buffer);
            packet >> buffer;
            u->setInfo("Fax", buffer);
            packet >> buffer;
            u->setInfo("Cellular", buffer);
            packet >> junkl;
            if (junkl && junkl != 0xFFFFFFFF) _itoa(junkl, buffer, 10);
            else buffer[0] = 0;
            u->setInfo("ZIP", buffer);
            packet >> junkChar;
            if (junkChar == 1) junkChar = 'F';
            if (junkChar == 2) junkChar = 'M';
            u->setInfo("Gender", junkChar);
            packet >> junkShort;
            u->setInfo("Age", (unsigned char)junkShort);
            packet >> junkChar;
            u->setInfo("BirthDay", junkChar);
            packet >> junkChar;
            u->setInfo("BirthMonth", junkChar);
            packet >> junkShort;
            u->setInfo("BirthYear", junkShort);
            break;

        case ICQ_CMDxRCV_USERxHOMExPAGE:
            packet >> buffer;
            u->setInfo("Homepage", buffer);
            break;
        }

		if (e->reply == 0) ProtoBroadcastAck(protoName, u->hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, 0, 0);
        doneEvent(true, udpSocket.handleVal, theSequence1);
        delete [] buffer;
        break;

    case ICQ_CMDxRCV_USERxINVALIDxUIN: // not a good uin
        T("[udp] invalid uin\n");
        ackUDP(theSequence);

        if ((e = getEvent(udpSocket.handleVal, theSequence1)) == NULL) break;

        checkUin = e->uin;
        T("invalid uin: %d\n", checkUin);
/*
        delete icqEvents[i2];
        icqEvents[i2] = icqEvents[icqEvents.size() - 1];
        icqEvents.pop_back();
*/
        break;

    case ICQ_CMDxRCV_USERxSTATUS: // user changed status packet
        packet >> checkUin;

        T("[udp] user %d changed status\n", checkUin);
        ackUDP(theSequence);

        packet >> newStatus;

        if ((u = getUserByUIN(checkUin, false)) == NULL) break;
        u->setStatus(toIdStatus(newStatus));
        break;

    case ICQ_CMDxRCV_USERxLISTxDONE: // end of user list
        T("[udp] end of user list.\n");
        ackUDP(theSequence);
      break;

    case ICQ_CMDxRCV_SEARCHxFOUND: // user found in search
        T("[udp] search found user\n");
        ackUDP(theSequence);

        char *alias, *firstName, *lastName, *email;
        unsigned char auth;

        alias = NULL;
        firstName = NULL;
        lastName = NULL;
        email = NULL;

        packet >> checkUin
               >> alias
               >> firstName
               >> lastName
               >> email
               >> auth;

        ICQSEARCHRESULT isr;

        isr.hdr.cbSize = sizeof(isr);
        isr.hdr.nick = alias;
        isr.hdr.firstName = firstName;
        isr.hdr.lastName = lastName;
        isr.hdr.email = email;
        isr.uin = checkUin;
        isr.auth = auth;
        ProtoBroadcastAck(protoName, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&isr);

        delete [] alias;
        delete [] firstName;
        delete [] lastName;
        delete [] email;
        break;

    case ICQ_CMDxRCV_SEARCHxDONE:
        T("[udp] search finished.\n");
        ackUDP(theSequence);

        packet >> searchSequence;
        searchSequence = theSequence1;

        ProtoBroadcastAck(protoName, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
        break;

    case ICQ_CMDxRCV_SYSxMSGxOFFLINE: // offline system message, now have to check the sub-command
        T("[udp] offline system message\n");
        ackUDP(theSequence);

        packet >> checkUin
               >> timedataStamp
               >> newCommand;

        timeStampLastMessage = timedataStamp;
        timedataStamp = CallService(MS_DB_TIME_TIMESTAMPTOLOCAL, timedataStamp, 0);

        processSystemMessage(packet, checkUin, newCommand, timedataStamp);
        break;

   case ICQ_CMDxRCV_SYSxMSGxONLINE: // online system message, now have to check the sub-command
        T("[udp] online system message\n");
        ackUDP(theSequence);

        packet >> checkUin
               >> newCommand;

        processSystemMessage(packet, checkUin, newCommand, time(NULL));
        break;

    case ICQ_CMDxRCV_SYSxMSGxDONE: // end of system messages
        T("[udp] end of system messages.\n");
        ackUDP(theSequence);

        if (timeStampLastMessage)
        {
            ackSYS(timeStampLastMessage);
            timeStampLastMessage = 0;
        }
        break;

    case ICQ_CMDxRCV_BROADCASTxMULTI:
        T("[udp] broadcast multi-packet (%d)\n", theSequence);
        ackUDP(theSequence);

        unsigned int i;
        unsigned char j, frameNo, frameSize;
        bool found;

        packet >> frameNo
               >> frameSize;

        icqEvents.push_back(new ICQEvent(ICQ_CMDxRCV_BROADCASTxMULTI, (unsigned short)frameNo, theSequence1, 0, &udpSocket, &packet, 0));

        {
            Packet multiPacket;

            for (j=0; j<frameSize; j++)
            {
                found = false;
                for (i=0; i<icqEvents.size(); i++)
                {
                    e = icqEvents[i];
                    if (e->cmd == ICQ_CMDxRCV_BROADCASTxMULTI && e->subCmd == j && e->isEvent(udpSocket.handleVal, theSequence1))
                    {
                        multiPacket << e->packet;
                        found = true;
                        break;
                    }
                }
                if (!found) break;
            }

            if (j == frameSize)
            {
                for (i=0; i<icqEvents.size(); i++)
                {
                    e = icqEvents[i];
                    if (e->cmd == ICQ_CMDxRCV_BROADCASTxMULTI && e->isEvent(udpSocket.handleVal, theSequence1))
                    {
                        icqEvents[i] = icqEvents[icqEvents.size() - 1];
                        icqEvents.pop_back();

                        delete e;
                    }
                }

                multiPacket.reset();
                processUdpPacket(multiPacket);
            }
        }
        break;

    case ICQ_CMDxRCV_BROADCASTxOFFLINE:
        T("[udp] offline broadcast message (%d)\n", theSequence);
        ackUDP(theSequence);

        packet >> checkUin
               >> timedataStamp
               >> newCommand;

        db_set_dw(NULL, protoName, "LastBroadcastTime", timedataStamp);
        timedataStamp = CallService(MS_DB_TIME_TIMESTAMPTOLOCAL, timedataStamp, 0);

        processSystemMessage(packet, checkUin, newCommand, timedataStamp);
        break;

   case ICQ_CMDxRCV_BROADCASTxONLINE:
        T("[udp] online broadcast message (%d)\n", theSequence);
        ackUDP(theSequence);

        packet >> checkUin
               >> newCommand;

        processSystemMessage(packet, checkUin, newCommand, time(NULL));
        break;

    case ICQ_CMDxRCV_BROADCASTxDONE:
        T("[udp] end of broadcast messages.\n");
        ackUDP(theSequence);
        break;

   case ICQ_CMDxRCV_SETxOFFLINE: // we got put offline by mirabilis for some reason
        T("[udp] kicked offline.\n");
        logoff(true);
        break;

   case ICQ_CMDxRCV_ACK: // icq acknowledgement
        T("[udp] received ack (%d)\n", theSequence);
        doneEvent(true, udpSocket.handleVal, theSequence);
        break;

   case ICQ_CMDxRCV_ERROR: // icq says go away
        T("[udp] server says bugger off.\n");
        logoff(true);
        break;

   case ICQ_CMDxRCV_HELLO: // hello packet from mirabilis received on logon
        T("[udp] received hello.\n");
        ackUDP(theSequence);

        int oldStatus;

        requestSystemMsg();
        requestBroadcastMsg();

//      pingTimer.start(PING_FREQUENCY * 1000);
        oldStatus = statusVal;
        statusVal = desiredStatus;
        ProtoBroadcastAck(protoName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, statusVal);

        updateContactList();
//        sendVisibleList();
//        sendInvisibleList();
        break;

    case ICQ_CMDxRCV_WRONGxPASSWD: // incorrect password sent in logon
        T("[udp] incorrect password.\n");
        ProtoBroadcastAck(protoName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
        MessageBox(NULL, Translate("Your ICQ Corp number and password combination was rejected by the ICQ Corporate server. Please go to M->Options->ICQCorp and try again."), protoName, MB_ICONERROR|MB_OK);
        break;

    case ICQ_CMDxRCV_BUSY: // server too busy to respond
        T("[udp] server busy, try again in a few minutes.\n");
        break;

    default: // what the heck is this packet?
        T("[udp] unknown packet:\n%s", packet.print());
        ackUDP(theSequence);
        break;
    }

    return command;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::processSystemMessage(Packet &packet, unsigned long checkUin, unsigned short newCommand, time_t timeSent)
{
    char *message/*, *sysMsg*/;
    ICQUser *u;
    unsigned int i, /*j,*/ messageLen;

    u = getUserByUIN(checkUin);

    message = NULL;
    packet >> message;

    switch (newCommand)
    {
    case ICQ_CMDxRCV_SYSxMSG:
        T("message through server from %d\n", checkUin);
        addMessage(u, message, ICQ_CMDxRCV_SYSxMSGxONLINE, ICQ_CMDxTCP_MSG, 0, timeSent);
        break;

    case ICQ_CMDxRCV_SYSxURL:
        T("url through server from %d\n", checkUin);
        addUrl(u, message, ICQ_CMDxRCV_SYSxMSGxONLINE, ICQ_CMDxTCP_URL, 0, timeSent);
        break;

    case ICQ_CMDxRCV_SYSxBROADCAST:
        T("broadcast message from %d\n", checkUin);

        messageLen = (unsigned int)mir_strlen(message);
        for (i=0; i<messageLen; i++) if (message[i] == (char)0xFE) message[i] = '\n';

        addMessage(u, message, ICQ_CMDxRCV_SYSxMSGxONLINE, ICQ_CMDxTCP_MSG, 0, timeSent);
        break;
/*
    case ICQ_CMDxRCV_SYSxAUTHxREQ:  // system message: authorisation request
        // 02 00 04 01 08 00 8F 76 20 00 06 00 41 00 41 70 6F 74 68 65 6F 73 69 73
        // FE 47 72 61 68 61 6D FE 52 6F 66 66 FE 67 72 6F 66 66 40 75 77 61 74 65
        // 72 6C 6F 6F 2E 63 61 FE 31 FE 50 6C 65 61 73 65 20 61 75 74 68 6F 72 69
        // 7A 65 20 6D 65 2E 00
        T("authorization request from %ld.\n", checkUin);
        packet >> messageLen;
        message = new char[messageLen + 1];
        for (i=0; i<=messageLen; i++)
        {
            packet >> message[i];
            if (message[i] == (char)0xFE) message[i] = '\n';
        }

        sysMsg = new char[messageLen + 128];
      sprintf(sysMsg, "(%s) Authorization request from %ld:\n%s", sm.timeRec(), checkUin, message);
      icqOwner.addMessage(sysMsg, ICQ_CMDxRCV_SYSxMSGxONLINE, ICQ_CMDxRCV_SMxREQxAUTH, 0, timeSent);
      sprintf(sysMsg, "Authorization request from %ld:\n%s", checkUin, message);
      addToSystemMessageHistory(sysMsg);
      playSound(soundSysMsg);
      delete sysMsg;
      delete message;
      break;

   case ICQ_CMDxRCV_SYSxAUTHxGRANTED: // system message: authorization granted

      outputWindow->wprintf("  (%s) Authorization granted from %ld.", sm.timeRec(), checkUin);
      packet >> messageLen;
      message = new char[messageLen + 1];
      for (i = 0; i <= messageLen; i++)
      {
         packet >> message[i];
         if (message[i] == (char)0xFE) message[i] = '\n';
      }

      sysMsg = new char[messageLen + 128];
      sprintf(sysMsg, "(%s) Authorization granted from %ld:\n%s", sm.timeRec(), checkUin, message);
      icqOwner.addMessage(sysMsg, ICQ_CMDxRCV_SYSxMSGxONLINE, ICQ_CMDxRCV_SMxREQxAUTH, 0, timeSent);
      sprintf(sysMsg, "Authorization granted from %ld:\n%s", checkUin, message);
      addToSystemMessageHistory(sysMsg);
      playSound(soundSysMsg);

      delete sysMsg;
      delete message;
      break;
*/
/*
   case ICQ_CMDxRCV_SYSxADDED:  // system message: added to a contact list
      outputWindow->wprintf("  %C(%s) user %C%ld%C added you to their contact list.", COLOR_RECEIVE, sm.timeRec(), COLOR_DATA, checkUin, COLOR_RECEIVE);
      sysMsg = new char[128];
      sprintf(sysMsg, "(%s) User %ld added you to their contact list.", sm.timeRec(), checkUin);
      icqOwner.addMessage(sysMsg, ICQ_CMDxRCV_SYSxMSGxONLINE, ICQ_CMDxRCV_SMxADDED, 0, timeSent);
      sprintf(sysMsg, "User %ld added you to their contact list.", checkUin);
      addToSystemMessageHistory(sysMsg);
      delete sysMsg;
      playSound(soundSysMsg);
*/
      /* there is a bunch of info about the given user in the packet but the read routine to get
         at it is totally broken right now
      int infoLen, j;
      packet >> infoLen;

      // declare all the strings we will need for reading in the user data
      char *userInfo, *aliasField, *firstNameField, *lastNameField, *emailField;
      userInfo =       new char[infoLen];
      aliasField =     new char[infoLen];
      firstNameField = new char[infoLen];
      lastNameField =  new char[infoLen];
      emailField =     new char[infoLen];

      // read in the user data from the packet
      for (i = 0; i < infoLen; i++) packet >> userInfo[i];

      // parse the user info string for the four fields
      i = j = 0;
      do { aliasField[j] = userInfo[i];     i++; j++;} while (userInfo[i] != (char)0xFE);
      aliasField[j] = '\0'; j = 0;
      do { firstNameField[j] = userInfo[i]; i++; j++;} while (userInfo[i] != (char)0xFE);
      firstNameField[j] = '\0';  j = 0;
      do { lastNameField[j] = userInfo[i];  i++; j++;} while (userInfo[i] != (char)0xFE);
      lastNameField[j] = '\0';  j = 0;
      do { emailField[j] = userInfo[i];     i++; j++;} while (i < infoLen);
      emailField[j] = '\0';

      *outputWindow << "  " << aliasField << " (" << firstNameField << " " << lastNameField << "), " << emailField << ".";

      delete userInfo; delete aliasField; delete firstNameField; delete lastNameField; delete emailField;
      break;
      */

    default:
        T("[udp] unknown system packet:\n%s", packet.print());
        break;
    }

    delete [] message;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::ackUDP(unsigned short theSequence)
{
    Packet packet;
    packet << ICQ_VERSION
           << ICQ_CMDxSND_ACK
           << theSequence
           << (unsigned short)0x00
           << uin
           << (unsigned int)0x00;

    T("[udp] sending ack (%d)\n", theSequence);
    udpSocket.sendPacket(packet);
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::ackSYS(unsigned int timeStamp)
{
    Packet packet;
    packet << ICQ_VERSION
           << ICQ_CMDxSND_SYSxMSGxDONExACK
           << sequenceVal
           << sequenceVal
           << uin
           << (unsigned int)0x00
           << timeStamp;

    T("[udp] sending system message ack (%d)\n", sequenceVal);
    sendICQ(udpSocket, packet, ICQ_CMDxSND_SYSxMSGxDONExACK, sequenceVal);
}

///////////////////////////////////////////////////////////////////////////////

ICQUser *ICQ::getUserByUIN(unsigned long uin, bool allowAdd)
{
    unsigned long i;
    ICQUser *u;

    for (i=0; i<icqUsers.size(); i++)
    {
        u = icqUsers[i];
        if (u->uin == uin) return u;
    }

    if (allowAdd)
    {
        T("unknown user %d, adding them to your list\n", uin);
        return addUser(uin, false);
    }

    T("ICQ sent unknown user %d\n", uin);
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

ICQUser *ICQ::getUserByContact(MCONTACT hContact)
{
    unsigned long i;
    ICQUser *u;

//  uin = db_get_dw(ccs->hContact, protoName, "UIN", 0);

    for (i=0; i<icqUsers.size(); i++)
    {
        u = icqUsers[i];
        if (u->hContact == hContact) return u;
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::requestSystemMsg()
{
    // request offline system messages
    // 02 00 4C 04 02 00 50 A5 82 00

    Packet packet;
    packet << ICQ_VERSION
           << ICQ_CMDxSND_SYSxMSGxREQ
           << sequenceVal
           << sequenceVal
           << uin
           << (unsigned int)0x00;

    T("[udp] sending offline system messages request (%d)...\n", sequenceVal);
    sendICQ(udpSocket, packet, ICQ_CMDxSND_SYSxMSGxREQ, sequenceVal);
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::requestBroadcastMsg()
{
    unsigned int timeStamp = db_get_dw(NULL, protoName, "LastBroadcastTime", 0);

    Packet packet;
    packet << ICQ_VERSION
           << ICQ_CMDxSND_BROADCASTxREQ
           << sequenceVal
           << sequenceVal
           << uin
           << (unsigned int)0x00
           << timeStamp
           << (unsigned int)0x00;

    T("[udp] sending offline broadcast messages request (%d)...\n", sequenceVal);
    sendICQ(udpSocket, packet, ICQ_CMDxSND_SYSxMSGxREQ, sequenceVal);
}

///////////////////////////////////////////////////////////////////////////////

bool ICQ::setStatus(unsigned short newStatus)
{
    if (!udpSocket.connected()) return false;

    Packet packet;
    packet << ICQ_VERSION
           << ICQ_CMDxSND_SETxSTATUS
           << sequenceVal
           << sequenceVal
           << uin
           << (unsigned int)0x00
           << toIcqStatus(newStatus);

    T("[udp] sending set status packet (%d)\n", sequenceVal);
    sendICQ(udpSocket, packet, ICQ_CMDxSND_SETxSTATUS, sequenceVal);
   
    desiredStatus = newStatus;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::updateContactList()
{
    char *proto;
    unsigned int i;
    int userCount;
    //HANDLE hContact;
    ICQUser *u;

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
        if (proto && !strcmp(proto, protoName))
        {
            if ((u = getUserByContact(hContact)) == NULL)
            {
                u = new ICQUser();
                u->hContact = hContact;
                u->uin = db_get_dw(hContact, protoName, "UIN", 0);
                icqUsers.push_back(u);
            }
            if (statusVal <= ID_STATUS_OFFLINE) u->setStatus(ID_STATUS_OFFLINE);
            else u->statusVal = db_get_w(hContact, protoName, "Status", ID_STATUS_OFFLINE);
        }
    }

    if (statusVal <= ID_STATUS_OFFLINE) return;


    // create user info packet
    Packet userPacket;
    for (i=0; i<icqUsers.size(); )
    {
		userCount = (unsigned int)icqUsers.size() - i;
        if (userCount > 100) userCount = 100;

        userPacket.clearPacket();
        userPacket << ICQ_VERSION
                   << ICQ_CMDxSND_USERxLIST
                   << sequenceVal
                   << sequenceVal
                   << uin
                   << (unsigned int)0x00
                   << (unsigned char)userCount;

        for (; userCount>0; userCount--) userPacket << icqUsers[i++]->uin;

        // send user info packet
        T("[udp] sending contact list (%d)...\n", sequenceVal);
        sendICQ(udpSocket, userPacket, ICQ_CMDxSND_USERxLIST, sequenceVal);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::sendVisibleList()
{
/*
    unsigned int i, numUsers = 0;
    ICQUser *u;

    if (statusVal != ID_STATUS_INVISIBLE) return;

    Packet userPacket;
    userPacket << ICQ_VERSION
               << ICQ_CMDxSND_VISxLIST
               << sequenceVal
               << sequenceVal
               << uin
               << (unsigned int)0x00;

    for (i=0; i<icqUsers.size(); i++)
    {
        u = icqUsers[i];
        if (u->statusVal != ID_STATUS_OFFLINE && db_get_w(u->hContact, protoName, "ApparentMode", 0) == ID_STATUS_ONLINE)
            numUsers++;
    }

    if (numUsers == 0) return;
    userPacket << (char)numUsers;

    for (i=0; i<icqUsers.size(); i++)
    {
        u = icqUsers[i];
        if (u->statusVal != ID_STATUS_OFFLINE && db_get_w(u->hContact, protoName, "ApparentMode", 0) == ID_STATUS_ONLINE)
            userPacket << icqUsers[i]->uin;
    }

    T("[udp] sending visible list (%d)\n", sequenceVal);
    sendICQ(udpSocket, userPacket, ICQ_CMDxSND_VISxLIST, sequenceVal);
*/    
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::sendInvisibleList()
{
/*
    unsigned int i, numUsers = 0;

    Packet userPacket;
    userPacket << ICQ_VERSION
               << ICQ_CMDxSND_INVISxLIST
               << sequenceVal
               << sequenceVal
               << uin
               << (unsigned int)0x00;

    for (i=0; i<icqUsers.size(); i++)
    {
        if (db_get_w(icqUsers[i]->hContact, protoName, "ApparentMode", 0) == ID_STATUS_OFFLINE)
            numUsers++;
    }

    if (numUsers == 0) return;
    userPacket << (char)numUsers;

    for (i=0; i<icqUsers.size(); i++)
    {
        if (db_get_w(icqUsers[i]->hContact, protoName, "ApparentMode", 0) == ID_STATUS_OFFLINE)
            userPacket << icqUsers[i]->uin;
    }

    T("[udp] sending invisible list (%d)\n", sequenceVal);
    sendICQ(udpSocket, userPacket, ICQ_CMDxSND_INVISxLIST, sequenceVal);
*/    
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::updateUserList(ICQUser *u, char list, char add)
{
/*
    Packet userPacket;
    userPacket << ICQ_VERSION
               << ICQ_CMDxSND_UPDATExLIST
               << sequenceVal
               << sequenceVal
               << uin
               << (unsigned int)0x00
               << u->uin
               << list
               << add;

    T("[udp] update user list (%d)\n", sequenceVal);
    sendICQ(udpSocket, userPacket, ICQ_CMDxSND_UPDATExLIST, sequenceVal);
*/    
}

///////////////////////////////////////////////////////////////////////////////

ICQUser *ICQ::addUser(unsigned int uin, bool persistent)
{
    unsigned int i;
    ICQUser *u;

    for (i=0; i<icqUsers.size(); i++)
    {
        u = icqUsers[i];
        if (u->uin == uin)
        {
            if (persistent)
            {
                db_unset(u->hContact, "CList", "NotOnList");
                db_unset(u->hContact, "CList", "Hidden");
            }
            return u;
        }
    }

    u = new ICQUser();
    u->uin = uin;
	u->hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
    icqUsers.push_back(u);

    CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)u->hContact, (LPARAM)protoName);
    u->setInfo("UIN", uin);

    if (persistent) getUserInfo(u, true);
    else
    {
        db_set_b(u->hContact, "CList", "NotOnList", 1);
        db_set_b(u->hContact, "CList", "Hidden", 1);
    }

    updateContactList();
    return u;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::addNewUser(ICQUser *u)
{
/*
    // update the users info from the server
    if (statusVal != ICQ_STATUS_OFFLINE)
    {
        Packet packet;  // alert server to new user

        packet << ICQ_VERSION
               << ICQ_CMDxSND_USERxADD
               << sequenceVal
               << sequenceVal
               << uin
               << (unsigned int)0x00
               << u->uin;

        T("[udp] alerting server to new user (%d)...\n", sequenceVal);
        sendICQ(udpSocket, packet, ICQ_CMDxSND_USERxADD, sequenceVal);

//      getUserInfo(u);
    }
*/
    updateContactList();
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::removeUser(ICQUser *u)
{
    unsigned int i;

    for (i=0; i<icqUsers.size(); i++) if (icqUsers[i] == u) break;
    if (i == icqUsers.size()) return;

    icqUsers[i] = icqUsers[icqUsers.size() - 1];
    icqUsers.pop_back();

    delete u;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::startSearch(unsigned char skrit, unsigned char smode, char *sstring, unsigned int s)
{
    Packet packet;
    packet << ICQ_VERSION
           << ICQ_CMDxSND_SEARCHxSTART
           << sequenceVal
           << (unsigned short)s
//           << (unsigned short)(icqOwner.sequence1())
           << uin
           << (unsigned int)0x00
           << (unsigned char)0xFF
           << skrit
           << (unsigned char)0x00
           << smode
           << sstring;

    T("[udp] starting search for user (%d)...\n", s);
    sendICQ(udpSocket, packet, ICQ_CMDxSND_SEARCHxSTART, sequenceVal);
}

///////////////////////////////////////////////////////////////////////////////

ICQEvent *ICQ::send(ICQUser *u, unsigned short cmd, char *cmdStr, char *m)
{
    ICQEvent *result;

    if (u->statusVal > ID_STATUS_OFFLINE && (result = sendTCP(u, cmd, cmdStr, m)) != NULL) return result;
    else return sendUDP(u, cmd, cmdStr, m);
}

///////////////////////////////////////////////////////////////////////////////

bool ICQ::openConnection(TCPSocket &socket)
{
    T("[tcp] connecting to %s on port %d...\n", inet_ntoa(*(in_addr*)&socket.remoteIPVal), socket.remotePortVal);
    socket.openConnection();

    if (!socket.connected())
    {
        T("[tcp] connect failed\n");
        return false;
    }

    T("[tcp] connection successful\n");

    Packet packet;
//    packet << ICQ_CMDxTCP_HANDSHAKE3
    packet << (unsigned char)0xFF
           << (unsigned int)0x02
           << (unsigned int)0x00
//           << (unsigned long)tcpSocket.localPortVal
           << uin
           << socket.localIPVal
           << socket.localIPVal
           << (unsigned char)0x04
           << (unsigned int)0x00;
//           << (unsigned long)tcpSocket.localPortVal;

    T("[tcp] sending handshake\n");
    if (!socket.sendPacket(packet))
    {
        T("[tcp] send failed\n");
        return false;
    }

    T("[tcp] setup completed\n");
    return true;
}

///////////////////////////////////////////////////////////////////////////////

ICQEvent *ICQ::sendTCP(ICQUser *u, unsigned short cmd, char *cmdStr, char *m)
{
    if (!u->socket.connected() && !openConnection(u->socket)) return NULL;

    unsigned int status;
    if (accept)
    {
        switch (statusVal)
        {
        case ID_STATUS_ONLINE: status = 0x00100000; break;
        case ID_STATUS_FREECHAT: status = 0x00000000; break;  // ??
        case ID_STATUS_AWAY: status = 0x01100000; break;
        case ID_STATUS_NA: status = 0x00100000; break;
        case ID_STATUS_DND: status = 0x00100000; break;
        case ID_STATUS_OCCUPIED: status = 0x02100000; break;
        case ID_STATUS_INVISIBLE: status = 0x00900000; break;      // ??
        default: status = 0x00100000; break;
        }
    }
    else status = 0x00000001;

    Packet packet;
    packet << uin
           << (unsigned short)0x02            // ICQ_VERSION
           << ICQ_CMDxTCP_START                 // ICQ_CMDxTCP_ACK, ICQ_CMDxTCP_START, ICQ_CMDxTCP_CANCEL
           << (unsigned short)0x00
           << uin
           << cmd
           << m
           << udpSocket.localIPVal
           << udpSocket.localIPVal
           << tcpSocket.localPortVal
           << (unsigned char)0x04
           << status
           << tcpSequenceVal--;

    T("[tcp] sending %s (%d)\n", cmdStr, tcpSequenceVal + 1);
    return sendICQ(u->socket, packet, ICQ_CMDxTCP_START, tcpSequenceVal + 1, u->uin, cmd);
}

///////////////////////////////////////////////////////////////////////////////

ICQEvent *ICQ::sendUDP(ICQUser *u, unsigned short cmd, char *cmdStr, char *m)
{
    Packet packet;
    packet << ICQ_VERSION
           << ICQ_CMDxSND_THRUxSERVER
           << sequenceVal
           << sequenceVal
           << uin
           << (unsigned int)0x00
           << u->uin
           << cmd
           << m;

/*  write for offline multi packet, but not work - little architecturial trouble:
    one big packet must divided on several little packets and Miranda use returned ONE event for control process sending,
    but not several events

    if (packet.size() > 450)
    {
        unsigned int i, j = 0;
        unsigned char c, frameNo, frameSize;

        packet.reset();

        frameSize = (packet.size()+449) / 450;
        for (frameNo=0; frameNo<frameSize; frameNo++)
        {
            Packet frame;
            frame << ICQ_VERSION
                  << ICQ_CMDxSND_MULTI
                  << sequenceVal
                  << sequenceVal
                  << uin
                  << (unsigned int)0x00
                  << frameSize
                  << frameNo;

            for (i=0; i<450 && j<packet.size(); i++, j++)
            {
                packet >> c;
                frame << c;
            }

            T("[udp] sending %s through server, part %d of %d (%d)\n", cmdStr, frameNo, frameSize, sequenceVal);
            sendICQ(udpSocket, packet, ICQ_CMDxSND_THRUxSERVER, sequenceVal, u->uin, cmd);
        }
    }
    else
*/
    {
        T("[udp] sending %s through server (%d)\n", cmdStr, sequenceVal);
        return sendICQ(udpSocket, packet, ICQ_CMDxSND_THRUxSERVER, sequenceVal, u->uin, cmd);
    }
}

///////////////////////////////////////////////////////////////////////////////

ICQEvent *ICQ::sendMessage(ICQUser *u, char *m)
{
    return send(u, ICQ_CMDxTCP_MSG, "message", m);
}

///////////////////////////////////////////////////////////////////////////////

ICQEvent *ICQ::sendUrl(ICQUser *u, char *url)
{
    unsigned int nameLen, descriptionLen;
    char *m, *description;
    ICQEvent *result;

	nameLen = (unsigned int)mir_strlen(url);
    description = (char*)url + nameLen + 1;
	descriptionLen = (unsigned int)mir_strlen(description);

    m = new char[nameLen + descriptionLen + 2];
    strcpy(m, description);
    strcpy(m + descriptionLen + 1, url);
    m[descriptionLen] = (char)0xFE;

    result = send(u, ICQ_CMDxTCP_URL, "url", m);
    delete [] m;

    return result;
}

///////////////////////////////////////////////////////////////////////////////

ICQEvent *ICQ::sendReadAwayMsg(ICQUser *u)
{
    unsigned short cmd;

    switch (u->statusVal)
    {
    case ID_STATUS_AWAY: cmd = ICQ_CMDxTCP_READxAWAYxMSG; break;
    case ID_STATUS_DND: cmd = ICQ_CMDxTCP_READxDNDxMSG; break;
    case ID_STATUS_NA: cmd = ICQ_CMDxTCP_READxNAxMSG; break;
    case ID_STATUS_OCCUPIED: cmd = ICQ_CMDxTCP_READxOCCUPIEDxMSG; break;
    case ID_STATUS_FREECHAT: cmd = ICQ_CMDxTCP_READxFREECHATxMSG; break;
    default: return NULL;
    }

    return sendTCP(u, cmd, "away message request", "");
}

///////////////////////////////////////////////////////////////////////////////

ICQTransfer *ICQ::sendFile(ICQUser *u, char *description, char *filename, unsigned int size, char **files)
{
    if (!u->socket.connected() && !openConnection(u->socket)) return NULL;

    unsigned int i;

    ICQTransfer *transfer = new ICQTransfer(u, tcpSequenceVal);

	for (i=0; files[i]; i++);
    transfer->files = new char*[i + 1];
	for (i=0; files[i]; i++) transfer->files[i] = _strdup(files[i]);
    transfer->files[i] = 0;

    transfer->description = _strdup(description);
    transfer->count = i;
    transfer->totalSize = size;

    transfer->path = _strdup(files[0]);
    *strrchr(transfer->path, '\\') = 0;

    icqTransfers.push_back(transfer);
    transfer->ack(ACKRESULT_SENTREQUEST);


    unsigned short cmd = ICQ_CMDxTCP_FILE;
    char *m = description;

    unsigned int status;
    if (accept)
    {
        switch (statusVal)
        {
        case ID_STATUS_ONLINE: status = 0x00100000; break;
        case ID_STATUS_FREECHAT: status = 0x00000000; break;  // ??
        case ID_STATUS_AWAY: status = 0x01100000; break;
        case ID_STATUS_NA: status = 0x00100000; break;
        case ID_STATUS_DND: status = 0x00100000; break;
        case ID_STATUS_OCCUPIED: status = 0x02100000; break;
        case ID_STATUS_INVISIBLE: status = 0x00900000; break;      // ??
        default: status = 0x00100000; break;
        }
    }
    else status = 0x00000001;

    Packet packet;
    packet << uin
           << (unsigned short)0x02            // ICQ_VERSION
           << ICQ_CMDxTCP_START                 // ICQ_CMDxTCP_ACK, ICQ_CMDxTCP_START, ICQ_CMDxTCP_CANCEL
           << (unsigned short)0x00
           << uin
           << cmd
           << m
           << udpSocket.localIPVal
           << udpSocket.localIPVal
           << tcpSocket.localPortVal
           << (unsigned char)0x04
           << status;


    packet << (unsigned int)0x00
           << filename
           << size
           << (unsigned int)0x00;

    packet << tcpSequenceVal--;

    T("[tcp] sending file request (%d)\n", tcpSequenceVal + 1);
    sendICQ(u->socket, packet, ICQ_CMDxTCP_START, tcpSequenceVal + 1, u->uin, cmd);
    return transfer;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::acceptFile(ICQUser *u, unsigned long hTransfer, char *path)
{
    unsigned int theSequence = hTransfer;
    unsigned short cmd = ICQ_CMDxTCP_FILE;
    char m[1] = { 0 };

    unsigned long status;
    if (accept)
    {
        switch (statusVal)
        {
        case ID_STATUS_ONLINE: status = 0x00100000; break;
        case ID_STATUS_FREECHAT: status = 0x00000000; break;  // ??
        case ID_STATUS_AWAY: status = 0x01100000; break;
        case ID_STATUS_NA: status = 0x00100000; break;
        case ID_STATUS_DND: status = 0x00100000; break;
        case ID_STATUS_OCCUPIED: status = 0x02100000; break;
        case ID_STATUS_INVISIBLE: status = 0x00900000; break;      // ??
        default: status = 0x00100000; break;
        }
    }
    else status = 0x00000001;

    Packet packet;
    packet << uin
           << (unsigned short)0x02            // ICQ_VERSION
           << ICQ_CMDxTCP_ACK                 // ICQ_CMDxTCP_ACK, ICQ_CMDxTCP_START, ICQ_CMDxTCP_CANCEL
           << (unsigned short)0x00
           << uin
           << cmd
           << m
           << udpSocket.localIPVal
           << udpSocket.localIPVal
           << tcpSocket.localPortVal
           << (unsigned char)0x04
//           << status;
           << (unsigned int)0x00;

    packet << (unsigned int)htons(tcpSocket.localPortVal)
           << m
           << (unsigned int)0x00
           << tcpSocket.localPortVal;

    packet << theSequence;

    T("[tcp] sending accept file ack (%d)\n", theSequence);
    u->socket.sendPacket(packet);
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::refuseFile(ICQUser *u, unsigned long hTransfer, char *reason)
{
    unsigned int theSequence = hTransfer;
    unsigned short cmd = ICQ_CMDxTCP_FILE;
    char m[1] = { 0 };

    unsigned int status;
    if (accept)
    {
        switch (statusVal)
        {
        case ID_STATUS_ONLINE: status = 0x00100000; break;
        case ID_STATUS_FREECHAT: status = 0x00000000; break;  // ??
        case ID_STATUS_AWAY: status = 0x01100000; break;
        case ID_STATUS_NA: status = 0x00100000; break;
        case ID_STATUS_DND: status = 0x00100000; break;
        case ID_STATUS_OCCUPIED: status = 0x02100000; break;
        case ID_STATUS_INVISIBLE: status = 0x00900000; break;      // ??
        default: status = 0x00100000; break;
        }
    }
    else status = 0x00000001;

    Packet packet;
    packet << uin
           << (unsigned short)0x02            // ICQ_VERSION
           << ICQ_CMDxTCP_ACK                   // ICQ_CMDxTCP_ACK, ICQ_CMDxTCP_START, ICQ_CMDxTCP_CANCEL
           << (unsigned short)0x00
           << uin
           << cmd
           << reason
           << udpSocket.localIPVal
           << udpSocket.localIPVal
           << tcpSocket.localPortVal
           << (unsigned char)0x04
//           << status;
           << (unsigned int)0x00000001;

    packet << (unsigned int)0x00
           << m
           << (unsigned int)0x00
           << (unsigned int)0x00;

    packet << theSequence;

    T("[tcp] sending refuse file ack (%d)\n", theSequence);
    u->socket.sendPacket(packet);
}

///////////////////////////////////////////////////////////////////////////////

bool ICQ::getUserInfo(ICQUser *u, bool basicInfo)
{
    unsigned short cmd = basicInfo ? ICQ_CMDxSND_USERxGETxBASICxINFO : ICQ_CMDxSND_USERxGETxINFO;

    Packet request;
    request << ICQ_VERSION
            << cmd
            << sequenceVal
            << sequenceVal
            << uin
            << (unsigned int)0x00
            << u->uin;

    T("[udp] sending user %s info request (%d)...\n", basicInfo ? "basic" : "details", sequenceVal);
    sendICQ(udpSocket, request, cmd, sequenceVal, u->uin, 0, basicInfo ? 1 : 5);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::authorize(unsigned int uinToAuthorize)
{
    Packet packet;
    packet << ICQ_VERSION
           << ICQ_CMDxSND_AUTHORIZE
           << sequenceVal
           << sequenceVal
           << uin
           << (unsigned int)0x00
           << uinToAuthorize
           << (unsigned int)0x00010008   // who knows, seems to be constant
           << (unsigned char)0x00;

    T("[udp] sending authorization (%d)\n", sequenceVal);
    sendICQ(udpSocket, packet, ICQ_CMDxSND_AUTHORIZE, sequenceVal);
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::processTcpPacket(Packet &packet, unsigned int hSocket)
{
    unsigned int i, checkUin, senderIp, localIp, userStatus, senderPort, junkLong, thePort, theTCPSequence;
    unsigned short version, command, junkShort, newCommand, /*messageLen,*/ cicqVersion;
    unsigned char cicqChar, junkChar;
    char *message = NULL;
    ICQUser *u;
    static unsigned int chatUin, chatSequence;

    packet >> checkUin
           >> version
           >> command      // so far either message stuff or message ack
           >> junkShort    // 00 00 to fill in the MSB of the command long int which is read in as a short
           >> checkUin
           >> newCommand   // if a message then what type, message/chat/read away message/...
           >> message
           >> senderIp
           >> localIp
           >> senderPort
           >> junkChar
           >> userStatus;

    u = getUserByUIN(checkUin);
    switch(command)
    {
    case ICQ_CMDxTCP_START: // incoming tcp packet containing one of many possible things
        switch(newCommand)  // do a switch on what it could be
        {
        case ICQ_CMDxTCP_MSG:  // straight message from a user
            T("[tcp] message from %d.\n", checkUin);

            packet >> theTCPSequence;

            ackTCP(packet, u, newCommand, theTCPSequence);
            addMessage(u, message, ICQ_CMDxTCP_START, ICQ_CMDxTCP_MSG, theTCPSequence, time(NULL));
            break;

        case ICQ_CMDxTCP_CHAT:
            T("[tcp] chat request from %d.\n", checkUin);

            packet >> junkLong
                   >> junkLong
                   >> junkShort
                   >> junkChar
                   >> theTCPSequence
                   >> cicqChar
                   >> cicqVersion;
/*
            char *chatReq;
            chatReq = new char[messageLen + 32];
            sprintf(chatReq, "Chat request:\n   %s", message);
//            u->addMessage(chatReq, ICQ_CMDxTCP_START, ICQ_CMDxTCP_CHAT, theTCPSequence, 0, (cicqChar == 'C'), cicqVersion);
            delete chatReq;
*/
            break;

        case ICQ_CMDxTCP_URL:  // url sent
            T("[tcp] url from %d.\n", checkUin);

            packet >> theTCPSequence;

            ackTCP(packet, u, newCommand, theTCPSequence);
            addUrl(u, message, ICQ_CMDxTCP_START, ICQ_CMDxTCP_URL, theTCPSequence, time(NULL));
            break;

        case ICQ_CMDxTCP_FILE:
            unsigned int size;
            char *fileName;

            fileName = NULL;
            packet >> junkLong
                   >> fileName
                   >> size
                   >> junkLong
                   >> theTCPSequence;

            T("[tcp] file transfer request from %d (%d)\n", checkUin, theTCPSequence);

            addFileReq(u, message, fileName, size, ICQ_CMDxTCP_START, ICQ_CMDxTCP_FILE, theTCPSequence, time(NULL));
            delete [] fileName;
            break;

        case ICQ_CMDxTCP_READxAWAYxMSG: // read away message
        case ICQ_CMDxTCP_READxOCCUPIEDxMSG:
        case ICQ_CMDxTCP_READxNAxMSG:
        case ICQ_CMDxTCP_READxDNDxMSG:
        case ICQ_CMDxTCP_READxFREECHATxMSG:
            T("[tcp] %d requested read of away message.\n", checkUin);

            packet >> theTCPSequence;
            ackTCP(packet, u, newCommand, theTCPSequence);
            break;
        }
        break;

    case ICQ_CMDxTCP_ACK:  // message received packet
        switch (newCommand)
        {
        case ICQ_CMDxTCP_MSG:
            packet >> theTCPSequence;
            break;

		case ICQ_CMDxTCP_CHAT:
            packet >> junkShort
                   >> junkChar
                   >> junkLong   // port backwards
                   >> thePort    // port to connect to for chat
                   >> theTCPSequence;

            if (chatSequence != theTCPSequence || chatUin != checkUin)  // only if this is the first chat ack packet
            {                                                        // do we do anything
                chatSequence = theTCPSequence;
                chatUin = checkUin;
//                emit eventResult(u, ICQ_CMDxTCP_CHAT, userStatus == 0x0000 ? true : false, thePort);
            }
            break;

		case ICQ_CMDxTCP_URL:
            packet >> theTCPSequence;
            break;

		case ICQ_CMDxTCP_FILE:
            packet >> junkLong
                   >> junkShort
                   >> junkChar
                   >> junkLong
                   >> thePort
                   >> theTCPSequence;

            T("[tcp] file transfer ack from %d (%d)\n", u->uin, theTCPSequence);

            ICQTransfer *t;
            for (i=0; i<icqTransfers.size(); i++)
            {
                t = icqTransfers[i];
                if (t->uin == checkUin && !t->socket.connected())
                {
                    if (userStatus != 0)
                    {
                        T("[tcp] file transfer denied by %d\n", checkUin);
                        ProtoBroadcastAck(protoName, t->hContact, ACKTYPE_FILE, ACKRESULT_DENIED, t, 0);
                        delete t;
                        icqTransfers[i] = icqTransfers[icqTransfers.size() - 1];
                        icqTransfers.pop_back();
                        break;
                    }

                    if (!t->socket.setDestination(u->socket.remoteIPVal, thePort))
                    {
                        T("[tcp] can't set destination\n");
                        break;
                    }
                    t->ack(ACKRESULT_CONNECTING);
                    if (openConnection(t->socket))
                    {
                        t->ack(ACKRESULT_CONNECTED);
                        t->sendPacket0x00();
                    }
                    break;
                }
            }
            break;

        case ICQ_CMDxTCP_READxAWAYxMSG:
        case ICQ_CMDxTCP_READxOCCUPIEDxMSG:
        case ICQ_CMDxTCP_READxNAxMSG:
        case ICQ_CMDxTCP_READxDNDxMSG:
        case ICQ_CMDxTCP_READxFREECHATxMSG:
            packet >> theTCPSequence;
            addAwayMsg(u, message, ICQ_CMDxTCP_START, ICQ_CMDxTCP_READxAWAYxMSG, theTCPSequence, time(NULL));
            break;
        }

        // output the away message if there is one (ie if user status is not online)
        if (userStatus == 0x0000) T("[tcp] ack from %d (%d).\n", u->uin, theTCPSequence);
        else if (userStatus == 0x0001)
        {
            T("[tcp] refusal from %d (%d): %s\n", u->uin, theTCPSequence, message);
        }
        else
        {
//            u->setAwayMessage(message);
            T("[tcp] ack from %d (%d).\n", u->uin, theTCPSequence);
//            T("[tcp] ack from %d (%ld): %s\n", u->uin, theTCPSequence, message);
        }

        doneEvent(true, hSocket, theTCPSequence);
        break;

    case ICQ_CMDxTCP_CANCEL:
        switch (newCommand)
        {
        case ICQ_CMDxTCP_CHAT:
            T("[tcp] chat request from %d (%d) cancelled.\n", checkUin, theTCPSequence);
//            u->addMessage(chatReq, ICQ_CMDxTCP_CANCEL, ICQ_CMDxTCP_CHAT, 0);
            break;

        case ICQ_CMDxTCP_FILE:
            T("[tcp] file transfer request from %d (%d) cancelled.\n", u->uin, theTCPSequence);
//            u->addMessage(fileReq, ICQ_CMDxTCP_CANCEL, ICQ_CMDxTCP_FILE, 0);
            break;
        }
        break;

    default:
        T("[tcp] unknown packet:\n%s", packet.print());
        packet.reset();
    }
    delete [] message;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::ackTCP(Packet &packet, ICQUser *u, unsigned short newCommand, unsigned int theSequence)
{
    unsigned int status;

    switch (statusVal)
    {
    case ID_STATUS_ONLINE: status = 0x00100000; break;
    case ID_STATUS_FREECHAT: status = 0x00000000; break;    // ??
    case ID_STATUS_AWAY: status = 0x01100000; break;
    case ID_STATUS_NA: status = 0x00100000; break;
    case ID_STATUS_DND: status = 0x00100000; break;
    case ID_STATUS_OCCUPIED: status = 0x02100000; break;
    case ID_STATUS_INVISIBLE: status = 0x00900000; break;   // ??
    default: status = 0x00100000; break;
    }

    packet.clearPacket();
    packet << uin
           << (unsigned short)0x02
           << (unsigned short)ICQ_CMDxTCP_ACK // ICQ_CMDxTCP_ACK, ICQ_CMDxTCP_START, ICQ_CMDxTCP_CANCEL
           << (unsigned short)0x00
           << uin
           << newCommand
           << awayMessage
           << u->socket.localIPVal
           << u->socket.localIPVal
           << u->socket.localPortVal
           << (unsigned char)0x04
           << status
           << theSequence;

    T("[tcp] sending ack (%d)\n", theSequence);
    u->socket.sendPacket(packet);
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::recvUDP(int)
{
    Packet packet;

    // mirabilis contacts us using udp on this server
    if (udpSocket.receivePacket(packet)) processUdpPacket(packet);
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::recvNewTCP(int)
{
    ICQUser *u;
    Packet handshake;

    // our tcp incoming server
    TCPSocket newSocket(0);
    tcpSocket.receiveConnection(newSocket);
    newSocket.receivePacket(handshake);

    unsigned int ulJunk, newUin, localHost;
    unsigned short command, usJunk;
    unsigned char ucJunk;

    handshake >> command;

    if (command != ICQ_CMDxTCP_HANDSHAKE && command != ICQ_CMDxTCP_HANDSHAKE2 && command != ICQ_CMDxTCP_HANDSHAKE3)
    {
        T("[tcp] garbage packet:\n%s", handshake.print());
        handshake.reset();
    }
    else
    {
        handshake >> ulJunk
                  >> usJunk
                  >> ucJunk
                  >> newUin
                  >> localHost
                  >> localHost
                  >> ulJunk
                  >> ucJunk;

        u = getUserByUIN(newUin);
        if (!u->socket.connected())
        {
            T("[tcp] connection from uin %d.\n", newUin);
            u->socket.transferConnectionFrom(newSocket);
        }
        else
        {
            unsigned int i;
            ICQTransfer *t;

            T("[tcp] file direct connection from uin %d.\n", newUin);
            for (i=0; i<icqTransfers.size(); i++)
            {
                t = icqTransfers[i];
                if (t->uin == newUin && !t->socket.connected()) t->socket.transferConnectionFrom(newSocket);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::recvTCP(SOCKET hSocket)
{
    unsigned int i;
    Packet packet;
    ICQUser *u;

    for(i=0; i<icqUsers.size(); i++)
    {
        u = icqUsers[i];
        if (u->socket.handleVal == hSocket)
        {
            if (!u->socket.receivePacket(packet))
            {
                T("[tcp] connection to %d lost.\n", u->uin);
                return;
            }
            processTcpPacket(packet, hSocket);
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::recvTransferTCP(SOCKET hSocket)
{
    unsigned int i;
    Packet packet;
    ICQTransfer *transfer;

    for(i=0; i<icqTransfers.size(); i++)
    {
        transfer = icqTransfers[i];
        if (transfer->socket.handleVal == hSocket)
        {
            if (!transfer->socket.receivePacket(packet))
            {
//                T("[tcp] connection to %d lost.\n", s->uin);
                return;
            }
            transfer->processTcpPacket(packet);
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::addMessage(ICQUser *u, char *m, unsigned short theCmd, unsigned short theSubCmd, unsigned long theSequence, time_t t)
{
    CCSDATA ccs;
    PROTORECVEVENT pre;

    T("message: %s\n", m);

    ccs.hContact = u->hContact;
    ccs.szProtoService = PSR_MESSAGE;
    ccs.wParam = 0;
    ccs.lParam = (LPARAM)&pre;
    pre.flags = 0;
    pre.timestamp = t;
    pre.szMessage = (char*)m;
    pre.lParam = 0;
    CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::addUrl(ICQUser *u, char *m, unsigned short theCmd, unsigned short theSubCmd, unsigned long theSequence, time_t t)
{
    unsigned int i, messageLen;
    char *url;
    CCSDATA ccs;
    PROTORECVEVENT pre;

    messageLen = mir_tstrlen(m);
    for (i=0; i<messageLen; i++) if (m[i] == (char)0xFE) m[i] = 0;

    url = new char[messageLen + 1];
    lstrcpy(url, m + mir_tstrlen(m) + 1);
    lstrcpy(url + mir_tstrlen(url) + 1, m);

    ccs.hContact = u->hContact;
    ccs.szProtoService = PSR_URL;
    ccs.wParam = 0;
    ccs.lParam = (LPARAM)&pre;
    pre.flags = 0;
    pre.timestamp = t;
    pre.szMessage = url;
    pre.lParam = 0;
    CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);

    delete [] url;
}

///////////////////////////////////////////////////////////////////////////////

void ICQ::addAwayMsg(ICQUser *u, char *m, unsigned short theCmd, unsigned short theSubCmd, unsigned long theSequence, time_t t)
{
    CCSDATA ccs;
    PROTORECVEVENT pre;

    T("away msg: %s\n", m);

    ccs.hContact = u->hContact;
    ccs.szProtoService = PSR_AWAYMSG;
    ccs.wParam = u->statusVal;
    ccs.lParam = (LPARAM)&pre;
    pre.flags = 0;
    pre.timestamp = t;
    pre.szMessage = (char*)m;
    pre.lParam = theSequence;
    CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
}

///////////////////////////////////////////////////////////////////////////////
/*
void ICQ::addAdded(ICQUser *u, char *m, unsigned short theCmd, unsigned short theSubCmd, unsigned long theSequence, time_t t)
{
    DBEVENTINFO dbei;
    PBYTE pCurBlob;

    //blob is: uin(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
    ZeroMemory(&dbei, sizeof(dbei));
    dbei.cbSize=sizeof(dbei);
    dbei.szModule="ICQ";
    dbei.timestamp=TimestampLocalToGMT(YMDHMSToTime(year,month,day,hour,minute,0));
    dbei.flags=0;
    dbei.eventType=EVENTTYPE_ADDED;
    dbei.cbBlob=sizeof(DWORD)+mir_strlen(nick)+mir_strlen(first)+mir_strlen(last)+mir_strlen(email)+4;
    pCurBlob=dbei.pBlob=(PBYTE)malloc(dbei.cbBlob);
    CopyMemory(pCurBlob,&uin,sizeof(DWORD)); pCurBlob+=sizeof(DWORD);
    CopyMemory(pCurBlob,nick,mir_strlen(nick)+1); pCurBlob+=mir_strlen(nick)+1;
    CopyMemory(pCurBlob,first,mir_strlen(first)+1); pCurBlob+=mir_strlen(first)+1;
    CopyMemory(pCurBlob,last,mir_strlen(last)+1); pCurBlob+=mir_strlen(last)+1;
    CopyMemory(pCurBlob,email,mir_strlen(email)+1); pCurBlob+=mir_strlen(email)+1;
    CallService(MS_DB_EVENT_ADD,(WPARAM)(HANDLE)NULL,(LPARAM)&dbei);
}
*/
///////////////////////////////////////////////////////////////////////////////

void ICQ::addFileReq(ICQUser *u, char *m, char *filename, unsigned long size, unsigned short theCmd, unsigned short theSubCmd, unsigned long theSequence, time_t t)
{
    CCSDATA ccs;
    PROTORECVEVENT pre;
    char *szBlob;
/*
    filetransfer *ft;

    // Initialize a filetransfer struct
    ft = new filetransfer;
    memset(ft, 0, sizeof(filetransfer));
    ft->status = 0;
    ft->wCookie = wCookie;
    ft->szFilename = _strdup(fileName);
    ft->szDescription = _strdup(m);
    ft->dwUin = u->uin;
    ft->fileId = -1;
    ft->dwTotalSize = size;
    ft->nVersion = nVersion;
    ft->TS1 = dwID1;
    ft->TS2 = dwID2;
*/
    ICQTransfer *transfer = new ICQTransfer(u, theSequence);
    transfer->description = _strdup(m);
    transfer->totalSize = size;

    icqTransfers.push_back(transfer);



    // Send chain event
    szBlob = new char[sizeof(DWORD) + mir_strlen(filename) + mir_strlen(m) + 2];

    *(PDWORD)szBlob = (DWORD)transfer;
    strcpy(szBlob + sizeof(DWORD), filename);
    strcpy(szBlob + sizeof(DWORD) + mir_strlen(filename) + 1, m);

    ccs.hContact = u->hContact;
    ccs.szProtoService = PSR_FILE;
    ccs.wParam = 0;
    ccs.lParam = (LPARAM)&pre;
    pre.flags = 0;
    pre.timestamp = t;
    pre.szMessage = szBlob;
    pre.lParam = theSequence;
    CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);

    delete [] szBlob;

}

///////////////////////////////////////////////////////////////////////////////

void ICQ::doneUserFcn(bool ack, ICQEvent *icqEvent)
{
    unsigned int type = 0;

    if (icqEvent->subCmd == ICQ_CMDxTCP_MSG) type = ACKTYPE_MESSAGE;
    if (icqEvent->subCmd == ICQ_CMDxTCP_URL) type = ACKTYPE_URL;

    ProtoBroadcastAck(protoName, getUserByUIN(icqEvent->uin)->hContact, type, ack ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)icqEvent->sequence, 0);
}

///////////////////////////////////////////////////////////////////////////////
