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
	if ( GetContactBasicInfoW(hContact, &dwID, &dwGroupID, NULL, &dwContactSeverFlags, NULL, NULL, NULL, NULL))
		return;

	DWORD dwIconID = -1;
	DWORD dwXStatus = MRA_MIR_XSTATUS_NONE;

	if (m_bLoggedIn) {
		dwXStatus = getByte(hContact, DBSETTING_XSTATUSID, MRA_MIR_XSTATUS_NONE);
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
		CMStringW statusMsg;
		mraGetStringW(hContact, DBSETTING_BLOGSTATUS, statusMsg);
		if (statusMsg.GetLength())
			dwIconID = ADV_ICON_BLOGSTATUS;
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
	MraMPopSessionQueueAddUrl(hMPopSessionQueue, MRA_WIN_INBOX_URL);
	return 0;
}

INT_PTR CMraProto::MraShowInboxStatus(WPARAM wParam, LPARAM lParam)
{
	MraUpdateEmailStatus("", "", 0, 0);
	return 0;
}

INT_PTR CMraProto::MraSendSMS(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn || !wParam || !lParam)
		return 0;

	ptrW lpwszMessageXMLEncoded( mir_utf8decodeW((LPSTR)lParam));
	if (lpwszMessageXMLEncoded) {
		CMStringW decoded = DecodeXML( CMStringW(lpwszMessageXMLEncoded));
		if (decoded.GetLength())
			MraSMSW(NULL, CMStringA((LPSTR)wParam), decoded);
	}
	
	return 0;
}

INT_PTR CMraProto::MraEditProfile(WPARAM wParam, LPARAM lParam)
{
	MraMPopSessionQueueAddUrl(hMPopSessionQueue, MRA_EDIT_PROFILE_URL);
	return 0;
}

INT_PTR CMraProto::MraZhuki(WPARAM wParam, LPARAM lParam)
{
	MraMPopSessionQueueAddUrl(hMPopSessionQueue, MRA_ZHUKI_URL);
	return 0;
}

INT_PTR CMraProto::MraWebSearch(WPARAM wParam, LPARAM lParam)
{
	CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW | OUF_TCHAR, (LPARAM)MRA_SEARCH_URL);
	return 0;
}

INT_PTR CMraProto::MraUpdateAllUsersInfo(WPARAM wParam, LPARAM lParam)
{
	if ( MessageBox(NULL, TranslateT("Are you sure?"), TranslateTS(MRA_UPD_ALL_USERS_INFO_STR), MB_YESNO | MB_ICONQUESTION) == IDYES ) {
		for (HANDLE hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			CMStringA szEmail;
			if ( mraGetStringA(hContact, "e-mail", szEmail))
				MraWPRequestByEMail(hContact, ACKTYPE_GETINFO, szEmail);
		}
	}
	return 0;
}

INT_PTR CMraProto::MraCheckUpdatesUsersAvt(WPARAM wParam, LPARAM lParam)
{
	if ( MessageBox(NULL, TranslateT("Are you sure?"), TranslateTS(MRA_CHK_USERS_AVATARS_STR), MB_YESNO | MB_ICONQUESTION) == IDYES) {
		for (HANDLE hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			CMStringA szEmail;
			if (mraGetStringA(hContact, "e-mail", szEmail))
				if ( !IsEMailChatAgent(szEmail))
					MraAvatarsQueueGetAvatarSimple(hAvatarsQueueHandle, 0, hContact, 0);
		}
	}
	return 0;
}

INT_PTR CMraProto::MraRequestAuthForAll(WPARAM wParam, LPARAM lParam)
{
	if ( MessageBox(NULL, TranslateT("Are you sure?"), TranslateTS(MRA_REQ_AUTH_FOR_ALL_STR), MB_YESNO | MB_ICONQUESTION) == IDYES) {
		for (HANDLE hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			DWORD dwContactSeverFlags;
			if (GetContactBasicInfoW(hContact, NULL, NULL, NULL, &dwContactSeverFlags, NULL, NULL, NULL, NULL) == NO_ERROR)
			if (dwContactSeverFlags & CONTACT_INTFLAG_NOT_AUTHORIZED && dwContactSeverFlags != -1)
				MraRequestAuthorization((WPARAM)hContact, 0);
		}
	}
	return 0;
}

