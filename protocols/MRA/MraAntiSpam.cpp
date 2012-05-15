// started: 19.12.2006

#include "Mra.h"
#include "MraAntiSpam.h"
#include "MraRTFMsg.h"


typedef struct
{
	LPWSTR			lpwszBadWord;
	DWORD			dwBadWordLen;
} MRA_ANTISPAM_BAD_WORD;

static MRA_ANTISPAM_BAD_WORD *pmabwBadWords=NULL;
static SIZE_T dwBadWordsCount=0;



#define MRA_ANTISPAM_PREALLOC_COUNT	256
SIZE_T MraAntiSpamLoadBadWordsW()
{
	SIZE_T dwRet=0;

	char szSettingName[MAX_PATH];
	SIZE_T i=0,dwValueSize,dwAllocatedItemsCount=MRA_ANTISPAM_PREALLOC_COUNT;
	DBVARIANT dbv={0};
	DBCONTACTGETSETTING sVal={0};

	dbv.type=DBVT_WCHAR;
	sVal.pValue=&dbv;
	sVal.szModule=PROTOCOL_NAMEA;
	sVal.szSetting=szSettingName;


	if (pmabwBadWords || dwBadWordsCount) MraAntiSpamFreeBadWords();

	pmabwBadWords=(MRA_ANTISPAM_BAD_WORD*)MEMALLOC((sizeof(MRA_ANTISPAM_BAD_WORD)*dwAllocatedItemsCount));

	for(i=0;TRUE;i++)
	{
		mir_snprintf(szSettingName,SIZEOF(szSettingName),"AntiSpamBadWord %lu",i);
		dbv.type=DBVT_WCHAR;
		if (CallService(MS_DB_CONTACT_GETSETTING_STR,NULL,(LPARAM)&sVal))
		{
			break;
		}else{
			if (dwAllocatedItemsCount==i)
			{
				dwAllocatedItemsCount+=MRA_ANTISPAM_PREALLOC_COUNT;
				pmabwBadWords=(MRA_ANTISPAM_BAD_WORD*)MEMREALLOC(pmabwBadWords,(sizeof(MRA_ANTISPAM_BAD_WORD)*dwAllocatedItemsCount));
				if (pmabwBadWords==NULL) break;
			}

			dwValueSize=lstrlenW(dbv.pwszVal);
			pmabwBadWords[i].lpwszBadWord=(LPWSTR)MEMALLOC((dwValueSize*sizeof(WCHAR)));
			if (pmabwBadWords[i].lpwszBadWord)
			{//BuffToLowerCase(szValue,dbv.pszVal,dwValueSize);
				memmove(pmabwBadWords[i].lpwszBadWord,dbv.pwszVal,(dwValueSize*sizeof(WCHAR)));
				CharLowerBuffW(pmabwBadWords[i].lpwszBadWord,dwValueSize);
				pmabwBadWords[i].dwBadWordLen=dwValueSize;
			}
			DBFreeVariant(&dbv);

			if (pmabwBadWords[i].lpwszBadWord==NULL) break;
		}
	}
	dwBadWordsCount=i;

return(dwRet);
}


void MraAntiSpamFreeBadWords()
{
	SIZE_T i;

	for(i=0;i<dwBadWordsCount;i++)
	{
		if (pmabwBadWords[i].lpwszBadWord) MEMFREE(pmabwBadWords[i].lpwszBadWord);
	}

	MEMFREE(pmabwBadWords);
	dwBadWordsCount=0;
}




