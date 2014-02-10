/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "common.h"
#include "dialog.h"
#include "serverlist.h"

class Event
{
private:
	HANDLE handle;

public:
	Event() { handle = CreateEvent (0, FALSE, FALSE, 0); }
	~Event() { CloseHandle (handle); }

	operator HANDLE () { return handle; }

	void release() { SetEvent(handle); }
	void wait() { WaitForSingleObject(handle, INFINITE); }
};

class GenericJob
{
private:
	int openFileDialog();
	int openFolderDialog();
	void getFilesFromOpenDialog();
	int getFilesFromFolder(TCHAR *stzFolder);
	void addFile(TCHAR *fileName);

public:
	enum EMode
	{
		FTP_RAWFILE = 8,
		FTP_ZIPFILE	= 16,
		FTP_ZIPFOLDER = 32
	};

	enum EStatus
	{
		STATUS_CREATED = 0,
		STATUS_WAITING,
		STATUS_CONNECTING,
		STATUS_UPLOADING,
		STATUS_PACKING,
		STATUS_PAUSED,
		STATUS_CANCELED,
		STATUS_COMPLETED
	};

	MCONTACT hContact;
	int iFtpNum, fileID;
	ServerList::FTP *ftp;
	TCHAR stzFilePath[1024];
	TCHAR stzFileName[64];
	char szSafeFileName[64];
	EMode mode;
	EStatus status;

	UploadDialog::Tab *tab;
	vector<TCHAR *> files;
	TCHAR * operator[] (int i) const { return files[i];}

	GenericJob(MCONTACT hContact, int iFtpNum, EMode mode);
	GenericJob(GenericJob *job);
	virtual ~GenericJob();

	int getFiles();	
	int getFiles(void **objects, int objCount, DWORD flags);
	void setStatus(EStatus status);

	bool isCompleted();
	bool isPaused();
	bool isWaitting();
	bool isConnecting();
	bool isCanceled();

	TCHAR *getStatusString();

	virtual void start() = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual void cancel() = 0;
	virtual void addToUploadDlg() = 0;	
	virtual void pauseHandler() = 0;
	
	virtual void refreshTab(bool bTabChanged);
	virtual void closeTab() = 0;
	virtual void closeAllTabs() = 0;
	virtual void createToolTip() = 0;
};
