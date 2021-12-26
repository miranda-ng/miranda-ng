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
#include "serverlist.h"

struct Options 
{
	enum ETimeRange
	{
		TR_MINUTES= 0,
		TR_HOURS, 
		TR_DAYS
	};

	uint8_t selected;
	uint8_t defaultFTP;
	bool bCloseDlg;
	bool bCopyLink;
	bool bAutosend;
	bool bUseSubmenu;
	bool bHideInactive;
	bool bAutoDelete;
	int iCompressionLevel;
	bool bSetZipName;
	int iDeleteTime;
	ETimeRange timeRange;

	static void enableItems(HWND hwndDlg, bool state);
	static int InitOptions(WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK DlgProcOptsAccounts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK DlgProcOptsAdvanced(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);	

	void loadOptions();	
	void saveOptions() const;
};

extern Options opt;
