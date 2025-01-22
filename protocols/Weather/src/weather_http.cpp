/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG team
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

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// function to download webpage from the internet
// szUrl = URL of the webpage to be retrieved
// return value = 0 for success, 1 or HTTP error code for failure
// global var used: szData, szInfo = containing the retrieved data

int CWeatherProto::InternetDownloadFile(char *szUrl, char *cookie, char *userAgent, wchar_t **szData)
{
	if (userAgent == nullptr || userAgent[0] == 0)
		userAgent = NETLIB_USER_AGENT;

	// initialize the netlib request
	MHttpRequest nlhr(REQUEST_GET);
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_REDIRECT;
	nlhr.m_szUrl = szUrl;
	nlhr.AddHeader("User-Agent", userAgent);
	nlhr.AddHeader("Cache-Control", "no-cache");
	nlhr.AddHeader("Pragma", "no-cache");
	nlhr.AddHeader("Connection", "close");
	if (mir_strlen(cookie) > 0)
		nlhr.AddHeader("Cookie", cookie);

	// download the page
	NLHR_PTR nlhrReply(Netlib_HttpTransaction(m_hNetlibUser, &nlhr));
	if (nlhrReply == nullptr) {
		// if the data does not downloaded successfully (ie. disconnected), then return 1000 as error code
		*szData = (wchar_t*)mir_alloc(512);
		// store the error code in szData
		mir_wstrcpy(*szData, L"NetLib error occurred!!");
		return NLHRF_REDIRECT;
	}

	// if the recieved code is 200 OK
	int result;
	if (nlhrReply->resultCode == 200) {
		if (!nlhrReply->body.IsEmpty()) {
			bool bIsUtf = false;
			result = 0;

			// allocate memory and save the retrieved data
			auto *pszHdr = nlhrReply->FindHeader("Content-Type");
			// look for Content-Type=utf-8 in header
			if (pszHdr && strstr(_strlwr(pszHdr), "utf-8"))
				bIsUtf = true;
			else {
				char *end = nlhrReply->body.GetBuffer();
				while (end) {
					// look for
					// <meta http-equiv="Content-Type" content="utf-8" />
					char *beg = strstr(end, "<meta");
					if (beg) {
						end = strchr(beg, '>');
						if (end) {
							char tmp = *end;
							*end = 0;

							char *method = strstr(beg, "http-equiv=\"");
							if (method && _strnicmp(method + 12, "Content-Type", 12) == 0 && strstr(method, "utf-8")) {
								bIsUtf = true;
								*end = tmp;
								break;
							}
							else *end = tmp;
						}
					}
					else
						break;
				}
			}

			wchar_t *retVal = nullptr;
			if (bIsUtf)
				retVal = mir_utf8decodeW(nlhrReply->body);
			if (retVal == nullptr)
				retVal = mir_a2u(nlhrReply->body);
			*szData = retVal;
		}
		else result = DATA_EMPTY;
	}
	// return error code if the recieved code is neither 200 OK nor 302 Moved
	else {
		// store the error code in szData
		CMStringW wszError(FORMAT, L"Error occured! HTTP Error: %i\n", nlhrReply->resultCode);
		*szData = wszError.Detach();
		result = nlhrReply->resultCode;
	}

	// make a copy of the retrieved data, then free the memory of the http reply
	return result;
}
