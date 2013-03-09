#include "common.h"

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
			CopyMemory(lpwszRetBuff,dbv.pszVal,(dwReadedStringLen*sizeof(WCHAR)));//include null terminated
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
			CopyMemory(lpwszValueLocal,lpwszValue,(dwValueSize*sizeof(WCHAR)));
			bRet=(CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws)==0);
			
			MEMFREE(lpwszValueLocal);
		}
	}else{
		bRet=TRUE;
		DBDeleteContactSetting(hContact,lpszModule,lpszValueName);
	}
return(bRet);
}


LPSTR GetModuleName(HANDLE hContact)
{
	LPSTR lpszRet;

	if (hContact)
	{
		lpszRet=(LPSTR)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
		if (lpszRet==NULL) lpszRet=PROTOCOL_NAMEA;
	}else{
		lpszRet=PROTOCOL_NAMEA;
	}
return(lpszRet);
}


void EnableControlsArray(HWND hWndDlg,WORD *pwControlsList,SIZE_T dwControlsListCount,BOOL bEnabled)
{
	for(SIZE_T i=0;i<dwControlsListCount;i++) EnableWindow(GetDlgItem(hWndDlg,pwControlsList[i]),bEnabled);
}


void CListShowMenuItem(HANDLE hMenuItem,BOOL bShow)
{
	CLISTMENUITEM mi={0};

	mi.cbSize=sizeof(mi);
	mi.flags=CMIM_FLAGS;
	if (bShow==FALSE) mi.flags|=CMIF_HIDDEN;

	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hMenuItem,(LPARAM)&mi);
}


//This function gets a Cellular string szPhone and clean it from symbools.
SIZE_T CopyNumberA(LPSTR lpszOutBuff,LPSTR lpszBuff,SIZE_T dwLen)
{
	BYTE btChar;
	LPBYTE lpbOutBuff=(LPBYTE)lpszOutBuff,lpbInBuff=(LPBYTE)lpszBuff;

	for(SIZE_T i=0;i<dwLen;i++)
	{
		btChar=(*lpbInBuff++);
		if (btChar>='0' && btChar<='9') (*lpbOutBuff++)=btChar;
	}
	(*lpbOutBuff)=0;

return((lpbOutBuff-(LPBYTE)lpszOutBuff));
}

SIZE_T CopyNumberW(LPWSTR lpcOutBuff,LPWSTR lpcBuff,SIZE_T dwLen)
{
	WCHAR wChar;
	LPWSTR lpwszOutBuff=lpcOutBuff,lpwszInBuff=lpcBuff;

	for(SIZE_T i=0;i<dwLen;i++)
	{
		wChar=(*lpwszInBuff++);
		if (wChar>='0' && wChar<='9') (*lpwszOutBuff++)=wChar;
	}
	(*lpwszOutBuff)=0;

return((lpwszOutBuff-lpcOutBuff));
}


bool IsPhoneW(LPWSTR lpwszString,SIZE_T dwStringLen)
{
	if (dwStringLen <= 1)
		return false;

	for(SIZE_T i=0; i < dwStringLen; i++) {
		WCHAR wChar=(*lpwszString++);
		if (wChar<'0' || wChar>'9')
			if (wChar!='+' && wChar!='S' && wChar!='M' && wChar!=' ' && wChar!='(' && wChar!=')')
				return false;
	}
	return true;
}


DWORD GetContactPhonesCountParam(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName)
{
	DWORD dwRet=0;
	char szBuff[MAX_PATH];
	WCHAR wszPhone[MAX_PHONE_LEN];
	SIZE_T i,dwPhoneSize;

	if ( DB_GetStaticStringW(hContact,lpszModule,lpszValueName,wszPhone,SIZEOF(wszPhone),&dwPhoneSize))
		if ( IsPhoneW(wszPhone,dwPhoneSize))
			dwRet++;

	for (i=0; i <= PHONES_MIN_COUNT; i++) {
		mir_snprintf(szBuff,sizeof(szBuff),"%s%ld",lpszValueName,i);
		if ( DB_GetStaticStringW(hContact,lpszModule,szBuff,wszPhone,SIZEOF(wszPhone),&dwPhoneSize))
			if ( IsPhoneW(wszPhone,dwPhoneSize))
				dwRet++;
	}
	return dwRet;
}


