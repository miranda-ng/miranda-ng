/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

//---------------------------------------------------------------------------
#include "global.h"

INT_PTR (*g_MirCallService)(const char *, WPARAM, LPARAM)=NULL;
//INT_PTR (*CallService)(const char *,WPARAM,LPARAM);


//---------------------------------------------------------------------------
CSendHTTPServer::CSendHTTPServer(HWND Owner, MCONTACT hContact, bool /*bAsync*/)
: CSend(Owner, hContact, true){
	m_EnableItem		= SS_DLG_DESCRIPTION ; //| SS_DLG_AUTOSEND | SS_DLG_DELETEAFTERSSEND;
	m_pszSendTyp		= LPGENT("HTTPServer transfer");
	m_pszFileName		= NULL;
	m_fsi_pszSrvPath	= NULL;
	m_fsi_pszRealPath	= NULL;
}

CSendHTTPServer::~CSendHTTPServer(){
	mir_free(m_pszFileName);
	mir_free(m_fsi_pszSrvPath);
	mir_free(m_fsi_pszRealPath);
}

//---------------------------------------------------------------------------
int CSendHTTPServer::Send()
{
	if(!m_hContact) return 1;
	if (CallService(MS_HTTP_ACCEPT_CONNECTIONS, (WPARAM)true, 0) != 0) {
		Error(LPGENT("Could not start the HTTP Server plugin."));
		Exit(ACKRESULT_FAILED);
		return !m_bAsync;
	}

	if (!m_pszFileName) {
		m_pszFileName = GetFileNameA(m_pszFile);
	}
	mir_freeAndNil(m_fsi_pszSrvPath);
	mir_tcsadd(m_fsi_pszSrvPath, _T("/"));
	mir_tcsadd(m_fsi_pszSrvPath, _A2T(m_pszFileName));

	replaceStrT(m_fsi_pszRealPath, m_pszFile);

	ZeroMemory(&m_fsi, sizeof(m_fsi));
	m_fsi.lStructSize	= sizeof(STFileShareInfo);
	m_fsi.pszSrvPath	= m_fsi_pszSrvPath;
	m_fsi.nMaxDownloads	= -1;					// -1 = infinite
	m_fsi.pszRealPath	= m_fsi_pszRealPath;
	//m_fsi.dwOptions		= NULL;					//OPT_SEND_LINK only work on single chat;

	//start Send thread
	mir_forkthread(&CSendHTTPServer::SendThreadWrapper, this);
	return 0;
}

void CSendHTTPServer::SendThread() {
	INT_PTR ret;

	if (ServiceExists(MS_HTTP_GET_LINK)) {
		//patched plugin version
		ret = CallService(MS_HTTP_ADD_CHANGE_REMOVE, (WPARAM)m_hContact, (LPARAM)&m_fsi);
		if (!ret) {
			mir_free(m_URL);
			m_URL = (char*)CallService(MS_HTTP_GET_LINK, (WPARAM)m_fsi.pszSrvPath, NULL);
		}
	}
	else {
		//original plugin
		m_fsi.dwOptions  = OPT_SEND_LINK;

		//send DATA and wait for reply
		ret = CallService(MS_HTTP_ADD_CHANGE_REMOVE, (WPARAM)m_hContact, (LPARAM)&m_fsi);
	}
 
	if (ret != 0) {
		Error(LPGENT("%s (%i):\nCould not add a share to the HTTP Server plugin."),TranslateTS(m_pszSendTyp),ret);
		Exit(ret); return;
	}

	//Share the file by HTTP Server plugin, SendSS does not own the file anymore = auto-delete won't work
	m_bDeleteAfterSend = false;

	if (m_URL && *m_URL) {
		svcSendMsgExit(m_URL); return;
	}
	Exit(ACKRESULT_FAILED);
}

void CSendHTTPServer::SendThreadWrapper(void * Obj) {
	reinterpret_cast<CSendHTTPServer*>(Obj)->SendThread();
}

//---------------------------------------------------------------------------
/*
CSendHTTPServer::CContactMapping CSendHTTPServer::_CContactMapping;
INT_PTR CSendHTTPServer::MyCallService(const char *name, WPARAM wParam, LPARAM lParam) {
/ *
	CContactMapping::iterator Contact(_CContactMapping.end());
	if ( wParam == m_hContact && (
		(strcmp(name, MS_MSG_SENDMESSAGE)== 0) ||
		(strcmp(name, "SRMsg/LaunchMessageWindow")== 0) ))
	{
		m_URL= mir_strdup((char*)lParam);
		return 0;
	}* /
	return g_MirCallService(name, wParam, lParam);
}*/
