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

#include "common.h"

extern HINSTANCE hInst;

#ifdef _DEBUG
/* Debug: Ensure all registry calls do succeed and have valid parameters. 
 * Shows a details message box otherwise. */
static __inline LONG regchk(LONG res, const char *pszFunc, const void *pszInfo, BOOL fInfoUnicode, const char *pszFile, unsigned int nLine)
{
	if (res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND && res != ERROR_NO_MORE_ITEMS) {
		TCHAR szMsg[1024], *pszInfo2;
		char *pszErr;
		pszErr = GetWinErrorDescription(res);
		pszInfo2 = s2t(pszInfo, fInfoUnicode, FALSE);  /* does NULL check */
		mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("Access failed:\n%.64hs(%.128s)\n%.250hs(%u)\n%.256hs (%u)"), pszFunc, pszInfo2, pszFile, nLine, pszErr, res);
		MessageBox(NULL, szMsg, TranslateT("Registry warning"), MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_TASKMODAL);
		if (pszErr != NULL) LocalFree(pszErr);
		mir_free(pszInfo2);  /* does NULL check */
	}
	return res;
}
#undef  RegCloseKey
#define RegCloseKey(hKey) \
	regchk(RegCloseKey(hKey),"RegCloseKey",NULL,FALSE,__FILE__,__LINE__)
#undef  RegOpenKeyExA
#define RegOpenKeyExA(hKey,szSubkey,opt,rights,phKey) \
	regchk(RegOpenKeyExA(hKey,szSubkey,opt,rights,phKey),"RegOpenKeyExA",szSubkey,FALSE,__FILE__,__LINE__)
#undef  RegCreateKeyExA
#define RegCreateKeyExA(hKey,szSubkey,x,y,opt,rights,sec,phKey,pDisp) \
	regchk(RegCreateKeyExA(hKey,szSubkey,x,y,opt,rights,sec,phKey,pDisp),"RegCreateKeyExA",szSubkey,FALSE,__FILE__,__LINE__)
#undef  RegDeleteKeyA
#define RegDeleteKeyA(hKey,szName) \
	regchk(RegDeleteKeyA(hKey,szName),"RegDeleteKeyA",szName,FALSE,__FILE__,__LINE__)
#undef  RegSetValueExA
#define RegSetValueExA(hSubKey,szName,x,type,pVal,size) \
	regchk(RegSetValueExA(hSubKey,szName,x,type,pVal,size),"RegSetValueExA",szName,FALSE,__FILE__,__LINE__)
#undef  RegQueryValueExA
#define RegQueryValueExA(hKey,szName,x,pType,pVal,pSize) \
	regchk(RegQueryValueExA(hKey,szName,x,pType,pVal,pSize),"RegQueryValueExA",szName,FALSE,__FILE__,__LINE__)
#undef  RegQueryInfoKeyA
#define RegQueryInfoKeyA(hKey,x,y,z,pnKeys,pnKeyLen,a,pnVals,pnNames,pnValLen,sec,pTime) \
	regchk(RegQueryInfoKeyA(hKey,x,y,z,pnKeys,pnKeyLen,a,pnVals,pnNames,pnValLen,sec,pTime),"RegQueryInfoKeyA",NULL,FALSE,__FILE__,__LINE__)
#undef  RegEnumKeyExA
#define RegEnumKeyExA(hKey,idx,pName,pnName,x,y,z,pTime) \
	regchk(RegEnumKeyExA(hKey,idx,pName,pnName,x,y,z,pTime),"RegEnumKeyExA",NULL,FALSE,__FILE__,__LINE__)
#undef  RegDeleteValueA
#define RegDeleteValueA(hKey,szName) \
	regchk(RegDeleteValueA(hKey,szName),"RegDeleteValueA",szName,FALSE,__FILE__,__LINE__)
#undef  RegOpenKeyExW
#define RegOpenKeyExW(hKey,szSubkey,x,sam,phKey) \
	regchk(RegOpenKeyExW(hKey,szSubkey,x,sam,phKey),"RegOpenKeyExW",szSubkey,TRUE,__FILE__,__LINE__)
#undef  RegCreateKeyExW
#define RegCreateKeyExW(hKey,szSubkey,x,y,z,rights,p,phKey,q) \
	regchk(RegCreateKeyExW(hKey,szSubkey,x,y,z,rights,p,phKey,q),"RegCreateKeyExW",szSubkey,TRUE,__FILE__,__LINE__)
#undef  RegDeleteKeyW
#define RegDeleteKeyW(hKey,szName) \
	regchk(RegDeleteKeyW(hKey,szName),"RegDeleteKeyW",szName,TRUE,__FILE__,__LINE__)
#undef  RegSetValueExW
#define RegSetValueExW(hSubKey,szName,x,type,pVal,size) \
	regchk(RegSetValueExW(hSubKey,szName,x,type,pVal,size),"RegSetValueExW",szName,TRUE,__FILE__,__LINE__)
#undef  RegQueryValueExW
#define RegQueryValueExW(hKey,szName,x,pType,pVal,pSize) \
	regchk(RegQueryValueExW(hKey,szName,x,pType,pVal,pSize),"RegQueryValueExW",szName,TRUE,__FILE__,__LINE__)
#undef  RegQueryInfoKeyW
#define RegQueryInfoKeyW(hKey,x,y,z,pnKeys,pnKeyLen,a,pnVals,pnNames,pnValLen,sec,pTime) \
	regchk(RegQueryInfoKeyW(hKey,x,y,z,pnKeys,pnKeyLen,a,pnVals,pnNames,pnValLen,sec,pTime),"RegQueryInfoKeyW",NULL,TRUE,__FILE__,__LINE__)
#undef  RegEnumKeyExW
#define RegEnumKeyExW(hKey,idx,pName,pnName,x,y,z,pTime) \
	regchk(RegEnumKeyExW(hKey,idx,pName,pnName,x,y,z,pTime),"RegEnumKeyExW",NULL,TRUE,__FILE__,__LINE__)
#undef  RegDeleteValueW
#define RegDeleteValueW(hKey,szName) \
	regchk(RegDeleteValueW(hKey,szName),"RegDeleteValueW",szName,TRUE,__FILE__,__LINE__)
#endif // _DEBUG

/************************* Strings ********************************/

// mir_free() the return value
char *MakeFileClassName(const char *pszFileExt)
{
	int cbLen = mir_strlen(pszFileExt)+12;
	char *pszClass = (char*)mir_alloc(cbLen);
	if (pszClass != NULL)
		/* using correctly formated PROGID */
		mir_snprintf(pszClass, cbLen, "miranda%sfile", pszFileExt); /* includes dot, buffer safe */
	return pszClass;
}

// mir_free() the return value
char *MakeUrlClassName(const char *pszUrl)
{
	char *pszClass = mir_strdup(pszUrl);
	if (pszClass != NULL)
		/* remove trailing : */
		pszClass[mir_strlen(pszClass)-1]=0;
	return pszClass;
}

static BOOL IsFileClassName(char *pszClassName, char **ppszFileExt)
{
	*ppszFileExt = strchr(pszClassName,'.');
	return *ppszFileExt!=NULL;
}

