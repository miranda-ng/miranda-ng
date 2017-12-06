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

#ifndef transfer_h
#define transfer_h

///////////////////////////////////////////////////////////////////////////////

class ICQTransfer
{
public:
    unsigned int uin;
	MCONTACT hContact;
    unsigned int sequence;
    TCPSocket socket;
    char **files;
    char *description;
    char *path;
    unsigned char directory;
    unsigned int sending;
    unsigned int speed;
    unsigned int count;
	unsigned int current;
    char *fileName;
    unsigned int fileDate;
    unsigned int fileSize;
    unsigned int fileProgress;
    unsigned int totalSize;
    unsigned int totalProgress;
    unsigned int lastNotify;
    unsigned int hTimer;
    HANDLE hFile;

    ICQTransfer(ICQUser *u, unsigned int theSequence);
    ~ICQTransfer();
    void processTcpPacket(Packet &packet);
    void sendPacket0x00();
    void sendPacket0x01();
    void sendPacket0x02();
    void sendPacket0x03();
    void sendPacket0x04();
    void sendPacket0x05();
    void sendPacket0x06();
    void ack(unsigned int result);
    void process();
    void resume(int action, const char *newName);
    void openFile();
    void closeFile();
    void setFilePosition();
    void createDirectory();
};

///////////////////////////////////////////////////////////////////////////////

extern std::vector <ICQTransfer *> icqTransfers;

///////////////////////////////////////////////////////////////////////////////

#endif