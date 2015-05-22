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

std::vector <ICQTransfer *> icqTransfers;

///////////////////////////////////////////////////////////////////////////////

void WINAPI transferTimerProc(HWND hWnd, UINT Msg, UINT_PTR hTimer, DWORD Time)
{
    unsigned int i;

    KillTimer(NULL, hTimer);
    for (i=0; i<icqTransfers.size(); i++) if (hTimer == icqTransfers[i]->hTimer) icqTransfers[i]->process();
}

///////////////////////////////////////////////////////////////////////////////

ICQTransfer::ICQTransfer(ICQUser *u, unsigned int theSequence) :
    socket(WM_NETEVENT_TRANSFER)
{
    uin = u->uin;
    hContact = u->hContact;
    sequence = theSequence;
    files = NULL;
    description = NULL;
    path = NULL;
    sending = 0;
    speed = 100;
    count = 0;
    current = -1;
    fileName = NULL;
    fileSize = 0;
    fileProgress = 0;
    totalSize = 0;
    totalProgress = 0;
    lastNotify = 0;
    hTimer = NULL;
    hFile = INVALID_HANDLE_VALUE;
}

///////////////////////////////////////////////////////////////////////////////

ICQTransfer::~ICQTransfer()
{
    closeFile();
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::processTcpPacket(Packet &packet)
{
    unsigned int /*i,*/ status, junkLong;
    //unsigned short junkShort;
    unsigned char cmd/*, junkChar*/;
    char *name = NULL, *directoryName = NULL;

    packet >> cmd;
    switch (cmd)
    {
    case 0x00:
        T("[tcp] recieve initialising file transfer\n");
        packet >> junkLong
               >> count
               >> totalSize
               >> speed
               >> name;

        files = new char*[count + 1];
        ZeroMemory(files, (count+1)*sizeof(char*));

        ack(ACKRESULT_INITIALISING);
        sendPacket0x01();
        break;

    case 0x01:
        T("[tcp] ack initialising\n");
        packet >> speed
               >> name;

        ack(ACKRESULT_INITIALISING);
        sendPacket0x02();
        break;

    case 0x02:
        T("[tcp] recieve next file\n");
        packet >> directory
               >> files[++current]
               >> directoryName
               >> fileSize
               >> fileDate
               >> speed;

        if (directoryName[0])
        {
            char *fullName = new char[mir_strlen(directoryName) + mir_strlen(files[current]) + 2];
            sprintf(fullName, "%s\\%s", directoryName, files[current]);
            delete [] files[current];
            files[current] = fullName;
        }

        if (directory) createDirectory();
        else openFile();
        ack(ACKRESULT_NEXTFILE);

        if (fileProgress) ack(ACKRESULT_FILERESUME);
        else sendPacket0x03();
        break;

    case 0x03:
        T("[tcp] ack next file\n");
        packet >> fileProgress
               >> status
               >> speed;

        totalProgress += fileProgress;
        setFilePosition();
        ack(ACKRESULT_NEXTFILE);

        if (status != 0)
        {
            totalProgress += fileSize - fileProgress;
            fileProgress = fileSize;
            closeFile();
            ack(ACKRESULT_DATA);
        }

        process();
        break;

    case 0x04:
        T("[tcp] recieve stop file\n");
        packet >> junkLong;

        totalProgress += fileSize - fileProgress;
        fileProgress = fileSize;
        closeFile();
        ack(ACKRESULT_DATA);
        break;

    case 0x05:
        T("[tcp] recieve new speed\n");
        packet >> speed;
        break;

    case 0x06:
        unsigned long result;

        WriteFile(hFile, packet.data(), packet.dataSize(), &result, NULL);

        fileProgress += result;
        totalProgress += result;

        if (fileProgress >= fileSize) closeFile();
        ack(ACKRESULT_DATA);
        break;

    default:
        T("[tcp] unknown packet:\n%s", packet.print());
        packet.reset();
    }

    delete [] directoryName;
    delete [] name;
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::sendPacket0x00()
{
    char nick[1] = { 0 };

    sending = true;

    Packet packet;
    packet << (unsigned char)0x00
           << (unsigned int)0x00
           << count
           << totalSize
           << speed
           << nick;

    T("[tcp] send packet 0x00\n");
    socket.sendPacket(packet);
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::sendPacket0x01()
{
    char nick[1] = { 0 };

    Packet packet;
    packet << (unsigned char)0x01
           << speed
           << nick;

    T("[tcp] send packet 0x01\n");
    socket.sendPacket(packet);
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::sendPacket0x02()
{
    char *directoryName, *p;

    current++;
    openFile();

    directoryName = _strdup(fileName);
    p = strrchr(directoryName, '\\');
    p[0] = 0;
    p[1] = 0;

    Packet packet;
    packet << (unsigned char)0x02
           << directory
           << (strrchr(fileName, '\\') + 1)
           << (directoryName + mir_strlen(path) + 1)
           << fileSize
           << fileDate
           << speed;

    T("[tcp] send packet 0x02\n");
    socket.sendPacket(packet);
    ack(ACKRESULT_NEXTFILE);

    free(directoryName);
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::sendPacket0x03()
{
    Packet packet;
    packet << (unsigned char)0x03
           << fileProgress
           << (unsigned int)0x00
           << speed;

    setFilePosition();

    T("[tcp] send packet 0x03\n");
    socket.sendPacket(packet);
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::sendPacket0x04()
{
    T("[tcp] send packet 0x04\n");
/*
    icq_PacketAppend8(p, 0x04);
    icq_PacketAppend32(p, filenum);
*/
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::sendPacket0x05()
{
    T("[tcp] send packet 0x05\n");
/*
    icq_PacketAppend8(p, 0x05);
    icq_PacketAppend32(p, speed);
*/
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::sendPacket0x06()
{
    unsigned long result;

    Packet packet;
    packet << (unsigned char)0x06;

    ReadFile(hFile, packet.data(), 2048, &result, NULL);
    if (result == 0) return;
    packet.add(result);

//    T("[tcp] send packet 0x06\n");
    socket.sendPacket(packet);

    fileProgress += result;
    totalProgress += result;

    if (fileProgress >= fileSize) closeFile();
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::ack(unsigned int result)
{
    PROTOFILETRANSFERSTATUS fts;

    if (result == ACKRESULT_DATA && GetTickCount() < lastNotify+250 && fileProgress < fileSize) return;

    fts.cbSize = sizeof(fts);
    fts.hContact = hContact;
    //fts.sending = sending;
	fts.pszFiles = files;
    fts.totalFiles = count;
    fts.currentFileNumber = current;
    fts.totalBytes = totalSize;
    fts.totalProgress = totalProgress;
    fts.szWorkingDir = path;
    fts.szCurrentFile = fileName;
    fts.currentFileSize = fileSize;
    fts.currentFileProgress = fileProgress;
    fts.currentFileTime = CallService(MS_DB_TIME_TIMESTAMPTOLOCAL, fileDate, 0);
/*
    switch (session->status)
    {
    case FILE_STATUS_LISTENING: result = ACKRESULT_SENTREQUEST; break;
    case FILE_STATUS_CONNECTED: result = ACKRESULT_CONNECTED; break;
    case FILE_STATUS_CONNECTING: result = ACKRESULT_CONNECTING; break;
    case FILE_STATUS_INITIALIZING: result = ACKRESULT_INITIALISING; break;
    case FILE_STATUS_NEXT_FILE: result = ACKRESULT_NEXTFILE; break;
    case FILE_STATUS_SENDING:
    case FILE_STATUS_RECEIVING: result=ACKRESULT_DATA; break;
    }
*/
    ProtoBroadcastAck(protoName, hContact, ACKTYPE_FILE, result, this, (LPARAM)&fts);
    lastNotify = GetTickCount();

    if (result == ACKRESULT_DATA && current >= count-1 && fileProgress >= fileSize)
    {
        ProtoBroadcastAck(protoName, hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, this, 0);
        socket.closeConnection();

        unsigned int i;
        for (i=0; i<icqTransfers.size(); i++)
        {
            if (icqTransfers[i] == this)
            {
                delete icqTransfers[i];
                icqTransfers[i] = icqTransfers[icqTransfers.size() - 1];
                icqTransfers.pop_back();
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::process()
{
    unsigned int /*i, */startTime;

    hTimer = NULL;
    if (current >= count) return;

    startTime = GetTickCount();
    while (fileProgress < fileSize && GetTickCount() < startTime+100) sendPacket0x06();
    ack(ACKRESULT_DATA);

    if (fileProgress < fileSize) hTimer = SetTimer(NULL, 0, 1, (TIMERPROC)transferTimerProc);
    else if (current < count-1) sendPacket0x02();
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::resume(int action, const char *newName)
{
	switch (action)
	{
    case FILERESUME_OVERWRITE:
        T("[   ] overwrite existing file\n");
        fileProgress = 0;
        break;

    case FILERESUME_RESUME:
        T("[   ] file resume\n");
        break;

    case FILERESUME_RENAME:
        T("[   ] rename file\n");
        delete [] fileName;
        fileName = new char[mir_strlen(newName) + 1];
        strcpy(fileName, newName);
        files[current] = fileName;

        openFile();
        fileProgress = 0;
        break;

    case FILERESUME_SKIP:
        T("[   ] skip file\n");
        fileProgress = fileSize;
        break;
	}

    totalProgress += fileProgress;

    sendPacket0x03();
    ack(ACKRESULT_NEXTFILE);

    if (fileProgress) ack(ACKRESULT_DATA);
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::openFile()
{
    HANDLE hFind;
    WIN32_FIND_DATA findData;
    __int64 fileTime;

    if (hFile != INVALID_HANDLE_VALUE) closeFile();
    if (path) SetCurrentDirectory(path);

    fileName = files[current];

    hFind = FindFirstFile(fileName, &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            T("open directory %s\n", fileName);
            directory = 1;
            fileProgress = 0;
            fileSize = 0;
            fileDate = *(__int64*)(&findData.ftLastWriteTime) / 10000000 - 11644473600i64;
            return;
        }
    }
    directory = 0;

    hFile = CreateFile(fileName, sending ? GENERIC_READ : GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        char msg[2048];

        T("can't open file %s\n", fileName);
        sprintf(msg, "%s\n%s", sending ? Translate("Your file transfer has been aborted because one of the files that you selected to send is no longer readable from the disk. You may have deleted or moved it.") : Translate("Your file receive has been aborted because Miranda could not open the destination file in order to write to it. You may be trying to save to a read-only folder."), fileName);
        MessageBox(NULL, msg, Translate(protoName), MB_ICONWARNING|MB_OK);
        return;
    }

    if (sending)
    {
        fileProgress = 0;
        fileSize = GetFileSize(hFile, NULL);

        GetFileTime(hFile, NULL, NULL, (LPFILETIME)&fileTime);
        fileDate = fileTime / 10000000 - 11644473600i64;
    }
    else
    {
        fileProgress = GetFileSize(hFile, NULL);

        fileTime = (11644473600i64 + (__int64)fileDate) * 10000000;
        SetFileTime(hFile, NULL, NULL, (LPFILETIME)&fileTime);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::closeFile()
{
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::setFilePosition()
{
    if (hFile != INVALID_HANDLE_VALUE) SetFilePointer(hFile, fileProgress, NULL, FILE_BEGIN);
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::createDirectory()
{
    if (path) SetCurrentDirectory(path);

    fileName = files[current];
    CreateDirectory(fileName, NULL);

    fileProgress = 0;
}

///////////////////////////////////////////////////////////////////////////////
