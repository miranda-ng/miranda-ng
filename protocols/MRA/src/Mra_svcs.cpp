#include "Mra.h"
#include "proto.h"

const LPSTR lpcszStatusUri[] =
{
	"", // offline // "status_0",
	"STATUS_ONLINE", // "status_1",
	"STATUS_AWAY", 	// "status_2",
	"STATUS_INVISIBLE", // "status_3",
	"status_dnd",
	"status_chat",
	"status_4",
	"status_5",
	"status_6",
	"status_7",
	"status_8",
	"status_9",
	"status_10",
	"status_11",
	"status_12",
	"status_13",
	"status_14",
	"status_15",
	"status_16",
	"status_17",
	"status_18",
	"status_19",
	"status_20",
	"status_21",
	"status_22",
	"status_23",
	"status_24",
	//"status_25", // chat/dnd
	"status_26",
	"status_27",
	"status_28",
	"status_29",
	"status_30",
	//"status_31", // chat/dnd
	"status_32",
	"status_33",
	"status_34",
	"status_35",
	"status_36",
	"status_37",
	"status_38",
	"status_39",
	"status_40",
	"status_41",
	"status_42",
	"status_43",
	"status_44",
	"status_45",
	"status_46",
	"status_47",
	"status_48",
	"status_49",
	"status_50",
	"status_51",
	"status_52",
	"status_53",
	"status_dating",
	//"status_127", 145, 154
	NULL
};

const LPWSTR lpcszXStatusNameDef[] =
{
	LPGENT("None"),
	LPGENT("Sick"),
	LPGENT("Home"),
	LPGENT("Eating"),
	LPGENT("Compass"),
	LPGENT("On WC"),
	LPGENT("Cooking"),
	LPGENT("Walking"),
	LPGENT("Alien"),
	LPGENT("Shrimp"),
	LPGENT("Got lost"),
	LPGENT("Crazy"),
	LPGENT("Duck"),
	LPGENT("Playing"),
	LPGENT("Smoking"),
	LPGENT("Office"),
	LPGENT("Meeting"),
	LPGENT("Beer"),
	LPGENT("Coffee"),
	LPGENT("Working"),
	LPGENT("Relaxing"),
	LPGENT("On the phone"),
	LPGENT("In institute"),
	LPGENT("At school"),
	LPGENT("Wrong number"),
	LPGENT("Laughing"),
	LPGENT("Malicious"),
	LPGENT("Imp"),
	LPGENT("Blind"),
	LPGENT("Disappointed"),
	LPGENT("Almost crying"),
	LPGENT("Fearful"),
	LPGENT("Angry"),
	LPGENT("Vampire"),
	LPGENT("Ass"),
	LPGENT("Love"),
	LPGENT("Sleeping"),
	LPGENT("Cool!"),
	LPGENT("Peace!"),
	LPGENT("Cock a snook"),
	LPGENT("Get out"),
	LPGENT("Death"),
	LPGENT("Rocket"),
	LPGENT("Devil-fish"),
	LPGENT("Heavy metal"),
	LPGENT("Things look bad"),
	LPGENT("Squirrel"),
	LPGENT("Star"),
	LPGENT("Music"),
	LPGENT("Dating"),
	NULL
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CMraProto::SetExtraIcons(HANDLE hContact)
{
	DWORD dwID, dwGroupID, dwContactSeverFlags;
	if ( GetContactBasicInfoW(hContact, &dwID, &dwGroupID, NULL, &dwContactSeverFlags, NULL, NULL, 0, NULL, NULL, 0, NULL, NULL, 0, NULL))
		return;

	DWORD dwIconID = -1;
	DWORD dwXStatus = MRA_MIR_XSTATUS_NONE;

	if (m_bLoggedIn) {
		dwXStatus = mraGetByte(hContact, DBSETTING_XSTATUSID, MRA_MIR_XSTATUS_NONE);
		if (dwID == -1)
			dwIconID = (dwContactSeverFlags == -1) ? ADV_ICON_DELETED : ADV_ICON_NOT_ON_SERVER;
		else {
			if (dwGroupID == 103)
				dwIconID = ADV_ICON_PHONE;
			else {
				if (dwContactSeverFlags)
				if (dwContactSeverFlags == -1)
					dwIconID = ADV_ICON_DELETED;
				else
					dwIconID = ADV_ICON_NOT_AUTHORIZED;
			}
		}
	}

	if (dwIconID == -1) {
		size_t dwBlogStatusMsgSize = 0;

		mraGetStaticStringW(hContact, DBSETTING_BLOGSTATUS, NULL, 0, &dwBlogStatusMsgSize);
		if (dwBlogStatusMsgSize) dwIconID = ADV_ICON_BLOGSTATUS;
	}

	ExtraIcon_SetIcon(hExtraXstatusIcon, hContact,
		(IsXStatusValid(dwXStatus) || dwXStatus == MRA_MIR_XSTATUS_UNKNOWN) ? hXStatusAdvancedStatusIcons[dwXStatus] : NULL);
	ExtraIcon_SetIcon(hExtraInfo, hContact, (dwIconID != -1) ? gdiExtraStatusIconsItems[dwIconID].hIconHandle : NULL);
}

INT_PTR CMraProto::MraXStatusMenu(WPARAM wParam, LPARAM lParam, LPARAM param)
{
	if ( MraRequestXStatusDetails(param) == FALSE)
		MraSetXStatusInternal(param);
	return 0;
}

INT_PTR CMraProto::MraGotoInbox(WPARAM wParam, LPARAM lParam)
{
	MraMPopSessionQueueAddUrl(hMPopSessionQueue, MRA_WIN_INBOX_URL, sizeof(MRA_WIN_INBOX_URL));
	return 0;
}

INT_PTR CMraProto::MraShowInboxStatus(WPARAM wParam, LPARAM lParam)
{
	MraUpdateEmailStatus(NULL, 0, NULL, 0, 0, 0);
	return 0;
}

INT_PTR CMraProto::MraSendSMS(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn || !wParam || !lParam)
		return 0;

	ptrW lpwszMessageXMLEncoded( mir_utf8decodeW((LPSTR)lParam));
	size_t dwBuffLen = lstrlenA((LPSTR)lParam) + MAX_PATH;
	LPWSTR lpwszMessageXMLDecoded = (LPWSTR)mir_calloc((dwBuffLen*sizeof(WCHAR)));
	if (lpwszMessageXMLEncoded && lpwszMessageXMLDecoded) {
		size_t dwMessageXMLDecodedSize;
		DecodeXML(lpwszMessageXMLEncoded, lstrlen(lpwszMessageXMLEncoded), lpwszMessageXMLDecoded, dwBuffLen, &dwMessageXMLDecodedSize);
		if (dwMessageXMLDecodedSize)
			MraSMSW(NULL, (LPSTR)wParam , lstrlenA((LPSTR)wParam), lpwszMessageXMLDecoded, dwMessageXMLDecodedSize);
	}
	mir_free(lpwszMessageXMLDecoded);
	return 0;
}

