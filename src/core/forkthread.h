/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

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

/*

Purpose:
	
	A safe version of _beginthread()

Description:

	A new thread is created and the source thread is paused until
	internal code to call MS_SYSTEM_THREAD_PUSH is made in the context
	if the new thread.

	The source thread is then released and then the user supplied
	code is called, when that function returns -- MS_SYSTEM_THREAD_POP
	is called and then the thread returns.

	This insures that Miranda will not exit whilst new threads
	are trying to be born; and the unwind wait stack will unsure
	that Miranda will wait for all created threads to return as well.

Cavets:

	The function must be reimplemented across MT plugins, since thread
	creation depends on CRT which can not be shared.

*/
UINT_PTR forkthread (
	void (__cdecl *threadcode)(void*),
	unsigned long stacksize,
	void *arg
);

UINT_PTR forkthreadex(
	void *sec,
	unsigned stacksize,
	unsigned (__stdcall *threadcode)(void*),
	void *owner,
	void *arg,
	unsigned *thraddr
);
