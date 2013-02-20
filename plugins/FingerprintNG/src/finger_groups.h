case MIRANDA_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMiranda", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Miranda clients");
	break;

case MIRANDA_VERSION_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMirandaVersion", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Miranda clients");
	break;

case MIRANDA_PACKS_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMirandaPacks", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Miranda clients")_T("/")LPGENT("Pack overlays");
	break;

case MULTI_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMulti", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Multi-proto clients");
	break;

case AIM_CASE:
	if ( db_get_b(NULL, "Finger", "GroupAIM", 1))
		return LPGENT("Client icons")_T("/")LPGENT("AIM clients");
	break;

case GG_CASE:
	if ( db_get_b(NULL, "Finger", "GroupGG", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Gadu-Gadu clients");
	break;

case ICQ_CASE:
	if ( db_get_b(NULL, "Finger", "GroupICQ", 1))
		return LPGENT("Client icons")_T("/")LPGENT("ICQ clients");
	break;

case IRC_CASE:
	if ( db_get_b(NULL, "Finger", "GroupIRC", 1))
		return LPGENT("Client icons")_T("/")LPGENT("IRC clients");
	break;

case JABBER_CASE:
	if ( db_get_b(NULL, "Finger", "GroupJabber", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Jabber clients");
	break;

case MRA_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMRA", 1))
		return LPGENT("Client icons")_T("/")LPGENT("MRA clients");
	break;

case MSN_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMSN", 1))
		return LPGENT("Client icons")_T("/")LPGENT("MSN clients");
	break;

case QQ_CASE:
	if ( db_get_b(NULL, "Finger", "GroupQQ", 1))
		return LPGENT("Client icons")_T("/")LPGENT("QQ clients");
	break;

case RSS_CASE:
	if ( db_get_b(NULL, "Finger", "GroupRSS", 1))
		return LPGENT("Client icons")_T("/")LPGENT("RSS clients");
	break;

case TLEN_CASE:
	if ( db_get_b(NULL, "Finger", "GroupTlen", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Tlen clients");
	break;

case WEATHER_CASE:
	if ( db_get_b(NULL, "Finger", "GroupWeather", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Weather clients");
	break;

case YAHOO_CASE:
	if ( db_get_b(NULL, "Finger", "GroupYahoo", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Yahoo clients");
	break;

case OTHER_PROTOS_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOtherProtos", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Other protocols");
	break;

case OTHERS_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOthers", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Other icons");
	break;

case OVERLAYS_RESOURCE_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOverlaysResource", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Overlays")_T("/")LPGENT("Resource");
	break;

case OVERLAYS_PLATFORM_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOverlaysPlatform", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Overlays")_T("/")LPGENT("Platform");
	break;

case OVERLAYS_UNICODE_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOverlaysUnicode", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Overlays")_T("/")LPGENT("Platform");
	break;

case OVERLAYS_SECURITY_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOverlaysSecurity", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Overlays")_T("/")LPGENT("Security");
	break;

case OVERLAYS_PROTO_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOverlaysProtos", 1))
		return LPGENT("Client icons")_T("/")LPGENT("Overlays")_T("/")LPGENT("Protocol");
	break;

default:
	return LPGENT("Client icons");
