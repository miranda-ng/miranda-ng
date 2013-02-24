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

#include "Interface.h"
#include "Thread.h"
#include "intrinsics.h"

unsigned int __stdcall ThreadDistributor(void* Param)
{
	CThread * thread = static_cast<CThread *>(Param);
	DWORD result = thread->Wrapper();
	_endthreadex(result);
	return result; // to make the compiler happy
}

CThread::CThread(bool CreateSuspended)
{
	m_Handle = NULL;
	m_Terminated = 0;
	m_FreeOnTerminate = false;
	m_Finished = false;
	m_Suspended = CreateSuspended;
	m_ReturnValue = 0;
	unsigned int flags = 0;
	if (CreateSuspended)
		flags = CREATE_SUSPENDED;

	m_Handle = reinterpret_cast<HANDLE> (_beginthreadex(NULL, 0, &ThreadDistributor, this, flags, &m_ThreadID));
}
CThread::~CThread()
{
	if (!m_Finished && !m_Suspended)
	{
		Terminate();
		WaitFor();
	}
	if (m_Handle)
		CloseHandle(m_Handle);
}
DWORD CThread::Wrapper()
{
  Execute();

  bool dofree = FreeOnTerminate();
  DWORD result = ReturnValue();
	m_Finished = true;

	m_sigTerminate(this);
  if (dofree)
    delete this;

	return result;
}

void CThread::Resume()
{
	if (ResumeThread(m_Handle) == 1)
		XCHG_32(m_Suspended, 0);
}
void CThread::Suspend()
{
	SuspendThread(m_Handle);
	XCHG_32(m_Suspended, 1);
}
void CThread::Terminate()
{
	XCHG_32(m_Terminated, 1);
}
DWORD CThread::WaitFor()
{
	HANDLE tmp = m_Handle;
	DWORD result = WAIT_FAILED;

	if (WaitForSingleObject(m_Handle, INFINITE) != WAIT_FAILED)
		GetExitCodeThread(tmp, &result);

	return result;
}

void CThread::FreeOnTerminate(bool Terminate)
{
	XCHG_32(m_FreeOnTerminate, Terminate);
}
void CThread::ReturnValue(DWORD Value)
{
	XCHG_32(m_ReturnValue, Value);
}

void CThread::Priority(TPriority NewPriority)
{
	SetThreadPriority(m_Handle, NewPriority);
}
CThread::TPriority CThread::Priority()
{
	return static_cast<TPriority> (GetThreadPriority(m_Handle) & 0xffff);
}
