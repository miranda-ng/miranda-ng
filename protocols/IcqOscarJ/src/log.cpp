// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Joe Kucera
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------

#include "icqoscar.h"

extern BOOL bPopupService;

static const char *szLevelDescr[] = {LPGEN("ICQ Note"), LPGEN("ICQ Warning"), LPGEN("ICQ Error"), LPGEN("ICQ Fatal")};

struct LogMessageInfo {
	const char *szMsg;
	const char *szTitle;
  BYTE bLevel;
};


void __cdecl CIcqProto::icq_LogMessageThread(void* arg) 
{
	LogMessageInfo *err = (LogMessageInfo*)arg;
	if (!err)
		return;

	if (bPopupService && getByte("PopupsLogEnabled", DEFAULT_LOG_POPUPS_ENABLED))
	{
		ShowPopupMsg(NULL, err->szTitle, err->szMsg, err->bLevel); 

		SAFE_FREE((void**)&err->szMsg);
		SAFE_FREE((void**)&err);

		return;
	}

	bErrorBoxVisible = TRUE;
	if (err->szMsg && err->szTitle)
		MessageBoxUtf(NULL, err->szMsg, err->szTitle, MB_OK);
	SAFE_FREE((void**)&err->szMsg);
	SAFE_FREE((void**)&err);
	bErrorBoxVisible = FALSE;
}


void CIcqProto::icq_LogMessage(int level, const char *szMsg)
{
	debugLogA("%s", szMsg);

	int displayLevel = getByte("ShowLogLevel", LOG_WARNING);
	if (level >= displayLevel)
	{
		if (!bErrorBoxVisible || !getByte("IgnoreMultiErrorBox", 0))
		{ 
			// error not shown or allowed multi - show messagebox
			LogMessageInfo *lmi = (LogMessageInfo*)SAFE_MALLOC(sizeof(LogMessageInfo));
			lmi->bLevel = (BYTE)level;
			lmi->szMsg = null_strdup(szMsg);
			lmi->szTitle = szLevelDescr[level];
			ForkThread( &CIcqProto::icq_LogMessageThread, lmi);
		}
	}
}

void CIcqProto::icq_LogUsingErrorCode(int level, DWORD dwError, const char *szMsg)
{
	char szBuf[1024];
	char str[1024];
	char str2[64];
	char szErrorMsg[512];
	char *pszErrorMsg = NULL;
	int bNeedFree = FALSE;

	switch(dwError) {
	case ERROR_TIMEOUT:
	case WSAETIMEDOUT:
		pszErrorMsg = LPGEN("The server did not respond to the connection attempt within a reasonable time, it may be temporarily down. Try again later.");
		break;

	case ERROR_GEN_FAILURE:
		pszErrorMsg = LPGEN("The connection with the server was abortively closed during the connection attempt. You may have lost your local network connection.");
		break;

	case WSAEHOSTUNREACH:
	case WSAENETUNREACH:
		pszErrorMsg = LPGEN("Miranda was unable to resolve the name of a server to its numeric address. This is most likely caused by a catastrophic loss of your network connection (for example, your modem has disconnected), but if you are behind a proxy, you may need to use the 'Resolve hostnames through proxy' option in M->Options->Network.");
		break;

	case WSAEHOSTDOWN:
	case WSAENETDOWN:
	case WSAECONNREFUSED:
		pszErrorMsg = LPGEN("Miranda was unable to make a connection with a server. It is likely that the server is down, in which case you should wait for a while and try again later.");
		break;

	case ERROR_ACCESS_DENIED:
		pszErrorMsg = LPGEN("Your proxy rejected the user name and password that you provided. Please check them in M->Options->Network.");
		break;

	case WSAHOST_NOT_FOUND:
	case WSANO_DATA:
		pszErrorMsg = LPGEN("The server to which you are trying to connect does not exist. Check your spelling in M->Options->Network->ICQ.");
		break;

	default:
		{
			TCHAR err[512];

			if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, err, SIZEOF(err), NULL))
			{

				pszErrorMsg = make_utf8_string(err);

				bNeedFree = TRUE;
			}
			break;
		}
	}

	mir_snprintf(szBuf, sizeof(szBuf), "%s%s%s (%s %d)", 
		szMsg ? ICQTranslateUtfStatic(szMsg, str, 1024) : "", 
		szMsg ? "\r\n\r\n" : "",
		ICQTranslateUtfStatic(pszErrorMsg, szErrorMsg, 512), 
		ICQTranslateUtfStatic(LPGEN("error"), str2, 64),
		dwError);

	if (bNeedFree)
		SAFE_FREE(&pszErrorMsg);

	icq_LogMessage(level, szBuf);
}

void CIcqProto::icq_LogFatalParam(const char *szMsg, WORD wError)
{
	char str[MAX_PATH];
	char buf[MAX_PATH];

	mir_snprintf(buf, MAX_PATH, ICQTranslateUtfStatic(szMsg, str, MAX_PATH), wError);
	icq_LogMessage(LOG_FATAL, buf);
}
