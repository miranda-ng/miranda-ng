/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: translator.h 12299 2010-08-10 02:39:36Z silvercircle $
 *
 * string handling
 *
 */

#ifndef __STRINGS_H
#define __STRINGS_H

class CTranslator {

public:

	enum {
		NR_WEEKDAYS = 7,
		NR_MONTHS = 12
	};

	enum {
		CNT_MENU_STAYONTOP 										= 0,
		CNT_MENU_HIDETITLEBAR 									= 1,
		CNT_MENU_CONTAINEROPTIONS 								= 2,
		CNT_TITLE_DEFAULT										= 3,
		CNT_ATTACH_TO											= 4,
		GEN_META_CONTACT										= 5,
		GEN_META_FORCED											= 6,
		GEN_META_AUTOSELECT										= 7,
		GEN_META_USEPROTO										= 8,
		GEN_META_SETDEFAULT										= 9,
		GEN_MUC_NICKNAME										= 10,
		GEN_MUC_UID												= 11,
		GEN_MUC_STATUS											= 12,
		GEN_MUC_ROOM_TITLE_USER									= 13,
		GEN_MUC_ROOM_TITLE_USERS								= 14,
		GEN_MUC_ROOM_TITLE_FILTER								= 15,
		GEN_MUC_PRIVSESSION										= 16,
		GEN_MUC_PRIVSESSION_MULTI								= 17,
		GEN_MUC_FILTER_ERROR									= 18,
		GEN_MUC_FILTER_ERROR_TITLE								= 19,
		GEN_MUC_TEXTCOLOR										= 20,
		GEN_MUC_BGCOLOR											= 21,
		CNT_OPT_TITLE											= 22,
		CNT_OPT_TABSTOP											= 23,
		CNT_OPT_TABSBOTTOM										= 24,
		CNT_OPT_TABSLEFT										= 25,
		CNT_OPT_TABSRIGHT										= 26,
		CNT_OPT_HEADERBAR										= 27,
		GEN_MENUBAR_FILE										= 28,
		GEN_MENUBAR_VIEW										= 29,
		GEN_MENUBAR_USER										= 30,
		GEN_MENUBAR_ROOM										= 31,
		GEN_MENUBAR_LOG											= 32,
		GEN_MENUBAR_CONTAINER									= 33,
		GEN_MENUBAR_HELP										= 34,
		CNT_SBAR_SOUNDS											= 35,
		GEN_ENABLED												= 36,
		GEN_DISABLED											= 37,
		CNT_SBAR_MTN											= 38,
		GEN_IP_TIP_XSTATUS										= 39,
		GEN_IP_TIP_CLIENT										= 40,
		GEN_IP_TIP_STATUSMSG									= 41,
		GEN_IP_TIP_TITLE										= 42,
		GEN_SKIN_WARNCLOSE										= 43,
		GEN_SKIN_WARNCLOSE_TITLE								= 44,
		GEN_MTN_POPUP_WARNING									= 45,
		GEN_MTN_POPUP_UNSUPPORTED								= 46,
		GEN_CONTACT												= 47,
		GEN_MTN_START											= 48,
		GEN_MTN_STOP											= 49,
		GEN_FAVORITES											= 50,
		GEN_RECENT_SESSIONS										= 51,
		GEN_SBAR_LASTRECEIVED									= 52,
		GEN_SBAR_TIP_MSGLENGTH									= 53,
		CNT_OPT_TITLE_GEN										= 54,
		CNT_OPT_TITLE_LAYOUT									= 55,
		CNT_OPT_TITLE_TABS										= 56,
		CNT_OPT_TITLE_NOTIFY									= 57,
		CNT_OPT_TITLE_FLASHING									= 58,
		CNT_OPT_TITLE_TITLEBAR									= 59,
		CNT_OPT_TITLE_THEME										= 60,
		CNT_OPT_TITLE_TRANS										= 61,
		CNT_OPT_DESC_TABS										= 62,
		CNT_OPT_DESC_NOTIFY										= 63,
		CNT_OPT_DESC_THEME										= 64,
		CNT_OPT_DESC_TRANS										= 65,
		GEN_POPUPS_MESSAGE										= 66,
		GEN_POPUPS_UNKNOWN										= 67,
		GEN_POPUPS_NEW											= 68,
		GEN_NO_STATUS											= 69,
		GEN_MTN_STARTWITHNICK									= 70,
		GEN_MTN_TTITLE											= 71,
		GEN_MSG_TTITLE											= 72,
		GEN_ICONPACK_WARNING									= 73,		// unused!
		CNT_SELECT_FOR											= 74,
		CNT_SELECT_INUSE										= 75,
		CNT_SELECT_RENAMEERROR									= 76,
		CNT_SELECT_DELETEERROR									= 77,
		GEN_WARN_CLOSE											= 78,
		GEN_MSG_SAVE_NODIR										= 79,
		GEN_MSG_SAVE											= 80,
		GEN_MSG_SAVE_FILE_EXISTS								= 81,
		GEN_MUC_TOPIC_IS										= 82,
		GEN_MUC_NO_TOPIC										= 83,
		GEN_MTN_STOPPED											= 84,
		GEN_AVATAR_SETTINGS										= 85,
		GEN_AVATAR_SETOWN										= 86,
		GEN_WARNING_LOADTEMPLATES								= 87,
		GEN_TITLE_LOADTHEME										= 88,
		GEN_WARNING_PASTEANDSEND_DISABLED						= 89,
		GEN_WARNING_NUDGE_DISABLED								= 90,
		GEN_UNKNOWN_CONTACT										= 91,
		GEN_LOG_TODAY											= 92,
		GEN_LOG_YESTERDAY										= 93,
		GEN_LOG_USEDEFAULTCP									= 94,
		GEN_MSG_UINCOPY											= 95,
		GEN_MSG_NOUIN											= 96,
		GEN_MSG_UINCOPY_NOMC									= 97,
		GEN_MSG_SIGNEDOFF										= 98,
		GEN_MSG_SIGNEDON										= 99,
		GEN_MSG_CHANGEDSTATUS									= 100,
		GEN_SQ_WARNING											= 101,
		GEN_SQ_WARNING_TITLE									= 102,
		GEN_SQ_MULTISEND_NO_CONTACTS							= 103,
		GEN_SQ_DELIVERYFAILED									= 104,
		GEN_SQ_DELIVERYFAILEDLATE								= 105,
		GEN_SQ_MULTISEND_SUCCESS								= 106,
		GEN_SQ_QUEUED_MESSAGE									= 107,
		GEN_SQ_QUEUING_NOT_AVAIL								= 108,
		GEN_SQ_SENDLATER_HEADER									= 109,
		CNT_SBAR_SLIST											= 110,
		GEN_MSG_ENCODING										= 111,
		GEN_MSG_FAILEDSEND										= 112,
		GEN_MSG_TOO_LONG_SPLIT									= 113,
		GEN_MSG_TOO_LONG_NOSPLIT								= 114,
		GEN_MSG_CLOSE											= 115,
		GEN_MSG_SAVEANDCLOSE									= 116,
		GEN_MSG_LOGFROZENSTATIC									= 117,
		GEN_MSG_TIP_CONTACTMENU									= 118,
		GEN_MSG_BUTTON_RETRY									= 119,
		GEN_MSG_BUTTON_CANCEL									= 120,
		GEN_MSG_BUTTON_SENDLATER								= 121,
		GEN_MSG_SEL_COPIED										= 122,
		GEN_MSG_LOGFROZENQUEUED									= 123,
		GEN_MSG_UNKNOWNCLIENT									= 124,
		GEN_MSG_NOXSTATUSMSG									= 125,
		GEN_MSG_DELIVERYFAILURE									= 126,
		GEN_MSG_SENDTIMEOUT										= 127,
		GEN_MSG_SHOWPICTURE										= 128,
		GEN_MSG_NO_EDIT_NOTES									= 129,
		GEN_MSG_EDIT_NOTES_TIP									= 130,
		GEN_MSG_MC_OFFLINEPROTOCOL								= 131,
		GEN_MSG_OFFLINE_NO_FILE									= 132,
		GEN_STRING_FILE											= 133,
		GEN_STRING_MESSAGEFROM									= 134,
		GEN_SQ_MULTISENDERROR									= 135,
		GEN_MUC_LOOKUP											= 136,
		GEN_MUC_LOOKUP_NOWORD									= 137,
		GEN_MUC_MESSAGEAMP										= 138,
		GEN_STRING_UTF8											= 139,
		MUC_LOG_JOINED											= 140,
		MUC_LOG_ME_JOINED										= 141,
		MUC_LOG_LEFT											= 142,
		MUC_LOG_DISC											= 143,
		MUC_LOG_NICKCHANGE										= 144,
		MUC_LOG_ME_NICKCHANGE									= 145,
		MUC_LOG_KICK											= 146,
		MUC_LOG_NOTICE											= 147,
		MUC_LOG_TOPICIS											= 148,
		MUC_LOG_TOPICSETBYON									= 149,
		MUC_LOG_TOPICSETBY										= 150,
		MUC_LOG_STATUSENABLE									= 151,
		MUC_LOG_STATUSDISABLE									= 152,
		GEN_MUC_MENU_ADDTOHIGHLIGHT								= 153,
		GEN_MUC_HIGHLIGHT_ADD									= 154,
		GEN_MUC_HIGHLIGHT_EDIT									= 155,
		GEN_MUC_MENU_EDITHIGHLIGHTLIST							= 156,
		GEN_MSG_CONTACT_NOT_ON_LIST								= 157,
		GEN_SQ_SENDLATER_SUCCESS_POPUP							= 158,
		GEN_IP_MENU_COPY										= 159,
		GEN_IP_MENU_USER_DETAILS								= 160,
		GEN_IP_MENU_MSGPREFS									= 161,
		GEN_IP_MENU_ROOMPREFS									= 162,
		GEN_IP_MENU_HISTORY										= 163,
		GEN_STRING_HOUR											= 164,
		GEN_STRING_HOURS										= 165,
		GEN_STRING_MINUTE										= 166,
		GEN_STRING_MINUTES										= 167,
		MUC_SBAR_IDLEFORMAT										= 168,
		MUC_SBAR_ON_SERVER										= 169,
		MUC_SBAR_IDLEFORMAT_SHORT								= 170,
		CNT_OPT_TITLE_AVATARS									= 171,
		GEN_MUC_TRAY_HILIGHT									= 172,
		GEN_MUC_TRAY_MSG										= 173,
		GEN_MUC_TRAY_JOINED										= 174,
		GEN_MUC_TRAY_LEFT										= 175,
		GEN_MUC_TRAY_QUIT										= 176,
		GEN_MUC_TRAY_NICK										= 177,
		GEN_MUC_TRAY_KICK										= 178,
		GEN_MUC_TRAY_NOTICE										= 179,
		GEN_MUC_TRAY_TOPIC										= 180,
		GEN_MUC_TRAY_INFO										= 181,
		GEN_MUC_TRAY_STATUS_ON									= 182,
		GEN_MUC_TRAY_STATUS_OFF									= 183,

