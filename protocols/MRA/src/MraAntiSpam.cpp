// started: 19.12.2006

#include "Mra.h"
#include "MraAntiSpam.h"
#include "MraRTFMsg.h"

struct MRA_ANTISPAM_BAD_WORD
{
	LPWSTR lpwszBadWord;
	DWORD  dwBadWordLen;
};

static MRA_ANTISPAM_BAD_WORD *pmabwBadWords = NULL;
static size_t dwBadWordsCount = 0;

#define MRA_ANTISPAM_PREALLOC_COUNT	256

size_t MraAntiSpamLoadBadWordsW()
{
	DBVARIANT dbv = {0};
	size_t i = 0, dwValueSize, dwAllocatedItemsCount = MRA_ANTISPAM_PREALLOC_COUNT;

	if (pmabwBadWords || dwBadWordsCount)
		MraAntiSpamFreeBadWords();

	pmabwBadWords = (MRA_ANTISPAM_BAD_WORD*)mir_calloc((sizeof(MRA_ANTISPAM_BAD_WORD)*dwAllocatedItemsCount));

	for (i = 0;TRUE;i++) {
		char szSettingName[MAX_PATH];
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "AntiSpamBadWord %lu", i);
		if (db_get_ws(NULL, "MRA", szSettingName, &dbv))
			break;

		if (dwAllocatedItemsCount == i) {
			dwAllocatedItemsCount += MRA_ANTISPAM_PREALLOC_COUNT;
			pmabwBadWords = (MRA_ANTISPAM_BAD_WORD*)mir_realloc(pmabwBadWords, (sizeof(MRA_ANTISPAM_BAD_WORD)*dwAllocatedItemsCount));
			if (pmabwBadWords == NULL)
				break;
		}

		dwValueSize = lstrlenW(dbv.pwszVal);
		pmabwBadWords[i].lpwszBadWord = (LPWSTR)mir_calloc((dwValueSize*sizeof(WCHAR)));
		if (pmabwBadWords[i].lpwszBadWord) {
			memmove(pmabwBadWords[i].lpwszBadWord, dbv.pwszVal, (dwValueSize*sizeof(WCHAR)));
			CharLowerBuffW(pmabwBadWords[i].lpwszBadWord, DWORD(dwValueSize));
			pmabwBadWords[i].dwBadWordLen = DWORD(dwValueSize);
		}
		db_free(&dbv);

		if (pmabwBadWords[i].lpwszBadWord == NULL)
			break;
	}
	dwBadWordsCount = i;

	return 0;
}


void MraAntiSpamFreeBadWords()
{
	for (size_t i = 0; i < dwBadWordsCount; i++)
		mir_free(pmabwBadWords[i].lpwszBadWord);

	mir_free(pmabwBadWords);
	dwBadWordsCount = 0;
}

static WORD wMraAntiSpamControlsList[] = {
	IDC_CHK_TEMP_CONTACTS, IDC_CLN_NON_ALPHNUM, IDC_MAX_LANG_CHANGES, IDC_SHOWPOPUP, IDC_WRITETOSYSTEMHISTORY, IDC_SEND_SPAM_REPORT_TO_SERVER,
	IDC_BAD_WORDS_LIST, IDC_EDIT_BAD_WORD, IDC_BUTTON_ADD, IDC_BUTTONREMOVE, IDC_BUTTON_DEFAULT };

void MraAntiSpamEnableControls(HWND hWndDlg, BOOL bEnabled)
{
	EnableControlsArray(hWndDlg, (WORD*)&wMraAntiSpamControlsList, SIZEOF(wMraAntiSpamControlsList), bEnabled);
	EnableWindow(GetDlgItem(hWndDlg, IDC_DELETE_SPAMBOT_CONTACT), (bEnabled && IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_CHK_TEMP_CONTACTS)));
}