void MraAntiSpamEnableControls(HWND hWndDlg,BOOL bEnabled)
{
	WORD wMraAntiSpamControlsList[]={
							IDC_CHK_TEMP_CONTACTS,
							IDC_CLN_NON_ALPHNUM,
							IDC_MAX_LANG_CHANGES,
							IDC_SHOWPOPUP,
							IDC_WRITETOSYSTEMHISTORY,
							IDC_SEND_SPAM_REPORT_TO_SERVER,
							IDC_BAD_WORDS_LIST,
							IDC_EDIT_BAD_WORD,
							IDC_BUTTON_ADD,
							IDC_BUTTONREMOVE,
							IDC_BUTTON_DEFAULT
	};

	EnableControlsArray(hWndDlg,(WORD*)&wMraAntiSpamControlsList,SIZEOF(wMraAntiSpamControlsList),bEnabled);
	EnableWindow(GetDlgItem(hWndDlg,IDC_DELETE_SPAMBOT_CONTACT),(bEnabled && IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_CHK_TEMP_CONTACTS)));
}

INT_PTR CALLBACK MraAntiSpamDlgProcOpts(HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg){
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hWndDlg);

			CHECK_DLG_BUTTON(hWndDlg,IDC_ENABLE,DB_Mra_GetByte(NULL,"AntiSpamEnable",MRA_ANTISPAM_DEFAULT_ENABLE));
			CHECK_DLG_BUTTON(hWndDlg,IDC_CHK_TEMP_CONTACTS,DB_Mra_GetByte(NULL,"AntiSpamCheckTempContacts",MRA_ANTISPAM_DEFAULT_CHK_TEMP_CONTACTS));
			CHECK_DLG_BUTTON(hWndDlg,IDC_DELETE_SPAMBOT_CONTACT,DB_Mra_GetByte(NULL,"AntiSpamDeteleSpamBotContacts",MRA_ANTISPAM_DEFAULT_DELETE_SPAMBOT_CONTACT));
			CHECK_DLG_BUTTON(hWndDlg,IDC_CLN_NON_ALPHNUM,DB_Mra_GetByte(NULL,"AntiSpamCleanNonAlphaNumeric",MRA_ANTISPAM_DEFAULT_CLN_NON_ALPHNUM));
			SetDlgItemInt(hWndDlg,IDC_MAX_LANG_CHANGES,DB_Mra_GetDword(NULL,"AntiSpamMaxLangChanges",MRA_ANTISPAM_DEFAULT_MAX_LNG_CHANGES),FALSE);
			CHECK_DLG_BUTTON(hWndDlg,IDC_SHOWPOPUP,DB_Mra_GetByte(NULL,"AntiSpamShowPopUp",MRA_ANTISPAM_DEFAULT_SHOWPOP));
			CHECK_DLG_BUTTON(hWndDlg,IDC_WRITETOSYSTEMHISTORY,DB_Mra_GetByte(NULL,"AntiSpamWriteToSystemHistory",MRA_ANTISPAM_DEFAULT_WRITETOSYSTEMHISTORY));
			CHECK_DLG_BUTTON(hWndDlg,IDC_SEND_SPAM_REPORT_TO_SERVER,DB_Mra_GetByte(NULL,"AntiSpamSendSpamReportToSrv",MRA_ANTISPAM_DEFAULT_SEND_SPAM_REPORT_TO_SERVER));


			{// fill list
				char szSettingName[MAX_PATH];
				WCHAR wszBuff[MAX_PATH];

				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_RESETCONTENT,0,0);
				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_INITSTORAGE,(WPARAM)1024,(LPARAM)32768);
				for(SIZE_T i=0;TRUE;i++)
				{
					mir_snprintf(szSettingName,SIZEOF(szSettingName),"AntiSpamBadWord %lu",i);
					if (DB_Mra_GetStaticStringW(NULL,szSettingName,wszBuff,SIZEOF(wszBuff),NULL))
					{
						if (SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_FINDSTRING,-1,(LPARAM)wszBuff)==LB_ERR) SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_ADDSTRING,0,(LPARAM)wszBuff);
					}else{
						break;
					}
				}
			}
			//SET_DLG_ITEM_TEXTW(hWndDlg,IDC_EDIT_BAD_WORD,L"");

			MraAntiSpamEnableControls(hWndDlg,IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_ENABLE));
		}
		return(TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam)==IDC_ENABLE)
		{
			MraAntiSpamEnableControls(hWndDlg,IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_ENABLE));
		}

		if (LOWORD(wParam)==IDC_CHK_TEMP_CONTACTS)
		{
			EnableWindow(GetDlgItem(hWndDlg,IDC_DELETE_SPAMBOT_CONTACT),IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_CHK_TEMP_CONTACTS));
		}

		if (LOWORD(wParam)==IDC_BUTTON_ADD)
		{
			WCHAR szBadWord[MAX_PATH];
			
			if (GET_DLG_ITEM_TEXT(hWndDlg,IDC_EDIT_BAD_WORD,szBadWord,SIZEOF(szBadWord)))
			{
				SET_DLG_ITEM_TEXTW(hWndDlg,IDC_EDIT_BAD_WORD,L"");
				if (SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_FINDSTRING,-1,(LPARAM)szBadWord)==LB_ERR) SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_ADDSTRING,0,(LPARAM)szBadWord);
			}
		}

		if (LOWORD(wParam)==IDC_BUTTONREMOVE)
		{
			SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_DELETESTRING,SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_GETCURSEL,0,0),0);
		}

		if (LOWORD(wParam)==IDC_BUTTON_DEFAULT)
		{
			MraAntiSpamResetBadWordsList();
			{// fill list
				char szSettingName[MAX_PATH];
				WCHAR wszBuff[MAX_PATH];

				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_RESETCONTENT,0,0);
				SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_INITSTORAGE,(WPARAM)1024,(LPARAM)32768);
				for(SIZE_T i=0;TRUE;i++)
				{
					mir_snprintf(szSettingName,SIZEOF(szSettingName),"AntiSpamBadWord %lu",i);
					if (DB_Mra_GetStaticStringW(NULL,szSettingName,wszBuff,SIZEOF(wszBuff),NULL))
					{
						if (SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_FINDSTRING,-1,(LPARAM)wszBuff)==LB_ERR) SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_ADDSTRING,0,(LPARAM)wszBuff);
					}else{
						break;
					}
				}
			}
			MraAntiSpamLoadBadWordsW();
		}

		if ((LOWORD(wParam)==IDC_EDIT_BAD_WORD) && (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()) ) return(0);
		SendMessage(GetParent(hWndDlg),PSM_CHANGED,0,0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code){
		case PSN_APPLY:
			{
				char szSettingName[MAX_PATH];
				WCHAR szBadWord[MAX_PATH];
				SIZE_T i,dwCount;

				DBWriteContactSettingByte(NULL,PROTOCOL_NAMEA,"AntiSpamEnable",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_ENABLE));
				DBWriteContactSettingByte(NULL,PROTOCOL_NAMEA,"AntiSpamCheckTempContacts",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_CHK_TEMP_CONTACTS));
				DBWriteContactSettingByte(NULL,PROTOCOL_NAMEA,"AntiSpamDeteleSpamBotContacts",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_DELETE_SPAMBOT_CONTACT));
				DBWriteContactSettingByte(NULL,PROTOCOL_NAMEA,"AntiSpamCleanNonAlphaNumeric",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_CLN_NON_ALPHNUM));
				DB_Mra_SetDword(NULL,"AntiSpamMaxLangChanges",(DWORD)GetDlgItemInt(hWndDlg,IDC_MAX_LANG_CHANGES,NULL,FALSE));
				DBWriteContactSettingByte(NULL,PROTOCOL_NAMEA,"AntiSpamShowPopUp",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_SHOWPOPUP));
				DBWriteContactSettingByte(NULL,PROTOCOL_NAMEA,"AntiSpamWriteToSystemHistory",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_WRITETOSYSTEMHISTORY));
				DBWriteContactSettingByte(NULL,PROTOCOL_NAMEA,"AntiSpamSendSpamReportToSrv",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_SEND_SPAM_REPORT_TO_SERVER));

				dwCount=SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_GETCOUNT,0,0);
				for(i=0;i<dwCount;i++)
				{
					mir_snprintf(szSettingName,SIZEOF(szSettingName),"AntiSpamBadWord %lu",i);
					SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_BAD_WORDS_LIST,LB_GETTEXT,i,(LPARAM)szBadWord);
					DB_Mra_SetStringW(NULL,szSettingName,szBadWord);
				}

				dwCount=((i*2)+1024);
				for(;i<dwCount;i++)
				{
					mir_snprintf(szSettingName,SIZEOF(szSettingName),"AntiSpamBadWord %lu",i);
					DB_Mra_DeleteValue(NULL,szSettingName);
				}

				MraAntiSpamLoadBadWordsW();
			}
			return(TRUE);
		}
		break;
	}