INT_PTR CMraProto::MraEditProfile(WPARAM wParam, LPARAM lParam)
{
	MraMPopSessionQueueAddUrl(hMPopSessionQueue, MRA_EDIT_PROFILE_URL, sizeof(MRA_EDIT_PROFILE_URL));
	return 0;
}

INT_PTR CMraProto::MraZhuki(WPARAM wParam, LPARAM lParam)
{
	MraMPopSessionQueueAddUrl(hMPopSessionQueue, MRA_ZHUKI_URL, sizeof(MRA_ZHUKI_URL));
	return 0;
}

INT_PTR CMraProto::MraWebSearch(WPARAM wParam, LPARAM lParam)
{
	CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)MRA_SEARCH_URL);
	return 0;
}

INT_PTR CMraProto::MraUpdateAllUsersInfo(WPARAM wParam, LPARAM lParam)
{
	if ( MessageBox(NULL, TranslateT("Are you sure?"), TranslateW(MRA_UPD_ALL_USERS_INFO_STR), MB_YESNO | MB_ICONQUESTION) == IDYES ) {
		for (HANDLE hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			size_t dwEMailSize;
			CHAR szEMail[MAX_EMAIL_LEN];
			if ( mraGetStaticStringA(hContact, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize))
				MraWPRequestByEMail(hContact, ACKTYPE_GETINFO, szEMail, dwEMailSize);
		}
	}
	return 0;
}

INT_PTR CMraProto::MraCheckUpdatesUsersAvt(WPARAM wParam, LPARAM lParam)
{
	if ( MessageBox(NULL, TranslateT("Are you sure?"), TranslateW(MRA_CHK_USERS_AVATARS_STR), MB_YESNO | MB_ICONQUESTION) == IDYES) {
		for (HANDLE hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			size_t dwEMailSize;
			CHAR szEMail[MAX_EMAIL_LEN];

			if (mraGetStaticStringA(hContact, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize))
			if (IsEMailChatAgent(szEMail, dwEMailSize) == FALSE)// только для оптимизации, MraAvatarsQueueGetAvatarSimple сама умеет фильтровать чатконтакты
				MraAvatarsQueueGetAvatarSimple(hAvatarsQueueHandle, 0/*GAIF_FORCE*/, hContact, 0);
		}
	}
	return 0;
}

INT_PTR CMraProto::MraRequestAuthForAll(WPARAM wParam, LPARAM lParam)
{
	if ( MessageBox(NULL, TranslateT("Are you sure?"), TranslateW(MRA_REQ_AUTH_FOR_ALL_STR), MB_YESNO | MB_ICONQUESTION) == IDYES) {
		for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			DWORD dwContactSeverFlags;
			if (GetContactBasicInfoW(hContact, NULL, NULL, NULL, &dwContactSeverFlags, NULL, NULL, 0, NULL, NULL, 0, NULL, NULL, 0, NULL) == NO_ERROR)
			if (dwContactSeverFlags & CONTACT_INTFLAG_NOT_AUTHORIZED && dwContactSeverFlags != -1)
				MraRequestAuthorization((WPARAM)hContact, 0);
		}
	}
	return 0;
}

INT_PTR CMraProto::MraRequestAuthorization(WPARAM wParam, LPARAM lParam)
{
	if (wParam) {
		WCHAR wszAuthMessage[MAX_PATH];

		if (mraGetStaticStringW(NULL, "AuthMessage", wszAuthMessage, SIZEOF(wszAuthMessage), NULL) == FALSE)
			lstrcpynW(wszAuthMessage, TranslateW(MRA_DEFAULT_AUTH_MESSAGE), SIZEOF(wszAuthMessage));

		CHAR szEMail[MAX_EMAIL_LEN];
		size_t dwEMailSize, dwMessageSize;

		dwMessageSize = lstrlen(wszAuthMessage);
		if (dwMessageSize) {
			HANDLE hContact = (HANDLE)wParam;
			if ( mraGetStaticStringA(hContact, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize)) {
				BOOL bSlowSend = mraGetByte(NULL, "SlowSend", MRA_DEFAULT_SLOW_SEND);
				int iRet = MraMessageW(bSlowSend, hContact, ACKTYPE_AUTHREQ, MESSAGE_FLAG_AUTHORIZE, szEMail, dwEMailSize, wszAuthMessage, dwMessageSize, NULL, 0);
				if (bSlowSend == FALSE)
					ProtoBroadcastAck(hContact, ACKTYPE_AUTHREQ, ACKRESULT_SUCCESS, (HANDLE)iRet, 0);

				return 0;
			}
		}

		return 1;
	}
	return 0;
}

INT_PTR CMraProto::MraGrantAuthorization(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn || !wParam)
		return 0;

	CHAR szEMail[MAX_EMAIL_LEN];
	size_t dwEMailSize;

	// send without reason, do we need any ?
	if (mraGetStaticStringA((HANDLE)wParam, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize))
		MraAuthorize(szEMail, dwEMailSize);

	return 0;
}

