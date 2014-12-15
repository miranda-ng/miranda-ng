case MIRANDA_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupMiranda", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Miranda clients");
	break;

case MIRANDA_VERSION_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupMirandaVersion", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Miranda clients");
	break;

case MIRANDA_PACKS_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupMirandaPacks", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Miranda clients")_T("/")LPGENT("Pack overlays");
	break;

case MULTI_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupMulti", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Multi-proto clients");
	break;

case AIM_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupAIM", 1))
		return LPGENT("Client icons")_T("/")LPGENT("AIM clients");
	break;

case GG_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupGG", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Gadu-Gadu clients");
	break;

case ICQ_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupICQ", 1))
		return LPGENT("Client icons")_T("/")LPGENT("ICQ clients");
	break;

case IRC_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupIRC", 1))
		return LPGENT("Client icons")_T("/")LPGENT("IRC clients");
	break;

case JABBER_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupJabber", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Jabber clients");
	break;

case MRA_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupMRA", 1))
		return LPGENT("Client icons")_T("/")LPGENT("MRA clients");
	break;

case MSN_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupMSN", 1))
		return LPGENT("Client icons")_T("/")LPGENT("MSN clients");
	break;

case QQ_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupQQ", 1))
		return LPGENT("Client icons")_T("/")LPGENT("QQ clients");
	break;

case RSS_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupRSS", 1))
		return LPGENT("Client icons")_T("/")LPGENT("RSS clients");
	break;

case TLEN_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupTlen", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Tlen clients");
	break;

case WEATHER_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupWeather", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Weather clients");
	break;

case YAHOO_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupYahoo", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Yahoo clients");
	break;

case FACEBOOK_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupFacebook", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Facebook clients");
	break;

case VK_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupVK", 1))
		return LPGENT("Client icons")_T("/")LPGENT("VKontakte clients");
	break;

case OTHER_PROTOS_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOtherProtos", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Other protocols");
	break;

case OTHERS_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOthers", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Other icons");
	break;

case OVERLAYS_RESOURCE_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOverlaysResource", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Overlays")_T("/")LPGENT("Resource");
	break;

case OVERLAYS_PLATFORM_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOverlaysPlatform", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Overlays")_T("/")LPGENT("Platform");
	break;

case OVERLAYS_UNICODE_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOverlaysUnicode", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Overlays")_T("/")LPGENT("Platform");
	break;

case OVERLAYS_SECURITY_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOverlaysSecurity", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Overlays")_T("/")LPGENT("Security");
	break;

case OVERLAYS_PROTO_CASE:
	if (db_get_b(NULL, MODULENAME, "GroupOverlaysProtos", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Overlays")_T("/")LPGENT("Protocol");
	break;

default:
	return LPGENT("Client icons");
