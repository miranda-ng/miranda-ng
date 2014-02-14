/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#pragma once

#include <process.h>
#include <windows.h>
#include "sigslot.h"
#include "stdint.h"

class CThread
{
private:
	uint32_t volatile m_Terminated;
	uint32_t volatile m_FreeOnTerminate;
	uint32_t volatile m_Finished;
	uint32_t volatile m_Suspended;
	uint32_t volatile m_ReturnValue;
protected:
	HANDLE       m_Handle;
	unsigned int m_ThreadID;

	void ReturnValue(DWORD Value);
	virtual void Execute() = 0;
public:
	CThread(bool CreateSuspended);
	virtual ~CThread();

	DWORD Wrapper();

	void Resume();
	void Suspend();
	void Terminate();
	DWORD WaitFor();
	
	bool Suspended() {return m_Suspended != 0;};
	bool Terminated() {return m_Terminated != 0;};
	bool FreeOnTerminate() {return m_FreeOnTerminate != 0;};
	void FreeOnTerminate(bool Terminate);
	DWORD ReturnValue() {return m_ReturnValue;};

	typedef enum TPriority {
		tpIdle = THREAD_PRIORITY_IDLE,
		tpLowest = THREAD_PRIORITY_LOWEST,
		tpLower = THREAD_PRIORITY_BELOW_NORMAL,
		tpNormal = THREAD_PRIORITY_NORMAL,
		tpHigher = THREAD_PRIORITY_ABOVE_NORMAL,
		tpHighest = THREAD_PRIORITY_HIGHEST,
		tpRealTime = THREAD_PRIORITY_TIME_CRITICAL
	} TPriority;

	void Priority(TPriority NewPriority);
	TPriority Priority();

	typedef sigslot::signal1<CThread *> TOnTerminate;
	TOnTerminate m_sigTerminate;
	TOnTerminate & sigTerminate() {return m_sigTerminate;};
};