INT_PTR CMraProto::MraSendPostcard(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn)
		return 0;

	DWORD dwContactEMailCount = GetContactEMailCount((HANDLE)wParam, FALSE);
	if (dwContactEMailCount) {
		if (dwContactEMailCount == 1) {
			size_t dwUrlSize, dwEMailSize;
			CHAR szUrl[BUFF_SIZE_URL], szEMail[MAX_EMAIL_LEN];

			if ( GetContactFirstEMail((HANDLE)wParam, FALSE, szEMail, SIZEOF(szEMail), &dwEMailSize)) {
				BuffToLowerCase(szEMail, szEMail, dwEMailSize);
				dwUrlSize = mir_snprintf(szUrl, SIZEOF(szUrl), "http://cards.mail.ru/event.html?rcptname=%s&rcptemail=%s", GetContactNameA((HANDLE)wParam), szEMail);
				MraMPopSessionQueueAddUrl(hMPopSessionQueue, szUrl, dwUrlSize);
			}
		}
		else MraSelectEMailDlgShow((HANDLE)wParam, MRA_SELECT_EMAIL_TYPE_SEND_POSTCARD);
	}
	return 0;
}

INT_PTR CMraProto::MraViewAlbum(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn)
		return 0;

	DWORD dwContactEMailMRCount = GetContactEMailCount((HANDLE)wParam, TRUE);
	if (dwContactEMailMRCount) {
		if (dwContactEMailMRCount == 1) {
			size_t dwEMailSize;
			CHAR szEMail[MAX_EMAIL_LEN];
			if (GetContactFirstEMail((HANDLE)wParam, TRUE, szEMail, SIZEOF(szEMail), &dwEMailSize))
				MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_FOTO_URL, sizeof(MRA_FOTO_URL), szEMail, dwEMailSize);
		}
		else MraSelectEMailDlgShow((HANDLE)wParam, MRA_SELECT_EMAIL_TYPE_VIEW_ALBUM);
	}
	return 0;
}

INT_PTR CMraProto::MraReadBlog(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn)
		return 0;

	DWORD dwContactEMailMRCount = GetContactEMailCount((HANDLE)wParam, TRUE);
	if (dwContactEMailMRCount)
	if (dwContactEMailMRCount == 1) {
		CHAR szEMail[MAX_EMAIL_LEN];
		size_t dwEMailSize;
		if (GetContactFirstEMail((HANDLE)wParam, TRUE, szEMail, SIZEOF(szEMail), &dwEMailSize))
			MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_BLOGS_URL, sizeof(MRA_BLOGS_URL), szEMail, dwEMailSize);
	}
	else MraSelectEMailDlgShow((HANDLE)wParam, MRA_SELECT_EMAIL_TYPE_READ_BLOG);

	return 0;
}

INT_PTR CMraProto::MraReplyBlogStatus(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn)
		return 0;

	size_t dwBlogStatusMsgSize = 0;
	mraGetStaticStringW((HANDLE)wParam, DBSETTING_BLOGSTATUS, NULL, 0, &dwBlogStatusMsgSize);
	if (dwBlogStatusMsgSize || wParam == 0)
		MraSendReplyBlogStatus((HANDLE)wParam);

	return 0;
}

INT_PTR CMraProto::MraViewVideo(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn)
		return 0;

	DWORD dwContactEMailMRCount = GetContactEMailCount((HANDLE)wParam, TRUE);
	if (dwContactEMailMRCount) {
		if (dwContactEMailMRCount == 1) {
			CHAR szEMail[MAX_EMAIL_LEN];
			size_t dwEMailSize;
			if (GetContactFirstEMail((HANDLE)wParam, TRUE, szEMail, SIZEOF(szEMail), &dwEMailSize))
				MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_VIDEO_URL, sizeof(MRA_VIDEO_URL), szEMail, dwEMailSize);
		}
		else MraSelectEMailDlgShow((HANDLE)wParam, MRA_SELECT_EMAIL_TYPE_VIEW_VIDEO);
	}
	return 0;
}

INT_PTR CMraProto::MraAnswers(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn)
		return 0;

	DWORD dwContactEMailMRCount = GetContactEMailCount((HANDLE)wParam, TRUE);
	if (dwContactEMailMRCount) {
		if (dwContactEMailMRCount == 1) {
			CHAR szEMail[MAX_EMAIL_LEN];
			size_t dwEMailSize;
			if (GetContactFirstEMail((HANDLE)wParam, TRUE, szEMail, SIZEOF(szEMail), &dwEMailSize))
				MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_ANSWERS_URL, sizeof(MRA_ANSWERS_URL), szEMail, dwEMailSize);
		}
		else MraSelectEMailDlgShow((HANDLE)wParam, MRA_SELECT_EMAIL_TYPE_ANSWERS);
	}
	return 0;
}

INT_PTR CMraProto::MraWorld(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn)
		return 0;

	DWORD dwContactEMailMRCount = GetContactEMailCount((HANDLE)wParam, TRUE);
	if (dwContactEMailMRCount) {
		if (dwContactEMailMRCount == 1) {
			CHAR szEMail[MAX_EMAIL_LEN];
			size_t dwEMailSize;
			if (GetContactFirstEMail((HANDLE)wParam, TRUE, szEMail, SIZEOF(szEMail), &dwEMailSize))
				MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_WORLD_URL, sizeof(MRA_WORLD_URL), szEMail, dwEMailSize);
		}
		else MraSelectEMailDlgShow((HANDLE)wParam, MRA_SELECT_EMAIL_TYPE_WORLD);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMraProto::MraContactDeleted(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (!m_bLoggedIn || !hContact)
		return 0;

	if ( IsContactMra(hContact)) {
		CHAR szEMail[MAX_EMAIL_LEN];
		DWORD dwID, dwGroupID;
		size_t dwEMailSize;
		GetContactBasicInfoW(hContact, &dwID, &dwGroupID, NULL, NULL, NULL, szEMail, SIZEOF(szEMail), &dwEMailSize, NULL, 0, NULL, NULL, 0, NULL);

		MraSetContactStatus(hContact, ID_STATUS_OFFLINE);
		if ( !db_get_b(hContact, "CList", "NotOnList", 0) || dwID != -1)
			MraModifyContactW(hContact, dwID, CONTACT_FLAG_REMOVED, dwGroupID, szEMail, dwEMailSize, NULL, 0, NULL, 0);
		MraAvatarsDeleteContactAvatarFile(hAvatarsQueueHandle, hContact);
	}
	return 0;
}