// mir_free() the return value
TCHAR *MakeRunCommand(BOOL fMirExe,BOOL fFixedDbProfile)
{
	TCHAR szDbFile[MAX_PATH], szExe[MAX_PATH], *pszFmt;
	if (fFixedDbProfile) {
		if ( CallService(MS_DB_GETPROFILENAMET, SIZEOF(szDbFile), (LPARAM)szDbFile))
			return NULL;
		TCHAR *p = _tcsrchr(szDbFile, '.');
		if (p)
			*p = 0;
	}
	else mir_tstrcpy(szDbFile, _T("%1")); /* buffer safe */

	if ( !GetModuleFileName(fMirExe ? NULL : hInst, szExe, SIZEOF(szExe)))
		return NULL;

	if (fMirExe)
		/* run command for miranda32.exe */
			pszFmt = _T("\"%s\" \"/profile:%s\"");
	else {
		/* run command for rundll32.exe calling WaitForDDE */
		pszFmt = _T("rundll32.exe %s,WaitForDDE \"/profile:%s\"");
		/* ensure the command line is not too long */ 
		GetShortPathName(szExe, szExe, SIZEOF(szExe));
		/* surround by quotes if failed */
		DWORD len = mir_tstrlen(szExe);
		if ( _tcschr(szExe,_T(' ')) != NULL && (len+2) < SIZEOF(szExe)) {
			MoveMemory(szExe, szExe+1, (len+1)*sizeof(TCHAR));
			szExe[len+2] = szExe[0] = _T('\"');
			szExe[len+3] = 0;
		}
	}

	TCHAR tszBuffer[1024];
	mir_sntprintf(tszBuffer, SIZEOF(tszBuffer), pszFmt, szExe, szDbFile);
	return mir_tstrdup(tszBuffer);
}

static BOOL IsValidRunCommand(const TCHAR *pszRunCmd)
{
	TCHAR *buf,*pexe,*pargs;
	TCHAR szFullExe[MAX_PATH],*pszFilePart;
	buf=mir_tstrcpy((TCHAR*)_alloca((mir_tstrlen(pszRunCmd)+1)*sizeof(TCHAR)),pszRunCmd);
	/* split into executable path and arguments */
	if (buf[0]==_T('\"')) {
		pargs=_tcschr(&buf[1],_T('\"'));
		if (pargs!=NULL) *(pargs++)=0;
		pexe=&buf[1];
		if (*pargs==_T(' ')) ++pargs;
	} else {
		pargs=_tcschr(buf,_T(' '));
		if (pargs!=NULL) *pargs=0;
		pexe=buf;
	}
	if (SearchPath(NULL,pexe,_T(".exe"),SIZEOF(szFullExe),szFullExe,&pszFilePart)) {
		if (pszFilePart!=NULL)
			if (!mir_tstrcmpi(pszFilePart,_T("rundll32.exe")) || !mir_tstrcmpi(pszFilePart,_T("rundll.exe"))) {
				/* split into dll path and arguments */
				if (pargs[0]==_T('\"')) {
					++pargs;
					pexe=_tcschr(&pargs[1],_T('\"'));
					if (pexe!=NULL) *pexe=0;
				} else {
					pexe=_tcschr(pargs,_T(','));
					if (pexe!=NULL) *pexe=0;
				}
				return SearchPath(NULL,pargs,_T(".dll"),0,NULL,NULL)!=0;
			}
			return TRUE;
	}
	return FALSE;
}

// mir_free() the return value
TCHAR *MakeIconLocation(HMODULE hModule,WORD nIconResID)
{
	TCHAR szModule[MAX_PATH],*pszIconLoc=NULL;
	int cch;
	if ((cch=GetModuleFileName(hModule,szModule,SIZEOF(szModule))) != 0) {
		pszIconLoc=(TCHAR*)mir_alloc((cch+=8)*sizeof(TCHAR));
		if (pszIconLoc!=NULL)
			mir_sntprintf(pszIconLoc, cch, _T("%s,%i"), szModule, -(int)nIconResID); /* id may be 0, buffer safe */
	}
	return pszIconLoc;
}

// mir_free() the return value
TCHAR *MakeAppFileName(BOOL fMirExe)
{
	TCHAR szExe[MAX_PATH],*psz;
	if (GetModuleFileName(fMirExe?NULL:hInst,szExe,SIZEOF(szExe))) {
		psz=_tcsrchr(szExe,_T('\\'));
		if (psz!=NULL) ++psz;
		else psz=szExe;
		return mir_tstrdup(psz);
	}
	return NULL;
}

/************************* Helpers ********************************/

static LONG DeleteRegSubTree(HKEY hKey,const TCHAR *pszSubKey)
{
	LONG res;
	DWORD nMaxSubKeyLen,cchSubKey;
	TCHAR *pszSubKeyBuf;
	HKEY hSubKey;
	if ((res=RegOpenKeyEx(hKey,pszSubKey,0,KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS|DELETE,&hSubKey))==ERROR_SUCCESS) {
		if ((res=RegQueryInfoKey(hSubKey,NULL,NULL,NULL,NULL,&nMaxSubKeyLen,NULL,NULL,NULL,NULL,NULL,NULL))==ERROR_SUCCESS) {
			pszSubKeyBuf=(TCHAR*)mir_alloc((nMaxSubKeyLen+1)*sizeof(TCHAR));
			if (pszSubKeyBuf==NULL) res=ERROR_NOT_ENOUGH_MEMORY;
			while(!res) {
				cchSubKey=nMaxSubKeyLen+1;
				if ((res=RegEnumKeyEx(hSubKey,0,pszSubKeyBuf,&cchSubKey,NULL,NULL,NULL,NULL))==ERROR_SUCCESS)
					res=DeleteRegSubTree(hSubKey,pszSubKeyBuf); /* recursion */
			}
			mir_free(pszSubKeyBuf); /* does NULL check */
			if (res==ERROR_NO_MORE_ITEMS) res=ERROR_SUCCESS;
		}
		RegCloseKey(hSubKey);
	}
	if (!res) res=RegDeleteKey(hKey,pszSubKey);
	return res;
}

// hMainKey must have been opened with KEY_CREATE_SUB_KEY access right
static LONG SetRegSubKeyStrDefValue(HKEY hMainKey,const TCHAR *pszSubKey,const TCHAR *pszVal)
{
	HKEY hSubKey;
	LONG res=RegCreateKeyEx(hMainKey,pszSubKey,0,NULL,0,KEY_SET_VALUE|KEY_QUERY_VALUE,NULL,&hSubKey,NULL);
	if (!res) {
		res=RegSetValueEx(hSubKey,NULL,0,REG_SZ,(BYTE*)pszVal,(mir_tstrlen(pszVal)+1)*sizeof(TCHAR));
		RegCloseKey(hSubKey);
	}
	return res;
}

// hKey must have been opened with KEY_SET_VALUE access right
static void SetRegStrPrefixValue(HKEY hKey,const TCHAR *pszValPrefix,const TCHAR *pszVal)
{
	DWORD dwSize=(mir_tstrlen(pszVal)+mir_tstrlen(pszValPrefix)+1)*sizeof(TCHAR);
	TCHAR *pszStr=(TCHAR*)mir_alloc(dwSize);
	if (pszStr==NULL) return;
	mir_tstrcat(mir_tstrcpy(pszStr,pszValPrefix),pszVal); /* buffer safe */
	RegSetValueEx(hKey,NULL,0,REG_SZ,(BYTE*)pszStr,dwSize);
	mir_free(pszStr);
}

// hKey must have been opened with KEY_QUERY_VALUE access right
// mir_free() the return value
static TCHAR *GetRegStrValue(HKEY hKey,const TCHAR *pszValName)
{
	TCHAR *pszVal,*pszVal2;
	DWORD dwSize,dwType;
	/* get size */
	if (!RegQueryValueEx(hKey,pszValName,NULL,NULL,NULL,&dwSize) && dwSize>sizeof(TCHAR)) {
		pszVal=(TCHAR*)mir_alloc(dwSize+sizeof(TCHAR));
		if (pszVal!=NULL) {
			/* get value */
			if (!RegQueryValueEx(hKey,pszValName,NULL,&dwType,(BYTE*)pszVal,&dwSize)) {
				pszVal[dwSize/sizeof(TCHAR)]=0;
				if (dwType==REG_EXPAND_SZ) {
					dwSize=MAX_PATH;
					pszVal2=(TCHAR*)mir_alloc(dwSize*sizeof(TCHAR));
					if (ExpandEnvironmentStrings(pszVal,pszVal2,dwSize)) {
						mir_free(pszVal);
						return pszVal2;
					}
					mir_free(pszVal2);
				} else if (dwType==REG_SZ)
					return pszVal;
			}
			mir_free(pszVal);
		}
	}
	return NULL;
}

