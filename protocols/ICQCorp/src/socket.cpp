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

///////////////////////////////////////////////////////////////////////////////
//
//  Socket
//
///////////////////////////////////////////////////////////////////////////////

Socket::Socket()
{
    handleVal = INVALID_SOCKET;
    remoteIPVal = 0;
    remotePortVal = 0;
    localIPVal = 0;
    localPortVal = 0;
    messageVal = 0;
}

///////////////////////////////////////////////////////////////////////////////

Socket::~Socket()
{
    if (connected()) closeConnection();
}

///////////////////////////////////////////////////////////////////////////////

bool Socket::connected()
{
    return handleVal != INVALID_SOCKET;
};

///////////////////////////////////////////////////////////////////////////////

bool Socket::completed(int result)
{
    if (result != SOCKET_ERROR || WSAGetLastError() != WSAEWOULDBLOCK) return true;

    Sleep(1);
    return false;
};

///////////////////////////////////////////////////////////////////////////////

void Socket::openSocket()
{
    if (connected()) WSAAsyncSelect(handleVal, icq.hWnd, messageVal, FD_READ|FD_ACCEPT|FD_CONNECT|FD_CLOSE);
}

///////////////////////////////////////////////////////////////////////////////

void Socket::closeSocket()
{
    if (connected()) WSAAsyncSelect(handleVal, icq.hWnd, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////

bool Socket::setDestination(unsigned long rIp, unsigned short rPort)
{
    char localName[128];

    if (rIp == 0) return false;
    gethostname(localName, 128);
    localIPVal = lookup(localName);

    // set up remote connection
    remoteIPVal = rIp;
    remotePortVal = rPort;
    remote.sin_port = htons(rPort);
    remote.sin_addr.s_addr = rIp;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool Socket::setDestination(char *rHost, unsigned short rPort)
{
    return setDestination(lookup(rHost), rPort);
};

///////////////////////////////////////////////////////////////////////////////

unsigned long Socket::lookup(char *h)
{
    unsigned long ina;
    struct hostent *host; 

    // check if the hostname is in dot and number notation
    ina = inet_addr(h);
    if (ina != INADDR_NONE) return ina;

    // try and resolve hostname
    if ((host = gethostbyname(h)) == NULL) return 0;  // Couldn't resolve hostname/ip
   
    // return the ip
    return *((unsigned long *)host->h_addr);
}

///////////////////////////////////////////////////////////////////////////////

bool Socket::openConnection(int Type)
{  
    struct sockaddr_in local;
   
    handleVal = socket(AF_INET, Type, 0);
    if (handleVal == INVALID_SOCKET) return false;

    memset(&local.sin_zero, 0, 8);
    local.sin_family = AF_INET;
    local.sin_port = htons(localPortVal);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
//    local.sin_addr.s_addr = htonl(localIPVal);

    memset(&remote.sin_zero, 0, 8);
    remote.sin_family = AF_INET;

    bind(handleVal, (sockaddr*)&local, sizeof(sockaddr));

    int sizeofSockaddr = sizeof(sockaddr);
    getsockname(handleVal, (sockaddr*)&local, &sizeofSockaddr);
    localPortVal = ntohs(local.sin_port);
    if (local.sin_addr.s_addr) localIPVal = ntohl(local.sin_addr.s_addr);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void Socket::closeConnection()
{
    if (connected())
    {
        closeSocket();
        shutdown(handleVal, SD_BOTH);
        closesocket(handleVal);
        handleVal = INVALID_SOCKET;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Socket::sendBuffer(char *buffer, unsigned int bufferSize)
{
    int result;

    while (bufferSize > 0)
    {
        while (! completed(result = send(handleVal, buffer, bufferSize, 0)));
        if (result <= 0)
        {
            closeConnection();
            return false;
        }
        buffer += result;
        bufferSize -= result;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool Socket::receiveBuffer(char *buffer, unsigned int bufferSize)
{
    int result;

    while (bufferSize > 0)
    {
        while (! completed(result = recv(handleVal, buffer, bufferSize, 0)));
        if (result <= 0)
        {
            closeConnection();
            return false;
        }
        buffer += result;
        bufferSize -= result;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
//  TCPSocket
//
///////////////////////////////////////////////////////////////////////////////

TCPSocket::TCPSocket(unsigned int msg)
{
    messageVal = msg;
}

///////////////////////////////////////////////////////////////////////////////

bool TCPSocket::openConnection()
{
    int result;

    if (remoteIPVal == 0) return false;
    if (! Socket::openConnection(SOCK_STREAM)) return false;

    result = connect(handleVal, (sockaddr*)&remote, sizeof(sockaddr));
    if (result == SOCKET_ERROR)
    {
        closeConnection();
        return false;
    }
    openSocket();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void TCPSocket::receiveConnection(TCPSocket &newSocket)
{
    int sizeofSockaddr = sizeof(sockaddr);
    newSocket.handleVal = accept(handleVal, (sockaddr*)&remote, &sizeofSockaddr);
    newSocket.openSocket();

    sizeofSockaddr = sizeof(sockaddr);
    getpeername(newSocket.handleVal, (sockaddr*)&newSocket.remote, &sizeofSockaddr);

    newSocket.remoteIPVal = newSocket.remote.sin_addr.s_addr;
    newSocket.remotePortVal = ntohs(newSocket.remote.sin_port);

    newSocket.localIPVal = localIPVal;
    newSocket.localPortVal = localPortVal;
}

///////////////////////////////////////////////////////////////////////////////

bool TCPSocket::startServer()
{
    if (! Socket::openConnection(SOCK_STREAM)) return false;
    listen(handleVal, 10);
    openSocket();
    return true;
}                         

///////////////////////////////////////////////////////////////////////////////

void TCPSocket::transferConnectionFrom(TCPSocket &from)
{
    handleVal = from.handleVal;
    remoteIPVal = from.remoteIPVal;
    remotePortVal = from.remotePortVal;
    localIPVal = from.localIPVal;
    localPortVal = from.localPortVal;

    from.closeSocket();
    from.handleVal = INVALID_SOCKET;
    openSocket();
}

///////////////////////////////////////////////////////////////////////////////

bool TCPSocket::sendPacket(Packet &p)
{
    return sendPacket(p, true);
}

///////////////////////////////////////////////////////////////////////////////

bool TCPSocket::receivePacket(Packet &p)
{
    return receivePacket(p, true);
}

///////////////////////////////////////////////////////////////////////////////

bool TCPSocket::sendPacket(Packet &p, bool sendSize)
{
    if (! connected()) return false;
    if (sendSize)
    {
        char pSize[2];

        pSize[0] = (p.sizeVal) & 0xFF;
        pSize[1] = (p.sizeVal >> 8) & 0xFF;

        if (! sendBuffer(pSize, 2)) return false;
    }
    return sendBuffer(p.buff, p.sizeVal);
}

///////////////////////////////////////////////////////////////////////////////

bool TCPSocket::receivePacket(Packet &p, bool recvSize)
{
    unsigned short sizeVal;

    if (! connected()) return false;
    WSAAsyncSelect(handleVal, icq.hWnd, messageVal, FD_ACCEPT|FD_CONNECT|FD_CLOSE);

    p.clearPacket();
    if (recvSize)
    {
        if (!receiveBuffer((char*)&sizeVal, 2) || sizeVal > p.maxSize) return false;
    }
    else sizeVal = 1;

    if (! receiveBuffer(p.buff, sizeVal)) return false;
    p.sizeVal = sizeVal;

    WSAAsyncSelect(handleVal, icq.hWnd, messageVal, FD_READ|FD_ACCEPT|FD_CONNECT|FD_CLOSE);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
//  UDPSocket
//
///////////////////////////////////////////////////////////////////////////////

UDPSocket::UDPSocket(unsigned int msg)
{
    messageVal = msg;
}

///////////////////////////////////////////////////////////////////////////////

bool UDPSocket::openConnection()
{  
    if (remoteIPVal == 0) return false;
    if (! Socket::openConnection(SOCK_DGRAM)) return false;
    openSocket();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UDPSocket::startServer()
{
    if (! Socket::openConnection(SOCK_DGRAM)) return false;
    openSocket();
    return true;
}      

///////////////////////////////////////////////////////////////////////////////

bool UDPSocket::sendPacket(Packet &p)
{
    if (! connected()) return false;

    unsigned int bytesSent = 0;
    bytesSent = sendto(handleVal, p.buff, p.sizeVal, 0, (sockaddr*)&remote, sizeof(sockaddr));

    // if unable to send anything
    if (bytesSent != p.sizeVal)
    {
        closeConnection();
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UDPSocket::receivePacket(Packet &p)
{
    if (! connected()) return false;

    int sizeofSockaddr = sizeof(sockaddr);
    p.clearPacket();
    p.sizeVal = recvfrom(handleVal, p.buff, p.maxSize, 0, (sockaddr*)&remote, &sizeofSockaddr);

    // it didn't manage to receive anything, there was an error, close the socket and return false
    if (int(p.sizeVal) <= 0)
    {
        closeConnection();
        return false;
    }

    // make sure the size won't overflow the buffer
    if (p.sizeVal > p.maxSize) p.sizeVal = p.maxSize - sizeof(unsigned long);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
