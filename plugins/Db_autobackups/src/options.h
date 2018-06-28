/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#pragma once

enum BackupType
{
	BT_DISABLED = 0,
	BT_START = 1,
	BT_EXIT = 2,
	BT_PERIODIC = 4
};

enum PeriodType
{
	PT_DAYS,
	PT_HOURS,
	PT_MINUTES
};

struct Options
{
	CMOption<BYTE>	backup_types;
	CMOption<WORD>	period;
	CMOption<BYTE>	period_type;
	wchar_t			folder[MAX_PATH];
	CMOption<WORD>	num_backups;
	CMOption<BYTE>	disable_progress;
	CMOption<BYTE>	disable_popups;
	CMOption<BYTE>	use_zip;
	CMOption<BYTE>	backup_profile;
	CMOption<BYTE>	use_cloudfile;
	CMOption<char*>	cloudfile_service;

	Options();
};