INT_PTR CMraProto::MraRequestAuthorization(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (!hContact)
		return 0;

	CMStringW wszAuthMessage;
	if ( !mraGetStringW(NULL, "AuthMessage", wszAuthMessage))
		wszAuthMessage = TranslateT(MRA_DEFAULT_AUTH_MESSAGE);

	if ( !wszAuthMessage.IsEmpty()) {
		CMStringA szEmail;
		if ( mraGetStringA(hContact, "e-mail", szEmail)) {
			BOOL bSlowSend = getByte("SlowSend", MRA_DEFAULT_SLOW_SEND);
			int iRet = MraMessage(bSlowSend, hContact, ACKTYPE_AUTHREQ, MESSAGE_FLAG_AUTHORIZE, szEmail, wszAuthMessage, NULL, 0);
			if (bSlowSend == FALSE)
				ProtoBroadcastAck(hContact, ACKTYPE_AUTHREQ, ACKRESULT_SUCCESS, (HANDLE)iRet, 0);

			return 0;
		}
	}

	return 1;
}

INT_PTR CMraProto::MraGrantAuthorization(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn || !wParam)
		return 0;

	// send without reason, do we need any ?
	CMStringA szEmail;
	if (mraGetStringA((HANDLE)wParam, "e-mail", szEmail))
		MraAuthorize(szEmail);

	return 0;
}

INT_PTR CMraProto::MraSendPostcard(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn)
		return 0;

	DWORD dwContactEMailCount = GetContactEMailCount((HANDLE)wParam, FALSE);
	if (dwContactEMailCount) {
		if (dwContactEMailCount == 1) {
			CMStringA szUrl, szEmail;
			if ( GetContactFirstEMail((HANDLE)wParam, FALSE, szEmail)) {
				szEmail.MakeLower();
				szUrl.Format("http://cards.mail.ru/event.html?rcptname=%s&rcptemail=%s", GetContactNameA((HANDLE)wParam), szEmail);
				MraMPopSessionQueueAddUrl(hMPopSessionQueue, szUrl);
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
			CMStringA szEmail;
			if ( GetContactFirstEMail((HANDLE)wParam, TRUE, szEmail))
				MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_FOTO_URL, szEmail);
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
		CMStringA szEmail;
		if (GetContactFirstEMail((HANDLE)wParam, TRUE, szEmail))
			MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_BLOGS_URL, szEmail);
	}
	else MraSelectEMailDlgShow((HANDLE)wParam, MRA_SELECT_EMAIL_TYPE_READ_BLOG);

	return 0;
}

INT_PTR CMraProto::MraReplyBlogStatus(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn)
		return 0;

	CMStringW blogStatusMsg;
	mraGetStringW((HANDLE)wParam, DBSETTING_BLOGSTATUS, blogStatusMsg);
	if (!blogStatusMsg.IsEmpty()|| wParam == 0)
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
			CMStringA szEmail;
			if (GetContactFirstEMail((HANDLE)wParam, TRUE, szEmail))
				MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_VIDEO_URL, szEmail);
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
			CMStringA szEmail;
			if (GetContactFirstEMail((HANDLE)wParam, TRUE, szEmail))
				MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_ANSWERS_URL, szEmail);
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
			CMStringA szEmail;
			if (GetContactFirstEMail((HANDLE)wParam, TRUE, szEmail))
				MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_WORLD_URL, szEmail);
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
		DWORD dwID, dwGroupID;
		GetContactBasicInfoW(hContact, &dwID, &dwGroupID, NULL, NULL, NULL, NULL, NULL, NULL);

		MraSetContactStatus(hContact, ID_STATUS_OFFLINE);
		if ( !db_get_b(hContact, "CList", "NotOnList", 0) || dwID != -1) {
			DWORD dwFlags = CONTACT_FLAG_REMOVED;
			MraModifyContact(hContact, &dwID, &dwFlags);
		}
		MraAvatarsDeleteContactAvatarFile(hAvatarsQueueHandle, hContact);
	}
	return 0;
}

