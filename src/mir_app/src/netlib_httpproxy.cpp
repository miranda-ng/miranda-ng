/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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
#include "netlib.h"

typedef enum
{
	reqHelloGet,
	reqOldGet,
	reqOldPost,
	reqNewPost,
}
RequestType;

/////////////////////////////////////////////////////////////////////////////////////////

#define NETLIBHTTP_RETRYCOUNT   3
#define NETLIBHTTP_RETRYTIMEOUT 2000

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Netlib_SetHttpProxyInfo(HNETLIBCONN nlc, const NETLIBHTTPPROXYINFO *nlhpi)
{
	if (GetNetlibHandleType(nlc) != NLH_CONNECTION || nlhpi == nullptr || nlhpi->szHttpPostUrl == nullptr) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	mir_free(nlc->nlhpi.szHttpGetUrl);
	mir_free(nlc->nlhpi.szHttpPostUrl);

	nlc->nlhpi.combinePackets = 1;
	memcpy(&nlc->nlhpi, nlhpi, sizeof(*nlhpi));
	if (nlc->nlhpi.combinePackets == 0)
		nlc->nlhpi.combinePackets = 1;

	nlc->nlhpi.szHttpGetUrl = mir_strdup(nlc->nlhpi.szHttpGetUrl);
	nlc->nlhpi.szHttpPostUrl = mir_strdup(nlc->nlhpi.szHttpPostUrl);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Netlib_SetStickyHeaders(HNETLIBUSER nlu, const char *szHeaders)
{
	if (GetNetlibHandleType(nlu) != NLH_USER)
		return ERROR_INVALID_PARAMETER;
	
	replaceStr(nlu->szStickyHeaders, szHeaders); // pointer is ours
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Netlib_SetPollingTimeout(HNETLIBCONN nlc, int iTimeout)
{
	if (GetNetlibHandleType(nlc) != NLH_CONNECTION)
		return -1;
	
	int oldTimeout = nlc->pollingTimeout;
	nlc->pollingTimeout = iTimeout;
	return oldTimeout;
}
