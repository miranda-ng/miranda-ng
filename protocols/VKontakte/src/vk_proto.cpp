/*
Copyright (c) 2013-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static int sttCompareAsyncHttpRequest(const AsyncHttpRequest *p1, const AsyncHttpRequest *p2)
{
	if (p1->m_priority == p2->m_priority)
		return (int)p1->m_reqNum - (int)p2->m_reqNum;
	return (int)p2->m_priority - (int)p1->m_priority;
}

mir_cs csInstances;

CVkProto::CVkProto(const char *szModuleName, const wchar_t *pwszUserName) :
	PROTO<CVkProto>(szModuleName, pwszUserName),
	m_arRequestsQueue(10, sttCompareAsyncHttpRequest),
	m_cookies(5),
	m_msgId(1),
	m_chats(1, NumericKeySortT),
	m_ChatsTyping(1, NumericKeySortT),
	m_iLoadHistoryTask(0),
	m_bNotifyForEndLoadingHistory(false),
	m_bNotifyForEndLoadingHistoryAllContact(false),
	m_hAPIConnection(nullptr),
	m_pollingConn(nullptr),
	m_bSetBroadcast(false),
	m_bNeedSendOnline(false),
	m_bErr404Return(false),
	m_vkOptions(this)
{
	m_tWorkThreadTimer = m_tPoolThreadTimer = time(0);

	InitQueue();

	CreateProtoService(PS_CREATEACCMGRUI, &CVkProto::SvcCreateAccMgrUI);
	CreateProtoService(PS_GETAVATARINFO, &CVkProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CVkProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CVkProto::SvcGetMyAvatar);
	CreateProtoService(PS_SET_LISTENINGTO, &CVkProto::SvcSetListeningTo);

	HookProtoEvent(ME_OPT_INITIALISE, &CVkProto::OnOptionsInit);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = m_tszUserName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	CMStringA szListeningTo(FORMAT, "%sEnabled", m_szModuleName);
	db_set_b(0, "ListeningTo", szListeningTo, m_vkOptions.iMusicSendMetod == 0 ? 0 : 1);

	delSetting("InviteGroupIds");
	db_set_resident(m_szModuleName, "ActiveHistoryTask");

	// Set all contacts offline -- in case we crashed
	SetAllContactStatuses(ID_STATUS_OFFLINE);

	// Group chats
	GCREGISTER gcr = {};
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	CreateProtoService(PS_MENU_LOADHISTORY, &CVkProto::SvcGetAllServerHistoryForContact);

	CreateProtoService(PS_LEAVECHAT, &CVkProto::OnLeaveChat);
	CreateProtoService(PS_JOINCHAT, &CVkProto::OnJoinChat);
	HookProtoEvent(ME_GC_EVENT, &CVkProto::OnChatEvent);
	HookProtoEvent(ME_GC_BUILDMENU, &CVkProto::OnGcMenuHook);
}

CVkProto::~CVkProto()
{
	debugLogA("CVkProto::~CVkProto");
	UninitQueue();
	UnInitMenus();
	if (m_hPopupClassError)
		Popup_UnregisterClass(m_hPopupClassError);
	if (m_hPopupClassNotification)
		Popup_UnregisterClass(m_hPopupClassNotification);
}

void CVkProto::OnModulesLoaded()
{
	Clist_GroupCreate(0, m_vkOptions.pwszDefaultGroup);

	// Other hooks
	HookProtoEvent(ME_MSG_WINDOWEVENT, &CVkProto::OnProcessSrmmEvent);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &CVkProto::OnDbEventRead);
	HookProtoEvent(ME_DB_CONTACT_SETTINGCHANGED, &CVkProto::OnDbSettingChanged);

	//Sounds
	g_plugin.addSound("VKNewsFeed", m_tszUserName, LPGENW("Newsfeed & notification event"));

	InitPopups();
	InitMenus();
	InitDBCustomEvents();
	InitSmileys();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::InitSmileys()
{
	if (!m_vkOptions.bStikersAsSmileys)
		return;

	if (m_vkOptions.bUseStikersAsStaticSmileys)
		return;

	CMStringW wszPath(FORMAT, L"%s\\%S\\Stickers\\*.png", VARSW(L"%miranda_avatarcache%").get(), m_szModuleName);
	SMADD_CONT cont = { 2, m_szModuleName, wszPath };
	CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, LPARAM(&cont));
}

// Menu support

void CVkProto::OnBuildProtoMenu()
{
	for (auto &it : m_hProtoMenuItems)
		it = nullptr;

	CMenuItem mi(&g_plugin);
	mi.root = Menu_GetProtocolRoot(this);

	// Proto menu
	if (m_vkOptions.bShowProtoMenuItem0) {
		mi.pszService = PS_CREATECHAT;
		mi.position = 10009 + PMI_CREATECHAT;
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_JOIN);
		mi.name.a = LPGEN("Create new chat");
		SET_UID(mi, 0xc217748d, 0x7218, 0x4a62, 0xab, 0x37, 0x9, 0x58, 0x6a, 0x88, 0x71, 0x3e);
		m_hProtoMenuItems[PMI_CREATECHAT] = Menu_AddProtoMenuItem(&mi, m_szModuleName);
	}

	if (m_vkOptions.bShowProtoMenuItem1) {
		mi.pszService = PS_SETSTATUSMSG;
		mi.position = 10009 + PMI_SETSTATUSMSG;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_STATUS);
		mi.name.a = LPGEN("Status message");
		SET_UID(mi, 0x696bd932, 0xeee4, 0x4c4c, 0xa6, 0xf8, 0xb3, 0x72, 0xdf, 0xc6, 0xfa, 0xd1);
		m_hProtoMenuItems[PMI_SETSTATUSMSG] = Menu_AddProtoMenuItem(&mi, m_szModuleName);
	}

	if (m_vkOptions.bShowProtoMenuItem2) {
		mi.pszService = PS_WALLPOST;
		mi.position = 10009 + PMI_WALLPOST;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_WALL);
		mi.name.a = LPGEN("Send message to my wall");
		SET_UID(mi, 0x50a80044, 0xdddd, 0x47e6, 0x9e, 0x90, 0x32, 0x7b, 0x88, 0x13, 0x21, 0x4e);
		m_hProtoMenuItems[PMI_WALLPOST] = Menu_AddProtoMenuItem(&mi, m_szModuleName);
	}

	if (m_vkOptions.bShowProtoMenuItem3) {
		mi.pszService = PS_LOADVKNEWS;
		mi.position = 10009 + PMI_LOADVKNEWS;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_NOTIFICATION);
		mi.name.a = LPGEN("Load news from VK");
		SET_UID(mi, 0x7c449456, 0xb731, 0x48cc, 0x9c, 0x4e, 0x20, 0xe4, 0x66, 0x7a, 0x16, 0x23);
		m_hProtoMenuItems[PMI_LOADVKNEWS] = Menu_AddProtoMenuItem(&mi, m_szModuleName);
	}

	if (m_vkOptions.bShowProtoMenuItem4) {
		mi.pszService = PS_GETALLSERVERHISTORY;
		mi.position = 10009 + PMI_GETALLSERVERHISTORY;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_HISTORY);
		mi.name.a = LPGEN("Load history for all contacts from VK");
		SET_UID(mi, 0xe5028605, 0x92eb, 0x4956, 0xa0, 0xd0, 0x53, 0xb, 0x11, 0x44, 0x8f, 0x14);
		m_hProtoMenuItems[PMI_GETALLSERVERHISTORY] = Menu_AddProtoMenuItem(&mi, m_szModuleName);
	}

	if (m_vkOptions.bShowProtoMenuItem5) {
		mi.pszService = PS_WIPENONFRIENDS;
		mi.position = 10009 + PMI_WIPENONFRIENDS;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_FRIENDDEL);
		mi.name.a = LPGEN("Wipe contacts missing in friend list");
		SET_UID(mi, 0xcfe99159, 0xf237, 0x4546, 0x80, 0x3e, 0x51, 0x88, 0x26, 0x55, 0xdc, 0x5f);
		m_hProtoMenuItems[PMI_WIPENONFRIENDS] = Menu_AddProtoMenuItem(&mi, m_szModuleName);
	}

	if (m_vkOptions.bShowProtoMenuItem6) {
		mi.pszService = PS_VISITPROFILE;
		mi.position = 10009 + PMI_VISITPROFILE;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_VISITPROFILE);
		mi.name.a = LPGEN("Visit profile");
		SET_UID(mi, 0x9550515e, 0x2a45, 0x4913, 0x95, 0x1a, 0x1e, 0xfa, 0x7, 0xc6, 0x2d, 0x60);
		m_hProtoMenuItems[PMI_VISITPROFILE] = Menu_AddProtoMenuItem(&mi, m_szModuleName);
	}
}

void CVkProto::InitMenus()
{
	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU, &CVkProto::OnPreBuildContactMenu);

	//Contact Menu Services
	CreateProtoService(PS_GETSERVERHISTORYLAST1DAY, &CVkProto::SvcGetServerHistoryLastNDay<1>);
	CreateProtoService(PS_GETSERVERHISTORYLAST3DAY, &CVkProto::SvcGetServerHistoryLastNDay<3>);
	CreateProtoService(PS_GETSERVERHISTORYLAST7DAY, &CVkProto::SvcGetServerHistoryLastNDay<7>);
	CreateProtoService(PS_GETSERVERHISTORYLAST30DAY, &CVkProto::SvcGetServerHistoryLastNDay<30>);
	CreateProtoService(PS_GETSERVERHISTORYLAST90DAY, &CVkProto::SvcGetServerHistoryLastNDay<90>);
	CreateProtoService(PS_GETALLSERVERHISTORYFORCONTACT, &CVkProto::SvcGetAllServerHistoryForContact);
	CreateProtoService(PS_GETALLSERVERHISTORY, &CVkProto::SvcGetAllServerHistory);
	CreateProtoService(PS_VISITPROFILE, &CVkProto::SvcVisitProfile);
	CreateProtoService(PS_CREATECHAT, &CVkProto::SvcCreateChat);
	CreateProtoService(PS_ADDASFRIEND, &CVkProto::SvcAddAsFriend);
	CreateProtoService(PS_DELETEFRIEND, &CVkProto::SvcDeleteFriend);
	CreateProtoService(PS_BANUSER, &CVkProto::SvcBanUser);
	CreateProtoService(PS_REPORTABUSE, &CVkProto::SvcReportAbuse);
	CreateProtoService(PS_DESTROYKICKCHAT, &CVkProto::SvcDestroyKickChat);
	CreateProtoService(PS_OPENBROADCAST, &CVkProto::SvcOpenBroadcast);
	CreateProtoService(PS_LOADVKNEWS, &CVkProto::SvcLoadVKNews);
	CreateProtoService(PS_WIPENONFRIENDS, &CVkProto::SvcWipeNonFriendContacts);
	CreateProtoService(PS_SETSTATUSMSG, &CVkProto::SvcSetStatusMsg);
	CreateProtoService(PS_WALLPOST, &CVkProto::SvcWallPost);
	CreateProtoService(PS_MARKMESSAGESASREAD, &CVkProto::SvcMarkMessagesAsRead);


	// Contact Menu Items
	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;

	mi.pszService = PS_VISITPROFILE;
	mi.position = -200001000 + CMI_VISITPROFILE;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_VISITPROFILE);
	mi.name.w = LPGENW("Visit profile");
	SET_UID(mi, 0x828cc50e, 0x398d, 0x43a2, 0xbf, 0xd3, 0xa9, 0x96, 0x47, 0x9d, 0x52, 0xff);
	m_hContactMenuItems[CMI_VISITPROFILE] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_MARKMESSAGESASREAD;
	mi.position = -200001000 + CMI_MARKMESSAGESASREAD;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_MARKMESSAGESASREAD);
	mi.name.w = LPGENW("Mark messages as read");
	SET_UID(mi, 0x2587a649, 0xe5d5, 0x4e90, 0x8b, 0x35, 0x81, 0x4c, 0xb1, 0x5, 0x94, 0x7);
	m_hContactMenuItems[CMI_MARKMESSAGESASREAD] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_WALLPOST;
	mi.position = -200001000 + CMI_WALLPOST;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_WALL);
	mi.name.w = LPGENW("Send message to user's wall");
	SET_UID(mi, 0xd8841aaf, 0x15f6, 0x4be9, 0x9f, 0x4f, 0x16, 0xa9, 0x47, 0x6a, 0x19, 0x81);
	m_hContactMenuItems[CMI_WALLPOST] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_ADDASFRIEND;
	mi.position = -200001000 + CMI_ADDASFRIEND;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_FRIENDADD);
	mi.name.w = LPGENW("Add as friend");
	SET_UID(mi, 0xf11b9a7f, 0x569, 0x4023, 0xb0, 0xd6, 0xa3, 0x16, 0xf6, 0xd4, 0xfb, 0xb5);
	m_hContactMenuItems[CMI_ADDASFRIEND] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_DELETEFRIEND;
	mi.position = -200001000 + CMI_DELETEFRIEND;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_FRIENDDEL);
	mi.name.w = LPGENW("Delete from friend list");
	SET_UID(mi, 0x1e26514, 0x854f, 0x4e60, 0x8c, 0xf8, 0xab, 0xaa, 0xe0, 0xc3, 0xa5, 0xa7);
	m_hContactMenuItems[CMI_DELETEFRIEND] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_BANUSER;
	mi.position = -200001000 + CMI_BANUSER;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_BAN);
	mi.name.w = LPGENW("Ban user");
	SET_UID(mi, 0x7ba06bab, 0xf770, 0x4938, 0x9c, 0x76, 0xef, 0x40, 0xbc, 0x55, 0x0, 0x9b);
	m_hContactMenuItems[CMI_BANUSER] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_REPORTABUSE;
	mi.position = -200001000 + CMI_REPORTABUSE;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ABUSE);
	mi.name.w = LPGENW("Report abuse");
	SET_UID(mi, 0x56454cb9, 0xd80, 0x4050, 0xbe, 0xfc, 0x2c, 0xf6, 0x10, 0x2a, 0x7d, 0x19);
	m_hContactMenuItems[CMI_REPORTABUSE] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_DESTROYKICKCHAT;
	mi.position = -200001000 + CMI_DESTROYKICKCHAT;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_FRIENDDEL);
	mi.name.w = LPGENW("Destroy room");
	SET_UID(mi, 0x4fa6e75a, 0x30cd, 0x4482, 0xae, 0x8f, 0x0, 0x38, 0xd0, 0x17, 0x33, 0xcd);
	m_hContactMenuItems[CMI_DESTROYKICKCHAT] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_OPENBROADCAST;
	mi.position = -200001000 + CMI_OPENBROADCAST;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_BROADCAST);
	mi.name.w = LPGENW("Open broadcast");
	SET_UID(mi, 0x85251a06, 0xf734, 0x4985, 0x8c, 0x36, 0x6f, 0x66, 0x46, 0xf9, 0xa0, 0x10);
	m_hContactMenuItems[CMI_OPENBROADCAST] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_LOADVKNEWS;
	mi.position = -200001000 + CMI_LOADVKNEWS;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_NOTIFICATION);
	mi.name.w = LPGENW("Load news from VK");
	SET_UID(mi, 0xe1f6888b, 0x21ae, 0x409f, 0x82, 0xa2, 0x7b, 0x72, 0xef, 0x47, 0x9, 0xc0);
	m_hContactMenuItems[CMI_LOADVKNEWS] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	// Sync history menu
	mi.pszService = PS_GETSERVERHISTORY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_HISTORY);
	mi.name.w = LPGENW("Reload messages from vk.com...");
	SET_UID(mi, 0xc6b59e9f, 0x5250, 0x4146, 0xb6, 0xf3, 0x2d, 0xe1, 0x4, 0x3b, 0x95, 0xf5);
	m_hContactMenuItems[CMI_GETSERVERHISTORY] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.root = m_hContactMenuItems[CMI_GETSERVERHISTORY];

	mi.pszService = PS_GETSERVERHISTORYLAST1DAY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY + 100 + CHMI_GETSERVERHISTORYLAST1DAY;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_HISTORY);
	mi.name.w = LPGENW("for last 1 day");
	SET_UID(mi, 0x508dce88, 0x1a9a, 0x4dd7, 0x90, 0xf4, 0x41, 0x35, 0x7b, 0xc3, 0x17, 0xed);
	m_hContactHistoryMenuItems[CHMI_GETSERVERHISTORYLAST1DAY] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_GETSERVERHISTORYLAST3DAY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY + 100 + CHMI_GETSERVERHISTORYLAST3DAY;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_HISTORY);
	mi.name.w = LPGENW("for last 3 days");
	SET_UID(mi, 0x9a878764, 0x5bbf, 0x433a, 0xbd, 0x50, 0xa9, 0xb9, 0x16, 0x1f, 0x99, 0x29);
	m_hContactHistoryMenuItems[CHMI_GETSERVERHISTORYLAST3DAY] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_GETSERVERHISTORYLAST7DAY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY + 100 + CHMI_GETSERVERHISTORYLAST7DAY;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_HISTORY);
	mi.name.w = LPGENW("for last week");
	SET_UID(mi, 0xc6482460, 0xd280, 0x4596, 0x97, 0x4b, 0xf7, 0xfa, 0x6d, 0xe, 0xd5, 0xda);
	m_hContactHistoryMenuItems[CHMI_GETSERVERHISTORYLAST7DAY] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_GETSERVERHISTORYLAST30DAY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY + 100 + CHMI_GETSERVERHISTORYLAST30DAY;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_HISTORY);
	mi.name.w = LPGENW("for last 30 days");
	SET_UID(mi, 0xc48e8a9f, 0x2860, 0x4d5b, 0xa8, 0xdf, 0xb8, 0x3f, 0xdf, 0x7b, 0xa2, 0xba);
	m_hContactHistoryMenuItems[CHMI_GETSERVERHISTORYLAST30DAY] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_GETSERVERHISTORYLAST90DAY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY + 100 + CHMI_GETSERVERHISTORYLAST90DAY;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_HISTORY);
	mi.name.w = LPGENW("for last 90 days");
	SET_UID(mi, 0xd8e30530, 0xa585, 0x4672, 0xa6, 0x39, 0x18, 0xc9, 0xc9, 0xcb, 0xc7, 0x7d);
	m_hContactHistoryMenuItems[CHMI_GETSERVERHISTORYLAST90DAY] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_GETALLSERVERHISTORYFORCONTACT;
	mi.position = -200001000 + CMI_GETSERVERHISTORY + 100 + CHMI_GETALLSERVERHISTORYFORCONTACT;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_HISTORY);
	mi.name.w = LPGENW("for all time");
	SET_UID(mi, 0xaee3d02b, 0x3667, 0x47c8, 0x9f, 0x43, 0x14, 0xb7, 0xab, 0x52, 0x14, 0x94);
	m_hContactHistoryMenuItems[CHMI_GETALLSERVERHISTORYFORCONTACT] = Menu_AddContactMenuItem(&mi, m_szModuleName);
}

int CVkProto::OnPreBuildContactMenu(WPARAM hContact, LPARAM)
{
	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	bool bisFriend = !getBool(hContact, "Auth", true);
	bool bisBroadcast = !(IsEmpty(ptrW(db_get_wsa(hContact, m_szModuleName, "AudioUrl"))));
	bool bIsGroup = IsGroupUser(hContact);
	Menu_ShowItem(m_hContactMenuItems[CMI_VISITPROFILE], userID != VK_FEED_USER);
	Menu_ShowItem(m_hContactMenuItems[CMI_MARKMESSAGESASREAD], !isChatRoom(hContact) && userID != VK_FEED_USER);
	Menu_ShowItem(m_hContactMenuItems[CMI_WALLPOST], !isChatRoom(hContact));
	Menu_ShowItem(m_hContactMenuItems[CMI_ADDASFRIEND], !bisFriend && !isChatRoom(hContact) && userID != VK_FEED_USER && !bIsGroup);
	Menu_ShowItem(m_hContactMenuItems[CMI_DELETEFRIEND], bisFriend && userID != VK_FEED_USER && !bIsGroup);
	Menu_ShowItem(m_hContactMenuItems[CMI_BANUSER], !isChatRoom(hContact) && userID != VK_FEED_USER && !bIsGroup);
	Menu_ShowItem(m_hContactMenuItems[CMI_REPORTABUSE], !isChatRoom(hContact) && userID != VK_FEED_USER && !bIsGroup);
	Menu_ShowItem(m_hContactMenuItems[CMI_DESTROYKICKCHAT], isChatRoom(hContact) && getBool(hContact, "off"));
	Menu_ShowItem(m_hContactMenuItems[CMI_OPENBROADCAST], !isChatRoom(hContact) && bisBroadcast);
	Menu_ShowItem(m_hContactMenuItems[CMI_GETSERVERHISTORY], !isChatRoom(hContact) && userID != VK_FEED_USER);
	Menu_ShowItem(m_hContactMenuItems[CMI_LOADVKNEWS], userID == VK_FEED_USER);
	for (int i = 0; i < CHMI_COUNT; i++)
		Menu_ShowItem(m_hContactHistoryMenuItems[i], !isChatRoom(hContact) && userID != VK_FEED_USER);
	return 0;
}

void CVkProto::UnInitMenus()
{
	debugLogA("CVkProto::UnInitMenus");
	for (int i = 0; i < PMI_COUNT; i++)
		if (m_hProtoMenuItems[i])
			Menu_RemoveItem(m_hProtoMenuItems[i]);

	for (int i = 0; i < CHMI_COUNT; i++)
		Menu_RemoveItem(m_hContactHistoryMenuItems[i]);

	for (int i = 0; i < CMI_COUNT; i++)
		Menu_RemoveItem(m_hContactMenuItems[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////
// PopUp support

LRESULT CALLBACK PopupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
	case WM_CONTEXTMENU:
	{
		CVkSendMsgParam *pd = (CVkSendMsgParam *)PUGetPluginData(hwnd);
		if (pd != nullptr && pd->hContact != 0)
			CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)pd->hContact, 0);
		PUDeletePopup(hwnd);
	}
	break;
	case UM_FREEPLUGINDATA:
	{
		CVkSendMsgParam *pd = (CVkSendMsgParam *)PUGetPluginData(hwnd);
		delete pd;
	}
	return FALSE;
	default:
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

void CVkProto::InitPopups(void)
{
	wchar_t desc[256];
	char name[256];

	POPUPCLASS ppc = {};
	ppc.flags = PCF_UNICODE;
	ppc.pszName = name;
	ppc.pszDescription.w = desc;
	ppc.PluginWindowProc = PopupDlgProc;
	ppc.lParam = APF_RETURN_HWND;

	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Errors"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Error");
	ppc.hIcon = Skin_LoadIcon(SKINICON_ERROR);
	ppc.colorBack = RGB(191, 0, 0);     // Red
	ppc.colorText = RGB(255, 245, 225); // Yellow
	ppc.iSeconds = 60;
	m_hPopupClassError = Popup_RegisterClass(&ppc);

	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Notifications"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Notification");
	ppc.hIcon = g_plugin.getIcon(IDI_NOTIFICATION);
	ppc.colorBack = RGB(190, 225, 255); // Blue
	ppc.colorText = RGB(255, 255, 255); // White
	ppc.iSeconds = 4;
	m_hPopupClassNotification = Popup_RegisterClass(&ppc);
}

void CVkProto::MsgPopup(MCONTACT hContact, const wchar_t *wszMsg, const wchar_t *wszTitle, bool err)
{
	char name[256];

	POPUPDATACLASS ppd = {};
	ppd.szTitle.w = wszTitle;
	ppd.szText.w = wszMsg;
	ppd.pszClassName = name;
	ppd.hContact = hContact;
	ppd.PluginData = new CVkSendMsgParam(hContact);
	mir_snprintf(name, "%s_%s", m_szModuleName, err ? "Error" : "Notification");
	Popup_AddClass(&ppd);
}

void CVkProto::MsgPopup(const wchar_t *wszMsg, const wchar_t *wszTitle, bool err)
{
	MsgPopup(0, wszMsg, wszTitle, err);
}
//////////////////////////////////////////////////////////////////////////////

void  CVkProto::InitDBCustomEvents()
{
	// custom event
	DBEVENTTYPEDESCR dbEventType = {};
	dbEventType.module = m_szModuleName;
	dbEventType.flags = DETF_HISTORY | DETF_MSGWINDOW;

	dbEventType.eventType = VK_USER_DEACTIVATE_ACTION;
	dbEventType.descr = Translate("VK user activate/deactivate action");
	DbEvent_RegisterType(&dbEventType);

}

//////////////////////////////////////////////////////////////////////////////

void CVkProto::OnShutdown()
{
	debugLogA("CVkProto::OnPreShutdown");

	m_bTerminated = true;
	SetEvent(m_evRequestsQueue);
}

//////////////////////////////////////////////////////////////////////////////

INT_PTR CVkProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_CHAT | PF1_SERVERCLIST | PF1_AUTHREQ | PF1_BASICSEARCH
			| PF1_SEARCHBYNAME | PF1_SEARCHBYEMAIL | PF1_MODEMSG | PF1_FILESEND | PF1_FILERESUME;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_IDLE;

	case PFLAGNUM_3:
		return PF2_ONLINE;

	case PFLAGNUM_4:
		return PF4_AVATARS | PF4_SUPPORTTYPING | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE
			| PF4_OFFLINEFILES | PF4_READNOTIFY | PF4_GROUPCHATFILES | PF4_SERVERMSGID;

	case PFLAG_MAXLENOFMESSAGE:
		return 4096;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)L"VKontakte ID";
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

MCONTACT CVkProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	debugLogA("CVkProto::AddToList");

	int uid = _wtoi(psr->id.w);
	if (!uid)
		return 0;

	MCONTACT hContact = FindUser(uid, true);
	RetrieveUserInfo(uid);
	return hContact;
}

int CVkProto::AuthRequest(MCONTACT hContact, const wchar_t *message)
{
	debugLogA("CVkProto::AuthRequest");
	if (!IsOnline())
		return 1;

	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	if (userID == VK_INVALID_USER || !hContact || userID == VK_FEED_USER)
		return 1;

	if (userID < 0)
		return 1;


	wchar_t msg[501] = { 0 };
	if (message)
		wcsncpy_s(msg, _countof(msg), message, _TRUNCATE);

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/friends.add.json", true, &CVkProto::OnReceiveAuthRequest)
		<< INT_PARAM("user_id", userID)
		<< WCHAR_PARAM("text", msg))->pUserInfo = new CVkSendMsgParam(hContact);

	return 0;
}

void CVkProto::OnReceiveAuthRequest(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveAuthRequest %d", reply->resultCode);
	CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;
	if (reply->resultCode == 200 && param) {
		JSONNode jnRoot;
		const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
		if (jnResponse) {
			int iRet = jnResponse.as_int();
			setByte(param->hContact, "Auth", 0);
			if (iRet == 2) {
				CMStringW msg, wszNick(db_get_wsm(param->hContact, m_szModuleName, "Nick"));
				if (wszNick.IsEmpty())
					wszNick = TranslateT("(Unknown contact)");
				msg.AppendFormat(TranslateT("User %s added as friend"), wszNick.c_str());
				MsgPopup(param->hContact, msg, wszNick);
			}
		}
		else {
			switch (pReq->m_iErrorCode) {
			case VKERR_HIMSELF_AS_FRIEND:
				MsgPopup(param->hContact, TranslateT("You cannot add yourself as friend"), TranslateT("Error"), true);
				break;
			case VKERR_YOU_ON_BLACKLIST:
				MsgPopup(param->hContact, TranslateT("Cannot add this user to friends as they have put you on their blacklist"), TranslateT("Error"), true);
				break;
			case VKERR_USER_ON_BLACKLIST:
				MsgPopup(param->hContact, TranslateT("Cannot add this user to friends as you put him on blacklist"), TranslateT("Error"), true);
				break;
			}
		}
	}

	if (param && (!pReq->bNeedsRestart || m_bTerminated)) {
		delete param;
		pReq->pUserInfo = nullptr;
	}
}

int CVkProto::Authorize(MEVENT hDbEvent)
{
	debugLogA("CVkProto::Authorize");
	if (!IsOnline())
		return 1;
	MCONTACT hContact = MContactFromDbEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	return AuthRequest(hContact, nullptr);
}

int CVkProto::AuthDeny(MEVENT hDbEvent, const wchar_t*)
{
	debugLogA("CVkProto::AuthDeny");
	if (!IsOnline())
		return 1;
	MCONTACT hContact = MContactFromDbEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	return SvcDeleteFriend(hContact, (LPARAM)true);
}

int CVkProto::UserIsTyping(MCONTACT hContact, int type)
{
	debugLogA("CVkProto::UserIsTyping");
	if (PROTOTYPE_SELFTYPING_ON == type) {
		LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
		if (userID == VK_INVALID_USER || !IsOnline() || userID == VK_FEED_USER)
			return 1;

		if (!IsEmpty(ptrW(db_get_wsa(hContact, m_szModuleName, "Deactivated"))))
			return 1;

		if (m_vkOptions.iMarkMessageReadOn == MarkMsgReadOn::markOnTyping)
			MarkMessagesRead(hContact);

		if (m_iStatus == ID_STATUS_INVISIBLE)
			return 1;

		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.setActivity.json", true, &CVkProto::OnReceiveSmth, AsyncHttpRequest::rpLow)
			<< INT_PARAM("user_id", userID)
			<< CHAR_PARAM("type", "typing"));
		return 0;
	}
	return 1;
}

int CVkProto::GetInfo(MCONTACT hContact, int)
{
	debugLogA("CVkProto::GetInfo");
	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	if (userID == VK_INVALID_USER || userID == VK_FEED_USER)
		return 1;
	RetrieveUserInfo(userID);
	return 0;
}

void CVkProto::OnContactDeleted(MCONTACT hContact)
{
	ptrW pwszNick(db_get_wsa(hContact, m_szModuleName, "Nick"));
	debugLogW(L"CVkProto::OnContactDeleted %s", pwszNick.get());

	if (!Contact::OnList(hContact) || getBool(hContact, "SilentDelete") || isChatRoom((MCONTACT)hContact))
		return;

	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	if (userID == VK_INVALID_USER || userID == VK_FEED_USER)
		return;

	CONTACTDELETE_FORM_PARAMS *param = new CONTACTDELETE_FORM_PARAMS(pwszNick, true, !getBool(hContact, "Auth", true), true);
	CVkContactDeleteForm dlg(this, param);
	dlg.DoModal();

	debugLogW(L"CVkProto::OnContactDeleted %s DeleteDialog=%d DeleteFromFriendlist=%d", pwszNick.get(), param->bDeleteDialog,  param->bDeleteFromFriendlist);
	if (!(param->bDeleteDialog || param->bDeleteFromFriendlist))
		return;

	CMStringA code(FORMAT, "var userID=\"%d\";", userID);

	if (param->bDeleteDialog)
		code += "API.messages.deleteConversation({\"peer_id\":userID});";

	if (param->bDeleteFromFriendlist)
		code += "API.friends.delete({\"user_id\":userID});";

	code += "return 1;";

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveSmth)
		<< CHAR_PARAM("code", code.c_str()));
}