		GEN_MUC_POPUP_MSG										= 184,
		GEN_MUC_POPUP_JOINED									= 185,
		GEN_MUC_POPUP_LEFT										= 186,
		GEN_MUC_POPUP_LEFT1										= 187,
		GEN_MUC_POPUP_QUIT										= 188,
		GEN_MUC_POPUP_QUIT1										= 189,
		GEN_MUC_POPUP_NICK										= 190,
		GEN_MUC_POPUP_KICK										= 191,
		GEN_MUC_POPUP_KICK1										= 192,
		GEN_MUC_POPUP_NOTICE									= 193,
		GEN_MUC_POPUP_TOPIC										= 194,
		GEN_MUC_POPUP_TOPIC1									= 195,
		GEN_MUC_POPUP_STATUS_ON									= 196,
		GEN_MUC_POPUP_STATUS_OFF								= 197,
		CNT_OPT_TITLE_SOUNDS									= 198,
		GEN_IP_IDLENOTICE										= 199,
		GEN_INFOTIP_STATUSMSG									= 200,
		GEN_INFOTIP_XSTATUS										= 201,
		GEN_INFOTIP_LISTENING									= 202,
		GEN_INFOTIP_CLIENT										= 203,
		GEN_BB_IMGTOOLTIP										= 204,
		QMGR_COL_ODATE											= 205,
		QMGR_COL_MESSAGETEXT									= 206,
		QMGR_COL_STATUS											= 207,
		QMGR_COL_LASTSENDINFO									= 208,
		QMGR_FILTER_ALLCONTACTS									= 209,
		QMGR_STATUS_FAILED										= 210,
		QMGR_STATUS_SENTOK										= 211,
		QMGR_STATUS_PENDING										= 212,
		QMGR_STATUS_WAITACK										= 213,
		QMGR_ERROR_NOMULTISEND									= 214,
		QMGR_STATUS_REMOVED										= 215,
		QMGR_WARNING_REMOVAL									= 216,
		QMGR_TITLE												= 217,
		QMGR_STATUS_HOLD										= 218,
		QMGR_STATUS_DEFERRED									= 219,
		GEN_SQ_SENDLATER_FAILED_POPUP							= 220,
		GEN_SQ_SENDLATER_ERROR_MSG_TOO_LONG						= 221,
		GEN_DEFAULT_CONTAINER_NAME								= 222,
		GEN_STRING_EVENT_FILE_NODESC							= 223,
		GEN_STRING_EVENT_FILE_INVALID							= 224,
		GEN_STRING_EVENT_FILE									= 225,
		GEN_TOOLTIP_ADDCONTACT									= 226,
		GEN_TOOLTIP_DONTADD										= 227,
		GEN_TOOLTIP_EXPANDSIDEBAR								= 228,
		GEN_TASKBAR_STRING_CHAT_ROOM							= 229,
		GEN_TASKBAR_STRING_SERVER_WINDOW						= 230,
		GEN_TASKBAR_STRING_UNREAD								= 231,
		GEN_TASKBAR_STRING_USERS								= 232,
		GEN_AEROPEEK_NOHPP										= 233,
		GEN_STRING_WARNING_TITLE								= 234,
		STR_LAST 												= 235
	};

