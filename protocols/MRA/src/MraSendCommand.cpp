#include "Mra.h"
#include "MraSendCommand.h"
#include "MraRTFMsg.h"
#include "proto.h"

static void SetUL(LPBYTE *plpBuff, DWORD dwData)
{
	(*(DWORD*)(*plpBuff)) = dwData;
	(*plpBuff) += sizeof(DWORD);
}

static void SetUIDL(LPBYTE *plpBuff, DWORDLONG dwData)
{
	(*(DWORDLONG*)(*plpBuff)) = dwData;
	(*plpBuff) += sizeof(DWORDLONG);
}

static void SetGUID(LPBYTE *plpBuff, MRA_GUID guidData)
{
	(*(MRA_GUID*)(*plpBuff)) = guidData;
	(*plpBuff) += sizeof(MRA_GUID);
}

static void SetLPS(LPBYTE *plpBuff, LPCSTR lpszData, DWORD dwSize)
{
	(*(DWORD*)(*plpBuff)) = dwSize;
	(*plpBuff) += sizeof(DWORD);
	memmove((*plpBuff), lpszData, dwSize);
	(*plpBuff) += dwSize;
}

static void SetLPSW(LPBYTE *plpBuff, LPCWSTR lpwszData, DWORD dwSize)
{
	dwSize *= sizeof(WCHAR);
	(*(DWORD*)(*plpBuff)) = dwSize;
	(*plpBuff) += sizeof(DWORD);
	memmove((*plpBuff), lpwszData, dwSize);
	(*plpBuff) += dwSize;
}

static void SetLPSWtoA(LPBYTE *plpBuff, LPCWSTR lpwszData, DWORD dwSize)
{
	dwSize = WideCharToMultiByte(MRA_CODE_PAGE, 0, lpwszData, dwSize, (LPSTR)((*plpBuff)+sizeof(DWORD)), (dwSize*sizeof(WCHAR)), NULL, NULL);
	(*(DWORD*)(*plpBuff)) = dwSize;
	(*plpBuff) += (sizeof(DWORD)+dwSize);
}

static void SetLPSLowerCase(LPBYTE *plpBuff, LPCSTR lpszData, DWORD dwSize)
{
	(*(DWORD*)(*plpBuff)) = dwSize;
	(*plpBuff) += sizeof(DWORD);
	BuffToLowerCase((*plpBuff), lpszData, dwSize);
	(*plpBuff) += dwSize;
}

static void SetLPSLowerCaseW(LPBYTE *plpBuff, LPCWSTR lpwszData, DWORD dwSize)
{
	dwSize *= sizeof(WCHAR);
	(*(DWORD*)(*plpBuff)) = dwSize;
	(*plpBuff) += sizeof(DWORD);
	memmove((*plpBuff), lpwszData, dwSize);
	CharLowerBuff((LPWSTR)(*plpBuff), (dwSize/sizeof(WCHAR)));
	(*plpBuff) += dwSize;
}

/////////////////////////////////////////////////////////////////////////////////////////