int CMraProto::MraDbSettingChanged(WPARAM wParam, LPARAM lParam)
{
	if (!m_bLoggedIn || !lParam)
		return 0;

	HANDLE hContact = (HANDLE)wParam;
	if (hContact == NULL)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;

	// это наш контакт, он не временный (есть в списке на сервере) и его обновление разрешено
	if ( IsContactMra(hContact) && !db_get_b(hContact, "CList", "NotOnList", 0) && getDword(hContact, "HooksLocked", FALSE) == FALSE) {
		if ( !strcmp(cws->szModule, "CList")) {
			// MyHandle setting
			if ( !strcmp(cws->szSetting, "MyHandle")) {
				// always store custom nick
				CMStringW wszNick;
				if (cws->value.type == DBVT_DELETED) {
					wszNick = GetContactNameW(hContact);
					db_set_ws(hContact, "CList", "MyHandle", wszNick);
				}
				else if (cws->value.pszVal) {
					switch (cws->value.type) {
					case DBVT_WCHAR:
						wszNick = cws->value.pwszVal;
						break;
					case DBVT_UTF8:
						wszNick = ptrW( mir_utf8decodeW(cws->value.pszVal));
						break;
					case DBVT_ASCIIZ:
						wszNick = ptrW( mir_a2u_cp(cws->value.pszVal, MRA_CODE_PAGE));
						break;
					}
					if (wszNick.GetLength())
						MraModifyContact(hContact, 0, 0, 0, 0, &wszNick);
				}
			}
			// Group setting
			else if ( !strcmp(cws->szSetting, "Group")) {
				CMStringW wszGroup;
				// manage group on server
				switch (cws->value.type) {
				case DBVT_WCHAR:
					wszGroup = cws->value.pwszVal;
					break;
				case DBVT_UTF8:
					wszGroup = ptrW( mir_utf8decodeW(cws->value.pszVal));
					break;
				case DBVT_ASCIIZ:
					wszGroup = ptrW( mir_a2u_cp(cws->value.pszVal, MRA_CODE_PAGE));
					break;
				}
				if (wszGroup.GetLength()) {
					DWORD dwGroupID = getDword("GroupID", -1);
					if (dwGroupID != -1)
						MraMoveContactToGroup(hContact, dwGroupID, wszGroup);
				}
			}
			// NotOnList setting. Has a temporary contact just been added permanently?
			else if ( !strcmp(cws->szSetting, "NotOnList")) {
				if (cws->value.type == DBVT_DELETED || (cws->value.type == DBVT_BYTE && cws->value.bVal == 0)) {
					CMStringW wszAuthMessage;
					if ( !mraGetStringW(NULL, "AuthMessage", wszAuthMessage))
						wszAuthMessage = TranslateT(MRA_DEFAULT_AUTH_MESSAGE);

					db_unset(hContact, "CList", "Hidden");

					CMStringA szEmail, szPhones;
					CMStringW wszNick;
					DWORD dwGroupID, dwContactFlag;
					GetContactBasicInfoW(hContact, NULL, &dwGroupID, &dwContactFlag, NULL, NULL, &szEmail, &wszNick, &szPhones);
					MraAddContact(hContact, dwContactFlag, dwGroupID, szEmail, wszNick, &szPhones, &wszAuthMessage);
				}
			}
			// Hidden setting
			else if ( !strcmp(cws->szSetting, "Hidden")) {
				DWORD dwContactFlag = GetContactFlags(hContact);
				if (cws->value.type == DBVT_DELETED || (cws->value.type == DBVT_BYTE && cws->value.bVal == 0))
					dwContactFlag &= ~CONTACT_FLAG_SHADOW;
				else
					dwContactFlag |= CONTACT_FLAG_SHADOW;

				MraModifyContact(hContact, 0, &dwContactFlag);
			}
		}
		// Ignore section
		else if ( !strcmp(cws->szModule, "Ignore")) {
			if ( !strcmp(cws->szSetting, "Mask1")) {
				DWORD dwContactFlag = GetContactFlags(hContact);
				if (cws->value.type == DBVT_DELETED || (cws->value.type == DBVT_DWORD && cws->value.dVal&IGNOREEVENT_MESSAGE) == 0)
					dwContactFlag &= ~CONTACT_FLAG_IGNORE;
				else
					dwContactFlag |= CONTACT_FLAG_IGNORE;

				MraModifyContact(hContact, 0, &dwContactFlag);
			}
		}
		// User info section
		else if ( !strcmp(cws->szModule, "UserInfo")) {
			if ( !strcmp(cws->szSetting, "MyPhone0") || !strcmp(cws->szSetting, "MyPhone1") || !strcmp(cws->szSetting, "MyPhone2"))
				MraModifyContact(hContact);
		}
	}
	return 0;
}

