/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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

#include "stdafx.h"

MHttpRequest::MHttpRequest()
{
	cbSize = sizeof(NETLIBHTTPREQUEST);
	requestType = REQUEST_GET;
}

MHttpRequest::~MHttpRequest()
{
	for (int i = 0; i < headersCount; i++) {
		mir_free(headers[i].szName);
		mir_free(headers[i].szValue);
	}
	mir_free(headers);
	mir_free(pData);
}

void MHttpRequest::AddHeader(LPCSTR szName, LPCSTR szValue)
{
	for (int i = 0; i < headersCount; i++)
		if (!mir_strcmp(headers[i].szName, szName)) {
			replaceStr(headers[i].szValue, szValue);
			return;
		}

	headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
	headers[headersCount].szName = mir_strdup(szName);
	headers[headersCount].szValue = mir_strdup(szValue);
	headersCount++;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(MHttpRequest*) operator<<(MHttpRequest *pReq, const INT_PARAM &param)
{
	CMStringA &s = pReq->m_szParam;
	if (!s.IsEmpty())
		s.AppendChar('&');
	s.AppendFormat("%s=%ld", param.szName, param.iValue);
	return pReq;
}

MIR_APP_DLL(MHttpRequest*) operator<<(MHttpRequest *pReq, const INT64_PARAM &param)
{
	CMStringA &s = pReq->m_szParam;
	if (!s.IsEmpty())
		s.AppendChar('&');
	s.AppendFormat("%s=%lld", param.szName, param.iValue);
	return pReq;
}

MIR_APP_DLL(MHttpRequest*) operator<<(MHttpRequest *pReq, const CHAR_PARAM &param)
{
	CMStringA &s = pReq->m_szParam;
	if (!s.IsEmpty())
		s.AppendChar('&');
	s.AppendFormat("%s=%s", param.szName, mir_urlEncode(param.szValue).c_str());
	return pReq;
}

MIR_APP_DLL(MHttpRequest*) operator<<(MHttpRequest *pReq, const WCHAR_PARAM &param)
{
	T2Utf szValue(param.wszValue);
	CMStringA &s = pReq->m_szParam;
	if (!s.IsEmpty())
		s.AppendChar('&');
	s.AppendFormat("%s=%s", param.szName, mir_urlEncode(szValue).c_str());
	return pReq;
}