DWORD CMraProto::MraMessageW(BOOL bAddToQueue, HANDLE hContact, DWORD dwAckType, DWORD dwFlags, LPSTR lpszEMail, size_t dwEMailSize, LPCWSTR lpwszMessage, size_t dwMessageSize, LPBYTE lpbMultiChatData, size_t dwMultiChatDataSize)
{
	Netlib_Logf(hNetlibUser, "Sending message: flags %08x, to '%S', message '%S'\n", dwFlags, lpszEMail, lpwszMessage);

	DWORD dwRet = 0;
	LPBYTE lpbData, lpbDataCurrent;
	LPSTR lpszMessageConverted = (LPSTR)lpwszMessage;
	LPSTR lpszMessageRTF = NULL;
	size_t dwMessageConvertedSize = (dwMessageSize*sizeof(WCHAR)), dwMessageRTFSize = 0;

	if ( MraIsMessageFlashAnimation(lpwszMessage, dwMessageSize))
		dwFlags |= MESSAGE_FLAG_FLASH;

	// pack auth message
	if (dwFlags & MESSAGE_FLAG_AUTHORIZE) {
		LPBYTE lpbAuthMsgBuff;
		size_t dwMessageConvertedBuffSize = (((((dwMessageSize*sizeof(WCHAR))+1024)+2)/3)*4);

		lpszMessageConverted = (LPSTR)mir_calloc(dwMessageConvertedBuffSize);
		lpbAuthMsgBuff = (LPBYTE)mir_calloc(((dwMessageSize*sizeof(WCHAR))+1024));
		if (lpszMessageConverted && lpbAuthMsgBuff) {
			lpbDataCurrent = lpbAuthMsgBuff;
			SetUL(&lpbDataCurrent, 2);
			SetLPSW(&lpbDataCurrent, NULL, 0);//***deb possible nick here
			SetLPSW(&lpbDataCurrent, lpwszMessage, dwMessageSize);

			BASE64EncodeUnSafe(lpbAuthMsgBuff, (lpbDataCurrent-lpbAuthMsgBuff), lpszMessageConverted, dwMessageConvertedBuffSize, &dwMessageConvertedSize);
		}
		else {
			mir_free(lpszMessageConverted);
			lpszMessageConverted = (LPSTR)lpwszMessage;
		}
		mir_free(lpbAuthMsgBuff);
	}
	// messages with Flash
	else if (dwFlags & MESSAGE_FLAG_FLASH) {
		size_t dwRFTBuffSize = (((dwMessageSize*sizeof(WCHAR))*4)+8192), dwRTFDataSize;

		dwFlags |= MESSAGE_FLAG_RTF;
		lpszMessageRTF = (LPSTR)mir_calloc(dwRFTBuffSize);
		ptrA lpbRTFData((char*)mir_calloc(dwRFTBuffSize));
		if (lpszMessageRTF && lpbRTFData) {
			DWORD dwBackColour = getDword(NULL, "RTFBackgroundColour", MRA_DEFAULT_RTF_BACKGROUND_COLOUR);
			lpbDataCurrent = (LPBYTE)lpszMessageRTF;

			WideCharToMultiByte(MRA_CODE_PAGE, 0, lpwszMessage, dwMessageSize, (LPSTR)lpbRTFData, dwRFTBuffSize, NULL, NULL);

			SetUL(&lpbDataCurrent, 4);
			SetLPS(&lpbDataCurrent, (LPSTR)lpbRTFData, dwMessageSize);// сообщение что у собеседника плохая версия :)
			SetLPS(&lpbDataCurrent, (LPSTR)&dwBackColour, sizeof(DWORD));// цвет фона
			SetLPS(&lpbDataCurrent, (LPSTR)lpbRTFData, dwMessageSize);// сам мульт ANSI
			SetLPSW(&lpbDataCurrent, lpwszMessage, dwMessageSize);// сам мульт UNICODE

			dwRTFDataSize = dwRFTBuffSize;
			if ( compress2((LPBYTE)(LPSTR)lpbRTFData, (DWORD*)&dwRTFDataSize, (LPBYTE)lpszMessageRTF, (lpbDataCurrent-(LPBYTE)lpszMessageRTF), Z_BEST_COMPRESSION) == Z_OK)
				BASE64EncodeUnSafe(lpbRTFData, dwRTFDataSize, lpszMessageRTF, dwRFTBuffSize, &dwMessageRTFSize);
		}
	}
	// standart message
	else if ((dwFlags & (MESSAGE_FLAG_CONTACT | MESSAGE_FLAG_NOTIFY | MESSAGE_FLAG_SMS)) == 0) {
		// Only if message is simple text message or RTF or ALARM
		if (dwFlags & MESSAGE_FLAG_RTF) { // add RFT part
			size_t dwRFTBuffSize = (((dwMessageSize*sizeof(WCHAR))*16)+8192), dwRTFDataSize;

			lpszMessageRTF = (LPSTR)mir_calloc(dwRFTBuffSize);
			ptrA lpbRTFData((char*)mir_calloc(dwRFTBuffSize));
			if (lpszMessageRTF && lpbRTFData) {
				if ( !MraConvertToRTFW(lpwszMessage, dwMessageSize, (LPSTR)lpbRTFData, dwRFTBuffSize, &dwRTFDataSize)) {
					DWORD dwBackColour = getDword(NULL, "RTFBackgroundColour", MRA_DEFAULT_RTF_BACKGROUND_COLOUR);
					lpbDataCurrent = (LPBYTE)lpszMessageRTF;

					SetUL(&lpbDataCurrent, 2);
					SetLPS(&lpbDataCurrent, (LPSTR)lpbRTFData, dwRTFDataSize);
					SetLPS(&lpbDataCurrent, (LPSTR)&dwBackColour, sizeof(DWORD));

					dwRTFDataSize = dwRFTBuffSize;
					if ( compress2((LPBYTE)(LPSTR)lpbRTFData, (DWORD*)&dwRTFDataSize, (LPBYTE)lpszMessageRTF, (lpbDataCurrent-(LPBYTE)lpszMessageRTF), Z_BEST_COMPRESSION) == Z_OK)
						BASE64EncodeUnSafe(lpbRTFData, dwRTFDataSize, lpszMessageRTF, dwRFTBuffSize, &dwMessageRTFSize);
				}
			}
		}
	}

	if (lpszMessageRTF == NULL || dwMessageRTFSize == 0) dwFlags &= ~(MESSAGE_FLAG_RTF|MESSAGE_FLAG_FLASH);
	if (lpbMultiChatData == NULL || dwMultiChatDataSize == 0) dwFlags &= ~MESSAGE_FLAG_MULTICHAT;

	lpbData = (LPBYTE)mir_calloc((sizeof(DWORD)+dwEMailSize+dwMessageConvertedSize+dwMessageRTFSize+dwMultiChatDataSize+128));
	if (lpbData) {
		lpbDataCurrent = lpbData;
		SetUL(&lpbDataCurrent, dwFlags);
		SetLPSLowerCase(&lpbDataCurrent, lpszEMail, dwEMailSize);
		SetLPS(&lpbDataCurrent, lpszMessageConverted, dwMessageConvertedSize);
		SetLPS(&lpbDataCurrent, lpszMessageRTF, dwMessageRTFSize);
		if (dwFlags&MESSAGE_FLAG_MULTICHAT) SetLPS(&lpbDataCurrent, (LPSTR)lpbMultiChatData, dwMultiChatDataSize);

		if (bAddToQueue)
			dwRet = MraSendQueueCMD(hSendQueueHandle, 0, hContact, dwAckType, (LPBYTE)lpwszMessage, (dwMessageSize*sizeof(WCHAR)), MRIM_CS_MESSAGE, lpbData, (lpbDataCurrent-lpbData));
		else
			dwRet = MraSendCMD(MRIM_CS_MESSAGE, lpbData, (lpbDataCurrent-lpbData));
		mir_free(lpbData);
	}

	if (lpszMessageConverted != (LPSTR)lpwszMessage)
		mir_free(lpszMessageConverted);
	mir_free(lpszMessageRTF);

	return dwRet;
}

