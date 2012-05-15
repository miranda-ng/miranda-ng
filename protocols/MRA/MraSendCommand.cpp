#include "Mra.h"
#include "MraSendCommand.h"
#include "MraRTFMsg.h"
#include "proto.h"





void			SetUL				(LPBYTE *plpBuff,DWORD dwData);
void			SetUIDL				(LPBYTE *plpBuff,DWORDLONG dwData);
void			SetGUID				(LPBYTE *plpBuff,MRA_GUID guidData);
void			SetLPS				(LPBYTE *plpBuff,LPSTR lpszData,DWORD dwSize);
void			SetLPSW				(LPBYTE *plpBuff,LPWSTR lpwszData,DWORD dwSize);
void			SetLPSWtoA			(LPBYTE *plpBuff,LPWSTR lpwszData,DWORD dwSize);
void			SetLPSLowerCase		(LPBYTE *plpBuff,LPSTR lpszData,DWORD dwSize);
void			SetLPSLowerCaseW	(LPBYTE *plpBuff,LPWSTR lpwszData,DWORD dwSize);






DWORD MraSendCommand_MessageW(BOOL bAddToQueue,HANDLE hContact,DWORD dwAckType,DWORD dwFlags,LPSTR lpszEMail,SIZE_T dwEMailSize,LPWSTR lpwszMessage,SIZE_T dwMessageSize,LPBYTE lpbMultiChatData,SIZE_T dwMultiChatDataSize)
{// Сообщение
	DWORD dwRet=0;
	LPBYTE lpbData,lpbDataCurrent;
	LPSTR lpszMessageConverted=(LPSTR)lpwszMessage;
	LPSTR lpszMessageRTF=NULL;
	SIZE_T dwMessageConvertedSize=(dwMessageSize*sizeof(WCHAR)),dwMessageRTFSize=0;


	if (MraIsMessageFlashAnimation(lpwszMessage,dwMessageSize)) dwFlags|=MESSAGE_FLAG_FLASH;

	if (dwFlags&MESSAGE_FLAG_AUTHORIZE)
	{// pack auth message
		LPBYTE lpbAuthMsgBuff;
		SIZE_T dwMessageConvertedBuffSize=(((((dwMessageSize*sizeof(WCHAR))+1024)+2)/3)*4);

		lpszMessageConverted=(LPSTR)MEMALLOC(dwMessageConvertedBuffSize);
		lpbAuthMsgBuff=(LPBYTE)MEMALLOC(((dwMessageSize*sizeof(WCHAR))+1024));
		if (lpszMessageConverted && lpbAuthMsgBuff)
		{
			lpbDataCurrent=lpbAuthMsgBuff;
			SetUL(&lpbDataCurrent,2);
			SetLPSW(&lpbDataCurrent,NULL,0);//***deb possible nick here
			SetLPSW(&lpbDataCurrent,lpwszMessage,dwMessageSize);

			BASE64EncodeUnSafe(lpbAuthMsgBuff,(lpbDataCurrent-lpbAuthMsgBuff),lpszMessageConverted,dwMessageConvertedBuffSize,&dwMessageConvertedSize);
		}else{
			MEMFREE(lpszMessageConverted);
			lpszMessageConverted=(LPSTR)lpwszMessage;
		}
		MEMFREE(lpbAuthMsgBuff);
	}else
	if (dwFlags&MESSAGE_FLAG_FLASH)
	{// особая упаковка для флеша
		if (masMraSettings.lpfnCompress2)// only if func exist
		{
			SIZE_T dwRFTBuffSize=(((dwMessageSize*sizeof(WCHAR))*4)+8192),dwRTFDataSize;
			LPBYTE lpbRTFData;

			dwFlags|=MESSAGE_FLAG_RTF;
			lpszMessageRTF=(LPSTR)MEMALLOC(dwRFTBuffSize);
			lpbRTFData=(LPBYTE)MEMALLOC(dwRFTBuffSize);
			if (lpszMessageRTF && lpbRTFData)
			{
				DWORD dwBackColour;

				dwBackColour=DB_Mra_GetDword(NULL,"RTFBackgroundColour",MRA_DEFAULT_RTF_BACKGROUND_COLOUR);
				lpbDataCurrent=(LPBYTE)lpszMessageRTF;

				WideCharToMultiByte(MRA_CODE_PAGE,0,lpwszMessage,dwMessageSize,(LPSTR)lpbRTFData,dwRFTBuffSize,NULL,NULL);

				SetUL(&lpbDataCurrent,4);
				SetLPS(&lpbDataCurrent,(LPSTR)lpbRTFData,dwMessageSize);// сообщение что у собеседника плохая версия :)
				SetLPS(&lpbDataCurrent,(LPSTR)&dwBackColour,sizeof(DWORD));// цвет фона
				SetLPS(&lpbDataCurrent,(LPSTR)lpbRTFData,dwMessageSize);// сам мульт ANSI
				SetLPSW(&lpbDataCurrent,lpwszMessage,dwMessageSize);// сам мульт UNICODE

				dwRTFDataSize=dwRFTBuffSize;
				if ((PCOMPRESS2(masMraSettings.lpfnCompress2))(lpbRTFData,(DWORD*)&dwRTFDataSize,(LPBYTE)lpszMessageRTF,(lpbDataCurrent-(LPBYTE)lpszMessageRTF),Z_BEST_COMPRESSION)==Z_OK)
				{
					BASE64EncodeUnSafe(lpbRTFData,dwRTFDataSize,lpszMessageRTF,dwRFTBuffSize,&dwMessageRTFSize);
				}
			}
			MEMFREE(lpbRTFData);
		}
	}else// standart message
	if ((dwFlags&(MESSAGE_FLAG_CONTACT|MESSAGE_FLAG_NOTIFY|MESSAGE_FLAG_SMS))==0)
	{// Only if message is simple text message or RTF or ALARM
		if (dwFlags&MESSAGE_FLAG_RTF)// add RFT part
		if (masMraSettings.lpfnCompress2)// only if func exist
		{// обычный ртф
			SIZE_T dwRFTBuffSize=(((dwMessageSize*sizeof(WCHAR))*16)+8192),dwRTFDataSize;
			LPBYTE lpbRTFData;

			lpszMessageRTF=(LPSTR)MEMALLOC(dwRFTBuffSize);
			lpbRTFData=(LPBYTE)MEMALLOC(dwRFTBuffSize);
			if (lpszMessageRTF && lpbRTFData)
			{
				if (MraConvertToRTFW(lpwszMessage,dwMessageSize,(LPSTR)lpbRTFData,dwRFTBuffSize,&dwRTFDataSize)==NO_ERROR)
				{
					DWORD dwBackColour;

					dwBackColour=DB_Mra_GetDword(NULL,"RTFBackgroundColour",MRA_DEFAULT_RTF_BACKGROUND_COLOUR);
					lpbDataCurrent=(LPBYTE)lpszMessageRTF;

					SetUL(&lpbDataCurrent,2);
					SetLPS(&lpbDataCurrent,(LPSTR)lpbRTFData,dwRTFDataSize);
					SetLPS(&lpbDataCurrent,(LPSTR)&dwBackColour,sizeof(DWORD));

					dwRTFDataSize=dwRFTBuffSize;
					if ((PCOMPRESS2(masMraSettings.lpfnCompress2))(lpbRTFData,(DWORD*)&dwRTFDataSize,(LPBYTE)lpszMessageRTF,(lpbDataCurrent-(LPBYTE)lpszMessageRTF),Z_BEST_COMPRESSION)==Z_OK)
					{
						BASE64EncodeUnSafe(lpbRTFData,dwRTFDataSize,lpszMessageRTF,dwRFTBuffSize,&dwMessageRTFSize);
					}
				}
			}
			MEMFREE(lpbRTFData);
		}
	}//*/
	if (lpszMessageRTF==NULL || dwMessageRTFSize==0) dwFlags&=~(MESSAGE_FLAG_RTF|MESSAGE_FLAG_FLASH);
	if (lpbMultiChatData==NULL || dwMultiChatDataSize==0) dwFlags&=~MESSAGE_FLAG_MULTICHAT;

	lpbData=(LPBYTE)MEMALLOC((sizeof(DWORD)+dwEMailSize+dwMessageConvertedSize+dwMessageRTFSize+dwMultiChatDataSize+128));
	if (lpbData)
	{
		lpbDataCurrent=lpbData;
		SetUL(&lpbDataCurrent,dwFlags);
		SetLPSLowerCase(&lpbDataCurrent,lpszEMail,dwEMailSize);
		SetLPS(&lpbDataCurrent,lpszMessageConverted,dwMessageConvertedSize);
		SetLPS(&lpbDataCurrent,lpszMessageRTF,dwMessageRTFSize);
		if (dwFlags&MESSAGE_FLAG_MULTICHAT) SetLPS(&lpbDataCurrent,(LPSTR)lpbMultiChatData,dwMultiChatDataSize);

		if (bAddToQueue)
		{
			dwRet=MraSendQueueCMD(masMraSettings.hSendQueueHandle,0,hContact,dwAckType,(LPBYTE)lpwszMessage,(dwMessageSize*sizeof(WCHAR)),MRIM_CS_MESSAGE,lpbData,(lpbDataCurrent-lpbData));
		}else{
			dwRet=MraSendCMD(MRIM_CS_MESSAGE,lpbData,(lpbDataCurrent-lpbData));
		}
		MEMFREE(lpbData);
	}
	if (lpszMessageConverted!=(LPSTR)lpwszMessage) MEMFREE(lpszMessageConverted);
	MEMFREE(lpszMessageRTF);

return(dwRet);
}


