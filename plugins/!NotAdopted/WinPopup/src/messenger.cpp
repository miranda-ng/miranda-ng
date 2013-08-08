/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2010 Nikolay Raspopov

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

static bool IsMessengerRunning()
{
	bool found = false;
	HANDLE h = CreateMailslot (_T("\\\\.\\mailslot\\") MESSENGER_MAIL, 0, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		if (GetLastError () == ERROR_ALREADY_EXISTS)
			found = true;
		else
			LOG ("IsMessengerRunning() error 0x%08x", GetLastError ());
	} else
		CloseHandle (h);		
	return found;
}

messenger pluginMessenger;			// Прием/отправка сообщений через Messenger

messenger::messenger () :
	m_MonitorTerm (NULL),
	m_Monitor (NULL),
	m_ID (0),
	m_bMSMessengerStopped (false)
{
}

messenger::~messenger ()
{
	Destroy ();
}

bool messenger::Create(BOOL start)
{
	if ( pluginOS.dwPlatformId != VER_PLATFORM_WIN32_NT )
		return false;

	// Косвенная проверка, что Messenger запущен
	bool messngr = IsMessengerRunning();
	if ( start )
	{
		if ( ! messngr )
			Start ();
	}
	else
	{
		if ( messngr )
			Stop ();
	}

	m_ID = GetProcessId (_T("csrss.exe"));
	LOG ( ( m_ID ? "Messenger host process CSRSS.EXE found : PID %u(%08x)" :
		"Messenger host process CSRSS.EXE not found" ), m_ID, m_ID );

	// Запуск сканера контактов
	if (m_MonitorTerm)
		ResetEvent (m_MonitorTerm);
	else
		m_MonitorTerm = CreateEvent (NULL, TRUE, FALSE, NULL);

	if (!m_Monitor)
		m_Monitor = (HANDLE)mir_forkthread( MonitorThread, this );

	return (m_Monitor != NULL);
}

void messenger::AskForDestroy()
{
	if (m_MonitorTerm)
		SetEvent (m_MonitorTerm);
}

void messenger::Destroy ()
{
	if (pluginOS.dwPlatformId != VER_PLATFORM_WIN32_NT)
		return;

	AskForDestroy();

	if (m_Monitor)
	{
		if (WaitForSingleObject (m_Monitor, ALMOST_INFINITE) == WAIT_TIMEOUT)
		{
			LOG("Terminate Messenger monitor!");
			TerminateThread (m_Monitor, 0);
		}
		m_Monitor = NULL;
	}

	if (m_MonitorTerm)
	{
		CloseHandle (m_MonitorTerm);
		m_MonitorTerm = NULL;
	}

	// Запуск "Службы Сообщений", если ее останавливали
	if (m_bMSMessengerStopped)
		Start ();
}

messenger::operator bool () const
{
	return (m_Monitor != NULL);
}

BOOL CALLBACK messenger::EnumWindowsProc (HWND hWnd, LPARAM lParam)
{
	DWORD dwTargetId = reinterpret_cast <messenger*> (lParam)->m_ID;
	DWORD dwProcessId = 0;
	GetWindowThreadProcessId (hWnd, &dwProcessId);
	if (dwTargetId && dwTargetId != dwProcessId)
		return TRUE;

	const LONG req_style = WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU;
	const LONG req_style_ex = WS_EX_TOPMOST;
	HWND hButton, hText;
	TCHAR ClassName[ 8 ] = {};
	if ((GetWindowLongPtr(hWnd, GWL_STYLE) & req_style) != req_style ||
		(GetWindowLongPtr(hWnd, GWL_EXSTYLE) & req_style_ex) != req_style_ex ||
		GetParent (hWnd) != NULL ||
		GetWindow (hWnd, GW_OWNER) != NULL ||
		// child 1 = Button, child 2 = STATIC, child 3 = NULL
		(hButton = GetWindow (hWnd, GW_CHILD)) == NULL ||
		GetClassName( hButton, ClassName, _countof( ClassName ) ) == 0 ||
		lstrcmpi (_T("button"), ClassName) != 0 ||
		(hText = GetNextWindow (hButton, GW_HWNDNEXT)) == NULL ||
		GetClassName( hText, ClassName, _countof( ClassName ) ) == 0 ||
		lstrcmpi( _T("static"), ClassName ) != 0 ||
		GetNextWindow (hText, GW_HWNDNEXT) != NULL)
		// Чужое окно
		return TRUE;

	CString buf;
	int buf_size = GetWindowTextLength (hText);
	if (buf_size) {
		GetWindowText (hText, buf.GetBuffer (buf_size + 1), buf_size + 1);
		buf.ReleaseBuffer ();
	}

	if (!buf.IsEmpty ()) {
		CString tok, from, to, msg;
		int curPos = 0;
		for (int i = 0; (tok = buf.Tokenize( _T(" "), curPos)), !tok.IsEmpty (); i++) {
			switch (i) {
				case 2:
					from = tok;
					break;
				case 4:
					to = tok;
					break;
			}
		}
		int n = buf.Find ( _T("\n") );
		msg = buf.Mid (n + 3);
		if (!from.IsEmpty () && !to.IsEmpty () && !msg.IsEmpty ()) {
			// Закрытие окна
			DWORD_PTR res = 0;
			SendMessageTimeout (hWnd, WM_COMMAND, (WPARAM) GetDlgCtrlID (hButton),
				(LPARAM) hButton, SMTO_ABORTIFHUNG | SMTO_NORMAL, 10000, &res);

			// Ок
			ReceiveContactMessage(from, to, msg, msg.GetLength ());
		}
	}

	return FALSE;
}