INT_PTR CALLBACK MraAntiSpamDlgProcOpts(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMraProto *ppro = (CMraProto*)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg);
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, lParam);
		ppro = (CMraProto*)lParam;

		CHECK_DLG_BUTTON(hWndDlg, IDC_ENABLE, ppro->getByte("AntiSpamEnable", MRA_ANTISPAM_DEFAULT_ENABLE));
		CHECK_DLG_BUTTON(hWndDlg, IDC_CHK_TEMP_CONTACTS, ppro->getByte("AntiSpamCheckTempContacts", MRA_ANTISPAM_DEFAULT_CHK_TEMP_CONTACTS));
		CHECK_DLG_BUTTON(hWndDlg, IDC_DELETE_SPAMBOT_CONTACT, ppro->getByte("AntiSpamDeteleSpamBotContacts", MRA_ANTISPAM_DEFAULT_DELETE_SPAMBOT_CONTACT));
		CHECK_DLG_BUTTON(hWndDlg, IDC_CLN_NON_ALPHNUM, ppro->getByte("AntiSpamCleanNonAlphaNumeric", MRA_ANTISPAM_DEFAULT_CLN_NON_ALPHNUM));
		SetDlgItemInt(hWndDlg, IDC_MAX_LANG_CHANGES, ppro->getDword("AntiSpamMaxLangChanges", MRA_ANTISPAM_DEFAULT_MAX_LNG_CHANGES), FALSE);
		CHECK_DLG_BUTTON(hWndDlg, IDC_SHOWPOPUP, ppro->getByte("AntiSpamShowPopUp", MRA_ANTISPAM_DEFAULT_SHOWPOP));
		CHECK_DLG_BUTTON(hWndDlg, IDC_WRITETOSYSTEMHISTORY, ppro->getByte("AntiSpamWriteToSystemHistory", MRA_ANTISPAM_DEFAULT_WRITETOSYSTEMHISTORY));
		CHECK_DLG_BUTTON(hWndDlg, IDC_SEND_SPAM_REPORT_TO_SERVER, ppro->getByte("AntiSpamSendSpamReportToSrv", MRA_ANTISPAM_DEFAULT_SEND_SPAM_REPORT_TO_SERVER));
		{
			// fill list
			char szSettingName[MAX_PATH];
			WCHAR wszBuff[MAX_PATH];

			SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_RESETCONTENT, 0, 0);
			SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_INITSTORAGE, (WPARAM)1024, (LPARAM)32768);
			for (size_t i = 0; TRUE; i++) {
				mir_snprintf(szSettingName, SIZEOF(szSettingName), "AntiSpamBadWord %lu", i);
				if (ppro->mraGetStaticStringW(NULL, szSettingName, wszBuff, SIZEOF(wszBuff), NULL)) {
					if (SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_FINDSTRING, -1, (LPARAM)wszBuff) == LB_ERR)
						SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_ADDSTRING, 0, (LPARAM)wszBuff);
				}
				else break;
			}
		}

		MraAntiSpamEnableControls(hWndDlg, IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_ENABLE));
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_ENABLE)
			MraAntiSpamEnableControls(hWndDlg, IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_ENABLE));

		if (LOWORD(wParam) == IDC_CHK_TEMP_CONTACTS)
			EnableWindow(GetDlgItem(hWndDlg, IDC_DELETE_SPAMBOT_CONTACT), IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_CHK_TEMP_CONTACTS));

		if (LOWORD(wParam) == IDC_BUTTON_ADD) {
			WCHAR szBadWord[MAX_PATH];

			if (GET_DLG_ITEM_TEXT(hWndDlg, IDC_EDIT_BAD_WORD, szBadWord, SIZEOF(szBadWord))) {
				SET_DLG_ITEM_TEXTW(hWndDlg, IDC_EDIT_BAD_WORD, L"");
				if (SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_FINDSTRING, -1, (LPARAM)szBadWord) == LB_ERR) SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_ADDSTRING, 0, (LPARAM)szBadWord);
			}
		}

		if (LOWORD(wParam) == IDC_BUTTONREMOVE)
			SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_DELETESTRING, SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_GETCURSEL, 0, 0), 0);

		if (LOWORD(wParam) == IDC_BUTTON_DEFAULT) {
			MraAntiSpamResetBadWordsList();

			char szSettingName[MAX_PATH];
			WCHAR wszBuff[MAX_PATH];

			SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_RESETCONTENT, 0, 0);
			SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_INITSTORAGE, (WPARAM)1024, (LPARAM)32768);
			for (size_t i = 0;TRUE;i++) {
				mir_snprintf(szSettingName, SIZEOF(szSettingName), "AntiSpamBadWord %lu", i);
				if (ppro->mraGetStaticStringW(NULL, szSettingName, wszBuff, SIZEOF(wszBuff), NULL)) {
					if (SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_FINDSTRING, -1, (LPARAM)wszBuff) == LB_ERR)
						SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_ADDSTRING, 0, (LPARAM)wszBuff);
				}
				else break;
			}
			MraAntiSpamLoadBadWordsW();
		}

		if ((LOWORD(wParam) == IDC_EDIT_BAD_WORD) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;
		SendMessage(GetParent(hWndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			{
				char szSettingName[MAX_PATH];
				WCHAR szBadWord[MAX_PATH];
				size_t i, dwCount;

				ppro->setByte("AntiSpamEnable", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_ENABLE));
				ppro->setByte("AntiSpamCheckTempContacts", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_CHK_TEMP_CONTACTS));
				ppro->setByte("AntiSpamDeteleSpamBotContacts", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_DELETE_SPAMBOT_CONTACT));
				ppro->setByte("AntiSpamCleanNonAlphaNumeric", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_CLN_NON_ALPHNUM));
				ppro->setDword("AntiSpamMaxLangChanges", (DWORD)GetDlgItemInt(hWndDlg, IDC_MAX_LANG_CHANGES, NULL, FALSE));
				ppro->setByte("AntiSpamShowPopUp", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_SHOWPOPUP));
				ppro->setByte("AntiSpamWriteToSystemHistory", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_WRITETOSYSTEMHISTORY));
				ppro->setByte("AntiSpamSendSpamReportToSrv", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_SEND_SPAM_REPORT_TO_SERVER));

				dwCount = SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_GETCOUNT, 0, 0);
				for (i = 0; i < dwCount; i++) {
					mir_snprintf(szSettingName, SIZEOF(szSettingName), "AntiSpamBadWord %lu", i);
					SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_BAD_WORDS_LIST, LB_GETTEXT, i, (LPARAM)szBadWord);
					ppro->mraSetStringW(NULL, szSettingName, szBadWord);
				}

				dwCount = ((i*2)+1024);
				for (; i < dwCount; i++) {
					mir_snprintf(szSettingName, SIZEOF(szSettingName), "AntiSpamBadWord %lu", i);
					ppro->delSetting(szSettingName);
				}

				MraAntiSpamLoadBadWordsW();
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////