DWORD MraSendCommand_MessageAskW(DWORD dwMsgID,DWORD dwFlags,LPSTR lpszEMail,SIZE_T dwEMailSize,LPWSTR lpwszMessage,SIZE_T dwMessageSize,LPSTR lpwszMessageRTF,SIZE_T dwMessageRTFSize)
{// Подтверждение получения сообщения
	DWORD dwRet=0;

	if (lpszEMail && dwEMailSize>4 && lpwszMessage && dwMessageSize)
	{
		LPBYTE lpbData,lpbDataCurrent;

		lpbData=(LPBYTE)MEMALLOC((sizeof(DWORD)+sizeof(DWORD)+dwEMailSize+dwMessageSize+dwMessageRTFSize+32));
		if (lpbData)
		{
			lpbDataCurrent=lpbData;
			SetUL(&lpbDataCurrent,dwMsgID);//UL msg_id
			SetUL(&lpbDataCurrent,dwFlags);//UL flags
			SetLPSLowerCase(&lpbDataCurrent,lpszEMail,dwEMailSize);// LPS from e-mail ANSI
			SetLPS(&lpbDataCurrent,(LPSTR)lpwszMessage,dwMessageSize);// LPS message UNICODE
			if (dwFlags&MESSAGE_FLAG_RTF) 
				SetLPS(&lpbDataCurrent,(LPSTR)lpwszMessageRTF,dwMessageRTFSize);// LPS	//rtf-formatted message (>=1.1)	- MESSAGE_FLAG_RTF

			dwRet=MraSendCMD(MRIM_CS_MESSAGE_ACK,lpbData,(lpbDataCurrent-lpbData));
			//dwRet=MraSendCMD(MRIM_CS_MESSAGE,lpbData,(lpbDataCurrent-lpbData));
			MEMFREE(lpbData);
		}
	}else{
		DebugBreak();
	}
return(dwRet);
}


DWORD MraSendCommand_MessageRecv(LPSTR lpszFrom,SIZE_T dwFromSize,DWORD dwMsgID)
{// Подтверждение получения сообщения
	DWORD dwRet=0;

	if (lpszFrom && dwFromSize>4)
	{
		LPBYTE lpbData,lpbDataCurrent;

		lpbData=(LPBYTE)MEMALLOC((dwFromSize+sizeof(DWORD)+32));
		if (lpbData)
		{
			lpbDataCurrent=lpbData;
			SetLPSLowerCase(&lpbDataCurrent,lpszFrom,dwFromSize);
			SetUL(&lpbDataCurrent,dwMsgID);

			dwRet=MraSendCMD(MRIM_CS_MESSAGE_RECV,lpbData,(lpbDataCurrent-lpbData));
			MEMFREE(lpbData);
		}
	}else{
		DebugBreak();
	}
return(dwRet);
}