// Send confirmation
DWORD CMraProto::MraMessageAskW(DWORD dwMsgID, DWORD dwFlags, LPSTR lpszEMail, size_t dwEMailSize, LPWSTR lpwszMessage, size_t dwMessageSize, LPSTR lpwszMessageRTF, size_t dwMessageRTFSize)
{
	DWORD dwRet = 0;

	if (lpszEMail && dwEMailSize>4 && lpwszMessage && dwMessageSize) {
		LPBYTE lpbData, lpbDataCurrent;
		lpbData = (LPBYTE)mir_calloc((sizeof(DWORD)+sizeof(DWORD)+dwEMailSize+dwMessageSize+dwMessageRTFSize+32));
		if (lpbData) {
			lpbDataCurrent = lpbData;
			SetUL(&lpbDataCurrent, dwMsgID);//UL msg_id
			SetUL(&lpbDataCurrent, dwFlags);//UL flags
			SetLPSLowerCase(&lpbDataCurrent, lpszEMail, dwEMailSize);// LPS from e-mail ANSI
			SetLPS(&lpbDataCurrent, (LPSTR)lpwszMessage, dwMessageSize);// LPS message UNICODE
			if (dwFlags&MESSAGE_FLAG_RTF)
				SetLPS(&lpbDataCurrent, (LPSTR)lpwszMessageRTF, dwMessageRTFSize);// LPS	//rtf-formatted message ( >= 1.1)	- MESSAGE_FLAG_RTF

			dwRet = MraSendCMD(MRIM_CS_MESSAGE_ACK, lpbData, (lpbDataCurrent-lpbData));
			mir_free(lpbData);
		}
	}

	return dwRet;
}

DWORD CMraProto::MraMessageRecv(LPSTR lpszFrom, size_t dwFromSize, DWORD dwMsgID)
{
	DWORD dwRet = 0;

	if (lpszFrom && dwFromSize>4) {
		LPBYTE lpbData, lpbDataCurrent;
		lpbData = (LPBYTE)mir_calloc((dwFromSize+sizeof(DWORD)+32));
		if (lpbData) {
			lpbDataCurrent = lpbData;
			SetLPSLowerCase(&lpbDataCurrent, lpszFrom, dwFromSize);
			SetUL(&lpbDataCurrent, dwMsgID);

			dwRet = MraSendCMD(MRIM_CS_MESSAGE_RECV, lpbData, (lpbDataCurrent-lpbData));
			mir_free(lpbData);
		}
	}
	return dwRet;
}

// Adds new contact
DWORD CMraProto::MraAddContactW(HANDLE hContact, DWORD dwContactFlag, DWORD dwGroupID, LPSTR lpszEMail, size_t dwEMailSize, LPWSTR lpwszCustomName, size_t dwCustomNameSize, LPSTR lpszPhones, size_t dwPhonesSize, LPWSTR lpwszAuthMessage, size_t dwAuthMessageSize, DWORD dwActions)
{
	DWORD dwRet = 0;

	if (lpszEMail && dwEMailSize>4) {
		LPBYTE lpbData = (LPBYTE)mir_calloc((sizeof(DWORD)+sizeof(DWORD)+dwEMailSize+(dwCustomNameSize*sizeof(WCHAR))+dwPhonesSize+(((((dwAuthMessageSize*sizeof(WCHAR))+1024)+2)/3)*4)+32)+sizeof(DWORD));
		if (lpbData) {
			dwContactFlag |= CONTACT_FLAG_UNICODE_NAME;

			LPBYTE lpbDataCurrent = lpbData;
			SetUL(&lpbDataCurrent, dwContactFlag);
			SetUL(&lpbDataCurrent, dwGroupID);
			SetLPSLowerCase(&lpbDataCurrent, lpszEMail, dwEMailSize);
			SetLPSW(&lpbDataCurrent, lpwszCustomName, dwCustomNameSize);
			SetLPS(&lpbDataCurrent, lpszPhones, dwPhonesSize);

			// pack auth message
			LPBYTE lpbAuthMsgBuff, lpbAuthMessageConverted, lpbAuthDataCurrent;
			size_t dwAuthMessageConvertedBuffSize = (((((dwAuthMessageSize*sizeof(WCHAR))+1024)+2)/3)*4), dwAuthMessageConvertedSize = 0;

			lpbAuthMessageConverted = (LPBYTE)mir_calloc(dwAuthMessageConvertedBuffSize);
			lpbAuthMsgBuff = (LPBYTE)mir_calloc(((dwAuthMessageSize*sizeof(WCHAR))+1024));
			if (lpbAuthMessageConverted && lpbAuthMsgBuff) {
				lpbAuthDataCurrent = lpbAuthMsgBuff;
				SetUL(&lpbAuthDataCurrent, 2);
				SetLPSW(&lpbAuthDataCurrent, NULL, 0);//***deb possible nick here
				SetLPSW(&lpbAuthDataCurrent, lpwszAuthMessage, dwAuthMessageSize);

				BASE64EncodeUnSafe(lpbAuthMsgBuff, (lpbAuthDataCurrent-lpbAuthMsgBuff), lpbAuthMessageConverted, dwAuthMessageConvertedBuffSize, &dwAuthMessageConvertedSize);
			}
			SetLPS(&lpbDataCurrent, (LPSTR)lpbAuthMessageConverted, dwAuthMessageConvertedSize);
			mir_free(lpbAuthMsgBuff);
			mir_free(lpbAuthMessageConverted);

			SetUL(&lpbDataCurrent, dwActions);

			dwRet = MraSendQueueCMD(hSendQueueHandle, 0, hContact, ACKTYPE_ADDED, NULL, 0, MRIM_CS_ADD_CONTACT, lpbData, (lpbDataCurrent-lpbData));
			mir_free(lpbData);
		}
	}
	return dwRet;
}