static LPWSTR lpwszBadWords[] =
{
	// web words
	L"@", L"http://", L"ftp://", L"www", L"wap.", L".com", L".org", L".info", L".net", L".ua", L".uz", L".to", L".biz", L".vu", L".ru", L"mailto:", L"ICQ", L"SKYPE",

	// spam words
	L"sex", L"seks", L"sekc", L"xxx", L"phorno", L"video", L"photo", L"SMS", L"WMZ", L"WMR", L"narod", L"ucoz", L"ref", L"Agent", L"+7", L"495", L"$", L"spam", L"URL", L"%2E",

	// spam words rus
	L"прода", L"цена", L"деньги", L"денег", L"секс", L"лесби", L"лезби", L"анал", L"порн", L"эротич", L"видео", L"фильм", L"кино", L"муз", L"фотки", L"фото", L"СМС", L"аськ", L"база", L"сайт", L"проект", L"рассылк", L"обьявлен", L"реклам", L"услуг", L"оплат", L"заказ", L"пиши", L"звони", L"работ", L"зараб", L"зайди", L"загляни", L"посети", L"посмотр", L"погляд", L"точка", L"тчк", L"спам", L"ссылк", L"майл", L"агент", L"супер", L"лох", L"регистрац", L"троян", L"вирус", L"взлом",

	// executable
	L".ade", L".adp", L".bas", L".bat", L".chm", L".cmd", L".com", L".cpl", L".crt", L".exe", L".hlp", L".hta", L".inf", L".ins", L".isp", L".js", L".jse", L".lnk", L".mdb", L".mde", L".msc", L".msi", L".msp", L".mst", L".pcd", L".pif", L".reg", L".scr", L".sct", L".shs", L".shb", L".url", L".vb", L".vbe", L".vbs", L".wsc", L".wsf", L".wsh",

	// archives
	L".zip", L".rar", L".cab", L".arj", L".tar", L".gz", L".zoo", L".lzh", L".jar", L".ace", L".z", L".iso", L".bz2", L".uue", L".gzip",

	NULL
};

