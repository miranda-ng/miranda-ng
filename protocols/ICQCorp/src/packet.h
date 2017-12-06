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

#ifndef packet_h
#define packet_h

const int MAX_PACKET_SIZE = 8192;

///////////////////////////////////////////////////////////////////////////////

class Packet
{
public:
    Packet();
    Packet(Packet *packet);
//    Packet(char *newBuff, unsigned long buffSize);
    ~Packet();
    void clearPacket();
    void reset() { nextData = buff; };
    void add(unsigned int s);
    char *data() { return nextData; };
    unsigned int size() { return sizeVal; };
    unsigned int dataSize() { return buff + sizeVal - nextData; };
    Packet &operator << (unsigned int data);
    Packet &operator << (unsigned short data);
    Packet &operator << (unsigned char data);
    Packet &operator << (char *data);
    Packet &operator << (Packet *packet);
    Packet &operator >> (unsigned int &in);
    Packet &operator >> (unsigned short &in);
    Packet &operator >> (unsigned char &in);
    Packet &operator >> (char *&in);
    char *print();
protected:
    char *buff;
    char *nextData;
    unsigned int maxSize;
    unsigned int sizeVal;

    friend class UDPSocket;
    friend class TCPSocket;
};  

///////////////////////////////////////////////////////////////////////////////

#endif
