/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team,
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

MHttpRequest::MHttpRequest(int _1) :
	requestType(_1),
	flags(0),
	pUserInfo(nullptr),
	nlc(0),
	timeout(30000)
{
}

MHttpRequest::~MHttpRequest()
{
}

void MHttpRequest::SetData(const void *pData, size_t cbLen)
{
	m_szParam.Truncate((int)cbLen);
	memcpy(m_szParam.GetBuffer(), pData, cbLen);
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

MIR_APP_DLL(MHttpRequest *) operator<<(MHttpRequest *pReq, const BOOL_PARAM &param)
{
	CMStringA &s = pReq->m_szParam;
	if (!s.IsEmpty())
		s.AppendChar('&');
	s.AppendFormat("%s=%s", param.szName, param.bValue ? "true" : "false");
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