void MraAntiSpamResetBadWordsList()
{
	char szSettingName[MAX_PATH];
	size_t i, dwMax;

	for (i = 0; lpwszBadWords[i]; i++) {
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "AntiSpamBadWord %lu", i);
		db_set_ws(NULL, "MRA", szSettingName, lpwszBadWords[i]);
	}

	dwMax = (i+1024);
	for (; i < dwMax; i++) {
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "AntiSpamBadWord %lu", i);
		db_unset(NULL, "MRA", szSettingName);
	}
}

///////////////////////////////////////////////////////////////////////////////

static WCHAR lpwszExceptedChars[] = L"\r\n .,<>//?;:'\"\\][}{`-=~!@#$%^&*()_+№1234567890";

BOOL MraAntiSpamIsCharExcepted(WCHAR wcTestChar)
{
	for (size_t i = 0; i < SIZEOF(lpwszExceptedChars); i++)
		if (lpwszExceptedChars[i] == wcTestChar)
			return TRUE;

	return FALSE;
}

size_t MraAntiSpamCalcLangChanges(LPWSTR lpwszMessage, size_t dwMessageSize)
{
	size_t dwRet = 0;

	if (lpwszMessage && dwMessageSize) {
		WCHAR wcPrevChar = 0;
		for (size_t i = 1; i < dwMessageSize; i++) {
			if (MraAntiSpamIsCharExcepted(lpwszMessage[i]) == FALSE) {
				if (wcPrevChar)
					if ( (max(wcPrevChar, lpwszMessage[i])-min(wcPrevChar, lpwszMessage[i])) > 255)
						dwRet++;

				wcPrevChar = lpwszMessage[i];
			}
		}
	}

	return dwRet;
}

size_t MraAntiSpamCleanNonAlphaNumeric(LPWSTR lpwszMessage, size_t dwMessageSize)
{
	size_t dwRet = 0;

	if (lpwszMessage && dwMessageSize) {
		LPWSTR lpwszCurWritePos = lpwszMessage;
		for (size_t i = 0;i<dwMessageSize;i++)
			if (IsCharAlphaNumeric(lpwszMessage[i]))
				*lpwszCurWritePos++ = lpwszMessage[i];

		*lpwszCurWritePos = 0;
		dwRet = lpwszCurWritePos - lpwszMessage;
	}
	return dwRet;
}


BOOL MraAntiSpamTestMessageForBadWordsW(LPWSTR lpwszMessage, size_t dwMessageSize)
{
	if (lpwszMessage && dwMessageSize)
		for (size_t i = 0; i < dwBadWordsCount; i++)
			if (MemoryFind(0, lpwszMessage, (dwMessageSize*sizeof(WCHAR)), pmabwBadWords[i].lpwszBadWord, (pmabwBadWords[i].dwBadWordLen*sizeof(WCHAR))))
				return TRUE;

	return FALSE;
}