return(FALSE);
}


void MraAntiSpamResetBadWordsList()
{
	char szSettingName[MAX_PATH];
	SIZE_T i,dwMax;
	LPWSTR lpwszBadWords[]=
	{
		// web words
		L"@",L"http://",L"ftp://",L"www",L"wap.",L".com",L".org",L".info",L".net",L".ua",L".uz",L".to",L".biz",L".vu",L".ru",L"mailto:",L"ICQ",L"SKYPE",

		// spam words
		L"sex",L"seks",L"sekc",L"xxx",L"phorno",L"video",L"photo",L"SMS",L"WMZ",L"WMR",L"narod",L"ucoz",L"ref",L"Agent",L"+7",L"495",L"$",L"spam",L"URL",L"%2E",

		// spam words rus
		L"прода",L"цена",L"деньги",L"денег",L"секс",L"лесби",L"лезби",L"анал",L"порн",L"эротич",L"видео",L"фильм",L"кино",L"муз",L"фотки",L"фото",L"СМС",L"аськ",L"база",L"сайт",L"проект",L"рассылк",L"обьявлен",L"реклам",L"услуг",L"оплат",L"заказ",L"пиши",L"звони",L"работ",L"зараб",L"зайди",L"загляни",L"посети",L"посмотр",L"погляд",L"точка",L"тчк",L"спам",L"ссылк",L"майл",L"агент",L"супер",L"лох",L"регистрац",L"троян",L"вирус",L"взлом",

		// executable
		L".ade",L".adp",L".bas",L".bat",L".chm",L".cmd",L".com",L".cpl",L".crt",L".exe",L".hlp",L".hta",L".inf",L".ins",L".isp",L".js",L".jse",L".lnk",L".mdb",L".mde",L".msc",L".msi",L".msp",L".mst",L".pcd",L".pif",L".reg",L".scr",L".sct",L".shs",L".shb",L".url",L".vb",L".vbe",L".vbs",L".wsc",L".wsf",L".wsh",

		// archives
		L".zip",L".rar",L".cab",L".arj",L".tar",L".gz",L".zoo",L".lzh",L".jar",L".ace",L".z",L".iso",L".bz2",L".uue",L".gzip",

		NULL
	};

	for(i=0;lpwszBadWords[i];i++)
	{
		mir_snprintf(szSettingName,SIZEOF(szSettingName),"AntiSpamBadWord %lu",i);
		DB_Mra_SetStringW(NULL,szSettingName,lpwszBadWords[i]);
	}

	dwMax=(i+1024);
	for(;i<dwMax;i++)
	{
		mir_snprintf(szSettingName,SIZEOF(szSettingName),"AntiSpamBadWord %lu",i);
		DB_Mra_DeleteValue(NULL,szSettingName);
	}
}


