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

class messenger
{
public:
	messenger ();
	~messenger ();

	bool Create (BOOL start);
	void AskForDestroy();			// Для ускорения
	void Destroy ();
	operator bool () const;
	
	bool Start ();					// Запуск "Службы Сообщений"
	bool Stop ();					// Останов "Службы Сообщений"
	bool Enable ();					// Разрешение запуска  "Службы Сообщений"
	
	bool SendMessengerMessage(HANDLE hContact, LPCTSTR msg, DWORD& err);

protected:
	HANDLE	m_MonitorTerm;			// Событие для остановки
	HANDLE	m_Monitor;				// Периодическая проверка
	DWORD	m_ID;					// ID процесса CSRSS.EXE
	bool	m_bMSMessengerStopped;	// Флаг показывающий был ли остановлен
									// MS Messenger с помощью функции Stop()

	void Monitor ();
	static BOOL CALLBACK EnumWindowsProc (HWND hwnd, LPARAM lParam);
	static void MonitorThread (LPVOID lpParameter);
	static bool WaitForService (LPCTSTR reason, SC_HANDLE service, DWORD process, DWORD end);
};

extern messenger pluginMessenger;			// Прием/отправка сообщений через Messenger