DWORD MraSendCommand_AddContactW(HANDLE hContact,DWORD dwContactFlag,DWORD dwGroupID,LPSTR lpszEMail,SIZE_T dwEMailSize,LPWSTR lpwszCustomName,SIZE_T dwCustomNameSize,LPSTR lpszPhones,SIZE_T dwPhonesSize,LPWSTR lpwszAuthMessage,SIZE_T dwAuthMessageSize,DWORD dwActions)
{// Добавление нового контакта
	DWORD dwRet=0;

	if (lpszEMail && dwEMailSize>4)
	{
		LPBYTE lpbData,lpbDataCurrent;

		lpbData=(LPBYTE)MEMALLOC((sizeof(DWORD)+sizeof(DWORD)+dwEMailSize+(dwCustomNameSize*sizeof(WCHAR))+dwPhonesSize+(((((dwAuthMessageSize*sizeof(WCHAR))+1024)+2)/3)*4)+32)+sizeof(DWORD));
		if (lpbData)
		{
			dwContactFlag|=CONTACT_FLAG_UNICODE_NAME;

			lpbDataCurrent=lpbData;
			SetUL(&lpbDataCurrent,dwContactFlag);
			SetUL(&lpbDataCurrent,dwGroupID);
			SetLPSLowerCase(&lpbDataCurrent,lpszEMail,dwEMailSize);
			SetLPSW(&lpbDataCurrent,lpwszCustomName,dwCustomNameSize);
			SetLPS(&lpbDataCurrent,lpszPhones,dwPhonesSize);
			{// pack auth message
				LPBYTE lpbAuthMsgBuff,lpbAuthMessageConverted,lpbAuthDataCurrent;
				SIZE_T dwAuthMessageConvertedBuffSize=(((((dwAuthMessageSize*sizeof(WCHAR))+1024)+2)/3)*4),dwAuthMessageConvertedSize=0;
				
				lpbAuthMessageConverted=(LPBYTE)MEMALLOC(dwAuthMessageConvertedBuffSize);
				lpbAuthMsgBuff=(LPBYTE)MEMALLOC(((dwAuthMessageSize*sizeof(WCHAR))+1024));
				if (lpbAuthMessageConverted && lpbAuthMsgBuff)
				{
					lpbAuthDataCurrent=lpbAuthMsgBuff;
					SetUL(&lpbAuthDataCurrent,2);
					SetLPSW(&lpbAuthDataCurrent,NULL,0);//***deb possible nick here
					SetLPSW(&lpbAuthDataCurrent,lpwszAuthMessage,dwAuthMessageSize);

					BASE64EncodeUnSafe(lpbAuthMsgBuff,(lpbAuthDataCurrent-lpbAuthMsgBuff),lpbAuthMessageConverted,dwAuthMessageConvertedBuffSize,&dwAuthMessageConvertedSize);
				}
				SetLPS(&lpbDataCurrent,(LPSTR)lpbAuthMessageConverted,dwAuthMessageConvertedSize);
				MEMFREE(lpbAuthMsgBuff);
				MEMFREE(lpbAuthMessageConverted);
			}
			SetUL(&lpbDataCurrent,dwActions);

			dwRet=MraSendQueueCMD(masMraSettings.hSendQueueHandle,0,hContact,ACKTYPE_ADDED,NULL,0,MRIM_CS_ADD_CONTACT,lpbData,(lpbDataCurrent-lpbData));
			MEMFREE(lpbData);
		}
	}else{
		//DebugBreak();
	}
return(dwRet);
}


DWORD MraSendCommand_ModifyContactW(HANDLE hContact,DWORD dwID,DWORD dwContactFlag,DWORD dwGroupID,LPSTR lpszEMail,SIZE_T dwEMailSize,LPWSTR lpwszCustomName,SIZE_T dwCustomNameSize,LPSTR lpszPhones,SIZE_T dwPhonesSize)
{// Изменение контакта
	DWORD dwRet=0;

	LPBYTE lpbData,lpbDataCurrent;

	if (dwID!=-1)
	{
		lpbData=(LPBYTE)MEMALLOC((sizeof(DWORD)+sizeof(DWORD)+sizeof(DWORD)+dwEMailSize+(dwCustomNameSize*sizeof(WCHAR))+dwPhonesSize+32));
		if (lpbData)
		{
			dwContactFlag|=CONTACT_FLAG_UNICODE_NAME;

			lpbDataCurrent=lpbData;
			SetUL(&lpbDataCurrent,dwID);
			SetUL(&lpbDataCurrent,dwContactFlag);
			SetUL(&lpbDataCurrent,dwGroupID);
			SetLPSLowerCase(&lpbDataCurrent,lpszEMail,dwEMailSize);
			SetLPSW(&lpbDataCurrent,lpwszCustomName,dwCustomNameSize);
			SetLPS(&lpbDataCurrent,lpszPhones,dwPhonesSize);

			dwRet=MraSendQueueCMD(masMraSettings.hSendQueueHandle,0,hContact,ACKTYPE_ADDED,NULL,0,MRIM_CS_MODIFY_CONTACT,lpbData,(lpbDataCurrent-lpbData));
			MEMFREE(lpbData);
		}
	}else{
		DebugBreak();
	}
return(dwRet);
}


DWORD MraSendCommand_OfflineMessageDel(DWORDLONG dwMsgUIDL)
{// Удаление сохраненного сообщения
return(MraSendCMD(MRIM_CS_DELETE_OFFLINE_MESSAGE,&dwMsgUIDL,sizeof(DWORDLONG)));
}


DWORD MraSendCommand_Authorize(LPSTR lpszEMail,SIZE_T dwEMailSize)
{// Авторизация пользователя на добавление в контакт-лист
	DWORD dwRet=0;

	if (lpszEMail && dwEMailSize>4)
	{
		LPBYTE lpbData,lpbDataCurrent;

		lpbData=(LPBYTE)MEMALLOC((dwEMailSize+32));
		if (lpbData)
		{
			lpbDataCurrent=lpbData;
			SetLPSLowerCase(&lpbDataCurrent,lpszEMail,dwEMailSize);

			dwRet=MraSendCMD(MRIM_CS_AUTHORIZE,lpbData,(lpbDataCurrent-lpbData));
			MEMFREE(lpbData);
		}
	}else{
		DebugBreak();
	}
return(dwRet);
}