BOOL CMraProto::MraAntiSpamHasMessageBadWordsW(LPWSTR lpwszMessage, size_t dwMessageSize)
{
	BOOL bRet = FALSE;

	if (lpwszMessage && dwMessageSize) {
		LPWSTR lpwszMessageConverted = (LPWSTR)mir_alloc((dwMessageSize+1)*sizeof(WCHAR));
		if (lpwszMessageConverted) {
			size_t dwtm;

			// в нижний регистр всё сообщение
			memmove(lpwszMessageConverted, lpwszMessage, (dwMessageSize*sizeof(WCHAR)));
			CharLowerBuffW(lpwszMessageConverted, DWORD(dwMessageSize));

			// 1 проход: считаем колличество переключений языка
			dwtm = getDword("AntiSpamMaxLangChanges", MRA_ANTISPAM_DEFAULT_MAX_LNG_CHANGES);
			if (dwtm)
				if (dwtm <= MraAntiSpamCalcLangChanges(lpwszMessageConverted, dwMessageSize))
					bRet = TRUE;

			// 2 проход: ищем плохие слова
			if (bRet == FALSE)
				bRet = MraAntiSpamTestMessageForBadWordsW(lpwszMessageConverted, dwMessageSize);

			// 3 проход: оставляем только буквы + цифры и снова ищем плохие слова
			if (bRet == FALSE)
			if (getByte("AntiSpamCleanNonAlphaNumeric", MRA_ANTISPAM_DEFAULT_ENABLE)) {
				dwMessageSize = MraAntiSpamCleanNonAlphaNumeric(lpwszMessageConverted, dwMessageSize);
				bRet = MraAntiSpamTestMessageForBadWordsW(lpwszMessageConverted, dwMessageSize);
			}

			mir_free(lpwszMessageConverted);
		}
	}
	return bRet;
}

