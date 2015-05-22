#include "stdafx.h"
#include "proto.h"

INT_PTR CMraProto::MraGotoInbox(WPARAM, LPARAM)
{
	MraMPopSessionQueueAddUrl(hMPopSessionQueue, MRA_WIN_INBOX_URL);
	return 0;
}

INT_PTR CMraProto::MraShowInboxStatus(WPARAM, LPARAM)
{
	MraUpdateEmailStatus("", "", true);
	return 0;
}

INT_PTR CMraProto::MraEditProfile(WPARAM, LPARAM)
{
	MraMPopSessionQueueAddUrl(hMPopSessionQueue, MRA_EDIT_PROFILE_URL);
	return 0;
}

INT_PTR CMraProto::MraWebSearch(WPARAM, LPARAM)
{
	CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW | OUF_TCHAR, (LPARAM)MRA_SEARCH_URL);
	return 0;
}

INT_PTR CMraProto::MraUpdateAllUsersInfo(WPARAM, LPARAM)
{
	if (!m_bLoggedIn)
		return 0;

	if (MessageBox(NULL, TranslateT("Are you sure?"), TranslateT(MRA_UPD_ALL_USERS_INFO_STR), MB_YESNO | MB_ICONQUESTION) == IDYES) {
		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			CMStringA szEmail;
			if (mraGetStringA(hContact, "e-mail", szEmail))
				MraWPRequestByEMail(hContact, ACKTYPE_GETINFO, szEmail);
		}
	}
	return 0;
}

INT_PTR CMraProto::MraCheckUpdatesUsersAvt(WPARAM, LPARAM)
{
	if (MessageBox(NULL, TranslateT("Are you sure?"), TranslateT(MRA_CHK_USERS_AVATARS_STR), MB_YESNO | MB_ICONQUESTION) == IDYES) {
		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			CMStringA szEmail;
			if (mraGetStringA(hContact, "e-mail", szEmail))
			if (!IsEMailChatAgent(szEmail))
				MraAvatarsQueueGetAvatarSimple(hAvatarsQueueHandle, 0, hContact);
		}
	}
	return 0;
}

INT_PTR CMraProto::MraRequestAuthForAll(WPARAM, LPARAM)
{
	if (!m_bLoggedIn)
		return 0;

	if (MessageBox(NULL, TranslateT("Are you sure?"), TranslateT(MRA_REQ_AUTH_FOR_ALL_STR), MB_YESNO | MB_ICONQUESTION) == IDYES) {
		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			DWORD dwContactSeverFlags;
			if (GetContactBasicInfoW(hContact, NULL, NULL, NULL, &dwContactSeverFlags, NULL, NULL, NULL, NULL) == NO_ERROR)
			if (dwContactSeverFlags & CONTACT_INTFLAG_NOT_AUTHORIZED && dwContactSeverFlags != -1)
				MraRequestAuthorization(hContact, 0);
		}
	}
	return 0;
}

INT_PTR CMraProto::MraRequestAuthorization(WPARAM hContact, LPARAM)
{
	if (!hContact || !m_bLoggedIn)
		return 0;

	CMStringW wszAuthMessage;
	if (!mraGetStringW(NULL, "AuthMessage", wszAuthMessage))
		wszAuthMessage = TranslateT(MRA_DEFAULT_AUTH_MESSAGE);

	if (wszAuthMessage.IsEmpty())
		return 1;

	CMStringA szEmail;
	if (mraGetStringA(hContact, "e-mail", szEmail)) {
		BOOL bSlowSend = getByte("SlowSend", MRA_DEFAULT_SLOW_SEND);
		int iRet = MraMessage(bSlowSend, hContact, ACKTYPE_AUTHREQ, MESSAGE_FLAG_AUTHORIZE, szEmail, wszAuthMessage, NULL, 0);
		if (bSlowSend == FALSE)
			ProtoBroadcastAck(hContact, ACKTYPE_AUTHREQ, ACKRESULT_SUCCESS, (HANDLE)iRet, 0);

		return 0;
	}
	return 1;
}

INT_PTR CMraProto::MraGrantAuthorization(WPARAM wParam, LPARAM)
{
	if (!m_bLoggedIn || !wParam)
		return 0;

	// send without reason, do we need any ?
	CMStringA szEmail;
	if (mraGetStringA(wParam, "e-mail", szEmail))
		MraAuthorize(szEmail);

	return 0;
}