DWORD MraSendCommand_ChangeStatusW(DWORD dwStatus,LPSTR lpszStatusUri,SIZE_T dwStatusUriSize,LPWSTR lpwszStatusTitle,SIZE_T dwStatusTitleSize,LPWSTR lpwszStatusDesc,SIZE_T dwStatusDescSize,DWORD dwFutureFlags)
{// Изменение статуса
	DWORD dwRet=0;
	LPBYTE lpbData,lpbDataCurrent;

	if (dwStatusUriSize>SPEC_STATUS_URI_MAX)	dwStatusUriSize=SPEC_STATUS_URI_MAX;
	if (dwStatusTitleSize>STATUS_TITLE_MAX)		dwStatusTitleSize=STATUS_TITLE_MAX;
	if (dwStatusDescSize>STATUS_DESC_MAX)		dwStatusDescSize=STATUS_DESC_MAX;

	lpbData=(LPBYTE)MEMALLOC((sizeof(DWORD)+dwStatusUriSize+(dwStatusTitleSize*sizeof(WCHAR))+(dwStatusDescSize*sizeof(WCHAR))+sizeof(DWORD)+32));
	if (lpbData)
	{
		lpbDataCurrent=lpbData;
		SetUL(&lpbDataCurrent,dwStatus);
		SetLPS(&lpbDataCurrent,lpszStatusUri,dwStatusUriSize);
		SetLPSW(&lpbDataCurrent,lpwszStatusTitle,dwStatusTitleSize);
		SetLPSW(&lpbDataCurrent,lpwszStatusDesc,dwStatusDescSize);
		SetUL(&lpbDataCurrent,dwFutureFlags);

		dwRet=MraSendCMD(MRIM_CS_CHANGE_STATUS,lpbData,(lpbDataCurrent-lpbData));
		MEMFREE(lpbData);
	}
return(dwRet);
}


DWORD MraSendCommand_FileTransfer(LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwIDRequest,DWORD dwFilesTotalSize,LPWSTR lpwszFiles,SIZE_T dwFilesSize,LPSTR lpszAddreses,SIZE_T dwAddresesSize)
{// Отправка файлов
	DWORD dwRet=0;

	if (lpszEMail && dwEMailSize>4)
	{
		SIZE_T dwFilesSizeA;
		LPBYTE lpbData,lpbDataCurrent;

		dwFilesSizeA=WideCharToMultiByte(MRA_CODE_PAGE,0,lpwszFiles,dwFilesSize,NULL,0,NULL,NULL);
		lpbData=(LPBYTE)MEMALLOC((dwEMailSize+dwFilesSizeA+(dwFilesSize*sizeof(WCHAR))+dwAddresesSize+MAX_PATH));
		if (lpbData)
		{
			lpbDataCurrent=lpbData;
			SetLPSLowerCase(&lpbDataCurrent,lpszEMail,dwEMailSize);
			SetUL(&lpbDataCurrent,dwIDRequest);
			SetUL(&lpbDataCurrent,dwFilesTotalSize);
			SetUL(&lpbDataCurrent,(sizeof(DWORD)+dwFilesSizeA + sizeof(DWORD)+(sizeof(DWORD)+sizeof(DWORD)+(dwFilesSize*sizeof(WCHAR))) + sizeof(DWORD)+dwAddresesSize));//SetLPS(&lpbDataCurrent,(LPSTR)lpbDataEx,dwDataExSize);
			{
				SetLPSWtoA(&lpbDataCurrent,lpwszFiles,dwFilesSize);
				SetUL(&lpbDataCurrent,(sizeof(DWORD)+sizeof(DWORD)+(dwFilesSize*sizeof(WCHAR))));//SetLPS(&lpbDataCurrent,lpbDescription,dwDescriptionSize);
				{
					SetUL(&lpbDataCurrent,1);
					SetLPSW(&lpbDataCurrent,lpwszFiles,dwFilesSize);
				}
				SetLPS(&lpbDataCurrent,lpszAddreses,dwAddresesSize);
			}

			dwRet=MraSendCMD(MRIM_CS_FILE_TRANSFER,lpbData,(lpbDataCurrent-lpbData));
			MEMFREE(lpbData);
		}
	}else{
		DebugBreak();
	}
return(dwRet);
}


DWORD MraSendCommand_FileTransferAck(DWORD dwStatus,LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwIDRequest,LPBYTE lpbDescription,SIZE_T dwDescriptionSize)
{// Ответ на отправку файлов
	DWORD dwRet=0;

	if (lpszEMail && dwEMailSize>4)
	{
		LPBYTE lpbData,lpbDataCurrent;

		lpbData=(LPBYTE)MEMALLOC((dwEMailSize+dwDescriptionSize+32));
		if (lpbData)
		{
			lpbDataCurrent=lpbData;
			SetUL(&lpbDataCurrent,dwStatus);
			SetLPSLowerCase(&lpbDataCurrent,lpszEMail,dwEMailSize);
			SetUL(&lpbDataCurrent,dwIDRequest);
			SetLPS(&lpbDataCurrent,(LPSTR)lpbDescription,dwDescriptionSize);

			dwRet=MraSendCMD(MRIM_CS_FILE_TRANSFER_ACK,lpbData,(lpbDataCurrent-lpbData));
			MEMFREE(lpbData);
		}
	}else{
		DebugBreak();
	}
return(dwRet);
}


