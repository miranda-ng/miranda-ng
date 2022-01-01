/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors

#ifndef __SENDQUEUE_H
#define __SENDQUEUE_H

#define TIMERID_MSGSEND      100
#define TIMEOUT_TYPEOFF      10000      // send type off after 10 seconds of inactivity
#define SB_CHAR_WIDTH        45
#define SEND_FLAGS PREF_UNICODE

/*
 * send flags
 */

#define	SENDJOBS_MAX_SENDS 100

struct SendJob
{
	int      iSendId;
	char    *szSendBuffer;
	size_t   iSendLength;	// length of message in utf-8 octets (used to check maxlen)
	int      sendCount;
	MCONTACT hContact;
	HWND     hOwnerWnd;
	unsigned iStatus;
	wchar_t    szErrorMsg[128];
	uint32_t    dwFlags;
	int      iAcksNeeded;
	HANDLE   hEventSplit;
	int      chunkSize;
	uint32_t    dwTime;
};

class SendQueue {
public:
	enum {
		NR_SENDJOBS = 30,
		SQ_ERROR = 2,
		SQ_INPROGRESS = 1,
		SQ_UNDEFINED = 0
	};

	SendQueue()
	{
		memset(m_jobs, 0, (sizeof(SendJob) * NR_SENDJOBS));
		m_currentIndex = 0;
	}

	void	inc() { m_currentIndex++; }
	void	dec() { m_currentIndex--; }

	~SendQueue()
	{
		for (int i = 0; i < NR_SENDJOBS; i++)
			mir_free(m_jobs[i].szSendBuffer);
	}

	SendJob *getJobByIndex(const int index) { return(&m_jobs[index]); }

	void   clearJob(const int index);
	int    findNextFailed(const CMsgDialog *dat) const;
	void   handleError(CMsgDialog *dat, const int iEntry) const;
	int    addTo(CMsgDialog *dat, size_t iLen, int dwFlags);
	int    sendQueued(CMsgDialog *dat, const int iEntry);
	size_t getSendLength(const int iEntry);
	void   checkQueue(const CMsgDialog *dat) const;
	void   logError(CMsgDialog *dat, int iSendJobIndex, const wchar_t *szErrMsg) const;
	void   recallFailed(CMsgDialog *dat, int iEntry) const;
	void   showErrorControls(CMsgDialog *dat, const int showCmd) const;
	int    ackMessage(CMsgDialog *dat, WPARAM wParam, LPARAM lParam);
	int    doSendLater(int iIndex, CMsgDialog *dat, MCONTACT hContact = 0, bool fIsSendLater = true);

	// static members
	static LRESULT TSAPI WarnPendingJobs(unsigned int uNrMessages);

private:
	SendJob m_jobs[NR_SENDJOBS];
	int     m_currentIndex;
};

extern SendQueue *sendQueue;

void TSAPI HandleIconFeedback(CMsgDialog *dat, HICON iIcon);

#endif /* __SENDQUEUE_H */