// change contact
DWORD CMraProto::MraModifyContactW(HANDLE hContact, DWORD dwID, DWORD dwContactFlag, DWORD dwGroupID, LPSTR lpszEMail, size_t dwEMailSize, LPWSTR lpwszCustomName, size_t dwCustomNameSize, LPSTR lpszPhones, size_t dwPhonesSize)
{
	DWORD dwRet = 0;
	if (dwID != -1) {
		LPBYTE lpbData = (LPBYTE)mir_calloc((sizeof(DWORD)+sizeof(DWORD)+sizeof(DWORD)+dwEMailSize+(dwCustomNameSize*sizeof(WCHAR))+dwPhonesSize+32));
		if (lpbData)
		{
			dwContactFlag |= CONTACT_FLAG_UNICODE_NAME;

			LPBYTE lpbDataCurrent = lpbData;
			SetUL(&lpbDataCurrent, dwID);
			SetUL(&lpbDataCurrent, dwContactFlag);
			SetUL(&lpbDataCurrent, dwGroupID);
			SetLPSLowerCase(&lpbDataCurrent, lpszEMail, dwEMailSize);
			SetLPSW(&lpbDataCurrent, lpwszCustomName, dwCustomNameSize);
			SetLPS(&lpbDataCurrent, lpszPhones, dwPhonesSize);

			dwRet = MraSendQueueCMD(hSendQueueHandle, 0, hContact, ACKTYPE_ADDED, NULL, 0, MRIM_CS_MODIFY_CONTACT, lpbData, (lpbDataCurrent-lpbData));
			mir_free(lpbData);
		}
	}
	return dwRet;
}

// remove stored message
DWORD CMraProto::MraOfflineMessageDel(DWORDLONG dwMsgUIDL)
{
	return MraSendCMD(MRIM_CS_DELETE_OFFLINE_MESSAGE, &dwMsgUIDL, sizeof(DWORDLONG));
}

// autorize a user & add him to a roster
DWORD CMraProto::MraAuthorize(LPSTR lpszEMail, size_t dwEMailSize)
{
	DWORD dwRet = 0;

	if (lpszEMail && dwEMailSize>4) {
		LPBYTE lpbData;
		lpbData = (LPBYTE)mir_calloc((dwEMailSize+32));
		if (lpbData) {
			LPBYTE lpbDataCurrent = lpbData;
			SetLPSLowerCase(&lpbDataCurrent, lpszEMail, dwEMailSize);
			dwRet = MraSendCMD(MRIM_CS_AUTHORIZE, lpbData, (lpbDataCurrent-lpbData));
			mir_free(lpbData);
		}
	}
	return dwRet;
}

// change status
DWORD CMraProto::MraChangeStatusW(DWORD dwStatus, LPSTR lpszStatusUri, size_t dwStatusUriSize, LPCWSTR lpwszStatusTitle, size_t dwStatusTitleSize, LPCWSTR lpwszStatusDesc, size_t dwStatusDescSize, DWORD dwFutureFlags)
{
	DWORD dwRet = 0;

	if (dwStatusUriSize > SPEC_STATUS_URI_MAX)  dwStatusUriSize = SPEC_STATUS_URI_MAX;
	if (dwStatusTitleSize > STATUS_TITLE_MAX)   dwStatusTitleSize = STATUS_TITLE_MAX;
	if (dwStatusDescSize > STATUS_DESC_MAX)     dwStatusDescSize = STATUS_DESC_MAX;

	LPBYTE lpbData = (LPBYTE)mir_calloc((sizeof(DWORD)+dwStatusUriSize+(dwStatusTitleSize*sizeof(WCHAR))+(dwStatusDescSize*sizeof(WCHAR))+sizeof(DWORD)+32));
	if (lpbData) {
		LPBYTE lpbDataCurrent = lpbData;
		SetUL(&lpbDataCurrent, dwStatus);
		SetLPS(&lpbDataCurrent, lpszStatusUri, dwStatusUriSize);
		SetLPSW(&lpbDataCurrent, lpwszStatusTitle, dwStatusTitleSize);
		SetLPSW(&lpbDataCurrent, lpwszStatusDesc, dwStatusDescSize);
		SetUL(&lpbDataCurrent, dwFutureFlags);

		dwRet = MraSendCMD(MRIM_CS_CHANGE_STATUS, lpbData, (lpbDataCurrent-lpbData));
		mir_free(lpbData);
	}
	return dwRet;
}

