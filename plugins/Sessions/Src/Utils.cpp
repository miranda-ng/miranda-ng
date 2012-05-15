/*
Sessions Management plugin for Miranda IM

Copyright (C) 2007-2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "sessions.h"

void AddSessionMark(HANDLE hContact,int mode,char bit)
{
	unsigned int i;
	char temp_1[1]={'\0'},temp_2[1]={'\0'};
	char szDst[256]={'\0'};
	char* pszBuffer=NULL;
	if(mode==0)
	{	
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsMarks", &dbv) && dbv.pszVal)
		{
			temp_1[0]=dbv.pszVal[0];
			for (i=0;i<ses_limit;i++)		
			{
				temp_2[0]=dbv.pszVal[i+1];
				dbv.pszVal[i+1]=temp_1[0];
				temp_1[0]=temp_2[0];
			}
			for (i=ses_limit;i<10;i++)
				dbv.pszVal[i]='0';
			dbv.pszVal[0]=bit;
			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsMarks", dbv.pszVal);
			DBFreeVariant(&dbv);
		}
		else if(bit=='1') DBWriteContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsMarks", "10000000000");
	}

	if (mode==1)
	{
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsMarks", &dbv) && dbv.pszVal)
		{   
			if(strlen(dbv.pszVal)<g_ses_count)
			{
				pszBuffer = (char*)mir_alloc(g_ses_count+1);
				ZeroMemory(pszBuffer,g_ses_count+1);
				strcpy(pszBuffer,dbv.pszVal);
			}
			else
				pszBuffer = mir_strdup(dbv.pszVal);

			DBFreeVariant(&dbv);

			temp_1[0]=pszBuffer[0];
			for (i=0;i<g_ses_count;i++)		
			{
				temp_2[0]=pszBuffer[i+1];
				pszBuffer[i+1]=temp_1[0];
				temp_1[0]=temp_2[0];
			}
			pszBuffer[0]=bit;
			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsMarks", pszBuffer);
			
			mir_free(pszBuffer);
		}
		else if(bit=='1')DBWriteContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsMarks", "10000000000");
		else  DBWriteContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsMarks", "00000000000");
	}
}

void RemoveSessionMark(HANDLE hContact,int mode,int marknum)
{
	unsigned int i=1;
	char temp_1[1]={'\0'},temp_2[1]={'\0'};
	char szDst[256]={'\0'};
	if(mode==0)
	{
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsMarks", &dbv) && dbv.pszVal)
		{
			for (i=marknum;i<ses_limit;i++)		
				dbv.pszVal[i]=dbv.pszVal[i+1];

			for (i=ses_limit;i<10;i++)
				dbv.pszVal[i]='0';

			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsMarks", dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	if (mode==1)
	{
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsMarks", &dbv) && dbv.pszVal)
		{
			for (i=marknum;i<ses_limit;i++)		
				dbv.pszVal[i]=dbv.pszVal[i+1];

			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsMarks", dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
}

void SetSessionMark(HANDLE hContact,int mode,char bit,unsigned int marknum)
{
	char* pszBuffer=NULL;

	if(mode==0)
	{	
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsMarks", &dbv) && dbv.pszVal)
		{
			dbv.pszVal[marknum]=bit;
			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsMarks", dbv.pszVal);

			DBFreeVariant(&dbv);
		}
	}

	if (mode==1)
	{
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsMarks", &dbv) && dbv.pszVal)
		{
			pszBuffer = mir_strdup(dbv.pszVal);
			pszBuffer[marknum]=bit;
			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsMarks", pszBuffer);
			mir_free(pszBuffer);
			DBFreeVariant(&dbv);
		}
	}
}

BOOL LoadContactsFromMask(HANDLE hContact,int mode,int count)
{	
	if(mode==0)
	{	
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsMarks", &dbv) && dbv.pszVal)
		{
			if (dbv.pszVal[count]=='1')
			{
				DBFreeVariant(&dbv);
				return 1;
			}
		}
		else
			return 0;
	}

	if (mode==1)
	{
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsMarks", &dbv) && dbv.pszVal)
		{
			if (dbv.pszVal[count]=='1')
			{
				DBFreeVariant(&dbv);
				return 1;
			}
		}
		else
			return 0;
	}
	return 0;
}

void AddInSessionOrder(HANDLE hContact,int mode,int ordernum,int writemode)
{
	int i;
	char temp_1[2]={'\0'},temp_2[2]={'\0'};
	char szDst[256]={'\0'};
	char* pszBuffer=NULL;
	char szFormNumBuf[255]={'\0'};

 	if(ordernum<10)
 		mir_snprintf(szFormNumBuf, SIZEOF(szFormNumBuf), "%u%u", 0, ordernum);
 	else
		mir_snprintf(szFormNumBuf, SIZEOF(szFormNumBuf), "%u", ordernum);
	
	if(mode==0)
	{
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsOrder", &dbv) && dbv.pszVal)
		{	  
			char* temp=NULL;
			char* temp2=NULL;
			int len= (int)strlen(dbv.pszVal);
			if(!len)
				len=20;

			temp2 = (char*)mir_alloc(len-1);
			ZeroMemory(temp2,len-1);
			temp = (char*)mir_alloc(len+1);
			ZeroMemory(temp,len+1);
			if(ordernum<10)
			{
				strncpy( temp2, dbv.pszVal, len-2 );
				temp2[len-2]='\0';
				mir_snprintf(temp, len+1, "%u%u%s", 0, ordernum, temp2);
			}
			else
				mir_snprintf(temp, len+1, "%u%s", ordernum, temp2);

			for (i=(ses_limit*2);i<20;i++)
				temp[i]='0';

			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsOrder", temp);
			mir_free(temp);
			mir_free(temp2);
			DBFreeVariant(&dbv);
		}
		else if(writemode==1)
		{
			ZeroMemory(szFormNumBuf,SIZEOF(szFormNumBuf));
			if(ordernum<10)
				mir_snprintf(szFormNumBuf, SIZEOF(szFormNumBuf), "%u%u%s", 0, ordernum, "000000000000000000");
 			else
				mir_snprintf(szFormNumBuf, SIZEOF(szFormNumBuf), "%u%s", ordernum, "000000000000000000");

			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsOrder", szFormNumBuf);
		}
	}

	if (mode==1)
	{
		DBVARIANT dbv;
		int advlen=0;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsOrder", &dbv) && dbv.pszVal)
		{
			char* temp=NULL;
			char* temp2=NULL;
			int len=0;
			if(strlen(dbv.pszVal)<(g_ses_count*2))
			{
				pszBuffer = (char*)mir_alloc(g_ses_count*2+1);
				ZeroMemory(pszBuffer,g_ses_count*2+1);
				strcpy(pszBuffer,dbv.pszVal);
			}
			else
			{
				pszBuffer = mir_strdup(dbv.pszVal);
			}
			if(!(advlen=len= (int)strlen(pszBuffer)))
				advlen=len=20;
			else
				advlen=len+2;
			
			temp = (char*)mir_alloc(advlen+1);
			ZeroMemory(temp,advlen+1);

			if(ordernum<10)
				mir_snprintf(temp, advlen+1, "%u%u%s", 0, ordernum, dbv.pszVal);
			else 
				mir_snprintf(temp, advlen+1, "%u%s", ordernum, dbv.pszVal);
				
			DBFreeVariant(&dbv);

			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsOrder", temp);
			mir_free(temp);
			mir_free(pszBuffer);
		}
		else if(writemode==1)
			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsOrder", szFormNumBuf);
		else
			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsOrder", "00");
	}
}

int GetInSessionOrder(HANDLE hContact,int mode,int count)
{
	int iOrder=0;
	char szTemp[3]={'\0'};
	if(mode==0)
	{	
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsOrder", &dbv) && dbv.pszVal)
		{
			szTemp[0]=dbv.pszVal[count*2];
			szTemp[1]=dbv.pszVal[count*2+1];
			iOrder=atoi(szTemp);
			DBFreeVariant(&dbv);
		}
	}
	if (mode==1)
	{
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsOrder", &dbv) && dbv.pszVal)
		{
			szTemp[0]=dbv.pszVal[count*2];
			szTemp[1]=dbv.pszVal[count*2+1];
			iOrder=atoi(szTemp);
			DBFreeVariant(&dbv);
		}
	}
	return iOrder;
}

void SetInSessionOrder(HANDLE hContact,int mode,int count,unsigned int ordernum)
{
	int iOrder=0;
	char szTemp[3]={'\0'};

	if(ordernum<10)
		mir_snprintf(szTemp, SIZEOF(szTemp), "%u%u", 0, ordernum);
	else
		mir_snprintf(szTemp, SIZEOF(szTemp), "%u", ordernum);

	if(mode==0)
	{	
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsOrder", &dbv) && dbv.pszVal)
		{
			dbv.pszVal[count*2]=szTemp[0];
			dbv.pszVal[count*2+1]=szTemp[1];
			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "LastSessionsOrder", dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}

	if (mode==1)
	{
		DBVARIANT dbv;
		if(!DBGetContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsOrder", &dbv) && dbv.pszVal)
		{
			dbv.pszVal[count*2]=szTemp[0];
			dbv.pszVal[count*2+1]=szTemp[1];
			DBWriteContactSettingString(hContact, __INTERNAL_NAME, "UserSessionsOrder", dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
}

BOOL ResaveSettings(char* szName,int iFirst,int iLimit,TCHAR* szBuffer)
{
	BYTE marked,marked_t;
	char szNameBuf[256]={0};

	for (int i = iFirst; i < iLimit; i++)
	{
		if (szBuffer)
		{
			DBVARIANT dbv_temp = {0};
			mir_snprintf(szNameBuf, SIZEOF(szNameBuf), "%s_%u", szName, i);
			DBGetContactSettingTString(NULL, __INTERNAL_NAME, szNameBuf, &dbv_temp);
			DBWriteContactSettingTString(NULL, __INTERNAL_NAME, szNameBuf, szBuffer);
			marked=IsMarkedUserDefSession(i);
			MarkUserDefSession(i,(BYTE)((i==iFirst)?IsMarkedUserDefSession(iFirst-1):marked_t));
			marked_t=marked;
			mir_free(szBuffer);
			if (dbv_temp.ptszVal)
			{				
				szBuffer = mir_tstrdup(dbv_temp.ptszVal);
				DBFreeVariant(&dbv_temp);
			}
			else
				return 0;
		}
		else
			break;
	}
	return 1;
}

int AddToCurSession (DWORD wparam,LPARAM lparam)
{
	int i;
	if (CheckForDuplicate(session_list,wparam)==-1)
	{
		for (i=0;;i++) 
		{
			if(session_list[i]==0)
			{
				session_list[i]=wparam;
				break;
			}
		}
	}
	return 0;
}

int DelFromCurSession(DWORD wparam,LPARAM lparam) 
{
	int i=0;
	while(session_list[i]!=0)
	{
		if(session_list[i]==wparam)
		{
			while(session_list[i+1]!=0)
			{
				session_list[i]=session_list[i+1];
				i++;
			}
			session_list[i]=0;
		}
		i++;
	}
	return 0;
}

int CheckForDuplicate(DWORD contact_list[],DWORD lparam)
{
	int i=0;
	DWORD s_list[255]={'0'};
	memcpy(s_list, contact_list,SIZEOF(s_list));
	for (i=0;;i++)
	{
		if (s_list[i]==lparam)
			return i;
		if (s_list[i]==0)
			return -1;
	}
	return 0;
}

int LoadSessionToCombobox (HWND hdlg,BOOL mode,int iLimit,char* pszSetting,int iFirstNum)
{
	int ses_count=iFirstNum,i, index;
	char szBuffer[256]={0};
	TCHAR *pszBuffer=NULL;
	if(session_list_recovered[0]&&!iFirstNum&&!mode)
	{
		index = SendDlgItemMessage(hdlg, IDC_LIST, CB_ADDSTRING, 0, (LPARAM)TranslateT("Session Before Last Crash"));
		SendDlgItemMessage(hdlg, IDC_LIST, CB_SETITEMDATA, (WPARAM)index, (LPARAM) 256);
	}
	for(i=0;i < iLimit; i++)
	{
		mir_snprintf(szBuffer, SIZEOF(szBuffer), "%s_%u", pszSetting, i);
		pszBuffer = DBGetStringT(NULL, __INTERNAL_NAME, szBuffer);
		if (pszBuffer)
		{
			if(!IsMarkedUserDefSession(i+iFirstNum)||mode==1)
			{
				index = SendDlgItemMessage(hdlg, IDC_LIST, CB_ADDSTRING, 0, (LPARAM)pszBuffer);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETITEMDATA, (WPARAM)index, (LPARAM) (DWORD)i+iFirstNum);
			}
			else
			{
				SendDlgItemMessage(hdlg, IDC_LIST, CB_INSERTSTRING, 0, (LPARAM)pszBuffer);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETITEMDATA, (WPARAM)0, (LPARAM) (DWORD)i+iFirstNum);
			}
			mir_free(pszBuffer);
			pszBuffer=NULL;
		}
		else
		{
			if (i==0) ses_count=iLimit-iFirstNum;
			break;
		}
	}
	if(pszBuffer)mir_free(pszBuffer);
	return ses_count;
}

int FillFavoritesMenu (HMENU hMenu,int iLimit)
{
	int i, iItems=0;
	char szBuffer[256]={0};
	TCHAR *pszBuffer=NULL;

	for(i=0;i < iLimit; i++)
	{
		if(IsMarkedUserDefSession(i))
		{
			mir_snprintf(szBuffer, SIZEOF(szBuffer), "%s_%u", "UserSessionDsc", i);
			pszBuffer = DBGetStringT(NULL, __INTERNAL_NAME, szBuffer);
			if ((pszBuffer = DBGetStringT(NULL, __INTERNAL_NAME, szBuffer)))
			{
				AppendMenu(hMenu,MF_STRING,i+1,pszBuffer);
				iItems++;
				//TODO: test this
				mir_free(pszBuffer);
				pszBuffer=NULL;
			}
		}
	}
	if(pszBuffer) mir_free(pszBuffer);
	return iItems;
}

void OffsetWindow(HWND parent, HWND hwnd, int dx, int dy)
{
	POINT pt;
	RECT rc;
	GetWindowRect(hwnd, &rc);
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(parent, &pt);
	rc.left  =pt.x ;
	rc.top   =pt.y ;

	OffsetRect(&rc, dx, dy);
	SetWindowPos(hwnd, NULL, rc.left, rc.top,0, 0, SWP_NOZORDER |SWP_NOSIZE);
}

int CheckContactVisibility(HANDLE hContact)
{ 
	if (DBGetContactSettingByte(hContact, "CList", "Hidden", 0))
		return 0;
	return 1;
}

void RenameUserDefSession(int ses_count,TCHAR* ptszNewName)
{
	char szSession[256]={0};
	mir_snprintf(szSession, SIZEOF(szSession), "%s_%u", "UserSessionDsc", ses_count);
	DBWriteContactSettingTString(NULL, __INTERNAL_NAME, szSession, ptszNewName);
}

int MarkUserDefSession(int ses_count,BYTE bCheck)
{
	char szSessionName[256]={0};
	mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "FavUserSession", ses_count);
	DBWriteContactSettingByte(NULL, __INTERNAL_NAME, szSessionName, bCheck);
	return 0;
}

BYTE IsMarkedUserDefSession(int ses_count)
{
	char szSessionName[256]={0};
	mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u","FavUserSession", ses_count);
	return DBGetContactSettingByte(NULL, __INTERNAL_NAME, szSessionName, 0);
}

int AddIcon(HICON icon, char *name, TCHAR *description)
{
	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszSection = _T(__INTERNAL_NAME);
	sid.cx = sid.cy = 16;
	sid.ptszDescription = description;
	sid.pszName = name;
	sid.hDefaultIcon = icon;
	return CallService(MS_SKIN2_ADDICON, 0, (LPARAM) &sid);
}

void SavePosition(HWND hwnd, char *wndName)
{
	RECT rc;
	char buffer[512];
	GetWindowRect(hwnd, &rc);
	mir_snprintf(buffer, SIZEOF(buffer), "%sPosX", wndName);
	DBWriteContactSettingDword(0, __INTERNAL_NAME, buffer, rc.left);
	mir_snprintf(buffer, SIZEOF(buffer), "%sPosY", wndName);
	DBWriteContactSettingDword(0, __INTERNAL_NAME, buffer, rc.top);
}

void LoadPosition(HWND hWnd, char *wndName)
{	
	char buffer[512];
	int x,y ;
	mir_snprintf(buffer, SIZEOF(buffer), "%sPosX", wndName);
	x = DBGetContactSettingDword(0, __INTERNAL_NAME, buffer, ((GetSystemMetrics(SM_CXSCREEN)) / 2)-130);
	mir_snprintf(buffer, SIZEOF(buffer), "%sPosY", wndName);
	y = DBGetContactSettingDword(0, __INTERNAL_NAME, buffer, ((GetSystemMetrics(SM_CYSCREEN))/ 2)-80);
	SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE);
}