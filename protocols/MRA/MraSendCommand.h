#if !defined(AFX_MRA_SENDCOMMAND_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_SENDCOMMAND_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


DWORD			MraSendCommand_MessageW			(BOOL bAddToQueue,HANDLE hContact,DWORD dwAckType,DWORD dwFlags,LPSTR lpszEMail,SIZE_T dwEMailSize,LPWSTR lpwszMessage,SIZE_T dwMessageSize,LPBYTE lpbMultiChatData,SIZE_T dwMultiChatDataSize);
DWORD			MraSendCommand_MessageAskW		(DWORD dwMsgID,DWORD dwFlags,LPSTR lpszEMail,SIZE_T dwEMailSize,LPWSTR lpwszMessage,SIZE_T dwMessageSize,LPSTR lpwszMessageRTF,SIZE_T dwMessageRTFSize);
DWORD			MraSendCommand_MessageRecv		(LPSTR lpszFrom,SIZE_T dwFromSize,DWORD dwMsgID);
DWORD			MraSendCommand_AddContactW		(HANDLE hContact,DWORD dwContactFlag,DWORD dwGroupID,LPSTR lpszEMail,SIZE_T dwEMailSize,LPWSTR lpwszCustomName,SIZE_T dwCustomNameSize,LPSTR lpszPhones,SIZE_T dwPhonesSize,LPWSTR lpwszAuthMessage,SIZE_T dwAuthMessageSize,DWORD dwActions);
DWORD			MraSendCommand_ModifyContactW	(HANDLE hContact,DWORD dwID,DWORD dwContactFlag,DWORD dwGroupID,LPSTR lpszEMail,SIZE_T dwEMailSize,LPWSTR lpwszCustomName,SIZE_T dwCustomNameSize,LPSTR lpszPhones,SIZE_T dwPhonesSize);
DWORD			MraSendCommand_OfflineMessageDel(DWORDLONG dwMsgUIDL);
DWORD			MraSendCommand_Authorize		(LPSTR lpszEMail,SIZE_T dwEMailSize);
DWORD			MraSendCommand_ChangeStatusW	(DWORD dwStatus,LPSTR lpszStatusUri,SIZE_T dwStatusUriSize,LPWSTR lpwszStatusTitle,SIZE_T dwStatusTitleSize,LPWSTR lpwszStatusDesc,SIZE_T dwStatusDescSize,DWORD dwFutureFlags);
DWORD			MraSendCommand_FileTransfer		(LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwIdRequest,DWORD dwFilesTotalSize,LPWSTR lpwszFiles,SIZE_T dwFilesSize,LPSTR lpszAddreses,SIZE_T dwAddresesSize);
DWORD			MraSendCommand_FileTransferAck	(DWORD dwStatus,LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwIdRequest,LPBYTE lpbDescription,SIZE_T dwDescriptionSize);
DWORD			MraSendCommand_WPRequestW		(HANDLE hContact,DWORD dwAckType,DWORD dwRequestFlags,LPSTR lpszUser,SIZE_T dwUserSize,LPSTR lpszDomain,SIZE_T dwDomainSize,LPWSTR lpwszNickName,SIZE_T dwNickNameSize,LPWSTR lpwszFirstName,SIZE_T dwFirstNameSize,LPWSTR lpwszLastName,SIZE_T dwLastNameSize,DWORD dwSex,DWORD dwDate1,DWORD dwDate2,DWORD dwCityID,DWORD dwZodiak,DWORD dwBirthdayMonth,DWORD dwBirthdayDay,DWORD dwCountryID,DWORD dwOnline);
DWORD			MraSendCommand_WPRequestA		(HANDLE hContact,DWORD dwAckType,DWORD dwRequestFlags,LPSTR lpszUser,SIZE_T dwUserSize,LPSTR lpszDomain,SIZE_T dwDomainSize,LPSTR lpszNickName,SIZE_T dwNickNameSize,LPSTR lpszFirstName,SIZE_T dwFirstNameSize,LPSTR lpszLastName,SIZE_T dwLastNameSize,DWORD dwSex,DWORD dwDate1,DWORD dwDate2,DWORD dwCityID,DWORD dwZodiak,DWORD dwBirthdayMonth,DWORD dwBirthdayDay,DWORD dwCountryID,DWORD dwOnline);
DWORD			MraSendCommand_WPRequestByEMail	(HANDLE hContact,DWORD dwAckType,LPSTR lpszEMail,SIZE_T dwEMailSize);
DWORD			MraSendCommand_Game				(LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwGameSessionID,DWORD dwGameMsg,DWORD dwGameMsgID,LPSTR lpszData,SIZE_T dwDataSize);
DWORD			MraSendCommand_Login2W			(LPSTR lpszLogin,SIZE_T dwLoginSize,LPSTR lpszPassword,SIZE_T dwPasswordSize,DWORD dwStatus,LPSTR lpszStatusUri,SIZE_T dwStatusUriSize,LPWSTR lpwszStatusTitle,SIZE_T dwStatusTitleSize,LPWSTR lpwszStatusDesc,SIZE_T dwStatusDescSize,DWORD dwFutureFlags,LPSTR lpszUserAgentFormated,SIZE_T dwUserAgentFormatedSize,LPSTR lpszUserAgent,SIZE_T dwUserAgentSize);
DWORD			MraSendCommand_SMSW				(HANDLE hContact,LPSTR lpszPhone,SIZE_T dwPhoneSize,LPWSTR lpwszMessage,SIZE_T dwMessageSize);
DWORD			MraSendCommand_Proxy			(LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwIDRequest,DWORD dwDataType,LPSTR lpszData,SIZE_T dwDataSize,LPSTR lpszAddreses,SIZE_T dwAddresesSize,MRA_GUID mguidSessionID);
DWORD			MraSendCommand_ProxyAck			(DWORD dwStatus,LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwIDRequest,DWORD dwDataType,LPSTR lpszData,SIZE_T dwDataSize,LPSTR lpszAddreses,SIZE_T dwAddresesSize,MRA_GUID mguidSessionID);
DWORD			MraSendCommand_ChangeUserBlogStatus(DWORD dwFlags,LPWSTR lpwszText,SIZE_T dwTextSize,DWORDLONG dwBlogStatusID);

DWORD			MraSendPacket					(HANDLE hConnection,DWORD dwCMDNum,DWORD dwType,LPVOID lpData,SIZE_T dwDataSize);
DWORD			MraSendCMD						(DWORD dwType,LPVOID lpData,SIZE_T dwDataSize);
DWORD			MraSendQueueCMD					(HANDLE hSendQueueHandle,DWORD dwFlags,HANDLE hContact,DWORD dwAckType,LPBYTE lpbDataQueue,SIZE_T dwDataQueueSize,DWORD dwType,LPVOID lpData,SIZE_T dwDataSize);



#endif // !defined(AFX_MRA_SENDCOMMAND_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