DWORD CMraProto::MraAntiSpamReceivedMessageW(LPSTR lpszEMail, size_t dwEMailSize, DWORD dwMessageFlags, LPWSTR lpwszMessage, size_t dwMessageSize)
{
	DWORD dwRet = MESSAGE_NOT_SPAM;;

	if ((dwMessageFlags&(MESSAGE_FLAG_SYSTEM|MESSAGE_FLAG_CONTACT|MESSAGE_FLAG_NOTIFY|MESSAGE_FLAG_SMS|MESSAGE_SMS_DELIVERY_REPORT|MESSAGE_FLAG_ALARM|MESSAGE_FLAG_MULTICHAT)) == 0)
	if (getByte("AntiSpamEnable", MRA_ANTISPAM_DEFAULT_ENABLE))
	if (IsEMailChatAgent(lpszEMail, dwEMailSize) == FALSE) { // enabled, message must be checked
		BOOL bCheckMessage = TRUE;
		dwRet = MESSAGE_SPAM;
		HANDLE hContact = MraHContactFromEmail(lpszEMail, dwEMailSize, FALSE, TRUE, NULL);
		if (hContact) {
			DWORD dwID, dwGroupID, dwContactFlag, dwContactSeverFlags;
			GetContactBasicInfoW(hContact, &dwID, &dwGroupID, &dwContactFlag, &dwContactSeverFlags, NULL, NULL, 0, NULL, NULL, 0, NULL, NULL, 0, NULL);

			// not temp contact, OK
			if ((dwID != -1 && db_get_b(hContact, "CList", "NotOnList", 0) == 0) || dwGroupID == -2) {
				bCheckMessage = FALSE;
				dwRet = MESSAGE_NOT_SPAM;
			}
			else { // temp contact
				if (getByte("AntiSpamCheckTempContacts", MRA_ANTISPAM_DEFAULT_CHK_TEMP_CONTACTS) == FALSE ||
					(((dwMessageFlags&MESSAGE_FLAG_AUTHORIZE) && getByte("AutoAuthGrandNewUsers", MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS) && getByte("AutoAuthGrandNewUsersDisableSPAMCheck", MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK))))
				{// проверка временного контакта
					bCheckMessage = FALSE;
					dwRet = MESSAGE_NOT_SPAM;
				}
			}
		}

		if (bCheckMessage)
			if (MraAntiSpamHasMessageBadWordsW(lpwszMessage, dwMessageSize) == FALSE)
				 dwRet = MESSAGE_NOT_SPAM;

		if (dwRet == MESSAGE_SPAM) {
			BOOL bAntiSpamShowPopup, bAntiSpamWriteToSystemHistory, bAntiSpamDeteleSpamBotContacts;

			bAntiSpamShowPopup = (BOOL)getByte("AntiSpamShowPopUp", MRA_ANTISPAM_DEFAULT_SHOWPOP);
			bAntiSpamWriteToSystemHistory = (BOOL)getByte("AntiSpamWriteToSystemHistory", MRA_ANTISPAM_DEFAULT_WRITETOSYSTEMHISTORY);
			bAntiSpamDeteleSpamBotContacts = (BOOL)getByte("AntiSpamDeteleSpamBotContacts", MRA_ANTISPAM_DEFAULT_DELETE_SPAMBOT_CONTACT);

			if (bAntiSpamShowPopup || bAntiSpamWriteToSystemHistory) {
				char szEMail[MAX_EMAIL_LEN];
				WCHAR wszBuff[MRA_MAXLENOFMESSAGE*2];
				size_t dwDBMessageSize;

				memmove(szEMail, lpszEMail, dwEMailSize);
				(*((WORD*)(szEMail+dwEMailSize))) = 0;

				dwDBMessageSize = mir_sntprintf(wszBuff, SIZEOF(wszBuff), L"%s: %S\r\n", TranslateW(L"Spam bot blocked"), szEMail);
				if (dwMessageSize>(SIZEOF(wszBuff)-(dwDBMessageSize+sizeof(DWORD)))) dwMessageSize = (SIZEOF(wszBuff)-(dwDBMessageSize+sizeof(DWORD)));
				memmove((wszBuff+dwDBMessageSize), lpwszMessage, (dwMessageSize*sizeof(WCHAR)));
				dwDBMessageSize += dwMessageSize;
				(*((DWORD*)(wszBuff+dwDBMessageSize))) = 0;
				dwDBMessageSize += sizeof(DWORD);

				if (bAntiSpamShowPopup)
					MraPopupShowFromAgentW(MRA_POPUP_TYPE_INFORMATION, 0, wszBuff);

				if (bAntiSpamWriteToSystemHistory) {
					CHAR szBuff[MRA_MAXLENOFMESSAGE*2];
					WideCharToMultiByte(CP_UTF8, 0, wszBuff, DWORD(dwDBMessageSize), szBuff, SIZEOF(szBuff), NULL, NULL);

					DBEVENTINFO dbei = { sizeof(dbei) };
					dbei.szModule = m_szModuleName;
					dbei.timestamp = _time32(NULL);
					dbei.flags = (DBEF_READ|DBEF_UTF);
					dbei.eventType = EVENTTYPE_MESSAGE;
					dbei.cbBlob = DWORD(dwDBMessageSize*sizeof(WCHAR));
					dbei.pBlob = (PBYTE)szBuff;
					db_event_add(NULL, &dbei);
				}

				if (hContact && bAntiSpamDeteleSpamBotContacts) {
					dwDBMessageSize = mir_sntprintf(wszBuff, SIZEOF(wszBuff), L"%s: %S", TranslateW(L"Spam bot contact deleted"), szEMail);

					if (bAntiSpamShowPopup)
						MraPopupShowFromAgentW(MRA_POPUP_TYPE_INFORMATION, 0, wszBuff);

					if (bAntiSpamWriteToSystemHistory) {
						(*((DWORD*)(wszBuff+dwDBMessageSize))) = 0;
						dwDBMessageSize += sizeof(DWORD);

						CHAR szBuff[MRA_MAXLENOFMESSAGE*2];
						WideCharToMultiByte(CP_UTF8, 0, wszBuff, DWORD(dwDBMessageSize), szBuff, SIZEOF(szBuff), NULL, NULL);

						DBEVENTINFO dbei = { sizeof(dbei) };
						dbei.szModule = m_szModuleName;
						dbei.timestamp = _time32(NULL);
						dbei.flags = (DBEF_READ|DBEF_UTF);
						dbei.eventType = EVENTTYPE_MESSAGE;
						dbei.cbBlob = DWORD(dwDBMessageSize*sizeof(WCHAR));
						dbei.pBlob = (PBYTE)szBuff;
						db_event_add(NULL, &dbei);
					}
				}
			}

			if (hContact && bAntiSpamDeteleSpamBotContacts)
				CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
		}
	}
	return dwRet;
}
