#include "stdafx.h"

CMPlugin g_plugin;

HANDLE hExtraXStatus;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {68F5A030-BA32-48EC-9507-5C2FBDEA5217}
	{ 0x68f5a030, 0xba32, 0x48ec, { 0x95, 0x7, 0x5c, 0x2f, 0xbd, 0xea, 0x52, 0x17 }}
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CSteamProto>("STEAM", pluginInfoEx)
{
	SetUniqueId(DBKEY_STEAM_ID);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

void CMPlugin::InitSteamServices()
{
	// message descriptors map
	messages[EMsg::Multi] = &cmsg_multi__descriptor;
	messages[EMsg::ClientHeartBeat] = &cmsg_client_heart_beat__descriptor;
	messages[EMsg::ClientHello] = &cmsg_client_hello__descriptor;
	messages[EMsg::ClientLogon] = &cmsg_client_logon__descriptor;
	messages[EMsg::ClientLogOnResponse] = &cmsg_client_logon_response__descriptor;
	messages[EMsg::ClientLogOff] = &cmsg_client_log_off__descriptor;
	messages[EMsg::ClientLoggedOff] = &cmsg_client_logged_off__descriptor;
	messages[EMsg::ClientUpdateMachineAuth] = &cmsg_client_update_machine_auth__descriptor;
	messages[EMsg::ClientUpdateMachineAuthResponse] = &cmsg_client_update_machine_auth_response__descriptor;
	messages[EMsg::ClientNewLoginKey] = &cmsg_client_new_login_key__descriptor;
	messages[EMsg::ClientNewLoginKeyAccepted] = &cmsg_client_new_login_key_accepted__descriptor;
	messages[EMsg::ClientRequestWebAPIAuthenticateUserNonceResponse] = &cmsg_client_request_web_apiauthenticate_user_nonce_response__descriptor;
	messages[EMsg::ClientCMList] = &cmsg_client_cmlist__descriptor;
	messages[EMsg::ClientItemAnnouncements] = &cmsg_client_item_announcements__descriptor;
	messages[EMsg::ClientRequestItemAnnouncements] = &cmsg_client_request_item_announcements__descriptor;
	messages[EMsg::ClientCommentNotifications] = &cmsg_client_comment_notifications__descriptor;
	messages[EMsg::ClientRequestCommentNotifications] = &cmsg_client_request_comment_notifications__descriptor;
	messages[EMsg::ClientUserNotifications] = &cmsg_client_user_notifications__descriptor;
	messages[EMsg::ClientFSOfflineMessageNotification] = &cmsg_client_offline_message_notification__descriptor;
	messages[EMsg::ClientFSRequestOfflineMessageCount] = &cmsg_client_request_offline_message_count__descriptor;
	messages[EMsg::ClientGamesPlayed] = &cmsg_client_games_played__descriptor;
	messages[EMsg::ClientGamesPlayedWithDataBlob] = &cmsg_client_games_played__descriptor;
	messages[EMsg::ClientAccountInfo] = &cmsg_client_account_info__descriptor;
	// messages[EMsg::ClientEmailAddrInfo] = &cmsg_Client__EmailAddrInfo;
	messages[EMsg::ClientIsLimitedAccount] = &cmsg_client_is_limited_account__descriptor;
	messages[EMsg::ClientWalletInfoUpdate] = &cmsg_client_wallet_info_update__descriptor;
	messages[EMsg::ClientLicenseList] = &cmsg_client_license_list__descriptor;
	// messages[EMsg::ClientGMSServerQuery] = &cmsg_Client_GMSServerQuery;
	// messages[EMsg::GMSClientServerQueryResponse] = &cmsg_GMSClientServerQueryResponse;
	messages[EMsg::ClientPICSChangesSinceResponse] = &cmsg_client_picschanges_since_response__descriptor;
	messages[EMsg::ClientPICSProductInfoResponse] = &cmsg_client_picsproduct_info_response__descriptor;
	messages[EMsg::ClientPICSAccessTokenResponse] = &cmsg_client_picsaccess_token_response__descriptor;
	messages[EMsg::EconTrading_InitiateTradeResponse] = &cmsg_trading__initiate_trade_request__descriptor;
	messages[EMsg::EconTrading_InitiateTradeResult] = &cmsg_trading__initiate_trade_response__descriptor;
	messages[EMsg::EconTrading_StartSession] = &cmsg_trading__start_session__descriptor;
	messages[EMsg::ClientChangeStatus] = &cmsg_client_change_status__descriptor;
	messages[EMsg::ClientAddFriendResponse] = &cmsg_client_add_friend_response__descriptor;
	messages[EMsg::ClientRemoveFriend] = &cmsg_client_remove_friend__descriptor;
	messages[EMsg::ClientFSGetFriendsSteamLevels] = &cmsg_client_fsget_friends_steam_levels__descriptor;
	messages[EMsg::ClientFSGetFriendsSteamLevelsResponse] = &cmsg_client_fsget_friends_steam_levels_response__descriptor;
	messages[EMsg::ClientPersonaState] = &cmsg_client_persona_state__descriptor;
	messages[EMsg::ClientClanState] = &cmsg_client_clan_state__descriptor;
	messages[EMsg::ClientFriendsList] = &cmsg_client_friends_list__descriptor;
	messages[EMsg::ClientRequestFriendData] = &cmsg_client_request_friend_data__descriptor;
	messages[EMsg::ClientFriendMsg] = &cmsg_client_friend_msg__descriptor;
	messages[EMsg::ClientChatInvite] = &cmsg_client_chat_invite__descriptor;
	messages[EMsg::ClientFriendMsgIncoming] = &cmsg_client_friend_msg_incoming__descriptor;
	messages[EMsg::ClientFriendMsgEchoToSender] = &cmsg_client_friend_msg_incoming__descriptor;
	messages[EMsg::ClientFSGetFriendMessageHistory] = &cmsg_client_chat_get_friend_message_history__descriptor;
	messages[EMsg::ClientFSGetFriendMessageHistoryResponse] = &cmsg_client_chat_get_friend_message_history_response__descriptor;
	messages[EMsg::ClientFriendsGroupsList] = &cmsg_client_friends_groups_list__descriptor;
	messages[EMsg::AMClientCreateFriendsGroup] = &cmsg_client_create_friends_group__descriptor;
	messages[EMsg::AMClientCreateFriendsGroupResponse] = &cmsg_client_create_friends_group_response__descriptor;
	messages[EMsg::AMClientDeleteFriendsGroup] = &cmsg_client_delete_friends_group__descriptor;
	messages[EMsg::AMClientDeleteFriendsGroupResponse] = &cmsg_client_delete_friends_group_response__descriptor;
	messages[EMsg::AMClientRenameFriendsGroup] = &cmsg_client_manage_friends_group__descriptor;
	messages[EMsg::AMClientRenameFriendsGroupResponse] = &cmsg_client_manage_friends_group_response__descriptor;
	messages[EMsg::AMClientAddFriendToGroup] = &cmsg_client_add_friend_to_group__descriptor;
	messages[EMsg::AMClientAddFriendToGroupResponse] = &cmsg_client_add_friend_to_group_response__descriptor;
	messages[EMsg::AMClientRemoveFriendFromGroup] = &cmsg_client_remove_friend_from_group__descriptor;
	messages[EMsg::AMClientRemoveFriendFromGroupResponse] = &cmsg_client_remove_friend_from_group_response__descriptor;
	messages[EMsg::ClientPlayerNicknameList] = &cmsg_client_player_nickname_list__descriptor;
	messages[EMsg::AMClientSetPlayerNickname] = &cmsg_client_set_player_nickname__descriptor;
	messages[EMsg::AMClientSetPlayerNicknameResponse] = &cmsg_client_set_player_nickname_response__descriptor;
	messages[EMsg::ClientRegisterKey] = &cmsg_client_register_key__descriptor;
	messages[EMsg::ClientPurchaseResponse] = &cmsg_client_purchase_response__descriptor;
	messages[EMsg::ClientRequestFreeLicense] = &cmsg_client_request_free_license__descriptor;
	messages[EMsg::ClientRequestFreeLicenseResponse] = &cmsg_client_request_free_license_response__descriptor;
	messages[EMsg::ClientGetNumberOfCurrentPlayersDP] = &cmsg_dpget_number_of_current_players__descriptor;
	messages[EMsg::ClientGetNumberOfCurrentPlayersDPResponse] = &cmsg_dpget_number_of_current_players_response__descriptor;
	messages[EMsg::ClientGetAppOwnershipTicketResponse] = &cmsg_client_get_app_ownership_ticket_response__descriptor;
	messages[EMsg::ClientGameConnectTokens] = &cmsg_client_game_connect_tokens__descriptor;
	messages[EMsg::ClientAuthList] = &cmsg_client_auth_list__descriptor;
	messages[EMsg::ClientAuthListAck] = &cmsg_client_auth_list_ack__descriptor;
	messages[EMsg::ClientTicketAuthComplete] = &cmsg_client_ticket_auth_complete__descriptor;
	messages[EMsg::ClientRequestEncryptedAppTicket] = &cmsg_client_request_encrypted_app_ticket__descriptor;
	messages[EMsg::ClientRequestEncryptedAppTicketResponse] = &cmsg_client_request_encrypted_app_ticket_response__descriptor;
	messages[EMsg::ClientCurrentUIMode] = &cmsg_client_uimode__descriptor;
	messages[EMsg::ClientVanityURLChangedNotification] = &cmsg_client_vanity_urlchanged_notification__descriptor;
	messages[EMsg::ClientAMGetPersonaNameHistory] = &cmsg_client_amget_persona_name_history__descriptor;
	messages[EMsg::ClientAMGetPersonaNameHistoryResponse] = &cmsg_client_amget_persona_name_history_response__descriptor;
	messages[EMsg::ClientGetCDNAuthToken] = &cmsg_client_get_cdnauth_token__descriptor;
	messages[EMsg::ClientGetCDNAuthTokenResponse] = &cmsg_client_get_cdnauth_token_response__descriptor;
	messages[EMsg::ClientKickPlayingSession] = &cmsg_client_kick_playing_session__descriptor;
	messages[EMsg::ClientPlayingSessionState] = &cmsg_client_playing_session_state__descriptor;
	messages[EMsg::ClientToGC] = &cmsg_gcclient__descriptor;
	messages[EMsg::ClientFromGC] = &cmsg_gcclient__descriptor;
	messages[EMsg::ClientRichPresenceUpload] = &cmsg_client_rich_presence_upload__descriptor;
	messages[EMsg::ClientRichPresenceRequest] = &cmsg_client_rich_presence_request__descriptor;
	messages[EMsg::ClientRichPresenceInfo] = &cmsg_client_rich_presence_info__descriptor;
	messages[EMsg::ClientGetEmoticonList] = &cmsg_client_get_emoticon_list__descriptor;
	messages[EMsg::ClientEmoticonList] = &cmsg_client_emoticon_list__descriptor;
	messages[EMsg::ClientGetAuthorizedDevicesResponse] = &cmsg_client_get_authorized_devices__descriptor;
	messages[EMsg::ClientAuthorizeLocalDeviceRequest] = &cmsg_client_authorize_local_device_request__descriptor;
	messages[EMsg::ClientAuthorizeLocalDeviceResponse] = &cmsg_client_authorize_local_device__descriptor;
	messages[EMsg::ClientDeauthorizeDeviceRequest] = &cmsg_client_deauthorize_device_request__descriptor;
	messages[EMsg::ClientDeauthorizeDevice] = &cmsg_client_deauthorize_device__descriptor;
	messages[EMsg::ClientUseLocalDeviceAuthorizations] = &cmsg_client_use_local_device_authorizations__descriptor;

	// message handlers
	messageHandlers[EMsg::ClientLoggedOff] = ServiceResponseHandler(&CSteamProto::OnClientLogoff);
	messageHandlers[EMsg::ClientLogOnResponse] = ServiceResponseHandler(&CSteamProto::OnClientLogon);
	messageHandlers[EMsg::ClientFriendsList] = ServiceResponseHandler(&CSteamProto::OnGotFriendList);
	messageHandlers[EMsg::ClientPersonaState] = ServiceResponseHandler(&CSteamProto::OnGotFriendInfo);
	messageHandlers[EMsg::ClientClanState] = ServiceResponseHandler(&CSteamProto::OnGotClanInfo);
	messageHandlers[EMsg::ClientPICSProductInfoResponse] = ServiceResponseHandler(&CSteamProto::OnGotAppInfo);

	// services from steammessages_auth.steamclient.proto
	services["Authentication"] = &authentication__descriptor;
	services["AuthenticationSupport"] = &authentication_support__descriptor;

	// services from steammessages_chat.steamclient.proto
	services["Chat"] = &chat__descriptor;
	services["ChatRoom"] = &chat_room__descriptor;
	services["ChatRoomClient"] = &chat_room_client__descriptor;

	// services from steammessages_deviceauth.steamclient.proto
	services["DeviceAuth"] = &device_auth__descriptor;

	// services from steammessages_friendmessages.steamclient.proto
	services["FriendMessages"] = &friend_messages__descriptor;
	services["FriendMessagesClient"] = &friend_messages_client__descriptor;

	// services from steammessages_notifications.steamclient.proto
	services["SteamNotificationClient"] = &steam_notification_client__descriptor;

	// static service handlers
	serviceHandlers[PollAuthSessionStatus] = ServiceResponseHandler(&CSteamProto::OnPollSession);
	serviceHandlers[GetPasswordRSAPublicKey] = ServiceResponseHandler(&CSteamProto::OnGotRsaKey);
	serviceHandlers[BeginAuthSessionViaCredentials] = ServiceResponseHandler(&CSteamProto::OnBeginSession);
	serviceHandlers[UpdateAuthSessionWithSteamGuardCode] = ServiceResponseHandler(&CSteamProto::OnGotConfirmationCode);

	serviceHandlers[FriendSendMessage] = ServiceResponseHandler(&CSteamProto::OnMessageSent);
	serviceHandlers[FriendGetActiveSessions] = ServiceResponseHandler(&CSteamProto::OnGotConversations);
	serviceHandlers[FriendGetRecentMessages] = ServiceResponseHandler(&CSteamProto::OnGotRecentMessages);
	serviceHandlers[FriendGetIncomingMessage] = ServiceResponseHandler(&CSteamProto::OnGotIncomingMessage);
	serviceHandlers[FriendNotifyAckMessage] = ServiceResponseHandler(&CSteamProto::OnGotMarkRead);

	serviceHandlers[GetMyChatRoomGroups] = ServiceResponseHandler(&CSteamProto::OnGetMyChats);
	serviceHandlers[GetChatHistory] = ServiceResponseHandler(&CSteamProto::OnGetChatHistory);
	serviceHandlers[DeleteChatMessage] = ServiceResponseHandler(&CSteamProto::OnDoNothing);
	serviceHandlers[NotifyIncomingChatMessage] = ServiceResponseHandler(&CSteamProto::OnGetChatMessage);
	serviceHandlers[NotifyAckChatMessageEcho] = ServiceResponseHandler(&CSteamProto::OnDoNothing);
	serviceHandlers[NotifyModifiedChatMessage] = ServiceResponseHandler(&CSteamProto::OnDoNothing);
	serviceHandlers[NotifyChatGroupUserStateChanged] = ServiceResponseHandler(&CSteamProto::OnChatChanged);

	serviceHandlers[GetOwnAuthorizedDevices] = ServiceResponseHandler(&CSteamProto::OnGotDeviceList);

	serviceHandlers[NotificationReceived] = ServiceResponseHandler(&CSteamProto::OnGotNotification);
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] = {
	{ LPGEN("Protocol icon"), "main", IDI_STEAM },
	{ LPGEN("Gaming icon"), "gaming", IDI_GAMING },
};

int CMPlugin::Load()
{
	InitSteamServices();

	// icons
	g_plugin.registerIcon("Protocols/" MODULENAME, iconList, MODULENAME);

	// extra statuses
	hExtraXStatus = ExtraIcon_RegisterIcolib("steam_game", LPGEN("Steam game"), g_plugin.getIconHandle(IDI_GAMING));

	HookEvent(ME_SKIN_ICONSCHANGED, OnReloadIcons);

	// menus
	CSteamProto::InitMenus();
	return 0;
}
