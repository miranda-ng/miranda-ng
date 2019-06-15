/*

Facebook plugin for Miranda NG
Copyright © 2019 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdafx.h"

AsyncHttpRequest* FacebookProto::CreateGraphql(const char *szName, const char *szMethod)
{
	AsyncHttpRequest *pReq = new AsyncHttpRequest();
	pReq->requestType = REQUEST_POST;
	pReq->szUrl = "https://graph.facebook.com/graphql";
	pReq << CHAR_PARAM("api_key", FB_APP_KEY) << CHAR_PARAM("device_id", szDeviceID) << CHAR_PARAM("fb_api_req_friendly_name", szName)
		<< CHAR_PARAM("format", "json") << CHAR_PARAM("method", szMethod);

	CMStringA szLocale = getMStringA(DBKEY_LOCALE);
	if (szLocale.IsEmpty())
		szLocale = "en";
	pReq << CHAR_PARAM("locale", szLocale);

	unsigned int id;
	Utils_GetRandom(&id, sizeof(id));
	id &= ~0x80000000;
	pReq << INT_PARAM("queryid", id);

	return pReq;
}

void AsyncHttpRequest::CalcSig()
{
	CMStringA szSrc = m_szParam;
	szSrc.Append(FB_APP_SECRET);
}
