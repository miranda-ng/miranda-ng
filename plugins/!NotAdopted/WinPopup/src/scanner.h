/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2006 Nikolay Raspopov

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

class contact_scanner
{
public:
	contact_scanner ();
	~contact_scanner ();

	bool Create ();
	void AskForDestroy();		// Для ускорения
	void Destroy ();
	operator bool () const;

	// Проверка статуса контакта (асинхронная и синхронная)
	static int ScanContact(HANDLE  hContact);

protected:
	HANDLE	m_ScannerTerm;		// Событие для остановки
	HANDLE	m_Scanner;			// Хэндлер нити периодической проверки контактов

	// Получить следующий контакт нуждающийся в проверке
	static HANDLE GetNextScannableContact ();
	// Рабочая нить цикла периодической проверки контактов
	void Scanner ();
	static void ScannerThread (LPVOID lpParameter);
};

extern contact_scanner pluginScanner;