/*DWORD MraSendCommand_WPRequestA(HANDLE hContact,DWORD dwAckType,DWORD dwRequestFlags,LPSTR lpszUser,SIZE_T dwUserSize,LPSTR lpszDomain,SIZE_T dwDomainSize,LPSTR lpszNickName,SIZE_T dwNickNameSize,LPSTR lpszFirstName,SIZE_T dwFirstNameSize,LPSTR lpszLastName,SIZE_T dwLastNameSize,DWORD dwSex,DWORD dwDate1,DWORD dwDate2,DWORD dwCityID,DWORD dwZodiak,DWORD dwBirthdayMonth,DWORD dwBirthdayDay,DWORD dwCountryID,DWORD dwOnline)
{// Поиск контакта
	//WCHAR wszUser[MAX_PATH],wszDomain[MAX_PATH];
	WCHAR wszNickName[MAX_PATH],wszFirstName[MAX_PATH],wszLastName[MAX_PATH];

	//if (lpszUser && dwUserSize)				dwUserSize=MultiByteToWideChar(MRA_CODE_PAGE,0,lpszUser,dwUserSize,wszUser,SIZEOF(wszUser));
	//if (lpszDomain && dwDomainSize)			dwDomainSize=MultiByteToWideChar(MRA_CODE_PAGE,0,lpszDomain,dwDomainSize,wszDomain,SIZEOF(wszDomain));
	if (lpszNickName && dwNickNameSize)		dwNickNameSize=MultiByteToWideChar(MRA_CODE_PAGE,0,lpszNickName,dwNickNameSize,wszNickName,SIZEOF(wszNickName));
	if (lpszFirstName && dwFirstNameSize)	dwFirstNameSize=MultiByteToWideChar(MRA_CODE_PAGE,0,lpszFirstName,dwFirstNameSize,wszFirstName,SIZEOF(wszFirstName));
	if (lpszLastName && dwLastNameSize)		dwLastNameSize=MultiByteToWideChar(MRA_CODE_PAGE,0,lpszLastName,dwLastNameSize,wszLastName,SIZEOF(wszLastName));

return(MraSendCommand_WPRequestW(hContact,dwAckType,dwRequestFlags,lpszUser,dwUserSize,lpszDomain,dwDomainSize,wszNickName,dwNickNameSize,wszFirstName,dwFirstNameSize,wszLastName,dwLastNameSize,dwSex,dwDate1,dwDate2,dwCityID,dwZodiak,dwBirthdayMonth,dwBirthdayDay,dwCountryID,dwOnline));
}//*/

DWORD MraSendCommand_WPRequestW(HANDLE hContact,DWORD dwAckType,DWORD dwRequestFlags,LPSTR lpszUser,SIZE_T dwUserSize,LPSTR lpszDomain,SIZE_T dwDomainSize,LPWSTR lpwszNickName,SIZE_T dwNickNameSize,LPWSTR lpwszFirstName,SIZE_T dwFirstNameSize,LPWSTR lpwszLastName,SIZE_T dwLastNameSize,DWORD dwSex,DWORD dwDate1,DWORD dwDate2,DWORD dwCityID,DWORD dwZodiak,DWORD dwBirthdayMonth,DWORD dwBirthdayDay,DWORD dwCountryID,DWORD dwOnline)
{// Поиск контакта
	DWORD dwRet=0;
	LPBYTE lpbData,lpbDataCurrent;

	lpbData=(LPBYTE)MEMALLOC(((dwUserSize+dwDomainSize+dwNickNameSize+dwFirstNameSize+dwLastNameSize)*sizeof(WCHAR))+4096);
	if (lpbData)
	{
		CHAR szBuff[MAX_PATH];
		SIZE_T dwBuffSize;

		lpbDataCurrent=lpbData;

		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_USER))		{SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_USER);SetLPSLowerCase(&lpbDataCurrent,lpszUser,dwUserSize);}
		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_DOMAIN))		{SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_DOMAIN);SetLPSLowerCase(&lpbDataCurrent,lpszDomain,dwDomainSize);}
		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_NICKNAME))	{SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_NICKNAME);SetLPSW(&lpbDataCurrent,lpwszNickName,dwNickNameSize);}
		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_FIRSTNAME))	{SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_FIRSTNAME);SetLPSW(&lpbDataCurrent,lpwszFirstName,dwFirstNameSize);}
		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_LASTNAME))	{SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_LASTNAME);SetLPSW(&lpbDataCurrent,lpwszLastName,dwLastNameSize);}
		
		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_SEX))
		{
			dwBuffSize=mir_snprintf(szBuff,SIZEOF(szBuff),"%lu",dwSex);
			SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_SEX);
			SetLPS(&lpbDataCurrent,szBuff,dwBuffSize);
		}

		/*if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY))
		{
			dwBuffSize=mir_snprintf(szBuff,SIZEOF(szBuff),"%lu",dwSex);
			SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY);
			SetLPS(&lpbDataCurrent,szBuff,dwBuffSize);
		}*/

		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_DATE1))
		{
			dwBuffSize=mir_snprintf(szBuff,SIZEOF(szBuff),"%lu",dwDate1);
			SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_DATE1);
			SetLPS(&lpbDataCurrent,szBuff,dwBuffSize);
		}

		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_DATE2))
		{
			dwBuffSize=mir_snprintf(szBuff,SIZEOF(szBuff),"%lu",dwDate2);
			SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_DATE2);
			SetLPS(&lpbDataCurrent,szBuff,dwBuffSize);
		}

		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_CITY_ID))
		{
			dwBuffSize=mir_snprintf(szBuff,SIZEOF(szBuff),"%lu",dwCityID);
			SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_CITY_ID);
			SetLPS(&lpbDataCurrent,szBuff,dwBuffSize);
		}

		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_ZODIAC))
		{
			dwBuffSize=mir_snprintf(szBuff,SIZEOF(szBuff),"%lu",dwZodiak);
			SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_ZODIAC);
			SetLPS(&lpbDataCurrent,szBuff,dwBuffSize);
		}

		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_MONTH))
		{
			dwBuffSize=mir_snprintf(szBuff,SIZEOF(szBuff),"%lu",dwBirthdayMonth);
			SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_MONTH);
			SetLPS(&lpbDataCurrent,szBuff,dwBuffSize);
		}

		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_DAY))
		{
			dwBuffSize=mir_snprintf(szBuff,SIZEOF(szBuff),"%lu",dwBirthdayDay);
			SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_DAY);
			SetLPS(&lpbDataCurrent,szBuff,dwBuffSize);
		}

		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_COUNTRY_ID))
		{
			dwBuffSize=mir_snprintf(szBuff,SIZEOF(szBuff),"%lu",dwCountryID);
			SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_COUNTRY_ID);
			SetLPS(&lpbDataCurrent,szBuff,dwBuffSize);
		}

		if (GetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_ONLINE))
		{
			dwBuffSize=mir_snprintf(szBuff,SIZEOF(szBuff),"%lu",dwOnline);
			SetUL(&lpbDataCurrent,MRIM_CS_WP_REQUEST_PARAM_ONLINE);
			SetLPS(&lpbDataCurrent,szBuff,dwBuffSize);
		}

		dwRet=MraSendQueueCMD(masMraSettings.hSendQueueHandle,0,hContact,dwAckType,NULL,0,MRIM_CS_WP_REQUEST,lpbData,(lpbDataCurrent-lpbData));
		MEMFREE(lpbData);
	}
