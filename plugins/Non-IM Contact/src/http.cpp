/*
Weather Protocol plugin for Miranda IM
Copyright (C) 2002-2004 Calvin Che

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

/* This file contain the source related to downloading weather info
   from the web using netlib
*/

#include "commonheaders.h"

char *szInfo;
char *szData;
HANDLE hNetlibUser;

// function to download webpage from the internet
// szUrl = URL of the webpage to be retrieved
// return value = 0 for success, 1 or HTTP error code for failure
// global var used: szData, szInfo = containing the retrieved data
int InternetDownloadFile (char *szUrl) 
{
	NETLIBHTTPREQUEST nlhr={0};

	// initialize the netlib request
	nlhr.cbSize=sizeof(nlhr);
	nlhr.requestType=REQUEST_GET;
	nlhr.flags=NLHRF_DUMPASTEXT;
	nlhr.szUrl= szUrl;
	// change the header so the plugin is pretended to be IE 6 + WinXP
	nlhr.headersCount++;
	nlhr.headers=(NETLIBHTTPHEADER*)malloc(sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
	memcpy(nlhr.headers,nlhr.headers,sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
	nlhr.headers[nlhr.headersCount-1].szName="User-Agent";
	nlhr.headers[nlhr.headersCount-1].szValue="Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)";

	// download the page
	NETLIBHTTPREQUEST *nlhrReply=(NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION,(WPARAM)hNetlibUser,(LPARAM)&nlhr);
	if (nlhrReply) {
		// return error code if the recieved code is neither 200 OK or 302 Moved
		if (nlhrReply->resultCode != 200 && nlhrReply->resultCode != 302)
			return nlhrReply->resultCode;
		// if the recieved code is 200 OK
		else if (nlhrReply->resultCode == 200) 
		{
			// allocate memory and save the retrieved data
			szData = (char *)malloc(mir_strlen(nlhrReply->pData)+2);
			mir_strncpy(szData, nlhrReply->pData, mir_strlen(nlhrReply->pData));
		}
		// if the recieved code is 302 Moved, Found, etc
		else if (nlhrReply->resultCode == 302) 
		{	// page moved
			int i;
			// get the url for the new location and save it to szInfo
			// look for the reply header "Location"
			for (i=0; i<nlhrReply->headersCount; i++) {
				if (!mir_strcmp(nlhrReply->headers[i].szName, "Location")) {
					szData = (char *)malloc(512);
					// add "Moved/Location:" in front of the new URL for identification
					mir_snprintf(szData, 512, "Moved/Location: %s\n", nlhrReply->headers[i].szValue);
					break;
				}
			}
			// log the new url into netlib log
			CallService(MS_NETLIB_LOG,(WPARAM)hNetlibUser,(LPARAM)szData);
		}
	}
	// if the data does not downloaded successfully (ie. disconnected), then return 1 as error code
	else   return 1;

	// make a copy of the retrieved data, then free the memory of the http reply
	szInfo = szData;
	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)nlhrReply);

	// the recieved data is empty, data was not recieved, so return an error code of 1
	if (!mir_strcmp(szInfo, ""))  return 1;
	return 0;
}

//============  NETLIB INITIALIZATION  ============

void NetlibInit() {
	NETLIBUSER nlu={0};
	nlu.cbSize=sizeof(nlu);
	nlu.flags=NUF_OUTGOING|NUF_HTTPCONNS|NUF_NOHTTPSOPTION|NUF_TCHAR;
	nlu.szSettingsModule= MODNAME;
	nlu.ptszDescriptiveName= TranslateT("Non-IM Contacts");
	hNetlibUser=(HANDLE)CallService(MS_NETLIB_REGISTERUSER,0,(LPARAM)&nlu);
}
