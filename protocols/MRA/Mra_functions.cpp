#include "Mra.h"



typedef struct SetXStatusData_s {
	DWORD dwXStatus;
	DWORD dwCountdown;
	HICON hDlgIcon;
}SetXStatusData;


typedef struct
{
	HANDLE hContact;
}SetBlogStatusData;




#define RECURSION_DATA_STACK_ITEMS_MIN 128
struct RECURSION_DATA_STACK_ITEM
{
	HANDLE hFind;
	DWORD dwFileNameLen;
	WIN32_FIND_DATA w32fdFindFileData;
};


typedef struct _MRA_APC_QUEUE_ITEM
{
	FIFO_MT_ITEM ffmtItem;
	PAPCFUNC pfnAPC;
}MRA_APC_QUEUE_ITEM;




void					MraAPCQueueProcess			(PFIFO_MT pffmtAPCQueue);
void CALLBACK			ThreadMarandaCallbackAck	(ULONG_PTR dwParam);
LRESULT CALLBACK		MessageEditSubclassProc		(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK		SetXStatusDlgProc			(HWND hWndDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK		SendReplyBlogStatusDlgProc	(HWND hWndDlg,UINT message,WPARAM wParam,LPARAM lParam);



DWORD MraGetSelfVersionString(LPSTR lpszSelfVersion,SIZE_T dwSelfVersionSize,SIZE_T *pdwSelfVersionSizeRet)
{
	DWORD dwRetErrorCode;

	if (lpszSelfVersion && dwSelfVersionSize)
	{
		DWORD dwMirVer=CallService(MS_SYSTEM_GETVERSION,0,0);
		LPSTR	lpszUnicode=(IsUnicodeEnv()? " Unicode":""),
				lpszSecIM=(ServiceExists("SecureIM/IsContactSecured")? " + SecureIM":"");
		SIZE_T dwSelfVersionSizeRet;

		dwSelfVersionSizeRet=mir_snprintf(lpszSelfVersion,dwSelfVersionSize,"Miranda IM %lu.%lu.%lu.%lu%s (MRA v%lu.%lu.%lu.%lu)%s, version: %lu.%lu",(((dwMirVer)>>24)&0xFF),(((dwMirVer)>>16)&0xFF),(((dwMirVer)>>8)&0xFF),((dwMirVer)&0xFF),lpszUnicode,(((PLUGIN_VERSION_DWORD)>>24)&0xFF),(((PLUGIN_VERSION_DWORD)>>16)&0xFF),(((PLUGIN_VERSION_DWORD)>>8)&0xFF),((PLUGIN_VERSION_DWORD)&0xFF),lpszSecIM,PROTO_MAJOR(PROTO_VERSION),PROTO_MINOR(PROTO_VERSION));
		
		if (pdwSelfVersionSizeRet) (*pdwSelfVersionSizeRet)=dwSelfVersionSizeRet;
		dwRetErrorCode=NO_ERROR;
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}

DWORD GetParamValue(LPSTR lpszData,SIZE_T dwDataSize,LPSTR lpszParamName,SIZE_T dwParamNameSize,LPSTR lpszParamValue,SIZE_T dwParamValueSize,SIZE_T *pParamValueSizeRet)
{
	DWORD dwRetErrorCode;

	if (lpszData && dwDataSize && lpszParamName && dwParamNameSize && lpszParamValue && dwParamValueSize)
	{
		char szData[USER_AGENT_MAX+4096];
		LPSTR lpszParamDataStart,lpszParamDataEnd;

		dwRetErrorCode=ERROR_NOT_FOUND;

		dwDataSize=((dwDataSize<SIZEOF(szData))? dwDataSize:SIZEOF(szData));
		BuffToLowerCase(szData,lpszData,dwDataSize);

		lpszParamDataStart=(LPSTR)MemoryFind(0,szData,dwDataSize,lpszParamName,dwParamNameSize);
		if (lpszParamDataStart)
		if ((*((WORD*)(lpszParamDataStart+dwParamNameSize)))==(*((WORD*)"=\"")))
		{
			lpszParamDataStart+=dwParamNameSize+2;
			lpszParamDataEnd=(LPSTR)MemoryFindByte((lpszParamDataStart-szData),szData,dwDataSize,'"');
			if (lpszParamDataEnd)
			{
				memmove(lpszParamValue,(lpszData+(lpszParamDataStart-szData)),(lpszParamDataEnd-lpszParamDataStart));
				if (pParamValueSizeRet) (*pParamValueSizeRet)=(lpszParamDataEnd-lpszParamDataStart);
				dwRetErrorCode=NO_ERROR;
			}
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}

DWORD MraGetVersionStringFromFormated(LPSTR lpszUserAgentFormated,SIZE_T dwUserAgentFormatedSize,LPSTR lpszVersion,SIZE_T dwVersionSize,SIZE_T *pdwVersionSizeRet)
{
	DWORD dwRetErrorCode;

	if (lpszUserAgentFormated && dwUserAgentFormatedSize && lpszVersion && dwVersionSize)
	{
		char szBuff[4096];
		SIZE_T dwBuffSize,dwVersionSizeRet;

		if (GetParamValue(lpszUserAgentFormated,dwUserAgentFormatedSize,"name",4,szBuff,SIZEOF(szBuff),&dwBuffSize)==NO_ERROR && CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,szBuff,((dwBuffSize<10)? dwBuffSize:10),"Miranda IM",10)==CSTR_EQUAL)
		{// Miranda IM
			GetParamValue(lpszUserAgentFormated,dwUserAgentFormatedSize,"title",5,lpszVersion,dwVersionSize,pdwVersionSizeRet);
		}else{
			dwVersionSizeRet=0;

			if (GetParamValue(lpszUserAgentFormated,dwUserAgentFormatedSize,"client",6,lpszVersion,dwVersionSize,&dwBuffSize)==NO_ERROR)
			{
				dwVersionSizeRet+=dwBuffSize;
				(*((BYTE*)(lpszVersion+dwVersionSizeRet)))=' ';
			}

			if (GetParamValue(lpszUserAgentFormated,dwUserAgentFormatedSize,"name",4,lpszVersion,dwVersionSize,&dwBuffSize)==NO_ERROR)
			{
				dwVersionSizeRet+=dwBuffSize;
				(*((BYTE*)(lpszVersion+dwVersionSizeRet)))=' ';
			}

			if (GetParamValue(lpszUserAgentFormated,dwUserAgentFormatedSize,"title",5,lpszVersion,dwVersionSize,&dwBuffSize)==NO_ERROR)
			{
				dwVersionSizeRet+=dwBuffSize;
				(*((BYTE*)(lpszVersion+dwVersionSizeRet)))=' ';
			}

			if (GetParamValue(lpszUserAgentFormated,dwUserAgentFormatedSize,"version",7,(lpszVersion+dwVersionSizeRet+1),(dwVersionSize-dwVersionSizeRet),&dwBuffSize)==NO_ERROR)
			{
				dwVersionSizeRet+=(dwBuffSize+1);
				(*((BYTE*)(lpszVersion+dwVersionSizeRet)))='.';
				if (GetParamValue(lpszUserAgentFormated,dwUserAgentFormatedSize,"build",5,(lpszVersion+dwVersionSizeRet+1),(dwVersionSize-dwVersionSizeRet),&dwBuffSize)==NO_ERROR)
				{
					dwVersionSizeRet+=(dwBuffSize+1);
				}
			}

			if (dwVersionSizeRet==0)
			{// no data extracted, copy raw
				dwVersionSizeRet=((dwUserAgentFormatedSize<dwVersionSize)? dwUserAgentFormatedSize:dwVersionSize);
				memmove(lpszVersion,lpszUserAgentFormated,dwVersionSizeRet);
			}

			if (pdwVersionSizeRet) (*pdwVersionSizeRet)=dwVersionSizeRet;
		}
		dwRetErrorCode=NO_ERROR;
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraAddrListGetFromBuff(LPSTR lpszAddreses,SIZE_T dwAddresesSize,MRA_ADDR_LIST *pmalAddrList)
{
	DWORD dwRetErrorCode;

	if (lpszAddreses && dwAddresesSize && pmalAddrList)
	{
		LPSTR lpszCurrentItem,lpszDelimiter,lpszEndItem;
		DWORD dwAllocatedCount;

		dwAllocatedCount=ALLOCATED_COUNT;
		pmalAddrList->dwAddrCount=0;
		pmalAddrList->pmaliAddress=(MRA_ADDR_LIST_ITEM*)MEMALLOC(sizeof(MRA_ADDR_LIST_ITEM)*dwAllocatedCount);
		lpszCurrentItem=lpszAddreses;
		
		while(TRUE)
		{
			lpszEndItem=(LPSTR)MemoryFindByte((lpszCurrentItem-lpszAddreses),lpszAddreses,dwAddresesSize,';');
			if (lpszEndItem==NULL) lpszEndItem=(lpszAddreses+dwAddresesSize);
			if (lpszEndItem)
			{
				lpszDelimiter=(LPSTR)MemoryFindByte((lpszCurrentItem-lpszAddreses),lpszAddreses,dwAddresesSize,':');
				if (lpszDelimiter)
				{
					if (pmalAddrList->dwAddrCount==dwAllocatedCount)
					{
						dwAllocatedCount+=ALLOCATED_COUNT;
						pmalAddrList->pmaliAddress=(MRA_ADDR_LIST_ITEM*)MEMREALLOC(pmalAddrList->pmaliAddress,(sizeof(MRA_ADDR_LIST_ITEM)*dwAllocatedCount));
					}

					(*lpszDelimiter)=0;
					pmalAddrList->pmaliAddress[pmalAddrList->dwAddrCount].dwAddr=inet_addr(lpszCurrentItem);
					pmalAddrList->pmaliAddress[pmalAddrList->dwAddrCount].dwPort=StrToUNum32((lpszDelimiter+1),(lpszEndItem-(lpszDelimiter+1)));
					(*lpszDelimiter)=':';
					pmalAddrList->dwAddrCount++;
					lpszCurrentItem=(lpszEndItem+1);
				}else{
					break;
				}
				if (lpszEndItem==(lpszAddreses+dwAddresesSize)) break;
			}else{
				break;
			}
		}
		pmalAddrList->pmaliAddress=(MRA_ADDR_LIST_ITEM*)MEMREALLOC(pmalAddrList->pmaliAddress,(sizeof(MRA_ADDR_LIST_ITEM)*pmalAddrList->dwAddrCount));
		dwRetErrorCode=NO_ERROR;
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MraAddrListGetToBuff(MRA_ADDR_LIST *pmalAddrList,LPSTR lpszBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSizeRet)
{
	DWORD dwRetErrorCode;

	if (pmalAddrList && lpszBuff && dwBuffSize)
	{
		LPSTR lpszCurPos=lpszBuff;

		for (SIZE_T i=0;i<pmalAddrList->dwAddrCount;i++)
		{
			lpszCurPos+=mir_snprintf(lpszCurPos,(dwBuffSize-((SIZE_T)lpszCurPos-(SIZE_T)lpszBuff)),"%s:%lu;",inet_ntoa((*((in_addr*)&pmalAddrList->pmaliAddress[i].dwAddr))),pmalAddrList->pmaliAddress[i].dwPort);
		}

		if (pdwBuffSizeRet) (*pdwBuffSizeRet)=(lpszCurPos-lpszBuff);
		dwRetErrorCode=NO_ERROR;
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


void MraAddrListStoreToContact(HANDLE hContact,MRA_ADDR_LIST *pmalAddrList)
{
	if (hContact && pmalAddrList)
	if (pmalAddrList->dwAddrCount)
	{
		DB_Mra_SetDword(hContact,"OldIP",DB_Mra_GetDword(hContact,"IP",0));
		DB_Mra_SetDword(hContact,"IP",HTONL(pmalAddrList->pmaliAddress[0].dwAddr));
		if (pmalAddrList->dwAddrCount>1)
		{
			DB_Mra_SetDword(hContact,"OldRealIP",DB_Mra_GetDword(hContact,"RealIP",0));
			DB_Mra_SetDword(hContact,"RealIP",HTONL(pmalAddrList->pmaliAddress[1].dwAddr));
		}
	}
}


void MraAddrListFree(MRA_ADDR_LIST *pmalAddrList)
{
	if (pmalAddrList)
	{
		MEMFREE(pmalAddrList->pmaliAddress);
		pmalAddrList->dwAddrCount=0;
	}
}




void DB_Mra_CreateResidentSetting(LPSTR lpszValueName)
{
	char szSetting[2*MAX_PATH];

	mir_snprintf(szSetting,SIZEOF(szSetting),"%s/%s",PROTOCOL_NAMEA,lpszValueName);
	CallService(MS_DB_SETSETTINGRESIDENT,TRUE,(WPARAM)szSetting);
}

BOOL DB_GetStaticStringA(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPSTR lpszRetBuff,SIZE_T dwRetBuffSize,SIZE_T *pdwRetBuffSize)
{// sizes in wchars
	BOOL bRet=FALSE;
	SIZE_T dwReadedStringLen,dwRetBuffSizeLocal;
	DBVARIANT dbv={0};
	DBCONTACTGETSETTING sVal={0};

	dbv.type=DBVT_WCHAR;
	sVal.pValue=&dbv;
	sVal.szModule=lpszModule;
	sVal.szSetting=lpszValueName;
	if (CallService(MS_DB_CONTACT_GETSETTING_STR,(WPARAM)hContact,(LPARAM)&sVal)==0)
	{
		dwReadedStringLen=lstrlenW(dbv.pwszVal);
		if (lpszRetBuff && (dwRetBuffSize>dwReadedStringLen))
		{
			dwRetBuffSizeLocal=WideCharToMultiByte(MRA_CODE_PAGE,0,dbv.pwszVal,dwReadedStringLen,lpszRetBuff,dwRetBuffSize,NULL,NULL);
			(*((CHAR*)(lpszRetBuff+dwRetBuffSizeLocal)))=0;
			bRet=TRUE;
		}else{
			dwRetBuffSizeLocal=dwReadedStringLen;
			if (lpszRetBuff && dwRetBuffSize>=sizeof(WORD)) (*((WORD*)lpszRetBuff))=0;
		}
		if (pdwRetBuffSize) (*pdwRetBuffSize)=dwRetBuffSizeLocal;

		DBFreeVariant(&dbv);
	}else{
		if (lpszRetBuff && dwRetBuffSize>=sizeof(WORD)) (*((WORD*)lpszRetBuff))=0;
		if (pdwRetBuffSize)	(*pdwRetBuffSize)=0;
	}
return(bRet);
}


BOOL DB_GetStaticStringW(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPWSTR lpwszRetBuff,SIZE_T dwRetBuffSize,SIZE_T *pdwRetBuffSize)
{// sizes in wchars
	BOOL bRet=FALSE;
	SIZE_T dwReadedStringLen;
	DBVARIANT dbv={0};
	DBCONTACTGETSETTING sVal={0};

	dbv.type=DBVT_WCHAR;
	sVal.pValue=&dbv;
	sVal.szModule=lpszModule;
	sVal.szSetting=lpszValueName;
	if (CallService(MS_DB_CONTACT_GETSETTING_STR,(WPARAM)hContact,(LPARAM)&sVal)==0)
	{
		dwReadedStringLen=lstrlenW(dbv.pwszVal);
		if (lpwszRetBuff && (dwRetBuffSize>dwReadedStringLen))
		{
			memmove(lpwszRetBuff,dbv.pszVal,(dwReadedStringLen*sizeof(WCHAR)));//include null terminated
			(*((WCHAR*)(lpwszRetBuff+dwReadedStringLen)))=0;
			bRet=TRUE;
		}else{
			if (lpwszRetBuff && dwRetBuffSize>=sizeof(WCHAR)) (*((WCHAR*)lpwszRetBuff))=0;
		}
		if (pdwRetBuffSize) (*pdwRetBuffSize)=dwReadedStringLen;

		DBFreeVariant(&dbv);
	}else{
		if (lpwszRetBuff && dwRetBuffSize>=sizeof(WCHAR)) (*((WCHAR*)lpwszRetBuff))=0;
		if (pdwRetBuffSize)	(*pdwRetBuffSize)=0;
	}
return(bRet);
}


BOOL DB_SetStringExA(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPSTR lpszValue,SIZE_T dwValueSize)
{
	BOOL bRet=FALSE;
	
	if (lpszValue && dwValueSize)
	{
		LPWSTR lpwszValueLocal;
		SIZE_T dwValueSizeLocal;

		dwValueSizeLocal=(dwValueSize+MAX_PATH);
		lpwszValueLocal=(LPWSTR)MEMALLOC((dwValueSizeLocal*sizeof(WCHAR)));

		if (lpwszValueLocal)
		{
			DBCONTACTWRITESETTING cws={0};

			cws.szModule=lpszModule;
			cws.szSetting=lpszValueName;
			cws.value.type=DBVT_WCHAR;
			cws.value.pwszVal=(WCHAR*)lpwszValueLocal;

			dwValueSizeLocal=MultiByteToWideChar(MRA_CODE_PAGE,0,lpszValue,dwValueSize,lpwszValueLocal,dwValueSizeLocal);
			lpwszValueLocal[dwValueSizeLocal]=0;
			bRet=(CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws)==0);

			MEMFREE(lpwszValueLocal);
		}
	}else{
		bRet=TRUE;
		DBDeleteContactSetting(hContact,lpszModule,lpszValueName);
	}
return(bRet);
}


BOOL DB_SetStringExW(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPWSTR lpwszValue,SIZE_T dwValueSize)
{
	BOOL bRet=FALSE;
	
	if (lpwszValue && dwValueSize)
	{
		LPWSTR lpwszValueLocal=(LPWSTR)MEMALLOC(((dwValueSize+MAX_PATH)*sizeof(WCHAR)));
		
		if (lpwszValueLocal)
		{
			DBCONTACTWRITESETTING cws={0};

			cws.szModule=lpszModule;
			cws.szSetting=lpszValueName;
			cws.value.type=DBVT_WCHAR;
			cws.value.pwszVal=(WCHAR*)lpwszValueLocal;
			memmove(lpwszValueLocal,lpwszValue,(dwValueSize*sizeof(WCHAR)));
			bRet=(CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws)==0);
			
			MEMFREE(lpwszValueLocal);
		}
	}else{
		bRet=TRUE;
		DBDeleteContactSetting(hContact,lpszModule,lpszValueName);
	}
return(bRet);
}


int DB_WriteContactSettingBlob(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPVOID lpValue,SIZE_T dwValueSize)
{
	DBCONTACTWRITESETTING cws={0};

	cws.szModule=lpszModule;
	cws.szSetting=lpszValueName;
	cws.value.type=DBVT_BLOB;
	cws.value.pbVal=(LPBYTE)lpValue;
	cws.value.cpbVal=(WORD)dwValueSize;
return(CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws));
}


BOOL DB_GetContactSettingBlob(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPVOID lpRet,SIZE_T dwRetBuffSize,SIZE_T *pdwRetBuffSize)
{
	BOOL bRet=FALSE;
	DBVARIANT dbv;
	DBCONTACTGETSETTING sVal={0};

	sVal.pValue=&dbv;
	sVal.szModule=lpszModule;
	sVal.szSetting=lpszValueName;
	if (CallService(MS_DB_CONTACT_GETSETTING,(WPARAM)hContact,(LPARAM)&sVal)==0)
	{
		if (dbv.type==DBVT_BLOB)
		{
			if (dwRetBuffSize>=dbv.cpbVal)
			{
				memmove(lpRet,dbv.pbVal,dbv.cpbVal);
				bRet=TRUE;
			}
			if (pdwRetBuffSize) (*pdwRetBuffSize)=dbv.cpbVal;
		}
		DBFreeVariant(&dbv);
	}else{
		if (pdwRetBuffSize)	(*pdwRetBuffSize)=0;
		bRet=FALSE;
	}
return(bRet);
}


DWORD GetContactFlags(HANDLE hContact)
{
	DWORD dwRet=0;

	if (IsContactMra(hContact))
	{
		CHAR szEMail[MAX_EMAIL_LEN];
		SIZE_T dwEMailSize;

		dwRet=DB_Mra_GetDword(hContact,"ContactFlags",0);
		dwRet&=~(CONTACT_FLAG_REMOVED|CONTACT_FLAG_GROUP|CONTACT_FLAG_INVISIBLE|CONTACT_FLAG_VISIBLE|CONTACT_FLAG_IGNORE|CONTACT_FLAG_SHADOW|CONTACT_FLAG_MULTICHAT);
		dwRet|=CONTACT_FLAG_UNICODE_NAME;

		if (DB_Mra_GetStaticStringA(hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
		{
			if (IsEMailChatAgent(szEMail,dwEMailSize))
			{
				dwRet|=CONTACT_FLAG_MULTICHAT;
			}
		}

		if (DBGetContactSettingByte(hContact,"CList","Hidden",0)) dwRet|=CONTACT_FLAG_SHADOW;
		
		switch(DB_Mra_GetWord(hContact,"ApparentMode",0)){
		case ID_STATUS_OFFLINE:
			dwRet|=CONTACT_FLAG_INVISIBLE;
			break;
		case ID_STATUS_ONLINE:
			dwRet|=CONTACT_FLAG_VISIBLE;
			break;
		}

		if (CallService(MS_IGNORE_ISIGNORED,(WPARAM)hContact,IGNOREEVENT_MESSAGE)) dwRet|=CONTACT_FLAG_IGNORE;
	}
return(dwRet);
}

DWORD SetContactFlags(HANDLE hContact,DWORD dwContactFlag)
{
	DWORD dwRetErrorCode=NO_ERROR;

	if (IsContactMra(hContact))
	{
		DB_Mra_SetDword(hContact,"ContactFlags",dwContactFlag);

		if (dwContactFlag&CONTACT_FLAG_SHADOW)
		{
			DBWriteContactSettingByte(hContact,"CList","Hidden",1);
		}else{
			DBDeleteContactSetting(hContact,"CList","Hidden");
		}
		switch(dwContactFlag&(CONTACT_FLAG_INVISIBLE|CONTACT_FLAG_VISIBLE)){
		case CONTACT_FLAG_INVISIBLE:
			DB_Mra_SetWord(hContact,"ApparentMode",ID_STATUS_OFFLINE);
			break;
		case CONTACT_FLAG_VISIBLE:
			DB_Mra_SetWord(hContact,"ApparentMode",ID_STATUS_ONLINE);
			break;
		default:
			DB_Mra_SetWord(hContact,"ApparentMode",0);
			break;
		}

		if (dwContactFlag&CONTACT_FLAG_IGNORE)
		{
			CallService(MS_IGNORE_IGNORE,(WPARAM)hContact,IGNOREEVENT_MESSAGE);
		}else{
			CallService(MS_IGNORE_UNIGNORE,(WPARAM)hContact,IGNOREEVENT_MESSAGE);
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD GetContactBasicInfoW(HANDLE hContact,DWORD *pdwID,DWORD *pdwGroupID,DWORD *pdwContactFlag,DWORD *pdwContactSeverFlags,DWORD *pdwStatus,LPSTR lpszEMail,SIZE_T dwEMailSize,SIZE_T *pdwEMailSize,LPWSTR lpwszNick,SIZE_T dwNickSize,SIZE_T *pdwNickSize,LPSTR lpszPhones,SIZE_T dwPhonesSize,SIZE_T *pdwPhonesSize)
{
	DWORD dwRetErrorCode=NO_ERROR;

	if (IsContactMra(hContact))
	{
		if (pdwID)						(*pdwID)=DB_Mra_GetDword(hContact,"ContactID",-1);
		if (pdwGroupID)					(*pdwGroupID)=DB_Mra_GetDword(hContact,"GroupID",-1);
		if (pdwContactSeverFlags)		(*pdwContactSeverFlags)=DB_Mra_GetDword(hContact,"ContactSeverFlags",0);
		if (pdwStatus)					(*pdwStatus)=MraGetContactStatus(hContact);
		if (pdwContactFlag)				(*pdwContactFlag)=GetContactFlags(hContact);

		if (lpszEMail && pdwEMailSize)	DB_Mra_GetStaticStringA(hContact,"e-mail",lpszEMail,dwEMailSize,pdwEMailSize);
		if (lpwszNick && pdwNickSize)
		{
			DB_GetStaticStringW(hContact,"CList","MyHandle",lpwszNick,dwNickSize,pdwNickSize);
			//if ((*pdwNickSize)==0) DebugBreak();
		}
		if (lpszPhones && pdwPhonesSize)
		{
			char szPhone[MAX_PATH],szValue[MAX_PATH];
			SIZE_T dwPhoneSize,dwCopyed=0;

			for(SIZE_T i=0;i<3;i++)
			{
				mir_snprintf(szValue,SIZEOF(szValue),"MyPhone%lu",i);
				if (DB_GetStaticStringA(hContact,"UserInfo",szValue,szPhone,SIZEOF(szPhone),&dwPhoneSize))
				{
					if (dwCopyed) (*((LPBYTE)(lpszPhones+dwCopyed++)))=',';
					dwCopyed+=CopyNumber((lpszPhones+dwCopyed),szPhone,dwPhoneSize);
				}
			}
			(*pdwPhonesSize)=dwCopyed;
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD SetContactBasicInfoW(HANDLE hContact,DWORD dwSetInfoFlags,DWORD dwFlags,DWORD dwID,DWORD dwGroupID,DWORD dwContactFlag,DWORD dwContactSeverFlags,DWORD dwStatus,LPSTR lpszEMail,SIZE_T dwEMailSize,LPWSTR lpwszNick,SIZE_T dwNickSize,LPSTR lpszPhones,SIZE_T dwPhonesSize)
{
	DWORD dwRetErrorCode=NO_ERROR;

	if (IsContactMra(hContact))
	{
		// LOCK
		if (dwSetInfoFlags&SCBIFSI_LOCK_CHANGES_EVENTS) DB_Mra_SetDword(hContact,"HooksLocked",TRUE);

		// поля которые нужны, и изменения которых не отслеживаются
		if(dwFlags&SCBIF_ID) DB_Mra_SetDword(hContact,"ContactID",dwID);
		
		if(dwFlags&SCBIF_EMAIL)
		{
			if (lpszEMail && dwEMailSize) DB_Mra_SetStringExA(hContact,"e-mail",lpszEMail,dwEMailSize);
		}

		// поля изменения которых отслеживаются
		if(dwFlags&SCBIF_GROUP_ID) DB_Mra_SetDword(hContact,"GroupID",dwGroupID);
		
		if(dwFlags&SCBIF_NICK)
		{
			if((dwFlags&SCBIF_FLAG) && ((dwContactFlag&CONTACT_FLAG_UNICODE_NAME)==0))
			{
				if (lpwszNick && dwNickSize) DB_SetStringExA(hContact,"CList","MyHandle",(LPSTR)lpwszNick,dwNickSize);
			}else{
				if (lpwszNick && dwNickSize) DB_SetStringExW(hContact,"CList","MyHandle",lpwszNick,dwNickSize);
				//if (dwNickSize==0) DebugBreak();
			}
		}

		if(dwFlags&SCBIF_PHONES)
		{
			if (lpszPhones && dwPhonesSize)
			{
				char szPhone[MAX_PATH],szValue[MAX_PATH];
				LPSTR lpszCurPhone,lpszPhoneNext;
				SIZE_T i,dwCurPhoneSize;

				i=0;
				lpszCurPhone=lpszPhones;
				lpszPhoneNext=lpszPhones;
				while(lpszPhoneNext)
				{
					lpszPhoneNext=(LPSTR)MemoryFindByte((lpszCurPhone-lpszPhones),lpszPhones,dwPhonesSize,',');
					if (lpszPhoneNext)
					{
						dwCurPhoneSize=(lpszPhoneNext-lpszCurPhone);
					}else{
						dwCurPhoneSize=((lpszPhones+dwPhonesSize)-lpszCurPhone);
					}

					szPhone[0]='+';
					memmove((szPhone+1),lpszCurPhone,min(dwCurPhoneSize,(SIZEOF(szPhone)-1)));
					mir_snprintf(szValue,SIZEOF(szValue),"MyPhone%lu",i);
					DB_SetStringExA(hContact,"UserInfo",szValue,szPhone,(1+dwCurPhoneSize));

					i++;
					lpszCurPhone=(lpszPhoneNext+1);
				}
			}
		}

		if(dwFlags&SCBIF_FLAG) SetContactFlags(hContact,dwContactFlag);

		if(dwFlags&SCBIF_SERVER_FLAG) DB_Mra_SetDword(hContact,"ContactSeverFlags",dwContactSeverFlags);

		if(dwFlags&SCBIF_STATUS) MraSetContactStatus(hContact,dwStatus);

		SetExtraIcons(hContact);
		// UNLOCK
		if (dwSetInfoFlags&SCBIFSI_LOCK_CHANGES_EVENTS) DB_Mra_SetDword(hContact,"HooksLocked",FALSE);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}



HANDLE MraHContactFromEmail(LPSTR lpszEMail,SIZE_T dwEMailSize,BOOL bAddIfNeeded,BOOL bTemporary,BOOL *pbAdded)
{
	HANDLE hContact=NULL;

	if (lpszEMail && dwEMailSize)
	{
		BOOL bFounded=FALSE;
		CHAR szEMailLocal[MAX_EMAIL_LEN];
		SIZE_T dwEMailLocalSize;

		if (dwEMailSize==-1) dwEMailSize=lstrlenA(lpszEMail);
		//check not already on list
		for(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{
			if (IsContactMra(hContact))
			if (DB_Mra_GetStaticStringA(hContact,"e-mail",szEMailLocal,SIZEOF(szEMailLocal),&dwEMailLocalSize))
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,szEMailLocal,dwEMailLocalSize,lpszEMail,dwEMailSize)==CSTR_EQUAL)
			{
				if (bTemporary==FALSE) DBDeleteContactSetting(hContact,"CList","NotOnList");
				bFounded=TRUE;
				break;
			}
		}

		if (bFounded==FALSE && bAddIfNeeded)
		{//not already there: add
			if (IsEMailChatAgent(lpszEMail,dwEMailSize))
			{
				GCSESSION gcw={0};
				WCHAR wszEMail[MAX_EMAIL_LEN]={0};

				gcw.cbSize=sizeof(GCSESSION);
				gcw.iType=GCW_CHATROOM;
				gcw.pszModule=PROTOCOL_NAMEA;
				gcw.ptszName=wszEMail;
				gcw.ptszID=(LPWSTR)wszEMail;
				gcw.dwFlags=GC_UNICODE;
				MultiByteToWideChar(MRA_CODE_PAGE,0,lpszEMail,dwEMailSize,wszEMail,SIZEOF(wszEMail));

				if (CallServiceSync(MS_GC_NEWSESSION,NULL,(LPARAM)&gcw)==0)
				{
					BOOL bChatAdded=FALSE;
					for(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
					{
						if (IsContactMra(hContact))
						if (DB_Mra_GetStaticStringA(hContact,"ChatRoomID",szEMailLocal,SIZEOF(szEMailLocal),&dwEMailLocalSize))
						if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,szEMailLocal,dwEMailLocalSize,lpszEMail,dwEMailSize)==CSTR_EQUAL)
						{
							bChatAdded=TRUE;
							break;
						}
					}
					if (bChatAdded==FALSE) hContact=NULL;
				}
			}else{
				hContact=(HANDLE)CallService(MS_DB_CONTACT_ADD,0,0);
				CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)hContact,(LPARAM)PROTOCOL_NAMEA);
			}
			
			if (hContact)
			{
				if (IsEMailChatAgent(lpszEMail,dwEMailSize))
				{
					SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,(SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS|SCBIF_EMAIL),-1,-1,0,CONTACT_INTFLAG_NOT_AUTHORIZED,ID_STATUS_ONLINE,lpszEMail,dwEMailSize,NULL,0,NULL,0);
				}else{
					if (bTemporary) DBWriteContactSettingByte(hContact,"CList","NotOnList",1);
					DB_Mra_SetStringExA(hContact,"MirVer",MIRVER_UNKNOWN,(sizeof(MIRVER_UNKNOWN)-1));
					SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,(SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS|SCBIF_EMAIL),-1,-1,0,CONTACT_INTFLAG_NOT_AUTHORIZED,ID_STATUS_OFFLINE,lpszEMail,dwEMailSize,NULL,0,NULL,0);
				}
			}
		}

		if (pbAdded) (*pbAdded)=(bFounded==FALSE && bAddIfNeeded && hContact);
	}
return(hContact);
}



BOOL MraUpdateContactInfo(HANDLE hContact)
{
	BOOL bRet=FALSE;

	if (masMraSettings.bLoggedIn && hContact)
	if (IsContactMra(hContact))
	{
		CHAR szEMail[MAX_EMAIL_LEN];
		SIZE_T dwEMailSize;

		if (DB_Mra_GetStaticStringA(hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
		{
			MraAvatarsQueueGetAvatarSimple(masMraSettings.hAvatarsQueueHandle,GAIF_FORCE,hContact,0);

			if (MraSendCommand_WPRequestByEMail(hContact,ACKTYPE_GETINFO,szEMail,dwEMailSize))
			{
				bRet=TRUE;
			}
		}
	}
return(bRet);
}



DWORD MraContactCapabilitiesGet(HANDLE hContact)
{
return(DB_Mra_GetDword(hContact,DBSETTING_CAPABILITIES,0));
}

void MraContactCapabilitiesSet(HANDLE hContact,DWORD dwFutureFlags)
{
	DB_Mra_SetDword(hContact,DBSETTING_CAPABILITIES,dwFutureFlags);
}


DWORD MraGetContactStatus(HANDLE hContact)
{
return(DB_Mra_GetWord(hContact,"Status",ID_STATUS_OFFLINE));
}


DWORD MraSetContactStatus(HANDLE hContact,DWORD dwNewStatus)
{
	DWORD dwOldStatus=MraGetContactStatus(hContact);
	
	if (dwNewStatus!=dwOldStatus)
	{
		BOOL bChatAgent;

		bChatAgent=IsContactChatAgent(hContact);

		if (dwNewStatus==ID_STATUS_OFFLINE)
		{
			if (hContact)
			{
				DB_Mra_SetByte(hContact,DBSETTING_XSTATUSID,MRA_MIR_XSTATUS_NONE);
				DB_Mra_DeleteValue(hContact,DBSETTING_XSTATUSNAME);
				DB_Mra_DeleteValue(hContact,DBSETTING_XSTATUSMSG);
				DB_Mra_DeleteValue(hContact,DBSETTING_BLOGSTATUSTIME);
				DB_Mra_DeleteValue(hContact,DBSETTING_BLOGSTATUSID);
				DB_Mra_DeleteValue(hContact,DBSETTING_BLOGSTATUS);
				DB_Mra_DeleteValue(hContact,DBSETTING_BLOGSTATUSMUSIC);
				MraContactCapabilitiesSet(hContact,0);
				if (bChatAgent) MraChatSessionDestroy(hContact);
			}
			DB_Mra_SetDword(hContact,"LogonTS",0);
			DB_Mra_DeleteValue(hContact,"IP");
			DB_Mra_DeleteValue(hContact,"RealIP");
		}else{
			if (dwOldStatus==ID_STATUS_OFFLINE)
			{
				DWORD dwTime=(DWORD)_time32(NULL);

				DB_Mra_SetDword(hContact,"LogonTS",dwTime);
				DB_Mra_SetDword(hContact,"OldLogonTS",dwTime);

				if (bChatAgent) MraChatSessionNew(hContact);
			}
			MraAvatarsQueueGetAvatarSimple(masMraSettings.hAvatarsQueueHandle,0,hContact,0);
		}

		DB_Mra_SetWord(hContact,"Status",(WORD)dwNewStatus);
	}
return(dwOldStatus);
}


void MraUpdateEmailStatus(LPSTR lpszFrom,SIZE_T dwFromSize,LPSTR lpszSubject,SIZE_T dwSubjectSize,DWORD dwDate,DWORD dwUIDL)
{
	BOOL bTrayIconNewMailNotify;
	WCHAR szStatusText[MAX_SECONDLINE];

	bTrayIconNewMailNotify=DB_Mra_GetByte(NULL,"TrayIconNewMailNotify",MRA_DEFAULT_TRAYICON_NEW_MAIL_NOTIFY);

	if (masMraSettings.dwEmailMessagesUnRead)
	{
		LPSTR lpszEMail;
		SIZE_T dwEMailSize;
		HANDLE hContact=NULL;
		WCHAR szMailBoxStatus[MAX_SECONDLINE];

		mir_sntprintf(szMailBoxStatus,SIZEOF(szMailBoxStatus),TranslateW(L"Unread mail is available: %lu/%lu messages"),masMraSettings.dwEmailMessagesUnRead,masMraSettings.dwEmailMessagesTotal);

		if ((lpszFrom && dwFromSize) || (lpszSubject && dwSubjectSize))
		{
			WCHAR szFrom[MAX_PATH]={0},szSubject[MAX_PATH]={0};

			if (GetEMailFromString(lpszFrom,dwFromSize,&lpszEMail,&dwEMailSize))
			{
				hContact=MraHContactFromEmail(lpszEMail,dwEMailSize,FALSE,TRUE,NULL);
			}

			dwFromSize=MultiByteToWideChar(MRA_CODE_PAGE,0,lpszFrom,dwFromSize,szFrom,SIZEOF(szFrom));
			szFrom[dwFromSize]=0;

			dwSubjectSize=MultiByteToWideChar(MRA_CODE_PAGE,0,lpszSubject,dwSubjectSize,szSubject,SIZEOF(szSubject));
			szSubject[dwSubjectSize]=0;

			mir_sntprintf(szStatusText,SIZEOF(szStatusText),TranslateW(L"From: %s\r\nSubject: %s\r\n%s"),szFrom,szSubject,szMailBoxStatus);
		}else{
			lstrcpynW(szStatusText,szMailBoxStatus,SIZEOF(szStatusText));
		}
		
		if (bTrayIconNewMailNotify)
		{
			char szServiceFunction[MAX_PATH],*pszServiceFunctionName;
			CLISTEVENT cle={0};

			cle.cbSize=sizeof(cle);
			//cle.hContact;
			//cle.hDbEvent;
			cle.lpszProtocol=PROTOCOL_NAMEA;
			cle.hIcon=IconLibGetIcon(masMraSettings.hMainMenuIcons[0]);
			cle.flags=(CLEF_UNICODE|CLEF_PROTOCOLGLOBAL);
			cle.pszService="";
			cle.ptszTooltip=szStatusText;

			if (DB_Mra_GetByte(NULL,"TrayIconNewMailClkToInbox",MRA_DEFAULT_TRAYICON_NEW_MAIL_CLK_TO_INBOX))
			{
				memmove(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
				pszServiceFunctionName=(szServiceFunction+PROTOCOL_NAME_LEN);
				memmove(pszServiceFunctionName,MRA_GOTO_INBOX,sizeof(MRA_GOTO_INBOX));
				cle.pszService=szServiceFunction;
			}
			CallService(MS_CLIST_ADDEVENT,0,(LPARAM)&cle);
		}

		SkinPlaySound(masMraSettings.szNewMailSound);
		if (hContact)
		{// update user info
			MraUpdateContactInfo(hContact);
			MraPopupShowFromContactW(hContact,MRA_POPUP_TYPE_EMAIL_STATUS,(MRA_POPUP_ALLOW_ENTER),szStatusText);
		}else{
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_EMAIL_STATUS,(MRA_POPUP_ALLOW_ENTER),szStatusText);
		}
	}else{
		if (DB_Mra_GetByte(NULL,"IncrementalNewMailNotify",MRA_DEFAULT_INC_NEW_MAIL_NOTIFY))
		{
			if (bTrayIconNewMailNotify) CallService(MS_CLIST_REMOVEEVENT,0,(LPARAM)PROTOCOL_NAMEA);
			PUDeletePopUp(masMraSettings.hWndEMailPopupStatus);
			masMraSettings.hWndEMailPopupStatus=NULL;
		}else{
			mir_sntprintf(szStatusText,SIZEOF(szStatusText),TranslateW(L"No unread mail is available\r\nTotal messages: %lu"),masMraSettings.dwEmailMessagesTotal);
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_EMAIL_STATUS,(MRA_POPUP_ALLOW_ENTER),szStatusText);
		}
	}
}


BOOL IsUnicodeEnv()
{// Are we running under unicode Miranda core ?
	BOOL bRet=FALSE;
	char szBuff[64]={0};

	if (CallService(MS_SYSTEM_GETVERSIONTEXT,SIZEOF(szBuff),(LPARAM)szBuff)==0)
	{
		BuffToLowerCase(szBuff,szBuff,SIZEOF(szBuff));
		bRet=(MemoryFind(0,szBuff,SIZEOF(szBuff),"unicode",7)!=NULL);
	}
return(bRet);
}


BOOL IsHTTPSProxyUsed(HANDLE hNetlibUser)
{
	BOOL bRet=FALSE;
	NETLIBUSERSETTINGS nlus={0};

	nlus.cbSize=sizeof(nlus);
	if (CallService(MS_NETLIB_GETUSERSETTINGS,(WPARAM)hNetlibUser,(LPARAM)&nlus))
	{
		if (nlus.useProxy && nlus.proxyType==PROXYTYPE_HTTPS) bRet=TRUE;
	}
return(bRet);
}


BOOL IsContactMra(HANDLE hContact)
{// определяет принадлежность контакта данной копии плагина
return(CallService(MS_PROTO_ISPROTOONCONTACT,(WPARAM)hContact,(LPARAM)PROTOCOL_NAMEA));
}


BOOL IsContactMraProto(HANDLE hContact)
{// определяется является ли контакт контактом MRA протокола, не зависимо от того какому плагину он принадлежит
	BOOL bRet=FALSE;
	LPSTR lpszProto=(LPSTR)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);

	if (lpszProto)
	{
		WCHAR szBuff[MAX_PATH];

		if (DB_GetStaticStringW(hContact,lpszProto,"AvatarLastCheckTime",szBuff,SIZEOF(szBuff),NULL))
		if (DB_GetStaticStringW(hContact,lpszProto,"AvatarLastModifiedTime",szBuff,SIZEOF(szBuff),NULL))
		{
			bRet=TRUE;
		}
	}
return(bRet);
}



BOOL IsEMailMy(LPSTR lpszEMail,SIZE_T dwEMailSize)
{
	BOOL bRet=FALSE;

	if (lpszEMail)
	{
		CHAR szEMailMy[MAX_EMAIL_LEN];
		SIZE_T dwEMailMySize;

		if (DB_Mra_GetStaticStringA(NULL,"e-mail",szEMailMy,SIZEOF(szEMailMy),&dwEMailMySize))
		{
			if (dwEMailSize==0) dwEMailSize=lstrlenA(lpszEMail);

			if (dwEMailMySize==dwEMailSize)
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpszEMail,dwEMailSize,szEMailMy,dwEMailMySize)==CSTR_EQUAL)
			{
				bRet=TRUE;
			}
		}
	}
return(bRet);
}


BOOL IsEMailChatAgent(LPSTR lpszEMail,SIZE_T dwEMailSize)
{
	BOOL bRet=FALSE;

	if (lpszEMail)
	{
		if (dwEMailSize==0) dwEMailSize=lstrlenA(lpszEMail);
		if ((sizeof(MAILRU_CHAT_CONF_DOMAIN)-1)<dwEMailSize)
		if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,MAILRU_CHAT_CONF_DOMAIN,(sizeof(MAILRU_CHAT_CONF_DOMAIN)-1),(lpszEMail+(dwEMailSize-(sizeof(MAILRU_CHAT_CONF_DOMAIN)-1))),(sizeof(MAILRU_CHAT_CONF_DOMAIN)-1))==CSTR_EQUAL)
		{
			if ( (*(BYTE*)(lpszEMail+((dwEMailSize-(sizeof(MAILRU_CHAT_CONF_DOMAIN)-1))-1)))=='@' )
			{
				bRet=TRUE;
			}
		}
	}
return(bRet);
}


BOOL IsContactChatAgent(HANDLE hContact)
{
	BOOL bRet=FALSE;

	if (hContact)
	{
		CHAR szEMail[MAX_EMAIL_LEN];
		SIZE_T dwEMailSize;

		if (DB_Mra_GetStaticStringA(hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
		{
			bRet=IsEMailChatAgent(szEMail,dwEMailSize);
		}
		//bRet=((GetContactFlags(hContact)&CONTACT_FLAG_MULTICHAT)!=0);
	}
return(bRet);
}



BOOL IsEMailMR(LPSTR lpszEMail,SIZE_T dwEMailSize)
{
	BOOL bRet=FALSE;

	if (lpszEMail)
	{
		SIZE_T i,dwDomainLen;

		if (dwEMailSize==0) dwEMailSize=lstrlenA(lpszEMail);
		for(i=0;lpcszMailRuDomains[i];i++)
		{
			dwDomainLen=lstrlenA(lpcszMailRuDomains[i]);
			if (dwDomainLen<dwEMailSize)
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpcszMailRuDomains[i],dwDomainLen,(lpszEMail+(dwEMailSize-dwDomainLen)),dwDomainLen)==CSTR_EQUAL)
			{
				if ( (*(BYTE*)(lpszEMail+((dwEMailSize-dwDomainLen)-1)))=='@' )
				{
					bRet=TRUE;
					break;
				}
			}
		}
	}
return(bRet);
}


BOOL GetEMailFromString(LPSTR lpszBuff,SIZE_T dwBuffSize,LPSTR *plpszEMail,SIZE_T *pdwEMailSize)
{
	BOOL bRet=FALSE;

	if (lpszBuff && dwBuffSize)
	{
		LPSTR lpszEMailStart,lpszEMailEnd;

		if ((lpszEMailStart=(LPSTR)MemoryFindByte(0,lpszBuff,dwBuffSize,'<')))
		{
			lpszEMailStart++;
			if ((lpszEMailEnd=(LPSTR)MemoryFindByte((lpszEMailStart-lpszBuff),lpszBuff,dwBuffSize,'>')))
			{
				if (plpszEMail)		(*plpszEMail)=lpszEMailStart;
				if (pdwEMailSize)	(*pdwEMailSize)=(lpszEMailEnd-lpszEMailStart);

				bRet=TRUE;
			}
		}
	}

	if (bRet==FALSE)
	{
		if (plpszEMail)		(*plpszEMail)=NULL;
		if (pdwEMailSize)	(*pdwEMailSize)=0;
	}

return(bRet);
}


DWORD GetContactEMailCountParam(HANDLE hContact,BOOL bMRAOnly,LPSTR lpszModule,LPSTR lpszValueName)
{
	DWORD dwRet=0;
	CHAR szBuff[MAX_PATH],szEMail[MAX_EMAIL_LEN];
	SIZE_T i,dwEMailSize;

	if(DB_GetStaticStringA(hContact,lpszModule,lpszValueName,szEMail,SIZEOF(szEMail),&dwEMailSize))
	{
		if (bMRAOnly==FALSE || IsEMailMR(szEMail,dwEMailSize)) dwRet++;
	}

	for (i=0;TRUE;i++)
	{
		mir_snprintf(szBuff,SIZEOF(szBuff),"%s%lu",lpszValueName,i);
		if(DB_GetStaticStringA(hContact,lpszModule,szBuff,szEMail,SIZEOF(szEMail),&dwEMailSize))
		{
			if (bMRAOnly==FALSE || IsEMailMR(szEMail,dwEMailSize)) dwRet++;
		}else{
			if (i>EMAILS_MIN_COUNT) break;
		}
	}
return(dwRet);
}


DWORD GetContactEMailCount(HANDLE hContact,BOOL bMRAOnly)
{
	DWORD dwRet=0;
	LPSTR lpszProto;

	if (hContact)
	{
		lpszProto=(LPSTR)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
	}else{
		lpszProto=PROTOCOL_NAMEA;
	}
	dwRet+=GetContactEMailCountParam(hContact,bMRAOnly,lpszProto,"e-mail");
	dwRet+=GetContactEMailCountParam(hContact,bMRAOnly,"UserInfo","e-mail");
	dwRet+=GetContactEMailCountParam(hContact,bMRAOnly,"UserInfo","Mye-mail");
	dwRet+=GetContactEMailCountParam(hContact,bMRAOnly,"UserInfo","Companye-mail");
	dwRet+=GetContactEMailCountParam(hContact,bMRAOnly,"UserInfo","MyCompanye-mail");

return(dwRet);
}



BOOL GetContactFirstEMailParam(HANDLE hContact,BOOL bMRAOnly,LPSTR lpszModule,LPSTR lpszValueName,LPSTR lpszRetBuff,SIZE_T dwRetBuffSize,SIZE_T *pdwRetBuffSize)
{
	BOOL bRet=FALSE;
	CHAR szBuff[MAX_PATH],szEMail[MAX_EMAIL_LEN];
	SIZE_T i,dwEMailSize;

	if(DB_GetStaticStringA(hContact,lpszModule,lpszValueName,szEMail,SIZEOF(szEMail),&dwEMailSize))
	{
		if (bMRAOnly==FALSE || IsEMailMR(szEMail,dwEMailSize))
		{
			lstrcpynA(lpszRetBuff,szEMail,dwRetBuffSize);
			if (pdwRetBuffSize) (*pdwRetBuffSize)=dwEMailSize;
			bRet=TRUE;
		}
	}

	for (i=0;bRet==FALSE;i++)
	{
		mir_snprintf(szBuff,SIZEOF(szBuff),"%s%lu",lpszValueName,i);
		if(DB_GetStaticStringA(hContact,lpszModule,szBuff,szEMail,SIZEOF(szEMail),&dwEMailSize))
		{
			if (bMRAOnly==FALSE || IsEMailMR(szEMail,dwEMailSize))
			{
				lstrcpynA(lpszRetBuff,szEMail,dwRetBuffSize);
				if (pdwRetBuffSize) (*pdwRetBuffSize)=dwEMailSize;
				bRet=TRUE;
				break;
			}
		}else{
			if (i>EMAILS_MIN_COUNT) break;
		}
	}
return(bRet);
}


BOOL GetContactFirstEMail(HANDLE hContact,BOOL bMRAOnly,LPSTR lpszRetBuff,SIZE_T dwRetBuffSize,SIZE_T *pdwRetBuffSize)
{
	BOOL bRet=FALSE;
	LPSTR lpszProto;

	if (hContact)
	{
		lpszProto=(LPSTR)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
	}else{
		lpszProto=PROTOCOL_NAMEA;
	}
	bRet=GetContactFirstEMailParam(hContact,bMRAOnly,lpszProto,"e-mail",lpszRetBuff,dwRetBuffSize,pdwRetBuffSize);
	if (bRet==FALSE)	bRet=GetContactFirstEMailParam(hContact,bMRAOnly,"UserInfo","e-mail",lpszRetBuff,dwRetBuffSize,pdwRetBuffSize);
	if (bRet==FALSE)	bRet=GetContactFirstEMailParam(hContact,bMRAOnly,"UserInfo","Mye-mail",lpszRetBuff,dwRetBuffSize,pdwRetBuffSize);
	if (bRet==FALSE)	bRet=GetContactFirstEMailParam(hContact,bMRAOnly,"UserInfo","Companye-mail",lpszRetBuff,dwRetBuffSize,pdwRetBuffSize);
	if (bRet==FALSE)	bRet=GetContactFirstEMailParam(hContact,bMRAOnly,"UserInfo","MyCompanye-mail",lpszRetBuff,dwRetBuffSize,pdwRetBuffSize);

return(bRet);
}


/////////////////////////////////////////////////////////////////////////////////////////
// 
DWORD MraAPCQueueAdd(PAPCFUNC pfnAPC,PFIFO_MT pffmtAPCQueue,ULONG_PTR dwData)
{
	DWORD dwRetErrorCode;

	if (masMraSettings.hThreadAPC && pffmtAPCQueue && pfnAPC)
	{
		MRA_APC_QUEUE_ITEM *pqiApcQueueItem;

		pqiApcQueueItem=(MRA_APC_QUEUE_ITEM*)MEMALLOC(sizeof(MRA_APC_QUEUE_ITEM));
		if (pqiApcQueueItem)
		{
			pqiApcQueueItem->pfnAPC=pfnAPC;
			FifoMTItemPush(pffmtAPCQueue,(PCFIFO_MT_ITEM)pqiApcQueueItem,(LPVOID)dwData);
			SetEvent(masMraSettings.hWaitEventThreadAPCHandle);
			dwRetErrorCode=NO_ERROR;
		}else{
			dwRetErrorCode=GetLastError();
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


void MraAPCQueueProcess(PFIFO_MT pffmtAPCQueue)
{
	LPVOID lpData;
	MRA_APC_QUEUE_ITEM *pqiApcQueueItem;

	while(FifoMTItemPop(pffmtAPCQueue,(PFIFO_MT_ITEM*)&pqiApcQueueItem,&lpData)==NO_ERROR)
	{
		SleepEx(10,FALSE);
		pqiApcQueueItem->pfnAPC((ULONG_PTR)lpData);
		MEMFREE(pqiApcQueueItem);
	}
}


void MraAPCQueueDestroy(PFIFO_MT pffmtAPCQueue)
{
	if (masMraSettings.hThreadAPC)
	{
		LPVOID lpData;
		PFIFO_MT_ITEM pffmtiFifoItem;

		FifoMTLock(pffmtAPCQueue);

		while(FifoMTItemPop(pffmtAPCQueue,&pffmtiFifoItem,&lpData)==NO_ERROR)
		{
			MEMFREE(lpData);
			MEMFREE(pffmtiFifoItem);
		}

		FifoMTUnLock(pffmtAPCQueue);
		FifoMTDestroy(pffmtAPCQueue);
	}
}


void ShowFormatedErrorMessage(LPWSTR lpwszErrText,DWORD dwErrorCode)
{
	WCHAR szErrorText[2048],szErrDescription[1024];
	SIZE_T dwErrDescriptionSize;

	if (dwErrorCode==NO_ERROR)
	{
		mir_sntprintf(szErrorText,SIZEOF(szErrorText),L"%s",TranslateW(lpwszErrText));
	}else{
		dwErrDescriptionSize=(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,dwErrorCode,0,szErrDescription,(SIZEOF(szErrDescription)-sizeof(WCHAR)),NULL)-2);
		szErrDescription[dwErrDescriptionSize]=0;
		mir_sntprintf(szErrorText,SIZEOF(szErrorText),L"%s %lu: %s",TranslateW(lpwszErrText),dwErrorCode,szErrDescription);
	}
	MraPopupShowFromAgentW(MRA_POPUP_TYPE_ERROR,0,szErrorText);
}


DWORD ProtoBroadcastAckAsynchEx(const char *szModule,HANDLE hContact,int type,int hResult,HANDLE hProcess,LPARAM lParam,SIZE_T dwLparamSize)
{
	DWORD dwRetErrorCode;
	SIZE_T dwModuleSize;
	ACKDATA *lpAck;

	dwModuleSize=lstrlenA(szModule);
	if (dwLparamSize==-1) dwLparamSize=lstrlenA((LPSTR)lParam);
	lpAck=(ACKDATA*)MEMALLOC((sizeof(ACKDATA)+dwModuleSize+dwLparamSize+sizeof(DWORD)));
	if (lpAck)
	{
		lpAck->cbSize=sizeof(ACKDATA);
		lpAck->szModule=(((char*)lpAck)+sizeof(ACKDATA));
		lpAck->hContact=hContact;
		lpAck->type=type;
		lpAck->result=hResult;
		lpAck->hProcess=hProcess;
		if (dwLparamSize)
		{
			lpAck->lParam=(LPARAM)(lpAck->szModule+dwModuleSize);
			memmove((LPVOID)lpAck->lParam,(LPVOID)lParam,dwLparamSize);
		}else{
			lpAck->lParam=lParam;
		}
		memmove((LPVOID)lpAck->szModule,(LPVOID)szModule,dwModuleSize);

		if ((dwRetErrorCode=MraAPCQueueAdd(ThreadMarandaCallbackAck,&masMraSettings.ffmtAPCQueue,(ULONG_PTR)lpAck))!=NO_ERROR)
		{
			CallService(MS_PROTO_BROADCASTACK,0,(LPARAM)lpAck);
			MEMFREE(lpAck);
		}
	}else{
		dwRetErrorCode=GetLastError();
	}
return(dwRetErrorCode);
}


void CALLBACK ThreadMarandaCallbackAck(ULONG_PTR dwParam)
{
	if (dwParam)
	{
		CallService(MS_PROTO_BROADCASTACK,0,(LPARAM)dwParam);
		MEMFREE(dwParam);
	}
}


void MraUserAPCThreadProc(LPVOID lpParameter)
{
	DWORD dwWaitRetCode=WAIT_TIMEOUT,dwCurTickTime,dwNextCheckTime,dwLastPingTime,dwFailCounter;

	dwNextCheckTime=0;
	dwFailCounter=0;

	while(InterlockedExchangeAdd((volatile LONG*)&masMraSettings.dwAPCThreadRunning,0))
	{
		MraAPCQueueProcess(&masMraSettings.ffmtAPCQueue);

		if (masMraSettings.hThreadWorker)
		if (IsThreadAlive(masMraSettings.hThreadWorker))
		{// check man thread last answer time
			dwCurTickTime=GetTickCount();

			if (dwCurTickTime>dwNextCheckTime)
			{
				dwLastPingTime=InterlockedExchangeAdd((volatile LONG*)&masMraSettings.dwThreadWorkerLastPingTime,0);
				if ((dwCurTickTime>dwLastPingTime) && ((dwCurTickTime-dwLastPingTime)>(THREAD_MAX_PING_TIME*1000)))
				{// ping failure, thread not answer
					dwFailCounter++;
					DebugPrintCRLFW(L"Watchdog: mra worker thread not answering");
					if (dwFailCounter>THREAD_MAX_PING_FAIL_COUNT)
					{
						DebugPrintCRLFW(L"Watchdog: TERMINATING mra worker thread");
						TerminateThread(masMraSettings.hThreadWorker,WAIT_TIMEOUT);
						MraThreadClean();
						dwFailCounter=0;
					}
				}else{// ping ok, reset failures counter
					dwFailCounter=0;
				}
				dwNextCheckTime=(dwCurTickTime+(THREAD_MAX_PING_TIME*1000));
			}
		}else{// main thread is die, clean up
			DebugPrintCRLFW(L"Watchdog: mra worker thread is down!!!!");
			MraThreadClean();
		}
		dwWaitRetCode=WaitForSingleObjectEx(masMraSettings.hWaitEventThreadAPCHandle,THREAD_SLEEP_TIME,FALSE);
	}

	if (masMraSettings.hWaitEventThreadAPCHandle)
	{
		CloseHandle(masMraSettings.hWaitEventThreadAPCHandle);
		masMraSettings.hWaitEventThreadAPCHandle=NULL;
	}
	MraAPCQueueDestroy(&masMraSettings.ffmtAPCQueue);
	masMraSettings.hThreadAPC=NULL;
	InterlockedExchange((volatile LONG*)&masMraSettings.dwAPCThreadRunning,FALSE);

	DebugPrintCRLFW(L"Watchdog: APC thread ended, no watching.");
}



DWORD CreateBlobFromContact(HANDLE hContact,LPWSTR lpwszRequestReason,SIZE_T dwRequestReasonSize,LPBYTE lpbBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSizeRet)
{
	DWORD dwRetErrorCode;
	SIZE_T dwBuffSizeRet=((sizeof(DWORD)*2)+dwRequestReasonSize+5),dwSize;
	
	if (dwBuffSize>=dwBuffSizeRet)
	{
		PBYTE pCurBlob=lpbBuff;
		
		(*(DWORD*)pCurBlob)=0;pCurBlob+=sizeof(DWORD);
		(*(DWORD*)pCurBlob)=(DWORD)hContact;pCurBlob+=sizeof(DWORD);

		DB_Mra_GetStaticStringA(hContact,"Nick",(LPSTR)pCurBlob,(dwBuffSize-(pCurBlob-lpbBuff)),&dwSize);
		(*(pCurBlob+dwSize))=0;
		pCurBlob+=(dwSize+1);

		DB_Mra_GetStaticStringA(hContact,"FirstName",(LPSTR)pCurBlob,(dwBuffSize-(pCurBlob-lpbBuff)),&dwSize);
		(*(pCurBlob+dwSize))=0;
		pCurBlob+=(dwSize+1);

		DB_Mra_GetStaticStringA(hContact,"LastName",(LPSTR)pCurBlob,(dwBuffSize-(pCurBlob-lpbBuff)),&dwSize);
		(*(pCurBlob+dwSize))=0;
		pCurBlob+=(dwSize+1);

		DB_Mra_GetStaticStringA(hContact,"e-mail",(LPSTR)pCurBlob,(dwBuffSize-(pCurBlob-lpbBuff)),&dwSize);
		(*(pCurBlob+dwSize))=0;
		pCurBlob+=(dwSize+1);

		dwSize=WideCharToMultiByte(MRA_CODE_PAGE,0,lpwszRequestReason,dwRequestReasonSize,(LPSTR)pCurBlob,(dwBuffSize-(pCurBlob-lpbBuff)),NULL,NULL);
		//memmove(pCurBlob,lpszRequestReason,dwRequestReasonSize);
		(*(pCurBlob+dwSize))=0;
		pCurBlob+=(dwSize+1);
		
		dwBuffSizeRet=(pCurBlob-lpbBuff);
		dwRetErrorCode=NO_ERROR;
	}else{
		dwRetErrorCode=ERROR_INSUFFICIENT_BUFFER;
	}

	if (pdwBuffSizeRet) (*pdwBuffSizeRet)=dwBuffSizeRet;
return(dwRetErrorCode);
}



void CListCreateMenu(LONG lPosition,LONG lPopupPosition,HANDLE hMainIcon,LPSTR pszContactOwner,LPSTR lpszCListMenuType,const GUI_DISPLAY_ITEM *pgdiItems,HANDLE *hIcoLibIcons,SIZE_T dwCount,HANDLE *hResult)
{
	if (pgdiItems && hIcoLibIcons && dwCount && hResult)
	{
		char szServiceFunction[MAX_PATH],*pszServiceFunctionName;
		CLISTMENUITEM mi={0};

		memmove(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
		pszServiceFunctionName=szServiceFunction+PROTOCOL_NAME_LEN;

		mi.cbSize=sizeof(mi);
		mi.popupPosition=lPopupPosition;
		mi.ptszPopupName=PROTOCOL_DISPLAY_NAMEW;
		mi.pszService=szServiceFunction;
		mi.flags=(CMIF_UNICODE|CMIF_ICONFROMICOLIB);

		for (SIZE_T i=0;i<dwCount;i++)
		{
			memmove(pszServiceFunctionName,pgdiItems[i].lpszName,(lstrlenA(pgdiItems[i].lpszName)+1));
			if (pgdiItems[i].lpFunc) CreateServiceFunction(szServiceFunction,pgdiItems[i].lpFunc);
			mi.position=(lPosition+i);
			mi.icolibItem=hIcoLibIcons[i];
			mi.ptszName=pgdiItems[i].lpwszDescr;
			
			if (i==0 && hMainIcon)
			{
				mi.icolibItem=hMainIcon;
				mi.flags=(CMIF_UNICODE);
			}
			
			hResult[i]=(HANDLE)CallService(lpszCListMenuType,0,(LPARAM)&mi);
			
			if (i==0 && hMainIcon)
			{
				mi.flags=CMIM_FLAGS|CMIM_ICON|CMIF_ICONFROMICOLIB;
				mi.icolibItem=hIcoLibIcons[i];
				CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hResult[i],(LPARAM)&mi);
				mi.flags=(CMIF_UNICODE|CMIF_ICONFROMICOLIB);
			}
		}
	}
}


void CListDestroyMenu(const GUI_DISPLAY_ITEM *pgdiItems,SIZE_T dwCount)
{
	if (pgdiItems && dwCount)
	{
		CHAR szServiceFunction[MAX_PATH],*pszServiceFunctionName;

		memmove(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
		pszServiceFunctionName=szServiceFunction+PROTOCOL_NAME_LEN;

		for (SIZE_T i=0;i<dwCount;i++)
		{
			memmove(pszServiceFunctionName,pgdiItems[i].lpszName,(lstrlenA(pgdiItems[i].lpszName)+1));
			if (pgdiItems[i].lpFunc) DestroyServiceFunction(szServiceFunction);
		}
	}
}


void CListShowMenuItem(HANDLE hMenuItem,BOOL bShow)
{
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.flags=CMIM_FLAGS;
	if (bShow==FALSE) mi.flags|=CMIF_HIDDEN;

	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hMenuItem,(LPARAM)&mi);
}

/*void CListSetMenuItemIcon(HANDLE hMenuItem,HANDLE hIcon)
{
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.flags=CMIM_FLAGS|CMIM_ICON|CMIF_ICONFROMICOLIB;
	mi.icolibItem=hIcon;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hMenuItem,(LPARAM)&mi);
}*/

int ExtraSetIcon(HANDLE hExtraIcon,HANDLE hContact,HANDLE hImage,int iColumnType)
{
	int iRet;

	if (hExtraIcon)
	{
		iRet=ExtraIcon_SetIcon(hExtraIcon,hContact,hImage);
	}else{
		IconExtraColumn iec;
		
		iec.cbSize=sizeof(iec);
		iec.ColumnType=iColumnType;
		iec.hImage=((hImage!=NULL)? hImage:INVALID_HANDLE_VALUE);

		iRet=CallService(MS_CLIST_EXTRA_SET_ICON,(WPARAM)hContact,(LPARAM)&iec);
	}
return(iRet);
}



SIZE_T CopyNumber(LPCVOID lpcOutBuff,LPCVOID lpcBuff,SIZE_T dwLen)
{
	BYTE btChar;
	LPBYTE lpbOutBuff=(LPBYTE)lpcOutBuff,lpbInBuff=(LPBYTE)lpcBuff;

	for(SIZE_T i=0;i<dwLen;i++)
	{
		btChar=(*lpbInBuff++);
		if (btChar>='0' && btChar<='9') (*lpbOutBuff++)=btChar;
	}
	(*lpbOutBuff)=0;

return((lpbOutBuff-(LPBYTE)lpcOutBuff));
}


/*
BOOL IsPhone(LPSTR lpszString,SIZE_T dwStringSize)
{
	BOOL bRet;

	if (dwStringSize>1)
	{// country code
		BYTE btChar;

		bRet=TRUE;
		for(SIZE_T i=0;i<dwStringSize;i++)
		{
			btChar=(*lpszString++);
			if (btChar<'0' || btChar>'9')
			if (btChar!='+' && btChar!='S' && btChar!='M' && btChar!=' ' && btChar!='(' && btChar!=')')
			{
				bRet=FALSE;
				break;
			}
		}
	}else{
		bRet=FALSE;
	}
return(bRet);
}


BOOL IsContactPhoneParam(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPSTR lpszPhone,SIZE_T dwPhoneSize)
{
	BOOL bRet=FALSE;
	char szBuff[MAX_PATH],szPhoneLocal[MAX_EMAIL_LEN];
	SIZE_T i,dwPhoneSizeLocal;

	if(DB_GetStaticStringA(hContact,lpszModule,lpszValueName,szPhoneLocal,SIZEOF(szPhoneLocal),&dwPhoneSizeLocal))
	if (IsPhone(szPhoneLocal,dwPhoneSizeLocal))
	{
		dwPhoneSizeLocal=CopyNumber(szPhoneLocal,szPhoneLocal,dwPhoneSizeLocal);
		if (MemoryCompare(szPhoneLocal,dwPhoneSizeLocal,lpszPhone,dwPhoneSize)==CMEM_EQUAL)
		{
			bRet=TRUE;
		}
	}

	for (i=0;bRet==FALSE;i++)
	{
		mir_snprintf(szBuff,SIZEOF(szBuff),"%s%lu",lpszValueName,i);
		if(DB_GetStaticStringA(hContact,lpszModule,szBuff,szPhoneLocal,SIZEOF(szPhoneLocal),&dwPhoneSizeLocal))
		{
			if (IsPhone(szPhoneLocal,dwPhoneSizeLocal))
			{
				dwPhoneSizeLocal=CopyNumber(szPhoneLocal,szPhoneLocal,dwPhoneSizeLocal);
				if (MemoryCompare(szPhoneLocal,dwPhoneSizeLocal,lpszPhone,dwPhoneSize)==CMEM_EQUAL)
				{
					bRet=TRUE;
					break;
				}
			}
		}else{
			if (i>PHONES_MIN_COUNT) break;
		}
	}
return(bRet);
}


BOOL IsContactPhone(HANDLE hContact,LPSTR lpszPhone,SIZE_T dwPhoneSize)
{
	BOOL bRet=FALSE;
	char szPhoneLocal[MAX_EMAIL_LEN];
	LPSTR lpszProto;
	SIZE_T dwPhoneSizeLocal;

	if (hContact)
	{
		lpszProto=(LPSTR)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
	}else{
		lpszProto=PROTOCOL_NAMEA;
	}
	dwPhoneSizeLocal=CopyNumber(szPhoneLocal,lpszPhone,dwPhoneSize);

	if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,lpszProto,"Phone",szPhoneLocal,dwPhoneSizeLocal);
	if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,lpszProto,"Cellular",szPhoneLocal,dwPhoneSizeLocal);
	if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,lpszProto,"Fax",szPhoneLocal,dwPhoneSizeLocal);
	if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,"UserInfo","MyPhone",szPhoneLocal,dwPhoneSizeLocal);
	if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,"UserInfo","Phone",szPhoneLocal,dwPhoneSizeLocal);
	if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,"UserInfo","Cellular",szPhoneLocal,dwPhoneSizeLocal);
	if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,"UserInfo","Fax",szPhoneLocal,dwPhoneSizeLocal);

return(bRet);
}


HANDLE MraHContactFromPhone(LPSTR lpszPhone,SIZE_T dwPhoneSize,BOOL bAddIfNeeded,BOOL bTemporary,BOOL *pbAdded)
{
	HANDLE hContact=NULL;

	if (lpszPhone && dwPhoneSize)
	{
		BOOL bFounded=FALSE;

		//check not already on list
		for(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{
			if (IsContactPhone(hContact,lpszPhone,dwPhoneSize))
			{
				if (bTemporary==FALSE) DBDeleteContactSetting(hContact,"CList","NotOnList");
				bFounded=TRUE;
				break;
			}
		}

		if (bFounded==FALSE && bAddIfNeeded)
		{//not already there: add
			hContact=(HANDLE)CallService(MS_DB_CONTACT_ADD,0,0);
			CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)hContact,(LPARAM)PROTOCOL_NAMEA);
			//SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,(SCBIF_ID|SCBIF_GROUP_ID|SCBIF_FLAG|SCBIF_SERVER_FLAG|SCBIF_STATUS|SCBIF_EMAIL|SCBIF_PHONES),-1,-1,0,CONTACT_INTFLAG_NOT_AUTHORIZED,ID_STATUS_OFFLINE,lpszEMail,dwEMailSize,NULL,0,lpszPhone,dwPhoneSize);
			if (bTemporary) DBWriteContactSettingByte(hContact,"CList","NotOnList",1);
			DB_Mra_SetStringExA(hContact,"MirVer",MIRVER_UNKNOWN,(sizeof(MIRVER_UNKNOWN)-1));
			MraSetContactStatus(hContact,ID_STATUS_OFFLINE);
		}

		if (pbAdded) (*pbAdded)=(bFounded==FALSE && bAddIfNeeded && hContact);
	}
return(hContact);
}*/

void EnableControlsArray(HWND hWndDlg,WORD *pwControlsList,SIZE_T dwControlsListCount,BOOL bEnabled)
{
	for(SIZE_T i=0;i<dwControlsListCount;i++) EnableWindow(GetDlgItem(hWndDlg,pwControlsList[i]),bEnabled);
}



LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	LRESULT lrRet=0;
	WNDPROC OldMessageEditProc=(WNDPROC)GetWindowLongPtr(hwnd,GWLP_USERDATA);

	if (msg==WM_CHAR)
	if (GetKeyState(VK_CONTROL)&0x8000) 
	{
		if(wParam=='\n') 
		{
			PostMessage(GetParent(hwnd),WM_COMMAND,IDOK,0);
			return(0);
		}
		if (wParam==1) 
		{// ctrl-a
			SendMessage(hwnd,EM_SETSEL,0,-1);
			return(0);
		}
		if (wParam==23) 
		{// ctrl-w
			SendMessage(GetParent(hwnd),WM_CLOSE,0,0);
			return(0);
		}
	}
	
	if (OldMessageEditProc) lrRet=CallWindowProc(OldMessageEditProc,hwnd,msg,wParam,lParam);

return(lrRet);
}


BOOL MraRequestXStatusDetails(DWORD dwXStatus)
{
	BOOL bRet;

	if (IsXStatusValid(dwXStatus))
	{
		bRet=(DialogBoxParamW(masMraSettings.hInstance,MAKEINTRESOURCE(IDD_SETXSTATUS),NULL,SetXStatusDlgProc,(LPARAM)dwXStatus)!=-1);
	}else{
		bRet=FALSE;
	}
return(bRet);
}

INT_PTR CALLBACK SetXStatusDlgProc(HWND hWndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=FALSE;
	SetXStatusData *psxsData=(SetXStatusData*)GetWindowLongPtr(hWndDlg,GWLP_USERDATA);

	switch(message){
    case WM_INITDIALOG:// set our xStatus
		TranslateDialogDefault(hWndDlg);

		psxsData=(SetXStatusData*)MEMALLOC(sizeof(SetXStatusData));
		if (psxsData)
		{
			char szValueName[MAX_PATH];
			HWND hWndEdit;
			WCHAR szBuff[STATUS_TITLE_MAX+STATUS_DESC_MAX];
			WNDPROC OldMessageEditProc;

			psxsData->dwXStatus=lParam;
			psxsData->hDlgIcon=IconLibGetIcon(masMraSettings.hXStatusAdvancedStatusIcons[psxsData->dwXStatus]);
			psxsData->dwCountdown=5;

			hWndEdit=GetDlgItem(hWndDlg,IDC_XTITLE);
			OldMessageEditProc=(WNDPROC)SetWindowLongPtr(hWndEdit,GWLP_WNDPROC,(LONG_PTR)MessageEditSubclassProc);
			SetWindowLongPtr(hWndEdit,GWLP_USERDATA,(LONG_PTR)OldMessageEditProc);

			hWndEdit=GetDlgItem(hWndDlg,IDC_XMSG);
			OldMessageEditProc=(WNDPROC)SetWindowLongPtr(hWndEdit,GWLP_WNDPROC,(LONG_PTR)MessageEditSubclassProc);
			SetWindowLongPtr(hWndEdit,GWLP_USERDATA,(LONG_PTR)OldMessageEditProc);

			SetWindowLongPtr(hWndDlg,GWLP_USERDATA,(LONG_PTR)psxsData);
			SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_XTITLE,EM_LIMITTEXT,STATUS_TITLE_MAX,0);
			SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_XMSG,EM_LIMITTEXT,STATUS_DESC_MAX,0);
			SendMessage(hWndDlg,WM_SETICON,ICON_BIG,(LPARAM)psxsData->hDlgIcon);
			SendMessage(hWndDlg,WM_SETTEXT,0,(LPARAM)TranslateW(lpcszXStatusNameDef[psxsData->dwXStatus]));

			mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%ldName",psxsData->dwXStatus);
			if (DB_Mra_GetStaticStringW(NULL,szValueName,szBuff,(STATUS_TITLE_MAX+1),NULL))
			{// custom xstatus name
				SET_DLG_ITEM_TEXT(hWndDlg,IDC_XTITLE,szBuff);
			}else{// default xstatus name
				SET_DLG_ITEM_TEXT(hWndDlg,IDC_XTITLE,TranslateW(lpcszXStatusNameDef[psxsData->dwXStatus]));
			}

			mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%ldMsg",psxsData->dwXStatus);
			if (DB_Mra_GetStaticStringW(NULL,szValueName,szBuff,(STATUS_DESC_MAX+1),NULL))
			{// custom xstatus description
				SET_DLG_ITEM_TEXT(hWndDlg,IDC_XMSG,szBuff);
			}else{// default xstatus description
				SET_DLG_ITEM_TEXT(hWndDlg,IDC_XMSG,L"");
			}

			SendMessage(hWndDlg,WM_TIMER,0,0);
			SetTimer(hWndDlg,1,1000,0);
			iRet=TRUE;
		}
		break;
	case WM_TIMER:
		if(psxsData->dwCountdown!=-1) 
		{
			WCHAR szBuff[MAX_PATH];
			mir_sntprintf(szBuff,SIZEOF(szBuff),L"%s %ld",TranslateW(L"Closing in"),psxsData->dwCountdown);
			SET_DLG_ITEM_TEXT(hWndDlg,IDOK,szBuff);
			psxsData->dwCountdown--;
			break;
		}
    case WM_CLOSE:
      DestroyWindow(hWndDlg);
      break;
    case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDOK:
			DestroyWindow(hWndDlg);
			break;
		case IDC_XTITLE:
		case IDC_XMSG:
			KillTimer(hWndDlg,1);
			SET_DLG_ITEM_TEXTW(hWndDlg,IDOK,TranslateW(L"OK"));
			break;
		}
		break;
    case WM_DESTROY:
		if (psxsData)
		{ // set our xStatus
			char szValueName[MAX_PATH];
			HWND hWndEdit;
			WCHAR szBuff[STATUS_TITLE_MAX+STATUS_DESC_MAX];
			SIZE_T dwBuffSize;
			WNDPROC OldMessageEditProc;
			CLISTMENUITEM mi={0};

			SetWindowLongPtr(hWndDlg,GWLP_USERDATA,(LONG_PTR)0);

			hWndEdit=GetDlgItem(hWndDlg,IDC_XTITLE);
			OldMessageEditProc=(WNDPROC)GetWindowLongPtr(hWndEdit,GWLP_USERDATA);
			SetWindowLongPtr(hWndEdit,GWLP_WNDPROC,(LONG_PTR)OldMessageEditProc);
			SetWindowLongPtr(hWndEdit,GWLP_USERDATA,(LONG_PTR)0);

			hWndEdit=GetDlgItem(hWndDlg,IDC_XMSG);
			OldMessageEditProc=(WNDPROC)GetWindowLongPtr(hWndEdit,GWLP_USERDATA);
			SetWindowLongPtr(hWndEdit,GWLP_WNDPROC,(LONG_PTR)OldMessageEditProc);
			SetWindowLongPtr(hWndEdit,GWLP_USERDATA,(LONG_PTR)0);

			dwBuffSize=GET_DLG_ITEM_TEXT(hWndDlg,IDC_XMSG,szBuff,(STATUS_DESC_MAX+1));
			mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%ldMsg",psxsData->dwXStatus);
			DB_Mra_SetStringExW(NULL,szValueName,szBuff,dwBuffSize);
			DB_Mra_SetStringExW(NULL,DBSETTING_XSTATUSMSG,szBuff,dwBuffSize);

			dwBuffSize=GET_DLG_ITEM_TEXT(hWndDlg,IDC_XTITLE,szBuff,(STATUS_TITLE_MAX+1));
			if (dwBuffSize==0)
			{// user delete all text
				lstrcpynW(szBuff,TranslateW(lpcszXStatusNameDef[psxsData->dwXStatus]),(STATUS_TITLE_MAX+1));
				dwBuffSize=lstrlenW(szBuff);
			}
			mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%dName",psxsData->dwXStatus);
			DB_Mra_SetStringExW(NULL,szValueName,szBuff,dwBuffSize);
			DB_Mra_SetStringExW(NULL,DBSETTING_XSTATUSNAME,szBuff,dwBuffSize);
			
			mi.cbSize=sizeof(mi);
			mi.flags=(CMIM_NAME|CMIF_UNICODE);
			mi.ptszName=szBuff;
			CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)masMraSettings.hXStatusMenuItems[psxsData->dwXStatus],(LPARAM)&mi);

			MraSetXStatusInternal(psxsData->dwXStatus);

			MEMFREE(psxsData);
		}
		EndDialog(hWndDlg,NO_ERROR);
		break;
	//default:
	//	bRet=DefWindowProc(hWndDlg,message,wParam,lParam);
	//	break;
	}
return(iRet);
}


BOOL MraSendReplyBlogStatus(HANDLE hContact)
{
	BOOL bRet;

	bRet=(CreateDialogParam(masMraSettings.hInstance,MAKEINTRESOURCE(IDD_MINIBLOG),NULL,SendReplyBlogStatusDlgProc,(LPARAM)hContact)!=NULL);
return(bRet);
}

INT_PTR CALLBACK SendReplyBlogStatusDlgProc(HWND hWndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=FALSE;
	SetBlogStatusData *psbsdData=(SetBlogStatusData*)GetWindowLongPtr(hWndDlg,GWLP_USERDATA);

	switch(message){
    case WM_INITDIALOG:// set our xStatus
		TranslateDialogDefault(hWndDlg);

		psbsdData=(SetBlogStatusData*)MEMALLOC(sizeof(SetBlogStatusData));
		if (psbsdData)
		{
			HWND hWndEdit;
			DWORD dwTime;
			WCHAR szBuff[MICBLOG_STATUS_MAX];
			WNDPROC OldMessageEditProc;
			SYSTEMTIME stBlogStatusTime={0};

			psbsdData->hContact=(HANDLE)lParam;
			//psbsdData->hDlgIcon=IconLibGetIcon(masMraSettings.hXStatusAdvancedStatusIcons[psxsData->dwXStatus]);
			SetWindowLongPtr(hWndDlg,GWLP_USERDATA,(LONG_PTR)psbsdData);

			hWndEdit=GetDlgItem(hWndDlg,IDC_MSG_TO_SEND);
			OldMessageEditProc=(WNDPROC)SetWindowLongPtr(hWndEdit,GWLP_WNDPROC,(LONG_PTR)MessageEditSubclassProc);
			SetWindowLongPtr(hWndEdit,GWLP_USERDATA,(LONG_PTR)OldMessageEditProc);
			SendMessage(hWndEdit,EM_LIMITTEXT,MICBLOG_STATUS_MAX,0);


			SendMessage(hWndDlg,WM_SETICON,ICON_BIG,(LPARAM)IconLibGetIcon(masMraSettings.hMainMenuIcons[5]));

			if (DB_Mra_GetStaticStringW(psbsdData->hContact,DBSETTING_BLOGSTATUS,szBuff,SIZEOF(szBuff),NULL))
			{// blog status message
				SET_DLG_ITEM_TEXT(hWndDlg,IDC_USER_BLOG_STATUS_MSG,szBuff);
			}

			if (psbsdData->hContact)
			{// reply to some user blog
				mir_sntprintf(szBuff,SIZEOF(szBuff),TranslateW(L"Reply to %s blog status"),GetContactNameW(psbsdData->hContact));
				SendMessage(hWndDlg,WM_SETTEXT,0,(LPARAM)szBuff);
			}else{
				SendMessage(hWndDlg,WM_SETTEXT,0,(LPARAM)TranslateW(L"Set my blog status"));
			}

			dwTime=DB_Mra_GetDword(psbsdData->hContact,DBSETTING_BLOGSTATUSTIME,0);
			if (dwTime && MakeLocalSystemTimeFromTime32(dwTime,&stBlogStatusTime))
			{
				mir_sntprintf(szBuff,SIZEOF(szBuff),L"%s: %04ld.%02ld.%02ld %02ld:%02ld",TranslateW(L"Writed"),stBlogStatusTime.wYear,stBlogStatusTime.wMonth,stBlogStatusTime.wDay,stBlogStatusTime.wHour,stBlogStatusTime.wMinute);
			}else{
				szBuff[0]=0;
			}
			SET_DLG_ITEM_TEXT(hWndDlg,IDC_STATIC_WRITED_TIME,szBuff);

			EnableWindow(GetDlgItem(hWndDlg,IDC_CHK_NOTIFY),(psbsdData->hContact==NULL));

			iRet=TRUE;
		}
		break;
    case WM_CLOSE:
      DestroyWindow(hWndDlg);
      break;
    case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDOK:
			{
				DWORD dwFlags;
				WCHAR szBuff[MICBLOG_STATUS_MAX];
				SIZE_T dwBuffSize;
				DWORDLONG dwBlogStatusID;

				dwBuffSize=GET_DLG_ITEM_TEXT(hWndDlg,IDC_MSG_TO_SEND,szBuff,SIZEOF(szBuff));
				if (psbsdData->hContact)
				{
					dwFlags=(MRIM_BLOG_STATUS_REPLY|MRIM_BLOG_STATUS_NOTIFY);
					DB_Mra_GetContactSettingBlob(psbsdData->hContact,DBSETTING_BLOGSTATUSID,&dwBlogStatusID,sizeof(DWORDLONG),NULL);
				}else{
					dwFlags=MRIM_BLOG_STATUS_UPDATE;
					if (IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_CHK_NOTIFY)) dwFlags|=MRIM_BLOG_STATUS_NOTIFY;
					dwBlogStatusID=0;
				}
				MraSendCommand_ChangeUserBlogStatus(dwFlags,szBuff,dwBuffSize,dwBlogStatusID);
			}
		case IDCANCEL:
			DestroyWindow(hWndDlg);
			break;
		case IDC_MSG_TO_SEND:
			if (HIWORD(wParam)==EN_CHANGE)
			{
				WCHAR wszBuff[MAX_PATH];
				SIZE_T dwMessageSize=GET_DLG_ITEM_TEXT_LENGTH(hWndDlg,IDC_MSG_TO_SEND);

				EnableWindow(GetDlgItem(hWndDlg,IDOK),dwMessageSize);
				mir_sntprintf(wszBuff,SIZEOF(wszBuff),L"%d/%d",dwMessageSize,MICBLOG_STATUS_MAX);
				SET_DLG_ITEM_TEXTW(hWndDlg,IDC_STATIC_CHARS_COUNTER,wszBuff);
			}
			break;			
		}
		break;
    case WM_DESTROY:
		if (psbsdData)
		{
			HWND hWndEdit;
			WNDPROC OldMessageEditProc;

			SetWindowLongPtr(hWndDlg,GWLP_USERDATA,(LONG_PTR)0);

			hWndEdit=GetDlgItem(hWndDlg,IDC_MSG_TO_SEND);
			OldMessageEditProc=(WNDPROC)GetWindowLongPtr(hWndEdit,GWLP_USERDATA);
			SetWindowLongPtr(hWndEdit,GWLP_WNDPROC,(LONG_PTR)OldMessageEditProc);
			SetWindowLongPtr(hWndEdit,GWLP_USERDATA,(LONG_PTR)0);


			MEMFREE(psbsdData);
		}
		EndDialog(hWndDlg,NO_ERROR);
		break;
	//default:
	//	bRet=DefWindowProc(hWndDlg,message,wParam,lParam);
	//	break;
	}
return(iRet);
}