return(dwRet);
}


DWORD MraSendCommand_WPRequestByEMail(HANDLE hContact,DWORD dwAckType,LPSTR lpszEMail,SIZE_T dwEMailSize)
{// Поиск контакта по EMail
	DWORD dwRet=0,dwRequestFlags=0;
	
	if (lpszEMail && dwEMailSize>4)
	{
		LPSTR lpszUser,lpszDomain;
		SIZE_T dwUserSize,dwDomainSize;

		lpszDomain=(LPSTR)MemoryFindByte(0,lpszEMail,dwEMailSize,'@');
		if (lpszDomain)
		{
			lpszUser=lpszEMail;
			dwUserSize=(lpszDomain-lpszEMail);
			lpszDomain++;
			dwDomainSize=(dwEMailSize-(dwUserSize+1));

			SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_USER);
			SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_DOMAIN);

			dwRet=MraSendCommand_WPRequestW(hContact,dwAckType,dwRequestFlags,lpszUser,dwUserSize,lpszDomain,dwDomainSize,NULL,0,NULL,0,NULL,0,0,0,0,0,0,0,0,0,0);
		}
	}
return(dwRet);
}


DWORD MraSendCommand_Game(LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwGameSessionID,DWORD dwGameMsg,DWORD dwGameMsgID,LPSTR lpszData,SIZE_T dwDataSize)
{// Отправка файлов
	DWORD dwRet=0;

	if (lpszEMail && dwEMailSize>4)
	{
		LPBYTE lpbData,lpbDataCurrent;

		lpbData=(LPBYTE)MEMALLOC((dwEMailSize+(sizeof(DWORD)*4)+dwDataSize+32));
		if (lpbData)
		{
			lpbDataCurrent=lpbData;
			SetLPSLowerCase(&lpbDataCurrent,lpszEMail,dwEMailSize);
			SetUL(&lpbDataCurrent,dwGameSessionID);
			SetUL(&lpbDataCurrent,dwGameMsg);
			SetUL(&lpbDataCurrent,dwGameMsgID);
			SetUL(&lpbDataCurrent,_time32(NULL));
			SetLPS(&lpbDataCurrent,lpszData,dwDataSize);

			dwRet=MraSendCMD(MRIM_CS_GAME,lpbData,(lpbDataCurrent-lpbData));
			MEMFREE(lpbData);
		}
	}else{
		DebugBreak();
	}
return(dwRet);
}


DWORD MraSendCommand_Login2W(LPSTR lpszLogin,SIZE_T dwLoginSize,LPSTR lpszPassword,SIZE_T dwPasswordSize,DWORD dwStatus,LPSTR lpszStatusUri,SIZE_T dwStatusUriSize,LPWSTR lpwszStatusTitle,SIZE_T dwStatusTitleSize,LPWSTR lpwszStatusDesc,SIZE_T dwStatusDescSize,DWORD dwFutureFlags,LPSTR lpszUserAgentFormated,SIZE_T dwUserAgentFormatedSize,LPSTR lpszUserAgent,SIZE_T dwUserAgentSize)
{// Авторизация
	DWORD dwRet=0;
	LPBYTE lpbData,lpbDataCurrent;

	if (dwStatusUriSize>SPEC_STATUS_URI_MAX)	dwStatusUriSize=SPEC_STATUS_URI_MAX;
	if (dwStatusTitleSize>STATUS_TITLE_MAX)		dwStatusTitleSize=STATUS_TITLE_MAX;
	if (dwStatusDescSize>STATUS_DESC_MAX)		dwStatusDescSize=STATUS_DESC_MAX;
	if (dwUserAgentFormatedSize>USER_AGENT_MAX)	dwUserAgentFormatedSize=USER_AGENT_MAX;
	if (dwUserAgentSize>MAX_CLIENT_DESCRIPTION)	dwUserAgentSize=MAX_CLIENT_DESCRIPTION;

	lpbData=(LPBYTE)MEMALLOC((dwLoginSize+dwPasswordSize+sizeof(DWORD)+dwStatusUriSize+(dwStatusTitleSize*sizeof(WCHAR))+(dwStatusDescSize*sizeof(WCHAR))+2+sizeof(DWORD)+(sizeof(DWORD)*2)+dwUserAgentFormatedSize+dwUserAgentSize+32));
	if (lpbData)
	{
		lpbDataCurrent=lpbData;
		SetLPS(&lpbDataCurrent,lpszLogin,dwLoginSize);
		SetLPS(&lpbDataCurrent,lpszPassword,dwPasswordSize);
		SetUL(&lpbDataCurrent,dwStatus);
		SetLPS(&lpbDataCurrent,lpszStatusUri,dwStatusUriSize);
		SetLPSW(&lpbDataCurrent,lpwszStatusTitle,dwStatusTitleSize);
		SetLPSW(&lpbDataCurrent,lpwszStatusDesc,dwStatusDescSize);
		SetUL(&lpbDataCurrent,dwFutureFlags);
		SetLPS(&lpbDataCurrent,lpszUserAgentFormated,dwUserAgentFormatedSize);
		SetLPS(&lpbDataCurrent,"ru",2);
		SetLPS(&lpbDataCurrent,NULL,0);
		SetLPS(&lpbDataCurrent,NULL,0);
		SetLPS(&lpbDataCurrent,lpszUserAgent,dwUserAgentSize);// LPS client description /max 256

		dwRet=MraSendCMD(MRIM_CS_LOGIN2,lpbData,(lpbDataCurrent-lpbData));
		MEMFREE(lpbData);
	}
return(dwRet);
}


