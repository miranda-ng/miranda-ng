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
//
// the sendlater class

#ifndef __SENDLATER_H
#define __SENDLATER_H

#define TIMERID_SENDLATER 12000
#define TIMERID_SENDLATER_TICK 13000

#define TIMEOUT_SENDLATER 10000
#define TIMEOUT_SENDLATER_TICK 200

enum {
	SENDLATER_AGE_THRESHOLD = (86400 * 3),				// 3 days, older messages will be removed from the db.
	SENDLATER_RESEND_THRESHOLD = 180,					// timeouted messages should be resent after that many seconds
	SENDLATER_PROCESS_INTERVAL = 50						// process the list of waiting job every this many seconds
};

namespace SendLater
{
	void   shutDown();

	bool   isInteractive();
	bool   isJobListEmpty();
	time_t lastProcessed();
	void   setLastProcessed(const time_t _t);
	void   flushQueue();
	bool   haveJobs();

	bool   processCurrentJob();
	void   processContacts();
	void   startJobListProcess();
	int    addJob(const char *szSetting, void *lParam);
	void   addContact(const MCONTACT hContact);
	HANDLE processAck(const ACKDATA *ack);

	void   invokeQueueMgrDlg();
	void   qMgrUpdate(bool fReEnable = false);

	extern CMOption<bool> Avail, ErrorPopups, SuccessPopups;
};

#endif /* __SENDLATER_H */
