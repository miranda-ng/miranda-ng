/*
            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2014-22 Miranda NG team (https://miranda-ng.org)

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
	if (!g_hNetlibUser) { // check Netlib
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED);
		return !m_bAsync;
	}
	memset(&m_nlhr, 0, sizeof(m_nlhr));
	char* tmp; tmp = mir_u2a(m_pszFile);
	HTTPFormData frm[] = {
		{ "Authorization", HTTPFORM_HEADER("Client-ID 2a7303d78abe041") },
		{ "image", HTTPFORM_FILE(tmp) },
	};

	int error = HTTPFormCreate(&m_nlhr, REQUEST_POST, "https://api.imgur.com/3/image", frm, _countof(frm));
	mir_free(tmp);
	if (error)
		return !m_bAsync;
	// start upload thread
	if (m_bAsync) {
		mir_forkthread(&CSendHost_Imgur::SendThread, this);
		return 0;
	}
	SendThread(this);
	return 1;
}

void CSendHost_Imgur::SendThread(void* obj)
{
	CSendHost_Imgur *self = (CSendHost_Imgur*)obj;
	// send DATA and wait for m_nlreply
	NLHR_PTR reply(Netlib_HttpTransaction(g_hNetlibUser, &self->m_nlhr));
	self->HTTPFormDestroy(&self->m_nlhr);
	if (reply) {
		if (reply->dataLength) {
			JSONROOT root(reply->pData);
			if (root) {
				if ((*root)["success"].as_bool()) {
					self->m_URL = (*root)["data"]["link"].as_mstring();
					int idx = self->m_URL.ReverseFind('.');
					if (idx != -1) {
						self->m_URLthumb = self->m_URL;
						self->m_URLthumb.Insert(idx, 'm');
					}
					self->svcSendMsgExit(self->m_URL); return;
				}
				else self->Error(SS_ERR_RESPONSE, self->m_pszSendTyp, (*root)["status"].as_int(), 0);
			}
			else self->Error(SS_ERR_RESPONSE, self->m_pszSendTyp, reply->resultCode);
		}
		else self->Error(SS_ERR_RESPONSE, self->m_pszSendTyp, reply->resultCode);
	}
	else self->Error(SS_ERR_NORESPONSE, self->m_pszSendTyp, self->m_nlhr.resultCode);

	self->Exit(ACKRESULT_FAILED);
}
