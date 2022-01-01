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

CSendHost_UploadPie::CSendHost_UploadPie(HWND Owner, MCONTACT hContact, bool bAsync, int expire)
	: m_expire(expire), CSend(Owner, hContact, bAsync)
{
	m_EnableItem = SS_DLG_DESCRIPTION | SS_DLG_AUTOSEND | SS_DLG_DELETEAFTERSSEND;
	m_pszSendTyp = LPGENW("Image upload");
}

CSendHost_UploadPie::~CSendHost_UploadPie()
{
}

/////////////////////////////////////////////////////////////////////////////////////////

static const char kHostURL[] = "https://uploadpie.com/";

int CSendHost_UploadPie::Send()
{
	if (!g_hNetlibUser) { // check Netlib
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED);
		return !m_bAsync;
	}
	memset(&m_nlhr, 0, sizeof(m_nlhr));
	char* tmp; tmp = mir_u2a(m_pszFile);
	HTTPFormData frm[] = {
		{ "MAX_FILE_SIZE", HTTPFORM_INT(3145728) },
		{ "upload", HTTPFORM_INT(1) },
		{ "uploadedfile", HTTPFORM_FILE(tmp) },
		{ "expire", HTTPFORM_INT(m_expire) },
	};

	int error = HTTPFormCreate(&m_nlhr, REQUEST_POST, kHostURL, frm, _countof(frm));
	mir_free(tmp);
	if (error)
		return !m_bAsync;
	
	// start upload thread
	if (m_bAsync) {
		mir_forkthread(&CSendHost_UploadPie::SendThread, this);
		return 0;
	}
	SendThread(this);
	return 1;
}

void CSendHost_UploadPie::SendThread(void* obj)
{
	CSendHost_UploadPie* self = (CSendHost_UploadPie*)obj;
	// send DATA and wait for m_nlreply
	NLHR_PTR reply(Netlib_HttpTransaction(g_hNetlibUser, &self->m_nlhr));
	self->HTTPFormDestroy(&self->m_nlhr);
	if (reply) {
		if (reply->resultCode >= 200 && reply->resultCode < 300 && reply->dataLength) {
			reply->pData[reply->dataLength - 1] = '\0'; // make sure its null terminated
			char* url = reply->pData;
			do {
				char* pos;
				if ((url = strstr(url, kHostURL))) {
					for (pos = url + _countof(kHostURL)-1; (*pos >= '0'&&*pos <= '9') || (*pos >= 'a'&&*pos <= 'z') || (*pos >= 'A'&&*pos <= 'Z') || *pos == '_' || *pos == '-' || *pos == '"' || *pos == '\''; ++pos) {
						if (*pos == '"' || *pos == '\'') break;
					}
					if (url + _countof(kHostURL)-1 != pos && (*pos == '"' || *pos == '\'')) {
						*pos = '\0';
						break;
					}
					++url;
				}
			} while (url);
			
			if (url) {
				self->m_URL = url;
				self->svcSendMsgExit(url); return;
			}
			else { // check error mess from server
				const char* err = GetHTMLContent(reply->pData, "<p id=\"error\"", "</p>");
				wchar_t* werr;
				if (err) werr = mir_a2u(err);
				else werr = mir_a2u(reply->pData);
				self->Error(L"%s", werr);
				mir_free(werr);
			}
		}
		else self->Error(SS_ERR_RESPONSE, self->m_pszSendTyp, reply->resultCode);
	}
	else self->Error(SS_ERR_NORESPONSE, self->m_pszSendTyp, self->m_nlhr.resultCode);

	self->Exit(ACKRESULT_FAILED);
}