// Отправка файлов
DWORD CMraProto::MraFileTransfer(LPSTR lpszEMail, size_t dwEMailSize, DWORD dwIDRequest, DWORD dwFilesTotalSize, LPWSTR lpwszFiles, size_t dwFilesSize, LPSTR lpszAddreses, size_t dwAddressesSize)
{
	DWORD dwRet = 0;

	if (lpszEMail && dwEMailSize>4) {
		int dwFilesSizeA = WideCharToMultiByte(MRA_CODE_PAGE, 0, lpwszFiles, dwFilesSize, NULL, 0, NULL, NULL);
		LPBYTE lpbData = (LPBYTE)mir_calloc((dwEMailSize+dwFilesSizeA+(dwFilesSize*sizeof(WCHAR))+dwAddressesSize+MAX_PATH));
		if (lpbData) {
			LPBYTE lpbDataCurrent = lpbData;
			SetLPSLowerCase(&lpbDataCurrent, lpszEMail, dwEMailSize);
			SetUL(&lpbDataCurrent, dwIDRequest);
			SetUL(&lpbDataCurrent, dwFilesTotalSize);
			SetUL(&lpbDataCurrent, sizeof(DWORD)*5 + dwFilesSizeA + dwFilesSize*sizeof(WCHAR) + DWORD(dwAddressesSize));

			SetLPSWtoA(&lpbDataCurrent, lpwszFiles, dwFilesSize);
			SetUL(&lpbDataCurrent, sizeof(DWORD)*2 + dwFilesSize*sizeof(WCHAR));

			SetUL(&lpbDataCurrent, 1);
			SetLPSW(&lpbDataCurrent, lpwszFiles, dwFilesSize);

			SetLPS(&lpbDataCurrent, lpszAddreses, dwAddressesSize);

			dwRet = MraSendCMD(MRIM_CS_FILE_TRANSFER, lpbData, lpbDataCurrent-lpbData);
			mir_free(lpbData);
		}
	}
	return dwRet;
}

// Ответ на отправку файлов
DWORD CMraProto::MraFileTransferAck(DWORD dwStatus, LPSTR lpszEMail, size_t dwEMailSize, DWORD dwIDRequest, LPBYTE lpbDescription, size_t dwDescriptionSize)
{
	DWORD dwRet = 0;

	if (lpszEMail && dwEMailSize>4) {
		LPBYTE lpbData = (LPBYTE)mir_calloc((dwEMailSize+dwDescriptionSize+32));
		if (lpbData) {
			LPBYTE lpbDataCurrent = lpbData;
			SetUL(&lpbDataCurrent, dwStatus);
			SetLPSLowerCase(&lpbDataCurrent, lpszEMail, dwEMailSize);
			SetUL(&lpbDataCurrent, dwIDRequest);
			SetLPS(&lpbDataCurrent, (LPSTR)lpbDescription, dwDescriptionSize);

			dwRet = MraSendCMD(MRIM_CS_FILE_TRANSFER_ACK, lpbData, (lpbDataCurrent-lpbData));
			mir_free(lpbData);
		}
	}
	return dwRet;
}

// Поиск контакта
HANDLE CMraProto::MraWPRequestW(HANDLE hContact, DWORD dwAckType, DWORD dwRequestFlags, LPSTR lpszUser, size_t dwUserSize, LPSTR lpszDomain, size_t dwDomainSize, LPCWSTR lpwszNickName, size_t dwNickNameSize, LPCWSTR lpwszFirstName, size_t dwFirstNameSize, LPCWSTR lpwszLastName, size_t dwLastNameSize, DWORD dwSex, DWORD dwDate1, DWORD dwDate2, DWORD dwCityID, DWORD dwZodiak, DWORD dwBirthdayMonth, DWORD dwBirthdayDay, DWORD dwCountryID, DWORD dwOnline)
{
	DWORD dwRet = 0;
	LPBYTE lpbData = (LPBYTE)mir_calloc(((dwUserSize+dwDomainSize+dwNickNameSize+dwFirstNameSize+dwLastNameSize)*sizeof(WCHAR))+4096);
	if (lpbData) {
		CHAR szBuff[MAX_PATH];
		size_t dwBuffSize;

		LPBYTE lpbDataCurrent = lpbData;

		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_USER))      { SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_USER);SetLPSLowerCase(&lpbDataCurrent, lpszUser, dwUserSize); }
		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_DOMAIN))    { SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_DOMAIN);SetLPSLowerCase(&lpbDataCurrent, lpszDomain, dwDomainSize); }
		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_NICKNAME))  { SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_NICKNAME);SetLPSW(&lpbDataCurrent, lpwszNickName, dwNickNameSize); }
		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_FIRSTNAME)) { SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_FIRSTNAME);SetLPSW(&lpbDataCurrent, lpwszFirstName, dwFirstNameSize); }
		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_LASTNAME))  { SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_LASTNAME);SetLPSW(&lpbDataCurrent, lpwszLastName, dwLastNameSize); }

		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_SEX)) {
			dwBuffSize = mir_snprintf(szBuff, SIZEOF(szBuff), "%lu", dwSex);
			SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_SEX);
			SetLPS(&lpbDataCurrent, szBuff, dwBuffSize);
		}

		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_DATE1)) {
			dwBuffSize = mir_snprintf(szBuff, SIZEOF(szBuff), "%lu", dwDate1);
			SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_DATE1);
			SetLPS(&lpbDataCurrent, szBuff, dwBuffSize);
		}

		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_DATE2)) {
			dwBuffSize = mir_snprintf(szBuff, SIZEOF(szBuff), "%lu", dwDate2);
			SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_DATE2);
			SetLPS(&lpbDataCurrent, szBuff, dwBuffSize);
		}

		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_CITY_ID)) {
			dwBuffSize = mir_snprintf(szBuff, SIZEOF(szBuff), "%lu", dwCityID);
			SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_CITY_ID);
			SetLPS(&lpbDataCurrent, szBuff, dwBuffSize);
		}

		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_ZODIAC)) {
			dwBuffSize = mir_snprintf(szBuff, SIZEOF(szBuff), "%lu", dwZodiak);
			SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_ZODIAC);
			SetLPS(&lpbDataCurrent, szBuff, dwBuffSize);
		}

		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_MONTH)) {
			dwBuffSize = mir_snprintf(szBuff, SIZEOF(szBuff), "%lu", dwBirthdayMonth);
			SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_MONTH);
			SetLPS(&lpbDataCurrent, szBuff, dwBuffSize);
		}

		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_DAY)) {
			dwBuffSize = mir_snprintf(szBuff, SIZEOF(szBuff), "%lu", dwBirthdayDay);
			SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_DAY);
			SetLPS(&lpbDataCurrent, szBuff, dwBuffSize);
		}

		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_COUNTRY_ID)) {
			dwBuffSize = mir_snprintf(szBuff, SIZEOF(szBuff), "%lu", dwCountryID);
			SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_COUNTRY_ID);
			SetLPS(&lpbDataCurrent, szBuff, dwBuffSize);
		}

		if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_ONLINE)) {
			dwBuffSize = mir_snprintf(szBuff, SIZEOF(szBuff), "%lu", dwOnline);
			SetUL(&lpbDataCurrent, MRIM_CS_WP_REQUEST_PARAM_ONLINE);
			SetLPS(&lpbDataCurrent, szBuff, dwBuffSize);
		}

		dwRet = MraSendQueueCMD(hSendQueueHandle, 0, hContact, dwAckType, NULL, 0, MRIM_CS_WP_REQUEST, lpbData, (lpbDataCurrent-lpbData));
		mir_free(lpbData);
	}
	return (HANDLE)dwRet;
}

