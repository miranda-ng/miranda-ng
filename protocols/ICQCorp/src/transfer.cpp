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

#include "stdafx.h"

std::vector <ICQTransfer *> icqTransfers;

///////////////////////////////////////////////////////////////////////////////

static void WINAPI transferTimerProc(HWND, UINT, UINT_PTR hTimer, DWORD)
{
	KillTimer(nullptr, hTimer);

	for (size_t i = 0; i < icqTransfers.size(); i++)
		if (hTimer == icqTransfers[i]->hTimer)
			icqTransfers[i]->process();
}

///////////////////////////////////////////////////////////////////////////////

ICQTransfer::ICQTransfer(ICQUser *u, unsigned int theSequence) :
	socket(WM_NETEVENT_TRANSFER)
{
	uin = u->dwUIN;
	hContact = u->hContact;
	sequence = theSequence;
	files = nullptr;
	description = nullptr;
	path = nullptr;
	sending = 0;
	speed = 100;
	count = 0;
	current = -1;
	fileName = nullptr;
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
	unsigned char cmd/*, junkChar*/;
	char *name = nullptr, *directoryName = nullptr, *szFileName = nullptr;

	packet >> cmd;
	switch (cmd) {
	case 0x00:
		Netlib_Logf(hNetlibUser, "[tcp] receive initialising file transfer\n");
		packet >> junkLong
			>> count
			>> totalSize
			>> speed
			>> name;

		files = new wchar_t*[count + 1];
		ZeroMemory(files, (count + 1)*sizeof(wchar_t*));

		ack(ACKRESULT_INITIALISING);
		sendPacket0x01();
		break;

	case 0x01:
		Netlib_Logf(hNetlibUser, "[tcp] ack initialising\n");
		packet >> speed >> name;

		ack(ACKRESULT_INITIALISING);
		sendPacket0x02();
		break;

	case 0x02:
		Netlib_Logf(hNetlibUser, "[tcp] receive next file\n");
		packet >> directory
			>> szFileName
			>> directoryName
			>> fileSize
			>> fileDate
			>> speed;

		++current;
		if (directoryName[0])
			files[current] = CMStringW(FORMAT, L"%S\\%S", directoryName, szFileName).Detach();
		else
			files[current] = mir_a2u(szFileName);

		if (directory)
			createDirectory();
		else
			openFile();
		ack(ACKRESULT_NEXTFILE);

		if (fileProgress)
			ack(ACKRESULT_FILERESUME);
		else
			sendPacket0x03();
		break;

	case 0x03:
		Netlib_Logf(hNetlibUser, "[tcp] ack next file\n");
		packet >> fileProgress >> status >> speed;

		totalProgress += fileProgress;
		setFilePosition();
		ack(ACKRESULT_NEXTFILE);

		if (status != 0) {
			totalProgress += fileSize - fileProgress;
			fileProgress = fileSize;
			closeFile();
			ack(ACKRESULT_DATA);
		}

		process();
		break;

	case 0x04:
		Netlib_Logf(hNetlibUser, "[tcp] receive stop file\n");
		packet >> junkLong;

		totalProgress += fileSize - fileProgress;
		fileProgress = fileSize;
		closeFile();
		ack(ACKRESULT_DATA);
		break;

	case 0x05:
		Netlib_Logf(hNetlibUser, "[tcp] receive new speed\n");
		packet >> speed;
		break;

	case 0x06:
		unsigned long result;

		WriteFile(hFile, packet.data(), packet.dataSize(), &result, nullptr);

		fileProgress += result;
		totalProgress += result;

		if (fileProgress >= fileSize) closeFile();
		ack(ACKRESULT_DATA);
		break;

	default:
		Netlib_Logf(hNetlibUser, "[tcp] unknown packet:\n%s", packet.print());
		packet.reset();
	}

	delete[] directoryName;
	delete[] name;
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

	Netlib_Logf(hNetlibUser, "[tcp] send packet 0x00\n");
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

	Netlib_Logf(hNetlibUser, "[tcp] send packet 0x01\n");
	socket.sendPacket(packet);
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::sendPacket0x02()
{
	current++;
	openFile();

	wchar_t *directoryName = _wcsdup(fileName);
	wchar_t *p = wcsrchr(directoryName, '\\');
	p[0] = 0;
	p[1] = 0;

	Packet packet;
	packet << (unsigned char)0x02
		<< directory
		<< _T2A(wcsrchr(fileName, '\\') + 1)
		<< _T2A(directoryName + mir_wstrlen(path) + 1)
		<< fileSize
		<< fileDate
		<< speed;

	Netlib_Logf(hNetlibUser, "[tcp] send packet 0x02\n");
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

	Netlib_Logf(hNetlibUser, "[tcp] send packet 0x03\n");
	socket.sendPacket(packet);
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::sendPacket0x04()
{
	Netlib_Logf(hNetlibUser, "[tcp] send packet 0x04\n");
	// icq_PacketAppend8(p, 0x04);
	// icq_PacketAppend32(p, filenum);
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::sendPacket0x05()
{
	Netlib_Logf(hNetlibUser, "[tcp] send packet 0x05\n");
	// icq_PacketAppend8(p, 0x05);
	// icq_PacketAppend32(p, speed);
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::sendPacket0x06()
{
	Packet packet;
	packet << (unsigned char)0x06;

	unsigned long result;
	ReadFile(hFile, packet.data(), 2048, &result, nullptr);
	if (result == 0)
		return;

	packet.add(result);

	socket.sendPacket(packet);

	fileProgress += result;
	totalProgress += result;

	if (fileProgress >= fileSize)
		closeFile();
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::ack(unsigned int result)
{
	if (result == ACKRESULT_DATA && GetTickCount() < lastNotify + 250 && fileProgress < fileSize)
		return;

	PROTOFILETRANSFERSTATUS fts;
	fts.flags = PFTS_UNICODE;
	fts.hContact = hContact;
	fts.pszFiles.w = files;
	fts.totalFiles = count;
	fts.currentFileNumber = current;
	fts.totalBytes = totalSize;
	fts.totalProgress = totalProgress;
	fts.szWorkingDir.w = path;
	fts.szCurrentFile.w = fileName;
	fts.currentFileSize = fileSize;
	fts.currentFileProgress = fileProgress;
	fts.currentFileTime = TimeZone_ToLocal(fileDate);

	ProtoBroadcastAck(protoName, hContact, ACKTYPE_FILE, result, this, (LPARAM)&fts);
	lastNotify = GetTickCount();

	if (result == ACKRESULT_DATA && current >= count - 1 && fileProgress >= fileSize) {
		ProtoBroadcastAck(protoName, hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, this, 0);
		socket.closeConnection();

		for (size_t i = 0; i < icqTransfers.size(); i++) {
			if (icqTransfers[i] == this) {
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
	if (current >= count)
		return;

	startTime = GetTickCount();
	while (fileProgress < fileSize && GetTickCount() < startTime + 100)
		sendPacket0x06();
	ack(ACKRESULT_DATA);

	if (fileProgress < fileSize)
		hTimer = SetTimer(nullptr, 0, 1, transferTimerProc);
	else if (current < count - 1)
		sendPacket0x02();
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::resume(int action, const wchar_t*)
{
	switch (action) {
	case FILERESUME_OVERWRITE:
		Netlib_Logf(hNetlibUser, "[   ] overwrite existing file\n");
		fileProgress = 0;
		break;

	case FILERESUME_RESUME:
		Netlib_Logf(hNetlibUser, "[   ] file resume\n");
		break;

	case FILERESUME_RENAME:
		Netlib_Logf(hNetlibUser, "[   ] rename file\n");
		
		replaceStrW(fileName, fileName);

		openFile();
		fileProgress = 0;
		break;

	case FILERESUME_SKIP:
		Netlib_Logf(hNetlibUser, "[   ] skip file\n");
		fileProgress = fileSize;
		break;
	}

	totalProgress += fileProgress;

	sendPacket0x03();
	ack(ACKRESULT_NEXTFILE);

	if (fileProgress)
		ack(ACKRESULT_DATA);
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::openFile()
{
	if (hFile != INVALID_HANDLE_VALUE)
		closeFile();
	if (path)
		SetCurrentDirectoryW(path);

	fileName = files[current];

	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(fileName, &findData);
	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			Netlib_Logf(hNetlibUser, "open directory %S\n", fileName);
			directory = 1;
			fileProgress = 0;
			fileSize = 0;
			fileDate = *(__int64*)(&findData.ftLastWriteTime) / 10000000 - 11644473600i64;
			return;
		}
	}
	directory = 0;

	hFile = CreateFileW(fileName, sending ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		wchar_t msg[2048];

		Netlib_Logf(hNetlibUser, "can't open file %S\n", fileName);
		mir_snwprintf(msg, L"%s\n%s", sending ? 
			TranslateT("Your file transfer has been aborted because one of the files that you selected to send is no longer readable from the disk. You may have deleted or moved it.") : 
			TranslateT("Your file receive has been aborted because Miranda could not open the destination file in order to write to it. You may be trying to save to a read-only folder."), 
			fileName);
		MessageBox(nullptr, msg, _A2T(protoName), MB_ICONWARNING | MB_OK);
		return;
	}

	__int64 fileTime;
	if (sending) {
		fileProgress = 0;
		fileSize = GetFileSize(hFile, nullptr);

		GetFileTime(hFile, nullptr, nullptr, (LPFILETIME)&fileTime);
		fileDate = fileTime / 10000000 - 11644473600i64;
	}
	else {
		fileProgress = GetFileSize(hFile, nullptr);

		fileTime = (11644473600i64 + (__int64)fileDate) * 10000000;
		SetFileTime(hFile, nullptr, nullptr, (LPFILETIME)&fileTime);
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
	if (hFile != INVALID_HANDLE_VALUE)
		SetFilePointer(hFile, fileProgress, nullptr, FILE_BEGIN);
}

///////////////////////////////////////////////////////////////////////////////

void ICQTransfer::createDirectory()
{
	if (path)
		SetCurrentDirectoryW(path);

	fileName = files[current];
	CreateDirectoryW(fileName, nullptr);

	fileProgress = 0;
}