INT_PTR CMraProto::MraSendEmail(WPARAM wParam, LPARAM)
{
	DWORD dwContactEMailCount = GetContactEMailCount(wParam, FALSE);
	if (dwContactEMailCount) {
		if (dwContactEMailCount == 1) {
			CMStringA szUrl, szEmail;
			if (GetContactFirstEMail(wParam, FALSE, szEmail)) {
				szEmail.MakeLower();
				szUrl.Format("https://e.mail.ru/cgi-bin/sentmsg?To=%s", szEmail);
				MraMPopSessionQueueAddUrl(hMPopSessionQueue, szUrl);
			}
		}
		else MraSelectEMailDlgShow(wParam, MRA_SELECT_EMAIL_TYPE_SEND_POSTCARD);
	}
	return 0;
}

INT_PTR CMraProto::MraSendPostcard(WPARAM wParam, LPARAM)
{
	DWORD dwContactEMailCount = GetContactEMailCount(wParam, FALSE);
	if (dwContactEMailCount) {
		if (dwContactEMailCount == 1) {
			CMStringA szUrl, szEmail;
			if (GetContactFirstEMail(wParam, FALSE, szEmail)) {
				szEmail.MakeLower();
				szUrl.Format("http://cards.mail.ru/event.html?rcptname=%s&rcptemail=%s", GetContactNameA((HANDLE)wParam), szEmail);
				MraMPopSessionQueueAddUrl(hMPopSessionQueue, szUrl);
			}
		}
		else MraSelectEMailDlgShow(wParam, MRA_SELECT_EMAIL_TYPE_SEND_POSTCARD);
	}
	return 0;
}

INT_PTR CMraProto::MraViewAlbum(WPARAM wParam, LPARAM)
{
	DWORD dwContactEMailMRCount = GetContactEMailCount(wParam, TRUE);
	if (dwContactEMailMRCount) {
		if (dwContactEMailMRCount == 1) {
			CMStringA szEmail;
			if (GetContactFirstEMail(wParam, TRUE, szEmail))
				MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_FOTO_URL, szEmail);
		}
		else MraSelectEMailDlgShow(wParam, MRA_SELECT_EMAIL_TYPE_VIEW_ALBUM);
	}
	return 0;
}

INT_PTR CMraProto::MraReplyBlogStatus(WPARAM wParam, LPARAM)
{
	if (!m_bLoggedIn)
		return 0;

	CMStringW blogStatusMsg;
	mraGetStringW(wParam, DBSETTING_BLOGSTATUS, blogStatusMsg);
	if (!blogStatusMsg.IsEmpty() || wParam == 0)
		MraSendReplyBlogStatus(wParam);

	return 0;
}

INT_PTR CMraProto::MraViewVideo(WPARAM wParam, LPARAM)
{
	DWORD dwContactEMailMRCount = GetContactEMailCount(wParam, TRUE);
	if (dwContactEMailMRCount) {
		if (dwContactEMailMRCount == 1) {
			CMStringA szEmail;
			if (GetContactFirstEMail(wParam, TRUE, szEmail))
				MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_VIDEO_URL, szEmail);
		}
		else MraSelectEMailDlgShow(wParam, MRA_SELECT_EMAIL_TYPE_VIEW_VIDEO);
	}
	return 0;
}

INT_PTR CMraProto::MraAnswers(WPARAM wParam, LPARAM)
{
	DWORD dwContactEMailMRCount = GetContactEMailCount(wParam, TRUE);
	if (dwContactEMailMRCount) {
		if (dwContactEMailMRCount == 1) {
			CMStringA szEmail;
			if (GetContactFirstEMail(wParam, TRUE, szEmail))
				MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_ANSWERS_URL, szEmail);
		}
		else MraSelectEMailDlgShow(wParam, MRA_SELECT_EMAIL_TYPE_ANSWERS);
	}
	return 0;
}