BOOL MraAntiSpamIsCharExcepted(WCHAR wcTestChar)
{
	BOOL bRet=FALSE;
	SIZE_T i;
	LPWSTR lpwszExceptedChars=L"\r\n .,<>//?;:'\"\\][}{`-=~!@#$%^&*()_+№1234567890";
	
	//lstrlenW(lpwszExceptedChars)=46
	for(i=0;i<46;i++)
	{
		if (lpwszExceptedChars[i]==wcTestChar)
		{
			bRet=TRUE;
			break;
		}
	}
return(bRet);
}


SIZE_T MraAntiSpamCalcLangChanges(LPWSTR lpwszMessage,SIZE_T dwMessageSize)
{
	SIZE_T dwRet=0;

	if (lpwszMessage && dwMessageSize)
	{
		WCHAR wcPrevChar=0;
		SIZE_T i;

		for(i=1;i<dwMessageSize;i++)
		{
			if (MraAntiSpamIsCharExcepted(lpwszMessage[i])==FALSE)
			{
				if (wcPrevChar)
				if ( (max(wcPrevChar,lpwszMessage[i])-min(wcPrevChar,lpwszMessage[i])) > 255)
				{
					dwRet++;
				}
				wcPrevChar=lpwszMessage[i];
			}
		}
	}
return(dwRet);
}


