case MIRANDA_CASE:
	if(DBGetContactSettingByte(NULL, "Finger", "GroupMiranda", 1))
		SectName = _T("Client Icons/Miranda");
	break;
case MULTIPROTOCOL_CASE:		
	if(DBGetContactSettingByte(NULL, "Finger", "GroupMulti", 1))
		SectName = _T("Client Icons/Multi-Protocol");	
	break;
case ICQ_OFF_CASE:	
	if(DBGetContactSettingByte(NULL, "Finger", "GroupIcq", 1))
		SectName = _T("Client Icons/ICQ/Official");
	break;
case ICQ_UNOFF_CASE:	
	if(DBGetContactSettingByte(NULL, "Finger", "GroupIcq", 1))
		SectName = _T("Client Icons/ICQ/Unofficial");
	break;
case JABBER_CASE:		
	if(DBGetContactSettingByte(NULL, "Finger", "GroupJabber", 1))
		SectName = _T("Client Icons/Jabber");	
	break;
case MSN_CASE:		
	if(DBGetContactSettingByte(NULL, "Finger", "GroupMsn", 1))
		SectName = _T("Client Icons/MSN");
	break;
case AIM_CASE:		
	if(DBGetContactSettingByte(NULL, "Finger", "GroupAim", 1))
		SectName = _T("Client Icons/AIM");
	break;
case YAHOO_CASE:		
	if(DBGetContactSettingByte(NULL, "Finger", "GroupYahoo", 1))
		SectName = _T("Client Icons/Yahoo");
	break;
case IRC_CASE:
	if(DBGetContactSettingByte(NULL, "Finger", "GroupIrc", 1))
		SectName = _T("Client Icons/IRC");
	break;
case VOIP_CASE:	
	if(DBGetContactSettingByte(NULL, "Finger", "GroupVoIP", 1))
		SectName = _T("Client Icons/VoIP");
	break;
case SKYPE_CASE:	
	if(DBGetContactSettingByte(NULL, "Finger", "GroupSkype", 1))
		SectName = _T("Client Icons/Skype");
	break;
case GADU_CASE:	
	if(DBGetContactSettingByte(NULL, "Finger", "GroupGadu", 1))
		SectName = _T("Client Icons/Gadu-Gadu");
	break;
case PACKS_CASE:	
	if(DBGetContactSettingByte(NULL, "Finger", "GroupPacks", 1))
		SectName = _T("Client Icons/Miranda/Pack overlays");
	break;
case MRA_CASE:	
	if(DBGetContactSettingByte(NULL, "Finger", "GroupMail", 1))
		SectName = _T("Client Icons/Mail.Ru Agent");
	break;
case OVER1_CASE:
	if(DBGetContactSettingByte(NULL, "Finger", "GroupMiranda", 1))
		SectName = _T("Client Icons/Miranda/Overlays#1");
	break;
case SECURITY_CASE:
	if(DBGetContactSettingByte(NULL, "Finger", "GroupOtherProtos", 1))
		SectName = _T("Client Icons/Overlays/Security");
	break;
case OTHER_CASE:
	if(DBGetContactSettingByte(NULL, "Finger", "GroupOtherProtos", 1))
		SectName = _T("Client Icons/Other");
	break;
case OVER2_CASE:
	if(DBGetContactSettingByte(NULL, "Finger", "GroupMiranda", 1))
		SectName = _T("Client Icons/Miranda/Overlays#2");
	break;
case PLATFORM_CASE:	
	if(DBGetContactSettingByte(NULL, "Finger", "GroupOtherProtos", 1))
		SectName = _T("Client Icons/Overlays/Platform");
	break;
case WEATHER_CASE:	
	if(DBGetContactSettingByte(NULL, "Finger", "GroupWeather", 1))
		SectName = _T("Client Icons/Weather");
	break;
case RSS_CASE:	
	if(DBGetContactSettingByte(NULL, "Finger", "GroupRSS", 1))
		SectName = _T("Client Icons/RSS");
	break;
case QQ_CASE:	
	if(DBGetContactSettingByte(NULL, "Finger", "GroupQQ", 1))
		SectName = _T("Client Icons/QQ");
	break;
case TLEN_CASE:	
	if(DBGetContactSettingByte(NULL, "Finger", "GroupTlen", 1))
		SectName = _T("Client Icons/Tlen");
	break;
default:
	SectName = _T("Client Icons");