// hKey must have been opened with KEY_QUERY_VALUE access right
static BOOL IsRegStrValue(HKEY hKey,const TCHAR *pszValName,const TCHAR *pszCmpVal)
{
	BOOL fSame=FALSE;
	TCHAR *pszVal=GetRegStrValue(hKey,pszValName);
	if (pszVal!=NULL) {
		fSame=!mir_tstrcmp(pszVal,pszCmpVal);
		mir_free(pszVal);
	}
	return fSame;
}

// hKey must have been opened with KEY_QUERY_VALUE access right
static BOOL IsRegStrValueA(HKEY hKey,const TCHAR *pszValName,const char *pszCmpVal)
{
	BOOL fSame=FALSE;
	char *pszValA;
	TCHAR *pszVal=GetRegStrValue(hKey,pszValName);
	if (pszVal!=NULL) {
		pszValA=t2a(pszVal);
		if (pszValA!=NULL)
			fSame=!mir_strcmp(pszValA,pszCmpVal);
		mir_free(pszValA); /* does NULL check */
		mir_free(pszVal);
	}
	return fSame;
}

/************************* Backup to DB ***************************/

#define REGF_ANSI  0x80000000 /* this bit is set in dwType for ANSI registry data */ 

// pData must always be Unicode data, registry supports Unicode even on Win95
static void WriteDbBackupData(const char *pszSetting,DWORD dwType,BYTE *pData,DWORD cbData)
{
	size_t cbLen = cbData + sizeof(DWORD);
	PBYTE buf = (PBYTE)mir_alloc(cbLen);
	if (buf) {
		*(DWORD*)buf = dwType;
		memcpy(buf+sizeof(DWORD), pData, cbData);
		db_set_blob(NULL, "AssocMgr", pszSetting, buf, (unsigned)cbLen);
		mir_free(buf);
	}
}

// mir_free() the value returned in ppData 
static BOOL ReadDbBackupData(const char *pszSetting,DWORD *pdwType,BYTE **ppData,DWORD *pcbData)
{
	DBVARIANT dbv;
	if (!db_get(0, "AssocMgr", pszSetting, &dbv)) {
		if (dbv.type==DBVT_BLOB && dbv.cpbVal>=sizeof(DWORD)) {
			*pdwType=*(DWORD*)dbv.pbVal;
			*ppData=dbv.pbVal;
			*pcbData=dbv.cpbVal-sizeof(DWORD);
			MoveMemory(*ppData,*ppData+sizeof(DWORD),*pcbData);
			return TRUE;
		}
		db_free(&dbv);
	}
	return FALSE;
}

struct BackupRegTreeParam {
	char **ppszDbPrefix;
	DWORD *pdwDbPrefixSize;
	int level;
};

static void BackupRegTree_Worker(HKEY hKey,const char *pszSubKey,struct BackupRegTreeParam *param)
{
	LONG res;
	DWORD nMaxSubKeyLen,nMaxValNameLen,nMaxValSize;
	DWORD index,cchName,dwType,cbData;
	BYTE *pData;
	char *pszName;
	register TCHAR *ptszName;
	DWORD nDbPrefixLen;
	if ((res=RegOpenKeyExA(hKey,pszSubKey,0,KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS,&hKey))==ERROR_SUCCESS) {
		if ((res=RegQueryInfoKey(hKey,NULL,NULL,NULL,NULL,&nMaxSubKeyLen,NULL,NULL,&nMaxValNameLen,&nMaxValSize,NULL,NULL))==ERROR_SUCCESS) {
			if (nMaxSubKeyLen>nMaxValNameLen) nMaxValNameLen=nMaxSubKeyLen;
			/* prepare buffer */
			nDbPrefixLen=(DWORD)mir_strlen(*param->ppszDbPrefix)+mir_strlen(pszSubKey)+1;
			cchName=nDbPrefixLen+nMaxValNameLen+3;
			if (cchName>*param->pdwDbPrefixSize) {
				pszName=(char*)mir_realloc(*param->ppszDbPrefix,cchName);
				if (pszName==NULL) return;
				*param->ppszDbPrefix=pszName;
				*param->pdwDbPrefixSize=cchName;
			}
			mir_strcat(mir_strcat(*param->ppszDbPrefix,pszSubKey),"\\"); /* buffer safe */
			/* enum values */
			pszName=(char*)mir_alloc(nMaxValNameLen+1);
			if (nMaxValSize==0) nMaxValSize=1;
			pData=(BYTE*)mir_alloc(nMaxValSize);
			if (pszName!=NULL && pData!=NULL) {
				index=0;
				while(!res) {
					cchName=nMaxValNameLen+1;
					cbData=nMaxValSize;
					if ((res=RegEnumValueA(hKey,index++,pszName,&cchName,NULL,NULL,NULL,NULL))==ERROR_SUCCESS) {
						(*param->ppszDbPrefix)[nDbPrefixLen]=0;
						mir_strcat(*param->ppszDbPrefix,pszName); /* buffer safe */
						ptszName=a2t(pszName);
						if (ptszName!=NULL) {
							if (!RegQueryValueEx(hKey,ptszName,NULL,&dwType,pData,&cbData)) {

								WriteDbBackupData(*param->ppszDbPrefix,dwType,pData,cbData);

							}
							mir_free(ptszName);
						}
					}
				}
				if (res==ERROR_NO_MORE_ITEMS) res=ERROR_SUCCESS;
			}
			mir_free(pData); /* does NULL check */
			/* enum subkeys */
			if (param->level<32 && pszName!=NULL) {
				++param->level; /* can be max 32 levels deep (after prefix), restriction of RegCreateKeyEx() */
				index=0;
				while(!res) {
					cchName=nMaxSubKeyLen+1;
					if ((res=RegEnumKeyExA(hKey,index++,pszName,&cchName,NULL,NULL,NULL,NULL))==ERROR_SUCCESS) {
						(*param->ppszDbPrefix)[nDbPrefixLen]=0;
						BackupRegTree_Worker(hKey,pszName,param); /* recursion */
					}
				}
			}
			if (res==ERROR_NO_MORE_ITEMS) res=ERROR_SUCCESS;
			mir_free(pszName); /* does NULL check */
		}
		RegCloseKey(hKey);
	}
}

static void BackupRegTree(HKEY hKey,const char *pszSubKey,const char *pszDbPrefix)
{
	struct BackupRegTreeParam param;
	DWORD dwDbPrefixSize;
	param.level=0;
	param.pdwDbPrefixSize=&dwDbPrefixSize;
	param.ppszDbPrefix=(char**)&pszDbPrefix;
	pszDbPrefix=mir_strdup(pszDbPrefix);
	if (pszDbPrefix!=NULL) {
		dwDbPrefixSize=mir_strlen(pszDbPrefix)+1;
		BackupRegTree_Worker(hKey,pszSubKey,&param);
		mir_free((char*)pszDbPrefix);
	}
}

