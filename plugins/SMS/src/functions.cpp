#include "common.h"

BOOL DB_GetStaticStringW(MCONTACT hContact,LPSTR lpszModule,LPSTR lpszValueName,LPWSTR lpwszRetBuff,size_t dwRetBuffSize,size_t *pdwRetBuffSize)
{// sizes in wchars
	BOOL bRet=FALSE;
	size_t dwReadedStringLen;
	DBVARIANT dbv={0};
	if (db_get_ws(hContact, lpszModule, lpszValueName, &dbv)==0)
	{
		dwReadedStringLen=mir_wstrlen(dbv.pwszVal);
		if (lpwszRetBuff && (dwRetBuffSize>dwReadedStringLen))
		{
			memcpy(lpwszRetBuff,dbv.pszVal,(dwReadedStringLen*sizeof(WCHAR)));//include null terminated
			(*((WCHAR*)(lpwszRetBuff+dwReadedStringLen)))=0;
			bRet=TRUE;
		}else{
			if (lpwszRetBuff && dwRetBuffSize>=sizeof(WCHAR)) (*((WCHAR*)lpwszRetBuff))=0;
		}
		if (pdwRetBuffSize) (*pdwRetBuffSize)=dwReadedStringLen;

		db_free(&dbv);
	}else{
		if (lpwszRetBuff && dwRetBuffSize>=sizeof(WCHAR)) (*((WCHAR*)lpwszRetBuff))=0;
		if (pdwRetBuffSize)	(*pdwRetBuffSize)=0;
	}
	return(bRet);
}

LPSTR GetModuleName(MCONTACT hContact)
{
	LPSTR lpszRet;

	if (hContact) {
		lpszRet = GetContactProto(hContact);
		if (lpszRet == NULL)
			lpszRet = PROTOCOL_NAMEA;
	}
	else lpszRet = PROTOCOL_NAMEA;

	return lpszRet;
}

void EnableControlsArray(HWND hWndDlg,WORD *pwControlsList,size_t dwControlsListCount,BOOL bEnabled)
{
	for(size_t i=0;i<dwControlsListCount;i++) EnableWindow(GetDlgItem(hWndDlg,pwControlsList[i]),bEnabled);
}

//This function gets a Cellular string szPhone and clean it from symbools.
size_t CopyNumberA(LPSTR lpszOutBuff,LPSTR lpszBuff,size_t dwLen)
{
	BYTE btChar;
	LPBYTE lpbOutBuff=(LPBYTE)lpszOutBuff,lpbInBuff=(LPBYTE)lpszBuff;

	for(size_t i=0;i<dwLen;i++)
	{
		btChar=(*lpbInBuff++);
		if (btChar>='0' && btChar<='9') (*lpbOutBuff++)=btChar;
	}
	(*lpbOutBuff)=0;

	return((lpbOutBuff-(LPBYTE)lpszOutBuff));
}

size_t CopyNumberW(LPWSTR lpcOutBuff,LPWSTR lpcBuff,size_t dwLen)
{
	WCHAR wChar;
	LPWSTR lpwszOutBuff=lpcOutBuff,lpwszInBuff=lpcBuff;

	for(size_t i=0;i<dwLen;i++)
	{
		wChar=(*lpwszInBuff++);
		if (wChar>='0' && wChar<='9') (*lpwszOutBuff++)=wChar;
	}
	(*lpwszOutBuff)=0;

	return((lpwszOutBuff-lpcOutBuff));
}


bool IsPhoneW(LPWSTR lpwszString,size_t dwStringLen)
{
	if (dwStringLen <= 1)
		return false;

	for(size_t i=0; i < dwStringLen; i++) {
		WCHAR wChar=(*lpwszString++);
		if (wChar<'0' || wChar>'9')
			if (wChar!='+' && wChar!='S' && wChar!='M' && wChar!=' ' && wChar!='(' && wChar!=')')
				return false;
	}
	return true;
}


