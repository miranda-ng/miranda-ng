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

INT_PTR(*g_MirCallService)(const char *, WPARAM, LPARAM) = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////

CSendHTTPServer::CSendHTTPServer(HWND Owner, MCONTACT hContact, bool /*bAsync*/)
	: CSend(Owner, hContact, true)
{
	m_EnableItem = SS_DLG_DESCRIPTION; //| SS_DLG_AUTOSEND | SS_DLG_DELETEAFTERSSEND;
	m_pszSendTyp = LPGENW("HTTPServer transfer");
	m_pszFileName = nullptr;
	m_fsi_pszRealPath = nullptr;
}

CSendHTTPServer::~CSendHTTPServer()
{
	mir_free(m_pszFileName);
	mir_free(m_fsi_pszRealPath);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CSendHTTPServer::Send()
{
	if (!m_hContact) return 1;
	if (CallService(MS_HTTP_ACCEPT_CONNECTIONS, TRUE, 0) != 0) {
		Error(LPGENW("Could not start the HTTP Server plugin."));
		Exit(ACKRESULT_FAILED);
		return !m_bAsync;
	}

	if (!m_pszFileName) {
		m_pszFileName = GetFileNameA(m_pszFile);
	}

	m_fsi_pszSrvPath.Empty();
	m_fsi_pszSrvPath.AppendChar('/');
	m_fsi_pszSrvPath.Append(m_pszFileName);

	replaceStr(m_fsi_pszRealPath, _T2A(m_pszFile));

	memset(&m_fsi, 0, sizeof(m_fsi));
	m_fsi.lStructSize = sizeof(STFileShareInfo);
	m_fsi.nMaxDownloads = -1;					// -1 = infinite
	m_fsi.pszRealPath = m_fsi_pszRealPath;

	// start Send thread
	mir_forkthread(&CSendHTTPServer::SendThreadWrapper, this);
	return 0;
}

void CSendHTTPServer::SendThread()
{
	INT_PTR ret;

	if (ServiceExists(MS_HTTP_GET_LINK)) {
		// patched plugin version
		ret = CallService(MS_HTTP_ADD_CHANGE_REMOVE, (WPARAM)m_hContact, (LPARAM)&m_fsi);
		if (!ret) {
			m_URL = ptrA((char*)CallService(MS_HTTP_GET_LINK, (WPARAM)m_fsi.pszSrvPath, 0));
		}
	}
	else {
		// original plugin
		m_fsi.dwOptions = OPT_SEND_LINK;

		// send DATA and wait for reply
		ret = CallService(MS_HTTP_ADD_CHANGE_REMOVE, (WPARAM)m_hContact, (LPARAM)&m_fsi);
	}

	if (ret != 0) {
		Error(LPGENW("%s (%i):\nCould not add a share to the HTTP Server plugin."), TranslateW(m_pszSendTyp), ret);
		Exit(ret); return;
	}

	// Share the file by HTTP Server plugin, SendSS does not own the file anymore = auto-delete won't work
	m_bDeleteAfterSend = false;

	if (m_URL && *m_URL) {
		svcSendMsgExit(m_URL); return;
	}
	Exit(ACKRESULT_FAILED);
}

void CSendHTTPServer::SendThreadWrapper(void * Obj)
{
	reinterpret_cast<CSendHTTPServer*>(Obj)->SendThread();
}
