case MIRANDA_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMiranda", 1))
		return _T("Client Icons/Miranda");
	break;
case MULTIPROTOCOL_CASE:		
	if ( db_get_b(NULL, "Finger", "GroupMulti", 1))
		return _T("Client Icons/Multi-Protocol");	
	break;
case ICQ_OFF_CASE:	
	if ( db_get_b(NULL, "Finger", "GroupIcq", 1))
		return _T("Client Icons/ICQ/Official");
	break;
case ICQ_UNOFF_CASE:	
	if ( db_get_b(NULL, "Finger", "GroupIcq", 1))
		return _T("Client Icons/ICQ/Unofficial");
	break;
case JABBER_CASE:		
	if ( db_get_b(NULL, "Finger", "GroupJabber", 1))
		return _T("Client Icons/Jabber");	
	break;
case MSN_CASE:		
	if ( db_get_b(NULL, "Finger", "GroupMsn", 1))
		return _T("Client Icons/MSN");
	break;
case AIM_CASE:		
	if ( db_get_b(NULL, "Finger", "GroupAim", 1))
		return _T("Client Icons/AIM");
	break;
case YAHOO_CASE:		
	if ( db_get_b(NULL, "Finger", "GroupYahoo", 1))
		return _T("Client Icons/Yahoo");
	break;
case IRC_CASE:
	if ( db_get_b(NULL, "Finger", "GroupIrc", 1))
		return _T("Client Icons/IRC");
	break;
case VOIP_CASE:	
	if ( db_get_b(NULL, "Finger", "GroupVoIP", 1))
		return _T("Client Icons/VoIP");
	break;
case SKYPE_CASE:	
	if ( db_get_b(NULL, "Finger", "GroupSkype", 1))
		return _T("Client Icons/Skype");
	break;
case GADU_CASE:	
	if ( db_get_b(NULL, "Finger", "GroupGadu", 1))
		return _T("Client Icons/Gadu-Gadu");
	break;
case PACKS_CASE:	
	if ( db_get_b(NULL, "Finger", "GroupPacks", 1))
		return _T("Client Icons/Miranda/Pack overlays");
	break;
case MRA_CASE:	
	if ( db_get_b(NULL, "Finger", "GroupMail", 1))
		return _T("Client Icons/Mail.Ru Agent");
	break;
case OVER1_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMiranda", 1))
		return _T("Client Icons/Miranda/Overlays#1");
	break;
case SECURITY_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOtherProtos", 1))
		return _T("Client Icons/Overlays/Security");
	break;
case OTHER_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOtherProtos", 1))
		return _T("Client Icons/Other");
	break;
case OVER2_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMiranda", 1))
		return _T("Client Icons/Miranda/Overlays#2");
	break;
case PLATFORM_CASE:	
	if ( db_get_b(NULL, "Finger", "GroupOtherProtos", 1))
		return _T("Client Icons/Overlays/Platform");
	break;
case WEATHER_CASE:	
	if ( db_get_b(NULL, "Finger", "GroupWeather", 1))
		return _T("Client Icons/Weather");
	break;
case RSS_CASE:	
	if ( db_get_b(NULL, "Finger", "GroupRSS", 1))
		return _T("Client Icons/RSS");
	break;
case QQ_CASE:	
	if ( db_get_b(NULL, "Finger", "GroupQQ", 1))
		return _T("Client Icons/QQ");
	break;
case TLEN_CASE:	
	if ( db_get_b(NULL, "Finger", "GroupTlen", 1))
		return _T("Client Icons/Tlen");
	break;
default:
	return _T("Client Icons");