DWORD GetContactPhonesCount(HANDLE hContact)
{
	DWORD dwRet = 0;
	LPSTR lpszProto = GetContactProto(hContact);
	if (lpszProto) {
		dwRet += GetContactPhonesCountParam(hContact,lpszProto,"Phone");
		dwRet += GetContactPhonesCountParam(hContact,lpszProto,"Cellular");
		dwRet += GetContactPhonesCountParam(hContact,lpszProto,"Fax");
	}
	dwRet += GetContactPhonesCountParam(hContact,"UserInfo","MyPhone");
	dwRet += GetContactPhonesCountParam(hContact,"UserInfo","Phone");
	dwRet += GetContactPhonesCountParam(hContact,"UserInfo","Cellular");
	dwRet += GetContactPhonesCountParam(hContact,"UserInfo","Fax");
	return dwRet;
}


BOOL IsContactPhoneParam(HANDLE hContact,LPSTR lpszModule,LPSTR lpszValueName,LPWSTR lpwszPhone,SIZE_T dwPhoneSize)
{
	char szBuff[MAX_PATH];
	WCHAR wszPhoneLocal[MAX_PHONE_LEN];
	SIZE_T i,dwPhoneSizeLocal;

	if ( DB_GetStaticStringW(hContact,lpszModule,lpszValueName,wszPhoneLocal,SIZEOF(wszPhoneLocal),&dwPhoneSizeLocal))
	if ( IsPhoneW(wszPhoneLocal,dwPhoneSizeLocal)) {
		dwPhoneSizeLocal = CopyNumberW(wszPhoneLocal,wszPhoneLocal,dwPhoneSizeLocal);
		if (MemoryCompare(wszPhoneLocal,dwPhoneSizeLocal,lpwszPhone,dwPhoneSize)==CSTR_EQUAL)
			return TRUE;
	}

	for (i=0; i <= PHONES_MIN_COUNT; i++) {
		mir_snprintf(szBuff,sizeof(szBuff),"%s%ld",lpszValueName,i);
		if ( DB_GetStaticStringW(hContact,lpszModule,szBuff,wszPhoneLocal,SIZEOF(wszPhoneLocal),&dwPhoneSizeLocal)) {
			if (IsPhoneW(wszPhoneLocal,dwPhoneSizeLocal)) {
				dwPhoneSizeLocal=CopyNumberW(wszPhoneLocal,wszPhoneLocal,dwPhoneSizeLocal);
				if (MemoryCompare(wszPhoneLocal,dwPhoneSizeLocal,lpwszPhone,dwPhoneSize) == CSTR_EQUAL)
					return TRUE;
			}
		}
	}
	return FALSE;
}


BOOL IsContactPhone(HANDLE hContact,LPWSTR lpwszPhone,SIZE_T dwPhoneSize)
{
	BOOL bRet=FALSE;
	WCHAR wszPhoneLocal[MAX_PHONE_LEN];
	LPSTR lpszProto;
	SIZE_T dwPhoneSizeLocal;

	dwPhoneSizeLocal=CopyNumberW(wszPhoneLocal,lpwszPhone,dwPhoneSize);
	lpszProto=(LPSTR)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
	if (lpszProto) {
		if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,lpszProto,"Phone",wszPhoneLocal,dwPhoneSizeLocal);
		if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,lpszProto,"Cellular",wszPhoneLocal,dwPhoneSizeLocal);
		if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,lpszProto,"Fax",wszPhoneLocal,dwPhoneSizeLocal);
	}
	if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,"UserInfo","MyPhone",wszPhoneLocal,dwPhoneSizeLocal);
	if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,"UserInfo","Phone",wszPhoneLocal,dwPhoneSizeLocal);
	if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,"UserInfo","Cellular",wszPhoneLocal,dwPhoneSizeLocal);
	if (bRet==FALSE) bRet=IsContactPhoneParam(hContact,"UserInfo","Fax",wszPhoneLocal,dwPhoneSizeLocal);

	return bRet;
}