DWORD MraSendCommand_SMSW(HANDLE hContact,LPSTR lpszPhone,SIZE_T dwPhoneSize,LPWSTR lpwszMessage,SIZE_T dwMessageSize)
{// Отправка SMS
	DWORD dwRet=0;
	LPBYTE lpbData,lpbDataCurrent,lpbDataQueue;
	LPSTR lpszPhoneLocal;

	lpbData=(LPBYTE)MEMALLOC((dwPhoneSize+(dwMessageSize*sizeof(WCHAR))+32));
	lpbDataQueue=(LPBYTE)MEMALLOC((dwPhoneSize+(dwMessageSize*sizeof(WCHAR))+32));
	lpszPhoneLocal=(LPSTR)MEMALLOC((dwPhoneSize+32));
	if (lpbData && lpbDataQueue && lpszPhoneLocal)
	{
		lpszPhoneLocal[0]='+';
		dwPhoneSize=1+CopyNumber((lpszPhoneLocal+1),lpszPhone,dwPhoneSize);

		lpbDataCurrent=lpbData;
		SetUL(&lpbDataCurrent,0);
		SetLPS(&lpbDataCurrent,lpszPhoneLocal,dwPhoneSize);
		SetLPSW(&lpbDataCurrent,lpwszMessage,dwMessageSize);

		(*(DWORD*)lpbDataQueue)=dwPhoneSize;
		memmove((lpbDataQueue+sizeof(DWORD)),lpszPhoneLocal,(dwPhoneSize+1));
		memmove((lpbDataQueue+sizeof(DWORD)+dwPhoneSize+1),lpwszMessage,((dwMessageSize*sizeof(WCHAR))+1));

		dwRet=MraSendQueueCMD(masMraSettings.hSendQueueHandle,0,hContact,ICQACKTYPE_SMS,lpbDataQueue,(dwPhoneSize+(dwMessageSize*sizeof(WCHAR))+sizeof(DWORD)+2),MRIM_CS_SMS,lpbData,(lpbDataCurrent-lpbData));
		MEMFREE(lpbData);
		MEMFREE(lpszPhoneLocal);
	}else{
		MEMFREE(lpbData);
		MEMFREE(lpbDataQueue);
		MEMFREE(lpszPhoneLocal);
	}
return(dwRet);
}



DWORD MraSendCommand_Proxy(LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwIDRequest,DWORD dwDataType,LPSTR lpszData,SIZE_T dwDataSize,LPSTR lpszAddreses,SIZE_T dwAddresesSize,MRA_GUID mguidSessionID)
{// Соединение с прокси
	DWORD dwRet=0;

	if (lpszEMail && dwEMailSize>4)
	{
		LPBYTE lpbData,lpbDataCurrent;

		lpbData=(LPBYTE)MEMALLOC((dwEMailSize+(sizeof(DWORD)*2)+dwDataSize+dwAddresesSize+sizeof(MRA_GUID)+32));
		if (lpbData)
		{
			lpbDataCurrent=lpbData;
			SetLPSLowerCase(&lpbDataCurrent,lpszEMail,dwEMailSize);
			SetUL(&lpbDataCurrent,dwIDRequest);
			SetUL(&lpbDataCurrent,dwDataType);
			SetLPS(&lpbDataCurrent,lpszData,dwDataSize);
			SetLPS(&lpbDataCurrent,lpszAddreses,dwAddresesSize);
			SetGUID(&lpbDataCurrent,mguidSessionID);

			dwRet=MraSendCMD(MRIM_CS_PROXY,lpbData,(lpbDataCurrent-lpbData));
			MEMFREE(lpbData);
		}
	}else{
		DebugBreak();
	}
return(dwRet);
}


DWORD MraSendCommand_ProxyAck(DWORD dwStatus,LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwIDRequest,DWORD dwDataType,LPSTR lpszData,SIZE_T dwDataSize,LPSTR lpszAddreses,SIZE_T dwAddresesSize,MRA_GUID mguidSessionID)
{// Ответ на соединение с прокси
	DWORD dwRet=0;

	if (lpszEMail && dwEMailSize>4)
	{
		LPBYTE lpbData,lpbDataCurrent;

		lpbData=(LPBYTE)MEMALLOC((dwEMailSize+(sizeof(DWORD)*3)+dwDataSize+dwAddresesSize+sizeof(MRA_GUID)+32));
		if (lpbData)
		{
			lpbDataCurrent=lpbData;
			SetUL(&lpbDataCurrent,dwStatus);
			SetLPSLowerCase(&lpbDataCurrent,lpszEMail,dwEMailSize);
			SetUL(&lpbDataCurrent,dwIDRequest);
			SetUL(&lpbDataCurrent,dwDataType);
			SetLPS(&lpbDataCurrent,lpszData,dwDataSize);
			SetLPS(&lpbDataCurrent,lpszAddreses,dwAddresesSize);
			SetGUID(&lpbDataCurrent,mguidSessionID);

			dwRet=MraSendCMD(MRIM_CS_PROXY_ACK,lpbData,(lpbDataCurrent-lpbData));
			MEMFREE(lpbData);
		}
	}else{
		DebugBreak();
	}
return(dwRet);
}


DWORD MraSendCommand_ChangeUserBlogStatus(DWORD dwFlags,LPWSTR lpwszText,SIZE_T dwTextSize,DWORDLONG dwBlogStatusID)
{// Отправка сообщения в микроблог
	DWORD dwRet=0;
	LPBYTE lpbData,lpbDataCurrent;

	if (dwTextSize>MICBLOG_STATUS_MAX) dwTextSize=MICBLOG_STATUS_MAX;

	lpbData=(LPBYTE)MEMALLOC((sizeof(DWORD)+(dwTextSize*sizeof(WCHAR))+sizeof(DWORDLONG)+32));
	if (lpbData)
	{
		lpbDataCurrent=lpbData;
		SetUL(&lpbDataCurrent,dwFlags);
		SetLPSW(&lpbDataCurrent,lpwszText,dwTextSize);
		SetUIDL(&lpbDataCurrent,dwBlogStatusID);

		dwRet=MraSendCMD(MRIM_CS_CHANGE_USER_BLOG_STATUS,lpbData,(lpbDataCurrent-lpbData));
		MEMFREE(lpbData);
	}

return(dwRet);
}


void SetUL(LPBYTE *plpBuff,DWORD dwData)
{
	(*(DWORD*)(*plpBuff))=dwData;
	(*plpBuff)+=sizeof(DWORD);
}