int CMraProto::MraDbSettingChanged(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn || !lParam)
		return 0;

	HANDLE hContact = (HANDLE)wParam;
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;

	if (hContact) {
		// это наш контакт, он не временный (есть в списке на сервере) и его обновление разрешено
		if ( IsContactMra(hContact) && !db_get_b(hContact, "CList", "NotOnList", 0) && mraGetDword(hContact, "HooksLocked", FALSE) == FALSE) {
			CHAR szEMail[MAX_EMAIL_LEN], szPhones[MAX_EMAIL_LEN];
			WCHAR wszNick[MAX_EMAIL_LEN];
			DWORD dwID, dwGroupID, dwContactFlag;
			size_t dwEMailSize, dwNickSize, dwPhonesSize;

			if ( CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, cws->szModule, -1, "CList", 5) == CSTR_EQUAL) {
				// MyHandle setting
				if (CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, cws->szSetting, -1, "MyHandle", 8) == CSTR_EQUAL) {
					LPWSTR lpwszNewNick;

					// allways store custom nick
					if (cws->value.type == DBVT_DELETED) {
						lstrcpynW(wszNick, GetContactNameW(hContact), SIZEOF(wszNick));
						lpwszNewNick = wszNick;
						dwNickSize = lstrlenW(lpwszNewNick);
						DB_SetStringExW(hContact, "CList", "MyHandle", lpwszNewNick, dwNickSize);
					}
					else if (cws->value.pszVal) {
						switch (cws->value.type) {
						case DBVT_WCHAR:
							lpwszNewNick = cws->value.pwszVal;
							dwNickSize = lstrlenW(lpwszNewNick);
							break;
						case DBVT_UTF8:
							lpwszNewNick = wszNick;
							dwNickSize = MultiByteToWideChar(CP_UTF8, 0, cws->value.pszVal, -1, wszNick, SIZEOF(wszNick));
							break;
						case DBVT_ASCIIZ:
							lpwszNewNick = wszNick;
							dwNickSize = MultiByteToWideChar(MRA_CODE_PAGE, 0, cws->value.pszVal, -1, wszNick, SIZEOF(wszNick));
							break;
						default:
							lpwszNewNick = NULL;
							dwNickSize = 0;
							break;
						}
						if (lpwszNewNick)
						if (GetContactBasicInfoW(hContact, &dwID, &dwGroupID, &dwContactFlag, NULL, NULL, szEMail, SIZEOF(szEMail), &dwEMailSize, NULL, 0, NULL, szPhones, SIZEOF(szPhones), &dwPhonesSize) == NO_ERROR)
							MraModifyContactW(hContact, dwID, dwContactFlag, dwGroupID, szEMail, dwEMailSize, lpwszNewNick, dwNickSize, szPhones, dwPhonesSize);
					}
				}
				// Group setting
				else if ( CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, cws->szSetting, -1, "Group", 5) == CSTR_EQUAL ) {
					// manage group on server
					switch (cws->value.type) {
					case DBVT_ASCIIZ:
						break;
					case DBVT_DELETED:
						break;
					}
				}
				// NotOnList setting. Has a temporary contact just been added permanently?
				else if ( CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, cws->szSetting, -1, "NotOnList", 9) == CSTR_EQUAL) {
					if (cws->value.type == DBVT_DELETED || (cws->value.type == DBVT_BYTE && cws->value.bVal == 0)) {
						WCHAR wszAuthMessage[MAX_PATH];
						size_t dwAuthMessageSize;
						if ( mraGetStaticStringW(NULL, "AuthMessage", wszAuthMessage, SIZEOF(wszAuthMessage), &dwAuthMessageSize) == FALSE) {
							lstrcpynW(wszAuthMessage, TranslateW(MRA_DEFAULT_AUTH_MESSAGE), SIZEOF(wszAuthMessage));
							dwAuthMessageSize = lstrlenW(wszAuthMessage);
						}

						db_unset(hContact, "CList", "Hidden");
						GetContactBasicInfoW(hContact, NULL, &dwGroupID, &dwContactFlag, NULL, NULL, szEMail, SIZEOF(szEMail), &dwEMailSize, wszNick, SIZEOF(wszNick), &dwNickSize, szPhones, SIZEOF(szPhones), &dwPhonesSize);
						MraAddContactW(hContact, dwContactFlag, dwGroupID, szEMail, dwEMailSize, wszNick, dwNickSize, szPhones, dwPhonesSize, wszAuthMessage, dwAuthMessageSize, 0);
					}
				}
				// Hidden setting
				else if ( CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, cws->szSetting, -1, "Hidden" , 6) == CSTR_EQUAL) {
					GetContactBasicInfoW(hContact, &dwID, &dwGroupID, &dwContactFlag, NULL, NULL, szEMail, SIZEOF(szEMail), &dwEMailSize, wszNick, SIZEOF(wszNick), &dwNickSize, szPhones, SIZEOF(szPhones), &dwPhonesSize);
					if (cws->value.type == DBVT_DELETED || (cws->value.type == DBVT_BYTE && cws->value.bVal == 0))
						dwContactFlag &= ~CONTACT_FLAG_SHADOW;
					else
						dwContactFlag |= CONTACT_FLAG_SHADOW;

					MraModifyContactW(hContact, dwID, dwContactFlag, dwGroupID, szEMail, dwEMailSize, wszNick, dwNickSize, szPhones, dwPhonesSize);
				}
			}
			// Ignore section
			else if ( CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, cws->szModule, -1, "Ignore", 6) == CSTR_EQUAL) {
				if ( CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, cws->szSetting, -1, "Mask1", 5) == CSTR_EQUAL) {
					GetContactBasicInfoW(hContact, &dwID, &dwGroupID, &dwContactFlag, NULL, NULL, szEMail, SIZEOF(szEMail), &dwEMailSize, wszNick, SIZEOF(wszNick), &dwNickSize, szPhones, SIZEOF(szPhones), &dwPhonesSize);
					if (cws->value.type == DBVT_DELETED || (cws->value.type == DBVT_DWORD && cws->value.dVal&IGNOREEVENT_MESSAGE) == 0)
						dwContactFlag &= ~CONTACT_FLAG_IGNORE;
					else
						dwContactFlag |= CONTACT_FLAG_IGNORE;

					MraModifyContactW(hContact, dwID, dwContactFlag, dwGroupID, szEMail, dwEMailSize, wszNick, dwNickSize, szPhones, dwPhonesSize);
				}
			}
			// User info section
			else if (CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, cws->szModule, -1, "UserInfo", 8) == CSTR_EQUAL) {
				if ( CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, cws->szSetting, -1, "MyPhone0", 8) == CSTR_EQUAL ||
					  CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, cws->szSetting, -1, "MyPhone1", 8) == CSTR_EQUAL ||
					  CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, cws->szSetting, -1, "MyPhone2", 8) == CSTR_EQUAL) {
					GetContactBasicInfoW(hContact, &dwID, &dwGroupID, &dwContactFlag, NULL, NULL, szEMail, SIZEOF(szEMail), &dwEMailSize, wszNick, SIZEOF(wszNick), &dwNickSize, szPhones, SIZEOF(szPhones), &dwPhonesSize);
					MraModifyContactW(hContact, dwID, dwContactFlag, dwGroupID, szEMail, dwEMailSize, wszNick, dwNickSize, szPhones, dwPhonesSize);
				}
			}
		}
	}
	// not contact
	else {
		if ( CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, cws->szModule, -1, "CListGroups", 11) == CSTR_EQUAL) {
			// manage group on server
			switch (cws->value.type) {
			case DBVT_ASCIIZ:
				break;
			case DBVT_DELETED:
				break;
			}
		}
	}

	return 0;
}