INT_PTR CMraProto::MraWorld(WPARAM wParam, LPARAM)
{
	DWORD dwContactEMailMRCount = GetContactEMailCount(wParam, TRUE);
	if (dwContactEMailMRCount) {
		if (dwContactEMailMRCount == 1) {
			CMStringA szEmail;
			if (GetContactFirstEMail(wParam, TRUE, szEmail))
				MraMPopSessionQueueAddUrlAndEMail(hMPopSessionQueue, MRA_WORLD_URL, szEmail);
		}
		else MraSelectEMailDlgShow(wParam, MRA_SELECT_EMAIL_TYPE_WORLD);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMraProto::MraRebuildContactMenu(WPARAM hContact, LPARAM)
{
	bool bIsContactMRA, bHasEMail, bHasEMailMR, bChatAgent;
	DWORD dwContactSeverFlags = 0;
	CMStringW blogStatusMsgSize;

	// proto own contact
	bIsContactMRA = IsContactMra(hContact);
	if (bIsContactMRA) {
		bHasEMail = true;
		bHasEMailMR = true;
		bChatAgent = IsContactChatAgent(hContact);
		GetContactBasicInfoW(hContact, NULL, NULL, NULL, &dwContactSeverFlags, NULL, NULL, NULL, NULL);
		mraGetStringW(hContact, DBSETTING_BLOGSTATUS, blogStatusMsgSize);
	}
	// non proto contact
	else {
		bHasEMail = false;
		bHasEMailMR = false;
		bChatAgent = false;
		if (!getByte(NULL, "HideMenuItemsForNonMRAContacts", MRA_DEFAULT_HIDE_MENU_ITEMS_FOR_NON_MRA))
		if (!IsContactMraProto(hContact))// избегаем добавления менюшек в контакты других копий MRA
		if (GetContactEMailCount(hContact, FALSE)) {
			bHasEMail = true;
			if (GetContactEMailCount(hContact, TRUE)) bHasEMailMR = true;
		}
	}
	// menu root;
	Menu_ShowItem(hContactMenuRoot, bHasEMail);

	//"Request authorization"
	Menu_ShowItem(hContactMenuItems[0], (m_bLoggedIn && bIsContactMRA));// && (dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED)

	//"Grant authorization"
	Menu_ShowItem(hContactMenuItems[1], (m_bLoggedIn && bIsContactMRA && !bChatAgent));

	//"&Send E-Mail"
	Menu_ShowItem(hContactMenuItems[2], (bHasEMail && !bChatAgent));

	//"&Send postcard"
	Menu_ShowItem(hContactMenuItems[3], (bHasEMail && !bChatAgent));

	//"&View Album"
	Menu_ShowItem(hContactMenuItems[4], (bHasEMailMR && !bChatAgent));

	//"Reply Blog Status"
	Menu_ShowItem(hContactMenuItems[5], (m_bLoggedIn && blogStatusMsgSize.GetLength() && !bChatAgent));

	//"View Video"
	Menu_ShowItem(hContactMenuItems[6], (bHasEMailMR && !bChatAgent));

	//"Answers"
	Menu_ShowItem(hContactMenuItems[7], (bHasEMailMR && !bChatAgent));

	//"World"
	Menu_ShowItem(hContactMenuItems[8], (bHasEMailMR && !bChatAgent));

	//"Send &Nudge"
	Menu_ShowItem(hContactMenuItems[9], (!m_heNudgeReceived) ? (m_bLoggedIn && bIsContactMRA) : 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMraProto::MraRebuildStatusMenu(WPARAM, LPARAM)
{
	CHAR szServiceFunction[MAX_PATH * 2], *pszServiceFunctionName, szValueName[MAX_PATH];
	strncpy(szServiceFunction, m_szModuleName, sizeof(szServiceFunction));
	pszServiceFunctionName = szServiceFunction + mir_strlen(m_szModuleName);

	TCHAR szItem[MAX_PATH + 64];
	mir_sntprintf(szItem, SIZEOF(szItem), _T("%s Custom Status"), m_tszUserName);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 2000060000;
	mi.popupPosition = 500085000;
	mi.ptszPopupName = szItem;
	mi.flags = CMIF_UNICODE;
	mi.pszService = szServiceFunction;
	mi.pszContactOwner = m_szModuleName;

	CMStringW szStatusTitle;

	DWORD dwCount = MRA_XSTATUS_OFF_CLI_COUNT;
	if (getByte(NULL, "xStatusShowAll", MRA_DEFAULT_SHOW_ALL_XSTATUSES))
		dwCount = MRA_XSTATUS_COUNT;
	for (DWORD i = 0; i < dwCount; i ++) {
		mir_snprintf(pszServiceFunctionName, 100, "/menuXStatus%ld", i);
		mi.position ++;
		if (i) {
			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldName", i);
			if (mraGetStringW(NULL, szValueName, szStatusTitle))
				mi.ptszName = (TCHAR*)szStatusTitle.c_str();
			else
				mi.ptszName = (TCHAR*)lpcszXStatusNameDef[i];

			mi.icolibItem = hXStatusAdvancedStatusIcons[i];
		}
		else {
			mi.ptszName = (TCHAR*)lpcszXStatusNameDef[i];
			mi.hIcon = NULL;
		}
		hXStatusMenuItems[i] = Menu_AddStatusMenuItem(&mi);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

HGENMENU CMraProto::CListCreateMenu(LONG lPosition, LONG lPopupPosition, BOOL bIsMain, const IconItem *pgdiItems, size_t dwCount, HGENMENU *hResult)
{
	if (!pgdiItems || !dwCount || !hResult)
		return NULL;

	char szServiceFunction[MAX_PATH];
	strncpy(szServiceFunction, m_szModuleName, sizeof(szServiceFunction));
	char *pszServiceFunctionName = szServiceFunction + mir_strlen(m_szModuleName);

	CLISTMENUITEM mi = { sizeof(mi) };

	HGENMENU hRootMenu, (*fnAddFunc)(CLISTMENUITEM*);
	if (bIsMain) {
		fnAddFunc = Menu_AddProtoMenuItem;

		hRootMenu = MO_GetProtoRootMenu(m_szModuleName);
		if (hRootMenu == NULL) {
			mi.ptszName = m_tszUserName;
			mi.hParentMenu = HGENMENU_ROOT;
			mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
			mi.hIcon = g_hMainIcon;
			hRootMenu = Menu_AddProtoMenuItem(&mi);
		}

		mi.position = 20003;
		mi.hParentMenu = hRootMenu;
		mi.flags = CMIF_ROOTHANDLE;
	}
	else {
		fnAddFunc = Menu_AddContactMenuItem;
		mi.position = lPosition;
		mi.flags = CMIF_ROOTPOPUP;
	}

	mi.pszName = LPGEN("Services...");
	mi.hIcon = g_hMainIcon;
	hRootMenu = fnAddFunc(&mi);

	mi.flags = CMIF_CHILDPOPUP;
	mi.hParentMenu = hRootMenu;
	mi.popupPosition = lPopupPosition;
	mi.pszService = szServiceFunction;

	for (size_t i = 0; i < dwCount; i++) {
		strcpy(pszServiceFunctionName, pgdiItems[i].szName);
		mi.position = int(lPosition + i);
		mi.icolibItem = pgdiItems[i].hIcolib;
		mi.pszName = pgdiItems[i].szDescr;
		hResult[i] = fnAddFunc(&mi);
	}

	return hRootMenu;
}

void CMraProto::InitMenus()
{
	/* Main menu and contacts services. */
	CreateProtoService(MRA_GOTO_INBOX, &CMraProto::MraGotoInbox);
	CreateProtoService(MRA_SHOW_INBOX_STATUS, &CMraProto::MraShowInboxStatus);
	CreateProtoService(MRA_EDIT_PROFILE, &CMraProto::MraEditProfile);
	CreateProtoService(MRA_VIEW_ALBUM, &CMraProto::MraViewAlbum);
	CreateProtoService(MRA_REPLY_BLOG_STATUS, &CMraProto::MraReplyBlogStatus);
	CreateProtoService(MRA_VIEW_VIDEO, &CMraProto::MraViewVideo);
	CreateProtoService(MRA_ANSWERS, &CMraProto::MraAnswers);
	CreateProtoService(MRA_WORLD, &CMraProto::MraWorld);
	CreateProtoService(MRA_WEB_SEARCH, &CMraProto::MraWebSearch);
	CreateProtoService(MRA_UPD_ALL_USERS_INFO, &CMraProto::MraUpdateAllUsersInfo);
	CreateProtoService(MRA_CHK_USERS_AVATARS, &CMraProto::MraCheckUpdatesUsersAvt);
	CreateProtoService(MRA_REQ_AUTH_FOR_ALL, &CMraProto::MraRequestAuthForAll);
	/* Contacts only services. */
	CreateProtoService(MRA_REQ_AUTH, &CMraProto::MraRequestAuthorization);
	CreateProtoService(MRA_GRANT_AUTH, &CMraProto::MraGrantAuthorization);
	CreateProtoService(MRA_SEND_EMAIL, &CMraProto::MraSendEmail);
	CreateProtoService(MRA_SEND_POSTCARD, &CMraProto::MraSendPostcard);

	hContactMenuRoot = CListCreateMenu(-2000001001, -500050000, FALSE, gdiContactMenuItems, CONTACT_MENU_ITEMS_COUNT, hContactMenuItems);

	// xstatus menu
	for (DWORD i = 0; i < MRA_XSTATUS_COUNT; i++) {
		char szServiceName[100];
		mir_snprintf(szServiceName, SIZEOF(szServiceName), "/menuXStatus%d", i);
		CreateProtoServiceParam(szServiceName, &CMraProto::MraXStatusMenu, i);
	}
}
