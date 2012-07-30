// Copyright © 2008 sss, chaos.persei
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


#include "commonheaders.h"

/*HANDLE CreateThreadEx(pThreadFuncEx AFunc, void* arg, DWORD* pThreadID)
{
  FORK_THREADEX_PARAMS params;
  DWORD dwThreadId;
  HANDLE hThread;

  params.pFunc      = AFunc;
  params.arg        = arg;
  params.iStackSize = 0;
  params.threadID   = &dwThreadId;
  hThread = (HANDLE)CallService(MS_SYSTEM_FORK_THREAD_EX, 0, (LPARAM)&params);
  if (pThreadID)
    *pThreadID = dwThreadId;

  return hThread;
}*/

TCHAR* __stdcall UniGetContactSettingUtf(HANDLE hContact, const char *szModule,const char* szSetting, TCHAR* szDef)
{
  DBVARIANT dbv = {DBVT_DELETED};
  TCHAR* szRes;
  if (DBGetContactSettingTString(hContact, szModule, szSetting, &dbv))
	 return _tcsdup(szDef);
  if(dbv.pszVal)
	  szRes = _tcsdup(dbv.ptszVal);
  DBFreeVariant(&dbv);
  return szRes;
}

// case-insensitive _tcsstr
/*#define NEWTSTR_ALLOCA(A) (A==NULL)?NULL:_tcscpy((TCHAR*)alloca(sizeof(TCHAR)*(_tcslen(A)+1)),A)
const TCHAR *stristr( const TCHAR *str, const TCHAR *substr)
{
	TCHAR *p;
	TCHAR *str_up = NEWTSTR_ALLOCA(str);
	TCHAR *substr_up = NEWTSTR_ALLOCA(substr);

	CharUpperBuff(str_up, lstrlen(str_up));
	CharUpperBuff(substr_up, lstrlen(substr_up));

	p = _tcsstr(str_up, substr_up);
	return p ? (str + (p - str_up)) : NULL;
}*/

char *date()
{
	static char d[11];
	char *tmp = __DATE__, m[4], mn[3] = "01";
	m[0]=tmp[0];
	m[1]=tmp[1];
	m[2]=tmp[2];
	if(strstr(m,"Jan"))
		strcpy(mn,"01");
	else if(strstr(m,"Feb"))
		strcpy(mn,"02");
	else if(strstr(m,"Mar"))
		strcpy(mn,"03");
	else if(strstr(m,"Apr"))
		strcpy(mn,"04");
	else if(strstr(m,"May"))
		strcpy(mn,"05");
	else if(strstr(m,"Jun"))
		strcpy(mn,"06");
	else if(strstr(m,"Jul"))
		strcpy(mn,"07");
	else if(strstr(m,"Aug"))
		strcpy(mn,"08");
	else if(strstr(m,"Sep"))
		strcpy(mn,"09");
	else if(strstr(m,"Oct"))
		strcpy(mn,"10");
	else if(strstr(m,"Nov"))
		strcpy(mn,"11");
	else if(strstr(m,"Dec"))
		strcpy(mn,"12");
	d[0]=tmp[7];
	d[1]=tmp[8];
	d[2]=tmp[9];
	d[3]=tmp[10];
	d[4]='.';
	d[5]=mn[0];
	d[6]=mn[1];
	d[7]='.';
	if (tmp[4] == ' ')
		d[8] = '0';
	else
		d[8]=tmp[4];
	d[9]=tmp[5];
	return d;
}
HANDLE NetLib_CreateConnection(HANDLE hUser, NETLIBOPENCONNECTION* nloc) //from icq )
{
	HANDLE hConnection;

	nloc->cbSize = sizeof(NETLIBOPENCONNECTION);
	nloc->flags |= NLOCF_V2;

	hConnection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hUser, (LPARAM)nloc);
	if (!hConnection && (GetLastError() == 87))
	{ // this ensures, an old Miranda will be able to connect also
		nloc->cbSize = NETLIBOPENCONNECTION_V1_SIZE;
		hConnection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hConnection, (LPARAM)nloc);
	}
	return hConnection;	
}