DWORD GetYears(CONST PSYSTEMTIME pcstSystemTime)
{
	DWORD dwRet=0;

	if (pcstSystemTime)
	{
		SYSTEMTIME stTime;

		GetLocalTime(&stTime);
		dwRet=(stTime.wYear-pcstSystemTime->wYear);
		if (stTime.wMonth<pcstSystemTime->wMonth)
		{// день рожденья будет в след месяце
			dwRet--;
		}else{// др ещё будет в этом месяце или уже был...
			if (stTime.wMonth==pcstSystemTime->wMonth)
			{// др в этом месяце
				if (stTime.wDay<pcstSystemTime->wDay)
				{// ещё только будет, не сегодня
					dwRet--;
				}// др ещё будет сегодня или уже был...
			}
		}
	}
return(dwRet);
}



DWORD FindFile(LPWSTR lpszFolder,DWORD dwFolderLen,LPWSTR lpszFileName,DWORD dwFileNameLen,LPWSTR lpszRetFilePathName,DWORD dwRetFilePathLen,DWORD *pdwRetFilePathLen)
{
	DWORD dwRetErrorCode;

	if (lpszFolder && dwFolderLen && lpszFileName && dwFileNameLen)
	{
		WCHAR szPath[32768];
		DWORD dwPathLen,dwRecDeepAllocated,dwRecDeepCurPos,dwFilePathLen;
		RECURSION_DATA_STACK_ITEM *prdsiItems;

		if (dwFolderLen==-1) dwFolderLen=lstrlenW(lpszFolder);
		if (dwFileNameLen==-1) dwFileNameLen=lstrlenW(lpszFileName);

		dwRecDeepCurPos=0;
		dwRecDeepAllocated=RECURSION_DATA_STACK_ITEMS_MIN;
		prdsiItems=(RECURSION_DATA_STACK_ITEM*)MEMALLOC(dwRecDeepAllocated*sizeof(RECURSION_DATA_STACK_ITEM));
		if (prdsiItems)
		{
			dwPathLen=dwFolderLen;
			memmove(szPath,lpszFolder,(dwPathLen*sizeof(WCHAR)));
			if (szPath[(dwPathLen-1)]!='\\')
			{
				szPath[dwPathLen]='\\';
				dwPathLen++;
			}
			szPath[dwPathLen]=0;
			lstrcatW(szPath,L"*.*");

			prdsiItems[dwRecDeepCurPos].dwFileNameLen=0;
			prdsiItems[dwRecDeepCurPos].hFind=FindFirstFileEx(szPath,FindExInfoStandard,&prdsiItems[dwRecDeepCurPos].w32fdFindFileData,FindExSearchNameMatch,NULL,0);
			if (prdsiItems[dwRecDeepCurPos].hFind!=INVALID_HANDLE_VALUE)
			{
				dwRetErrorCode=ERROR_FILE_NOT_FOUND;
				do
				{
					dwPathLen-=prdsiItems[dwRecDeepCurPos].dwFileNameLen;

					while(dwRetErrorCode==ERROR_FILE_NOT_FOUND && FindNextFile(prdsiItems[dwRecDeepCurPos].hFind,&prdsiItems[dwRecDeepCurPos].w32fdFindFileData))
					{
						if (prdsiItems[dwRecDeepCurPos].w32fdFindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
						{// folder
							if (CompareString(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName,-1,L".",1)!=CSTR_EQUAL)
							if (CompareString(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName,-1,L"..",2)!=CSTR_EQUAL)
							{
								prdsiItems[dwRecDeepCurPos].dwFileNameLen=(lstrlenW(prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName)+1);
								memmove((szPath+dwPathLen),prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName,(prdsiItems[dwRecDeepCurPos].dwFileNameLen*sizeof(WCHAR)));
								lstrcatW(szPath,L"\\*.*");
								dwPathLen+=prdsiItems[dwRecDeepCurPos].dwFileNameLen;

								dwRecDeepCurPos++;
								if (dwRecDeepCurPos==dwRecDeepAllocated)
								{// need more space
									dwRecDeepAllocated+=RECURSION_DATA_STACK_ITEMS_MIN;
									prdsiItems=(RECURSION_DATA_STACK_ITEM*)MEMREALLOC(prdsiItems,dwRecDeepAllocated*sizeof(RECURSION_DATA_STACK_ITEM));
									if (prdsiItems==NULL)
									{
										dwRecDeepCurPos=0;
										dwRetErrorCode=GetLastError();
										break;
									}
								}
								prdsiItems[dwRecDeepCurPos].hFind=FindFirstFileEx(szPath,FindExInfoStandard,&prdsiItems[dwRecDeepCurPos].w32fdFindFileData,FindExSearchNameMatch,NULL,0);
							}
						}else{// file
							if (CompareString(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName,-1,lpszFileName,dwFileNameLen)==CSTR_EQUAL)
							{
								prdsiItems[dwRecDeepCurPos].dwFileNameLen=lstrlenW(prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName);
								memmove((szPath+dwPathLen),prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName,((prdsiItems[dwRecDeepCurPos].dwFileNameLen+1)*sizeof(WCHAR)));
								dwFilePathLen=(dwPathLen+prdsiItems[dwRecDeepCurPos].dwFileNameLen);

								if (pdwRetFilePathLen) (*pdwRetFilePathLen)=dwFilePathLen;
								if (lpszRetFilePathName && dwRetFilePathLen)
								{
									dwFilePathLen=min(dwFilePathLen,dwRetFilePathLen);
									memmove(lpszRetFilePathName,szPath,((dwFilePathLen+1)*sizeof(WCHAR)));
								}

								dwRetErrorCode=NO_ERROR;
							}
						}
					}
					
					if (prdsiItems) FindClose(prdsiItems[dwRecDeepCurPos].hFind);
					dwRecDeepCurPos--;
				}while(dwRecDeepCurPos!=-1);
			}
			MEMFREE(prdsiItems);
		}else{
			dwRetErrorCode=GetLastError();
		}
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}


DWORD MemFillRandom(LPVOID lpBuff,SIZE_T dwBuffSize)
{
	DWORD dwRetErrorCode;

	if (lpBuff && dwBuffSize)
	{
		HCRYPTPROV hCryptProv=NULL;

		if (CryptAcquireContext(&hCryptProv,NULL,MS_STRONG_PROV,PROV_RSA_FULL,0)==FALSE && GetLastError()==NTE_BAD_KEYSET) CryptAcquireContext(&hCryptProv,NULL,MS_STRONG_PROV,PROV_RSA_FULL,CRYPT_NEWKEYSET);

		if (hCryptProv)
		{
			if (CryptGenRandom(hCryptProv,dwBuffSize,(BYTE*)lpBuff))
			{
				dwRetErrorCode=NO_ERROR;
			}else{
				dwRetErrorCode=GetLastError();
			}
		}else{
			dwRetErrorCode=GetLastError();
		}

		CryptReleaseContext(hCryptProv,0);
	}else{
		dwRetErrorCode=ERROR_INVALID_HANDLE;
	}
return(dwRetErrorCode);
}





#define MRA_PASS_CRYPT_VER	2



#if MRA_PASS_CRYPT_VER==1

BOOL SetPassDB(LPSTR lpszBuff,SIZE_T dwBuffSize)
{
	BOOL bRet=FALSE;
	BYTE btRandomData[256],btCryptedPass[256]={0},bthmacSHA1[SHA1HashSize]={0};
	char szEMail[MAX_EMAIL_LEN]={0};
	SIZE_T dwEMailSize;


#if /*defined (_DEBUG) ||*/ defined (REL_DEB)
	
	DB_Mra_SetStringExA(NULL,"Pass",lpszBuff,dwBuffSize);
	bRet=TRUE;
#else
	if (dwBuffSize<128)
	if (DB_Mra_GetStaticStringA(NULL,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
	{
		MemFillRandom(btRandomData,sizeof(btRandomData));

		SHA1GetDigest(lpszBuff,dwBuffSize,&btCryptedPass[1]);

		BASE64EncodeUnSafe(lpszBuff,dwBuffSize,&btCryptedPass[(1+SHA1HashSize)],(sizeof(btCryptedPass)-1),&dwBuffSize);
		btCryptedPass[0]=(BYTE)dwBuffSize;
		//memmove(&btCryptedPass[1],lpszBuff,dwBuffSize);

		hmac_sha1(btRandomData,sizeof(btRandomData),(BYTE*)szEMail,dwEMailSize,bthmacSHA1);

		RC4(btCryptedPass,sizeof(btCryptedPass),bthmacSHA1,SHA1HashSize);
		RC4(btCryptedPass,sizeof(btCryptedPass),btRandomData,sizeof(btRandomData));
		CopyMemoryReverseDWORD(btCryptedPass,btCryptedPass,sizeof(btCryptedPass));
		RC4(btCryptedPass,sizeof(btCryptedPass),bthmacSHA1,SHA1HashSize);

	
		DB_Mra_SetDword(NULL,"pCryptVer",MRA_PASS_CRYPT_VER);
		DB_Mra_WriteContactSettingBlob(NULL,"pCryptData",btRandomData,sizeof(btRandomData));
		DB_Mra_WriteContactSettingBlob(NULL,"pCryptPass",btCryptedPass,sizeof(btCryptedPass));
		
		bRet=TRUE;
	}
#endif
return(bRet);
}
#endif


#if MRA_PASS_CRYPT_VER==2
BOOL SetPassDB(LPSTR lpszBuff,SIZE_T dwBuffSize)
{
	BOOL bRet=FALSE;
	BYTE btRandomData[256],btCryptedPass[256]={0},bthmacSHA1[SHA1HashSize]={0};
	char szEMail[MAX_EMAIL_LEN]={0};
	SIZE_T dwEMailSize;


#if /*defined (_DEBUG) ||*/ defined (REL_DEB)
	
	DB_Mra_SetStringExA(NULL,"Pass",lpszBuff,dwBuffSize);
	bRet=TRUE;
#else
	if (dwBuffSize<128)
	if (DB_Mra_GetStaticStringA(NULL,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
	{
		MemFillRandom(btRandomData,sizeof(btRandomData));

		SHA1GetDigest(lpszBuff,dwBuffSize,&btCryptedPass[1]);

		//BASE64EncodeUnSafe(lpszBuff,dwBuffSize,&btCryptedPass[(1+SHA1HashSize)],(sizeof(btCryptedPass)-1),&dwBuffSize);
		memmove(&btCryptedPass[(1+SHA1HashSize)],lpszBuff,dwBuffSize);
		btCryptedPass[0]=(BYTE)dwBuffSize;
		//memmove(&btCryptedPass[1],lpszBuff,dwBuffSize);

		hmac_sha1(btRandomData,sizeof(btRandomData),(BYTE*)szEMail,dwEMailSize,bthmacSHA1);

		RC4(btCryptedPass,sizeof(btCryptedPass),bthmacSHA1,SHA1HashSize);
		RC4(btCryptedPass,sizeof(btCryptedPass),btRandomData,sizeof(btRandomData));
		CopyMemoryReverseDWORD(btCryptedPass,btCryptedPass,sizeof(btCryptedPass));
		RC4(btCryptedPass,sizeof(btCryptedPass),bthmacSHA1,SHA1HashSize);

	
		DB_Mra_SetDword(NULL,"pCryptVer",MRA_PASS_CRYPT_VER);
		DB_Mra_WriteContactSettingBlob(NULL,"pCryptData",btRandomData,sizeof(btRandomData));
		DB_Mra_WriteContactSettingBlob(NULL,"pCryptPass",btCryptedPass,sizeof(btCryptedPass));
		
		bRet=TRUE;
	}
#endif
return(bRet);
}
#endif


BOOL GetPassDB_v1(LPSTR lpszBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSize)
{
	BOOL bRet=FALSE;
	BYTE btRandomData[256]={0},btCryptedPass[256]={0},bthmacSHA1[SHA1HashSize]={0};
	char szEMail[MAX_EMAIL_LEN]={0};
	SIZE_T dwRandomDataSize,dwCryptedPass,dwEMailSize,dwPassSize;


	if (DB_Mra_GetDword(NULL,"pCryptVer",0)==1)
	if (DB_Mra_GetContactSettingBlob(NULL,"pCryptData",btRandomData,sizeof(btRandomData),&dwRandomDataSize))
	if (dwRandomDataSize==sizeof(btRandomData))
	if (DB_Mra_GetContactSettingBlob(NULL,"pCryptPass",btCryptedPass,sizeof(btCryptedPass),&dwCryptedPass))
	if (dwCryptedPass==sizeof(btCryptedPass))
	if (DB_Mra_GetStaticStringA(NULL,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
	{
		hmac_sha1(btRandomData,sizeof(btRandomData),(BYTE*)szEMail,dwEMailSize,bthmacSHA1);

		RC4(btCryptedPass,sizeof(btCryptedPass),bthmacSHA1,SHA1HashSize);
		CopyMemoryReverseDWORD(btCryptedPass,btCryptedPass,sizeof(btCryptedPass));
		RC4(btCryptedPass,sizeof(btCryptedPass),btRandomData,dwRandomDataSize);
		RC4(btCryptedPass,sizeof(btCryptedPass),bthmacSHA1,SHA1HashSize);

		dwPassSize=(*btCryptedPass);
		BASE64DecodeUnSafe(&btCryptedPass[(1+SHA1HashSize)],dwPassSize,&btCryptedPass[(1+SHA1HashSize)],(sizeof(btCryptedPass)-1),&dwPassSize);
		SHA1GetDigest(&btCryptedPass[(1+SHA1HashSize)],dwPassSize,btRandomData);
		if (MemoryCompare(&btCryptedPass[1],SHA1HashSize,btRandomData,SHA1HashSize)==CMEM_EQUAL)
		if (dwBuffSize>=dwPassSize)
		{
			memmove(lpszBuff,&btCryptedPass[(1+SHA1HashSize)],dwPassSize);
			(*(lpszBuff+dwPassSize))=0;

			if (pdwBuffSize) (*pdwBuffSize)=dwPassSize;
			bRet=TRUE;
		}
	}

return(bRet);
}


BOOL GetPassDB_v2(LPSTR lpszBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSize)
{
	BOOL bRet=FALSE;
	BYTE btRandomData[256]={0},btCryptedPass[256]={0},bthmacSHA1[SHA1HashSize]={0};
	char szEMail[MAX_EMAIL_LEN]={0};
	SIZE_T dwRandomDataSize,dwCryptedPass,dwEMailSize,dwPassSize;


	if (DB_Mra_GetDword(NULL,"pCryptVer",0)==2)
	if (DB_Mra_GetContactSettingBlob(NULL,"pCryptData",btRandomData,sizeof(btRandomData),&dwRandomDataSize))
	if (dwRandomDataSize==sizeof(btRandomData))
	if (DB_Mra_GetContactSettingBlob(NULL,"pCryptPass",btCryptedPass,sizeof(btCryptedPass),&dwCryptedPass))
	if (dwCryptedPass==sizeof(btCryptedPass))
	if (DB_Mra_GetStaticStringA(NULL,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
	{
		hmac_sha1(btRandomData,sizeof(btRandomData),(BYTE*)szEMail,dwEMailSize,bthmacSHA1);

		RC4(btCryptedPass,sizeof(btCryptedPass),bthmacSHA1,SHA1HashSize);
		CopyMemoryReverseDWORD(btCryptedPass,btCryptedPass,sizeof(btCryptedPass));
		RC4(btCryptedPass,sizeof(btCryptedPass),btRandomData,dwRandomDataSize);
		RC4(btCryptedPass,sizeof(btCryptedPass),bthmacSHA1,SHA1HashSize);

		dwPassSize=((*btCryptedPass)&0xff);
		SHA1GetDigest(&btCryptedPass[(1+SHA1HashSize)],dwPassSize,btRandomData);
		if (MemoryCompare(&btCryptedPass[1],SHA1HashSize,btRandomData,SHA1HashSize)==CMEM_EQUAL)
		if (dwBuffSize>=dwPassSize)
		{
			memmove(lpszBuff,&btCryptedPass[(1+SHA1HashSize)],dwPassSize);
			(*(lpszBuff+dwPassSize))=0;

			if (pdwBuffSize) (*pdwBuffSize)=dwPassSize;
			bRet=TRUE;
		}
	}

return(bRet);
}


BOOL GetPassDB(LPSTR lpszBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSize)
{
	BOOL bRet=FALSE;

#if /*defined (_DEBUG) ||*/ defined (REL_DEB)
	DB_Mra_GetStaticStringA(NULL,"Pass",lpszBuff,dwBuffSize,pdwBuffSize);
	bRet=TRUE;
#else
	switch (DB_Mra_GetDword(NULL,"pCryptVer",0)) {
	case 1:
		bRet=GetPassDB_v1(lpszBuff,dwBuffSize,pdwBuffSize);
		break;
	case 2:
		bRet=GetPassDB_v2(lpszBuff,dwBuffSize,pdwBuffSize);
		break;
	default:
		bRet=FALSE;
		break;
	}
#endif

return(bRet);
}




DWORD ReplaceInBuff(LPVOID lpInBuff,SIZE_T dwInBuffSize,SIZE_T dwReplaceItemsCount,LPVOID *plpInReplaceItems,SIZE_T *pdwInReplaceItemsCounts,LPVOID *plpOutReplaceItems,SIZE_T *pdwOutReplaceItemsCounts,LPVOID lpOutBuff,SIZE_T dwOutBuffSize,SIZE_T *pdwOutBuffSize)
{
	DWORD dwRetErrorCode=NO_ERROR;
	LPBYTE *plpbtFounded;
#ifdef _DEBUG //check tables
	for(SIZE_T i=0;i<dwReplaceItemsCount;i++)
	{
		if (lstrlen((LPTSTR)plpInReplaceItems[i])!=(pdwInReplaceItemsCounts[i]/sizeof(TCHAR))) DebugBreak();
		if (lstrlen((LPTSTR)plpOutReplaceItems[i])!=(pdwOutReplaceItemsCounts[i]/sizeof(TCHAR))) DebugBreak();
	}
#endif

	plpbtFounded=(LPBYTE*)MEMALLOC((sizeof(LPBYTE)*dwReplaceItemsCount));
	if (plpbtFounded)
	{
		LPBYTE lpbtOutBuffCur,lpbtInBuffCur,lpbtInBuffCurPrev,lpbtOutBuffMax;
		SIZE_T i,dwFirstFoundedIndex=0,dwFoundedCount=0,dwMemPartToCopy;

		lpbtInBuffCurPrev=(LPBYTE)lpInBuff;
		lpbtOutBuffCur=(LPBYTE)lpOutBuff;
		lpbtOutBuffMax=(((LPBYTE)lpOutBuff)+dwOutBuffSize);
		for(i=0;i<dwReplaceItemsCount;i++)
		{// loking for in first time
			plpbtFounded[i]=(LPBYTE)MemoryFind((lpbtInBuffCurPrev-(LPBYTE)lpInBuff),lpInBuff,dwInBuffSize,plpInReplaceItems[i],pdwInReplaceItemsCounts[i]);
			if (plpbtFounded[i]) dwFoundedCount++;
		}

		while(dwFoundedCount)
		{
			for(i=0;i<dwReplaceItemsCount;i++)
			{// looking for first to replace
				if (plpbtFounded[i] && (plpbtFounded[i]<plpbtFounded[dwFirstFoundedIndex] || plpbtFounded[dwFirstFoundedIndex]==NULL)) dwFirstFoundedIndex=i;
			}

			if (plpbtFounded[dwFirstFoundedIndex])
			{// in founded
				dwMemPartToCopy=(plpbtFounded[dwFirstFoundedIndex]-lpbtInBuffCurPrev);
				if (lpbtOutBuffMax>(lpbtOutBuffCur+(dwMemPartToCopy+pdwInReplaceItemsCounts[dwFirstFoundedIndex])))
				{
					memmove(lpbtOutBuffCur,lpbtInBuffCurPrev,dwMemPartToCopy);lpbtOutBuffCur+=dwMemPartToCopy;
					memmove(lpbtOutBuffCur,plpOutReplaceItems[dwFirstFoundedIndex],pdwOutReplaceItemsCounts[dwFirstFoundedIndex]);lpbtOutBuffCur+=pdwOutReplaceItemsCounts[dwFirstFoundedIndex];
					lpbtInBuffCurPrev=(plpbtFounded[dwFirstFoundedIndex]+pdwInReplaceItemsCounts[dwFirstFoundedIndex]);

					for(i=0;i<dwReplaceItemsCount;i++)
					{// loking for in next time
						if (plpbtFounded[i] && plpbtFounded[i]<lpbtInBuffCurPrev)
						{
							plpbtFounded[i]=(LPBYTE)MemoryFind((lpbtInBuffCurPrev-(LPBYTE)lpInBuff),lpInBuff,dwInBuffSize,plpInReplaceItems[i],pdwInReplaceItemsCounts[i]);
							if (plpbtFounded[i]==NULL) dwFoundedCount--;
						}
					}
				}else{
					dwRetErrorCode=ERROR_BUFFER_OVERFLOW;
					DebugBreak();
					break;
				}
			}else{// сюда по идее никогда не попадём, на всякий случай.
				DebugBreak();
				break;
			}
		}
		lpbtInBuffCur=(((LPBYTE)lpInBuff)+dwInBuffSize);
		memmove(lpbtOutBuffCur,lpbtInBuffCurPrev,(lpbtInBuffCur-lpbtInBuffCurPrev));lpbtOutBuffCur+=(lpbtInBuffCur-lpbtInBuffCurPrev);
		(*((WORD*)lpbtOutBuffCur))=0;

		MEMFREE(plpbtFounded);

		if (pdwOutBuffSize) (*pdwOutBuffSize)=(lpbtOutBuffCur-((LPBYTE)lpOutBuff));
	}else{
		dwRetErrorCode=GetLastError();
	}
return(dwRetErrorCode);
}


static const LPTSTR lpszXMLTags[]		={TEXT("&apos;"),		TEXT("&quot;"),		TEXT("&amp;"),		TEXT("&lt;"),		TEXT("&gt;")};
static const SIZE_T dwXMLTagsCount[]	={(6*sizeof(TCHAR)),	(6*sizeof(TCHAR)),	(5*sizeof(TCHAR)),	(4*sizeof(TCHAR)),	(4*sizeof(TCHAR))};
static const LPTSTR lpszXMLSymbols[]	={TEXT("\'"),			TEXT("\""),			TEXT("&"),			TEXT("<"),			TEXT(">")};
static const SIZE_T dwXMLSymbolsCount[]	={sizeof(TCHAR),		sizeof(TCHAR),		sizeof(TCHAR),		sizeof(TCHAR),		sizeof(TCHAR)};

//Decode XML coded string. The function translate special xml code into standard characters.
DWORD DecodeXML(LPTSTR lptszMessage,SIZE_T dwMessageSize,LPTSTR lptszMessageConverted,SIZE_T dwMessageConvertedBuffSize,SIZE_T *pdwMessageConvertedSize)
{
	DWORD dwRet=ReplaceInBuff(lptszMessage,(dwMessageSize*sizeof(TCHAR)),SIZEOF(lpszXMLTags),(LPVOID*)lpszXMLTags,(SIZE_T*)dwXMLTagsCount,(LPVOID*)lpszXMLSymbols,(SIZE_T*)dwXMLSymbolsCount,lptszMessageConverted,(dwMessageConvertedBuffSize*sizeof(TCHAR)),pdwMessageConvertedSize);

	if (pdwMessageConvertedSize) (*pdwMessageConvertedSize)/=sizeof(TCHAR);
return(dwRet);
}

//Encode XML coded string. The function translate special saved xml characters into special characters.
DWORD EncodeXML(LPTSTR lptszMessage,SIZE_T dwMessageSize,LPTSTR lptszMessageConverted,SIZE_T dwMessageConvertedBuffSize,SIZE_T *pdwMessageConvertedSize)
{
	DWORD dwRet=ReplaceInBuff(lptszMessage,(dwMessageSize*sizeof(TCHAR)),SIZEOF(lpszXMLTags),(LPVOID*)lpszXMLSymbols,(SIZE_T*)dwXMLSymbolsCount,(LPVOID*)lpszXMLTags,(SIZE_T*)dwXMLTagsCount,lptszMessageConverted,(dwMessageConvertedBuffSize*sizeof(TCHAR)),pdwMessageConvertedSize);
	
	if (pdwMessageConvertedSize) (*pdwMessageConvertedSize)/=sizeof(TCHAR);
return(dwRet);
}