//This function get a string cellular number and return the HANDLE of the contact that has this
//number in the miranda phonebook (and marked as an SMS able) at the User Details.
//If no one has this number function returns NULL.
HANDLE HContactFromPhone(LPWSTR lpwszPhone,SIZE_T dwPhoneSize)
{
	if (lpwszPhone && dwPhoneSize) {
		//check not already on list
		for (HANDLE hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
			if (IsContactPhone(hContact,lpwszPhone,dwPhoneSize))
				return hContact;
	}

	return NULL;
}


BOOL GetDataFromMessage(LPSTR lpszMessage,SIZE_T dwMessageSize,DWORD *pdwEventType,LPWSTR lpwszPhone,SIZE_T dwPhoneSize,SIZE_T *pdwPhoneSizeRet,UINT *piIcon)
{
	BOOL bRet=FALSE;

	DWORD dwEventTypeRet=0;
	SIZE_T dwPhoneSizeRet=0;
	UINT iIconRet=0;

	if (lpszMessage && dwMessageSize) {
		if ( MemoryCompare(lpszMessage,10,"SMS From: ",10) == CSTR_EQUAL) {
			LPSTR lpsz = (LPSTR)strstr(lpszMessage+10, "\r\n");
			if (lpsz) {
				if (lpwszPhone && dwPhoneSize) {
					dwPhoneSizeRet = MultiByteToWideChar(CP_UTF8,0,(lpszMessage+10),min((SIZE_T)(lpsz-(lpszMessage+10)),dwPhoneSize),lpwszPhone,dwPhoneSize);
					dwPhoneSizeRet = CopyNumberW(lpwszPhone, lpwszPhone, dwPhoneSizeRet);
				}
			}
			iIconRet = 0;
			dwEventTypeRet = ICQEVENTTYPE_SMS;
			bRet = TRUE;
		}
		else if (MemoryCompare(lpszMessage,23,"SMS Confirmation From: ",23) == CSTR_EQUAL) {
			LPSTR lpsz = (LPSTR)strstr(lpszMessage+23, "\r\n");
			if (lpsz) {
				if (lpwszPhone && dwPhoneSize) {
					dwPhoneSizeRet = MultiByteToWideChar(CP_UTF8,0,(lpszMessage+23),min((SIZE_T)(lpsz-(lpszMessage+23)),dwPhoneSize),lpwszPhone,dwPhoneSize);
					dwPhoneSizeRet = CopyNumberW(lpwszPhone, lpwszPhone, dwPhoneSizeRet);
				}

				lpsz += 2;
				if (MemoryCompare(lpsz,24,"SMS was sent succesfully",24)==CSTR_EQUAL)
					iIconRet = IDI_SMSSENT;
				else
					iIconRet = IDI_SMSNOTSENT;
			}
			dwEventTypeRet = ICQEVENTTYPE_SMSCONFIRMATION;
			bRet = TRUE;
		}
	}

	if (pdwPhoneSizeRet)	*pdwPhoneSizeRet = dwPhoneSizeRet;
	if (pdwEventType)		*pdwEventType = dwEventTypeRet;
	if (piIcon)				*piIcon = iIconRet;

	return bRet;
}

BOOL GetXMLFieldEx(LPSTR lpszXML,SIZE_T dwXMLSize,LPSTR *plpszData,SIZE_T *pdwDataSize,const char *tag1,...)
{
	BOOL bRet = FALSE;
	int thisLevel = 0;
	LPSTR lpszFindTag = (LPSTR)tag1, lpszTagEnd = lpszXML, lpszDataStart = NULL;

	va_list va;
	va_start(va,tag1);
	while (TRUE) {
		LPSTR lpszTagStart = (LPSTR)MemoryFindByte(lpszTagEnd-lpszXML, lpszXML, dwXMLSize, '<');
		if (!lpszTagStart)
			break;

		lpszTagEnd = (LPSTR)MemoryFindByte(lpszTagStart-lpszXML, lpszXML, dwXMLSize, '>');
		if (!lpszTagEnd)
			break;

		lpszTagStart++;
		lpszTagEnd--;
		if ((*((BYTE*)lpszTagStart)) == '/') {
			if (--thisLevel < 0) {
				if (lpszDataStart) {
					if (plpszData) (*plpszData)=lpszDataStart;
					if (pdwDataSize) (*pdwDataSize)=((lpszTagStart-1)-lpszDataStart);
					bRet=TRUE;
				}
				break;
			}
		}
		else {
			if (++thisLevel==1)
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpszFindTag,-1,lpszTagStart,((lpszTagEnd+1)-lpszTagStart))==CSTR_EQUAL) {
				lpszFindTag=va_arg(va,LPSTR);
				if (lpszFindTag==NULL) lpszDataStart=(lpszTagEnd+2);
				thisLevel=0;
			}
		}
	}
	va_end(va);
	return bRet;
}


