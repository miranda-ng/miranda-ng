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

Packet::Packet()
{
    maxSize = MAX_PACKET_SIZE;
    buff = new char[maxSize];
    nextData = buff;
    sizeVal = 0;
}

///////////////////////////////////////////////////////////////////////////////

Packet::Packet(Packet *packet)
{
    maxSize = packet->maxSize;
    buff = new char[maxSize];
    nextData = buff + (packet->nextData - packet->buff);
    sizeVal = packet->sizeVal;
    memcpy(buff, packet->buff, sizeVal);
}

///////////////////////////////////////////////////////////////////////////////
/*
Packet::Packet(char *newBuff, unsigned long buffSize)
{
    maxSize = MAX_PACKET_SIZE;
    if (buffSize > maxSize) buffSize = maxSize;
    buff = new char[maxSize];
    memcpy(buff, newBuff, buffSize);
    nextData = buff;
    sizeVal = buffSize;
}

///////////////////////////////////////////////////////////////////////////////
*/
Packet::~Packet()
{
    delete [] buff;
}

///////////////////////////////////////////////////////////////////////////////

void Packet::clearPacket()
{
    nextData = buff;
    sizeVal = 0;
}

///////////////////////////////////////////////////////////////////////////////

void Packet::add(unsigned int s)
{
    nextData += s;
    sizeVal += s;
}

///////////////////////////////////////////////////////////////////////////////

Packet &Packet::operator << (unsigned int data)
{
    *(unsigned int*)nextData = data;
    sizeVal += sizeof(unsigned int);
    nextData += sizeof(unsigned int);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Packet &Packet::operator << (unsigned short data)
{
    *(unsigned short*)nextData = data;
    sizeVal += sizeof(unsigned short);
    nextData += sizeof(unsigned short);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Packet &Packet::operator << (unsigned char data)
{
    *(unsigned char*)nextData = data;
    sizeVal += sizeof(unsigned char);
    nextData += sizeof(unsigned char);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Packet &Packet::operator << (char *data)
{
	unsigned int s = (unsigned int)strlen(data) + 1;
    operator << ((unsigned short)s);
    memcpy(nextData, data, s);
    sizeVal += s;
    nextData += s;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Packet &Packet::operator << (Packet *packet)
{
    unsigned int s = packet->dataSize();
    memcpy(nextData, packet->nextData, s);
    sizeVal += s;
    nextData += s;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Packet &Packet::operator >> (unsigned int &in)
{
    if (nextData+sizeof(unsigned int) > buff+sizeVal) in = 0;
    else
    {
        in = *(unsigned int*)nextData;
        nextData += sizeof(unsigned int);
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Packet &Packet::operator >> (unsigned short &in)
{
    if (nextData+sizeof(unsigned short) > buff+sizeVal) in = 0;
    else
    {
        in = *(unsigned short*)nextData;
        nextData += sizeof(unsigned short);
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Packet &Packet::operator >> (unsigned char &in)
{
    if (nextData+sizeof(unsigned char) > buff+sizeVal) in = 0;
    else
    {
        in = *(unsigned char*)nextData;
        nextData += sizeof(unsigned char);
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Packet &Packet::operator >> (char *&in)
{
    unsigned short s;

    operator >> (s);
    if (nextData+s > buff+sizeVal) in = 0;
    else
    {
        if (in == NULL) in = new char[s];
        memcpy(in, nextData, s);
        nextData += s;
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

char *Packet::print()
{
    unsigned int i;
    static char p[8196];
    char *pPos = p;

    for (i=0; i<sizeVal; i++)
    {
        if (i % 16 == 0)
        {
            sprintf(pPos, "  0x%04X:  %65c\n\0", i, ' ');
            pPos += 11;
        }

        sprintf(pPos, "%02X ", (unsigned char)buff[i]);
        if ((unsigned char)buff[i] > ' ') pPos[48 - (i % 16)*2 + 1] = buff[i];

        pPos += 3;
        if ((i+1) % 16 == 0)
        {
            pPos[0] = ' ';
            pPos += 18;
        }
    }
    if (sizeVal % 16 != 0) pPos[0] = ' ';
    return p;
}

///////////////////////////////////////////////////////////////////////////////
