#include "Mra.h"
#include "MraRTFMsg.h"
#include "proto.h"

class OutBuffer
{
	PBYTE   m_buf;
	size_t  m_max, m_actual;

public:
	OutBuffer() : m_buf(0), m_max(0), m_actual(0) {}
	~OutBuffer() { if (m_buf) free(m_buf); }

	void* Allocate(size_t len)
	{
		if (m_actual + len >= m_max) {
			size_t increment = (len > 4096) ? len+4096 : 4096;
			m_buf = (PBYTE)realloc(m_buf, m_max += increment);
		}

		void *res = m_buf + m_actual; m_actual += len;
		return res;
	}

	PBYTE Data() const { return m_buf; }
	size_t Len() const { return m_actual; }

	void SetUL(DWORD dwData)
	{
		*(DWORD*)Allocate(sizeof(DWORD)) = dwData;
	}

	void SetUIDL(DWORDLONG dwData)
	{
		*(DWORDLONG*)Allocate(sizeof(dwData)) = dwData;
	}

	void SetGUID(MRA_GUID guidData)
	{
		*(MRA_GUID*)Allocate(sizeof(guidData)) = guidData;
	}

	void SetLPS(const CMStringA &str)
	{
		SetUL(str.GetLength());
		memcpy( Allocate(str.GetLength()), str, str.GetLength());
	}

	void SetLPSW(const CMStringW &str)
	{
		DWORD dwBytes = str.GetLength() * sizeof(WCHAR);
		SetUL(dwBytes);
		memcpy( Allocate(dwBytes), str, dwBytes);
	}

	void SetLPSLowerCase(const CMStringA &str)
	{
		SetUL(str.GetLength());
		BuffToLowerCase((LPSTR)Allocate(str.GetLength()), str, str.GetLength());
	}

