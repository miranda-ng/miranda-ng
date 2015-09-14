/*
Copyright (c) 2013-15 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////

ULONG AsyncHttpRequest::m_reqCount = 0;

AsyncHttpRequest::AsyncHttpRequest()
{
	cbSize = sizeof(NETLIBHTTPREQUEST);
	m_bApiReq = true;
	AddHeader("Connection", "keep-alive");
	AddHeader("Accept-Encoding", "booo");
	pUserInfo = NULL;
	m_iRetry = MAX_RETRIES;
	m_iErrorCode = 0;
	bNeedsRestart = false;
	bIsMainConn = false;
	m_pFunc = NULL;
	bExpUrlEncode = false;
	m_reqNum = ::InterlockedIncrement(&m_reqCount);
	m_priority = rpLow;
}

AsyncHttpRequest::AsyncHttpRequest(CVkProto *ppro, int iRequestType, LPCSTR _url, bool bSecure, VK_REQUEST_HANDLER pFunc, RequestPriority rpPriority)
{
	cbSize = sizeof(NETLIBHTTPREQUEST);
	m_bApiReq = true;
	bIsMainConn = false;
	bExpUrlEncode = ppro->m_bUseNonStandardUrlEncode;
	AddHeader("Connection", "keep-alive");
	AddHeader("Accept-Encoding", "booo");

	flags = VK_NODUMPHEADERS | NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_REDIRECT;
	if (bSecure)
		flags |= NLHRF_SSL;

	if (*_url == '/') {	// relative url leads to a site
		m_szUrl = ((bSecure) ? "https://" : "http://") + CMStringA("api.vk.com");
		m_szUrl += _url;
		bIsMainConn = true;
	}
	else m_szUrl = _url;

	if (bSecure)
		this << CHAR_PARAM("access_token", ppro->m_szAccessToken);

	requestType = iRequestType;
	m_pFunc = pFunc;
	pUserInfo = NULL;
	m_iRetry = MAX_RETRIES;
	m_iErrorCode = 0;
	bNeedsRestart = false;
	m_reqNum = ::InterlockedIncrement(&m_reqCount);
	m_priority = rpPriority;
}

AsyncHttpRequest::~AsyncHttpRequest()
{
	for (int i = 0; i < headersCount; i++) {
		mir_free(headers[i].szName);
		mir_free(headers[i].szValue);
	}
	mir_free(headers);
	mir_free(pData);
}

void AsyncHttpRequest::AddHeader(LPCSTR szName, LPCSTR szValue)
{
	headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
	headers[headersCount].szName = mir_strdup(szName);
	headers[headersCount].szValue = mir_strdup(szValue);
	headersCount++;
}

void AsyncHttpRequest::Redirect(NETLIBHTTPREQUEST *nhr)
{
	for (int i = 0; i < nhr->headersCount; i++) {
		LPCSTR szValue = nhr->headers[i].szValue;
		if (!_stricmp(nhr->headers[i].szName, "Location"))
			m_szUrl = szValue;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

CVkFileUploadParam::CVkFileUploadParam(MCONTACT _hContact, const TCHAR* _desc, TCHAR** _files) :
	hContact(_hContact),
	Desc(mir_tstrdup(_desc)),
	FileName(mir_tstrdup(_files[0])),
	atr(NULL),
	fname(NULL),
	filetype(typeInvalid)
{}

CVkFileUploadParam::~CVkFileUploadParam()
{
	mir_free(Desc);
	mir_free(FileName);
	mir_free(atr);
	mir_free(fname);
}

CVkFileUploadParam::VKFileType CVkFileUploadParam::GetType()
{
	if (filetype != typeInvalid)
		return filetype;

	if (atr)
		mir_free(atr);
	if (fname)
		mir_free(fname);

	TCHAR img[] = _T(".jpg .jpeg .png .bmp");
	TCHAR audio[] = _T(".mp3");

	TCHAR DRIVE[3], DIR[256], FNAME[256], EXT[256];
	_tsplitpath(FileName, DRIVE, DIR, FNAME, EXT);

	T2Utf pszFNAME(FNAME), pszEXT(EXT);
	CMStringA fn(FORMAT, "%s%s", pszFNAME, pszEXT);
	fname = mir_strdup(fn);

	if (tlstrstr(img, EXT)) {
		filetype = CVkFileUploadParam::typeImg;
		atr = mir_strdup("photo");
	}
	else if (tlstrstr(audio, EXT)) {
		filetype = CVkFileUploadParam::typeAudio;
		atr = mir_strdup("file");
	}
	else {
		filetype = CVkFileUploadParam::typeDoc;
		atr = mir_strdup("file");
	}

	return filetype;
}

/////////////////////////////////////////////////////////////////////////////////////////

CVkChatUser* CVkChatInfo::GetUserById(LPCTSTR ptszId)
{
	int user_id = _ttoi(ptszId);
	return m_users.find((CVkChatUser*)&user_id);
}

/////////////////////////////////////////////////////////////////////////////////////////