/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-19 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "stdafx.h"
#include "openssl.h"


static HANDLE *mutexes;

unsigned long OpenSSL_thread_id()
{
    return static_cast<unsigned long>(GetCurrentThreadId());
}

void OpenSSL_lock(int mode, int n, const char *, int)
{
    if (mode & CRYPTO_LOCK) {
        WaitForSingleObject(mutexes[n], INFINITE);
	} else {
        ReleaseMutex(mutexes[n]);
	}
}

void OpenSSL_Init() {
	int num_locks = CRYPTO_num_locks();
	
	mutexes = new HANDLE[num_locks];
	
	for (int i = 0; i < num_locks; ++i) {
		mutexes[i] = CreateMutex(nullptr, FALSE, nullptr);
	}
	
	CRYPTO_set_id_callback(OpenSSL_thread_id);
    CRYPTO_set_locking_callback(OpenSSL_lock);	
}

void OpenSSL_Cleanup(){
    CRYPTO_set_id_callback(nullptr);
    CRYPTO_set_locking_callback(nullptr);
	
    for (int i = 0; i < CRYPTO_num_locks(); ++i) {
		CloseHandle(mutexes[i]);
	}
	
    delete[] mutexes;
}