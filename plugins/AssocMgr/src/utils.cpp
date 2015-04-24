/*

'File Association Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (AssocMgr-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

/************************* String Conv ****************************/

// mir_free() the return value
WCHAR* a2u(const char *pszAnsi,BOOL fMirCp)
{
	UINT codepage,cch;
	WCHAR *psz;
	
	if(pszAnsi==NULL) return NULL;
	codepage=fMirCp?CallService(MS_LANGPACK_GETCODEPAGE,0,0):CP_ACP;
	cch=MultiByteToWideChar(codepage,0,pszAnsi,-1,NULL,0);
	if (!cch) return NULL;

	psz=(WCHAR*)mir_alloc(cch*sizeof(WCHAR));
	if(psz!=NULL && !MultiByteToWideChar(codepage,0,pszAnsi,-1,psz,cch)) {
		mir_free(psz);
		return NULL;
	}
	return psz;
}

// mir_free() the return value
char* u2a(const WCHAR *pszUnicode,BOOL fMirCp)
{											
	UINT codepage,cch;
	char *psz;
	DWORD flags;

	if(pszUnicode==NULL) return NULL;
	codepage=fMirCp?CallService(MS_LANGPACK_GETCODEPAGE,0,0):CP_ACP;
	/* without WC_COMPOSITECHECK some characters might get out strange (see MS blog) */
	cch=WideCharToMultiByte(codepage,flags=WC_COMPOSITECHECK,pszUnicode,-1,NULL,0,NULL,NULL);
	if (!cch) cch=WideCharToMultiByte(codepage,flags=0,pszUnicode,-1,NULL,0,NULL,NULL);
	if (!cch) return NULL;

	psz=(char*)mir_alloc(cch);
	if(psz!=NULL && !WideCharToMultiByte(codepage,flags,pszUnicode,-1,psz,cch,NULL,NULL)) {
		mir_free(psz);
		return NULL;
	}
	return psz;
}

// mir_free() the return value
TCHAR* s2t(const void *pszStr,DWORD fUnicode,BOOL fMirCp)
{

	if(fUnicode) return mir_wstrdup((WCHAR*)pszStr);
	return a2u((char*)pszStr,fMirCp);

}

// mir_free() the return value
void* t2s(const TCHAR *pszStr,DWORD fUnicode,BOOL fMirCp)
{

	if (!fUnicode) return (void*)u2a(pszStr,fMirCp);
	return (void*)mir_wstrdup(pszStr);

}

/************************* Database *******************************/

struct EnumPrefixSettingsParams {
	char **settings;
	int nSettingsCount;
	const char *pszPrefix;
	int nPrefixLen;
};

static int EnumPrefixSettingsProc(const char *pszSetting,LPARAM lParam)
{
	struct EnumPrefixSettingsParams *param=(struct EnumPrefixSettingsParams*)lParam;
	if (!strncmp(pszSetting,param->pszPrefix,param->nPrefixLen)) {
		char **buf;
		/* resize storage array */
		buf = (char**)mir_realloc(param->settings,(param->nSettingsCount+1)*sizeof(char*));
		if(buf!=NULL) {
			param->settings=buf;
			buf[param->nSettingsCount]=mir_strdup(pszSetting);
			if(buf[param->nSettingsCount]!=NULL) ++param->nSettingsCount;
		}
	}
	return 0;
}

// mir_free() the returned pSettings after use
BOOL EnumDbPrefixSettings(const char *pszModule,const char *pszSettingPrefix,char ***pSettings,int *pnSettingsCount)
{
	DBCONTACTENUMSETTINGS dbces;
	struct EnumPrefixSettingsParams param;
	dbces.szModule=pszModule;
	dbces.pfnEnumProc=EnumPrefixSettingsProc;
	dbces.lParam=(LPARAM)&param;
	param.settings=NULL;
	param.nSettingsCount=0;
	param.pszPrefix=pszSettingPrefix;
	param.nPrefixLen=mir_strlen(pszSettingPrefix);
	CallService(MS_DB_CONTACT_ENUMSETTINGS,0,(LPARAM)&dbces);
	*pnSettingsCount=param.nSettingsCount;
	*pSettings=param.settings;
	return param.nSettingsCount!=0;
}

/************************* Error Output ***************************/

static void MessageBoxIndirectFree(void *param)
{
	MSGBOXPARAMSA *mbp = (MSGBOXPARAMSA*)param;
	MessageBoxIndirectA(mbp);
	mir_free((char*)mbp->lpszCaption); /* does NULL check */
	mir_free((char*)mbp->lpszText);    /* does NULL check */
	mir_free(mbp);
}

void ShowInfoMessage(BYTE flags,const char *pszTitle,const char *pszTextFmt,...)
{
	char szText[256]; /* max for systray */
	MSGBOXPARAMSA *mbp;

	va_list va;
	va_start(va,pszTextFmt);
	mir_vsnprintf(szText,SIZEOF(szText),pszTextFmt,va);
	va_end(va);

	if(ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
		MIRANDASYSTRAYNOTIFY msn;
		msn.cbSize=sizeof(msn);
		msn.szProto=NULL;
		msn.szInfoTitle=(char*)pszTitle;
		msn.szInfo=(char*)szText;
		msn.uTimeout=30000; /* max timeout */
		msn.dwInfoFlags=flags;
		if (!CallServiceSync(MS_CLIST_SYSTRAY_NOTIFY,0,(LPARAM)&msn))
			return; /* success */
	}

	mbp=(MSGBOXPARAMSA*)mir_calloc(sizeof(*mbp));
	if(mbp==NULL) return;
	mbp->cbSize=sizeof(*mbp);
	mbp->lpszCaption=mir_strdup(pszTitle);
	mbp->lpszText=mir_strdup(szText);
	mbp->dwStyle=MB_OK|MB_SETFOREGROUND|MB_TASKMODAL;
	mbp->dwLanguageId=LANGIDFROMLCID((LCID)CallService(MS_LANGPACK_GETLOCALE,0,0));
	switch(flags&NIIF_ICON_MASK) {
		case NIIF_INFO:    mbp->dwStyle|=MB_ICONINFORMATION; break;
		case NIIF_WARNING: mbp->dwStyle|=MB_ICONWARNING; break;
		case NIIF_ERROR:   mbp->dwStyle|=MB_ICONERROR;
	}
	mir_forkthread(MessageBoxIndirectFree, mbp);
}

// LocalFree() the return value
char* GetWinErrorDescription(DWORD dwLastError)
{
	char *buf=NULL;
	DWORD flags=FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM;
	if (!FormatMessageA(flags,NULL,dwLastError,LANGIDFROMLCID((LCID)CallService(MS_LANGPACK_GETLOCALE,0,0)),(char*)&buf,0,NULL))
		if(GetLastError()==ERROR_RESOURCE_LANG_NOT_FOUND) 
			FormatMessageA(flags,NULL,dwLastError,0,(char*)&buf,0,NULL);
	return buf;
}