void SetUIDL(LPBYTE *plpBuff,DWORDLONG dwData)
{
	(*(DWORDLONG*)(*plpBuff))=dwData;
	(*plpBuff)+=sizeof(DWORDLONG);
}


void SetGUID(LPBYTE *plpBuff,MRA_GUID guidData)
{
	(*(MRA_GUID*)(*plpBuff))=guidData;
	(*plpBuff)+=sizeof(MRA_GUID);
}


void SetLPS(LPBYTE *plpBuff,LPSTR lpszData,DWORD dwSize)
{
	(*(DWORD*)(*plpBuff))=dwSize;
	(*plpBuff)+=sizeof(DWORD);
	memmove((*plpBuff),lpszData,dwSize);
	(*plpBuff)+=dwSize;
}


void SetLPSW(LPBYTE *plpBuff,LPWSTR lpwszData,DWORD dwSize)
{
	dwSize*=sizeof(WCHAR);
	(*(DWORD*)(*plpBuff))=dwSize;
	(*plpBuff)+=sizeof(DWORD);
	memmove((*plpBuff),lpwszData,dwSize);
	(*plpBuff)+=dwSize;
}


void SetLPSWtoA(LPBYTE *plpBuff,LPWSTR lpwszData,DWORD dwSize)
{
	dwSize=WideCharToMultiByte(MRA_CODE_PAGE,0,lpwszData,dwSize,(LPSTR)((*plpBuff)+sizeof(DWORD)),(dwSize*sizeof(WCHAR)),NULL,NULL);
	(*(DWORD*)(*plpBuff))=dwSize;
	(*plpBuff)+=(sizeof(DWORD)+dwSize);
}



void SetLPSLowerCase(LPBYTE *plpBuff,LPSTR lpszData,DWORD dwSize)
{
	(*(DWORD*)(*plpBuff))=dwSize;
	(*plpBuff)+=sizeof(DWORD);
	BuffToLowerCase((*plpBuff),lpszData,dwSize);
	(*plpBuff)+=dwSize;
}


void SetLPSLowerCaseW(LPBYTE *plpBuff,LPWSTR lpwszData,DWORD dwSize)
{
	dwSize*=sizeof(WCHAR);
	(*(DWORD*)(*plpBuff))=dwSize;
	(*plpBuff)+=sizeof(DWORD);
	memmove((*plpBuff),lpwszData,dwSize);
	CharLowerBuff((LPWSTR)(*plpBuff),(dwSize/sizeof(WCHAR)));
	(*plpBuff)+=dwSize;
}


DWORD MraSendPacket(HANDLE hConnection,DWORD dwCMDNum,DWORD dwType,LPVOID lpData,SIZE_T dwDataSize)
{
	DWORD dwRet;

	LPBYTE lpbData;
	mrim_packet_header_t *pmaHeader;

	lpbData=(LPBYTE)MEMALLOC((dwDataSize+sizeof(mrim_packet_header_t)));
	if (lpbData)
	{
		pmaHeader=(mrim_packet_header_t*)lpbData;
		pmaHeader->magic=CS_MAGIC;
		pmaHeader->proto=PROTO_VERSION;// Версия протокола
		pmaHeader->seq=dwCMDNum;// Sequence
		pmaHeader->msg=dwType;// Тип пакета
		pmaHeader->dlen=dwDataSize;// Длина данных
		//bzero(&maHeader->reserved[0],24);// Зарезервировано

		memmove((lpbData+sizeof(mrim_packet_header_t)),lpData,dwDataSize);
		dwRet=Netlib_Send(hConnection,(LPSTR)lpbData,(dwDataSize+sizeof(mrim_packet_header_t)),0);
	
		/*if (dwType==MRIM_CS_ADD_CONTACT)
		{
			{
				char sztm[100];

				SHA1GetStringDigestA((LPSTR)lpbData,(dwDataSize+sizeof(mrim_packet_header_t)),sztm);
				OutputDebugStringA((sztm));
				OutputDebugStringA("\r\n");
			}

			{
				DWORD dwTemp;
				HANDLE hFile=CreateFileW(L"C:\\Documents and Settings\\rozhuk_im\\Рабочий стол\\AddContact.txt",GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

				if (hFile!=INVALID_HANDLE_VALUE)
				{
					WriteFile(hFile,lpbData,(dwDataSize+sizeof(mrim_packet_header_t)),&dwTemp,NULL);
					CloseHandle(hFile);
				}
			}
		}//*/
		MEMFREE(lpbData);
	}
return(dwRet);
}


DWORD MraSendCMD(DWORD dwType,LPVOID lpData,SIZE_T dwDataSize)
{
	DWORD dwRet;

	dwRet=InterlockedIncrement((LONG volatile*)&masMraSettings.dwCMDNum);
	EnterCriticalSection(&masMraSettings.csCriticalSectionSend);// guarding winsock internal buffers
	if (MraSendPacket(masMraSettings.hConnection,dwRet,dwType,lpData,dwDataSize)==0)
	{// err on send
		dwRet=0;
	}
	LeaveCriticalSection(&masMraSettings.csCriticalSectionSend);
return(dwRet);
}


DWORD MraSendQueueCMD(HANDLE hSendQueueHandle,DWORD dwFlags,HANDLE hContact,DWORD dwAckType,LPBYTE lpbDataQueue,SIZE_T dwDataQueueSize,DWORD dwType,LPVOID lpData,SIZE_T dwDataSize)
{
	DWORD dwRet;

	dwRet=InterlockedIncrement((LONG volatile*)&masMraSettings.dwCMDNum);
	if (MraSendQueueAdd(hSendQueueHandle,dwRet,dwFlags,hContact,dwAckType,lpbDataQueue,dwDataQueueSize)==NO_ERROR)
	{
		EnterCriticalSection(&masMraSettings.csCriticalSectionSend);// guarding winsock internal buffers
		if (MraSendPacket(masMraSettings.hConnection,dwRet,dwType,lpData,dwDataSize)==0)
		{
			MraSendQueueFree(hSendQueueHandle,dwRet);
			dwRet=0;
		}
		LeaveCriticalSection(&masMraSettings.csCriticalSectionSend);
	}
return(dwRet);
}



