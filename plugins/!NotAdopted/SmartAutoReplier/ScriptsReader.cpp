/*
 *  Smart Auto Replier (SAR) - auto replier plugin for Miranda IM
 *
 *  Copyright (C) 2004 - 2012 by Volodymyr M. Shcherbyna <volodymyr@shcherbyna.com>
 *
 *      This file is part of SAR.
 *
 *  SAR is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SAR is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SAR.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "scriptsreader.h"
#include "messageshandler.h"
#include "modemesshandler.h"

extern INT g_nCurrentMode;
extern CMessagesHandler * g_pMessHandler;

/// ctor
CScriptsReader::CScriptsReader(void)
{	
}

/// dtor
CScriptsReader::~CScriptsReader(void)
{
}

/// dtor
void CScriptsReader::InternalInit(void)
{
}

ACTIONS CScriptsReader::ParceString(LPTSTR & strMessage)
{
	ACTIONS act;
	return act;
}

bool CScriptsReader::Replace(LPSTR & lpString, LPSTR strReplWhat, LPSTR & strReplWith)
{
BEGIN_PROTECT_AND_LOG_CODE
	if (strcmp(lpString, strReplWhat) == 0)
	{
		strcpy(lpString, strReplWith);
		return true;
	}
	char * ptr = strstr(lpString, strReplWhat);
	if (ptr)
	{
	    int nPos = static_cast<int>(ptr - lpString + 1);
		if (!nPos)
			return false;
		char *szPrev = new char[nPos];
		
		if (!szPrev)
			return false;
		strncpy(szPrev, lpString, nPos - 1);
		int nPos2 = static_cast<int>(nPos + strlen(strReplWhat));
		int nLength = static_cast<int>(strlen(lpString)); 
		int nSize2 =  nLength - nPos2 + 1;		
		if (nSize2 == 0)
			nSize2++;
		char *szPost = new char[nSize2];
		if (!szPost)
		{
			delete szPrev;
			return false;
		}
		ptr += strlen(strReplWhat);
		if (nPos == 1)
		{			
			ptr++;
		}
		strncpy(szPost, ptr, nSize2);
		strcpy(lpString, szPrev);
		strcat(lpString, strReplWith);
		if (nPos == 1)
			strcat(lpString, " ");
		strcat(lpString, szPost);
		delete szPrev;
		delete szPost;

		return Replace(lpString, strReplWhat, strReplWith);
	}
	else
		return false;
END_PROTECT_AND_LOG_CODE
	return false;
}

/// returnes message that is replied to contact.
/// the core of a plugin ;) (a joke..)
bool CScriptsReader::GetReturnMessage(LPTSTR lpContactName, LPTSTR & lpMsg, LPTSTR & lpIncomingMsg)
{
BEGIN_PROTECT_AND_LOG_CODE
	DWORD dwSizeOfMess = SETTINGS_MESSAGE_MAXVALENGTH + SETTINGS_HEADER_MAXVALENGTH;
	dwSizeOfMess += _tcslen(lpIncomingMsg);
	LPTSTR szOwnNick = g_pMessHandler->GetContactName(NULL);
	if (szOwnNick)
		dwSizeOfMess += _tcslen(szOwnNick);

	bool bspecific = (lpMsg != NULL);
	dwSizeOfMess = dwSizeOfMess * 3;	/// Any replacing can cause increasing size of data
	LPTSTR lpPrev = NULL;
	if (bspecific)
	{
		dwSizeOfMess += _tcslen(lpMsg);
		lpPrev = lpMsg;
	}

	lpMsg = new TCHAR[dwSizeOfMess];

	memset(lpMsg, 0, dwSizeOfMess * sizeof(TCHAR));
	
	COMMON_RULE_ITEM & commRule = g_pMessHandler->getSettings().getStorage().getCommonRule();

	if (!bspecific)
	{
		_tcscpy(lpMsg, commRule.Message);
	}
	else
	{
		_tcscpy(lpMsg, lpPrev);
		VirtualFree (lpPrev, NULL, MEM_RELEASE);
	}

	/*TCHAR strDate[0x100] = {0};
	SYSTEMTIME t = {0};
	GetLocalTime(&t);
	sprintf(strDate, "[%d-%d-%d %d-%d-%d]", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
	LPSTR lp = strDate;
	if (!bspecific)
	{
		_tcscat(lpMsg, "\r\n");
		_tcscat(lpMsg, commRule.Message);
	}

	Replace(lpMsg, SETTINGS_SCRIPT_CURRDATA, lp);
	Replace(lpMsg, SETTINGS_SCRIPT_USERNAME, lpContactName);
	Replace(lpMsg, SETTINGS_SCRIPT_OWNNICKNAME, szOwnNick);

	LPTSTR lptMMess = CModeMessHandler::GetModeMessage(g_nCurrentMode);
		
	if (lptMMess)
	{
		Replace(lpMsg, SETTINGS_SCRIPT_MODEMESSAGE, lptMMess);
		/// seems like i should free this ptr
		/// but when this is done heap is corrupted
		/// so better we have a leak of small amount 
		/// of memory then corrupted heap. (imho).Sad
		/// free (lptMMess);
	}

	Replace(lpMsg, SETTINGS_SCRIPT_INCOMMINGMESSAGE, lpIncomingMsg);*/
	
	if (lpMsg)
		return true;
	else
		return false;

END_PROTECT_AND_LOG_CODE
	return false;
}