DWORD GetContactPhonesCountParam(MCONTACT hContact,LPSTR lpszModule,LPSTR lpszValueName)
{
	DWORD dwRet=0;
	char szBuff[MAX_PATH];
	WCHAR wszPhone[MAX_PHONE_LEN];
	size_t i,dwPhoneSize;

	if ( DB_GetStaticStringW(hContact,lpszModule,lpszValueName,wszPhone,SIZEOF(wszPhone),&dwPhoneSize))
		if ( IsPhoneW(wszPhone,dwPhoneSize))
			dwRet++;

	for (i=0; i <= PHONES_MIN_COUNT; i++) {
		mir_snprintf(szBuff,SIZEOF(szBuff),"%s%ld",lpszValueName,i);
		if ( DB_GetStaticStringW(hContact,lpszModule,szBuff,wszPhone,SIZEOF(wszPhone),&dwPhoneSize))
			if ( IsPhoneW(wszPhone,dwPhoneSize))
				dwRet++;
	}
	return dwRet;
}


DWORD GetContactPhonesCount(MCONTACT hContact)
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


BOOL IsContactPhoneParam(MCONTACT hContact,LPSTR lpszModule,LPSTR lpszValueName,LPWSTR lpwszPhone,size_t dwPhoneSize)
{
	char szBuff[MAX_PATH];
	WCHAR wszPhoneLocal[MAX_PHONE_LEN];
	size_t i,dwPhoneSizeLocal;

	if ( DB_GetStaticStringW(hContact,lpszModule,lpszValueName,wszPhoneLocal,SIZEOF(wszPhoneLocal),&dwPhoneSizeLocal))
		if ( IsPhoneW(wszPhoneLocal,dwPhoneSizeLocal)) {
			dwPhoneSizeLocal = CopyNumberW(wszPhoneLocal,wszPhoneLocal,dwPhoneSizeLocal);
			if (MemoryCompare(wszPhoneLocal,dwPhoneSizeLocal,lpwszPhone,dwPhoneSize)==CSTR_EQUAL)
				return TRUE;
		}

		for (i=0; i <= PHONES_MIN_COUNT; i++) {
			mir_snprintf(szBuff,SIZEOF(szBuff),"%s%ld",lpszValueName,i);
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


BOOL IsContactPhone(MCONTACT hContact,LPWSTR lpwszPhone,size_t dwPhoneSize)
{
	BOOL bRet=FALSE;
	WCHAR wszPhoneLocal[MAX_PHONE_LEN];

	size_t dwPhoneSizeLocal = CopyNumberW(wszPhoneLocal,lpwszPhone,dwPhoneSize);
	LPSTR lpszProto = GetContactProto(hContact);
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
MCONTACT HContactFromPhone(LPWSTR lpwszPhone,size_t dwPhoneSize)
{
	if (lpwszPhone && dwPhoneSize) {
		//check not already on list
		for (MCONTACT hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
			if (IsContactPhone(hContact,lpwszPhone,dwPhoneSize))
				return hContact;
	}

	return NULL;
}


BOOL GetDataFromMessage(LPSTR lpszMessage,size_t dwMessageSize,DWORD *pdwEventType,LPWSTR lpwszPhone,size_t dwPhoneSize,size_t *pdwPhoneSizeRet,UINT *piIcon)
{
	BOOL bRet=FALSE;

	DWORD dwEventTypeRet=0;
	size_t dwPhoneSizeRet=0;
	UINT iIconRet=0;

	if (lpszMessage && dwMessageSize) {
		if ( MemoryCompare(lpszMessage,10,"SMS From: ",10) == CSTR_EQUAL) {
			LPSTR lpsz = (LPSTR)strstr(lpszMessage+10, "\r\n");
			if (lpsz) {
				if (lpwszPhone && dwPhoneSize) {
					dwPhoneSizeRet = MultiByteToWideChar(CP_UTF8,0,(lpszMessage+10),min((size_t)(lpsz-(lpszMessage+10)),dwPhoneSize),lpwszPhone,dwPhoneSize);
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
					dwPhoneSizeRet = MultiByteToWideChar(CP_UTF8,0,(lpszMessage+23),min((size_t)(lpsz-(lpszMessage+23)),dwPhoneSize),lpwszPhone,dwPhoneSize);
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

BOOL GetXMLFieldEx(LPSTR lpszXML,size_t dwXMLSize,LPSTR *plpszData,size_t *pdwDataSize,const char *tag1,...)
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


BOOL GetXMLFieldExBuff(LPSTR lpszXML,size_t dwXMLSize,LPSTR lpszBuff,size_t dwBuffSize,size_t *pdwBuffSizeRet,const char *tag1,...)
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
							size_t dwBuffSizeRet=min((dwBuffSize-2),(size_t)((lpszTagStart-1)-lpszDataStart));
							if (lpszBuff && dwBuffSize) memcpy(lpszBuff,lpszDataStart,dwBuffSizeRet);(*((WORD*)(lpszBuff+dwBuffSizeRet)))=0;
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

DWORD ReplaceInBuff(LPVOID lpInBuff,size_t dwInBuffSize,size_t dwReplaceItemsCount,LPVOID *plpInReplaceItems,size_t *pdwInReplaceItemsCounts,LPVOID *plpOutReplaceItems,size_t *pdwOutReplaceItemsCounts,LPVOID lpOutBuff,size_t dwOutBuffSize,size_t *pdwOutBuffSize)
{
	DWORD dwRetErrorCode=NO_ERROR;
	LPBYTE *plpszFound,lpszMessageConvertedCur,lpszMessageCur,lpszMessageCurPrev,lpszMessageConvertedMax;
	size_t i,dwFirstFoundedIndex=0,dwFoundedCount=0,dwMemPartToCopy;

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
				memcpy(lpszMessageConvertedCur,lpszMessageCurPrev,dwMemPartToCopy);lpszMessageConvertedCur+=dwMemPartToCopy;
				memcpy(lpszMessageConvertedCur,plpOutReplaceItems[dwFirstFoundedIndex],pdwOutReplaceItemsCounts[dwFirstFoundedIndex]);lpszMessageConvertedCur+=pdwOutReplaceItemsCounts[dwFirstFoundedIndex];
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
	memcpy(lpszMessageConvertedCur,lpszMessageCurPrev,(lpszMessageCur-lpszMessageCurPrev));lpszMessageConvertedCur+=(lpszMessageCur-lpszMessageCurPrev);
	(*((WORD*)lpszMessageConvertedCur))=0;

	MEMFREE(plpszFound);

	if (pdwOutBuffSize) (*pdwOutBuffSize)=(lpszMessageConvertedCur-((LPBYTE)lpOutBuff));

	return(dwRetErrorCode);
}


static const LPTSTR lpszXMLTags[]		={TEXT("&apos;"),		TEXT("&quot;"),		TEXT("&amp;"),		TEXT("&lt;"),		TEXT("&gt;")};
static const size_t dwXMLTagsCount[]	={(6*sizeof(TCHAR)),	(6*sizeof(TCHAR)),	(5*sizeof(TCHAR)),	(4*sizeof(TCHAR)),	(4*sizeof(TCHAR))};
static const LPTSTR lpszXMLSymbols[]	={TEXT("\'"),			TEXT("\""),			TEXT("&"),			TEXT("<"),			TEXT(">")};
static const size_t dwXMLSymbolsCount[]	={sizeof(TCHAR),		sizeof(TCHAR),		sizeof(TCHAR),		sizeof(TCHAR),		sizeof(TCHAR)};

//Decode XML coded string. The function translate special xml code into standard characters.
DWORD DecodeXML(LPTSTR lptszMessage,size_t dwMessageSize,LPTSTR lptszMessageConverted,size_t dwMessageConvertedBuffSize,size_t *pdwMessageConvertedSize)
{
	DWORD dwRet=ReplaceInBuff(lptszMessage,(dwMessageSize*sizeof(TCHAR)),SIZEOF(lpszXMLTags),(LPVOID*)lpszXMLTags,(size_t*)dwXMLTagsCount,(LPVOID*)lpszXMLSymbols,(size_t*)dwXMLSymbolsCount,lptszMessageConverted,(dwMessageConvertedBuffSize*sizeof(TCHAR)),pdwMessageConvertedSize);

	if (pdwMessageConvertedSize) (*pdwMessageConvertedSize)/=sizeof(TCHAR);
	return(dwRet);
}

//Encode XML coded string. The function translate special saved xml characters into special characters.
DWORD EncodeXML(LPTSTR lptszMessage,size_t dwMessageSize,LPTSTR lptszMessageConverted,size_t dwMessageConvertedBuffSize,size_t *pdwMessageConvertedSize)
{
	DWORD dwRet=ReplaceInBuff(lptszMessage,(dwMessageSize*sizeof(TCHAR)),SIZEOF(lpszXMLTags),(LPVOID*)lpszXMLSymbols,(size_t*)dwXMLSymbolsCount,(LPVOID*)lpszXMLTags,(size_t*)dwXMLTagsCount,lptszMessageConverted,(dwMessageConvertedBuffSize*sizeof(TCHAR)),pdwMessageConvertedSize);

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
		mir_snprintf(str,SIZEOF(str),"Font%dCol",i);
		(*colour)=db_get_dw(NULL,SRMMMOD,str,fontOptionsList[0].defColour);
	}

	if (lf)
	{
		if (db_get(NULL,SRMMMOD,str,&dbv))
		{
			mir_tstrncpy(lf->lfFaceName,fontOptionsList[0].szDefFace,SIZEOF(lf->lfFaceName));
		}else{
			mir_tstrncpy(lf->lfFaceName,dbv.ptszVal,SIZEOF(lf->lfFaceName));
			db_free(&dbv);
		}

		mir_snprintf(str,SIZEOF(str),"Font%dSize",i);
		lf->lfHeight=(char)db_get_b(NULL,SRMMMOD,str,fontOptionsList[0].defSize);
		lf->lfWidth=0;
		lf->lfEscapement=0;
		lf->lfOrientation=0;
		mir_snprintf(str,SIZEOF(str),"Font%dSty",i);
		style=db_get_b(NULL,SRMMMOD,str,fontOptionsList[0].defStyle);
		lf->lfWeight=style&FONTF_BOLD?FW_BOLD:FW_NORMAL;
		lf->lfItalic=style&FONTF_ITALIC?1:0;
		lf->lfUnderline=0;
		lf->lfStrikeOut=0;
		mir_snprintf(str,SIZEOF(str),"Font%dSet",i);
		lf->lfCharSet=db_get_b(NULL,SRMMMOD,str,MsgDlgGetFontDefaultCharset(lf->lfFaceName));
		lf->lfOutPrecision=OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision=CLIP_DEFAULT_PRECIS;
		lf->lfQuality=DEFAULT_QUALITY;
		lf->lfPitchAndFamily=DEFAULT_PITCH|FF_DONTCARE;
		mir_snprintf(str,SIZEOF(str),"Font%d",i);
	}
}

LRESULT CALLBACK MessageSubclassProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
	case WM_CHAR: 
		if (wParam == '\n' && GetKeyState(VK_CONTROL) & 0x8000) {
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			return 0;
		}
	}

	return mir_callNextSubclass(hwnd, MessageSubclassProc, message, wParam, lParam);
}

//This function refresh account list. 
//It called when SMS plugin loaded and upon change in the account list.
int RefreshAccountList(WPARAM eventCode,LPARAM lParam)
{
	int dwAccCount=0,dwSMSAccountsCount=0;
	PROTOACCOUNT **ppaAccounts;

	ProtoEnumAccounts((int*)&dwAccCount,&ppaAccounts);

	FreeAccountList();
	ssSMSSettings.ppaSMSAccounts=(PROTOACCOUNT**)MEMALLOC((dwAccCount*sizeof(LPVOID)));
	if (ssSMSSettings.ppaSMSAccounts)
		for (int i=0; i < dwAccCount; i++)
			if ( IsAccountEnabled(ppaAccounts[i]))
				if ( ProtoServiceExists(ppaAccounts[i]->szModuleName,MS_ICQ_SENDSMS)) 
					ssSMSSettings.ppaSMSAccounts[dwSMSAccountsCount++] = ppaAccounts[i];

	ssSMSSettings.dwSMSAccountsCount = dwSMSAccountsCount;
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
