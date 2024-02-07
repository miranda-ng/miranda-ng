/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#ifndef M_FILE_H__
#define M_FILE_H__ 1

namespace File
{
	MIR_APP_DLL(wchar_t *) GetReceivedFolder(MCONTACT hContact, wchar_t *pwszDest, size_t cbDest, bool substVars = true);

	// List of files should be null-terminated
	MIR_APP_DLL(MWindow) Send(MCONTACT hContact, wchar_t** const ppFiles = nullptr);
	
};

// notifies a caller about file send start
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(HWND)hwndDialog
#define ME_FILEDLG_SUCCEEDED "SRFile/Dlg/Succeeded"

// notifies a caller about file send cancellation
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(HWND)hwndDialog
#define ME_FILEDLG_CANCELED "SRFile/Dlg/Canceled"

#endif // M_FILE_H__
