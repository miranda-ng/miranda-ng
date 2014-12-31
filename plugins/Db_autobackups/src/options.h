/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

typedef enum { BT_DISABLED = 0, BT_START = 1, BT_EXIT = 2, BT_PERIODIC = 4 } BackupType;
typedef enum { PT_DAYS, PT_HOURS, PT_MINUTES } PeriodType;

typedef struct Options_tag {
	int	backup_types;
	unsigned int period;
	PeriodType period_type;
	TCHAR	folder[MAX_PATH];
	size_t	num_backups;
	BOOL	disable_progress;
	BOOL	disable_popups;
	BOOL	use_zip;
} Options;

extern Options options;