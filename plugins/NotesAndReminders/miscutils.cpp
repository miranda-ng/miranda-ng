#include "globals.h"

BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
HANDLE (WINAPI *MyMonitorFromWindow)(HWND,DWORD);
BOOL (WINAPI *MyTzSpecificLocalTimeToSystemTime)(LPTIME_ZONE_INFORMATION,LPSYSTEMTIME,LPSYSTEMTIME);
BOOL (WINAPI *MySystemTimeToTzSpecificLocalTime)(LPTIME_ZONE_INFORMATION,LPSYSTEMTIME,LPSYSTEMTIME);


WORD ConvertHotKeyToControl(WORD HK)
{
  WORD R = 0;
  if ((HK & MOD_CONTROL) == MOD_CONTROL) R = R | HOTKEYF_CONTROL;
  if ((HK & MOD_ALT) == MOD_ALT) R = R | HOTKEYF_ALT;
  if ((HK & MOD_SHIFT) == MOD_SHIFT) R = R | HOTKEYF_SHIFT;
  return R;
}

WORD ConvertControlToHotKey(WORD HK)
{
  WORD R = 0;
  if ((HK & HOTKEYF_CONTROL) == HOTKEYF_CONTROL) R = R | MOD_CONTROL;
  if ((HK & HOTKEYF_ALT) == HOTKEYF_ALT) R = R | MOD_ALT;
  if ((HK & HOTKEYF_SHIFT) == HOTKEYF_SHIFT) R = R | MOD_SHIFT;
  return R;
}

void WriteSettingInt(HANDLE hContact,char *ModuleName,char *SettingName,int Value)
{
	DBCONTACTWRITESETTING cws = {0};
	DBVARIANT dbv = {0};
	dbv.type = DBVT_DWORD;
    dbv.dVal = Value;
	cws.szModule = ModuleName;
    cws.szSetting = SettingName;
    cws.value = dbv;
    CallService(MS_DB_CONTACT_WRITESETTING,(DWORD)hContact,(DWORD)&cws);
}

int ReadSettingInt(HANDLE hContact,char *ModuleName,char *SettingName,int Default)
{
	DBCONTACTGETSETTING cws = {0};
	DBVARIANT dbv = {0};
	dbv.type = DBVT_DWORD;
    dbv.dVal = Default;
	cws.szModule = ModuleName;
    cws.szSetting = SettingName;
	cws.pValue = &dbv;
    if (CallService(MS_DB_CONTACT_GETSETTING,(DWORD)hContact,(DWORD)&cws)) 
		return Default;
    else 
		return dbv.dVal;
}

void DeleteSetting(HANDLE hContact,char *ModuleName,char *SettingName)
{
	DBCONTACTGETSETTING dbcgs = {0};
	dbcgs.szModule = ModuleName;
	dbcgs.szSetting = SettingName;
	dbcgs.pValue = NULL;
	CallService(MS_DB_CONTACT_DELETESETTING,(DWORD)hContact,(DWORD)&dbcgs);
}

void FreeSettingBlob(WORD pSize,void * pbBlob)
{
	DBVARIANT dbv = {0};
	dbv.type = DBVT_BLOB;
	dbv.cpbVal = pSize;
	dbv.pbVal = (BYTE*)pbBlob;
	CallService(MS_DB_CONTACT_FREEVARIANT,0,(DWORD)&dbv);
}

void WriteSettingBlob(HANDLE hContact,char *ModuleName,char *SettingName,WORD pSize,void *pbBlob)
{
	DBCONTACTWRITESETTING cgs = {0};
	DBVARIANT dbv = {0};
	dbv.type = DBVT_BLOB;
	dbv.cpbVal = pSize;
	dbv.pbVal = (BYTE*)pbBlob;
	cgs.szModule = ModuleName;
	cgs.szSetting = SettingName;
	cgs.value = dbv;
	CallService(MS_DB_CONTACT_WRITESETTING,(DWORD)hContact,(DWORD)&cgs);
}

