/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"

char* CMsnProto::getSslResult(char** parUrl, const char* parAuthInfo, const char* hdrs, unsigned& status)
{
	mHttpsTS = clock();

	char* result = NULL;
	NETLIBHTTPREQUEST nlhr = { 0 };

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_POST;
	nlhr.flags = NLHRF_HTTP11 | NLHRF_DUMPASTEXT | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	nlhr.szUrl = *parUrl;
	nlhr.dataLength = (int)mir_strlen(parAuthInfo);
	nlhr.pData = (char*)parAuthInfo;
	nlhr.nlc = hHttpsConnection;

#ifndef _DEBUG
	if (strstr(*parUrl, "login")) nlhr.flags |= NLHRF_NODUMPSEND;
#endif

	nlhr.headersCount = 4;
	nlhr.headers = (NETLIBHTTPHEADER*)alloca(sizeof(NETLIBHTTPHEADER) * (nlhr.headersCount + 5));
	nlhr.headers[0].szName = "User-Agent";
	nlhr.headers[0].szValue = (char*)MSN_USER_AGENT;
	nlhr.headers[1].szName = "Accept";
	nlhr.headers[1].szValue = "text/*";
	nlhr.headers[2].szName = "Content-Type";
	nlhr.headers[2].szValue = "text/xml; charset=utf-8";
	nlhr.headers[3].szName = "Cache-Control";
	nlhr.headers[3].szValue = "no-cache";

	if (hdrs) {
		unsigned count = 0;
		char* hdrprs = NEWSTR_ALLOCA(hdrs);
		for (;;) {
			char* fnd = strchr(hdrprs, ':');
			if (fnd == NULL) break;
			*fnd = 0;
			fnd += 2;

			nlhr.headers[nlhr.headersCount].szName = hdrprs;
			nlhr.headers[nlhr.headersCount].szValue = fnd;

			fnd = strchr(fnd, '\r');
			*fnd = 0;
			hdrprs = fnd + 2;
			++nlhr.headersCount;
			if (++count >= 5) break;
		}
	}

	// download the page
	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION,
		(WPARAM)hNetlibUserHttps, (LPARAM)&nlhr);

	if (nlhrReply) {
		hHttpsConnection = nlhrReply->nlc;
		status = nlhrReply->resultCode;

		if (nlhrReply->szUrl) {
			mir_free(*parUrl);
			*parUrl = nlhrReply->szUrl;
			nlhrReply->szUrl = NULL;
		}

		result = nlhrReply->pData;

		nlhrReply->dataLength = 0;
		nlhrReply->pData = NULL;

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
	}
	else
		hHttpsConnection = NULL;

	mHttpsTS = clock();

	return result;
}

bool CMsnProto::getMyAvatarFile(char *url, TCHAR *fname)
{
	NETLIBHTTPREQUEST nlhr = { 0 };
	bool result = true;

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_HTTP11 | NLHRF_REDIRECT;
	nlhr.szUrl = url;

	nlhr.headersCount = 1;
	nlhr.headers = (NETLIBHTTPHEADER*)alloca(sizeof(NETLIBHTTPHEADER) * nlhr.headersCount);
	nlhr.headers[0].szName = "User-Agent";
	nlhr.headers[0].szValue = (char*)MSN_USER_AGENT;

	// download the page
	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION,
		(WPARAM)hNetlibUserHttps, (LPARAM)&nlhr);

	if (nlhrReply) {
		if (nlhrReply->resultCode == 200 && nlhrReply->dataLength)
			MSN_SetMyAvatar(fname, nlhrReply->pData, nlhrReply->dataLength);
		else
			result = false;

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
	}
	return result;
}