	enum {
		OPT_UPREFS_IPGLOBAL										= 0,
		OPT_UPREFS_IPON											= 1,
		OPT_UPREFS_IPOFF										= 2,
		OPT_UPREFS_AVON											= 3,
		OPT_UPREFS_AVOFF										= 4,
		OPT_UPREFS_FORCEHPP										= 5,
		OPT_UPREFS_FORCEIEV										= 6,
		OPT_UPREFS_FORCEDEFAULT									= 7,
		OPT_UPREFS_SIMPLETAGS									= 8,
		OPT_UPREFS_BBCODE										= 9,
		OPT_UPREFS_FORMATTING_OFF								= 10,
		OPT_UPREFS_DEFAULTCP									= 11,
		OPT_UPREFS_NOTZSVC										= 12,
		OPT_UPREFS_TITLE										= 13,
		OPT_UPREFS_MSGLOG										= 14,
		OPT_UPREFS_GENERIC										= 15,
		OPT_AERO_EFFECT_NONE									= 16,
		OPT_AERO_EFFECT_MILK									= 17,
		OPT_AERO_EFFECT_CARBON									= 18,
		OPT_AERO_EFFECT_SOLID									= 19,
		OPT_GEN_NONE											= 20,
		OPT_GEN_AUTO											= 21,
		OPT_GEN_SUNKEN											= 22,
		OPT_GEN_1PIXEL											= 23,
		OPT_GEN_ROUNDED											= 24,
		OPT_GEN_GLOBALLY_ON										= 25,
		OPT_GEN_ON_IF_PRESENT									= 26,
		OPT_GEN_GLOBALLY_OFF									= 27,
		OPT_GEN_ON_ALWAYS_BOTTOM								= 28,
		OPT_GEN_DONT_SHOW										= 29,
		OPT_TAB_LAYOUTTWEAKS									= 30,
		OPT_TAB_SKINLOAD										= 31,
		OPT_IPANEL_VISBILITY_TITLE								= 32,
		OPT_IPANEL_VISIBILTY_IM									= 33,
		OPT_IPANEL_VISIBILTY_CHAT								= 34,
		OPT_IPANEL_SYNC_TITLE_IM								= 35,
		OPT_IPANEL_SYNC_TITLE_MUC								= 36,
		OPT_IPANEL_VIS_INHERIT									= 37,
		OPT_IPANEL_VIS_OFF										= 38,
		OPT_IPANEL_VIS_ON										= 39,
		OPT_IPANEL_SIZE_GLOBAL									= 40,
		OPT_IPANEL_SIZE_PRIVATE									= 41,
		OPT_GEN_OFF												= 42,
		OPT_GEN_BBCODE											= 43,
		OPT_LOG_DEFAULT											= 44,
		OPT_LOG_IEVIEW											= 45,
		OPT_LOG_HPP												= 46,
		OPT_MTN_NEW												= 47,
		OPT_MTN_UNKNOWN											= 48,
		OPT_GEN_ALWAYS											= 49,
		OPT_MTN_NOTFOCUSED										= 50,
		OPT_MTN_ONLYCLOSED										= 51,
		OPT_CNT_ESCNORMAL										= 52,
		OPT_CNT_ESCMINIMIZE										= 53,
		OPT_CNT_ESCCLOS											= 54,
		OPT_MTN_UNSUPPORTED										= 55,
		OPT_SMODE_CHOOSE										= 56,
		OPT_MUC_LOGTIP1											= 57,
		OPT_MUC_LOGTIP2											= 58,
		OPT_MUC_LOGTIP3											= 59,
		OPT_MUC_LOGTIP4											= 60,
		OPT_MUC_LOGTIP5											= 61,
		OPT_MUC_LOGTIP6											= 62,
		OPT_MUC_LOGTIP7											= 63,
		OPT_MUC_LOGTIP8											= 64,
		OPT_MUC_LOGTIP9											= 65,
		OPT_MUC_LOGTIP10										= 66,
		OPT_MUC_LOGTIP11										= 67,
		OPT_MUC_LOGTIP12										= 68,
		OPT_MUC_LOGTIP13										= 69,
		OPT_MUC_LOGTIP14										= 70,
		OPT_MUC_LOGTIP15										= 71,
		OPT_MUC_LOGTIP16										= 72,
		OPT_MUC_LOGTIP17										= 73,
		OPT_MUC_LOGTIP18										= 74,
		OPT_MUC_LOGTIP19										= 75,
		OPT_MUC_LOGTIP20										= 76,
		OPT_MUC_LOGTIP21										= 77,
		OPT_MUC_LOGTIP22										= 78,
		OPT_MUC_OPTHEADER1										= 79,
		OPT_MUC_OPTHEADER2										= 80,
		OPT_MUC_VARIABLES										= 81,
		OPT_MUC_SELECTFOLDER									= 82,
		OPT_MUC_NOMARKERS										= 83,
		OPT_MUC_ASICONS											= 84,
		OPT_MUC_ASSYMBOLS										= 85,
		OPT_TEMP_TITLE											= 86,
		OPT_TEMP_RESET											= 87,
		OPT_TEMP_WASRESET										= 88,
		OPT_TEMP_HELPTITLE										= 89,
		OPT_TABS_GENERAL										= 90,
		OPT_TABS_TABS											= 91,
		OPT_TABS_CONTAINERS										= 92,
		OPT_TABS_LOG											= 93,
		OPT_TABS_TOOLBAR										= 94,
		OPT_TABS_ADVANCED										= 95,
		OPT_TABS_MUC_SETTINGS									= 96,
		OPT_TABS_MUC_LOG										= 97,
		OPT_TABS_MUC_EVENTS										= 98,
		OPT_TABS_MUC_HIGHLIGHT									= 99,
		OPT_MSGLOG_EXPLAINSETTINGS								= 100,
		OPT_SKIN_NOSKINSELECT									= 101,
		OPT_LAST												= 102
	};