SIZE_T MraAntiSpamCleanNonAlphaNumeric(LPWSTR lpwszMessage,SIZE_T dwMessageSize)
{
	SIZE_T dwRet=0;

	if (lpwszMessage && dwMessageSize)
	{
		LPWSTR lpwszCurWritePos=lpwszMessage;
		SIZE_T i;

		for(i=0;i<dwMessageSize;i++)
		{
			if (IsCharAlphaNumeric(lpwszMessage[i])) (*lpwszCurWritePos++)=lpwszMessage[i];
		}

		(*lpwszCurWritePos)=0;
		dwRet=(lpwszCurWritePos-lpwszMessage);
	}
return(dwRet);
}


BOOL MraAntiSpamTestMessageForBadWordsW(LPWSTR lpwszMessage,SIZE_T dwMessageSize)
{
	BOOL bRet=FALSE;

	if (lpwszMessage && dwMessageSize)
	{
		for(SIZE_T i=0;i<dwBadWordsCount;i++)
		{
			if (MemoryFind(0,lpwszMessage,(dwMessageSize*sizeof(WCHAR)),pmabwBadWords[i].lpwszBadWord,(pmabwBadWords[i].dwBadWordLen*sizeof(WCHAR))))
			{
				bRet=TRUE;
				break;
			}
		}
	}
return(bRet);
}



BOOL MraAntiSpamHasMessageBadWordsW(LPWSTR lpwszMessage,SIZE_T dwMessageSize)
{
	BOOL bRet=FALSE;

	if (lpwszMessage && dwMessageSize)
	{
		LPWSTR lpwszMessageConverted;

		lpwszMessageConverted=(LPWSTR)MEMALLOC((dwMessageSize*sizeof(WCHAR)));
		if (lpwszMessageConverted)
		{
			SIZE_T dwtm;

			/*{
				HANDLE hFile=CreateFile(L"C:\\Documents and Settings\\Rozhuk_IM\\Рабочий стол\\spam_raw.txt",GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

				if (hFile!=INVALID_HANDLE_VALUE)
				{
					WriteFile(hFile,lpwszMessageConverted,(dwMessageSize*sizeof(WCHAR)),&i,NULL);
					CloseHandle(hFile);
				}
			}//*/

			// в нижний регистр всё сообщение
			{//BuffToLowerCase(lpszMessageConverted,lpszMessage,dwMessageSize);
				memmove(lpwszMessageConverted,lpwszMessage,(dwMessageSize*sizeof(WCHAR)));
				CharLowerBuffW(lpwszMessageConverted,dwMessageSize);
			}

			// 1 проход: считаем колличество переключений языка
			dwtm=DB_Mra_GetDword(NULL,"AntiSpamMaxLangChanges",MRA_ANTISPAM_DEFAULT_MAX_LNG_CHANGES);
			if (dwtm)
			{
				if (dwtm<=MraAntiSpamCalcLangChanges(lpwszMessageConverted,dwMessageSize)) bRet=TRUE;
			}

			// 2 проход: ищем плохие слова
			if (bRet==FALSE) bRet=MraAntiSpamTestMessageForBadWordsW(lpwszMessageConverted,dwMessageSize);

			// 3 проход: оставляем только буквы + цифры и снова ищем плохие слова
			if (bRet==FALSE)
			if (DB_Mra_GetByte(NULL,"AntiSpamCleanNonAlphaNumeric",MRA_ANTISPAM_DEFAULT_ENABLE))
			{
				dwMessageSize=MraAntiSpamCleanNonAlphaNumeric(lpwszMessageConverted,dwMessageSize);
				bRet=MraAntiSpamTestMessageForBadWordsW(lpwszMessageConverted,dwMessageSize);
			}


			MEMFREE(lpwszMessageConverted);
		}
	}
return(bRet);
}