static LONG RestoreRegTree(HKEY hKey,const char *pszSubKey,const char *pszDbPrefix)
{
	char **ppszSettings,*pszSuffix;
	int nSettingsCount,i;
	char *pslash=NULL,*pnext,*pkeys;
	char *pszValName;
	WCHAR *pwszValName;
	HKEY hSubKey;
	DWORD dwType,cbData;
	BYTE *pData;

	int nDbPrefixLen=mir_strlen(pszDbPrefix);
	int nPrefixWithSubKeyLen=nDbPrefixLen+mir_strlen(pszSubKey)+1;
	char *pszPrefixWithSubKey=(char*)mir_alloc(nPrefixWithSubKeyLen+1);
	if (pszPrefixWithSubKey==NULL) return ERROR_OUTOFMEMORY;
	mir_strcat(mir_strcat(mir_strcpy(pszPrefixWithSubKey,pszDbPrefix),pszSubKey),"\\"); /* buffer safe */
	LONG res=ERROR_NO_MORE_ITEMS;
	if (pszPrefixWithSubKey!=NULL) {
		if (EnumDbPrefixSettings("AssocMgr",pszPrefixWithSubKey,&ppszSettings,&nSettingsCount)) {
			for(i=0;i<nSettingsCount;++i) {
				pszSuffix=&ppszSettings[i][nDbPrefixLen];
				/* key hierachy */
				pkeys=mir_strcpy((char*)_alloca(mir_strlen(pszSuffix)+1),pszSuffix);
				pnext=pkeys;
				while((pnext=strchr(pnext+1,_T('\\')))!=NULL) pslash=pnext;
				if (pslash!=NULL) {
					/* create subkey */
					*(pslash++)=0;
					hSubKey=hKey;
					if (pslash!=pkeys+1)
						if ((res=RegCreateKeyExA(hKey,pkeys,0,NULL,0,KEY_SET_VALUE,NULL,&hSubKey,NULL))!=ERROR_SUCCESS)
							break;
					pszValName=pslash;
					/* read data */
					if (ReadDbBackupData(ppszSettings[i],&dwType,&pData,&cbData)) {
						/* set value */
						if (!(dwType&REGF_ANSI)) {
							pwszValName=a2u(pszValName,FALSE);
							if (pwszValName!=NULL) res=RegSetValueExW(hSubKey,pwszValName,0,dwType,pData,cbData);
							else res=ERROR_NOT_ENOUGH_MEMORY;
							mir_free(pwszValName); /* does NULL check */
						} else res=RegSetValueExA(hSubKey,pszValName,0,dwType&~REGF_ANSI,pData,cbData);
						mir_free(pData);
					} else res=ERROR_INVALID_DATA;
					if (res) break;
					db_unset(NULL,"AssocMgr",ppszSettings[i]);
					if (hSubKey!=hKey) RegCloseKey(hSubKey);
				}
				mir_free(ppszSettings[i]);
			}
			mir_free(ppszSettings);
		}
		mir_free(pszPrefixWithSubKey);
	}
	return res;
}

static void DeleteRegTreeBackup(const char *pszSubKey,const char *pszDbPrefix)
{
	char **ppszSettings;
	int nSettingsCount,i;

	char *pszPrefixWithSubKey=(char*)mir_alloc(mir_strlen(pszDbPrefix)+mir_strlen(pszSubKey)+2);
	if (pszPrefixWithSubKey==NULL) return;
	mir_strcat(mir_strcat(mir_strcpy(pszPrefixWithSubKey,pszDbPrefix),pszSubKey),"\\"); /* buffer safe */
	if (pszPrefixWithSubKey!=NULL) {
		if (EnumDbPrefixSettings("AssocMgr",pszPrefixWithSubKey,&ppszSettings,&nSettingsCount)) {
			for(i=0;i<nSettingsCount;++i) {
				db_unset(NULL,"AssocMgr",ppszSettings[i]);
				mir_free(ppszSettings[i]);
			}
			mir_free(ppszSettings);
		}
		mir_free(pszPrefixWithSubKey);
	}
}

void CleanupRegTreeBackupSettings(void)
{
	/* delete old bak_* settings and try to restore backups */
	int nSettingsCount;
	char **ppszSettings;
	if ( !EnumDbPrefixSettings("AssocMgr", "bak_", &ppszSettings, &nSettingsCount))
		return;

	for(int i=0; i < nSettingsCount; ++i) {
		char *pszClassName = &ppszSettings[i][4];
		char *pszBuf = strchr(pszClassName,'\\');
		if (pszBuf != NULL) {
			*pszBuf = '\0';

			/* remove others in list with same class name */
			if(i < nSettingsCount-1){
				for(int j=i+1; j < nSettingsCount; ++j) {
					pszBuf = strchr(&ppszSettings[j][4],'\\');
					if (pszBuf != NULL) *pszBuf='\0';
					if (mir_strcmp(pszClassName, &ppszSettings[j][4])){
						if (pszBuf != NULL) *pszBuf='\\';
						continue;
					}

					mir_free(ppszSettings[j]);
					MoveMemory(&ppszSettings[j], &ppszSettings[j+1], ((--nSettingsCount)-j) * sizeof(char*));
					--j; /* reiterate current index */
				}
			}

			/* no longer registered? */
			if (!IsRegisteredAssocItem(pszClassName)) {
				char *pszFileExt;
				if (IsFileClassName(pszClassName, &pszFileExt))
					RemoveRegFileExt(pszFileExt, pszClassName);
				else
					RemoveRegClass(pszClassName);
			}
		}
		mir_free(ppszSettings[i]);
	}
	mir_free(ppszSettings);
}

/************************* Opera Support **************************/

/*
* These are helpers accessing the Opera settings file.
* Should work with Opera 6 up to 9.10 (current)
*/

