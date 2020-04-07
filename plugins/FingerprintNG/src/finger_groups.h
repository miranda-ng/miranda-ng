case MIRANDA_CASE:
	if (g_plugin.getByte("GroupMiranda", 1))
		return LPGENW("Client icons") L"/" LPGENW("Miranda clients");
	break;

case MIRANDA_VERSION_CASE:
	if (g_plugin.getByte("GroupMirandaVersion", 1))
		return LPGENW("Client icons") L"/" LPGENW("Miranda clients");
	break;

case MIRANDA_PACKS_CASE:
	if (g_plugin.getByte("GroupMirandaPacks", 1))
		return LPGENW("Client icons") L"/" LPGENW("Miranda clients") L"/" LPGENW("Pack overlays");
	break;

case MULTI_CASE:
	if (g_plugin.getByte("GroupMulti", 1))
		return LPGENW("Client icons") L"/" LPGENW("Multi-proto clients");
	break;

case GG_CASE:
	if (g_plugin.getByte("GroupGG", 1))
		return LPGENW("Client icons") L"/" LPGENW("Gadu-Gadu clients");
	break;

case ICQ_CASE:
	if (g_plugin.getByte("GroupICQ", 1))
		return LPGENW("Client icons") L"/" LPGENW("ICQ clients");
	break;

case IRC_CASE:
	if (g_plugin.getByte("GroupIRC", 1))
		return LPGENW("Client icons") L"/" LPGENW("IRC clients");
	break;

case JABBER_CASE:
	if (g_plugin.getByte("GroupJabber", 1))
		return LPGENW("Client icons") L"/" LPGENW("Jabber clients");
	break;

case RSS_CASE:
	if (g_plugin.getByte("GroupRSS", 1))
		return LPGENW("Client icons") L"/" LPGENW("RSS clients");
	break;

case WEATHER_CASE:
	if (g_plugin.getByte("GroupWeather", 1))
		return LPGENW("Client icons") L"/" LPGENW("Weather clients");
	break;

case FACEBOOK_CASE:
	if (g_plugin.getByte("GroupFacebook", 1))
		return LPGENW("Client icons") L"/" LPGENW("Facebook clients");
	break;

case VK_CASE:
	if (g_plugin.getByte("GroupVK", 1))
		return LPGENW("Client icons") L"/" LPGENW("VKontakte clients");
	break;

case OTHER_PROTOS_CASE:
	if (g_plugin.getByte("GroupOtherProtos", 1))
		return LPGENW("Client icons") L"/" LPGENW("Other protocols");
	break;

case OTHERS_CASE:
	if (g_plugin.getByte("GroupOthers", 1))
		return LPGENW("Client icons") L"/" LPGENW("Other icons");
	break;

case OVERLAYS_RESOURCE_CASE:
	if (g_plugin.getByte("GroupOverlaysResource", 1))
		return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Resource");
	break;

case OVERLAYS_PLATFORM_CASE:
	if (g_plugin.getByte("GroupOverlaysPlatform", 1))
		return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Platform");
	break;

case OVERLAYS_UNICODE_CASE:
	if (g_plugin.getByte("GroupOverlaysUnicode", 1))
		return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Platform");
	break;

case OVERLAYS_SECURITY_CASE:
	if (g_plugin.getByte("GroupOverlaysSecurity", 1))
		return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Security");
	break;

case OVERLAYS_PROTO_CASE:
	if (g_plugin.getByte("GroupOverlaysProtos", 1))
		return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Protocol");
	break;

default:
	return LPGENW("Client icons");
