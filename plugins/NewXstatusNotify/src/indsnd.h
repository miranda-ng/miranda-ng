/*
	NewXstatusNotify YM - Plugin for Miranda IM
	Copyright (c) 2001-2004 Luca Santarelli
	Copyright (c) 2005-2007 Vasilich
	Copyright (c) 2007-2011 yaho

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	*/

#ifndef INDSND_H
#define INDSND_H

#define EXTRA_IMAGE_SOUND		0
#define EXTRA_IMAGE_POPUP		1
#define EXTRA_IMAGE_XSTATUS		2
#define EXTRA_IMAGE_XLOGGING	3
#define EXTRA_IMAGE_STATUSMSG	4
#define EXTRA_IMAGE_SMSGLOGGING	5
#define EXTRA_IMAGE_DISABLEALL	6
#define EXTRA_IMAGE_ENABLEALL	7
#define EXTRA_IMAGE_DOT			8

#define STATUS_COLUMN			110

#define DEFAULT_SOUND			LPGEN("(default sound)")

int UserInfoInitialise(WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcFiltering(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif