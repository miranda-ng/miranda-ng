/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2009 Nikolay Raspopov

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

// Динамические функции загружаемые DllLoaderом
extern FuncPtrType(NET_API_STATUS (NET_API_FUNCTION *) (LMSTR, DWORD, LPBYTE*) ) fnNetWkstaGetInfo;
extern FuncPtrType(DWORD (NET_API_FUNCTION *) (LPVOID) ) fnNetApiBufferFree;
extern FuncPtrType(DWORD (NET_API_FUNCTION *) (LPCWSTR, LPCWSTR, LPCWSTR, LPBYTE, DWORD) ) fnNetMessageBufferSend;
extern FuncPtrType(SC_HANDLE (WINAPI *) (LPCTSTR, LPCTSTR, DWORD) ) fnOpenSCManager;
extern FuncPtrType(SC_HANDLE (WINAPI *) (SC_HANDLE, LPCTSTR, DWORD) ) fnOpenService;
extern FuncPtrType(BOOL (WINAPI *) (SC_HANDLE, DWORD, LPSERVICE_STATUS) ) fnControlService;
extern FuncPtrType(BOOL (WINAPI *) (SC_HANDLE, LPSERVICE_STATUS) ) fnQueryServiceStatus;
extern FuncPtrType(BOOL (WINAPI *) (SC_HANDLE) ) fnCloseServiceHandle;
extern FuncPtrType(BOOL (WINAPI *) (SC_HANDLE, DWORD, LPCTSTR*) ) fnStartService;
extern FuncPtrType(SC_LOCK (WINAPI *) (SC_HANDLE) ) fnLockServiceDatabase;
extern FuncPtrType(BOOL (WINAPI *) (SC_LOCK) ) fnUnlockServiceDatabase;
extern FuncPtrType(BOOL (WINAPI *) (SC_HANDLE, DWORD, DWORD, DWORD, LPCTSTR, LPCTSTR, LPDWORD, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR) ) fnChangeServiceConfig;