// Поиск контакта по EMail
HANDLE CMraProto::MraWPRequestByEMail(HANDLE hContact, DWORD dwAckType, LPCSTR lpszEMail, size_t dwEMailSize)
{
	HANDLE dwRet = 0;

	if (lpszEMail && dwEMailSize>4) {
		size_t dwUserSize, dwDomainSize;
		LPSTR lpszDomain = (LPSTR)MemoryFindByte(0, lpszEMail, dwEMailSize, '@');
		if (lpszDomain) {
			LPSTR lpszUser = (LPSTR)lpszEMail;
			dwUserSize = (lpszDomain-lpszEMail);
			lpszDomain++;
			dwDomainSize = (dwEMailSize-(dwUserSize+1));

			DWORD dwRequestFlags = 0;
			SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_USER);
			SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_DOMAIN);

			dwRet = MraWPRequestW(hContact, dwAckType, dwRequestFlags, lpszUser, dwUserSize, lpszDomain, dwDomainSize, NULL, 0, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		}
	}
	return dwRet;
}

// Отправка файлов
DWORD CMraProto::MraGame(LPSTR lpszEMail, size_t dwEMailSize, DWORD dwGameSessionID, DWORD dwGameMsg, DWORD dwGameMsgID, LPSTR lpszData, size_t dwDataSize)
{
	DWORD dwRet = 0;

	if (lpszEMail && dwEMailSize>4) {
		LPBYTE lpbData = (LPBYTE)mir_calloc((dwEMailSize+(sizeof(DWORD)*4)+dwDataSize+32));
		if (lpbData) {
			LPBYTE lpbDataCurrent = lpbData;
			SetLPSLowerCase(&lpbDataCurrent, lpszEMail, dwEMailSize);
			SetUL(&lpbDataCurrent, dwGameSessionID);
			SetUL(&lpbDataCurrent, dwGameMsg);
			SetUL(&lpbDataCurrent, dwGameMsgID);
			SetUL(&lpbDataCurrent, _time32(NULL));
			SetLPS(&lpbDataCurrent, lpszData, dwDataSize);

			dwRet = MraSendCMD(MRIM_CS_GAME, lpbData, (lpbDataCurrent-lpbData));
			mir_free(lpbData);
		}
	}
	return dwRet;
}