	void SetLPSLowerCaseW(const CMStringW &str)
	{
		DWORD dwSize = str.GetLength() * sizeof(WCHAR);
		SetUL(dwSize);
		WCHAR *buf = (WCHAR*)Allocate(dwSize);
		memcpy(buf, str, dwSize);
		CharLowerBuff(buf, str.GetLength());
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

DWORD CMraProto::MraMessage(BOOL bAddToQueue, MCONTACT hContact, DWORD dwAckType, DWORD dwFlags, const CMStringA &szEmail, const CMStringW &lpwszMessage, LPBYTE lpbMultiChatData, size_t dwMultiChatDataSize)
{
	debugLogA("Sending message: flags %08x, to '%S', message '%S'\n", dwFlags, szEmail, lpwszMessage);

	DWORD dwRet = 0;
	LPSTR lpszMessageConverted = (LPSTR)lpwszMessage.GetString();
	LPSTR lpszMessageRTF = NULL;
	size_t dwMessageConvertedSize = lpwszMessage.GetLength()*sizeof(WCHAR), dwMessageRTFSize = 0;

	if ( MraIsMessageFlashAnimation(lpwszMessage))
		dwFlags |= MESSAGE_FLAG_FLASH;

	// pack auth message
	if (dwFlags & MESSAGE_FLAG_AUTHORIZE) {
		OutBuffer buf;
		buf.SetUL(2);
		buf.SetLPSW(_T(""));//***deb possible nick here
		buf.SetLPSW(lpwszMessage);
		lpszMessageConverted = mir_base64_encode(buf.Data(), buf.Len());
		dwMessageConvertedSize = strlen(lpszMessageConverted);
	}
	// messages with Flash
	else if (dwFlags & MESSAGE_FLAG_FLASH) {
		dwFlags |= MESSAGE_FLAG_RTF;

		DWORD dwBackColour = getDword("RTFBackgroundColour", MRA_DEFAULT_RTF_BACKGROUND_COLOUR);
		char lpbRTFData[10000];

		OutBuffer buf;
		buf.SetUL(4);
		buf.SetLPS(lpbRTFData);// сообщение что у собеседника плохая версия :)
		buf.SetUL(4);
		buf.SetUL(dwBackColour);
		buf.SetLPS(lpbRTFData);// сам мульт ANSI
		buf.SetLPSW(lpwszMessage);// сам мульт UNICODE

		DWORD dwRTFDataSize = buf.Len();
		if ( compress2((LPBYTE)lpbRTFData, &dwRTFDataSize, buf.Data(), buf.Len(), Z_BEST_COMPRESSION) == Z_OK) {
			lpszMessageRTF = mir_base64_encode((LPBYTE)lpbRTFData, dwRTFDataSize);
			dwMessageRTFSize = lstrlenA(lpszMessageRTF);
		}
	}
	// standart message
	else if ((dwFlags & (MESSAGE_FLAG_CONTACT | MESSAGE_FLAG_NOTIFY | MESSAGE_FLAG_SMS)) == 0) {
		// Only if message is simple text message or RTF or ALARM
		if (dwFlags & MESSAGE_FLAG_RTF) { // add RFT part
			CMStringA lpbRTFData; lpbRTFData.Truncate(lpwszMessage.GetLength()*16 + 4096);
			if ( !MraConvertToRTFW(lpwszMessage, lpbRTFData)) {
				DWORD dwBackColour = getDword("RTFBackgroundColour", MRA_DEFAULT_RTF_BACKGROUND_COLOUR);
				
				OutBuffer buf;
				buf.SetUL(2);
				buf.SetLPS(lpbRTFData);
				buf.SetUL(4);
				buf.SetUL(dwBackColour);

				DWORD dwRTFDataSize = lpbRTFData.GetLength();
				if ( compress2((LPBYTE)(LPCSTR)lpbRTFData, &dwRTFDataSize, buf.Data(), buf.Len(), Z_BEST_COMPRESSION) == Z_OK) {
					lpszMessageRTF = mir_base64_encode((LPBYTE)(LPCSTR)lpbRTFData, dwRTFDataSize);
					dwMessageRTFSize = lstrlenA(lpszMessageRTF);
				}
			}
		}
	}

	if (lpszMessageRTF == NULL || dwMessageRTFSize == 0) dwFlags &= ~(MESSAGE_FLAG_RTF|MESSAGE_FLAG_FLASH);
	if (lpbMultiChatData == NULL || dwMultiChatDataSize == 0) dwFlags &= ~MESSAGE_FLAG_MULTICHAT;

	OutBuffer buf;
	buf.SetUL(dwFlags);
	buf.SetLPSLowerCase(szEmail);
	buf.SetLPS(CMStringA(lpszMessageConverted, dwMessageConvertedSize));
	buf.SetLPS(lpszMessageRTF);
	if (dwFlags & MESSAGE_FLAG_MULTICHAT)
		buf.SetLPS(CMStringA((LPSTR)lpbMultiChatData, dwMultiChatDataSize));

	if (bAddToQueue)
		dwRet = MraSendQueueCMD(hSendQueueHandle, 0, hContact, dwAckType, NULL, 0, MRIM_CS_MESSAGE, buf.Data(), buf.Len());
	else
		dwRet = MraSendCMD(MRIM_CS_MESSAGE, buf.Data(), buf.Len());

	return dwRet;
}

// Send confirmation
DWORD CMraProto::MraMessageAsk(DWORD dwMsgID, DWORD dwFlags, const CMStringA &szEmail, const CMStringW &lpwszMessage, const CMStringW &lpwszMessageRTF)
{
	if (szEmail.GetLength() <= 4 || lpwszMessage.IsEmpty())
		return 0;

	OutBuffer buf;
	buf.SetUL(dwMsgID);//UL msg_id
	buf.SetUL(dwFlags);//UL flags
	buf.SetLPSLowerCase(szEmail);// LPS from e-mail ANSI
	buf.SetLPSW(lpwszMessage);// LPS message UNICODE
	if (dwFlags & MESSAGE_FLAG_RTF)
		buf.SetLPSW(lpwszMessageRTF);// LPS	//rtf-formatted message ( >= 1.1)	- MESSAGE_FLAG_RTF

	return MraSendCMD(MRIM_CS_MESSAGE_ACK, buf.Data(), buf.Len());
}

DWORD CMraProto::MraMessageRecv(const CMStringA &szFrom, DWORD dwMsgID)
{
	if (szFrom.GetLength() <= 4)
		return 0;

	OutBuffer buf;
	buf.SetLPSLowerCase(szFrom);
	buf.SetUL(dwMsgID);
	return MraSendCMD(MRIM_CS_MESSAGE_RECV, buf.Data(), buf.Len());
}

// Adds new contact
DWORD CMraProto::MraAddContact(MCONTACT hContact, DWORD dwContactFlag, DWORD dwGroupID, const CMStringA &szEmail, const CMStringW &wszCustomName, const CMStringA *szPhones, const CMString* wszAuthMessage)
{
	if (szEmail.GetLength() <= 4 && !(dwContactFlag & CONTACT_FLAG_GROUP))
		return 0;

	dwContactFlag |= CONTACT_FLAG_UNICODE_NAME;
	if (dwGroupID == -1)
		dwGroupID = 0;

	OutBuffer buf;
	buf.SetUL(dwContactFlag);
	buf.SetUL(dwGroupID);
	buf.SetLPSLowerCase(szEmail);
	buf.SetLPSW(wszCustomName);
	buf.SetLPS((szPhones == NULL) ? "" : *szPhones);

	// pack auth message
	OutBuffer buf2;
	buf2.SetUL(2);
	buf2.SetLPSW(_T(""));//***deb possible nick here
	buf2.SetLPSW((wszAuthMessage == NULL) ? _T("") : *wszAuthMessage);
	buf.SetLPS(CMStringA( ptrA( mir_base64_encode(buf2.Data(), buf2.Len()))));

	buf.SetUL(0);

	return MraSendQueueCMD(hSendQueueHandle, 0, hContact, ACKTYPE_ADDED, NULL, 0, MRIM_CS_ADD_CONTACT, buf.Data(), buf.Len());
}

// change contact
DWORD CMraProto::MraModifyContact(MCONTACT hContact, DWORD *pdwID, DWORD *pdwContactFlag, DWORD *pdwGroupID, const CMStringA *pszEmail, const CMStringW *pwszCustomName, const CMStringA *pszPhones)
{
	CMStringA szEmail, szPhones;
	CMStringW wszNick, wszCustomName;
	DWORD dwID, dwGroupID, dwContactFlag;
	if (hContact)
		GetContactBasicInfoW(hContact, &dwID, &dwGroupID, &dwContactFlag, NULL, NULL, &szEmail, &wszNick, &szPhones);
	else
		dwID = dwGroupID = dwContactFlag = 0;

	if (pdwID) dwID = *pdwID;
	if (dwID == -1)
		return 0;

	if (pszEmail) szEmail = *pszEmail;
	if (pszPhones) szPhones = *pszPhones;
	if (pdwGroupID) dwGroupID = *pdwGroupID;
	if (pdwContactFlag) dwContactFlag = *pdwContactFlag;
	if (pwszCustomName) wszCustomName = *pwszCustomName;

	OutBuffer buf;
	buf.SetUL(dwID);
	buf.SetUL(dwContactFlag | CONTACT_FLAG_UNICODE_NAME);
	buf.SetUL(dwGroupID);
	buf.SetLPSLowerCase(szEmail);
	buf.SetLPSW(wszCustomName);
	buf.SetLPS(szPhones);

	return MraSendQueueCMD(hSendQueueHandle, 0, hContact, ACKTYPE_ADDED, NULL, 0, MRIM_CS_MODIFY_CONTACT, buf.Data(), buf.Len());
}

// remove stored message
DWORD CMraProto::MraOfflineMessageDel(DWORDLONG dwMsgUIDL)
{
	return MraSendCMD(MRIM_CS_DELETE_OFFLINE_MESSAGE, &dwMsgUIDL, sizeof(DWORDLONG));
}

// autorize a user & add him to a roster
DWORD CMraProto::MraAuthorize(const CMStringA& szEmail)
{
	if ( szEmail.GetLength() <= 4)
		return 0;

	OutBuffer buf;
	buf.SetLPSLowerCase(szEmail);
	return MraSendCMD(MRIM_CS_AUTHORIZE, buf.Data(), buf.Len());
}

// change status
DWORD CMraProto::MraChangeStatus(DWORD dwStatus, const CMStringA &szStatusUri, const CMStringW &wszStatusTitle, const CMStringW &wszStatusDesc, DWORD dwFutureFlags)
{
	OutBuffer buf;
	buf.SetUL(dwStatus);
	buf.SetLPS(szStatusUri);
	buf.SetLPSW(wszStatusTitle);
	buf.SetLPSW(wszStatusDesc);
	buf.SetUL(dwFutureFlags);
	return MraSendCMD(MRIM_CS_CHANGE_STATUS, buf.Data(), buf.Len());
}

// Отправка файлов
DWORD CMraProto::MraFileTransfer(const CMStringA &szEmail, DWORD dwIDRequest, DWORD dwFilesTotalSize, const CMStringW &wszFiles, const CMStringA &szAddresses)
{
	if (szEmail.GetLength() <= 4)
		return 0;

	CMStringA szFiles = wszFiles;

	OutBuffer buf;
	buf.SetLPSLowerCase(szEmail);
	buf.SetUL(dwIDRequest);
	buf.SetUL(dwFilesTotalSize);
	buf.SetUL(sizeof(DWORD)*5 + szFiles.GetLength() + wszFiles.GetLength()*sizeof(WCHAR) + szAddresses.GetLength());

	buf.SetLPS(szFiles);
	buf.SetUL(sizeof(DWORD)*2 + wszFiles.GetLength()*sizeof(WCHAR));

	buf.SetUL(1);
	buf.SetLPSW(wszFiles);

	buf.SetLPS(szAddresses);
	return MraSendCMD(MRIM_CS_FILE_TRANSFER, buf.Data(), buf.Len());
}

// Ответ на отправку файлов
DWORD CMraProto::MraFileTransferAck(DWORD dwStatus, const CMStringA &szEmail, DWORD dwIDRequest, LPBYTE lpbDescription, size_t dwDescriptionSize)
{
	if (szEmail.GetLength() <= 4)
		return 0;

	OutBuffer buf;
	buf.SetUL(dwStatus);
	buf.SetLPSLowerCase(szEmail);
	buf.SetUL(dwIDRequest);
	buf.SetLPS(lpbDescription);
	return MraSendCMD(MRIM_CS_FILE_TRANSFER_ACK, buf.Data(), buf.Len());
}

// Поиск контакта
HANDLE CMraProto::MraWPRequestW(MCONTACT hContact, DWORD dwAckType, DWORD dwRequestFlags, const CMStringA &szUser, const CMStringA &szDomain, const CMStringW &wszNickName, const CMStringW &wszFirstName, const CMStringW &wszLastName, DWORD dwSex, DWORD dwDate1, DWORD dwDate2, DWORD dwCityID, DWORD dwZodiak, DWORD dwBirthdayMonth, DWORD dwBirthdayDay, DWORD dwCountryID, DWORD dwOnline)
{
	OutBuffer buf;
	CMStringA tmp;

	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_USER))      { buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_USER); buf.SetLPSLowerCase(szUser); }
	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_DOMAIN))    { buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_DOMAIN); buf.SetLPSLowerCase(szDomain); }
	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_NICKNAME))  { buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_NICKNAME); buf.SetLPSW(wszNickName); }
	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_FIRSTNAME)) { buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_FIRSTNAME); buf.SetLPSW(wszFirstName); }
	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_LASTNAME))  { buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_LASTNAME); buf.SetLPSW(wszLastName); }

	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_SEX)) {
		tmp.Format("%lu", dwSex);
		buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_SEX);
		buf.SetLPS(tmp);
	}

	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_DATE1)) {
		tmp.Format("%lu", dwDate1);
		buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_DATE1);
		buf.SetLPS(tmp);
	}

	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_DATE2)) {
		tmp.Format("%lu", dwDate2);
		buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_DATE2);
		buf.SetLPS(tmp);
	}

	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_CITY_ID)) {
		tmp.Format("%lu", dwCityID);
		buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_CITY_ID);
		buf.SetLPS(tmp);
	}

	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_ZODIAC)) {
		tmp.Format("%lu", dwZodiak);
		buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_ZODIAC);
		buf.SetLPS(tmp);
	}

	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_MONTH)) {
		tmp.Format("%lu", dwBirthdayMonth);
		buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_MONTH);
		buf.SetLPS(tmp);
	}

	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_DAY)) {
		tmp.Format("%lu", dwBirthdayDay);
		buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_DAY);
		buf.SetLPS(tmp);
	}

	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_COUNTRY_ID)) {
		tmp.Format("%lu", dwCountryID);
		buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_COUNTRY_ID);
		buf.SetLPS(tmp);
	}

	if (GetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_ONLINE)) {
		tmp.Format("%lu", dwOnline);
		buf.SetUL(MRIM_CS_WP_REQUEST_PARAM_ONLINE);
		buf.SetLPS(tmp);
	}

	return (HANDLE)MraSendQueueCMD(hSendQueueHandle, dwRequestFlags, hContact, dwAckType, NULL, 0, MRIM_CS_WP_REQUEST, buf.Data(), buf.Len());
}