int CMraProto::MraRebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	BOOL bIsContactMRA, bHasEMail, bHasEMailMR, bChatAgent;
	DWORD dwContactSeverFlags = 0;
	size_t dwBlogStatusMsgSize = 0;
	HANDLE hContact = (HANDLE)wParam;

	// proto own contact
	bIsContactMRA = IsContactMra(hContact);
	if (bIsContactMRA) {
		bHasEMail = TRUE;
		bHasEMailMR = TRUE;
		bChatAgent = IsContactChatAgent(hContact);
		GetContactBasicInfoW(hContact, NULL, NULL, NULL, &dwContactSeverFlags, NULL, NULL, 0, NULL, NULL, 0, NULL, NULL, 0, NULL);
		mraGetStaticStringW(hContact, DBSETTING_BLOGSTATUS, NULL, 0, &dwBlogStatusMsgSize);
	}
	// non proto contact
	else bHasEMail = bHasEMailMR = bChatAgent = FALSE;

	//"Request authorization"
	Menu_ShowItem(hContactMenuItems[0], (m_bLoggedIn && bIsContactMRA));// && (dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED)

	//"Grant authorization"
	Menu_ShowItem(hContactMenuItems[1], (m_bLoggedIn && bIsContactMRA && bChatAgent == FALSE));

	//"&Send postcard"
	Menu_ShowItem(hContactMenuItems[2], (m_bLoggedIn && bHasEMail && bChatAgent == FALSE));

	//"&View Album"
	Menu_ShowItem(hContactMenuItems[3], (m_bLoggedIn && bHasEMailMR && bChatAgent == FALSE));

	//"&Read Blog"
	Menu_ShowItem(hContactMenuItems[4], (m_bLoggedIn && bHasEMailMR && bChatAgent == FALSE));

	//"Reply Blog Status"
	Menu_ShowItem(hContactMenuItems[5], (m_bLoggedIn && dwBlogStatusMsgSize && bChatAgent == FALSE));

	//"View Video"
	Menu_ShowItem(hContactMenuItems[6], (m_bLoggedIn && bHasEMailMR && bChatAgent == FALSE));

	//"Answers"
	Menu_ShowItem(hContactMenuItems[7], (m_bLoggedIn && bHasEMailMR && bChatAgent == FALSE));

	//"World"
	Menu_ShowItem(hContactMenuItems[8], (m_bLoggedIn && bHasEMailMR && bChatAgent == FALSE));

	//"Send &Nudge"
	Menu_ShowItem(hContactMenuItems[9], (!heNudgeReceived) ? (m_bLoggedIn && bIsContactMRA) : 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Extra icons

int CMraProto::MraExtraIconsApply(WPARAM wParam, LPARAM lParam)
{
	SetExtraIcons((HANDLE)wParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMraProto::MraRebuildStatusMenu(WPARAM wParam, LPARAM lParam)
{
	CHAR szServiceFunction[MAX_PATH*2], *pszServiceFunctionName, szValueName[MAX_PATH];
	strncpy(szServiceFunction, m_szModuleName, sizeof(szServiceFunction));
	pszServiceFunctionName = szServiceFunction + strlen(m_szModuleName);

	WCHAR szItem[MAX_PATH+64], szStatusTitle[STATUS_TITLE_MAX+4];
	mir_sntprintf(szItem, SIZEOF(szItem), L"%s Custom Status", m_tszUserName);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 2000060000;
	mi.popupPosition = 500085000;
	mi.ptszPopupName = szItem;
	mi.flags = CMIF_UNICODE;
	mi.pszService = szServiceFunction;
	mi.pszContactOwner = m_szModuleName;

	for (size_t i = 0; i < MRA_XSTATUS_COUNT; i++)  {
		mir_snprintf(pszServiceFunctionName, 100, "/menuXStatus%ld", i);
		mi.position++;
		if (i) {
			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldName", i);
			if (mraGetStaticStringW(NULL, szValueName, szStatusTitle, (STATUS_TITLE_MAX+1), NULL)) {
				szStatusTitle[STATUS_TITLE_MAX] = 0;
				mi.ptszName = szStatusTitle;
			}
			else mi.ptszName = lpcszXStatusNameDef[i];

			mi.icolibItem = hXStatusAdvancedStatusIcons[i];
		}
		else {
			mi.ptszName = lpcszXStatusNameDef[i];
			mi.hIcon = NULL;
		}
		hXStatusMenuItems[i] = Menu_AddStatusMenuItem(&mi);
	}
	return 0;
}

INT_PTR CMraProto::MraSetListeningTo(WPARAM wParam, LPARAM lParam)
{
	LISTENINGTOINFO *pliInfo = (LISTENINGTOINFO*)lParam;

	if (pliInfo == NULL || pliInfo->cbSize != sizeof(LISTENINGTOINFO))
	{
		MraChangeUserBlogStatus(MRIM_BLOG_STATUS_MUSIC, NULL, 0, 0);
		mraDelValue(NULL, DBSETTING_BLOGSTATUSMUSIC);
	}
	else if (pliInfo->dwFlags & LTI_UNICODE) {
		LPWSTR pwszListeningTo;
		WCHAR wszListeningTo[MICBLOG_STATUS_MAX+4];
		size_t dwListeningToSize;

		if ( ServiceExists(MS_LISTENINGTO_GETPARSEDTEXT)) {
			pwszListeningTo = (LPWSTR)CallService(MS_LISTENINGTO_GETPARSEDTEXT, (WPARAM)L"%track%. %title% - %artist% - %player%", (LPARAM)pliInfo);
			dwListeningToSize = lstrlenW(pwszListeningTo);
		}
		else {
			pwszListeningTo = wszListeningTo;
			dwListeningToSize = mir_sntprintf(pwszListeningTo, SIZEOF(wszListeningTo), L"%s. %s - %s - %s", pliInfo->ptszTrack?pliInfo->ptszTrack:L"", pliInfo->ptszTitle?pliInfo->ptszTitle:L"", pliInfo->ptszArtist?pliInfo->ptszArtist:L"", pliInfo->ptszPlayer?pliInfo->ptszPlayer:L"");
		}

		mraSetStringExW(NULL, DBSETTING_BLOGSTATUSMUSIC, pwszListeningTo, dwListeningToSize);
		MraChangeUserBlogStatus(MRIM_BLOG_STATUS_MUSIC, pwszListeningTo, dwListeningToSize, 0);

		if (pwszListeningTo != wszListeningTo)
			mir_free(pwszListeningTo);
	}

	return 0;
}

int CMraProto::MraMusicChanged(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case WAT_EVENT_PLAYERSTATUS:
		// stopped
		if (1 == lParam) {
			mraDelValue(NULL, DBSETTING_BLOGSTATUSMUSIC);
			MraChangeUserBlogStatus(MRIM_BLOG_STATUS_MUSIC, NULL, 0, 0);
		}
		break;

	case WAT_EVENT_NEWTRACK:
		{
			SONGINFO *psiSongInfo;
			if (WAT_RES_OK == CallService(MS_WAT_GETMUSICINFO, WAT_INF_UNICODE, (LPARAM)&psiSongInfo)) {
				WCHAR wszMusic[MICBLOG_STATUS_MAX+4];
				size_t dwMusicSize;

				dwMusicSize = mir_sntprintf(wszMusic, SIZEOF(wszMusic), L"%ld. %s - %s - %s", psiSongInfo->track, psiSongInfo->artist, psiSongInfo->title, psiSongInfo->player);
				mraSetStringExW(NULL, DBSETTING_BLOGSTATUSMUSIC, wszMusic, dwMusicSize);
				MraChangeUserBlogStatus(MRIM_BLOG_STATUS_MUSIC, wszMusic, dwMusicSize, 0);
			}
		}
		break;
	}

	return 0;
}

DWORD CMraProto::MraSetXStatusInternal(DWORD dwXStatus)
{
	DWORD dwOldStatusMode;

	if ( IsXStatusValid(dwXStatus)) {
		CHAR szValueName[MAX_PATH];
		WCHAR szBuff[4096];
		size_t dwBuffSize;

		// obsolete (TODO: remove in next version)
		mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldName", dwXStatus);
		if (mraGetStaticStringW(NULL, szValueName, szBuff, SIZEOF(szBuff), &dwBuffSize) == FALSE) {
			lstrcpynW(szBuff, lpcszXStatusNameDef[dwXStatus], SIZEOF(szBuff));
			dwBuffSize = lstrlenW(szBuff);
		}
		if (dwBuffSize>STATUS_TITLE_MAX) dwBuffSize = STATUS_TITLE_MAX;
		mraSetStringExW(NULL, DBSETTING_XSTATUSNAME, szBuff, dwBuffSize);

		// obsolete (TODO: remove in next version)
		mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldMsg", dwXStatus);
		if (mraGetStaticStringW(NULL, szValueName, szBuff, SIZEOF(szBuff), &dwBuffSize)) {
			if (dwBuffSize>STATUS_DESC_MAX) dwBuffSize = STATUS_DESC_MAX;
			mraSetStringExW(NULL, DBSETTING_XSTATUSMSG, szBuff, dwBuffSize);
		}
		else mraDelValue(NULL, DBSETTING_XSTATUSMSG);
	}
	else {
		mraDelValue(NULL, DBSETTING_XSTATUSNAME);
		mraDelValue(NULL, DBSETTING_XSTATUSMSG);
		dwXStatus = MRA_MIR_XSTATUS_NONE;
	}

	dwOldStatusMode = InterlockedExchange((volatile LONG*)&m_iXStatus, dwXStatus);
	mraSetByte(NULL, DBSETTING_XSTATUSID, (BYTE)dwXStatus);

	MraSendNewStatus(m_iStatus, dwXStatus, NULL, 0, NULL, 0);

	return dwOldStatusMode;
}

INT_PTR CMraProto::MraSetXStatusEx(WPARAM wParam, LPARAM lParam)
{
	INT_PTR iRet = 1;
	DWORD dwXStatus;
	CUSTOM_STATUS *pData = (CUSTOM_STATUS*)lParam;

	if (pData->cbSize >= sizeof(CUSTOM_STATUS)) {
		iRet = 0;

		if (pData->flags & CSSF_MASK_STATUS) {
			dwXStatus = *pData->status;
			if ( IsXStatusValid(dwXStatus) == FALSE && dwXStatus != MRA_MIR_XSTATUS_NONE)
				iRet = 1;
		}
		else dwXStatus = m_iXStatus;

		if (pData->flags & (CSSF_MASK_NAME|CSSF_MASK_MESSAGE) && iRet == 0) {
			if ( IsXStatusValid(dwXStatus) || dwXStatus == MRA_MIR_XSTATUS_NONE) {
				CHAR szValueName[MAX_PATH];
				size_t dwBuffSize;

				// set custom status name
				if (pData->flags & CSSF_MASK_NAME) {
					mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldName", dwXStatus);
					if (pData->flags & CSSF_UNICODE) {
						dwBuffSize = lstrlenW(pData->pwszName);
						if (dwBuffSize>STATUS_TITLE_MAX) dwBuffSize = STATUS_TITLE_MAX;

						mraSetStringExW(NULL, szValueName, pData->pwszName, dwBuffSize);
						mraSetStringExW(NULL, DBSETTING_XSTATUSNAME, pData->pwszName, dwBuffSize);
					}
					else {
						dwBuffSize = lstrlenA(pData->pszName);
						if (dwBuffSize > STATUS_TITLE_MAX)
							dwBuffSize = STATUS_TITLE_MAX;

						mraSetStringExA(NULL, szValueName, pData->pszName, dwBuffSize);
						mraSetStringExA(NULL, DBSETTING_XSTATUSNAME, pData->pszName, dwBuffSize);
					}
				}

				// set custom status message
				if (pData->flags & CSSF_MASK_MESSAGE) {
					mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldMsg", dwXStatus);
					if (pData->flags & CSSF_UNICODE) {
						dwBuffSize = lstrlenW(pData->pwszMessage);
						if (dwBuffSize>STATUS_TITLE_MAX) dwBuffSize = STATUS_DESC_MAX;

						mraSetStringExW(NULL, szValueName, pData->pwszMessage, dwBuffSize);
						mraSetStringExW(NULL, DBSETTING_XSTATUSMSG, pData->pwszMessage, dwBuffSize);
					}
					else {
						dwBuffSize = lstrlenA(pData->pszMessage);
						if (dwBuffSize>STATUS_TITLE_MAX) dwBuffSize = STATUS_DESC_MAX;

						mraSetStringExA(NULL, szValueName, pData->pszMessage, dwBuffSize);
						mraSetStringExA(NULL, DBSETTING_XSTATUSMSG, pData->pszMessage, dwBuffSize);
					}
				}
			}
			// неудача только если мы не ставили Хстатус и попытались записать сообщения для "нет" статуса
			else if ( !(pData->flags & CSSF_MASK_STATUS))
				iRet = 1;
		}

		// set/update xstatus code and/or message
		if (pData->flags & (CSSF_MASK_STATUS|CSSF_MASK_NAME|CSSF_MASK_MESSAGE) && iRet == 0)
			MraSetXStatusInternal(dwXStatus);

		// hide menu items
		if (pData->flags & CSSF_DISABLE_UI) {
			bHideXStatusUI = (*pData->wParam)? FALSE:TRUE;
			for (DWORD i = 0; i < MRA_XSTATUS_COUNT; i++)
				Menu_ShowItem(hXStatusMenuItems[i], !bHideXStatusUI);
		}
	}
	return iRet;
}

INT_PTR CMraProto::MraGetXStatusEx(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	CUSTOM_STATUS *pData = (CUSTOM_STATUS*)lParam;

	if (pData->cbSize < sizeof(CUSTOM_STATUS))
		return 1;

	// fill status member
	if (pData->flags & CSSF_MASK_STATUS)
		*pData->status = m_iXStatus;

	// fill status name member
	if (pData->flags & CSSF_MASK_NAME) {
		if (pData->flags & CSSF_DEFAULT_NAME) {
			DWORD dwXStatus = (pData->wParam == NULL) ? m_iXStatus : *pData->wParam;
			if ( !IsXStatusValid(dwXStatus))
				return 1;

			if (pData->flags & CSSF_UNICODE)
				lstrcpynW(pData->pwszName, lpcszXStatusNameDef[dwXStatus], (STATUS_TITLE_MAX+1));
			else {
				size_t dwStatusTitleSize = lstrlenW( lpcszXStatusNameDef[dwXStatus] );
				if (dwStatusTitleSize>STATUS_TITLE_MAX) dwStatusTitleSize = STATUS_TITLE_MAX;

				WideCharToMultiByte(MRA_CODE_PAGE, 0, lpcszXStatusNameDef[dwXStatus], (DWORD)dwStatusTitleSize, pData->pszName, MAX_PATH, NULL, NULL );
				(*((CHAR*)(pData->pszName+dwStatusTitleSize))) = 0;
			}
		}
		else {
			if (pData->flags & CSSF_UNICODE)
				mraGetStaticStringW(hContact, DBSETTING_XSTATUSNAME, pData->pwszName, (STATUS_TITLE_MAX+1), NULL);
			else
				mraGetStaticStringA(hContact, DBSETTING_XSTATUSNAME, pData->pszName, (STATUS_TITLE_MAX+1), NULL);
		}
	}

	// fill status message member
	if (pData->flags & CSSF_MASK_MESSAGE) {
		char szSetting[100];
		mir_snprintf(szSetting, SIZEOF(szSetting), "XStatus%dMsg", m_iXStatus);
		if (pData->flags & CSSF_UNICODE)
			mraGetStaticStringW(hContact, szSetting, pData->pwszMessage, (STATUS_DESC_MAX+1), NULL);
		else
			mraGetStaticStringA(hContact, szSetting, pData->pszMessage, (STATUS_DESC_MAX+1), NULL);
	}

	if (pData->flags & CSSF_DISABLE_UI)
		if (pData->wParam)
			*pData->wParam = bHideXStatusUI;

	if (pData->flags & CSSF_STATUSES_COUNT)
		if (pData->wParam)
			*pData->wParam = MRA_XSTATUS_COUNT-1;

	//**deb можно оптимизировать, данный параметр возможно уже был вычислен при получении самих текстов
	if (pData->flags & CSSF_STR_SIZES) {
		if (pData->wParam) mraGetStaticStringW(hContact, DBSETTING_XSTATUSNAME, NULL, 0, (size_t*)pData->wParam);
		if (pData->lParam) mraGetStaticStringW(hContact, DBSETTING_XSTATUSMSG, NULL, 0, (size_t*)pData->lParam);
	}
	return 0;
}

INT_PTR CMraProto::MraGetXStatusIcon(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0)
		wParam = m_iXStatus;

	return (INT_PTR)IconLibGetIconEx(hXStatusAdvancedStatusIcons[wParam], lParam);
}

DWORD CMraProto::MraSendNewStatus(DWORD dwStatusMir, DWORD dwXStatusMir, LPCWSTR lpwszStatusTitle, size_t dwStatusTitleSize, LPCWSTR lpwszStatusDesc, size_t dwStatusDescSize)
{
	if (!m_bLoggedIn)
		return 0;

	CHAR szValueName[MAX_PATH];
	WCHAR wszStatusTitle[STATUS_TITLE_MAX+4], wszStatusDesc[STATUS_DESC_MAX+4];
	DWORD dwStatus, dwXStatus;

	dwStatus = GetMraStatusFromMiradaStatus(dwStatusMir, dwXStatusMir, &dwXStatus);
	if ( IsXStatusValid(dwXStatusMir)) {
		if (lpwszStatusTitle == NULL || dwStatusTitleSize == 0) {
			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldName", dwXStatusMir);
			// custom xstatus name
			if (mraGetStaticStringW(NULL, szValueName, wszStatusTitle, (STATUS_TITLE_MAX+1), &dwStatusTitleSize))
				lpwszStatusTitle = wszStatusTitle;
			// default xstatus name
			else {
				lpwszStatusTitle = TranslateW(lpcszXStatusNameDef[dwXStatusMir]);
				dwStatusTitleSize = lstrlenW(lpwszStatusTitle);
			}
		}

		if (lpwszStatusDesc == NULL || dwStatusDescSize == 0) {
			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldMsg", dwXStatusMir);
			// custom xstatus description
			if (mraGetStaticStringW(NULL, szValueName, wszStatusDesc, (STATUS_DESC_MAX+1), &dwStatusDescSize))
				lpwszStatusDesc = wszStatusDesc;
			// default xstatus description
			else {
				lpwszStatusDesc = NULL;
				dwStatusDescSize = 0;
			}
		}
	}
	else {
		if (lpwszStatusTitle == NULL || dwStatusTitleSize == 0) {
			lpwszStatusTitle = GetStatusModeDescriptionW(dwStatusMir);
			dwStatusTitleSize = lstrlenW(lpwszStatusTitle);
		}
	}

	MraChangeStatusW(dwStatus, lpcszStatusUri[dwXStatus], lstrlenA(lpcszStatusUri[dwXStatus]), lpwszStatusTitle, dwStatusTitleSize, lpwszStatusDesc, dwStatusDescSize, ((mraGetByte(NULL, "RTFReceiveEnable", MRA_DEFAULT_RTF_RECEIVE_ENABLE)? FEATURE_FLAG_RTF_MESSAGE:0)|MRA_FEATURE_FLAGS));
	return 0;
}

INT_PTR CMraProto::MraSendNudge(WPARAM wParam, LPARAM lParam)
{
	if (m_bLoggedIn && wParam) {
		CHAR szEMail[MAX_EMAIL_LEN];
		LPWSTR lpwszAlarmMessage = TranslateW(MRA_ALARM_MESSAGE);
		size_t dwEMailSize;
		HANDLE hContact = (HANDLE)wParam;

		if (mraGetStaticStringA(hContact, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize))
			if (MraMessageW(FALSE, hContact, 0, (MESSAGE_FLAG_RTF|MESSAGE_FLAG_ALARM), szEMail, dwEMailSize, lpwszAlarmMessage, lstrlenW(lpwszAlarmMessage), NULL, 0))
				return 0;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Avatars

INT_PTR CMraProto::MraGetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_MAXSIZE:
		return -1;

	case AF_PROPORTION:
		return PIP_NONE;

	case AF_FORMATSUPPORTED:
		return 0; // no formats to set

	case AF_ENABLED:
		return 1; // allways on

	case AF_DONTNEEDDELAYS:
		return 0; // need delay

	case AF_MAXFILESIZE:
		return 0;

	case AF_DELAYAFTERFAIL:
		return 5000;
	}

	return 0;
}


INT_PTR CMraProto::MraGetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	if (lParam) {
		PROTO_AVATAR_INFORMATIONT *ppai = (PROTO_AVATAR_INFORMATIONT*)lParam;
		return (INT_PTR)MraAvatarsQueueGetAvatar(hAvatarsQueueHandle, (DWORD)wParam, ppai->hContact, NULL, (DWORD*)&ppai->format, ppai->filename);
	}
	return GAIR_NOAVATAR;
}


INT_PTR CMraProto::MraGetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if ( MraAvatarsGetFileName(hAvatarsQueueHandle, NULL, GetContactAvatarFormat(NULL, PA_FORMAT_DEFAULT), (LPTSTR)wParam, (size_t)lParam, NULL) == NO_ERROR) {
		LPSTR lpsz = (LPSTR)wParam;
		return 0;
	}
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR LoadModules(void)
{
	DebugPrintCRLFW(L"MRA/LoadModules - DONE");
	return 0;
}

void UnloadModules()
{
	DebugPrintCRLFW(L"MRA/UnloadModules - DONE");
}