BOOL GetXMLFieldExBuff(LPSTR lpszXML,SIZE_T dwXMLSize,LPSTR lpszBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSizeRet,const char *tag1,...)
{
	BOOL bRet=FALSE;
	int thisLevel=0;
	LPSTR lpszFindTag=(LPSTR)tag1,lpszTagStart,lpszTagEnd=lpszXML,lpszDataStart=NULL;
	va_list va;


	va_start(va,tag1);
	while (TRUE)
	{
		lpszTagStart=(LPSTR)MemoryFindByte((lpszTagEnd-lpszXML),lpszXML,dwXMLSize,'<');
		if (lpszTagStart)
		{
			lpszTagEnd=(LPSTR)MemoryFindByte((lpszTagStart-lpszXML),lpszXML,dwXMLSize,'>');
			if (lpszTagEnd)
			{
				lpszTagStart++;
				lpszTagEnd--;
				if ((*((BYTE*)lpszTagStart))=='/')
				{
					if (--thisLevel<0)
					{
						if (lpszDataStart)
						{
							SIZE_T dwBuffSizeRet=min((dwBuffSize-2),(SIZE_T)((lpszTagStart-1)-lpszDataStart));
							if (lpszBuff && dwBuffSize) CopyMemory(lpszBuff,lpszDataStart,dwBuffSizeRet);(*((WORD*)(lpszBuff+dwBuffSizeRet)))=0;
							if (pdwBuffSizeRet) (*pdwBuffSizeRet)=dwBuffSizeRet;
							bRet=TRUE;
						}
						break;
					}
				}else{
					if (++thisLevel==1)
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpszFindTag,-1,lpszTagStart,((lpszTagEnd+1)-lpszTagStart))==CSTR_EQUAL)
					{
						lpszFindTag=va_arg(va,LPSTR);
						if (lpszFindTag==NULL) lpszDataStart=(lpszTagEnd+2);
						thisLevel=0;
					}
				}
			}else{
				break;
			}
		}else{
			break;
		}
	}
	va_end(va);


	if (bRet==FALSE)
	{
		if (lpszBuff)		(*((WORD*)lpszBuff))=0;
		if (pdwBuffSizeRet)	(*pdwBuffSizeRet)=0;
	}

return(bRet);
}