// Поиск контакта по EMail
HANDLE CMraProto::MraWPRequestByEMail(MCONTACT hContact, DWORD dwAckType, CMStringA &szEmail)
{
	if (szEmail.GetLength() <= 4)
		return 0;

	int iStart = 0;
	CMStringA szUser   = szEmail.Tokenize("@", iStart);
	CMStringA szDomain = szEmail.Tokenize("@", iStart);

	DWORD dwRequestFlags = 0;
	SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_USER);
	SetBit(dwRequestFlags, MRIM_CS_WP_REQUEST_PARAM_DOMAIN);
	return MraWPRequestW(hContact, dwAckType, dwRequestFlags, szUser, szDomain, _T(""), _T(""), _T(""), 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

// Отправка файлов
DWORD CMraProto::MraGame(const CMStringA &szEmail, DWORD dwGameSessionID, DWORD dwGameMsg, DWORD dwGameMsgID, const CMStringA &szData)
{
	if (szEmail.GetLength() <= 4)
		return 0;

	OutBuffer buf;
	buf.SetLPSLowerCase(szEmail);
	buf.SetUL(dwGameSessionID);
	buf.SetUL(dwGameMsg);
	buf.SetUL(dwGameMsgID);
	buf.SetUL(_time32(NULL));
	buf.SetLPS(szData);
	return MraSendCMD(MRIM_CS_GAME, buf.Data(), buf.Len());
}

// Авторизация
DWORD CMraProto::MraLogin2W(CMStringA &szLogin, CMStringA &szPassword, DWORD dwStatus, CMStringA &szStatusUri, CMStringW &wszStatusTitle, CMStringW &wszStatusDesc, DWORD dwFutureFlags, CMStringA &szUserAgentFormatted, CMStringA &szUserAgent)
{
	if (szStatusUri.GetLength() > SPEC_STATUS_URI_MAX)     szStatusUri.Truncate(SPEC_STATUS_URI_MAX);
	if (wszStatusTitle.GetLength() > STATUS_TITLE_MAX)     wszStatusTitle.Truncate(STATUS_TITLE_MAX);
	if (wszStatusDesc.GetLength() > STATUS_DESC_MAX)       wszStatusDesc.Truncate(STATUS_DESC_MAX);
	if (szUserAgentFormatted.GetLength() > USER_AGENT_MAX) szUserAgentFormatted.Truncate(USER_AGENT_MAX);
	if (szUserAgent.GetLength() > MAX_CLIENT_DESCRIPTION)  szUserAgent.Truncate(MAX_CLIENT_DESCRIPTION);

	OutBuffer buf;
	buf.SetLPS(szLogin);
	buf.SetLPS(szPassword);
	buf.SetUL(dwStatus);
	buf.SetLPS(szStatusUri);
	buf.SetLPSW(wszStatusTitle);
	buf.SetLPSW(wszStatusDesc);
	buf.SetUL(dwFutureFlags);
	buf.SetLPS(szUserAgentFormatted);
	buf.SetLPS("ru");
	buf.SetLPS("");
	buf.SetLPS("");
	buf.SetLPS(szUserAgent);// LPS client description /max 256

	return MraSendCMD(MRIM_CS_LOGIN2, buf.Data(), buf.Len());
}

// Отправка SMS
DWORD CMraProto::MraSMSW(MCONTACT hContact, const CMStringA &lpszPhone, const CMStringW &lpwszMessage)
{
	CMStringA szPhoneLocal = "+" + CopyNumber(lpszPhone);

	OutBuffer buf;
	buf.SetUL(0);
	buf.SetLPS(szPhoneLocal);
	buf.SetLPSW(lpwszMessage);

	/* Save phone number for ack notify after send. */
	LPBYTE lpbData = (LPBYTE)mir_calloc(lpszPhone.GetLength() + sizeof(size_t));
	if (NULL == lpbData)
		return (0);
	memcpy(lpbData, lpszPhone, lpszPhone.GetLength());
	return MraSendQueueCMD(hSendQueueHandle, 0, hContact, ICQACKTYPE_SMS, lpbData, lpszPhone.GetLength(), MRIM_CS_SMS, buf.Data(), buf.Len());
}

// Соединение с прокси
DWORD CMraProto::MraProxy(const CMStringA &szEmail, DWORD dwIDRequest, DWORD dwDataType, const CMStringA &lpszData, const CMStringA &szAddresses, MRA_GUID mguidSessionID)
{
	if (szEmail.GetLength() <= 4)
		return 0;

	OutBuffer buf;
	buf.SetLPSLowerCase(szEmail);
	buf.SetUL(dwIDRequest);
	buf.SetUL(dwDataType);
	buf.SetLPS(lpszData);
	buf.SetLPS(szAddresses);
	buf.SetGUID(mguidSessionID);
	return MraSendCMD(MRIM_CS_PROXY, buf.Data(), buf.Len());
}

// Ответ на соединение с прокси
DWORD CMraProto::MraProxyAck(DWORD dwStatus, const CMStringA &szEmail, DWORD dwIDRequest, DWORD dwDataType, const CMStringA &lpszData, const CMStringA &szAddresses, MRA_GUID mguidSessionID)
{
	if (szEmail.GetLength() <= 4)
		return 0;

	OutBuffer buf;
	buf.SetUL(dwStatus);
	buf.SetLPSLowerCase(szEmail);
	buf.SetUL(dwIDRequest);
	buf.SetUL(dwDataType);
	buf.SetLPS(lpszData);
	buf.SetLPS(szAddresses);
	buf.SetGUID(mguidSessionID);
	return MraSendCMD(MRIM_CS_PROXY_ACK, buf.Data(), buf.Len());
}

// Отправка сообщения в микроблог
DWORD CMraProto::MraChangeUserBlogStatus(DWORD dwFlags, const CMStringW &wszText, DWORDLONG dwBlogStatusID)
{
	OutBuffer buf;
	buf.SetUL(dwFlags);
	buf.SetLPSW(wszText);
	buf.SetUIDL(dwBlogStatusID);
	return MraSendCMD(MRIM_CS_CHANGE_USER_BLOG_STATUS, buf.Data(), buf.Len());
}

DWORD CMraProto::MraSendPacket(HANDLE hConnection, DWORD dwCMDNum, DWORD dwType, LPVOID lpData, size_t dwDataSize)
{
	LPBYTE lpbData = (LPBYTE)_alloca(dwDataSize + sizeof(mrim_packet_header_t));

	mrim_packet_header_t *pmaHeader = (mrim_packet_header_t*)lpbData;
	memset(pmaHeader, 0, sizeof(mrim_packet_header_t));
	pmaHeader->magic = CS_MAGIC;
	pmaHeader->proto = (PROTO_VERSION_MAJOR << 16) + PROTO_VERSION_MINOR; // Версия протокола
	pmaHeader->seq = dwCMDNum;// Sequence
	pmaHeader->msg = dwType;// Тип пакета
	pmaHeader->dlen = dwDataSize;// Длина данных
	
	debugLogA("Sending packet %08x\n", dwType);

	memcpy((lpbData + sizeof(mrim_packet_header_t)), lpData, dwDataSize);
	return Netlib_Send(hConnection, (LPSTR)lpbData, (dwDataSize + sizeof(mrim_packet_header_t)), 0);
}

DWORD CMraProto::MraSendCMD(DWORD dwType, LPVOID lpData, size_t dwDataSize)
{
	DWORD dwRet = InterlockedIncrement((LONG volatile*)&dwCMDNum);

	mir_cslock l(csCriticalSectionSend); // guarding winsock internal buffers
	return !MraSendPacket(m_hConnection, dwRet, dwType, lpData, dwDataSize) ? 0 : dwRet;
}


DWORD CMraProto::MraSendQueueCMD(HANDLE hSendQueueHandle, DWORD dwFlags, MCONTACT hContact, DWORD dwAckType, LPBYTE lpbDataQueue, size_t dwDataQueueSize, DWORD dwType, LPVOID lpData, size_t dwDataSize)
{
	DWORD dwRet = InterlockedIncrement((LONG volatile*)&dwCMDNum);
	if ( !MraSendQueueAdd(hSendQueueHandle, dwRet, dwFlags, hContact, dwAckType, lpbDataQueue, dwDataQueueSize)) {
		mir_cslock l(csCriticalSectionSend); // guarding winsock internal buffers
		if ( !MraSendPacket(m_hConnection, dwRet, dwType, lpData, dwDataSize)) {
			MraSendQueueFree(hSendQueueHandle, dwRet);
			dwRet = 0;
		}
	}
	return dwRet;
}
