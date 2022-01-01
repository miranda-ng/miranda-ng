/*
Copyright © 2012-22 Miranda NG team
Copyright © 2009 Jim Porter

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

#include <io.h>

http::response CTwitterProto::Execute(AsyncHttpRequest *pReq)
{
	if (pReq->m_szUrl[0] == '/')
		pReq->m_szUrl.Insert(0, TWITTER_BASE_URL);

	bool bIsJson = false;
	if (!pReq->m_szParam.IsEmpty()) {
		if (pReq->requestType == REQUEST_POST) {
			if (pReq->m_szParam[0] == '{') {
				bIsJson = true;
				pReq->AddHeader("Content-Type", "application/json");
			}
			else pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
			pReq->AddHeader("Cache-Control", "no-cache");

			pReq->dataLength = (int)pReq->m_szParam.GetLength();
			pReq->pData = pReq->m_szParam.Detach();
		}
		else {
			pReq->m_szUrl.AppendChar('?');
			pReq->m_szUrl += pReq->m_szParam;
		}
	}

	CMStringA auth;
	if (pReq->requestType == REQUEST_GET)
		auth = OAuthWebRequestSubmit(pReq->m_szUrl, "GET", "");
	else
		auth = OAuthWebRequestSubmit(pReq->m_szUrl, "POST", (bIsJson) ? "" : pReq->pData);
	pReq->AddHeader("Authorization", auth);

	pReq->szUrl = pReq->m_szUrl.GetBuffer();
	pReq->flags = NLHRF_HTTP11 | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	pReq->nlc = m_hConnHttp;
	http::response resp_data;
	NLHR_PTR resp(Netlib_HttpTransaction(m_hNetlibUser, pReq));
	if (resp) {
		debugLogA("**SLURP - the server has responded!");
		m_hConnHttp = resp->nlc;
		resp_data.code = resp->resultCode;
		if (resp->pData)
			resp_data.data = resp->pData;
	}
	else {
		m_hConnHttp = nullptr;
		resp_data.code = 500;
		debugLogA("SLURP - there was no response!");
	}

	delete pReq;
	return resp_data;
}

bool save_url(HNETLIBUSER hNetlib, const CMStringA &url, const CMStringW &filename)
{
	NETLIBHTTPREQUEST req = { sizeof(req) };
	req.requestType = REQUEST_GET;
	req.flags = NLHRF_HTTP11 | NLHRF_REDIRECT;
	req.szUrl = const_cast<char*>(url.c_str());

	NLHR_PTR resp(Netlib_HttpTransaction(hNetlib, &req));
	if (!resp)
		return false;
	
	if (resp->resultCode != 200)
		return false;
	
	// Create folder if necessary
	if (CreatePathToFileW(filename) != ERROR_SUCCESS)
		return false;

	// Write to file
	if (FILE *f = _wfopen(filename, L"wb")) {
		fwrite(resp->pData, 1, resp->dataLength, f);
		fclose(f);
	}
	else return false;
	
	return true;
}
