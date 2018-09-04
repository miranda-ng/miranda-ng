/*
            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2014-18 Miranda NG team (https://miranda-ng.org)

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
*/
#include "stdafx.h"

CSendHost_Imgur::CSendHost_Imgur(HWND Owner, MCONTACT hContact, bool bAsync)
	: CSend(Owner, hContact, bAsync)
{
	m_EnableItem = SS_DLG_DESCRIPTION | SS_DLG_AUTOSEND | SS_DLG_DELETEAFTERSSEND;
	m_pszSendTyp = LPGENW("Image upload");
}

CSendHost_Imgur::~CSendHost_Imgur()
{
}

/////////////////////////////////////////////////////////////////////////////////////////

int CSendHost_Imgur::Send()
{
	if (!g_hNetlibUser) { /// check Netlib
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED);
		return !m_bAsync;
	}
	memset(&m_nlhr, 0, sizeof(m_nlhr));
	char* tmp; tmp = mir_u2a(m_pszFile);
	HTTPFormData frm[] = {
		{ "Authorization", HTTPFORM_HEADER("Client-ID 2a7303d78abe041") },
		{ "image", HTTPFORM_FILE(tmp) },
		//		{"name",""},// filename (detected if multipart / form-data)
		//		{"title",""},
		//		{"description",""},
	};
	int error = HTTPFormCreate(&m_nlhr, REQUEST_POST, "https://api.imgur.com/3/image", frm, sizeof(frm) / sizeof(HTTPFormData));
	mir_free(tmp);
	if (error)
		return !m_bAsync;
	/// start upload thread
	if (m_bAsync) {
		mir_forkthread(&CSendHost_Imgur::SendThread, this);
		return 0;
	}
	SendThread(this);
	return 1;
}

void CSendHost_Imgur::SendThread(void* obj)
{
	CSendHost_Imgur* self = (CSendHost_Imgur*)obj;
	/// send DATA and wait for m_nlreply
	NETLIBHTTPREQUEST* reply = Netlib_HttpTransaction(g_hNetlibUser, &self->m_nlhr);
	self->HTTPFormDestroy(&self->m_nlhr);
	if (reply) {
		if (reply->dataLength) {
			char buf[128];

			if (GetJSONBool(reply->pData, reply->dataLength, "success")) {
				GetJSONString(reply->pData, reply->dataLength, "data[link]", buf, sizeof(buf));

				self->m_URL = buf;
				int idx = self->m_URL.ReverseFind('.');
				if (idx != -1) {
					self->m_URLthumb = self->m_URL;
					self->m_URLthumb.Insert(idx, 'm');
				}
				Netlib_FreeHttpRequest(reply);
				self->svcSendMsgExit(self->m_URL); return;
			}
			else self->Error(SS_ERR_RESPONSE, self->m_pszSendTyp, GetJSONInteger(reply->pData, reply->dataLength, "status", 0));
		}
		else self->Error(SS_ERR_RESPONSE, self->m_pszSendTyp, reply->resultCode);

		Netlib_FreeHttpRequest(reply);
	}
	else self->Error(SS_ERR_NORESPONSE, self->m_pszSendTyp, self->m_nlhr.resultCode);

	self->Exit(ACKRESULT_FAILED);
}