	enum {
		WARN_RELNOTES											= 0,
		WARN_ICONPACKVERSION									= 1,
		WARN_EDITUSERNOTES										= 2,
		WARN_ICONPACKMISSING									= 3,
		WARN_AEROPEEKSKIN										= 4,
		WARN_CHAT_ENABLED										= 5,
		WARN_IMGSVC_MISSING										= 6,
		WARN_HPP_APICHECK										= 7,
		WARN_NO_SENDLATER										= 8,
		WARN_CLOSEWINDOW										= 9,
		WARN_OPTION_CLOSE										= 10,
		WARN_THEME_OVERWRITE									= 11,
		WARN_LAST												= 12
	};
	/*
	 * identities for the option trees
	 */

	enum {
		TREE_MODPLUS = 0,
		TREE_NEN = 1,
		TREE_MSG = 2,
		TREE_LOG = 3,
		TREE_TAB = 4,
	};

	CTranslator();
	~CTranslator();

	inline static const wchar_t* get(const UINT id)
	{
		return(m_translated[id]);
	}

	inline static const wchar_t* getOpt(const UINT id)
	{
		return(m_OptTranslated[id]);
	}

	inline static const wchar_t* getWarning(const UINT id)
	{
		return(m_WarningsTranslated[id]);
	}

