/*
	NewXstatusNotify YM - Plugin for Miranda IM
	Copyright (c) 2001-2004 Luca Santarelli
	Copyright (c) 2005-2007 Vasilich
	Copyright (c) 2007-2011 yaho

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "common.h"
#include "options.h"

extern OPTIONS opt;
extern STATUS StatusList[STATUS_COUNT];

TCHAR *db2t(DBVARIANT *dbv)
{
	TCHAR *buff;
	switch (dbv->type)
	{
		case DBVT_ASCIIZ: buff = mir_a2t(dbv->pszVal); break;
		case DBVT_WCHAR: buff = mir_tstrdup(dbv->ptszVal); break;
		case DBVT_UTF8: buff = mir_utf8decodeT(dbv->pszVal); break;
		default: buff = mir_tstrdup(_T(""));
	}

	if (buff[0] == 0)
	{
		mir_free(buff);
		buff = NULL;
	}
		
	return buff;
}

int DBGetStringDefault(HANDLE hContact, const char *szModule, const char *szSetting, TCHAR *setting, int size, const TCHAR *defaultValue)
{
	DBVARIANT dbv;
	if(!DBGetContactSettingTString(hContact, szModule, szSetting, &dbv))
	{
		_tcsncpy(setting, dbv.ptszVal, size);
		DBFreeVariant(&dbv);
		return 0;
	} 

	_tcsncpy(setting, defaultValue, size);
	return 1;
}

void HigherLower(int maxValue, int minValue) 
{
	TCHAR str[64] = { 0 };
	wsprintf(str, TranslateT("You cannot specify a value lower than %d and higher than %d."), minValue, maxValue);
}

void ShowLog(TCHAR *file)
{
	int res = (int)ShellExecute(NULL, _T("open"), file, NULL, NULL, SW_SHOW);
	if (res <= 32) // error
		MessageBox(0, TranslateT("Can't open the log file!"), TranslateT("NewXstatusNotify"), MB_OK | MB_ICONERROR);
}

int ProtoServiceExists(const char *szModule, const char *szService)
{
	char str[MAXMODULELABELLENGTH];
	if (szModule == NULL || szService == NULL) 
		return 0;

	strcpy(str,szModule);
	strcat(str,szService);
	return ServiceExists(str);
}

BOOL StatusHasAwayMessage(char *szProto, int status) 
{
	if (szProto != NULL)
	{
		unsigned long iSupportsSM = (unsigned long)CallProtoService(szProto, PS_GETCAPS, (WPARAM)PFLAGNUM_3, (LPARAM)0);
		return (iSupportsSM & Proto_Status2Flag(status)) ? TRUE : FALSE;
	}
	else
	{
		switch (status)
		{
			case ID_STATUS_AWAY:
			case ID_STATUS_NA:
			case ID_STATUS_OCCUPIED:
			case ID_STATUS_DND:
			case ID_STATUS_FREECHAT:
			case ID_STATUS_ONTHEPHONE:
			case ID_STATUS_OUTTOLUNCH:
				return TRUE;
			default:
				return FALSE;
		}
	}
}

void LogToFile(TCHAR *stzText)
{	
	FILE *fp = _tfopen(opt.LogFilePath, _T("a+b, ccs=UTF-8"));
	if (fp)
	{ 
		char *encodedText = mir_utf8encodeT(stzText);
		if (encodedText)
		{
			fprintf(fp, encodedText);
			mir_free(encodedText);
		}
		fclose(fp);
	}
}