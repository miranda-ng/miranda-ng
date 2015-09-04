/*
Copyright (c) 2013-15 Miranda NG project (http://miranda-ng.org)

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

static int sttCompareProtocols(const CVkProto *p1, const CVkProto *p2)
{
	return mir_tstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

static int sttCompareAsyncHttpRequest(const AsyncHttpRequest *p1, const AsyncHttpRequest *p2)
{
	if (p1->m_priority == p2->m_priority)
		return (int)p1->m_reqNum - (int)p2->m_reqNum;
	return (int)p2->m_priority - (int)p1->m_priority;
}

LIST<CVkProto> vk_Instances(1, sttCompareProtocols);
mir_cs csInstances;
static COLORREF sttColors[] = { 0, 1, 2, 3, 4, 5, 6 };

CVkProto::CVkProto(const char *szModuleName, const TCHAR *ptszUserName) :
	PROTO<CVkProto>(szModuleName, ptszUserName),
	m_arRequestsQueue(10, sttCompareAsyncHttpRequest),
	m_sendIds(3, PtrKeySortT),
	m_incIds(3, PtrKeySortT),
	m_cookies(5),
	m_msgId(1),
	m_chats(1, NumericKeySortT)
{
	InitQueue();

	CreateProtoService(PS_CREATEACCMGRUI, &CVkProto::SvcCreateAccMgrUI);
	CreateProtoService(PS_GETAVATARINFO, &CVkProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &CVkProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &CVkProto::SvcGetMyAvatar);
	CreateProtoService(PS_SET_LISTENINGTO, &CVkProto::SvcSetListeningTo);

	HookProtoEvent(ME_OPT_INITIALISE, &CVkProto::OnOptionsInit);

	TCHAR descr[512];
	mir_sntprintf(descr, TranslateT("%s server connection"), m_tszUserName);

	NETLIBUSER nlu = {sizeof(nlu)};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = m_szModuleName;
	nlu.ptszDescriptiveName = descr;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	m_defaultGroup = getTStringA("ProtoGroup");
	if (m_defaultGroup == NULL)
		m_defaultGroup = mir_tstrdup(_T("VKontakte"));
	Clist_CreateGroup(NULL, m_defaultGroup);

	m_bServerDelivery = getBool("ServerDelivery", true);
	m_bHideChats = getBool("HideChats", true);
	m_bMesAsUnread = getBool("MesAsUnread");
	m_bUseLocalTime = getBool("UseLocalTime");
	m_bReportAbuse = getBool("ReportAbuseOnBanUser");
	m_bClearServerHistory = getBool("ClearServerHistoryOnBanUser");
	m_bRemoveFromFrendlist = getBool("RemoveFromFrendlistOnBanUser");
	m_bRemoveFromClist = getBool("RemoveFromClistOnBanUser");
	m_bPopUpSyncHistory = getBool("PopUpSyncHistory");
	m_iMarkMessageReadOn = (MarkMsgReadOn)getByte("MarkMessageReadOn", markOnRead);
	m_bStikersAsSmyles = getBool("StikersAsSmyles");
	m_bUserForceOnlineOnActivity = getBool("UserForceOnlineOnActivity");
	m_iMusicSendMetod = (MusicSendMetod)getByte("MusicSendMetod", sendBroadcastOnly);
	m_iSyncHistoryMetod = (SyncHistoryMetod)getByte("SyncHistoryMetod", syncOff);
	CMStringA szListeningTo(m_szModuleName);
	szListeningTo += "Enabled";
	db_set_b(NULL, "ListeningTo", szListeningTo, m_iMusicSendMetod == 0 ? 0 : 1);
	
	m_bNewsEnabled = getBool("NewsEnabled");
	m_iMaxLoadNewsPhoto = getByte("MaxLoadNewsPhoto", 5);
	m_bNotificationsEnabled = getBool("NotificationsEnabled");
	m_bNotificationsMarkAsViewed = getBool("NotificationsMarkAsViewed", true);
	m_bSpecialContactAlwaysEnabled = getBool("SpecialContactAlwaysEnabled");
	m_iNewsInterval = getDword("NewsInterval", 15);
	m_iNotificationsInterval = getDword("NotificationsInterval", 1);
	m_iIMGBBCSupport = (IMGBBCSypport)getByte("IMGBBCSupport", imgNo);
	m_iBBCForNews = (BBCSupport)getByte("BBCForNews", bbcBasic);
	m_iBBCForAttachments = (BBCSupport)getByte("BBCForAttachments", bbcBasic);
	m_bUseBBCOnAttacmentsAsNews = getBool("UseBBCOnAttacmentsAsNews", true);
	m_bNewsAutoClearHistory = getBool("NewsAutoClearHistory");
	m_iNewsAutoClearHistoryInterval = getDword("NewsAutoClearHistoryInterval", 60*60*24*3);

	m_bNewsFilterPosts = getBool("NewsFilterPosts", true);
	m_bNewsFilterPhotos = getBool("NewsFilterPhotos", true);
	m_bNewsFilterTags = getBool("NewsFilterTags", true);
	m_bNewsFilterWallPhotos = getBool("NewsFilterWallPhotos", true);
	m_bNewsSourceFriends = getBool("NewsSourceFriends", true);
	m_bNewsSourceGroups = getBool("NewsSourceGroups", true);
	m_bNewsSourcePages = getBool("NewsSourcePages", true);
	m_bNewsSourceFollowing = getBool("NewsSourceFollowing", true);
	m_bNewsSourceIncludeBanned = getBool("NewsSourceIncludeBanned");
	m_bNewsSourceNoReposts = getBool("NewsSourceNoReposts");

	m_bNotificationFilterComments = getBool("NotificationFilterComments", true);
	m_bNotificationFilterLikes = getBool("NotificationFilterLikes", true);
	m_bNotificationFilterReposts = getBool("NotificationFilterReposts", true);
	m_bNotificationFilterMentions = getBool("NotificationFilterMentions", true);
	m_bNotificationFilterInvites = getBool("NotificationFilterInvites", true);

	m_bUseNonStandardUrlEncode = getBool("UseNonStandardUrlEncode", true);
	m_iInvisibleInterval = getDword("InvisibleInterval", 10);

	m_bShortenLinksForAudio = getBool("ShortenLinksForAudio", true);

	m_bSplitFormatFwdMsg = getBool("SplitFormatFwdMsg", true);
	
	m_bSetBroadcast = false;
	m_bNeedSendOnline = false;
	delSetting("InviteGroupIds");

	// Set all contacts offline -- in case we crashed
	SetAllContactStatuses(ID_STATUS_OFFLINE);
	{
		mir_cslock lck(csInstances);
		vk_Instances.insert(this);
	}
}

CVkProto::~CVkProto()
{
	debugLogA("CVkProto::~CVkProto");
	Netlib_CloseHandle(m_hNetlibUser); 
	m_hNetlibUser = NULL;
	UninitQueue();
	UnInitMenus();
	if (m_hPopupClassError)
		Popup_UnregisterClass(m_hPopupClassError);
	if (m_hPopupClassNotification)
		Popup_UnregisterClass(m_hPopupClassNotification);
	{
		mir_cslock lck(csInstances);
		vk_Instances.remove(this);
	}
}

int CVkProto::OnModulesLoaded(WPARAM, LPARAM)
{
	// Chats
	GCREGISTER gcr = { sizeof(gcr) };
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	gcr.nColors = _countof(sttColors);
	gcr.pColors = sttColors;
	CallServiceSync(MS_GC_REGISTER, NULL, (LPARAM)&gcr);
	CreateProtoService(PS_LEAVECHAT, &CVkProto::OnLeaveChat);
	CreateProtoService(PS_JOINCHAT, &CVkProto::OnJoinChat);
	HookProtoEvent(ME_GC_EVENT, &CVkProto::OnChatEvent);
	HookProtoEvent(ME_GC_BUILDMENU, &CVkProto::OnGcMenuHook);
	// Other hooks
	HookProtoEvent(ME_MSG_WINDOWEVENT, &CVkProto::OnProcessSrmmEvent);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &CVkProto::OnDbEventRead);
	HookProtoEvent(ME_DB_CONTACT_SETTINGCHANGED, &CVkProto::OnDbSettingChanged);

	InitPopups();
	InitMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Menu support 

void CVkProto::InitMenus()
{
	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU, &CVkProto::OnPreBuildContactMenu);

	//Contact Menu Services
	CreateProtoService(PS_GETSERVERHISTORYLAST1DAY, &CVkProto::SvcGetServerHistoryLast1Day);
	CreateProtoService(PS_GETSERVERHISTORYLAST3DAY, &CVkProto::SvcGetServerHistoryLast3Day);
	CreateProtoService(PS_GETSERVERHISTORYLAST7DAY, &CVkProto::SvcGetServerHistoryLast7Day);
	CreateProtoService(PS_GETSERVERHISTORYLAST30DAY, &CVkProto::SvcGetServerHistoryLast30Day);
	CreateProtoService(PS_GETSERVERHISTORYLAST90DAY, &CVkProto::SvcGetServerHistoryLast90Day);
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
	CreateProtoService(PS_SETSTATUSMSG, &CVkProto::SvcSetStatusMsg);
	CreateProtoService(PS_WALLPOST, &CVkProto::SvcWallPost);
		
	CMenuItem mi;
	mi.root = Menu_GetProtocolRoot(this);

	// Proto menu
	mi.pszService = PS_CREATECHAT;
	mi.position = 10009 + PMI_CREATECHAT;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_JOIN);
	mi.name.a = LPGEN("Create new chat");
	SET_UID(mi, 0xc217748d, 0x7218, 0x4a62, 0xab, 0x37, 0x9, 0x58, 0x6a, 0x88, 0x71, 0x3e);
	g_hProtoMenuItems[PMI_CREATECHAT] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_SETSTATUSMSG;
	mi.position = 10009 + PMI_SETSTATUSMSG;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_STATUS));
	mi.name.a = LPGEN("Status message");
	SET_UID(mi, 0x696bd932, 0xeee4, 0x4c4c, 0xa6, 0xf8, 0xb3, 0x72, 0xdf, 0xc6, 0xfa, 0xd1);
	g_hProtoMenuItems[PMI_SETSTATUSMSG] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_WALLPOST;
	mi.position = 10009 + PMI_WALLPOST;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_WALL));
	mi.name.a = LPGEN("Send message to my wall");
	SET_UID(mi, 0x50a80044, 0xdddd, 0x47e6, 0x9e, 0x90, 0x32, 0x7b, 0x88, 0x13, 0x21, 0x4e);
	g_hProtoMenuItems[PMI_WALLPOST] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_LOADVKNEWS;
	mi.position = 10009 + PMI_LOADVKNEWS;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_NOTIFICATION));
	mi.name.a = LPGEN("Load news from VK");
	SET_UID(mi, 0x7c449456, 0xb731, 0x48cc, 0x9c, 0x4e, 0x20, 0xe4, 0x66, 0x7a, 0x16, 0x23);
	g_hProtoMenuItems[PMI_LOADVKNEWS] = Menu_AddProtoMenuItem(&mi, m_szModuleName);
	
	mi.pszService = PS_VISITPROFILE;
	mi.position = 10009 + PMI_VISITPROFILE;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_VISITPROFILE));
	mi.name.a = LPGEN("Visit profile");
	SET_UID(mi, 0x9550515e, 0x2a45, 0x4913, 0x95, 0x1a, 0x1e, 0xfa, 0x7, 0xc6, 0x2d, 0x60);
	g_hProtoMenuItems[PMI_VISITPROFILE] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Contact Menu Items
	mi.root = NULL;
	mi.flags = CMIF_TCHAR;

	mi.pszService = PS_VISITPROFILE;
	mi.position = -200001000 + CMI_VISITPROFILE;
	mi.name.t = LPGENT("Visit profile");
	SET_UID(mi, 0x828cc50e, 0x398d, 0x43a2, 0xbf, 0xd3, 0xa9, 0x96, 0x47, 0x9d, 0x52, 0xff);
	g_hContactMenuItems[CMI_VISITPROFILE] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_WALLPOST;
	mi.position = -200001000 + CMI_WALLPOST;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_WALL));
	mi.name.t = LPGENT("Send message to user\'s wall");
	SET_UID(mi, 0xd8841aaf, 0x15f6, 0x4be9, 0x9f, 0x4f, 0x16, 0xa9, 0x47, 0x6a, 0x19, 0x81);
	g_hContactMenuItems[CMI_WALLPOST] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_ADDASFRIEND;
	mi.position = -200001000 + CMI_ADDASFRIEND;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_FRIENDADD));
	mi.name.t = LPGENT("Add as friend");
	SET_UID(mi, 0xf11b9a7f, 0x569, 0x4023, 0xb0, 0xd6, 0xa3, 0x16, 0xf6, 0xd4, 0xfb, 0xb5);
	g_hContactMenuItems[CMI_ADDASFRIEND] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_DELETEFRIEND;
	mi.position = -200001000 + CMI_DELETEFRIEND;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_FRIENDDEL));
	mi.name.t = LPGENT("Delete from friend list");
	SET_UID(mi, 0x1e26514, 0x854f, 0x4e60, 0x8c, 0xf8, 0xab, 0xaa, 0xe0, 0xc3, 0xa5, 0xa7);
	g_hContactMenuItems[CMI_DELETEFRIEND] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_BANUSER;
	mi.position = -200001000 + CMI_BANUSER;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_BAN));
	mi.name.t = LPGENT("Ban user");
	SET_UID(mi, 0x7ba06bab, 0xf770, 0x4938, 0x9c, 0x76, 0xef, 0x40, 0xbc, 0x55, 0x0, 0x9b);
	g_hContactMenuItems[CMI_BANUSER] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_REPORTABUSE;
	mi.position = -200001000 + CMI_REPORTABUSE;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_ABUSE));
	mi.name.t = LPGENT("Report abuse");
	SET_UID(mi, 0x56454cb9, 0xd80, 0x4050, 0xbe, 0xfc, 0x2c, 0xf6, 0x10, 0x2a, 0x7d, 0x19);
	g_hContactMenuItems[CMI_REPORTABUSE] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_DESTROYKICKCHAT;
	mi.position = -200001000 + CMI_DESTROYKICKCHAT;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_FRIENDDEL));
	mi.name.t = LPGENT("Destroy room");
	SET_UID(mi, 0x4fa6e75a, 0x30cd, 0x4482, 0xae, 0x8f, 0x0, 0x38, 0xd0, 0x17, 0x33, 0xcd);
	g_hContactMenuItems[CMI_DESTROYKICKCHAT] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_OPENBROADCAST;
	mi.position = -200001000 + CMI_OPENBROADCAST;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_BROADCAST));
	mi.name.t = LPGENT("Open broadcast");
	SET_UID(mi, 0x85251a06, 0xf734, 0x4985, 0x8c, 0x36, 0x6f, 0x66, 0x46, 0xf9, 0xa0, 0x10);
	g_hContactMenuItems[CMI_OPENBROADCAST] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_LOADVKNEWS;
	mi.position = -200001000 + CMI_LOADVKNEWS;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_NOTIFICATION));
	mi.name.t = LPGENT("Load news from VK");
	SET_UID(mi, 0xe1f6888b, 0x21ae, 0x409f, 0x82, 0xa2, 0x7b, 0x72, 0xef, 0x47, 0x9, 0xc0);
	g_hContactMenuItems[CMI_LOADVKNEWS] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	// Sync history menu
	mi.pszService = PS_GETSERVERHISTORY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_HISTORY));
	mi.name.t = LPGENT("Reload messages from vk.com...");
	SET_UID(mi, 0xc6b59e9f, 0x5250, 0x4146, 0xb6, 0xf3, 0x2d, 0xe1, 0x4, 0x3b, 0x95, 0xf5);
	g_hContactMenuItems[CMI_GETSERVERHISTORY] = Menu_AddContactMenuItem(&mi, m_szModuleName);
		
	mi.root = g_hContactMenuItems[CMI_GETSERVERHISTORY];

	mi.pszService = PS_GETSERVERHISTORYLAST1DAY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY + 100 + CHMI_GETSERVERHISTORYLAST1DAY;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_HISTORY));
	mi.name.t = LPGENT("for last 1 day");
	SET_UID(mi, 0x508dce88, 0x1a9a, 0x4dd7, 0x90, 0xf4, 0x41, 0x35, 0x7b, 0xc3, 0x17, 0xed);
	g_hContactHistoryMenuItems[CHMI_GETSERVERHISTORYLAST1DAY] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_GETSERVERHISTORYLAST3DAY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY + 100 + CHMI_GETSERVERHISTORYLAST3DAY;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_HISTORY));
	mi.name.t = LPGENT("for last 3 days");
	SET_UID(mi, 0x9a878764, 0x5bbf, 0x433a, 0xbd, 0x50, 0xa9, 0xb9, 0x16, 0x1f, 0x99, 0x29);
	g_hContactHistoryMenuItems[CHMI_GETSERVERHISTORYLAST3DAY] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_GETSERVERHISTORYLAST7DAY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY + 100 + CHMI_GETSERVERHISTORYLAST7DAY;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_HISTORY));
	mi.name.t = LPGENT("for last week");
	SET_UID(mi, 0xc6482460, 0xd280, 0x4596, 0x97, 0x4b, 0xf7, 0xfa, 0x6d, 0xe, 0xd5, 0xda);
	g_hContactHistoryMenuItems[CHMI_GETSERVERHISTORYLAST7DAY] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_GETSERVERHISTORYLAST30DAY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY + 100 + CHMI_GETSERVERHISTORYLAST30DAY;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_HISTORY));
	mi.name.t = LPGENT("for last 30 days");
	SET_UID(mi, 0xc48e8a9f, 0x2860, 0x4d5b, 0xa8, 0xdf, 0xb8, 0x3f, 0xdf, 0x7b, 0xa2, 0xba);
	g_hContactHistoryMenuItems[CHMI_GETSERVERHISTORYLAST30DAY] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_GETSERVERHISTORYLAST90DAY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY + 100 + CHMI_GETSERVERHISTORYLAST90DAY;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_HISTORY));
	mi.name.t = LPGENT("for last 90 days");
	SET_UID(mi, 0xd8e30530, 0xa585, 0x4672, 0xa6, 0x39, 0x18, 0xc9, 0xc9, 0xcb, 0xc7, 0x7d);
	g_hContactHistoryMenuItems[CHMI_GETSERVERHISTORYLAST90DAY] = Menu_AddContactMenuItem(&mi, m_szModuleName);

	mi.pszService = PS_GETALLSERVERHISTORY;
	mi.position = -200001000 + CMI_GETSERVERHISTORY + 100 + CHMI_GETALLSERVERHISTORY;
	mi.hIcolibItem = IcoLib_GetIconByHandle(GetIconHandle(IDI_HISTORY));
	mi.name.t = LPGENT("for all time");
	SET_UID(mi, 0xaee3d02b, 0x3667, 0x47c8, 0x9f, 0x43, 0x14, 0xb7, 0xab, 0x52, 0x14, 0x94);
	g_hContactHistoryMenuItems[CHMI_GETALLSERVERHISTORY] = Menu_AddContactMenuItem(&mi, m_szModuleName);
}

int CVkProto::OnPreBuildContactMenu(WPARAM hContact, LPARAM)
{
	LONG userID = getDword(hContact, "ID", -1);
	bool bisFriend = (getBool(hContact, "Auth", true) == 0);
	bool bisBroadcast = !(IsEmpty(ptrT(db_get_tsa(hContact, m_szModuleName, "AudioUrl"))));
	Menu_ShowItem(g_hContactMenuItems[CMI_VISITPROFILE], !isChatRoom(hContact) && userID != VK_FEED_USER);
	Menu_ShowItem(g_hContactMenuItems[CMI_WALLPOST], !isChatRoom(hContact));
	Menu_ShowItem(g_hContactMenuItems[CMI_ADDASFRIEND], !bisFriend && !isChatRoom(hContact) && userID != VK_FEED_USER);
	Menu_ShowItem(g_hContactMenuItems[CMI_DELETEFRIEND], bisFriend && userID != VK_FEED_USER);
	Menu_ShowItem(g_hContactMenuItems[CMI_BANUSER], !isChatRoom(hContact) && userID != VK_FEED_USER);
	Menu_ShowItem(g_hContactMenuItems[CMI_REPORTABUSE], !isChatRoom(hContact) && userID != VK_FEED_USER);
	Menu_ShowItem(g_hContactMenuItems[CMI_DESTROYKICKCHAT], isChatRoom(hContact) && getBool(hContact, "off"));
	Menu_ShowItem(g_hContactMenuItems[CMI_OPENBROADCAST], !isChatRoom(hContact) && bisBroadcast);
	Menu_ShowItem(g_hContactMenuItems[CMI_GETSERVERHISTORY], !isChatRoom(hContact) && userID != VK_FEED_USER);
	Menu_ShowItem(g_hContactMenuItems[CMI_LOADVKNEWS], userID == VK_FEED_USER);
	for (int i = 0; i < CHMI_COUNT; i++)
		Menu_ShowItem(g_hContactHistoryMenuItems[i], !isChatRoom(hContact) && userID != VK_FEED_USER);
	return 0;
}

void CVkProto::UnInitMenus()
{
	debugLogA("CVkProto::UnInitMenus");
	for (int i = 0; i < PMI_COUNT; i++)
		Menu_RemoveItem(g_hProtoMenuItems[i]);
	
	for (int i = 0; i < CHMI_COUNT; i++)
		Menu_RemoveItem(g_hContactHistoryMenuItems[i]);

	for (int i = 0; i < CMI_COUNT; i++)
		Menu_RemoveItem(g_hContactMenuItems[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////
// PopUp support 

void CVkProto::InitPopups(void)
{
	TCHAR desc[256];
	char name[256];
	POPUPCLASS ppc = { sizeof(ppc) };
	ppc.flags = PCF_TCHAR;

	mir_sntprintf(desc, _T("%s %s"), m_tszUserName, TranslateT("Errors"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Error");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = Skin_LoadIcon(SKINICON_ERROR);
	ppc.colorBack = RGB(191, 0, 0); //Red
	ppc.colorText = RGB(255, 245, 225); //Yellow
	ppc.iSeconds = 60;
	m_hPopupClassError = Popup_RegisterClass(&ppc);

	mir_sntprintf(desc, _T("%s %s"), m_tszUserName, TranslateT("Notification"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Notification");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = IcoLib_GetIconByHandle(GetIconHandle(IDI_NOTIFICATION));
	ppc.colorBack = RGB(190, 225, 255); //Blue
	ppc.colorText = RGB(255, 255, 255); //White
	ppc.iSeconds = 4;
	m_hPopupClassNotification = Popup_RegisterClass(&ppc);
}

void CVkProto::MsgPopup(MCONTACT hContact, const TCHAR *szMsg, const TCHAR *szTitle, bool err)
{
	if (ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {
		char name[256];

		POPUPDATACLASS ppd = { sizeof(ppd) };
		ppd.ptszTitle = szTitle;
		ppd.ptszText = szMsg;
		ppd.pszClassName = name;
		ppd.hContact = hContact;
		mir_snprintf(name, "%s_%s", m_szModuleName, err ? "Error" : "Notification");

		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&ppd);
	}
	else {
		DWORD mtype = MB_OK | MB_SETFOREGROUND | MB_ICONSTOP;
		MessageBox(NULL, szMsg, szTitle, mtype);
	}
}

//////////////////////////////////////////////////////////////////////////////

int CVkProto::OnPreShutdown(WPARAM, LPARAM)
{
	debugLogA("CVkProto::OnPreShutdown");
		
	m_bTerminated = true;
	SetEvent(m_evRequestsQueue);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

DWORD_PTR CVkProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_CHAT | PF1_SERVERCLIST | PF1_AUTHREQ | PF1_BASICSEARCH | PF1_SEARCHBYNAME | PF1_SEARCHBYEMAIL | PF1_MODEMSG | PF1_FILESEND | PF1_FILERESUME;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_ONTHEPHONE | PF2_IDLE;

	case PFLAGNUM_3:
		return PF2_ONLINE;

	case PFLAGNUM_4:
		return PF4_AVATARS | PF4_SUPPORTTYPING | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE | PF4_OFFLINEFILES | PF4_READNOTIFY;

	case PFLAGNUM_5:
		return PF2_ONTHEPHONE;

	case PFLAG_MAXLENOFMESSAGE:
		return 4096;

	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)"VK ID";

	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"ID";
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int CVkProto::OnEvent(PROTOEVENTTYPE event, WPARAM wParam, LPARAM lParam)
{
	switch (event) {
	case EV_PROTO_ONLOAD:
		return OnModulesLoaded(wParam,lParam);

	case EV_PROTO_ONEXIT:
		return OnPreShutdown(wParam,lParam);

	case EV_PROTO_ONOPTIONS:
		return OnOptionsInit(wParam,lParam);
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////////

MCONTACT CVkProto::AddToList(int, PROTOSEARCHRESULT* psr)
{
	debugLogA("CVkProto::AddToList");

	int uid = _ttoi(psr->id.t);
	if (!uid)
		return NULL;

	MCONTACT hContact = FindUser(uid, true);
	RetrieveUserInfo(uid);
	return hContact;
}

int CVkProto::AuthRequest(MCONTACT hContact,const TCHAR* message)
{
	debugLogA("CVkProto::AuthRequest");
	if (!IsOnline())
		return 1;

	LONG userID = getDword(hContact, "ID", -1);
	if (userID == -1 || !hContact || userID == VK_FEED_USER)
		return 1;
	
	TCHAR msg[501] = {0};
	if (message)
		_tcsncpy_s(msg, 500, message, _TRUNCATE);

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/friends.add.json", true, &CVkProto::OnReceiveAuthRequest)
		<< INT_PARAM("user_id", userID)
		<< TCHAR_PARAM("text", msg)
		<< VER_API)->pUserInfo = new CVkSendMsgParam(hContact);

	return 0;
}

void CVkProto::OnReceiveAuthRequest(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveAuthRequest %d", reply->resultCode);
	CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;
	if (reply->resultCode == 200) {
		JSONNode jnRoot;
		const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
		if (jnResponse) {
			int iRet = jnResponse.as_int();
			setByte(param->hContact, "Auth", 0);
			if (iRet == 2) {
				CMString msg,			
					tszNick(ptrT(db_get_tsa(param->hContact, m_szModuleName, "Nick")));
				if (tszNick.IsEmpty())
					tszNick = TranslateT("(Unknown contact)");
				msg.AppendFormat(TranslateT("User %s added as friend"), tszNick);
				MsgPopup(param->hContact, msg, tszNick);
			}
		} 
		else {
			switch (param->iCount) {
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
	if (!pReq->bNeedsRestart)
		delete param;
}

int CVkProto::Authorize(MEVENT hDbEvent)
{
	debugLogA("CVkProto::Authorize");
	if (!IsOnline())
		return 1;
	MCONTACT hContact = MContactFromDbEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;
	
	return AuthRequest(hContact, NULL);
}

int CVkProto::AuthDeny(MEVENT hDbEvent, const TCHAR*)
{
	debugLogA("CVkProto::AuthDeny");
	if (!IsOnline())
		return 1;
	MCONTACT hContact = MContactFromDbEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	return SvcDeleteFriend(hContact,(LPARAM)true);
}

int CVkProto::UserIsTyping(MCONTACT hContact, int type)
{
	debugLogA("CVkProto::UserIsTyping");
	if (PROTOTYPE_SELFTYPING_ON == type) {
		LONG userID = getDword(hContact, "ID", -1);
		if (userID == -1 || !IsOnline() || userID == VK_FEED_USER)
			return 1;
		
		if (m_iMarkMessageReadOn == markOnTyping)
			MarkMessagesRead(hContact);
		
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.setActivity.json", true, &CVkProto::OnReceiveSmth, AsyncHttpRequest::rpLow)
			<< INT_PARAM("user_id", userID) 
			<< CHAR_PARAM("type", "typing")
			<< VER_API);
		return 0;
	}
	return 1;
}

int CVkProto::GetInfo(MCONTACT hContact, int)
{
	debugLogA("CVkProto::GetInfo");
	LONG userID = getDword(hContact, "ID", -1);
	if (userID == -1 || userID == VK_FEED_USER)
		return 1;
	RetrieveUserInfo(userID);
	return 0;
}