// Авторизация
DWORD CMraProto::MraLogin2W(LPSTR lpszLogin, size_t dwLoginSize, LPSTR lpszPassword, size_t dwPasswordSize, DWORD dwStatus, LPSTR lpszStatusUri, size_t dwStatusUriSize, LPWSTR lpwszStatusTitle, size_t dwStatusTitleSize, LPWSTR lpwszStatusDesc, size_t dwStatusDescSize, DWORD dwFutureFlags, LPSTR dwUserAgentFormatted, size_t dwUserAgentFormattedSize, LPSTR lpszUserAgent, size_t dwUserAgentSize)
{
	DWORD dwRet = 0;

	if (dwStatusUriSize>SPEC_STATUS_URI_MAX)    dwStatusUriSize = SPEC_STATUS_URI_MAX;
	if (dwStatusTitleSize>STATUS_TITLE_MAX)     dwStatusTitleSize = STATUS_TITLE_MAX;
	if (dwStatusDescSize>STATUS_DESC_MAX)       dwStatusDescSize = STATUS_DESC_MAX;
	if (dwUserAgentFormattedSize>USER_AGENT_MAX) dwUserAgentFormattedSize = USER_AGENT_MAX;
	if (dwUserAgentSize>MAX_CLIENT_DESCRIPTION) dwUserAgentSize = MAX_CLIENT_DESCRIPTION;

	LPBYTE lpbData = (LPBYTE)mir_calloc((dwLoginSize+dwPasswordSize+sizeof(DWORD)+dwStatusUriSize+(dwStatusTitleSize*sizeof(WCHAR))+(dwStatusDescSize*sizeof(WCHAR))+2+sizeof(DWORD)+(sizeof(DWORD)*2)+dwUserAgentFormattedSize+dwUserAgentSize+32));
	if (lpbData) {
		LPBYTE lpbDataCurrent = lpbData;
		SetLPS(&lpbDataCurrent, lpszLogin, dwLoginSize);
		SetLPS(&lpbDataCurrent, lpszPassword, dwPasswordSize);
		SetUL(&lpbDataCurrent, dwStatus);
		SetLPS(&lpbDataCurrent, lpszStatusUri, dwStatusUriSize);
		SetLPSW(&lpbDataCurrent, lpwszStatusTitle, dwStatusTitleSize);
		SetLPSW(&lpbDataCurrent, lpwszStatusDesc, dwStatusDescSize);
		SetUL(&lpbDataCurrent, dwFutureFlags);
		SetLPS(&lpbDataCurrent, dwUserAgentFormatted, dwUserAgentFormattedSize);
		SetLPS(&lpbDataCurrent, "ru", 2);
		SetLPS(&lpbDataCurrent, NULL, 0);
		SetLPS(&lpbDataCurrent, NULL, 0);
		SetLPS(&lpbDataCurrent, lpszUserAgent, dwUserAgentSize);// LPS client description /max 256

		dwRet = MraSendCMD(MRIM_CS_LOGIN2, lpbData, (lpbDataCurrent-lpbData));
		mir_free(lpbData);
	}
	return dwRet;
}

// Отправка SMS
DWORD CMraProto::MraSMSW(HANDLE hContact, LPSTR lpszPhone, size_t dwPhoneSize, LPWSTR lpwszMessage, size_t dwMessageSize)
{
	DWORD dwRet = 0;
	LPBYTE lpbData = (LPBYTE)mir_calloc((dwPhoneSize+(dwMessageSize*sizeof(WCHAR))+32));
	LPBYTE lpbDataQueue = (LPBYTE)mir_calloc((dwPhoneSize+(dwMessageSize*sizeof(WCHAR))+32));
	LPSTR lpszPhoneLocal = (LPSTR)mir_calloc((dwPhoneSize+32));
	if (lpbData && lpbDataQueue && lpszPhoneLocal) {
		lpszPhoneLocal[0] = '+';
		dwPhoneSize = 1+CopyNumber((lpszPhoneLocal+1), lpszPhone, dwPhoneSize);

		LPBYTE lpbDataCurrent = lpbData;
		SetUL(&lpbDataCurrent, 0);
		SetLPS(&lpbDataCurrent, lpszPhoneLocal, dwPhoneSize);
		SetLPSW(&lpbDataCurrent, lpwszMessage, dwMessageSize);

		(*(DWORD*)lpbDataQueue) = dwPhoneSize;
		memmove((lpbDataQueue+sizeof(DWORD)), lpszPhoneLocal, (dwPhoneSize+1));
		memmove((lpbDataQueue+sizeof(DWORD)+dwPhoneSize+1), lpwszMessage, ((dwMessageSize*sizeof(WCHAR))+1));

		dwRet = MraSendQueueCMD(hSendQueueHandle, 0, hContact, ICQACKTYPE_SMS, lpbDataQueue, (dwPhoneSize+(dwMessageSize*sizeof(WCHAR))+sizeof(DWORD)+2), MRIM_CS_SMS, lpbData, (lpbDataCurrent-lpbData));
		mir_free(lpbData);
		mir_free(lpszPhoneLocal);
	}
	else {
		mir_free(lpbData);
		mir_free(lpbDataQueue);
		mir_free(lpszPhoneLocal);
	}
	return dwRet;
}

// Соединение с прокси
DWORD CMraProto::MraProxy(LPSTR lpszEMail, size_t dwEMailSize, DWORD dwIDRequest, DWORD dwDataType, LPSTR lpszData, size_t dwDataSize, LPSTR lpszAddreses, size_t dwAddressesSize, MRA_GUID mguidSessionID)
{
	DWORD dwRet = 0;

	if (lpszEMail && dwEMailSize>4) {
		LPBYTE lpbData = (LPBYTE)mir_calloc((dwEMailSize+(sizeof(DWORD)*2)+dwDataSize+dwAddressesSize+sizeof(MRA_GUID)+32));
		if (lpbData) {
			LPBYTE lpbDataCurrent = lpbData;
			SetLPSLowerCase(&lpbDataCurrent, lpszEMail, dwEMailSize);
			SetUL(&lpbDataCurrent, dwIDRequest);
			SetUL(&lpbDataCurrent, dwDataType);
			SetLPS(&lpbDataCurrent, lpszData, dwDataSize);
			SetLPS(&lpbDataCurrent, lpszAddreses, dwAddressesSize);
			SetGUID(&lpbDataCurrent, mguidSessionID);

			dwRet = MraSendCMD(MRIM_CS_PROXY, lpbData, (lpbDataCurrent-lpbData));
			mir_free(lpbData);
		}
	}
	return dwRet;
}