	inline static const wchar_t* getUntranslatedWarning(const UINT id)
	{
		return(m_Warnings[id]);
	}

	inline static const wchar_t* getWeekday(const UINT id)
	{
		return(weekDays_translated[id]);
	}

	inline static const wchar_t* getMonth(const UINT id)
	{
		return(months_translated[id]);
	}

	static void preTranslateAll()
	{
		int i;

		for(i = 0; i < STR_LAST; i++)
			m_translated[i] = TranslateTS(m_strings[i]);

		for(i = 0; i < OPT_LAST; i++)
			m_OptTranslated[i] = TranslateTS(m_OptStrings[i]);

		for(i = 0; i < WARN_LAST; i++)
			m_WarningsTranslated[i] = TranslateTS(m_Warnings[i]);

		translateGroupTree(m_lvGroupsModPlus);
		translateGroupTree(m_lvGroupsNEN);
		translateGroupTree(m_lvGroupsMsg);
		translateGroupTree(m_lvGroupsTab);
		translateGroupTree(m_lvGroupsLog);

		translateOptionTree(m_lvItemsModPlus);
		translateOptionTree(m_lvItemsNEN);
		translateOptionTree(m_lvItemsMsg);
		translateOptionTree(m_lvItemsTab);
		translateOptionTree(m_lvItemsLog);

		for(i = 0; i < NR_WEEKDAYS; i++)
			weekDays_translated[i] = TranslateTS(weekDays[i]);

		for(i = 0; i < NR_MONTHS; i++)
			months_translated[i] = TranslateTS(months[i]);
	}

	static void translateGroupTree(TOptionListGroup *groups);
	static void translateOptionTree(TOptionListItem *items);

	static	TOptionListItem* 	getTree(UINT id);
	static  TOptionListGroup* 	getGroupTree(UINT id);

private:
	static wchar_t *m_strings[STR_LAST];
	static wchar_t *m_translated[STR_LAST];

	static wchar_t *m_OptStrings[OPT_LAST];
	static wchar_t *m_OptTranslated[OPT_LAST];

	static wchar_t *m_Warnings[WARN_LAST];
	static wchar_t *m_WarningsTranslated[WARN_LAST];

	static wchar_t *weekDays[7];
	static wchar_t *months[12];

	static wchar_t *weekDays_translated[7];
	static wchar_t *months_translated[12];

	static TOptionListGroup		m_lvGroupsModPlus[], m_lvGroupsNEN[], m_lvGroupsMsg[], m_lvGroupsLog[], m_lvGroupsTab[];
	static TOptionListItem		m_lvItemsModPlus[], m_lvItemsNEN[], m_lvItemsMsg[], m_lvItemsLog[], m_lvItemsTab[];
};

#endif


