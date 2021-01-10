/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-20 Miranda NG team,
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
//
// the sendlater class

#ifndef __SENDLATER_H
#define __SENDLATER_H

#define TIMERID_SENDLATER 12000
#define TIMERID_SENDLATER_TICK 13000

#define TIMEOUT_SENDLATER 10000
#define TIMEOUT_SENDLATER_TICK 200

struct CSendLaterJob
{
	// job status/error codes
	enum {
		INVALID_CONTACT = 'I',
		JOB_DEFERRED = 'D',
		JOB_AGE = 'O',
		JOB_MYSTATUS = 'M',
		JOB_STATUS = 'S',
		JOB_WAITACK = 'A',
		JOB_REMOVABLE = 'R',
		JOB_HOLD = 'H',
	};

	// internal flags
	enum {
		SLF_SUSPEND = 1,
		SLF_INVALID = 2
	};

	void	readFlags();
	void	writeFlags();
	void	cleanDB();
	bool	isPersistentJob();
	bool	mustDelete();

	CSendLaterJob();
	~CSendLaterJob();

	char     szId[20];            // database key name (time stamp of original send)
	MCONTACT hContact;            // original contact where the message has been assigned
	MCONTACT hTargetContact;      // *real* contact (can be different for metacontacts, e.g).
	HANDLE   hProcess;            // returned from the protocols sending service. needed to find it in the ACK handler
	time_t   created;             // job was created at this time (important to kill jobs, that are too old)
	time_t   lastSent;            // time at which the delivery was initiated. used to handle timeouts
	char    *sendBuffer;          // utf-8 send buffer
	PBYTE    pBuf;                // conventional send buffer (for non-utf8 protocols)
	DWORD    dwFlags;
	int      iSendCount;          // # of times we tried to send it...
	bool     fSuccess, fFailed;
	BYTE     bCode;               // error/progress code (for the UI)
};

class CSendLater
{
	friend class CSendLaterDlg;

	void    processSingleContact(const MCONTACT hContact);
	int     sendIt(CSendLaterJob *job);

	LIST<void> m_sendLaterContactList;
	LIST<CSendLaterJob> m_sendLaterJobList;

	CMOption<bool> m_fAvail, m_fErrorPopups, m_fSuccessPopups;

	bool     m_fIsInteractive = false;
	time_t   m_last_sendlater_processed;
	int      m_currJob = -1;

public:
	enum {
		SENDLATER_AGE_THRESHOLD = (86400 * 3),				// 3 days, older messages will be removed from the db.
		SENDLATER_RESEND_THRESHOLD = 180,					// timeouted messages should be resent after that many seconds
		SENDLATER_PROCESS_INTERVAL = 50						// process the list of waiting job every this many seconds
	};

	CSendLater();
	~CSendLater();
	
	__inline bool   isAvail() { return m_fAvail; }
	__inline bool   haveErrorPopups() { return m_fErrorPopups; }
	__inline bool   haveSuccessPopups() { return m_fSuccessPopups; }

	__inline bool   isInteractive() const { return m_fIsInteractive; }
	__inline bool   isJobListEmpty() const { return m_sendLaterJobList.getCount() == 0; }
	__inline time_t lastProcessed() const { return m_last_sendlater_processed; }
	__inline void   setLastProcessed(const time_t _t) { m_last_sendlater_processed = _t; }
	__inline void   flushQueue() { m_last_sendlater_processed = 0; }
	__inline bool   haveJobs() const { return (m_sendLaterJobList.getCount() != 0 && m_currJob != -1); }

	static int _cdecl addStub(const char *szSetting, void *lParam);

	bool   processCurrentJob();
	void   processContacts();
	void   startJobListProcess();
	int    addJob(const char *szSetting, void *lParam);
	void   addContact(const MCONTACT hContact);
	HANDLE processAck(const ACKDATA *ack);

	void   invokeQueueMgrDlg();
	void   qMgrUpdate(bool fReEnable = false);
	static INT_PTR svcQMgr(WPARAM wParam, LPARAM lParam);
};

extern CSendLater *sendLater;

#endif /* __SENDLATER_H */