DWORD MraAntiSpamReceivedMessageW(LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwMessageFlags,LPWSTR lpwszMessage,SIZE_T dwMessageSize)
{
	DWORD dwRet=MESSAGE_NOT_SPAM;;

	if ((dwMessageFlags&(MESSAGE_FLAG_SYSTEM|MESSAGE_FLAG_CONTACT|MESSAGE_FLAG_NOTIFY|MESSAGE_FLAG_SMS|MESSAGE_SMS_DELIVERY_REPORT|MESSAGE_FLAG_ALARM|MESSAGE_FLAG_MULTICHAT))==0)
	if (DB_Mra_GetByte(NULL,"AntiSpamEnable",MRA_ANTISPAM_DEFAULT_ENABLE))
	if (IsEMailChatAgent(lpszEMail,dwEMailSize)==FALSE)
	{// enabled, message mast be cheked
		BOOL bCheckMessage=TRUE;
		HANDLE hContact;

		dwRet=MESSAGE_SPAM;
		hContact=MraHContactFromEmail(lpszEMail,dwEMailSize,FALSE,TRUE,NULL);
		if (hContact)
		{
			DWORD dwID,dwGroupID,dwContactFlag,dwContactSeverFlags;

			GetContactBasicInfoW(hContact,&dwID,&dwGroupID,&dwContactFlag,&dwContactSeverFlags,NULL,NULL,0,NULL,NULL,0,NULL,NULL,0,NULL);
			if ((dwID!=-1 && DBGetContactSettingByte(hContact,"CList","NotOnList",0)==0) || dwGroupID==-2)
			{// not temp contact, OK
				bCheckMessage=FALSE;
				dwRet=MESSAGE_NOT_SPAM;
			}else{// temp contact
				if (DB_Mra_GetByte(NULL,"AntiSpamCheckTempContacts",MRA_ANTISPAM_DEFAULT_CHK_TEMP_CONTACTS)==FALSE ||
					(((dwMessageFlags&MESSAGE_FLAG_AUTHORIZE) && DB_Mra_GetByte(NULL,"AutoAuthGrandNewUsers",MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS) && DB_Mra_GetByte(NULL,"AutoAuthGrandNewUsersDisableSPAMCheck",MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK))) )
				{// проверка временного контакта
					bCheckMessage=FALSE;
					dwRet=MESSAGE_NOT_SPAM;
				}
			}
		}

		if (bCheckMessage)
		{
			if (MraAntiSpamHasMessageBadWordsW(lpwszMessage,dwMessageSize)==FALSE)
			{// в простом тексте нет спама
				 dwRet=MESSAGE_NOT_SPAM;
			}
		}

		if (dwRet==MESSAGE_SPAM)
		{
			BOOL bAntiSpamShowPopUp,bAntiSpamWriteToSystemHistory,bAntiSpamDeteleSpamBotContacts;

			bAntiSpamShowPopUp=(BOOL)DB_Mra_GetByte(NULL,"AntiSpamShowPopUp",MRA_ANTISPAM_DEFAULT_SHOWPOP);
			bAntiSpamWriteToSystemHistory=(BOOL)DB_Mra_GetByte(NULL,"AntiSpamWriteToSystemHistory",MRA_ANTISPAM_DEFAULT_WRITETOSYSTEMHISTORY);
			bAntiSpamDeteleSpamBotContacts=(BOOL)DB_Mra_GetByte(NULL,"AntiSpamDeteleSpamBotContacts",MRA_ANTISPAM_DEFAULT_DELETE_SPAMBOT_CONTACT);

			if (bAntiSpamShowPopUp || bAntiSpamWriteToSystemHistory)
			{
				char szEMail[MAX_EMAIL_LEN];
				WCHAR wszBuff[MRA_MAXLENOFMESSAGE*2];
				SIZE_T dwDBMessageSize;

				memmove(szEMail,lpszEMail,dwEMailSize);
				(*((WORD*)(szEMail+dwEMailSize)))=0;

				dwDBMessageSize=mir_sntprintf(wszBuff,SIZEOF(wszBuff),L"%s: %S\r\n",TranslateW(L"Spam bot blocked"),szEMail);
				if (dwMessageSize>(SIZEOF(wszBuff)-(dwDBMessageSize+sizeof(DWORD)))) dwMessageSize=(SIZEOF(wszBuff)-(dwDBMessageSize+sizeof(DWORD)));
				memmove((wszBuff+dwDBMessageSize),lpwszMessage,(dwMessageSize*sizeof(WCHAR)));
				dwDBMessageSize+=dwMessageSize;
				(*((DWORD*)(wszBuff+dwDBMessageSize)))=0;
				dwDBMessageSize+=sizeof(DWORD);

				if (bAntiSpamShowPopUp) MraPopupShowFromAgentW(MRA_POPUP_TYPE_INFORMATION,0,wszBuff);

				if (bAntiSpamWriteToSystemHistory)
				{
					CHAR szBuff[MRA_MAXLENOFMESSAGE*2];
					DBEVENTINFO dbei={0};

					WideCharToMultiByte(CP_UTF8,0,wszBuff,dwDBMessageSize,szBuff,SIZEOF(szBuff),NULL,NULL);

					dbei.cbSize=sizeof(dbei);
					dbei.szModule=PROTOCOL_NAMEA;
					dbei.timestamp=_time32(NULL);
					dbei.flags=(DBEF_READ|DBEF_UTF);
					dbei.eventType=EVENTTYPE_MESSAGE;
					dbei.cbBlob=(dwDBMessageSize*sizeof(WCHAR));
					dbei.pBlob=(PBYTE)szBuff;

					CallService(MS_DB_EVENT_ADD,0,(LPARAM)&dbei);
				}

				if (hContact && bAntiSpamDeteleSpamBotContacts)
				{
					dwDBMessageSize=mir_sntprintf(wszBuff,SIZEOF(wszBuff),L"%s: %S",TranslateW(L"Spam bot contact deleted"),szEMail);
					
					if (bAntiSpamShowPopUp) MraPopupShowFromAgentW(MRA_POPUP_TYPE_INFORMATION,0,wszBuff);

					if (bAntiSpamWriteToSystemHistory)
					{
						CHAR szBuff[MRA_MAXLENOFMESSAGE*2];
						DBEVENTINFO dbei={0};

						(*((DWORD*)(wszBuff+dwDBMessageSize)))=0;
						dwDBMessageSize+=sizeof(DWORD);

						WideCharToMultiByte(CP_UTF8,0,wszBuff,dwDBMessageSize,szBuff,SIZEOF(szBuff),NULL,NULL);

						dbei.cbSize=sizeof(dbei);
						dbei.szModule=PROTOCOL_NAMEA;
						dbei.timestamp=_time32(NULL);
						dbei.flags=(DBEF_READ|DBEF_UTF);
						dbei.eventType=EVENTTYPE_MESSAGE;
						dbei.cbBlob=(dwDBMessageSize*sizeof(WCHAR));
						dbei.pBlob=(PBYTE)szBuff;

						CallService(MS_DB_EVENT_ADD,0,(LPARAM)&dbei);
					}
				}
			}

			if (hContact && bAntiSpamDeteleSpamBotContacts) CallService(MS_DB_CONTACT_DELETE,(WPARAM)hContact,0);
		}
	}
return(dwRet);
}
