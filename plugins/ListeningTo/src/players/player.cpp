/* 
Copyright (C) 2005-2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "..\commons.h"

extern void HasNewListeningInfo();

Player::Player() : name(_T("Player")), enabled(FALSE), needPoll(FALSE)
{
	memset(&listening_info, 0, sizeof(listening_info));
	InitializeCriticalSection(&cs);
}

Player::~Player()
{
	FreeData();
	DeleteCriticalSection(&cs);
}

void Player::NotifyInfoChanged()
{
	if (enabled)
		HasNewListeningInfo();
}

BOOL Player::GetListeningInfo(LISTENINGTOINFO *lti)
{
	EnterCriticalSection(&cs);

	BOOL ret;
	if (listening_info.cbSize == 0)
	{
		ret = FALSE;
	}
	else 
	{
		if (lti != NULL)
			CopyListeningInfo(lti, &listening_info);
		ret = TRUE;
	}

	LeaveCriticalSection(&cs);

	return ret;
}

void Player::FreeData()
{
	EnterCriticalSection(&cs);

	if (listening_info.cbSize != 0)
		FreeListeningInfo(&listening_info);

	LeaveCriticalSection(&cs);
}

LISTENINGTOINFO * Player::LockListeningInfo()
{
	EnterCriticalSection(&cs);

	return &listening_info;
}

void Player::ReleaseListeningInfo()
{
	LeaveCriticalSection(&cs);
}



ExternalPlayer::ExternalPlayer()
{
	name = _T("ExternalPlayer");
	needPoll = TRUE;

	window_classes = NULL;
	num_window_classes = 0;
	found_window = FALSE;
}

ExternalPlayer::~ExternalPlayer()
{
}

HWND ExternalPlayer::FindWindow()
{
	HWND hwnd = NULL;
	for(int i = 0; i < num_window_classes; i++)
	{
		hwnd = ::FindWindow(window_classes[i], NULL);
		if (hwnd != NULL)
			break;
	}
	return hwnd;
}

BOOL ExternalPlayer::GetListeningInfo(LISTENINGTOINFO *lti)
{
	if (FindWindow() == NULL)
		return FALSE;

	return Player::GetListeningInfo(lti);
}



CodeInjectionPlayer::CodeInjectionPlayer()
{
	name = _T("CodeInjectionPlayer");
	dll_name = NULL;
	message_window_class = NULL;
	next_request_time = 0;
}

CodeInjectionPlayer::~CodeInjectionPlayer()
{
}

void CodeInjectionPlayer::InjectCode()
{
	if (!opts.enable_code_injection)
		return;
	else if (next_request_time > GetTickCount())
		return;

	// Window is opened?
	HWND hwnd = FindWindow();
	if (hwnd == NULL)
		return;

	// Msg Window is registered? (aka plugin is running?)
	HWND msgHwnd = ::FindWindow(message_window_class, NULL);
	if (msgHwnd != NULL)
		return;


	next_request_time = GetTickCount() + 30000;


	// Get the dll path
	char dll_path[1024] = {0};
	if (!GetModuleFileNameA(hInst, dll_path, SIZEOF(dll_path)))
		return;

	char *p = strrchr(dll_path, '\\');
	if (p == NULL)
		return;

	p++;
	*p = '\0';

	size_t len = p - dll_path;

	mir_snprintf(p, 1024 - len, "listeningto\\%s.dll", dll_name);

	len = strlen(dll_path);

	// File exists?
	DWORD attribs = GetFileAttributesA(dll_path);
	if (attribs == 0xFFFFFFFF || !(attribs & FILE_ATTRIBUTE_ARCHIVE))
		return;

	// Do the code injection
	unsigned long pid;
	GetWindowThreadProcessId(hwnd, &pid);
	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION 
									| PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
	if (hProcess == NULL)
		return;

	char *_dll = (char *) VirtualAllocEx(hProcess, NULL, len+1, MEM_COMMIT, PAGE_READWRITE );
	if (_dll == NULL)
	{
		CloseHandle(hProcess);
		return;
	}
	WriteProcessMemory(hProcess, _dll, dll_path, len+1, NULL);

	HMODULE hKernel32 = GetModuleHandleA("kernel32");
	HANDLE hLoadLibraryA = GetProcAddress(hKernel32, "LoadLibraryA");
	DWORD threadId;
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE) hLoadLibraryA, 
										_dll, 0, &threadId);
	if (hThread == NULL)
	{
		VirtualFreeEx(hProcess, _dll, len+1, MEM_RELEASE);
		CloseHandle(hProcess);
		return;
	}
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	VirtualFreeEx(hProcess, _dll, len+1, MEM_RELEASE);
	CloseHandle(hProcess);
}

BOOL CodeInjectionPlayer::GetListeningInfo(LISTENINGTOINFO *lti)
{
	if (enabled)
		InjectCode();

	return ExternalPlayer::GetListeningInfo(lti);
}

