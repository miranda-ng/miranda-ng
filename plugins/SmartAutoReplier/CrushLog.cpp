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
#include "crushlog.h"
///#include "shlobj.h"

extern HINSTANCE hInst;
extern CCrushLog CRUSHLOGOBJ;

/// init static members
TCHAR CCrushLog::m_szLogPath[MAX_PATH] = {0};
CRITICAL_SECTION CCrushLog::m_critSect;

/// ctor
CCrushLog::CCrushLog(void)
{/// nothing for now
BEGIN_PROTECT_AND_LOG_CODE
END_PROTECT_AND_LOG_CODE
}

/// d'ctor
CCrushLog::~CCrushLog(void)
{
BEGIN_PROTECT_AND_LOG_CODE
END_PROTECT_AND_LOG_CODE
}

/// deinit internal data...
void CCrushLog::DeInit(void)
{
	DeleteCriticalSection(&m_critSect);
}

/// init internal data...
void CCrushLog::Init(void)
{
	/*HRESULT hr	 = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, m_szLogPath);
	size_t  nLen = _tcslen(m_szLogPath);

	if (m_szLogPath[nLen - 1] != TEXT('\\'))
	{
		_tcscat(m_szLogPath, TEXT("\\Sar\\"));
	}
	else
	{
		_tcscat(m_szLogPath, TEXT("Sar\\"));
	}

	CreateDirectory(m_szLogPath, NULL);
	
	_tcscat(m_szLogPath, LOG_FILENAME); */

	// RM TODO: fix path, use folders plugin

	if (_tcslen(m_szLogPath) == 0) /// let's get path to log file
	{
		GetModuleFileName(hInst, m_szLogPath, sizeof(m_szLogPath) * sizeof(TCHAR));
		
		TCHAR *ptrPos = _tcsrchr(m_szLogPath, '\\');
		if (ptrPos)
		{
			ptrPos++;
			_tcscpy(ptrPos, LOG_FILENAME);
		}
		else
		{
			_RPT0(_CRT_WARN, "CCrushLog::CCrushLog - unable to form path");
		}
	}

	InitializeCriticalSection(&m_critSect);
}

/// provides forming data struct and saving it to log
void CCrushLog::InternalProtect(CHAR*	FunctionName, CHAR* FileName, 
								CHAR*	CompilationDate, INT LineNumber,
								INT		LastError)
{
	CRUSHLOG_ITEM item = {0};

	item.CompilationDate = CompilationDate;
	item.FileName = FileName;
	item.FunctionName = FunctionName;
	item.LineNumber = LineNumber;
	item.LastError = LastError;
	g_crushLog.SaveToLog(item);

	TCHAR tch[MAX_PATH * 3] = {0};
	_tcscpy(tch, TEXT("Exception has occured please, analize or just send log file\n\r "));
	_tcscat(tch, CCrushLog::m_szLogPath);
	_tcscat(tch, TEXT("\r\n to developer."));

	::MessageBox (NULL, tch, TEXT("Smart Auto Replier crash. Send the log file to http://shcherbyna.com/forum/viewforum.php?f=8"), MB_OK);
}

/// saving log information
void CCrushLog::SaveToLog(CRUSHLOG_ITEM & item)
{
	EnterCriticalSection(&m_critSect);

	HANDLE hFile = NULL;
	hFile = CreateFile (m_szLogPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{		
		LeaveCriticalSection(&m_critSect);

		return;
	}

	{
		DWORD dwRetVal = 0;
		DWORD dw = GetFileSize(hFile, &dwRetVal);
		SetFilePointer(hFile, dw, NULL, FILE_BEGIN);
	}

	int nSize =  strlen(item.CompilationDate);
		nSize += strlen(item.FileName);
		nSize += strlen(item.FunctionName);
		nSize *= 2;

	CHAR *tsz = new CHAR[nSize];

	if (tsz)
	{
		strcpy(tsz, ("******\n"));
		strcat(tsz, item.CompilationDate);
		strcat(tsz, ("\n"));
		strcat(tsz, item.FileName);
		strcat(tsz, ("\n"));
		strcat(tsz, item.FunctionName);
		strcat(tsz, ("\n"));
		CHAR tchtemp[MAX_PATH] = {0};
		itoa(item.LineNumber, tchtemp, 10);
		strcat(tsz, tchtemp);
		strcat(tsz, ("\n"));
		itoa(item.LastError, tchtemp, 10);
		strcat(tsz, tchtemp);
		strcat(tsz, ("\n"));
		DWORD dw = 0;
		WriteFile (hFile, tsz, strlen(tsz), &dw, NULL);

		delete tsz;
	}

	CloseHandle (hFile);
	LeaveCriticalSection(&m_critSect);
}
