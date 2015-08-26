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

#include "stdafx.h"
#include "job_generic.h"
#include "utils.h"

class UploadJob: public GenericJob
{
	friend class UploadDialog;

private:
	static const int MAX_RUNNING_JOBS = 4;

	FILE *m_fp;
	CURL *m_hCurl;
	char m_szError[CURL_ERROR_SIZE];
	char m_buff[256];

	UINT64 m_uiFileSize;
	UINT64 m_uiTotalSent;
	UINT64 m_uiSent;
	time_t m_startTS;
	double m_lastSpeed[10];
	double m_avgSpeed;

	static mir_cs mutexJobCount;
	static int iRunningJobCount;

	char *getChmodString();
	char *getDelFileString();
	char *getUrlString();
	char *getDelUrlString();
	void copyLinkToML();
	void autoSend();

	CURL* curlInit(char *szUrl, struct curl_slist *headerList);

	static void waitingThread(void *arg); 
	static size_t ReadCallback(void *ptr, size_t size, size_t nmemb, void *arg);
	static INT_PTR CALLBACK DlgProcFileExists(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam); 

	bool fileExistsOnServer();
	void upload(); 
	void updateStats();

public:
	char m_szFileLink[256];
	static Event jobDone;

	UploadJob(MCONTACT _hContact, int _iFtpNum, EMode _mode);
	UploadJob(UploadJob *job);
	UploadJob(PackerJob *job);
	virtual ~UploadJob();

	virtual void start();
	virtual void pause();
	virtual void resume();
	virtual void cancel();
	virtual void addToUploadDlg();
	virtual void pauseHandler();

	virtual void refreshTab(bool bTabChanged);
	virtual void closeTab();
	virtual void closeAllTabs();
	virtual void createToolTip();
};