int CMraProto::MraRebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	BOOL bIsContactMRA, bHasEMail, bHasEMailMR, bChatAgent;
	DWORD dwContactSeverFlags = 0;
	CMStringW blogStatusMsgSize;
	HANDLE hContact = (HANDLE)wParam;

	// proto own contact
	bIsContactMRA = IsContactMra(hContact);
	if (bIsContactMRA) {
		bHasEMail = TRUE;
		bHasEMailMR = TRUE;
		bChatAgent = IsContactChatAgent(hContact);
		GetContactBasicInfoW(hContact, NULL, NULL, NULL, &dwContactSeverFlags, NULL, NULL, NULL, NULL);
		mraGetStringW(hContact, DBSETTING_BLOGSTATUS, blogStatusMsgSize);
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
	Menu_ShowItem(hContactMenuItems[5], (m_bLoggedIn && blogStatusMsgSize.GetLength() && bChatAgent == FALSE));

	//"View Video"
	Menu_ShowItem(hContactMenuItems[6], (m_bLoggedIn && bHasEMailMR && bChatAgent == FALSE));

	//"Answers"
	Menu_ShowItem(hContactMenuItems[7], (m_bLoggedIn && bHasEMailMR && bChatAgent == FALSE));

	//"World"
	Menu_ShowItem(hContactMenuItems[8], (m_bLoggedIn && bHasEMailMR && bChatAgent == FALSE));

	//"Send &Nudge"
	Menu_ShowItem(hContactMenuItems[9], (!m_heNudgeReceived) ? (m_bLoggedIn && bIsContactMRA) : 0);
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

	TCHAR szItem[MAX_PATH+64];
	mir_sntprintf(szItem, SIZEOF(szItem), _T("%s Custom Status"), m_tszUserName);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 2000060000;
	mi.popupPosition = 500085000;
	mi.ptszPopupName = szItem;
	mi.flags = CMIF_UNICODE;
	mi.pszService = szServiceFunction;
	mi.pszContactOwner = m_szModuleName;

	CMStringW szStatusTitle;
	for (size_t i = 0; i < MRA_XSTATUS_COUNT; i++)  {
		mir_snprintf(pszServiceFunctionName, 100, "/menuXStatus%ld", i);
		mi.position++;
		if (i) {
			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldName", i);
			if ( mraGetStringW(NULL, szValueName, szStatusTitle))
				mi.ptszName = (TCHAR*)szStatusTitle.c_str();
			else
				mi.ptszName = lpcszXStatusNameDef[i];

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
		MraChangeUserBlogStatus(MRIM_BLOG_STATUS_MUSIC, _T(""), 0);
		delSetting(DBSETTING_BLOGSTATUSMUSIC);
	}
	else if (pliInfo->dwFlags & LTI_UNICODE) {
		CMStringW wszListeningTo;

		if ( ServiceExists(MS_LISTENINGTO_GETPARSEDTEXT))
			wszListeningTo = ptrT((LPWSTR)CallService(MS_LISTENINGTO_GETPARSEDTEXT, (WPARAM)L"%track%. %title% - %artist% - %player%", (LPARAM)pliInfo));
		else
			wszListeningTo.Format(L"%s. %s - %s - %s", pliInfo->ptszTrack?pliInfo->ptszTrack:_T(""), pliInfo->ptszTitle?pliInfo->ptszTitle:_T(""), pliInfo->ptszArtist?pliInfo->ptszArtist:_T(""), pliInfo->ptszPlayer?pliInfo->ptszPlayer:_T(""));

		mraSetStringExW(NULL, DBSETTING_BLOGSTATUSMUSIC, wszListeningTo);
		MraChangeUserBlogStatus(MRIM_BLOG_STATUS_MUSIC, wszListeningTo, 0);
	}

	return 0;
}

int CMraProto::MraMusicChanged(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case WAT_EVENT_PLAYERSTATUS:
		// stopped
		if (1 == lParam) {
			delSetting(DBSETTING_BLOGSTATUSMUSIC);
			MraChangeUserBlogStatus(MRIM_BLOG_STATUS_MUSIC, _T(""), 0);
		}
		break;

	case WAT_EVENT_NEWTRACK:
		{
			SONGINFO *psiSongInfo;
			if (WAT_RES_OK == CallService(MS_WAT_GETMUSICINFO, WAT_INF_UNICODE, (LPARAM)&psiSongInfo)) {
				CMStringW wszMusic;
				wszMusic.Format(_T("%ld. %s - %s - %s"), psiSongInfo->track, psiSongInfo->artist, psiSongInfo->title, psiSongInfo->player);
				mraSetStringExW(NULL, DBSETTING_BLOGSTATUSMUSIC, wszMusic);
				MraChangeUserBlogStatus(MRIM_BLOG_STATUS_MUSIC, wszMusic, 0);
			}
		}
		break;
	}

	return 0;
}

