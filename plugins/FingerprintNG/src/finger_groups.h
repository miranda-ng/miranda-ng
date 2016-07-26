case MIRANDA_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupMiranda", 1))
		return LPGENW("Client icons") L"/" LPGENW("Miranda clients");
	break;

case MIRANDA_VERSION_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupMirandaVersion", 1))
		return LPGENW("Client icons") L"/" LPGENW("Miranda clients");
	break;

case MIRANDA_PACKS_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupMirandaPacks", 1))
		return LPGENW("Client icons") L"/" LPGENW("Miranda clients") L"/" LPGENW("Pack overlays");
	break;

case MULTI_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupMulti", 1))
		return LPGENW("Client icons") L"/" LPGENW("Multi-proto clients");
	break;

case AIM_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupAIM", 1))
		return LPGENW("Client icons") L"/" LPGENW("AIM clients");
	break;

case GG_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupGG", 1))
		return LPGENW("Client icons") L"/" LPGENW("Gadu-Gadu clients");
	break;

case ICQ_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupICQ", 1))
		return LPGENW("Client icons") L"/" LPGENW("ICQ clients");
	break;

case IRC_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupIRC", 1))
		return LPGENW("Client icons") L"/" LPGENW("IRC clients");
	break;

case JABBER_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupJabber", 1))
		return LPGENW("Client icons") L"/" LPGENW("Jabber clients");
	break;

case MRA_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupMRA", 1))
		return LPGENW("Client icons") L"/" LPGENW("MRA clients");
	break;

case MSN_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupMSN", 1))
		return LPGENW("Client icons") L"/" LPGENW("MSN clients");
	break;

case QQ_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupQQ", 1))
		return LPGENW("Client icons") L"/" LPGENW("QQ clients");
	break;

case RSS_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupRSS", 1))
		return LPGENW("Client icons") L"/" LPGENW("RSS clients");
	break;

case TLEN_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupTlen", 1))
		return LPGENW("Client icons") L"/" LPGENW("Tlen clients");
	break;

case WEATHER_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupWeather", 1))
		return LPGENW("Client icons") L"/" LPGENW("Weather clients");
	break;

case YAHOO_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupYahoo", 1))
		return LPGENW("Client icons") L"/" LPGENW("Yahoo clients");
	break;

case FACEBOOK_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupFacebook", 1))
		return LPGENW("Client icons") L"/" LPGENW("Facebook clients");
	break;

case VK_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupVK", 1))
		return LPGENW("Client icons") L"/" LPGENW("VKontakte clients");
	break;

case OTHER_PROTOS_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOtherProtos", 1))
		return LPGENW("Client icons") L"/" LPGENW("Other protocols");
	break;

case OTHERS_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOthers", 1))
		return LPGENW("Client icons") L"/" LPGENW("Other icons");
	break;

case OVERLAYS_RESOURCE_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOverlaysResource", 1))
		return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Resource");
	break;

case OVERLAYS_PLATFORM_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOverlaysPlatform", 1))
		return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Platform");
	break;

case OVERLAYS_UNICODE_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOverlaysUnicode", 1))
		return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Platform");
	break;

case OVERLAYS_SECURITY_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOverlaysSecurity", 1))
		return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Security");
	break;

case OVERLAYS_PROTO_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOverlaysProtos", 1))
		return LPGENW("Client icons") L"/" LPGENW("Overlays") L"/" LPGENW("Protocol");
	break;

default:
	return LPGENW("Client icons");