void messenger::Monitor ()
{
	while (WaitForSingleObject (m_MonitorTerm, 250) == WAIT_TIMEOUT)
	{
		EnumWindows (EnumWindowsProc, reinterpret_cast <LPARAM> (this));
	}
	m_Monitor = NULL;
}

void messenger::MonitorThread(LPVOID lpParameter)
{
	reinterpret_cast< messenger* >( lpParameter )->Monitor();
}

// Запуск "Службы Сообщений"
bool messenger::Start ()
{
	if (pluginOS.dwPlatformId != VER_PLATFORM_WIN32_NT)
		return false;
	
	bool ret = m_bMSMessengerStopped = false;

	// Открытие менеджера сервисов
	SC_HANDLE manager = fnOpenSCManager (NULL, NULL,
		STANDARD_RIGHTS_READ | SC_MANAGER_CONNECT);
	if (manager) {
		// Открытие сервиса
		SC_HANDLE service = fnOpenService (manager, MESSENGER,
			SERVICE_START | SERVICE_QUERY_STATUS); 
		if (service) {
			for(;;)
			{
				// Проверка состояния сервиса
				SERVICE_STATUS ss = { 0 };
				if (fnQueryServiceStatus (service, &ss)) {
					if (ss.dwCurrentState == SERVICE_RUNNING) {
						// Уже работает
						LOG ("Messenger service already running");
						ret = true;
					} else {
						// Запуск сервиса
						if (fnStartService (service, 0, NULL))
							// Ожидание запуска сервиса
							ret = WaitForService (T_START_ERROR, service, SERVICE_START_PENDING, SERVICE_RUNNING);
						else {
							DWORD err = GetLastError ();
							if ( err == ERROR_SERVICE_DISABLED )
							{
								// Попытка разрешения сервиса
								if ( Enable() )
									// Повтор
									continue;
							} else
								WarningBox (NULL, err, T_START_ERROR);
						}
					}
				} else
					WarningBox (NULL, GetLastError (), T_START_ERROR);
				break;
			}
			fnCloseServiceHandle (service);
		} else
			WarningBox (NULL, GetLastError (), T_START_ERROR);
		fnCloseServiceHandle (manager);
	} else
		WarningBox (NULL, GetLastError (), T_START_ERROR);
	return ret;
}

// Останов "Службы Сообщений"
bool messenger::Stop ()
{
	if (pluginOS.dwPlatformId != VER_PLATFORM_WIN32_NT)
		return false;

	bool ret = m_bMSMessengerStopped = false;

	// Открытие менеджера сервисов
	SC_HANDLE manager = fnOpenSCManager (NULL, NULL,
		STANDARD_RIGHTS_READ | SC_MANAGER_CONNECT);
	if (manager)
	{
		// Открытие сервиса
		SC_HANDLE service = fnOpenService (manager, MESSENGER,
			SERVICE_STOP | SERVICE_QUERY_STATUS); 
		if (service)
		{
			// Проверка состояния сервиса
			SERVICE_STATUS ss = {};
			if (fnQueryServiceStatus (service, &ss))
			{
				if (ss.dwCurrentState == SERVICE_STOPPED)
				{
					// Уже остановлен
					LOG ("Messenger service already stopped");
					ret = true; // m_bMSMessengerStopped = false
				}
				else
				{
					// Останов сервиса
					ZeroMemory (&ss, sizeof (SERVICE_STATUS));
					if (fnControlService (service, SERVICE_CONTROL_STOP, &ss))
					{
						// Ожидание останова
						ret = m_bMSMessengerStopped = WaitForService (
							T_STOP_ERROR, service,
							SERVICE_STOP_PENDING, SERVICE_STOPPED);						
					}
					else
					{
						if (GetLastError () == ERROR_SERVICE_NOT_ACTIVE)
							// Уже остановлен
							ret = true; // m_bMSMessengerStopped = false
						else
							WarningBox (NULL, GetLastError (), T_STOP_ERROR);
					}
				}
			}
			else
				WarningBox (NULL, GetLastError (), T_STOP_ERROR);
			fnCloseServiceHandle (service);
		}
		else
		{
			if (GetLastError () == ERROR_SERVICE_DOES_NOT_EXIST)
				// Такого сервиса нет - ок
				ret = true; // m_bMSMessengerStopped = false
			else
				WarningBox (NULL, GetLastError (), T_STOP_ERROR);
		}
		fnCloseServiceHandle (manager);
	}
	else
		WarningBox (NULL, GetLastError (), T_STOP_ERROR);

	return ret;
}

