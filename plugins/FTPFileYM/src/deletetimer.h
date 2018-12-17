/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "stdafx.h"
#include "options.h"

class DeleteTimer 
{	
private:
	UINT timerId = 0;

	static void CALLBACK AutoDeleteTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

public:
	void init();
	void deinit();
	void start();
	void stop();
};

extern DeleteTimer deleteTimer;
