/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 Sérgio Vieira Rolanski (portet from Borland C++)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////

CSendFTPFile::CSendFTPFile(HWND Owner, MCONTACT hContact, bool /*bAsync*/)
	: CSend(Owner, hContact, true)
{
	m_EnableItem = 0; //SS_DLG_DESCRIPTION | SS_DLG_AUTOSEND | SS_DLG_DELETEAFTERSSEND;
	m_pszSendTyp = LPGENW("FTPFile transfer");
	m_pszFileName = nullptr;
}

CSendFTPFile::~CSendFTPFile()
{
	mir_free(m_pszFileName);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CSendFTPFile::Send()
{
	if (!m_hContact) return 1;
	/*********************************************************************************************
	 * Send file (files) to the FTP server and copy file URL
	 * to message log or clipboard (according to plugin setting)
	 *   wParam = (HANDLE)hContact
	 *   lParam = (char *)filename
	 * Filename format is same as GetOpenFileName (OPENFILENAME.lpstrFile) returns,
	 * see http://msdn2.microsoft.com/en-us/library/ms646839.aspx
	 * Returns 0 on success or nonzero on failure
	 * if (!wParam || !lParam) return 1
	 ********************************************************************************************/
	mir_free(m_pszFileName);
	m_pszFileName = GetFileNameA(m_pszFile);
	size_t size = sizeof(char)*(mir_strlen(m_pszFileName) + 2);
	m_pszFileName = (char*)mir_realloc(m_pszFileName, size);
	m_pszFileName[size - 1] = NULL;

	// start Send thread
	mir_forkthread(&CSendFTPFile::SendThreadWrapper, this);
	return 0;
}

void CSendFTPFile::SendThread()
{

	INT_PTR ret = FTPFileUploadA(m_hContact, FNUM_DEFAULT, FMODE_RAWFILE, &m_pszFileName, 1);
	if (ret != 0) {
		Error(LPGENW("%s (%i):\nCould not add a share to the FTP File plugin."), TranslateW(m_pszSendTyp), ret);
		Exit(ret); return;
	}

	// Can't delete the file since FTP File plugin will use it
	m_bDeleteAfterSend = false;

	if (m_URL && *m_URL) {/// @fixme : m_URL never set
		svcSendMsgExit(m_URL); return;
	}
	Exit(ACKRESULT_FAILED);
}

void	CSendFTPFile::SendThreadWrapper(void * Obj)
{
	reinterpret_cast<CSendFTPFile*>(Obj)->SendThread();
}
