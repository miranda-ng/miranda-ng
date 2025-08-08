/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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

CSendHost_ImageShack::CSendHost_ImageShack(HWND Owner, MCONTACT hContact, bool bAsync) :
	CSend(Owner, hContact, bAsync)
{
	m_EnableItem = SS_DLG_DESCRIPTION | SS_DLG_AUTOSEND | SS_DLG_DELETEAFTERSSEND;
	m_pszSendTyp = LPGENW("Image upload");
}

CSendHost_ImageShack::~CSendHost_ImageShack()
{
}

/////////////////////////////////////////////////////////////////////////////////////////

int CSendHost_ImageShack::Send()
{
	if (!g_hNetlibUser) { // check Netlib
LBL_Error:
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED);
		return !m_bAsync;
	}

	CMStringA szKey(g_plugin.getMStringA("Key"));
	if (szKey.IsEmpty()) {
		ENTER_STRING es = {};
		es.szModuleName = MODULENAME;
		es.caption = TranslateT("Enter your personal API key from Imageshack");
		if (!EnterString(&es)) {
			m_pszSendTyp = LPGENW("API key is missing");
			goto LBL_Error;
		}

		szKey = es.ptszResult;
		g_plugin.setString("Key", szKey);
	}

	m_pRequest.reset(new MHttpRequest(REQUEST_POST));
	T2Utf tmp(m_pszFile);
	HTTPFormData frm[] = {
		// { "Referer", HTTPFORM_HEADER("http://www.imageshack.us/upload_api.php") },
		{ "fileupload", HTTPFORM_FILE(tmp) },
		// { "rembar", "yes" },// no info bar on thumb
		{ "public", "no" },
		{ "key", szKey.c_str() },
	};

	int error = HTTPFormCreate(m_pRequest.get(), "http://imageshack.us/upload_api.php", frm, sizeof(frm) / sizeof(HTTPFormData));
	if (error)
		return !m_bAsync;
	
	// start upload thread
	if (m_bAsync) {
		mir_forkthread(&CSendHost_ImageShack::SendThreadWrapper, this);
		return 0;
	}
	SendThread();
	return 1;
}

void CSendHost_ImageShack::SendThread()
{
	// send DATA and wait for m_nlreply
	NLHR_PTR reply(Netlib_HttpTransaction(g_hNetlibUser, m_pRequest.get()));
	if (reply) {
		if (reply->resultCode >= 200 && reply->resultCode < 300 && reply->body.GetLength()) {
			const char *url = nullptr;
			url = GetHTMLContent(reply->body.GetBuffer(), "<image_link>", "</image_link>");
			if (url && *url) {
				m_URLthumb = m_URL = url;

				int idx = m_URLthumb.ReverseFind('.');
				if (idx != -1 && m_URLthumb.GetLength() - idx > 2)
					m_URLthumb.Insert(idx + 1, "th");
				else
					m_URLthumb.Empty();

				svcSendMsgExit(url);
				return;
			}

			url = GetHTMLContent(reply->body.GetBuffer(), "<error ", "</error>");
			wchar_t *err = nullptr;
			if (url) err = mir_a2u(url);
			if (!err || !*err) { // fallback to server response mess
				mir_free(err);
				err = mir_a2u(reply->body);
			}
			Error(L"%s", err);
			mir_free(err);
		}
		else Error(SS_ERR_RESPONSE, m_pszSendTyp, reply->resultCode);
	}
	else Error(SS_ERR_NORESPONSE, m_pszSendTyp, 500);

	Exit(ACKRESULT_FAILED);
}

void CSendHost_ImageShack::SendThreadWrapper(void *Obj)
{
	reinterpret_cast<CSendHost_ImageShack *>(Obj)->SendThread();
}