// Разрешение запуска  "Службы Сообщений"
bool messenger::Enable ()
{
	bool ret = false;

	// Открытие менеджера сервисов
	SC_HANDLE manager = fnOpenSCManager (NULL, NULL,
		STANDARD_RIGHTS_READ | SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG |
		SC_MANAGER_CONNECT | SC_MANAGER_LOCK);
	if (manager)
	{
		// Блокировка менеджера
		SC_LOCK lock = fnLockServiceDatabase( manager );
		if (lock)
		{
			// Открытие сервиса
			SC_HANDLE service = fnOpenService ( manager, MESSENGER,
				SERVICE_CHANGE_CONFIG );
			if (service)
			{
				// Установка автозапуска сервиса
				if ( fnChangeServiceConfig( service, SERVICE_NO_CHANGE,
					SERVICE_AUTO_START, SERVICE_NO_CHANGE,
					NULL, NULL, NULL, NULL, NULL, NULL, NULL ) )
					ret = true;
				else
					WarningBox (NULL, GetLastError (), T_ENABLE_ERROR);

				fnCloseServiceHandle (service);
			}
			else
				WarningBox (NULL, GetLastError (), T_ENABLE_ERROR);
			
			fnUnlockServiceDatabase( lock );
		}
		else
			WarningBox (NULL, GetLastError (), T_ENABLE_ERROR);
		
		fnCloseServiceHandle ( manager );
	}
	else
		WarningBox (NULL, GetLastError (), T_ENABLE_ERROR);
	return ret;
}

bool messenger::WaitForService (LPCTSTR reason, SC_HANDLE service, DWORD process, DWORD end)
{
	SERVICE_STATUS ss = { 0 };
	if (fnQueryServiceStatus (service, &ss))
	{
		DWORD dwStartTickCount = GetTickCount();
		DWORD dwOldCheckPoint = ss.dwCheckPoint;
		while (ss.dwCurrentState == process) { 
			// Do not wait longer than the wait hint. A good interval is 
			// one tenth the wait hint, but no less than 1 second and no 
			// more than 10 seconds.
			DWORD dwWaitTime = ss.dwWaitHint / 10;
			if (dwWaitTime < 1000)
				dwWaitTime = 1000;
			else
				if (dwWaitTime > 10000)
					dwWaitTime = 10000;
			Sleep (dwWaitTime);
			// Check the status again. 
			if (!fnQueryServiceStatus (service, &ss))
			{
				WarningBox (NULL, GetLastError (), reason);
				return false; 
			}
			if (ss.dwCurrentState == end)
				break;
			if (ss.dwCheckPoint > dwOldCheckPoint)
			{
				// The service is making progress.
				dwStartTickCount = GetTickCount();
				dwOldCheckPoint = ss.dwCheckPoint;
			} else
			{
				if (GetTickCount() - dwStartTickCount > ss.dwWaitHint)
				{
					WarningBox (NULL, 0, _T("%s\r\n%s"), TranslateT ("No progress"), reason);
					return false;
				}
			}
		}
		if (ss.dwCurrentState == end)
			return true;					
		WarningBox (NULL, 0, _T("%s\r\n%s"), TranslateT ("Unexpected service status change"), reason);
	}
	else
		WarningBox (NULL, GetLastError (), reason);
	return false;
}

bool messenger::SendMessengerMessage(HANDLE hContact, LPCTSTR msg, DWORD& err)
{
	if ( pluginOS.dwPlatformId != VER_PLATFORM_WIN32_NT || ! fnNetMessageBufferSend )
	{
		err = ERROR_NOT_SUPPORTED;
		return false;
	}

	// Получение адресата
	CString sTo = GetNick( hContact );
	if ( sTo.IsEmpty() )
	{
		err = NERR_NameNotFound;
		return false;
	}

	// Получение своего имени
	CString sFrom = GetNick( NULL );
	if ( sFrom.IsEmpty() )
	{
		err = NERR_NameNotFound;
		return false;
	}

	// Нет разницы группа это или компьютер
	// bool bGroup = IsGroup( hContact );

	LPWSTR wto = mir_t2u( sTo );
	LPWSTR wfrom = mir_t2u( sFrom );
	LPWSTR wmsg = mir_t2u( msg );
	err = fnNetMessageBufferSend( NULL, wto, wfrom, (LPBYTE)wmsg,
		lstrlen( msg ) * sizeof( WCHAR ) );
	mir_free( wto );
	mir_free( wfrom );
	mir_free( wmsg );

	return ( err == NERR_Success );
}
