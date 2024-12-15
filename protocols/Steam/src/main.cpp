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