/*BOOL GetXMLFieldExW(LPWSTR lpwszXML,SIZE_T dwXMLSize,LPWSTR *plpwszData,SIZE_T *pdwDataSize,const WCHAR *tag1,...)
{
	BOOL bRet=FALSE;
	int thisLevel=0;
	LPWSTR lpwszFindTag=(LPWSTR)tag1,lpwszTagStart,lpwszTagEnd=lpwszXML,lpwszDataStart=NULL;
	va_list va;

	va_start(va,tag1);
	while (TRUE)
	{
		lpwszTagStart=(LPWSTR)MemoryFind((lpwszTagEnd-lpwszXML),lpwszXML,dwXMLSize,_T("<"),2);
		if (lpwszTagStart)
		{
			lpwszTagEnd=(LPWSTR)MemoryFind((lpwszTagStart-lpwszXML),lpwszXML,dwXMLSize,_T(">"),2);
			if (lpwszTagEnd)
			{
				lpwszTagStart++;
				lpwszTagEnd--;
				if ((*((WORD*)lpwszTagStart))==(*((WORD*)"/")))
				{
					if (--thisLevel<0)
					{
						if (lpwszDataStart)
						{
							if (plpwszData) (*plpwszData)=lpwszDataStart;
							if (pdwDataSize) (*pdwDataSize)=((lpwszTagStart-1)-lpwszDataStart);
							bRet=TRUE;
						}
						break;
					}
				}else{
					if (++thisLevel==1)
					if (CompareStringW(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpwszFindTag,-1,lpwszTagStart,((lpwszTagEnd+1)-lpwszTagStart))==CSTR_EQUAL)
					{
						lpwszFindTag=va_arg(va,LPWSTR);
						if (lpwszFindTag==NULL) lpwszDataStart=(lpwszTagEnd+2);
						thisLevel=0;
					}
				}
			}else{
				break;
			}
		}else{
			break;
		}
	}
	va_end(va);
return(bRet);
}


BOOL GetXMLFieldExBuffW(LPWSTR lpwszXML,SIZE_T dwXMLSize,LPWSTR lpwszBuff,SIZE_T dwBuffSize,SIZE_T *pdwBuffSizeRet,const WCHAR *tag1,...)
{
	BOOL bRet;
	LPWSTR lpwszData;
	SIZE_T dwDataSize;

	if ((bRet=GetXMLFieldExW(lpwszXML,dwXMLSize,&lpwszData,&dwDataSize,tag1)))
	{
		SIZE_T dwBuffSizeRet=min((dwBuffSize-2),dwDataSize);

		if (lpwszBuff && dwBuffSize) CopyMemory(lpwszBuff,lpwszData,dwBuffSizeRet);(*((WORD*)(lpwszBuff+dwBuffSizeRet)))=0;
		if (pdwBuffSizeRet) (*pdwBuffSizeRet)=dwBuffSizeRet;
	}else{
		if (lpwszBuff)		(*((WORD*)lpwszBuff))=0;
		if (pdwBuffSizeRet)	(*pdwBuffSizeRet)=0;
	}

return(bRet);
}//*/