DWORD CMraProto::MraSetXStatusInternal(DWORD dwXStatus)
{
	if ( IsXStatusValid(dwXStatus)) {
		CMStringW szBuff;

		// obsolete (TODO: remove in next version)
		char szValueName[MAX_PATH];
		mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldName", dwXStatus);
		if ( !mraGetStringW(NULL, szValueName, szBuff))
			szBuff = lpcszXStatusNameDef[dwXStatus];
		mraSetStringExW(NULL, DBSETTING_XSTATUSNAME, szBuff);

		// obsolete (TODO: remove in next version)
		mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldMsg", dwXStatus);
		if (mraGetStringW(NULL, szValueName, szBuff))
			mraSetStringExW(NULL, DBSETTING_XSTATUSMSG, szBuff);
		else
			delSetting(DBSETTING_XSTATUSMSG);
	}
	else {
		delSetting(DBSETTING_XSTATUSNAME);
		delSetting(DBSETTING_XSTATUSMSG);
		dwXStatus = MRA_MIR_XSTATUS_NONE;
	}

	DWORD dwOldStatusMode = InterlockedExchange((volatile LONG*)&m_iXStatus, dwXStatus);
	setByte(DBSETTING_XSTATUSID, (BYTE)dwXStatus);

	MraSendNewStatus(m_iStatus, dwXStatus, _T(""), _T(""));

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

				// set custom status name
				if (pData->flags & CSSF_MASK_NAME) {
					mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldName", dwXStatus);
					if (pData->flags & CSSF_UNICODE) {
						mraSetStringExW(NULL, szValueName, pData->pwszName);
						mraSetStringExW(NULL, DBSETTING_XSTATUSNAME, pData->pwszName);
					}
					else {
						mraSetStringExA(NULL, szValueName, pData->pszName);
						mraSetStringExA(NULL, DBSETTING_XSTATUSNAME, pData->pszName);
					}
				}

				// set custom status message
				if (pData->flags & CSSF_MASK_MESSAGE) {
					mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldMsg", dwXStatus);
					if (pData->flags & CSSF_UNICODE) {
						mraSetStringExW(NULL, szValueName, pData->pwszMessage);
						mraSetStringExW(NULL, DBSETTING_XSTATUSMSG, pData->pwszMessage);
					}
					else {
						mraSetStringExA(NULL, szValueName, pData->pszMessage);
						mraSetStringExA(NULL, DBSETTING_XSTATUSMSG, pData->pszMessage);
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
				lstrcpyn(pData->ptszName, lpcszXStatusNameDef[dwXStatus], (STATUS_TITLE_MAX+1));
			else {
				size_t dwStatusTitleSize = lstrlen( lpcszXStatusNameDef[dwXStatus] );
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

DWORD CMraProto::MraSendNewStatus(DWORD dwStatusMir, DWORD dwXStatusMir, const CMStringW &pwszStatusTitle, const CMStringW &pwszStatusDesc)
{
	if (!m_bLoggedIn)
		return 0;

	CMStringW wszStatusTitle, wszStatusDesc;
	DWORD dwXStatus, dwStatus = GetMraStatusFromMiradaStatus(dwStatusMir, dwXStatusMir, &dwXStatus);
	if ( IsXStatusValid(dwXStatusMir)) {
		char szValueName[MAX_PATH];
		if (pwszStatusTitle.IsEmpty()) {
			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldName", dwXStatusMir);
			// custom xstatus name
			if ( !mraGetStringW(NULL, szValueName, wszStatusTitle))
				wszStatusTitle = TranslateTS(lpcszXStatusNameDef[dwXStatusMir]);
		}
		else wszStatusTitle = pwszStatusTitle;

		if (pwszStatusDesc.IsEmpty()) {
			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldMsg", dwXStatusMir);
			// custom xstatus description
			mraGetStringW(NULL, szValueName, wszStatusDesc);
		}
		else wszStatusDesc = pwszStatusDesc;
	}
	else if (pwszStatusTitle.IsEmpty())
		wszStatusTitle = GetStatusModeDescriptionW(dwStatusMir);

	MraChangeStatus(dwStatus, lpcszStatusUri[dwXStatus], wszStatusTitle, wszStatusDesc, ((getByte("RTFReceiveEnable", MRA_DEFAULT_RTF_RECEIVE_ENABLE)? FEATURE_FLAG_RTF_MESSAGE:0)|MRA_FEATURE_FLAGS));
	return 0;
}

INT_PTR CMraProto::MraSendNudge(WPARAM wParam, LPARAM lParam)
{
	if (m_bLoggedIn && wParam) {
		LPWSTR lpwszAlarmMessage = TranslateW(MRA_ALARM_MESSAGE);
		HANDLE hContact = (HANDLE)wParam;

		CMStringA szEmail;
		if (mraGetStringA(hContact, "e-mail", szEmail))
			if (MraMessage(FALSE, hContact, 0, (MESSAGE_FLAG_RTF|MESSAGE_FLAG_ALARM), szEmail, lpwszAlarmMessage, NULL, 0))
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
		return 1; // always on

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
	CMStringW wszFileName;
	if ( MraAvatarsGetFileName(hAvatarsQueueHandle, NULL, GetContactAvatarFormat(NULL, PA_FORMAT_DEFAULT), wszFileName) == NO_ERROR) {
		lstrcpyn((LPTSTR)wParam, wszFileName, (size_t)lParam);
		return 0;
	}
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// processes clist group removal

int CMraProto::OnGroupChanged(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact == NULL) {
		CLISTGROUPCHANGE *cgc = (CLISTGROUPCHANGE*)lParam;
		if (cgc->pszOldName == NULL)
			return 0;

		MraGroupItem *pGrp = NULL;
		for (int i=0; i < m_groups.getCount(); i++) {
			MraGroupItem &p = m_groups[i];
			if ( !_tcscmp(p.m_name, cgc->pszOldName)) {
				pGrp = &p;
				break;
			}
		}
		if (pGrp == NULL) // no MRA contacts in it
			return 0;

		DWORD dwFlags = CONTACT_FLAG_GROUP;
		CMStringW wszGroup;
		if (cgc->pszNewName != NULL) // renaming group
			wszGroup = cgc->pszNewName;
		else { // removing group
			dwFlags |= CONTACT_FLAG_REMOVED;
			wszGroup = cgc->pszOldName;
		}
		MraModifyContact(NULL, &pGrp->m_id, &dwFlags, 0, 0, &wszGroup);
	}
	return 0;
}
