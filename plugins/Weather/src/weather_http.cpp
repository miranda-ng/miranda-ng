/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG Team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
This file contain the source related to downloading weather info
from the web using netlib
*/

#include "weather.h"

HANDLE hNetlibUser, hNetlibHttp;

int findHeader(NETLIBHTTPREQUEST *nlhrReply, char *hdr)
{
	int res = -1, i; 
	for (i=0; i<nlhrReply->headersCount; i++) 
	{
		if (_stricmp(nlhrReply->headers[i].szName, hdr) == 0) 
		{
			res = i;
			break;
		}
	}
	return res;
}

//============  DOWNLOAD NEW WEATHER  ============

// function to download webpage from the internet
// szUrl = URL of the webpage to be retrieved
// return value = 0 for success, 1 or HTTP error code for failure
// global var used: szData, szInfo = containing the retrieved data

int InternetDownloadFile (char *szUrl, char *cookie, char *userAgent, TCHAR **szData) 
{
	if (userAgent == NULL || userAgent[0] == 0)
		userAgent = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)";
	
	NETLIBHTTPHEADER headers[5];
	headers[0].szName = "User-Agent";
	headers[0].szValue = userAgent;
	headers[1].szName = "Cache-Control";
	headers[1].szValue = "no-cache";
	headers[2].szName = "Pragma";
	headers[2].szValue = "no-cache";
	headers[3].szName = "Connection";
	headers[3].szValue = "close";
	headers[4].szName = "Cookie";
	headers[4].szValue = cookie;

	// initialize the netlib request
	NETLIBHTTPREQUEST nlhr = { sizeof(nlhr) };
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	nlhr.szUrl = szUrl;
	nlhr.nlc = hNetlibHttp;
	nlhr.headers = headers;
	nlhr.headersCount = SIZEOF(headers);

	if (cookie == NULL || cookie[0] == 0)
		--nlhr.headersCount;

	// download the page
	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser, (LPARAM)&nlhr);
	if (nlhrReply == 0) {
	// if the data does not downloaded successfully (ie. disconnected), then return 1000 as error code
		*szData = (TCHAR*)mir_alloc(512);
		// store the error code in szData
		_tcscpy(*szData, _T("NetLib error occurred!!"));
		hNetlibHttp = NULL;
		return NLHRF_REDIRECT;
	}

	// if the recieved code is 200 OK
	int result;
	if (nlhrReply->resultCode == 200) {
		if (nlhrReply->dataLength) {
			bool bIsUtf = false;
			result = 0;

			// allocate memory and save the retrieved data
			int i = findHeader(nlhrReply, "Content-Type");
			if (i != -1 && strstr(_strlwr((char*)nlhrReply->headers[i].szValue), "utf-8"))
				bIsUtf = true;
			else {
				char* end = nlhrReply->pData;
				for (;;) {
					char* beg = strstr(end, "<meta");
					if (beg == NULL) break;
					else {
						char* method, tmp;
						end = strchr(beg, '>');
						tmp = *end; *end = 0;

						method = strstr(beg, "http-equiv=\"");
						if (method && _strnicmp(method+12, "Content-Type", 12) == 0 && strstr(method, "utf-8")) {
							bIsUtf = true;
							break;
						}
						else *end = tmp;
					}
				}
			}

			TCHAR *retVal = NULL;
			if (bIsUtf)
				retVal = mir_utf8decodeT( nlhrReply->pData );
			if (retVal == NULL)
				retVal = mir_a2t(nlhrReply->pData);
			*szData = retVal;
		}
		else result = DATA_EMPTY;
	}
	// return error code if the recieved code is neither 200 OK nor 302 Moved
	else {
		*szData = (TCHAR*)mir_alloc(512);
		// store the error code in szData
		mir_sntprintf(*szData, 512, _T("Error occured! HTTP Error: %i\n"), nlhrReply->resultCode);
		result = (int)nlhrReply->resultCode;
	}

	hNetlibHttp = nlhrReply->nlc;
	// make a copy of the retrieved data, then free the memory of the http reply
	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0, (LPARAM)nlhrReply);
	return result;
}

//============  NETLIB INITIALIZATION  ============

// initialize netlib support for weather protocol
void NetlibInit(void) 
{
	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING|NUF_HTTPCONNS|NUF_NOHTTPSOPTION|NUF_TCHAR;
	nlu.szSettingsModule = WEATHERPROTONAME;
	nlu.ptszDescriptiveName = TranslateT("Weather HTTP connections");
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER,0, (LPARAM)&nlu);
}

void NetlibHttpDisconnect(void) 
{
	if (hNetlibHttp)
	{
		HANDLE hConn = hNetlibHttp;
		hNetlibHttp = NULL;
		Netlib_CloseHandle(hConn);
	}
}