static BOOL Opera6_GetIniFilePath(TCHAR *szIniFile)
{
	HKEY hExeKey;
	TCHAR szPath[MAX_PATH],*p;
	BOOL fSuccess=FALSE;
	DWORD len;

	/* Info: http://opera-info.de/forum/thread.php?threadid=2905 */
	/* app path */
	if (!RegOpenKeyExA(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Netscape.exe",0,KEY_QUERY_VALUE,&hExeKey)) {
		/* exe name */
		p=GetRegStrValue(hExeKey,NULL);
		if (p!=NULL && _tcsstr(p,_T("Opera.exe"))!=NULL) {
			/* path */
			mir_free(p);
			p=GetRegStrValue(hExeKey,_T("Path"));
			len=mir_tstrlen(p);
			if (p[len-1]==_T('\\')) p[len-1]=0;
			fSuccess=(p!=NULL && ExpandEnvironmentStrings(p,szPath,MAX_PATH));
		}
		mir_free(p); /* does NULL check */
		RegCloseKey(hExeKey);
	}
	if (fSuccess) {
		TCHAR szFileBuf[MAX_PATH+34];
		/* operadef6.ini */
		mir_tstrcat(mir_tstrcpy(szFileBuf,szPath),_T("\\operadef6.ini")); /* buffer safe */
		/* If enabled Opera will use Windows profiles to store individual user settings */
		if (GetPrivateProfileInt(_T("System"),_T("Multi User"),0,szFileBuf)==1) {
			p=_tcsrchr(szPath,'\\');
			mir_tstrcpy(szFileBuf,_T("%APPDATA%\\Opera")); /* buffer safe */
			if (p!=NULL) mir_tstrcat(szFileBuf,p);  /* buffer safe */
		} else mir_tstrcpy(szFileBuf,szPath);
		/* opera6.ini */
		mir_tstrcat(szFileBuf,_T("\\profile\\opera6.ini")); /* buffer safe */
		fSuccess=ExpandEnvironmentStrings(szFileBuf,szIniFile,MAX_PATH)!=0;
	}
	/* check file existstance */
	if (fSuccess) {
		HANDLE hFile;
		hFile=CreateFile(szIniFile,0,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
		if (hFile==INVALID_HANDLE_VALUE) fSuccess=FALSE;
		else CloseHandle(hFile);
	}
	return fSuccess;
}

// pszProtoPrefix is expected to have no trailing :
static void Opera6_AddTrustedProto(const char *pszProtoPrefix)
{
	TCHAR szIniFile[MAX_PATH],*ptszProtoPrefix;
	if (Opera6_GetIniFilePath(szIniFile)) {
		/* trusted protocols */
		ptszProtoPrefix=a2t(pszProtoPrefix);
		if (ptszProtoPrefix!=NULL) {
			WritePrivateProfileString(_T("Trusted Protocols"),ptszProtoPrefix,_T("1,0,"),szIniFile);
			mir_free(ptszProtoPrefix);
		}
	}
}

static void Opera6_AddKnownMimeType(const char *pszMimeType,const char *pszFileExt,const TCHAR *pszDescription)
{
	TCHAR szIniFile[MAX_PATH],szVal[256],*ptszMimeType;
	if (Opera6_GetIniFilePath(szIniFile)) {
		/* section version */
		if (GetPrivateProfileInt(_T("File Types Section Info"),_T("Version"),0,szIniFile)==2) {
			ptszMimeType=a2t(pszMimeType);
			if (ptszMimeType!=NULL) {
				/* file type */
				mir_sntprintf(szVal,SIZEOF(szVal),_T("4,,,,%.15hs,|%.128s (%.16hs)"),&pszFileExt[1],pszDescription,pszFileExt);
				WritePrivateProfileString(_T("File Types"),ptszMimeType,szVal,szIniFile);
				/* file type extension */
				WritePrivateProfileString(_T("File Types Extension"),ptszMimeType,_T(",0"),szIniFile);
				mir_free(ptszMimeType);
			}
		}
	}
}

/************************* Class **********************************/

/*
* Add a new file class to the class list.
* This either represents a superclass for several file extensions or
* the the url object.
* Urls just need a class named after their prefix e.g. "http".
* File extensions should follow the rule "appname.extension". 
*/

// pszIconLoc, pszVerbDesc and pszDdeCmd are allowed to be NULL
// call GetLastError() on error to get more error details
BOOL AddRegClass(const char *pszClassName,const TCHAR *pszTypeDescription,const TCHAR *pszIconLoc,const TCHAR *pszAppName,const TCHAR *pszRunCmd,const TCHAR *pszDdeCmd,const TCHAR *pszDdeApp,const TCHAR *pszDdeTopic,const TCHAR *pszVerbDesc,BOOL fBrowserAutoOpen,BOOL fUrlProto,BOOL fIsShortcut)
{
	LONG res;
	HKEY hRootKey,hClassKey,hShellKey,hVerbKey,hDdeKey;

	/* some error checking for disallowed values (to avoid errors in registry) */
	if (strchr(pszClassName,'\\')!=NULL || strchr(pszClassName,' ')!=NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	/* try to open interactive user's classes key */
	if (RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Classes"),0,KEY_CREATE_SUB_KEY,&hRootKey))
		hRootKey=HKEY_CLASSES_ROOT; /* might be write protected by security settings */

	/* class */
	if ((res=RegCreateKeyExA(hRootKey,pszClassName,0,NULL,0,KEY_SET_VALUE|KEY_CREATE_SUB_KEY|DELETE|KEY_QUERY_VALUE,NULL,&hClassKey,NULL))==ERROR_SUCCESS) {
		/* backup class if shared */
		if (fUrlProto) BackupRegTree(hRootKey,pszClassName,"bak_");
		/* type description */
		if (fUrlProto) SetRegStrPrefixValue(hClassKey,_T("URL:"),pszTypeDescription);
		else RegSetValueEx(hClassKey,NULL,0,REG_SZ,(BYTE*)pszTypeDescription,(mir_tstrlen(pszTypeDescription)+1)*sizeof(TCHAR));
		/* default icon */
		if (pszIconLoc!=NULL) SetRegSubKeyStrDefValue(hClassKey,_T("DefaultIcon"),pszIconLoc);
		/* url protocol */
		if (!fUrlProto) RegDeleteValue(hClassKey,_T("URL Protocol"));
		else RegSetValueEx(hClassKey,_T("URL Protocol"),0,REG_SZ,NULL,0);
		/* moniker clsid */
		RegDeleteKey(hClassKey,_T("CLSID"));
		/* edit flags */
		{	DWORD dwFlags=0,dwSize=sizeof(dwFlags);
		RegQueryValueEx(hClassKey,_T("EditFlags"),NULL,NULL,(BYTE*)&dwFlags,&dwSize);
		if (fBrowserAutoOpen) dwFlags=(dwFlags&~FTA_AlwaysUnsafe)|FTA_OpenIsSafe;
		if (!fUrlProto) dwFlags|=FTA_HasExtension;
		else dwFlags=(dwFlags&~FTA_HasExtension)|FTA_Show; /* show classes without extension */
		RegSetValueEx(hClassKey,_T("EditFlags"),0,REG_DWORD,(BYTE*)&dwFlags,sizeof(dwFlags));
		}
		if (fIsShortcut) {
			RegSetValueExA(hClassKey,"IsShortcut",0,REG_SZ,NULL,0);
			RegSetValueExA(hClassKey,"NeverShowExt",0,REG_SZ,NULL,0);
		}
		/* shell */
		if ((res=RegCreateKeyEx(hClassKey,_T("shell"),0,NULL,0,KEY_SET_VALUE|KEY_CREATE_SUB_KEY,NULL,&hShellKey,NULL))==ERROR_SUCCESS) {
			/* default verb (when empty "open" is used) */
			RegSetValueEx(hShellKey,NULL,0,REG_SZ,(BYTE*)_T("open"),5*sizeof(TCHAR));
			/* verb */
			if ((res=RegCreateKeyEx(hShellKey,_T("open"),0,NULL,0,KEY_SET_VALUE|KEY_CREATE_SUB_KEY|DELETE,NULL,&hVerbKey,NULL))==ERROR_SUCCESS) {
				/* verb description */
				if (pszVerbDesc==NULL) RegDeleteValue(hVerbKey,NULL);
				else RegSetValueEx(hVerbKey,NULL,0,REG_SZ,(BYTE*)pszVerbDesc,(mir_tstrlen(pszVerbDesc)+1)*sizeof(TCHAR));
				/* friendly appname (mui string) */
				RegSetValueEx(hVerbKey,_T("FriendlyAppName"),0,REG_SZ,(BYTE*)pszAppName,(mir_tstrlen(pszAppName)+1)*sizeof(TCHAR));
				/* command */
				SetRegSubKeyStrDefValue(hVerbKey,_T("command"),pszRunCmd);
				/* ddeexec */
				if (pszDdeCmd!=NULL) {
					if (!RegCreateKeyEx(hVerbKey,_T("ddeexec"),0,NULL,0,KEY_SET_VALUE|KEY_CREATE_SUB_KEY|DELETE,NULL,&hDdeKey,NULL)) {
						/* command */
						RegSetValueEx(hDdeKey,NULL,0,REG_SZ,(BYTE*)pszDdeCmd,(mir_tstrlen(pszDdeCmd)+1)*sizeof(TCHAR));
						/* application */
						SetRegSubKeyStrDefValue(hDdeKey,_T("application"),pszDdeApp);
						/* topic */
						SetRegSubKeyStrDefValue(hDdeKey,_T("topic"),pszDdeTopic);
						/* ifexec */
						RegDeleteKey(hDdeKey,_T("ifexec"));
						RegCloseKey(hDdeKey);
					}
				} else {
					if (!RegOpenKeyEx(hVerbKey,_T("ddeexec"),0,DELETE,&hDdeKey)) {
						/* application */
						RegDeleteKey(hDdeKey,_T("application"));
						/* topic */
						RegDeleteKey(hDdeKey,_T("topic"));
						/* ifexec */
						RegDeleteKey(hDdeKey,_T("ifexec"));
						RegCloseKey(hDdeKey);
					}
					RegDeleteKey(hVerbKey,_T("ddeexec"));
				}
				/* drop target (WinXP+) */
				RegDeleteKey(hVerbKey,_T("DropTarget"));
				RegCloseKey(hVerbKey);
			}
			RegCloseKey(hShellKey);
			/* Opera support */
			if (fUrlProto) Opera6_AddTrustedProto(pszClassName);
		}
		RegCloseKey(hClassKey);
	}

	if (hRootKey!=HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);

	if (res) SetLastError(res);
	return !res;
}

BOOL RemoveRegClass(const char *pszClassName)
{
	LONG res;
	HKEY hRootKey,hClassKey,hShellKey,hVerbKey;
	TCHAR *ptszClassName,*ptszPrevRunCmd;

	/* try to open interactive user's classes key */
	if (RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Classes"),0,DELETE,&hRootKey))
		hRootKey=HKEY_CLASSES_ROOT;

	/* class name */
	ptszClassName=a2t(pszClassName);
	if (ptszClassName!=NULL)
		res=DeleteRegSubTree(hRootKey,ptszClassName);
	else res=ERROR_OUTOFMEMORY;
	mir_free(ptszClassName); /* does NULL check */

	/* backup only saved/restored for fUrlProto */
	if (!res) {
		if ((res=RestoreRegTree(hRootKey,pszClassName,"bak_"))==ERROR_SUCCESS)
			/* class */
				if (!RegOpenKeyExA(hRootKey,pszClassName,0,KEY_QUERY_VALUE,&hClassKey)) {
					/* shell */
					if (!RegOpenKeyEx(hClassKey,_T("shell"),0,KEY_QUERY_VALUE,&hShellKey)) {
						/* verb */
						if (!RegOpenKeyEx(hShellKey,_T("open"),0,KEY_QUERY_VALUE,&hVerbKey)) {
							/* command */
							ptszPrevRunCmd=GetRegStrValue(hVerbKey,_T("command"));
							if (ptszPrevRunCmd!=NULL && !IsValidRunCommand(ptszPrevRunCmd))
								res=DeleteRegSubTree(hRootKey,ptszClassName); /* backup outdated, remove all */
							mir_free(ptszPrevRunCmd); /* does NULL check */
							RegCloseKey(hVerbKey);
						}
						RegCloseKey(hShellKey);
					}
					RegCloseKey(hClassKey);
				}
	} else DeleteRegTreeBackup(pszClassName,"bak_");

	if (hRootKey!=HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);

	if (res==ERROR_SUCCESS || res==ERROR_FILE_NOT_FOUND || res==ERROR_NO_MORE_ITEMS) return TRUE;
	SetLastError(res);
	return FALSE;
}

/*
* Test if a given class belongs to the current process
* specified via its run command.
* This is especially needed for Urls where the same class name "http" can be
* registered and thus be overwritten by multiple applications.
*/

BOOL IsRegClass(const char *pszClassName,const TCHAR *pszRunCmd)
{
	BOOL fSuccess=FALSE;
	HKEY hClassKey,hShellKey,hVerbKey,hCmdKey;

	/* using the merged view classes key for reading */
	/* class */
	if (!RegOpenKeyExA(HKEY_CLASSES_ROOT,pszClassName,0,KEY_QUERY_VALUE,&hClassKey)) {
		/* shell */
		if (!RegOpenKeyEx(hClassKey,_T("shell"),0,KEY_QUERY_VALUE,&hShellKey)) {
			/* verb */
			if (!RegOpenKeyEx(hShellKey,_T("open"),0,KEY_QUERY_VALUE,&hVerbKey)) {
				/* command */
				if (!RegOpenKeyEx(hVerbKey,_T("command"),0,KEY_QUERY_VALUE,&hCmdKey)) {
					/* it is enough to check if the command is right */
					fSuccess=IsRegStrValue(hCmdKey,NULL,pszRunCmd);
					RegCloseKey(hCmdKey);
				}
				RegCloseKey(hVerbKey);
			}
			RegCloseKey(hShellKey);
		}
		RegCloseKey(hClassKey);
	}
	return fSuccess;
}

/*
* Extract the icon name of the class from the registry and load it.
* For uses especially with url classes.
*/

// DestroyIcon() the return value
HICON LoadRegClassSmallIcon(const char *pszClassName)
{
	HICON hIcon=NULL;
	HKEY hClassKey,hIconKey;
	TCHAR *pszIconLoc,*p;

	/* using the merged view classes key for reading */
	/* class */
	if (!RegOpenKeyExA(HKEY_CLASSES_ROOT,pszClassName,0,KEY_QUERY_VALUE,&hClassKey)) {
		/* default icon */
		if (!RegOpenKeyEx(hClassKey,_T("DefaultIcon"),0,KEY_QUERY_VALUE,&hIconKey)) {
			/* extract icon */
			pszIconLoc=GetRegStrValue(hIconKey,NULL);
			if (pszIconLoc!=NULL) {
				p=_tcsrchr(pszIconLoc,_T(','));
				if (p!=NULL) {
					*(p++)=0;
					ExtractIconEx(pszIconLoc,_ttoi(p),NULL,&hIcon,1);
				}
				mir_free(pszIconLoc);
			}
			RegCloseKey(hIconKey);
		}
		RegCloseKey(hClassKey);
	}

	return hIcon;
}

/************************* Extension ******************************/

/*
* Add a new file extension to the class list.
* The file extension needs to be associated with a class
* that has been registered previously.
* Multiple file extensions can be assigned to the same class.
* The class contains most settings as the run command etc.
*/

// pszMimeType is allowed to be NULL
BOOL AddRegFileExt(const char *pszFileExt,const char *pszClassName,const char *pszMimeType,BOOL fIsText)
{
	BOOL fSuccess=FALSE;
	HKEY hRootKey,hExtKey,hOpenWithKey;

	/* some error checking for disallowed values (to avoid errors in registry) */
	if (strchr(pszFileExt,'\\')!=NULL || strchr(pszFileExt,' ')!=NULL)
		return FALSE;

	/* try to open interactive user's classes key */
	if (RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Classes"),0,KEY_CREATE_SUB_KEY,&hRootKey))
		hRootKey=HKEY_CLASSES_ROOT;

	/* file ext */
	if (!RegCreateKeyExA(hRootKey,pszFileExt,0,NULL,0,KEY_SET_VALUE|KEY_QUERY_VALUE|KEY_CREATE_SUB_KEY,NULL,&hExtKey,NULL)) {
		/* backup previous app */
		BackupRegTree(hRootKey,pszFileExt,"bak_");
		/* remove any no-open flag */
		RegDeleteValue(hExtKey,_T("NoOpen"));
		/* open with progids */
		TCHAR *pszPrevClass=GetRegStrValue(hExtKey,NULL);
		if (pszPrevClass!=NULL && !IsRegStrValueA(hExtKey,NULL,pszClassName))
			if (!RegCreateKeyEx(hExtKey,_T("OpenWithProgids"),0,NULL,0,KEY_SET_VALUE,NULL,&hOpenWithKey,NULL)) {
				/* previous class (backup) */
				RegSetValueEx(hOpenWithKey,pszPrevClass,0,REG_NONE,NULL,0);
				RegCloseKey(hOpenWithKey);
			}
			mir_free(pszPrevClass); /* does NULL check */
			/* class name */
			fSuccess=!RegSetValueExA(hExtKey,NULL,0,REG_SZ,(BYTE*)pszClassName,mir_strlen(pszClassName)+1);
			/* mime type e.g. "application/x-icq" */
			if (pszMimeType!=NULL) RegSetValueExA(hExtKey,"Content Type",0,REG_SZ,(BYTE*)pszMimeType,mir_strlen(pszMimeType)+1);
			/* perceived type e.g. text (WinXP+) */
			if (fIsText) RegSetValueEx(hExtKey,_T("PerceivedType"),0,REG_SZ,(BYTE*)_T("text"),5*sizeof(TCHAR));
			RegCloseKey(hExtKey);
	}

	if (hRootKey!=HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
	return fSuccess;
}

void RemoveRegFileExt(const char *pszFileExt,const char *pszClassName)
{
	HKEY hRootKey,hExtKey,hSubKey;
	DWORD nOpenWithCount;
	TCHAR *pszPrevClassName=NULL;
	BOOL fRestored=FALSE;

	/* try to open interactive user's classes key */
	if (RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Classes"),0,DELETE,&hRootKey))
		hRootKey=HKEY_CLASSES_ROOT;

	/* file ext */
	if (!RegOpenKeyExA(hRootKey,pszFileExt,0,KEY_QUERY_VALUE|KEY_SET_VALUE|DELETE,&hExtKey)) {
		/* class name (the important part) */
		if (!RestoreRegTree(hRootKey,pszFileExt,"bak_")) {
			pszPrevClassName=GetRegStrValue(hExtKey,NULL);
			if (pszPrevClassName!=NULL) {
				/* previous class name still exists? */
				/* using the merged view classes key for reading */
				if (!RegOpenKeyEx(HKEY_CLASSES_ROOT,pszPrevClassName,0,KEY_QUERY_VALUE,&hSubKey)) {
					fRestored=TRUE;
					RegCloseKey(hSubKey);
				} else RegDeleteValue(hExtKey,NULL);
				mir_free(pszPrevClassName);
			}
		}
		if (pszPrevClassName==NULL) RegDeleteValue(hExtKey,NULL);
		/* open with progids (remove if empty) */
		nOpenWithCount=0;
		if (!RegOpenKeyEx(hExtKey,_T("OpenWithProgids"),0,KEY_SET_VALUE|KEY_QUERY_VALUE,&hSubKey)) {
			/* remove current class (if set by another app) */
			RegDeleteValueA(hSubKey,pszClassName);
			RegQueryInfoKey(hSubKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&nOpenWithCount,NULL,NULL,NULL);
			RegCloseKey(hSubKey);
		}
		if (!nOpenWithCount) RegDeleteKey(hExtKey,_T("OpenWithProgids")); /* delete if no values */
		RegCloseKey(hExtKey);
	} else DeleteRegTreeBackup(pszFileExt,"bak_");
	if (!fRestored) RegDeleteKeyA(hRootKey,pszFileExt); /* try to remove it all */

	if (hRootKey!=HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
}

/*
* Test if a given file extension belongs to the given class name.
* If it does not belong to the class name, it got reassigned and thus
* overwritten by another application.
*/

BOOL IsRegFileExt(const char *pszFileExt,const char *pszClassName)
{
	BOOL fSuccess=FALSE;
	HKEY hExtKey;

	/* using the merged view classes key for reading */
	/* file ext */
	if (!RegOpenKeyExA(HKEY_CLASSES_ROOT,pszFileExt,0,KEY_QUERY_VALUE,&hExtKey)) {
		/* class name */
		/* it is enough to check if the class is right */
		fSuccess=IsRegStrValueA(hExtKey,NULL,pszClassName);
		RegCloseKey(hExtKey);
	}
	return fSuccess;
}

/************************* Mime Type ******************************/

/*
* Add a given mime type to the global mime database.
*/

// returns TRUE if the mime type was not yet registered on the system,
// it needs to be removed when the file extension gets removed
BOOL AddRegMimeType(const char *pszMimeType,const char *pszFileExt,const TCHAR *pszDescription)
{
	BOOL fSuccess=FALSE;
	HKEY hRootKey,hDbKey,hTypeKey;

	/* some error checking for disallowed values (to avoid errors in registry) */
	if (strchr(pszMimeType,'\\')!=NULL || strchr(pszMimeType,' ')!=NULL)
		return FALSE;

	/* try to open interactive user's classes key */
	if (RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Classes"),0,KEY_QUERY_VALUE,&hRootKey))
		hRootKey=HKEY_CLASSES_ROOT;

	/* database */
	if (!RegOpenKeyEx(hRootKey,_T("MIME\\Database\\Content Type"),0,KEY_CREATE_SUB_KEY,&hDbKey)) {
		/* mime type */
		if (!RegCreateKeyExA(hDbKey,pszMimeType,0,NULL,0,KEY_QUERY_VALUE|KEY_SET_VALUE,NULL,&hTypeKey,NULL)) {
			/* file ext */
			if (RegQueryValueExA(hTypeKey,"Extension",NULL,NULL,NULL,NULL)) /* only set if not present */
				fSuccess=!RegSetValueExA(hTypeKey,"Extension",0,REG_SZ,(BYTE*)pszFileExt,mir_strlen(pszFileExt)+1);
			RegCloseKey(hTypeKey);
			/* Opera support */
			Opera6_AddKnownMimeType(pszMimeType,pszFileExt,pszDescription);
		}
		RegCloseKey(hDbKey);
	}

	if (hRootKey!=HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
	return fSuccess;
}

void RemoveRegMimeType(const char *pszMimeType,const char *pszFileExt)
{
	HKEY hRootKey,hDbKey,hTypeKey;
	BOOL fDelete=TRUE;

	/* try to open interactive user's classes key */
	if (RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Classes"),0,KEY_QUERY_VALUE,&hRootKey))
		hRootKey=HKEY_CLASSES_ROOT;

	/* database */
	if (!RegOpenKeyEx(hRootKey,_T("MIME\\Database\\Content Type"),0,DELETE,&hDbKey)) {
		/* mime type */
		if (!RegOpenKeyExA(hDbKey,pszMimeType,0,KEY_QUERY_VALUE,&hTypeKey)) {
			/* file ext */
			fDelete=IsRegStrValueA(hTypeKey,_T("Extension"),pszFileExt);
			RegCloseKey(hTypeKey);
		}
		if (fDelete) RegDeleteKeyA(hDbKey,pszMimeType);
		RegCloseKey(hDbKey);
	}

	if (hRootKey!=HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
}

/************************* Open-With App **************************/

/*
* Add Miranda as an option to the advanced "Open With..." dialog.
*/

// pszDdeCmd is allowed to be NULL
void AddRegOpenWith(const TCHAR *pszAppFileName,BOOL fAllowOpenWith,const TCHAR *pszAppName,const TCHAR *pszIconLoc,const TCHAR *pszRunCmd,const TCHAR *pszDdeCmd,const TCHAR *pszDdeApp,const TCHAR *pszDdeTopic)
{
	HKEY hRootKey,hAppsKey,hExeKey,hShellKey,hVerbKey,hDdeKey;

	/* try to open interactive user's classes key */
	if (RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Classes"),0,KEY_QUERY_VALUE,&hRootKey))
		hRootKey=HKEY_CLASSES_ROOT;

	/* database */
	if (!RegCreateKeyEx(hRootKey,_T("Applications"),0,NULL,0,KEY_CREATE_SUB_KEY,NULL,&hAppsKey,NULL)) {
		/* filename */
		if (!RegCreateKeyEx(hAppsKey,pszAppFileName,0,NULL,0,KEY_SET_VALUE|KEY_CREATE_SUB_KEY,NULL,&hExeKey,NULL)) {
			/* appname */
			RegSetValueEx(hExeKey,NULL,0,REG_SZ,(BYTE*)pszAppName,(mir_tstrlen(pszAppName)+1)*sizeof(TCHAR));
			/* no open-with flag */
			if (fAllowOpenWith) RegDeleteValue(hExeKey,_T("NoOpenWith"));
			else RegSetValueEx(hExeKey,_T("NoOpenWith"),0,REG_SZ,NULL,0);
			/* default icon */
			if (pszIconLoc!=NULL) SetRegSubKeyStrDefValue(hExeKey,_T("DefaultIcon"),pszIconLoc);
			/* shell */
			if (!RegCreateKeyEx(hExeKey,_T("shell"),0,NULL,0,KEY_SET_VALUE|KEY_CREATE_SUB_KEY,NULL,&hShellKey,NULL)) {
				/* default verb (when empty "open" is used) */
				RegSetValueEx(hShellKey,NULL,0,REG_SZ,(BYTE*)_T("open"),5*sizeof(TCHAR));
				/* verb */
				if (!RegCreateKeyEx(hShellKey,_T("open"),0,NULL,0,KEY_SET_VALUE|KEY_CREATE_SUB_KEY,NULL,&hVerbKey,NULL)) {
					/* friendly appname (mui string) */
					RegSetValueEx(hVerbKey,_T("FriendlyAppName"),0,REG_SZ,(BYTE*)pszAppName,(mir_tstrlen(pszAppName)+1)*sizeof(TCHAR));
					/* command */
					SetRegSubKeyStrDefValue(hVerbKey,_T("command"),pszRunCmd);
					/* ddeexec */
					if (pszDdeCmd!=NULL)
						if (!RegCreateKeyEx(hVerbKey,_T("ddeexec"),0,NULL,0,KEY_SET_VALUE|KEY_CREATE_SUB_KEY,NULL,&hDdeKey,NULL)) {
							/* command */
							RegSetValueEx(hDdeKey,NULL,0,REG_SZ,(BYTE*)pszDdeCmd,(mir_tstrlen(pszDdeCmd)+1)*sizeof(TCHAR));
							/* application */
							SetRegSubKeyStrDefValue(hDdeKey,_T("application"),pszDdeApp);
							/* topic */
							SetRegSubKeyStrDefValue(hDdeKey,_T("topic"),pszDdeTopic);
							RegCloseKey(hDdeKey);
						}
						RegCloseKey(hVerbKey);
				}
				RegCloseKey(hShellKey);
			}
			RegCloseKey(hExeKey);
		}
		RegCloseKey(hAppsKey);
	}

	if (hRootKey!=HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
}

void RemoveRegOpenWith(const TCHAR *pszAppFileName)
{
	HKEY hRootKey,hAppsKey,hExeKey,hShellKey,hVerbKey,hDdeKey;

	/* try to open interactive user's classes key */
	if (RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Classes"),0,KEY_QUERY_VALUE,&hRootKey))
		hRootKey=HKEY_CLASSES_ROOT;

	/* applications */
	if (!RegOpenKeyEx(hRootKey,_T("Applications"),0,DELETE,&hAppsKey)) {
		/* filename */
		if (!RegOpenKeyEx(hAppsKey,pszAppFileName,0,DELETE,&hExeKey)) {
			/* default icon */
			RegDeleteKey(hExeKey,_T("DefaultIcon"));
			/* shell */
			if (!RegOpenKeyEx(hExeKey,_T("shell"),0,DELETE,&hShellKey)) {
				/* verb */
				if (!RegOpenKeyEx(hShellKey,_T("open"),0,DELETE,&hVerbKey)) {
					/* command */
					RegDeleteKey(hVerbKey,_T("command"));
					/* ddeexec */
					if (!RegOpenKeyEx(hVerbKey,_T("ddeexec"),0,DELETE,&hDdeKey)) {
						/* application */
						RegDeleteKey(hDdeKey,_T("application"));
						/* topic */
						RegDeleteKey(hDdeKey,_T("topic"));
						RegCloseKey(hDdeKey);
					}
					RegDeleteKey(hVerbKey,_T("ddeexec"));
					RegCloseKey(hVerbKey);
				}
				RegDeleteKey(hShellKey,_T("open"));
				RegCloseKey(hShellKey);
			}
			RegDeleteKey(hExeKey,_T("shell"));
			/* supported types */
			RegDeleteKey(hExeKey,_T("SupportedTypes"));
			RegCloseKey(hExeKey);
		}
		RegDeleteKey(hAppsKey,pszAppFileName);
		RegCloseKey(hAppsKey);
	}

	if (hRootKey!=HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
}

/*
* Tell the "Open With..." dialog we support a given file extension.
*/

void AddRegOpenWithExtEntry(const TCHAR *pszAppFileName,const char *pszFileExt,const TCHAR *pszFileDesc)
{
	HKEY hRootKey,hAppsKey,hExeKey,hTypesKey;

	/* try to open interactive user's classes key */
	if (RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Classes"),0,KEY_QUERY_VALUE,&hRootKey))
		hRootKey=HKEY_CLASSES_ROOT;

	/* applications */
	if (!RegOpenKeyEx(hRootKey,_T("Applications"),0,KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS,&hAppsKey)) {
		/* filename */
		if (!RegOpenKeyEx(hAppsKey,pszAppFileName,0,KEY_CREATE_SUB_KEY,&hExeKey)) {
			/* supported types */
			if (!RegCreateKeyEx(hExeKey,_T("SupportedTypes"),0,NULL,0,KEY_SET_VALUE,NULL,&hTypesKey,NULL)) {	
				TCHAR *ptszFileExt;
				ptszFileExt=a2t(pszFileExt);
				if (ptszFileExt!=NULL)
					RegSetValueEx(hTypesKey,ptszFileExt,0,REG_SZ,(BYTE*)pszFileDesc,(mir_tstrlen(pszFileDesc)+1)*sizeof(TCHAR));
				mir_free(ptszFileExt); /* does NULL check */
				RegCloseKey(hTypesKey);
			}
			RegCloseKey(hExeKey);
		}
		RegCloseKey(hAppsKey);
	}

	if (hRootKey!=HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
}

void RemoveRegOpenWithExtEntry(const TCHAR *pszAppFileName,const char *pszFileExt)
{
	HKEY hRootKey,hAppsKey,hExeKey,hTypesKey;

	/* try to open interactive user's classes key */
	if (RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Classes"),0,KEY_QUERY_VALUE,&hRootKey))
		hRootKey=HKEY_CLASSES_ROOT;

	/* applications */
	if (!RegOpenKeyEx(hRootKey,_T("Applications"),0,KEY_QUERY_VALUE,&hAppsKey)) {
		/* filename */
		if (!RegOpenKeyEx(hAppsKey,pszAppFileName,0,KEY_QUERY_VALUE,&hExeKey)) {
			/* supported types */
			if (!RegOpenKeyEx(hExeKey,_T("SupportedTypes"),0,KEY_SET_VALUE,&hTypesKey)) {	
				RegDeleteValueA(hTypesKey,pszFileExt);
				RegCloseKey(hTypesKey);
			}
			RegCloseKey(hExeKey);
		}
		RegCloseKey(hAppsKey);
	}

	if (hRootKey!=HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
}

/************************* Autostart ******************************/

/*
* Add Miranda to the autostart list in the registry.
*/

BOOL AddRegRunEntry(const TCHAR *pszAppName,const TCHAR *pszRunCmd)
{
	BOOL fSuccess=FALSE;
	HKEY hRunKey;

	/* run */
	if (!RegCreateKeyEx(HKEY_CURRENT_USER,_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),0,NULL,0,KEY_SET_VALUE,NULL,&hRunKey,NULL)) {
		/* appname */
		fSuccess=!RegSetValueEx(hRunKey,pszAppName,0,REG_SZ,(BYTE*)pszRunCmd,(mir_tstrlen(pszRunCmd)+1)*sizeof(TCHAR));
		RegCloseKey(hRunKey);
	}
	return fSuccess;
}

BOOL RemoveRegRunEntry(const TCHAR *pszAppName,const TCHAR *pszRunCmd)
{
	HKEY hRunKey;

	/* run */
	LONG res = RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),0,KEY_QUERY_VALUE|KEY_SET_VALUE,&hRunKey);
	if (!res) {
		/* appname */
		if (IsRegStrValue(hRunKey,pszAppName,pszRunCmd))
			res=RegDeleteValue(hRunKey,pszAppName); /* only remove if current */
		RegCloseKey(hRunKey);
	}
	return res==ERROR_SUCCESS || res==ERROR_FILE_NOT_FOUND;
}

/*
* Check if the autostart item belongs to the current instance of Miranda.
*/

BOOL IsRegRunEntry(const TCHAR *pszAppName,const TCHAR *pszRunCmd)
{
	BOOL fState=FALSE;
	HKEY hRunKey;

	/* Run */
	if (!RegOpenKeyEx(HKEY_CURRENT_USER,_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),0,KEY_QUERY_VALUE,&hRunKey)) {
		/* appname */
		fState=IsRegStrValue(hRunKey,pszAppName,pszRunCmd);
		RegCloseKey(hRunKey);
	}
	return fState;
}
