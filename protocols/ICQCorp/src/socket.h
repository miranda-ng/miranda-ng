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

#ifndef socket_h
#define socket_h

#include "packet.h"

///////////////////////////////////////////////////////////////////////////////

class Socket
{
public:
    SOCKET handleVal;
    unsigned int remoteIPVal, remotePortVal;
    unsigned int localIPVal, localPortVal;
    unsigned int messageVal;
    Socket();
    ~Socket();
    bool connected();
    bool completed(int result);
    bool setDestination(unsigned long rIp, unsigned short rPort);
    bool setDestination(char *rHost, unsigned short rPort);
    void resetSocket();
    unsigned long lookup(char *h);
    bool openConnection(int Type);
    void closeConnection();
    virtual bool startServer() = 0;
    virtual bool sendPacket(Packet &p) = 0;
    virtual bool receivePacket(Packet &p) = 0;
protected:
    struct sockaddr_in remote;
    void openSocket();
    void closeSocket();
    bool sendBuffer(char *buffer, unsigned int bufferSize);
    bool receiveBuffer(char *buffer, unsigned int bufferSize);
};

///////////////////////////////////////////////////////////////////////////////

class TCPSocket : public Socket
{
public:
    TCPSocket(unsigned int msg);
    bool openConnection();
    virtual bool startServer();
    virtual bool sendPacket(Packet &p);
    virtual bool receivePacket(Packet &p);
    bool sendPacket(Packet &p, bool sendSize);
    bool receivePacket(Packet &p, bool recvSize);
    void receiveConnection(TCPSocket &newSocket);
    void transferConnectionFrom(TCPSocket &from);
};

///////////////////////////////////////////////////////////////////////////////

class UDPSocket : public Socket
{
public:
    UDPSocket(unsigned int msg);
    bool openConnection();
    virtual bool startServer();
    virtual bool sendPacket(Packet &p);
    virtual bool receivePacket(Packet &p);
};

///////////////////////////////////////////////////////////////////////////////

#endif
