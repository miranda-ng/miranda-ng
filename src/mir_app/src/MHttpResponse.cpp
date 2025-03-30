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

MHttpResponse::MHttpResponse()
{
}

MHttpResponse::~MHttpResponse()
{
	mir_free(szUrl);
	mir_free(szResultDescr);
}

/////////////////////////////////////////////////////////////////////////////////////////

JsonReply::JsonReply(MHttpResponse *pReply)
{
	if (pReply == nullptr) {
		m_errorCode = 500;
		return;
	}

	m_errorCode = pReply->resultCode;
	if (m_errorCode != 200)
		return;

	m_root = json_parse(pReply->body);
	if (m_root == nullptr) {
		m_errorCode = 500;
		return;
	}
}

JsonReply::~JsonReply()
{
	json_delete(m_root);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MHttpResponse helpers for Pascal
// declared only in m_netlib.inc

EXTERN_C MIR_APP_DLL(int) Netlib_HttpResult(MHttpResponse *nlhr)
{
	return (nlhr) ? nlhr->resultCode : 500;
}

EXTERN_C MIR_APP_DLL(char *) Netlib_HttpBuffer(MHttpResponse *nlhr, int &cbLen)
{
	if (!nlhr)
		return nullptr;

	cbLen = nlhr->body.GetLength();
	return nlhr->body.Detach();
}

EXTERN_C MIR_APP_DLL(char *) Netlib_HttpCookies(MHttpResponse *nlhr)
{
	if (!nlhr)
		return nullptr;

	return nlhr->GetCookies().Detach();
}