DWORD ReplaceInBuff(LPVOID lpInBuff,SIZE_T dwInBuffSize,SIZE_T dwReplaceItemsCount,LPVOID *plpInReplaceItems,SIZE_T *pdwInReplaceItemsCounts,LPVOID *plpOutReplaceItems,SIZE_T *pdwOutReplaceItemsCounts,LPVOID lpOutBuff,SIZE_T dwOutBuffSize,SIZE_T *pdwOutBuffSize)
{
	DWORD dwRetErrorCode=NO_ERROR;
	LPBYTE *plpszFound,lpszMessageConvertedCur,lpszMessageCur,lpszMessageCurPrev,lpszMessageConvertedMax;
	SIZE_T i,dwFirstFoundedIndex=0,dwFoundedCount=0,dwMemPartToCopy;

	plpszFound=(LPBYTE*)MEMALLOC((sizeof(LPBYTE)*dwReplaceItemsCount));

	lpszMessageCurPrev=(LPBYTE)lpInBuff;
	lpszMessageConvertedCur=(LPBYTE)lpOutBuff;
	lpszMessageConvertedMax=(((LPBYTE)lpOutBuff)+dwOutBuffSize);

	// looking for in first time
	for (i=0; i < dwReplaceItemsCount; i++) {
		plpszFound[i] = (LPBYTE)strstr((LPCSTR)lpInBuff + (lpszMessageCurPrev-(LPBYTE)lpInBuff), (LPCSTR)plpInReplaceItems[i]);
		if (plpszFound[i])
			dwFoundedCount++;
	}

	while (dwFoundedCount) {
		// looking for first to replace
		for(i=0; i < dwReplaceItemsCount; i++)
			if (plpszFound[i] && (plpszFound[i] < plpszFound[dwFirstFoundedIndex] || plpszFound[dwFirstFoundedIndex] == NULL))
				dwFirstFoundedIndex = i;

		// in founded
		if (plpszFound[dwFirstFoundedIndex]) {
			dwMemPartToCopy=(plpszFound[dwFirstFoundedIndex]-lpszMessageCurPrev);
			if (lpszMessageConvertedMax > (lpszMessageConvertedCur+(dwMemPartToCopy+pdwInReplaceItemsCounts[dwFirstFoundedIndex]))) {
				CopyMemory(lpszMessageConvertedCur,lpszMessageCurPrev,dwMemPartToCopy);lpszMessageConvertedCur+=dwMemPartToCopy;
				CopyMemory(lpszMessageConvertedCur,plpOutReplaceItems[dwFirstFoundedIndex],pdwOutReplaceItemsCounts[dwFirstFoundedIndex]);lpszMessageConvertedCur+=pdwOutReplaceItemsCounts[dwFirstFoundedIndex];
				lpszMessageCurPrev=(plpszFound[dwFirstFoundedIndex]+pdwInReplaceItemsCounts[dwFirstFoundedIndex]);

				for(i=0;i<dwReplaceItemsCount;i++)
				{// loking for in next time
					if (plpszFound[i] && plpszFound[i]<lpszMessageCurPrev)
					{
						dwFoundedCount--;
						plpszFound[i] = (LPBYTE)strstr((LPCSTR)lpInBuff + (lpszMessageCurPrev-(LPBYTE)lpInBuff), (LPCSTR)plpInReplaceItems[i]);
						if (plpszFound[i]) dwFoundedCount++;
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
	lpszMessageCur=(((LPBYTE)lpInBuff)+dwInBuffSize);
	CopyMemory(lpszMessageConvertedCur,lpszMessageCurPrev,(lpszMessageCur-lpszMessageCurPrev));lpszMessageConvertedCur+=(lpszMessageCur-lpszMessageCurPrev);
	(*((WORD*)lpszMessageConvertedCur))=0;

	MEMFREE(plpszFound);

	if (pdwOutBuffSize) (*pdwOutBuffSize)=(lpszMessageConvertedCur-((LPBYTE)lpOutBuff));

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


//(Taken from Miranda-IM source code:)
BYTE MsgDlgGetFontDefaultCharset(const TCHAR* szFont)
{
	return(DEFAULT_CHARSET);
}


void LoadMsgDlgFont(int i,LOGFONT *lf,COLORREF *colour)
{
	int style;
	char str[MAX_PATH];
	DBVARIANT dbv;

	if (colour)
	{
		mir_snprintf(str,sizeof(str),"Font%dCol",i);
		(*colour)=DBGetContactSettingDword(NULL,SRMMMOD,str,fontOptionsList[0].defColour);
	}

	if (lf)
	{
		if (DBGetContactSetting(NULL,SRMMMOD,str,&dbv))
		{
			lstrcpyn(lf->lfFaceName,fontOptionsList[0].szDefFace,SIZEOF(lf->lfFaceName));
		}else{
			lstrcpyn(lf->lfFaceName,dbv.ptszVal,SIZEOF(lf->lfFaceName));
			DBFreeVariant(&dbv);
		}

		mir_snprintf(str,sizeof(str),"Font%dSize",i);
		lf->lfHeight=(char)DBGetContactSettingByte(NULL,SRMMMOD,str,fontOptionsList[0].defSize);
		lf->lfWidth=0;
		lf->lfEscapement=0;
		lf->lfOrientation=0;
		mir_snprintf(str,sizeof(str),"Font%dSty",i);
		style=DBGetContactSettingByte(NULL,SRMMMOD,str,fontOptionsList[0].defStyle);
		lf->lfWeight=style&FONTF_BOLD?FW_BOLD:FW_NORMAL;
		lf->lfItalic=style&FONTF_ITALIC?1:0;
		lf->lfUnderline=0;
		lf->lfStrikeOut=0;
		mir_snprintf(str,sizeof(str),"Font%dSet",i);
		lf->lfCharSet=DBGetContactSettingByte(NULL,SRMMMOD,str,MsgDlgGetFontDefaultCharset(lf->lfFaceName));
		lf->lfOutPrecision=OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision=CLIP_DEFAULT_PRECIS;
		lf->lfQuality=DEFAULT_QUALITY;
		lf->lfPitchAndFamily=DEFAULT_PITCH|FF_DONTCARE;
		mir_snprintf(str,sizeof(str),"Font%d",i);
	}
}

LRESULT CALLBACK MessageSubclassProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	LRESULT lrRet=0;

	switch(message){
	case WM_CHAR:
		if (wParam=='\n' && GetKeyState(VK_CONTROL)&0x8000)
			PostMessage(GetParent(hwnd),WM_COMMAND,IDOK,0);
		return 0;
	}

	return mir_callNextSubclass(hwnd, MessageSubclassProc, message, wParam, lParam);
}

//This function refresh account list. 
//It called when SMS plugin loaded and upon change in the account list.
int RefreshAccountList(WPARAM eventCode,LPARAM lParam)
{
	SIZE_T dwAccCount=0,i,dwSMSAccountsCount=0;
	PROTOACCOUNT **ppaAccounts;

	ProtoEnumAccounts((int*)&dwAccCount,&ppaAccounts);

	FreeAccountList();
	ssSMSSettings.ppaSMSAccounts=(PROTOACCOUNT**)MEMALLOC((dwAccCount*sizeof(LPVOID)));
	if (ssSMSSettings.ppaSMSAccounts)
	{
		char szServiceName[MAX_PATH];

		for (i=0;i<dwAccCount;i++)
		{
			if (IsAccountEnabled(ppaAccounts[i]))
			{
				mir_snprintf(szServiceName,sizeof(szServiceName),"%s%s",ppaAccounts[i]->szModuleName,MS_ICQ_SENDSMS);
				if (ServiceExists(szServiceName)) 
				{
					ssSMSSettings.ppaSMSAccounts[dwSMSAccountsCount++]=ppaAccounts[i];
				}
			}
		}
	}
	ssSMSSettings.dwSMSAccountsCount=dwSMSAccountsCount;
	SendSMSWindowsUpdateAllAccountLists();

return 0;
}

//This function free the global account list. 
//This function should be called before list refresh or when SMS plugin unloaded.
void FreeAccountList()
{
	MEMFREE(ssSMSSettings.ppaSMSAccounts);
	ssSMSSettings.dwSMSAccountsCount=0;
}

//This function check if the module is in the account list that supports sms sending in miranda.
/*BOOL IsModuleInAccountList(LPSTR lpszModule)
{
	BOOL bRet=FALSE;

	if (ssSMSSettings.ppaSMSAccounts && ssSMSSettings.dwSMSAccountsCount)
	{
		for(SIZE_T i=0;i<ssSMSSettings.dwSMSAccountsCount;i++)
		{
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,ssSMSSettings.ppaSMSAccounts[i]->szModuleName,-1,lpszModule,-1)==CSTR_EQUAL)
			{
				bRet=TRUE;
				break;
			}
		}
	}
return(bRet);
}//*/