// Ответ на соединение с прокси
DWORD CMraProto::MraProxyAck(DWORD dwStatus, LPSTR lpszEMail, size_t dwEMailSize, DWORD dwIDRequest, DWORD dwDataType, LPSTR lpszData, size_t dwDataSize, LPSTR lpszAddreses, size_t dwAddressesSize, MRA_GUID mguidSessionID)
{
	DWORD dwRet = 0;

	if (lpszEMail && dwEMailSize>4) {
		LPBYTE lpbData = (LPBYTE)mir_calloc((dwEMailSize+(sizeof(DWORD)*3)+dwDataSize+dwAddressesSize+sizeof(MRA_GUID)+32));
		if (lpbData) {
			LPBYTE lpbDataCurrent = lpbData;
			SetUL(&lpbDataCurrent, dwStatus);
			SetLPSLowerCase(&lpbDataCurrent, lpszEMail, dwEMailSize);
			SetUL(&lpbDataCurrent, dwIDRequest);
			SetUL(&lpbDataCurrent, dwDataType);
			SetLPS(&lpbDataCurrent, lpszData, dwDataSize);
			SetLPS(&lpbDataCurrent, lpszAddreses, dwAddressesSize);
			SetGUID(&lpbDataCurrent, mguidSessionID);

			dwRet = MraSendCMD(MRIM_CS_PROXY_ACK, lpbData, (lpbDataCurrent-lpbData));
			mir_free(lpbData);
		}
	}
	return dwRet;
}

// Отправка сообщения в микроблог
DWORD CMraProto::MraChangeUserBlogStatus(DWORD dwFlags, LPWSTR lpwszText, size_t dwTextSize, DWORDLONG dwBlogStatusID)
{
	DWORD dwRet = 0;
	if (dwTextSize > MICBLOG_STATUS_MAX)
		dwTextSize = MICBLOG_STATUS_MAX;

	LPBYTE lpbData = (LPBYTE)mir_calloc((sizeof(DWORD)+(dwTextSize*sizeof(WCHAR))+sizeof(DWORDLONG)+32));
	if (lpbData) {
		LPBYTE lpbDataCurrent = lpbData;
		SetUL(&lpbDataCurrent, dwFlags);
		SetLPSW(&lpbDataCurrent, lpwszText, dwTextSize);
		SetUIDL(&lpbDataCurrent, dwBlogStatusID);

		dwRet = MraSendCMD(MRIM_CS_CHANGE_USER_BLOG_STATUS, lpbData, (lpbDataCurrent-lpbData));
		mir_free(lpbData);
	}

	return dwRet;
}

DWORD CMraProto::MraSendPacket(HANDLE hConnection, DWORD dwCMDNum, DWORD dwType, LPVOID lpData, size_t dwDataSize)
{
	DWORD dwRet;

	LPBYTE lpbData = (LPBYTE)mir_calloc((dwDataSize+sizeof(mrim_packet_header_t)));
	if (lpbData) {
		mrim_packet_header_t *pmaHeader = (mrim_packet_header_t*)lpbData;
		pmaHeader->magic = CS_MAGIC;
		pmaHeader->proto = (PROTO_VERSION_MAJOR<<16) + PROTO_VERSION_MINOR; // Версия протокола
		pmaHeader->seq = dwCMDNum;// Sequence
		pmaHeader->msg = dwType;// Тип пакета
		pmaHeader->dlen = dwDataSize;// Длина данных

		Netlib_Logf(hNetlibUser, "Sending packet %08x\n", dwType);

		memmove((lpbData+sizeof(mrim_packet_header_t)), lpData, dwDataSize);
		dwRet = Netlib_Send(hConnection, (LPSTR)lpbData, (dwDataSize+sizeof(mrim_packet_header_t)), 0);
		mir_free(lpbData);
	}
	return dwRet;
}

DWORD CMraProto::MraSendCMD(DWORD dwType, LPVOID lpData, size_t dwDataSize)
{
	DWORD dwRet = InterlockedIncrement((LONG volatile*)&dwCMDNum);

	mir_cslock l(csCriticalSectionSend); // guarding winsock internal buffers
	return !MraSendPacket(hConnection, dwRet, dwType, lpData, dwDataSize) ? 0 : dwRet;
}


DWORD CMraProto::MraSendQueueCMD(HANDLE hSendQueueHandle, DWORD dwFlags, HANDLE hContact, DWORD dwAckType, LPBYTE lpbDataQueue, size_t dwDataQueueSize, DWORD dwType, LPVOID lpData, size_t dwDataSize)
{
	DWORD dwRet = InterlockedIncrement((LONG volatile*)&dwCMDNum);
	if ( !MraSendQueueAdd(hSendQueueHandle, dwRet, dwFlags, hContact, dwAckType, lpbDataQueue, dwDataQueueSize)) {
		mir_cslock l(csCriticalSectionSend); // guarding winsock internal buffers
		if ( !MraSendPacket(hConnection, dwRet, dwType, lpData, dwDataSize)) {
			MraSendQueueFree(hSendQueueHandle, dwRet);
			dwRet = 0;
		}
	}
	return dwRet;
}
