/*
Weather Protocol plugin for Miranda IM
Copyright (C) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2002-2005 Calvin Che

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
int InternetDownloadFile (char *szUrl, char* cookie, char** szData) 
{
	int result = 0xBADBAD;
	char* szRedirUrl  = NULL;
	NETLIBHTTPREQUEST nlhr = {0}, *nlhrReply;
	NETLIBHTTPHEADER headers[6];

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
	nlhr.szUrl = szUrl;
	nlhr.nlc = hNetlibHttp;

	if (CallService(MS_SYSTEM_GETVERSION, 0, 0) >= PLUGIN_MAKE_VERSION(0,9,0,5))
		nlhr.flags |= NLHRF_PERSISTENT | NLHRF_REDIRECT;

	// change the header so the plugin is pretended to be IE 6 + WinXP
	nlhr.headersCount = 5;
	nlhr.headers = headers;
	nlhr.headers[0].szName  = "User-Agent";
	nlhr.headers[0].szValue = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)";
	nlhr.headers[1].szName  = "Cache-Control";
	nlhr.headers[1].szValue = "no-cache";
	nlhr.headers[2].szName  = "Pragma";
	nlhr.headers[2].szValue = "no-cache";
	nlhr.headers[3].szName  = "Connection";
	nlhr.headers[3].szValue = "close";
	nlhr.headers[4].szName  = "Cookie";
	nlhr.headers[4].szValue = cookie;
//	nlhr.headers[5].szName  = "If-Modified-Since";
//	nlhr.headers[5].szValue = "Tue, 24 Feb 2009 03:44:23 GMT";

	if (cookie == NULL || cookie[0] == 0) --nlhr.headersCount;

	while (result == 0xBADBAD)
	{
		// download the page
		nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION,
			(WPARAM)hNetlibUser,(LPARAM)&nlhr);

		if (nlhrReply) 
		{
			// if the recieved code is 200 OK
			if(nlhrReply->resultCode == 200) 
			{
				if (nlhrReply->dataLength)
				{
					char* end;
					int i;

					result = 0;

//				    i = findHeader(nlhrReply, "Date-Modified");

					// allocate memory and save the retrieved data
					*szData = (char *)mir_alloc(nlhrReply->dataLength + 2);

					memcpy(*szData, nlhrReply->pData, nlhrReply->dataLength);
					(*szData)[nlhrReply->dataLength] = 0;

					i = findHeader(nlhrReply, "Content-Type");
					if (i != -1)
					{
						if (strstr(_strlwr((char*)nlhrReply->headers[i].szValue), "utf-8"))
							mir_utf8decode(*szData, NULL);
					}

					end = *szData;
					for (;;)
					{
						char* beg = strstr(end, "<meta");
						if (beg == NULL) break;
						else
						{
							char* method, tmp;
							end = strchr(beg, '>');
							tmp = *end; *end  = 0;

							method  = strstr(beg, "http-equiv=\"");
							if (method && _strnicmp(method+12, "Content-Type", 12) == 0 && strstr(method, "utf-8"))
							{
								*end = tmp;
								mir_utf8decode(*szData, NULL);
								break;
							}
							else
								*end = tmp;
						}
					} 
				}
				else
					result = DATA_EMPTY;
			}
			// if the recieved code is 302 Moved, Found, etc
			// workaround for url forwarding
			else if(nlhrReply->resultCode == 302 || nlhrReply->resultCode == 301 || nlhrReply->resultCode == 303 ) // page moved
			{
				// get the url for the new location and save it to szInfo
				// look for the reply header "Location"
				int i = findHeader(nlhrReply, "Location");

				if (i != -1)
				{
					size_t rlen = 0;
					if (nlhrReply->headers[i].szValue[0] == '/')
					{
						char* szPath;
						char* szPref = strstr(szUrl, "://");
						szPref = szPref ? szPref + 3 : szUrl;
						szPath = strchr(szPref, '/');
						rlen = szPath != NULL ? szPath - szUrl : strlen(szUrl); 
					}

					szRedirUrl = (char*)mir_realloc(szRedirUrl, 
						rlen + strlen(nlhrReply->headers[i].szValue)*3 + 1);

					strncpy(szRedirUrl, szUrl, rlen);
					strcpy(szRedirUrl+rlen, nlhrReply->headers[i].szValue); 

					GetSearchStr(szRedirUrl);

					nlhr.szUrl = szRedirUrl;
				}
			}
			// return error code if the recieved code is neither 200 OK nor 302 Moved
			else 
			{
				*szData = (char *)mir_alloc(512);
				// store the error code in szData
				wsprintf(*szData, "Error occured! HTTP Error: %i\n", nlhrReply->resultCode);
				result = (int)nlhrReply->resultCode;
			}

			hNetlibHttp = nlhrReply->nlc;
			// make a copy of the retrieved data, then free the memory of the http reply
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)nlhrReply);
		}
		// if the data does not downloaded successfully (ie. disconnected), then return 1000 as error code
		else 
		{
			*szData = (char *)mir_alloc(512);
			// store the error code in szData
			strcpy(*szData, "NetLib error occurred!!");
			result = NETLIB_ERROR;
			hNetlibHttp = NULL;
		}

	}

	mir_free(szRedirUrl);

	return result;
}

//============  NETLIB INITIALIZATION  ============

// initialize netlib support for weather protocol
void NetlibInit(void) 
{
	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING|NUF_HTTPCONNS|NUF_NOHTTPSOPTION;
	nlu.szSettingsModule = WEATHERPROTONAME;
	nlu.szDescriptiveName = Translate("Weather HTTP connections");
	hNetlibUser=(HANDLE)CallService(MS_NETLIB_REGISTERUSER,0,(LPARAM)&nlu);
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

