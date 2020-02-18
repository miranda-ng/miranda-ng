/*
Copyright © 2012-20 Miranda NG team
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

http::response CTwitterProto::Execute(AsyncHttpRequest *req)
{
	if (!req->m_szParam.IsEmpty()) {
		if (req->requestType == REQUEST_POST) {
			req->AddHeader("Content-Type", "application/x-www-form-urlencoded");
			req->AddHeader("Cache-Control", "no-cache");

			req->dataLength = (int)req->m_szParam.GetLength();
			req->pData = req->m_szParam.GetBuffer();
		}
		else {
			req->m_szUrl.AppendChar('?');
			req->m_szUrl += req->m_szParam;
		}
	}

	CMStringA auth;
	if (req->requestType == REQUEST_GET)
		auth = OAuthWebRequestSubmit(req->m_szUrl, "GET", "");
	else
		auth = OAuthWebRequestSubmit(req->m_szUrl, "POST", req->m_szParam);
	req->AddHeader("Authorization", auth);

	req->szUrl = req->m_szUrl.GetBuffer();
	req->flags = NLHRF_HTTP11 | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	req->nlc = m_hConnHttp;
	http::response resp_data;
	NLHR_PTR resp(Netlib_HttpTransaction(m_hNetlibUser, req));
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

	return resp_data;
}

bool save_url(HNETLIBUSER hNetlib, const CMStringA &url, const CMStringW &filename)
{
	NETLIBHTTPREQUEST req = { sizeof(req) };
	req.requestType = REQUEST_GET;
	req.flags = NLHRF_HTTP11 | NLHRF_REDIRECT;
	req.szUrl = const_cast<char*>(url.c_str());

	NLHR_PTR resp(Netlib_HttpTransaction(hNetlib, &req));
	if (resp) {
		bool success = (resp->resultCode == 200);
		if (success) {
			// Create folder if necessary
			CreatePathToFileW(filename);

			// Write to file
			FILE *f = _wfopen(filename, L"wb");
			fwrite(resp->pData, 1, resp->dataLength, f);
			fclose(f);
		}
		return success;
	}

	return false;
}