void ReadSettingBlob(HANDLE hContact, char *ModuleName, char *SettingName, WORD *pSize, void **pbBlob)
{
	DBCONTACTGETSETTING cgs = {0};
	DBVARIANT dbv = {0};
	dbv.type = DBVT_BLOB;
//	dbv.cpbVal = (LOBYTE(pSize) | HIBYTE((BYTE)pbBlob)); // this is not used anyway
	cgs.szModule = ModuleName;
	cgs.szSetting = SettingName;
	cgs.pValue = &dbv;
	if (CallService(MS_DB_CONTACT_GETSETTING,(DWORD)hContact,(DWORD)&cgs)) 
	{
        pSize = 0; // tohle asi ve 4.2 neni
        pbBlob = NULL;
	}
    else 
	{
		*pSize = LOWORD(dbv.cpbVal);
		*pbBlob = (int*)dbv.pbVal;
	}
}

void WriteSettingIntArray(HANDLE hContact,char *ModuleName,char *SettingName,const int *Value, int Size)
{
	WriteSettingBlob(hContact,ModuleName,SettingName,sizeof(int)*Size,(void*)Value);
}

BOOL ReadSettingIntArray(HANDLE hContact,char *ModuleName,char *SettingName,int *Value, int Size)
{
	WORD sz = 4096;
	int *pData;
	BOOL bResult;

	ReadSettingBlob(hContact, ModuleName, SettingName, &sz, (void**)&pData);

	if (!pData)
		return FALSE;

	bResult = FALSE;

	if (sz == sizeof(int)*Size)
	{
		memcpy(Value, pData, sizeof(int)*Size);
		bResult = TRUE;
	}

	FreeSettingBlob(sz,pData);

	return bResult;
}

void TreeAdd(TREEELEMENT **root, void *Data)
{
	TREEELEMENT *NTE;
	NTE = (TREEELEMENT*)malloc(sizeof(TREEELEMENT));
	if (NTE)
	{
		NTE->ptrdata = Data;
		NTE->next = *root;
		*root = NTE;
	}
}

void TreeAddSorted(TREEELEMENT **root,void *Data,int (*CompareCb)(TREEELEMENT*,TREEELEMENT*))
{
	TREEELEMENT *TTE,*Prev;
	TREEELEMENT *NTE;

	if (!*root)
	{
		// list empty, just add normally
		TreeAdd(root,Data);
		return;
	}

	NTE = (TREEELEMENT*)malloc(sizeof(TREEELEMENT));
	if (!NTE)
		return;

	NTE->ptrdata = Data;
	NTE->next = NULL;

	// insert sorted

	Prev = NULL;
	TTE = *root;

	while (TTE)
	{
		if (CompareCb(NTE, TTE) < 0)
		{
			if (Prev)
			{
				Prev->next = NTE;
				NTE->next = TTE;
			}
			else
			{
				// first in list
				NTE->next = TTE;
				*root = NTE;
			}
			return;
		}

		Prev = TTE;
		TTE = (TREEELEMENT*)TTE->next;
	}

	// add last
	Prev->next = NTE;
}

void TreeDelete(TREEELEMENT **root,void *Item)
{
	TREEELEMENT *TTE,*Prev = NULL;
	TTE = *root;
	if (!TTE) return;
	while ((TTE) && (TTE->ptrdata != Item)) 
	{
		Prev = TTE;
		TTE = (TREEELEMENT*)TTE->next;
	}
	if (TTE)
	{
		if (Prev)
			Prev->next = TTE->next;
		else 
			*root = (TREEELEMENT*)TTE->next;
		SAFE_FREE((void**)&TTE);
	}
}

void *TreeGetAt(TREEELEMENT *root,int Item)
{
	TREEELEMENT *TTE;
	int I = 0;
	if (!root) return NULL;
	TTE = root;
	while ((TTE) && (I != Item))
	{
		TTE = (TREEELEMENT*)TTE->next;
		I++;
	}
	if (!TTE) 
		return NULL;
	else
		return TTE->ptrdata;
}

int TreeGetCount(TREEELEMENT *root)
{
	int I = 0;
	TREEELEMENT *TTE;
	if (!root) return 0;
	TTE = root;
	while (TTE)
	{
		TTE = (TREEELEMENT*)TTE->next;
		I++;
	}
	return I;
}
