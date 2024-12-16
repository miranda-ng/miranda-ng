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
	// messages[EMsg::ClientUpdateMachineAuth] = &cmsg_Client__UpdateMachineAuth;
	// messages[EMsg::ClientUpdateMachineAuthResponse] = &cmsg_Client_UpdateMachineAuthResponse;
	messages[EMsg::ClientNewLoginKey] = &cmsg_client_new_login_key__descriptor;
	messages[EMsg::ClientNewLoginKeyAccepted] = &cmsg_client_new_login_key_accepted__descriptor;
	messages[EMsg::ClientRequestWebAPIAuthenticateUserNonceResponse] = &cmsg_client_request_web_apiauthenticate_user_nonce_response__descriptor;
	messages[EMsg::ClientCMList] = &cmsg_client_cmlist__descriptor;
	// messages[EMsg::ClientItemAnnouncements] = &cmsg_Client__ItemAnnouncements;
	// messages[EMsg::ClientRequestItemAnnouncements] = &cmsg_Client_RequestItemAnnouncements;
	// messages[EMsg::ClientCommentNotifications] = &cmsg_Client_CommentNotifications;
	// messages[EMsg::ClientRequestCommentNotifications] = &cmsg_Client_RequestCommentNotifications;
	// messages[EMsg::ClientUserNotifications] = &cmsg_Client_UserNotifications;
	// messages[EMsg::ClientFSOfflineMessageNotification] = &cmsg_Client__OfflineMessageNotification;
	// messages[EMsg::ClientFSRequestOfflineMessageCount] = &cmsg_Client_RequestOfflineMessageCount;
	messages[EMsg::ClientGamesPlayed] = &cmsg_client_games_played__descriptor;
	messages[EMsg::ClientGamesPlayedWithDataBlob] = &cmsg_client_games_played__descriptor;
	messages[EMsg::ClientAccountInfo] = &cmsg_client_account_info__descriptor;
	// messages[EMsg::ClientEmailAddrInfo] = &cmsg_Client__EmailAddrInfo;
	messages[EMsg::ClientIsLimitedAccount] = &cmsg_client_is_limited_account__descriptor;
	messages[EMsg::ClientWalletInfoUpdate] = &cmsg_client_wallet_info_update__descriptor;
	// messages[EMsg::ClientLicenseList] = &cmsg_Client__LicenseList;
	// messages[EMsg::ClientGMSServerQuery] = &cmsg_Client_GMSServerQuery;
	// messages[EMsg::GMSClientServerQueryResponse] = &cmsg_GMSClientServerQueryResponse;
	// messages[EMsg::ClientPICSChangesSinceResponse] = &cmsg_Client_PICSChangesSinceResponse;
	// messages[EMsg::ClientPICSProductInfoResponse] = &cmsg_Client_PICSProductInfoResponse;
	// messages[EMsg::ClientPICSAccessTokenResponse] = &cmsg_Client_PICSAccessTokenResponse;
	// messages[EMsg::EconTrading_InitiateTradeResponse] = &cmsg_Trading_InitiateTradeResponse;
	// messages[EMsg::EconTrading_InitiateTradeResult] = &cmsg_Trading_InitiateTradeResponse;
	// messages[EMsg::EconTrading_StartSession] = &cmsg_Trading_StartSession;
	messages[EMsg::ClientChangeStatus] = &cmsg_client_change_status__descriptor;
	messages[EMsg::ClientAddFriendResponse] = &cmsg_client_add_friend_response__descriptor;
	messages[EMsg::ClientRemoveFriend] = &cmsg_client_remove_friend__descriptor;
	// messages[EMsg::ClientFSGetFriendsSteamLevels] = &cmsg_Client_FSGetFriendsSteamLevels;
	// messages[EMsg::ClientFSGetFriendsSteamLevelsResponse] = &cmsg_Client_FSGetFriendsSteamLevelsResponse;
	messages[EMsg::ClientPersonaState] = &cmsg_client_persona_state__descriptor;
	messages[EMsg::ClientClanState] = &cmsg_client_clan_state__descriptor;
	messages[EMsg::ClientFriendsList] = &cmsg_client_friends_list__descriptor;
	messages[EMsg::ClientRequestFriendData] = &cmsg_client_request_friend_data__descriptor;
	messages[EMsg::ClientFriendMsg] = &cmsg_client_friend_msg__descriptor;
	messages[EMsg::ClientChatInvite] = &cmsg_client_chat_invite__descriptor;
	messages[EMsg::ClientFriendMsgIncoming] = &cmsg_client_friend_msg_incoming__descriptor;
	messages[EMsg::ClientFriendMsgEchoToSender] = &cmsg_client_friend_msg_incoming__descriptor;
	// messages[EMsg::ClientFSGetFriendMessageHistory] = &cmsg_Client_ChatGetFriendMessageHistory;
	// messages[EMsg::ClientFSGetFriendMessageHistoryResponse] = &cmsg_Client_ChatGetFriendMessageHistoryResponse;
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
	// messages[EMsg::ClientRegisterKey] = &cmsg_Client_RegisterKey;
	// messages[EMsg::ClientPurchaseResponse] = &cmsg_Client_PurchaseResponse;
	// messages[EMsg::ClientRequestFreeLicense] = &cmsg_Client_RequestFreeLicense;
	// messages[EMsg::ClientRequestFreeLicenseResponse] = &cmsg_Client_RequestFreeLicenseResponse;
	// messages[EMsg::ClientGetNumberOfCurrentPlayersDP] = &cmsg_DPGetNumberOfCurrentPlayers;
	// messages[EMsg::ClientGetNumberOfCurrentPlayersDPResponse] = &cmsg_DPGetNumberOfCurrentPlayersResponse;
	messages[EMsg::ClientGetAppOwnershipTicketResponse] = &cmsg_client_get_app_ownership_ticket_response__descriptor;
	messages[EMsg::ClientGameConnectTokens] = &cmsg_client_game_connect_tokens__descriptor;
	messages[EMsg::ClientAuthList] = &cmsg_client_auth_list__descriptor;
	messages[EMsg::ClientAuthListAck] = &cmsg_client_auth_list_ack__descriptor;
	messages[EMsg::ClientTicketAuthComplete] = &cmsg_client_ticket_auth_complete__descriptor;
	messages[EMsg::ClientRequestEncryptedAppTicket] = &cmsg_client_request_encrypted_app_ticket__descriptor;
	messages[EMsg::ClientRequestEncryptedAppTicketResponse] = &cmsg_client_request_encrypted_app_ticket_response__descriptor;
	// messages[EMsg::ClientCurrentUIMode] = &cmsg_Client_UIMode;
	// messages[EMsg::ClientVanityURLChangedNotification] = &cmsg_Client_VanityURLChangedNotification;
	// messages[EMsg::ClientAMGetPersonaNameHistory] = &cmsg_Client_AMGetPersonaNameHistory;
	// messages[EMsg::ClientAMGetPersonaNameHistoryResponse] = &cmsg_Client_AMGetPersonaNameHistoryResponse;
	// messages[EMsg::ClientGetCDNAuthToken] = &cmsg_Client_GetCDNAuthToken;
	// messages[EMsg::ClientGetCDNAuthTokenResponse] = &cmsg_Client_GetCDNAuthTokenResponse;
	// messages[EMsg::ClientKickPlayingSession] = &cmsg_Client_KickPlayingSession;
	// messages[EMsg::ClientPlayingSessionState] = &cmsg_Client_PlayingSessionState;
	// messages[EMsg::ClientToGC] = &cmsg_GCClient;
	// messages[EMsg::ClientFromGC] = &cmsg_GCClient;
	// messages[EMsg::ClientRichPresenceUpload] = &cmsg_Client_RichPresenceUpload;
	// messages[EMsg::ClientRichPresenceRequest] = &cmsg_Client_RichPresenceRequest;
	// messages[EMsg::ClientRichPresenceInfo] = &cmsg_Client_RichPresenceInfo;
	messages[EMsg::ClientGetEmoticonList] = &cmsg_client_get_emoticon_list__descriptor;
	messages[EMsg::ClientEmoticonList] = &cmsg_client_emoticon_list__descriptor;
	// messages[EMsg::ClientGetAuthorizedDevicesResponse] = &cmsg_Client_GetAuthorizedDevices;
	// messages[EMsg::ClientAuthorizeLocalDeviceRequest] = &cmsg_Client_AuthorizeLocalDeviceRequest;
	// messages[EMsg::ClientAuthorizeLocalDeviceResponse] = &cmsg_Client_AuthorizeLocalDevice;
	// messages[EMsg::ClientDeauthorizeDeviceRequest] = &cmsg_Client_DeauthorizeDeviceRequest;
	// messages[EMsg::ClientDeauthorizeDevice] = &cmsg_Client_DeauthorizeDevice;
	// messages[EMsg::ClientUseLocalDeviceAuthorizations] = &cmsg_Client_UseLocalDeviceAuthorizations;

	// message handlers
	messageHandlers[EMsg::ClientLoggedOff] = ServiceResponseHandler(&CSteamProto::OnClientLogoff);
	messageHandlers[EMsg::ClientLogOnResponse] = ServiceResponseHandler(&CSteamProto::OnClientLogon);
	messageHandlers[EMsg::ClientFriendsList] = ServiceResponseHandler(&CSteamProto::OnGotFriendList);

	// services from steammessages_auth.steamclient.proto
	services["Authentication"] = &authentication__descriptor;
	services["AuthenticationSupport"] = &authentication_support__descriptor;

	// services from steammessages_chat.steamclient.proto
	services["Chat"] = &chat__descriptor;
	services["ChatRoom"] = &chat_room__descriptor;
	services["ChatRoomClient"] = &chat_room_client__descriptor;

	// services from steammessages_friendmessages.steamclient.proto
	services["FriendMessages"] = &friend_messages__descriptor;
	services["FriendMessagesClient"] = &friend_messages_client__descriptor;

	// static service handlers
	serviceHandlers[PollAuthSessionStatus] = ServiceResponseHandler(&CSteamProto::OnPollSession);
	serviceHandlers[GetPasswordRSAPublicKey] = ServiceResponseHandler(&CSteamProto::OnGotRsaKey);
	serviceHandlers[BeginAuthSessionViaCredentials] = ServiceResponseHandler(&CSteamProto::OnBeginSession);
	serviceHandlers[UpdateAuthSessionWithSteamGuardCode] = ServiceResponseHandler(&CSteamProto::OnGotConfirmationCode);

	serviceHandlers[FriendSendMessage] = ServiceResponseHandler(&CSteamProto::OnMessageSent);
}

int CMPlugin::Load()
{
	InitSteamServices();

	// extra statuses
	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, "gaming");
	hExtraXStatus = ExtraIcon_RegisterIcolib("steam_game", LPGEN("Steam game"), iconName);

	HookEvent(ME_SKIN_ICONSCHANGED, OnReloadIcons);

	// menus
	CSteamProto::InitMenus();
	return 0;
}
