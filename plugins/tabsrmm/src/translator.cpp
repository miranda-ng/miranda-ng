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
 * $Id: translator.cpp 12999 2010-10-22 07:32:53Z silvercircle $
 *
 * handle pretranslated strings
 *
 */

#include "commonheaders.h"

wchar_t* CTranslator::weekDays[7] = {LPGENT("Sunday"), LPGENT("Monday"),
		LPGENT("Tuesday"), LPGENT("Wednesday"),
		LPGENT("Thursday"), LPGENT("Friday"), LPGENT("Saturday")};

wchar_t* CTranslator::months[12] = {LPGENT("January"), LPGENT("February"),
		LPGENT("March"), LPGENT("April"), LPGENT("May"),
		LPGENT("June"), LPGENT("July"), LPGENT("August"),
		LPGENT("September"), LPGENT("October"), LPGENT("November"), LPGENT("December")};

wchar_t* CTranslator::weekDays_translated[7];
wchar_t* CTranslator::months_translated[12];

wchar_t* CTranslator::m_strings[STR_LAST] = {
	LPGENT("Stay on Top"),								/* CNT_MENU_STAYONTOP */
	LPGENT("Hide titlebar"),							/* CNT_MENU_HIDETITLEBAR */
	LPGENT("Container options..."),						/* CNT_MENU_CONTAINEROPTIONS */
	LPGENT("Message Session..."),						/* CNT_TITLE_DEFAULT */
	LPGENT("Attach to"),								/* CNT_ATTACH_TO */
	LPGENT("Meta Contact"),								/* GEN_META_CONTACT */
	LPGENT("(Forced)"),									/* GEN_META_FORCED */
	LPGENT("Autoselect"),								/* GEN_META_AUTOSELECT */
	LPGENT("Use Protocol"),								/* GEN_META_USEPROTO */
	LPGENT("Set Default Protocol"),						/* GEN_META_SETDEFAULT */
	LPGENT("Nick name"),								/* GEN_MUC_NICKNAME */
	LPGENT("Unique Id"),								/* GEN_MUC_UID */
	LPGENT("Status"),									/* GEN_MUC_STATUS */
	LPGENT("%s: Chat Room (%u user%s)"),				/* GEN_MUC_ROOM_TITLE_USER */
	LPGENT("%s: Chat Room (%u users%s)"),				/* GEN_MUC_ROOM_TITLE_USERS */
	LPGENT(", event filter active"),					/* GEN_MUC_ROOM_TITLE_FILTER */
	LPGENT("%s: Message Session"),						/* GEN_MUC_PRIVSESSION */
	LPGENT("%s: Message Session (%u users)"),			/* GEN_MUC_PRIVSESSION_MULTI */
	LPGENT("The filter canoot be enabled, because there are no event types selected either global or for this chat room"),  /* GEN_MUC_FILTER_ERROR */
	LPGENT("Event filter error"),						/* GEN_MUC_FILTER_ERROR_TITLE */
	LPGENT("Text color"),								/* GEN_MUC_TEXTCOLOR */
	LPGENT("Background color"),							/* GEN_MUC_BGCOLOR */
	LPGENT("Container options"),						/* CNT_OPT_TITLE */
	LPGENT("Tabs at the top"),							/* CNT_OPT_TABSTOP */
	LPGENT("Tabs at the bottom"),						/* CNT_OPT_TABSBOTTOM */
	LPGENT("Switch bar on the left side"),				/* CNT_OPT_TABSLEFT */
	LPGENT("Switch bar on the right side"), 			/* CNT_OPT_TABSRIGHT */
	LPGENT("Configure container options for\n%s"),		/* CNT_OPT_HEADERBAR */
	LPGENT("&File"), 									/* GEN_MENUBAR_FILE */
	LPGENT("&View"),									/* GEN_MENUBAR_VIEW */
	LPGENT("&User"),									/* GEN_MENUBAR_USER */
	LPGENT("&Room"),									/* GEN_MENUBAR_FILE */
	LPGENT("Message &Log"),								/* GEN_MENUBAR_LOG */
	LPGENT("&Container"),								/* GEN_MENUBAR_CONTAINER */
	LPGENT("Help"), 									/* GEN_MENUBAR_HELP */
	LPGENT("Sounds are %s. Click to toggle status, hold SHIFT and click to set for all open containers"), /* GEN_CNT_SBAR_SOUNDS */
	LPGENT("enabled"),									/* GEN_ENABLED */
	LPGENT("disabled"),									/* GEN_DISABLED */
	LPGENT("Sending typing notifications is %s."),		/* GEN_CNT_SBAR_MTN */
	LPGENT("Extended status for %s: %s"),				/* GEN_IP_TIP_XSTATUS */
	LPGENT("%s is using"),								/* GEN_IP_TIP_CLIENT */
	LPGENT("Status message for %s (%s)"),				/* GEN_IP_TIP_STATUSMSG */
	LPGENT("tabSRMM Information"),						/* GEN_IP_TIP_TITLE */
	LPGENT("All message containers need to close before the skin can be changed\nProceed?"), /* GEN_SKIN_WARNCLOSE */
	LPGENT("Change skin"),								/* GEN_SKIN_WARNCLOSE_TITLE */
	LPGENT("Warning: Popup plugin not found."),			/* GEN_MTN_POPUP_WARNING */
	LPGENT("Warning: Current Popup plugin version is not supported."), /* GEN_MTN_POPUP_UNSUPPORTED */
	LPGENT("Contact"),									/* GEN_CONTACT */
	LPGENT("...is typing a message."),					/* GEN_MTN_START */
	LPGENT("...has stopped typing."),					/* GEN_MTN_STOP */
	LPGENT("Favorites"),								/* GEN_FAVORITES */
	LPGENT("Recent Sessions"),							/* GEN_RECENT_SESSIONS */
	LPGENT("Last received: %s at %s"),					/* GEN_SBAR_LASTRECEIVED */
	LPGENT("There are %d pending send jobs. Message length: %d bytes, message length limit: %d bytes\n\n%d messages are queued for later delivery"), /* GEN_SBAR_TIP_MSGLENGTH */
	LPGENT("General options"),							/* CNT_OPT_TITLE_GEN */
	LPGENT("Window layout"),							/* CNT_OPT_TITLE_LAYOUT */
	LPGENT("Tabs and switch bar"),						/* CNT_OPT_TITLE_TABS */
	LPGENT("Notifications"),							/* CNT_OPT_TITLE_NOTIFY */
	LPGENT("Flashing"),									/* CNT_OPT_TITLE_FLASHING */
	LPGENT("Title bar"),								/* CNT_OPT_TITLE_TITLEBAR */
	LPGENT("Window size and theme"),					/* CNT_OPT_TITLE_THEME */
	LPGENT("Transparency"),								/* CNT_OPT_TITLE_TRANS */
	LPGENT("Choose your options for the tabbed user interface. Not all options can be applied to open windows. You may need to close and re-open them."), /* CNT_OPT_DESC_TABS */
	LPGENT("Select, when you want to see event notifications (popups) for this window. The settings apply to all tabs within this window."), /*CNT_OPT_DESC_NOTIFY */
	LPGENT("You can select a private theme (.tabsrmm file) for this container which will then override the default message log theme. You will have to close and re-open all message windows after changing this option."), /* CNT_OPT_DESC_THEME */
	LPGENT("This feature requires Windows 2000 or later and may be unavailable when using a container skin."), /* CNT_OPT_DESC_TRANS */
	LPGENT("Message"),									/* GEN_POPUPS_MESSAGE */
	LPGENT("Unknown event"),							/* GEN_POPUPS_UNKNOWN */
	LPGENT("New messages: "),							/* GEN_POPUPS_NEW */
	LPGENT("No status message"),		 				/* GEN_NO_STATUS */
	LPGENT("%s is typing a message."), 					/* GEN_MTN_STARTWITHNICK */
	LPGENT("Typing Notification"),						/* GEN_MTN_TTITLE */
	LPGENT("Message from %s"),							/* GEN_MSG_TTITLE */
	LPGENT("/"),										/* GEN_ICONPACK_WARNING */ /* NOT IN USE!! */
	LPGENT("Select container for %s"),					/* CNT_SELECT_FOR */
	LPGENT("This name is already in use"),				/* CNT_SELECT_INUSE */
	LPGENT("You cannot rename the default container"),  /* CNT_SELECT_RENAMEERROR */
	LPGENT("You cannot delete the default container"),  /* CNT_SELECT_DELETEERROR */
	LPGENT(" "), /* GEN_WARN_CLOSE */ /* UNUSED */
	LPGENT("Error creating destination directory"),		 /* GEN_MSG_SAVE_NODIR */
	LPGENT("Save contact picture"),						/* GEN_MSG_SAVE */
	LPGENT("The file exists. Do you want to overwrite it?"), /* GEN_MSG_SAVE_FILE_EXISTS */
	LPGENT("Topic is: %s"),								/* GEN_MUC_TOPIC_IS */
	LPGENT("no topic set."), 							/* GEN_MUC_NO_TOPIC */
	LPGENT("%s has entered text."),						/* GEN_MTN_STOPPED */
	LPGENT("Contact Picture Settings..."),				/* GEN_AVATAR_SETTINGS */
	LPGENT("Set Your Avatar..."),						/* GEN_AVATAR_SETOWN */
	LPGENT("Do you want to also read message templates from the theme?\nCaution: This will overwrite the stored template set which may affect the look of your message window significantly.\nSelect cancel to not load anything at all."), /* GEN_WARNING_LOADTEMPLATES */
	LPGENT("Load theme"),								/* GEN_TITLE_LOADTHEME */
	LPGENT("The 'paste and send' feature is disabled. You can enable it on the 'General' options page in the 'Sending Messages' section"), /* GEN_WARNING_PASTEANDSEND_DISABELD */
	LPGENT("Either the nudge plugin is not installed or the contact's protocol does not support sending a nudge event."),  /*GEN_WARNING_NUDGE_DISABLED */
	LPGENT("'(Unknown Contact)'"),						/* GEN_UNKNOWN_CONTACT */
	LPGENT("Today"),									/* GEN_LOG_TODAY */
	LPGENT("Yesterday"),								/* GEN_LOG_YESTERDAY */
	LPGENT("Use default codepage"),						/* GEN_LOG_USEDEFAAULTCP */
	LPGENT("UID: %s (SHIFT click -> copy to clipboard)\nClick for User's Details\nRight click for MetaContact control\nClick dropdown to add or remove user from your favorites."), /* GEN_MSG_UINCOPY */
	LPGENT("No UID"),									/* GEN_MSG_NOUIN */
	LPGENT("UID: %s (SHIFT click -> copy to clipboard)\nClick for User's Details\nClick dropdown to change this contact's favorite status."), /* GEN_MSG_UINCOPY_NO_MC */
	LPGENT("signed off."),								/* GEN_MSG_SIGNEDOFF */
	LPGENT("signed on and is now %s."),					/* GEN_MSG_SIGNEDON */
	LPGENT("changed status from %s to %s."), 			/* GEN_MSG_CHANGEDSTATUS */
	LPGENT("There are unsent messages waiting for confirmation.\nWhen you close the window now, Miranda will try to send them but may be unable to inform you about possible delivery errors.\nDo you really want to close the Window(s)?"), /* GEN_SQ_WARNING */
	LPGENT("Message window warning"), 					/* GEN_SQ_WARNING_TITLE */
	LPGENT("You haven't selected any contacts from the list. Click the checkbox box next to a name to send the message to that person."), /* GEN_SQ_MULTISEND_NOCONTACTS */
	LPGENT("A message delivery has failed.\nClick to open the message window."), /* GEN_SQ_DELIVERYFAILED */
	LPGENT("A message delivery has failed after the contacts chat window was closed. You may want to resend the last message"), /* GEN_SQ_DELIVERYFAILEDLATE */
	LPGENT("Multisend: successfully sent to: %s"), 		/* GEN_SQ_MULTISEND_SUCCESS */
	LPGENT("Message successfully queued for later delivery.\nIt will be sent as soon as possible and a popup will inform you about the result."), /* GEN_SQ_QUEUED_MESSAGE */
	LPGENT("The send later feature is not available on this protocol."), /* GEN_SQ_QUEUING_NOT_AVAIL */
	LPGENT("\n(Sent delayed. Original timestamp %s)"),  /* GEN_SQ_SENDLATER_HEADER */
	LPGENT("Session list.\nClick left for a list of open sessions.\nClick right to access favorites and quickly configure message window behavior"), /* CNT_SBAR_SLIST */
	LPGENT("Character Encoding"),						/* GEN_MSG_ENCODING */
	LPGENT("A message failed to send successfully."),   /* GEN_MSG_FAILEDSEND */
	LPGENT("WARNING: The message you are trying to paste exceeds the message size limit for the active protocol. It will be sent in chunks of max %d characters"), /* GEN_MSG_TOO_LONG_SPLIT */
	LPGENT("The message you are trying to paste exceeds the message size limit for the active protocol. Only the first %d characters will be sent."), /* GEN_MSG_TOO_LONG_NOSPLIT */
	LPGENT("Close Session"),							/* GEN_MSG_CLOSE */
	LPGENT("Save and close session"),					/* GEN_MSG_SAVEANDCLOSE */
	LPGENT("Autoscrolling is disabled (press F12 to enable it)"),					/* GEN_MSG_LOGFROZENSTATIC */
	LPGENT("Click for contact menu\nClick dropdown for window settings"), /*GEN_MSG_TIP_CONTACTMENU */
	LPGENT("Retry"),									/* GEN_MSG_BUTTON_RETRY */
	LPGENT("Cancel"),									/* GEN_MSG_BUTTON_CANCEL */
	LPGENT("Send later"),								/* GEN_MSG_BUTTON_SENDLATER */
	LPGENT("Selection copied to clipboard"),			/* GEN_MSG_SEL_COPIED */
	LPGENT("Autoscrolling is disabled, %d message(s) queued (press F12 to enable it)"),		/* GEN_MSG_LOGFROZENQUEUED */
	LPGENT("Unknown client"),							/* GEN_MSG_UNKNOWNCLIENT */
	LPGENT("No extended status message available"),		/* GEN_MSG_NOXSTATUSMSG */
	LPGENT("Delivery failure: %s"),						/* GEN_MSG_DELIVERYFAILURE */
	LPGENT("The message send timed out"),				/* GEN_MSG_SENDTIMEOUT */
	LPGENT("Show Contact Picture"),						/* GEN_MSG_SHOWPICTURE */
	LPGENT("You cannot edit user notes when there are unsent messages"), /* GEN_MSG_NO_EDIT_NOTES */
	LPGENT("You are editing the user notes. Click the button again or use the hotkey (default: Alt-N) to save the notes and return to normal messaging mode"), /* GEN_MSG_EDIT_NOTES_TIP */
	LPGENT("Warning: you have selected a subprotocol for sending the following messages which is currently offline"), /* GEN_MSG_MC_OFFLINEPROTOCOL */
	LPGENT("Contact is offline and this protocol does not support sending files to offline users."), /* GEN_MSG_OFFLINE_NO_FILE */
	LPGENT("File"),										/* GEN_STRING_FILE */
	LPGENT("Message from %s"),							/* GEN_STRING_MESSAGEFROM */
	LPGENT("Multisend: failed sending to: %s"),			/* GEN_SQ_MULTISENDERROR */
	LPGENT("Look up \'%s\':"), 							/* GEN_MUC_LOOKUP */
	LPGENT("No word to look up"),						/* GEN_MUC_LOOKUP_NOWORD */
	LPGENT("&Message"),									/* GEN_MUC_MESSAGEAMP */
	LPGENT("UTF-8"),									/* GEN_STRING_UTF8 */

	/* MUC LOG Formatting strings*/

	LPGENT("%s has joined"),							/* MUC_LOG_JOINED */
	LPGENT("You have joined %s"),						/* MUC_LOG_ME_JOINED */
	LPGENT("%s has left"), 								/* MUC_LOG_LEFT */
	LPGENT("%s has disconnected"),						/* MUC_LOG_DISC */
	LPGENT("%s is now known as %s"),					/* MUC_LOG_NICKCHANGE */
	LPGENT("You are now known as %s"),					/* MUC_LOG_ME_NICKCHANGE */
	LPGENT("%s kicked %s"),								/* MUC_LOG_KICK */
	LPGENT("Notice from %s: "), 						/* MUC_LOG_NOTICE */
	LPGENT("The topic is \'%s%s\'"),					/* MUC_LOG_TOPICIS */
	LPGENT(" (set by %s on %s)"), 						/* MUC_LOG_TOPICSETBYON */
	LPGENT(" (set by %s)"),								/* MUC_LOG_TOPICSETBY */
	LPGENT("%s enables \'%s\' status for %s"),			/* MUC_LOG_STATUSENABLE */
	LPGENT("%s disables \'%s\' status for %s"),			/* MUC_LOG_STATUSDISABLE */
	LPGENT("Highlight User..."),						/* GEN_MUC_MENU_ADDTOHIGHLIGHT */
	LPGENT("Add user to highlight list"),				/* GEN_MUC_HIGHLIGHT_ADD */
	LPGENT("Edit user highlight list"),					/* GEN_MUC_HIGHLIGHT_EDIT */
	LPGENT("Edit Highlight List..."), 					/* GEN_MUC_MENU_EDITHIGHLIGHTLIST */
	LPGENT("Contact not on list. You may add it..."),	/* GEN_MSG_CONTACT_NOT_ON_LIST */
	LPGENT("A send later job completed successfully.\nThe original message: %s"), /* GEN_SQ_SENDLATER_SUCCESS_POPUP */

	LPGENT("Copy To Clipboard"),						/* GEN_IP_MENU_COPY */
	LPGENT("Open User Details..."),						/* GEN_IP_MENU_USER_DETAILS */
	LPGENT("Messaging Settings..."),					/* GEN_IP_MENU_MSGPREFS */
	LPGENT("Room Settings..."),							/* GEN_IP_MENU_ROOMPREFS */
	LPGENT("Open History..."),							/* GEN_IP_MENU_HISTORY */

	LPGENT("hour"),										/* GEN_STRING_HOUR */
	LPGENT("hours"),									/* GEN_STRING_HOURS */
	LPGENT("minute"),									/* GEN_STRING_MINUTE */
	LPGENT("minutes"),									/* GEN_STRING_MINUTES */
	LPGENT(", %d %s, %d %s idle"),						/* MUC_SBAR_IDLEFORMAT */
	LPGENT("%s on %s%s"),								/* MUC_SBAR_ON_SERVER */
	LPGENT(", %d %s idle"),								/* MUC_SBAR_IDLEFORMAT_SHORT */
	LPGENT("Contact avatars"),							/* CNT_OPT_TITLE_AVATARS */

	/*
	 * MUC tray icon notifications
	 */

	LPGENT("%s wants your attention in %s"),			/* GEN_MUC_TRAY_HILIGHT */
	LPGENT("%s speaks in %s"),							/* GEN_MUC_TRAY_MSG */
	LPGENT("%s has joined %s"),							/* GEN_MUC_TRAY_JOINED */
	LPGENT("%s has left %s"),							/* GEN_MUC_TRAY_LEFT */
	LPGENT("%s has disconnected"),						/* GEN_MUC_TRAY_QUIT */
	LPGENT("%s is now known as %s"),					/* GEN_MUC_TRAY_NICK */
	LPGENT("%s kicked %s from %s"),						/* GEN_MUC_TRAY_KICK */
	LPGENT("Notice from %s"),							/* GEN_MUC_TRAY_NOTICE */
	LPGENT("Topic change in %s"),						/* GEN_MUC_TRAY_TOPIC */
	LPGENT("Information in %s"),						/* GEN_MUC_TRAY_INFO */
	LPGENT("%s enables \'%s\' status for %s in %s"),	/* GEN_MUC_TRAY_STATUS_ON */
	LPGENT("%s disables \'%s\' status for %s in %s"),	/* GEN_MUC_TRAY_STATUS_OFF */

	/*
	 * muc popups and disk logging
	 */

	LPGENT("%s%s says:%s %s"),							/* GEN_MUC_POPUP_MSG */
	LPGENT("%s has joined"),							/* GEN_MUC_POPUP_JOINED */
	LPGENT("%s has left"),								/* GEN_MUC_POPUP_LEFT */
	LPGENT("%s has left (%s)"),							/* GEN_MUC_POPUP_LEFT1 */
	LPGENT("%s has disconnected"),						/* GEN_MUC_POPUP_QUIT */
	LPGENT("%s has disconnected (%s)"),					/* GEN_MUC_POPUP_QUIT1 */
	LPGENT("%s is now known as %s"),					/* GEN_MUC_POPUP_NICK */
	LPGENT("%s kicked %s"),								/* GEN_MUC_POPUP_KICK */
	LPGENT("%s kicked %s (%s)"),						/* GEN_MUC_POPUP_KICK1 */
	LPGENT("Notice from %s: %s"),						/* GEN_MUC_POPUP_NOTICE */
	LPGENT("The topic is \'%s\'"),						/* GEN_MUC_POPUP_TOPIC */
	LPGENT("The topic is \'%s\' (set by %s)"),			/* GEN_MUC_POPUP_TOPIC1 */
	LPGENT("%s enables \'%s\' status for %s"),			/* GEN_MUC_POPUP_STATUS_ON */
	LPGENT("%s disables \'%s\' status for %s"),			/* GEN_MUC_POPUP_STATUS_OFF */

	LPGENT("Sound notifications"),						/* CNT_OPT_TITLE_SOUNDS */
	LPGENT("%s    Idle: %dh,%02dm"),					/* GEN_IP_IDLENOTICE */

	/**
	 * template strings for the richedit - based infopanel tooltip.
	 * Please DO NOT mess with the RTF formatting codes or you may destroy the layout of the
	 * tooltips, just translate the strings inside
	 */
	LPGENT("\\tab \\ul\\b Status message:\\ul0\\b0 \\par %s"), 	/* GEN_INFOTIP_STATUSMSG */
	LPGENT("\\par\\par\\tab \\ul\\b Extended status information:\\ul0\\b0 \\par "),		/* GEN_INFOTIP_XSTATUS */
	LPGENT("\\par\\par\\tab \\ul\\b Listening to:\\ul0\\b0 \\par %s"),					/* GEN_INFOTIP_LISTENING */
	LPGENT("\\par\\par\\ul\\b Client:\\ul0\\b0  %s"),									/* GEN_INFOTIP_CLIENT */

	LPGENT("Insert [img] tag / surround selected text with [img][/img]"),				/* GEN_BB_IMGTOOLTIP */
	LPGENT("Original timestamp"),														/* QMGR_COL_ODATE */
	LPGENT("Message text"),																/* QMGR_COL_MESSAGETEXT */
	LPGENT("Status"),																	/* QMGR_COL_STATUS */
	LPGENT("Last send info"),															/* QMGR_COL_LASTSENDINFO */
	LPGENT("<All contacts>"),															/* QMGR_FILTER_ALLCONTACTS */
	LPGENT("Failed"),																	/* QMGR_STATUS_FAILED */
	LPGENT("Sent OK"),																	/* QMGR_STATUS_SENTOK */
	LPGENT("Pending"),																	/* QMGR_STATUS_PENDING */
	LPGENT("Wait ACK"),																	/* QMGR_STATUS_WAITACK */
	LPGENT("Configuration issue|The unattended send feature is disabled. The \\b1 send later\\b0  and \\b1 send to multiple contacts\\b0  features depend on it.\n\nYou must enable it under \\b1Options->Message Sessions->Advanced tweaks\\b0. Changing this option requires a restart."), /* QMGR_ERROR_NOMULTISEND */
	LPGENT("Removed"),																	/* QMGR_STATUS_REMOVED */
	LPGENT("You are about to modify the state of one or more items in the\nunattended send queue. The requested action(s) will be executed at the next scheduled queue processing.\n\nThis action cannot be made undone."),	/* QMGR_WARNING_REMOVAL */
	LPGENT("Queue manager"),															/* QMGR_TITLE */
	LPGENT("Suspended"),																/* QMGR_STATUS_HOLD */
	LPGENT("Deferred"),																	/* QMGR_STATUS_DEFERRED */
	LPGENT("A send later job failed to complete.\nThe original message: %s"),			/* GEN_SQ_SENDLATER_FAILED_POPUP */
	LPGENT("The message cannot be sent delayed or to multiple contacts, because it exceeds the maximum allowed message length of %d bytes"), /* GEN_SQ_SENDLATER_ERROR_MSG_TOO_LONG */

	LPGENT("Default container"),														/* GEN_DEFAULT_CONTAINER_NAME */
	/*
	 * event notification popups
	 */
	LPGENT("No description given"),														/* GEN_STRING_EVENT_FILE_NODESC */
	LPGENT("Incoming file (invalid format"),											/* GEN_STRING_EVENT_FILE_INVALID */
	LPGENT("Incoming file"),															/* GEN_STRING_EVENT_FILE */
	/*
	 * tooltips for static message window buttons
	 */
	LPGENT("Add this contact permanently to your contact list"),						/* GEN_TOOLTIP_ADDCONTACT */
	LPGENT("Do not add this contact permanently"),										/* GEN_TOOLTIP_DONTADD */
	LPGENT("Expand or collapse the side bar"),											/* GEN_TOOLTIP_EXPANDSIDEBAR */

	/*
	 * task bar support (thumbnails)
	 */
	LPGENT("Chat room %s"),																/* GEN_TASKBAR_STRING_CHAT_ROOM */
	LPGENT("Server window"),															/* GEN_TASKBAR_STRING_SERVER_WINDOW */
	LPGENT("%d Unread"),																	/* GEN_TASKBAR_STRING_UNREAD */
	LPGENT("%d User(s)"),																/* GEN_TASKBAR_STRING_USERS */

	LPGENT("Previews not availble when using History++ plugin for message log display."), /* GEN_AEROPEEK_NOHPP */
	LPGENT("TabSRMM warning message"),													  /* GEN_STRING_WARNING_TITLE */
};

/*
 * these strings are used by option pages ONLY
 */

wchar_t* CTranslator::m_OptStrings[OPT_LAST] = {
	LPGENT("Use Global Setting"),									/* OPT_UPREFS_IPGLOBAL */
	LPGENT("Always On"),											/* OPT_UPREFS_ON */
	LPGENT("Always Off"),											/* OPT_UPREFS_OFF */
	LPGENT("Show always (if present)"),								/* OPT_UPREFS_AVON */
	LPGENT("Never show it at all"),									/* OPT_UPREFS_AVOFF */
	LPGENT("Force History++"),										/* OPT_UPREFS_FORCEHPP */
	LPGENT("Force IEView"), 										/* OPT_UPREFS_FORCEIEV */
	LPGENT("Force Default Message Log"),							/* OPT_UPREFS_FORCEDEFAULT */
	LPGENT("Simple Tags (*/_)"), 									/* OPT_UPREFS_SIMPLETAGS */
	LPGENT("BBCode"),												/* OPT_UPREFS_BBCODE */
	LPGENT("Force Off"),											/* OPT_UPREFS_FORMATTING_OFF */
	LPGENT("Use default codepage"),									/* OPT_UPREFS_DEFAULTCP */
	LPGENT("Time zone service is missing"),							/* OPT_UPREFS_NOTZSVC */
	LPGENT("Set messaging options for %s"),							/* OPT_UPREFS_TITLE */
	LPGENT("Message Log"),											/* OPT_UPREFS_MSGLOG */
	LPGENT("General"),												/* OPT_UPREFS_GENERIC */
	LPGENT(""),											/* OPT_AERO_EFFECT_NONE */
	LPGENT(""),											/* OPT_AERO_EFFECT_MILK */
	LPGENT(""), 					/* OPT_AERO_EFFECT_CARBON */
	LPGENT(""), 								/* OPT_AERO_EFFECT_SOLID */
	LPGENT("No border"),											/* OPT_GEN_NONE */
	LPGENT(""),											/* OPT_GEN_AUTO */
	LPGENT(""),												/* OPT_GEN_SUNKEN */
	LPGENT("1 pixel, solid"),										/* OPT_GEN_1PIXEL */
	LPGENT("Rounded (only for internal avatar drawing)"),	/* OPT_GEN_ROUNDED */
	LPGENT("Globally on"),											/* OPT_GEN_GLOBALLY ON */
	LPGENT("On, if present"),										/* OPT_GEN_ON_IF_PRESENT */
	LPGENT("Globally OFF"),											/* OPT_GEN_GLOBALLY_OFF */
	LPGENT("On, if present, always in bottom display"),				/* OPT_GEN_ON_ALWAYS_BOTTOM */
	LPGENT("Don't show them"),										/* OPT_GEN_DONT_SHOW */
	LPGENT("Window layout tweaks"), 								/* OPT_TAB_LAYOUTTWEAKS */
	LPGENT("Load and apply"),										/* OPT_TAB_SKINLOAD */
	LPGENT("Set panel visibility for this %s"),						/* OPT_IPANEL_VISIBILTY_TITLE */
	LPGENT("contact"), 												/* OPT_IPANEL_VISIBILTY_IM */
	LPGENT("chat room"),											/* OPT_IPANEL_VISIBILTY_CHAT */
	LPGENT("Do not synchronize the panel height with IM windows"),  /* OPT_IPANEL_SYNC_TITLE_IM */
	LPGENT("Do not synchronize the panel height with group chat windows"),  /* OPT_IPANEL_SYNC_TITLE_MUC */
	LPGENT("Inherit from container setting"), 						/* OPT_IPANEL_VIS_INHERIT */
	LPGENT("Always off"),											/* OPT_IPANEL_VIS_OFF */
	LPGENT("Always on"),											/* OPT_IPANEL_VIS_ON*/
	LPGENT("Use default size"), 									/* OPT_IPANEL_SIZE_GLOBAL */
	LPGENT("Use private size"), 									/* OPT_IPANEL_SIZE_PRIVATE */
	LPGENT("Off"),													/* OPT_GEN_OFF */
	LPGENT("BBCode"),												/* OPT_GEN_BBCODE */
	LPGENT("Default"),												/* OPT_LOG_DEFAULT */
	LPGENT("IEView plugin"),										/* OPT_LOG_IEVIEW */
	LPGENT("History++ plugin"),										/* OPT_LOG_HPP */
	LPGENT("** New contacts **"),									/* OPT_MTN_NEW */
	LPGENT("** Unknown contacts **"),								/* OPT_MTN_UNKNOWN */
	LPGENT("Always"), 												/* OPT_GEN_ALWAYS */
	LPGENT("Always, but no popup when window is focused"),			/* OPT_MTN_NOTFOUCSED */
	LPGENT("Only when no message window is open"),					/* OPT_MTN_ONLYCLOSED */
	LPGENT("Normal - close tab, if last tab is closed also close the window"), /* OPT_CNT_ESCNORMAL */
	LPGENT("Minimize the window to the task bar"),					/* OPT_CNT_ESCMINIMIZE */
	LPGENT("Close or hide window, depends on the close button setting above"), /* OPT_CNT_ESCCLOSE */
	LPGENT("Show balloon popup (unsupported system)"),				/* OPT_MTN_UNSUPPORTED */
	LPGENT("Choose status modes"),									/* OPT_SMODE_CHOOSE */
	LPGENT("nick of current contact (if defined)"),					/* OPT_MUC_LOGTIP1 */
	LPGENT("protocol name of current contact (if defined). Account name is used when protocol supports multiaccounts"), /* OPT_MUC_LOGTIP2 */
	LPGENT("UserID of current contact (if defined). It is like UIN Number for ICQ, JID for Jabber, etc."), /* OPT_MUC_LOGTIP3 */
	LPGENT("path to root miranda folder"),							/* OPT_MUC_LOGTIP4 */
	LPGENT("path to current miranda profile"),						/* OPT_MUC_LOGTIP5 */
	LPGENT("name of current miranda profile (filename, without extension)"), /* OPT_MUC_LOGTIP6 */
	LPGENT("will return parsed string %miranda_profile%\\Profiles\\%miranda_profilename%"), /* OPT_MUC_LOGTIP7 */
	LPGENT("same as environment variable %APPDATA% for currently logged-on Windows user"), /* OPT_MUC_LOGTIP8 */
	LPGENT("username for currently logged-on Windows user"),		/* OPT_MUC_LOGTIP9 */
	LPGENT("\"My Documents\" folder for currently logged-on Windows user"), /* OPT_MUC_LOGTIP10 */
	LPGENT("\"Desktop\" folder for currently logged-on Windows user"), /* OPT_MUC_LOGTIP11 */
	LPGENT("any environment variable defined in current Windows session (like %systemroot%, %allusersprofile%, etc.)"), /* OPT_MUC_LOGTIP12 */
	LPGENT("day of month, 1-31"),									/* OPT_MUC_LOGTIP13 */
	LPGENT("day of month, 01-31"),									/* OPT_MUC_LOGTIP14 */
	LPGENT("month number, 1-12"),									/* OPT_MUC_LOGTIP15 */
	LPGENT("month number, 01-12"),									/* OPT_MUC_LOGTIP16 */
	LPGENT("abbreviated month name"),								/* OPT_MUC_LOGTIP17 */
	LPGENT("full month name"),										/* OPT_MUC_LOGTIP18 */
	LPGENT("year without century, 01-99"),							/* OPT_MUC_LOGTIP19 */
	LPGENT("year with century, 1901-9999"),							/* OPT_MUC_LOGTIP20 */
	LPGENT("abbreviated weekday name"),								/* OPT_MUC_LOGTIP21 */
	LPGENT("full weekday name"),									/* OPT_MUC_LOGTIP22 */
	LPGENT("Appearance and functionality of chat room windows"), 	/* OPT_MUC_OPTHEADER1 */
	LPGENT("Appearance of the message log"),						/* OPT_MUC_OPTEHADER2 */
	LPGENT("Variables"), 											/* OPT_MUC_VARIABLES */
	LPGENT("Select Folder"),										/* OPT_MUC_SELECTFOLDER */
	LPGENT("No markers"),											/* OPT_MUC_NOMARKERS */
	LPGENT("Show as icons"),										/* OPT_MUC_ASICONS */
	LPGENT("Show as text symbols"),									/* OPT_MUC_ASSYMBOLS */
	LPGENT("Template Set Editor"),									/* OPT_TEMP_TITLE */
	LPGENT("This will reset the template set to the default built-in templates. Are you sure you want to do this?"), /* OPT_TEMP_RESET */
	LPGENT("Template set was successfully reset, please close and reopen all message windows. This template editor window will now close."), /* OPT_TEMP_WASRESET */
	LPGENT("Template editor help"),									/* OPT_TEMP_HELPTITLE */
	LPGENT("General"),												/* OPT_TABS_GENERAL */
	LPGENT("Tabs and layout"),										/* OPT_TABS_TABS */
	LPGENT("Containers"),											/* OPT_TABS_CONTAINERS */
	LPGENT("Message log"),											/* OPT_TABS_LOG */
	LPGENT("Tool bar"),												/* OPT_TABS_TOOLBAR */
	LPGENT("Advanced tweaks"),										/* OPT_TABS_ADVANCED */
	LPGENT("Settings"),												/* OPT_TABS_MUC_SETTINGS */
	LPGENT("Log formatting"),										/* OPT_TABS_MUC_LOG */
	LPGENT("Events and filters"),									/* OPT_TABS_MUC_EVENTS */
	LPGENT("Highlighting"),											/* OPT_TABS_MUC_HIGHLIGHT */
	LPGENT("You have chosen to use an external plugin for displaying the message history in the chat window. Most of the settings on this page are for the standard message log viewer only and will have no effect. To change the appearance of the message log, you must configure either IEView or History++."),									/* OPT_MSGLOG_EXPLAINSETTINGS */
	LPGENT("<no skin>"),											/* OPT_SKIN_NOSKINSELECT */
};

/** IMPORTANT note to translators for translation of the warning dialogs:
 *
 * Make sure to NOT remove the pipe character ( | ) from the strings. This separates the
 * warning title from the actual warning text.
 *
 * Also, do NOT insert multiple | characters in the translated string. Not well-formatted
 * warnings cannot be translated and the plugin will show the untranslated versions.
 *
 * strings marked with a NOT TRANSLATABLE comment cannot be translated at all. This
 * will be used for important and critical error messages only.
 *
 * some strings are empty, this is intentional and used for error messages that share
 * the message with other possible error notifications (popups, tool tips etc.)
 *
 * Entries that do not use the LPGENT() macro are NOT TRANSLATABLE, so don't bother translating them.
 */
wchar_t* CTranslator::m_Warnings[WARN_LAST] = {
	LPGENT("Important release notes|A test warning message"),							/* WARN_TEST */ /* reserved for important notes after upgrade - NOT translatable */
	L"Icon pack version check|The installed icon pack is outdated and might be incompatible with TabSRMM version 3.\n\n\\b1Missing or misplaced icons are possible issues with the currently installed icon pack.\\b0 ",			/* WARN_ICONPACKVERSION */ /* NOT TRANSLATABLE */
	LPGENT("Edit user notes|You are editing the user notes. Click the button again or use the hotkey (default: Alt-N) to save the notes and return to normal messaging mode"),  /* WARN_EDITUSERNOTES */
	L"Missing component|The icon pack is missing. Please install it to the default icons folder.\n\nNo icons will be available",		/* WARN_ICONPACKMISSING */ /* NOT TRANSLATABLE */
	LPGENT("Aero peek warning|You have enabled Aero Peek features and loaded a custom container window skin\n\nThis can result in minor visual anomalies in the live preview feature."),	/* WARN_AEROPEEKSKIN */
	L"TabSRMM group chat module|TabSRMM could not enable its group chat module. The most likely cause is that you have installed and enabled \\b1chat.dll\\b0  or another plugin that provides groupchat services.\n\nShould I try to fix this now \\b1(a restart of Miranda is required to apply these changes)?\\b0", /* WARN_CHAT_ENABLED */ /* NOT TRANSLATABLE */
	L"Filetransfer problem|Sending the image by file transfer failed.\n\nPossible reasons: File transfers not supported, either you or the target contact is offline, or you are invisible and the target contact is not on your visibilty list.", /* WARN_IMGSVC_MISSING */ /* NOT TRANSLATABLE */
	LPGENT("Settings problem|The option \\b1 History->Imitate IEView API\\b0  is enabled and the History++ plugin is active. This can cause problems when using IEView as message log viewer.\n\nShould I correct the option (a restart is required)?"), /* WARN_HPP_APICHECK */
	L" ", /* WARN_NO_SENDLATER */ /*uses QMGR_ERROR_NOMULTISEND */
	LPGENT("Closing Window|You are about to close a window with multiple tabs open.\n\nProceed?"),		/* WARN_CLOSEWINDOW */
	LPGENT("Closing options dialog|To reflect the changes done by importing a theme in the options dialog, the dialog must be closed after loading a theme \\b1 and unsaved changes might be lost\\b0 .\n\nDo you want to continue?"), /* WARN_OPTION_CLOSE */
	LPGENT("Loading a theme|Loading a color and font theme can overwrite the settings defined by your skin.\n\nDo you want to continue?"), /* WARN_THEME_OVERWRITE */
};

wchar_t* CTranslator::m_translated[STR_LAST];
wchar_t* CTranslator::m_OptTranslated[OPT_LAST];
wchar_t* CTranslator::m_WarningsTranslated[WARN_LAST];

TOptionListGroup CTranslator::m_lvGroupsModPlus[] = {
	0, LPGENT("Message window tweaks"),
	0, LPGENT("Error feedback"),
	0, NULL
};

TOptionListItem CTranslator::m_lvItemsModPlus[] = {
	0, LPGENT("Show client icon in status bar (fingerprint plugin required)"), 0, LOI_TYPE_SETTING, (UINT_PTR)"adv_ClientIconInStatusBar", 0,
	0, LPGENT("Enable typing sounds"), 0, LOI_TYPE_SETTING, (UINT_PTR)"adv_soundontyping", 0,
	0, LPGENT("Disable animated GIF avatars (will not affect already open message windows)"), 0, LOI_TYPE_SETTING, (UINT_PTR)"adv_DisableAniAvatars", 0,
	0, LPGENT("Close current tab on send"), 0, LOI_TYPE_SETTING, (UINT_PTR)"adv_AutoClose_2", 0,
	0, LPGENT("Disable error popups on sending failures"), 0, LOI_TYPE_SETTING, (UINT_PTR)"adv_noErrorPopups", 1,
	0, LPGENT("Automatic keyboard layout: Do not load the system default for new contacts"), 1, LOI_TYPE_SETTING, (UINT_PTR)"adv_leaveKeyboardAlone", 0,
	0, LPGENT("Enable unattended send (experimental feature, required for multisend and send later) (*)"), 0, LOI_TYPE_SETTING, (UINT_PTR)"sendLaterAvail", 0,
	0, NULL, 0, 0, 0, 0
};

TOptionListItem CTranslator::m_lvItemsNEN [] = {
	0, LPGENT("Show a preview of the event"), IDC_CHKPREVIEW, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bPreview, 1,
	0, LPGENT("Don't announce event when message dialog is open"), IDC_CHKWINDOWCHECK, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bWindowCheck, 1,
	0, LPGENT("Don't announce events from RSS protocols"), IDC_NORSS, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bNoRSS, 1,
	0, LPGENT("Enable the system tray icon"), IDC_ENABLETRAYSUPPORT, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bTraySupport, 2,
	0, LPGENT("Merge new events for the same contact into existing popup"), 1, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bMergePopup, 6,
	0, LPGENT("Show headers"), IDC_CHKSHOWHEADERS, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bShowHeaders, 6,
	0, LPGENT("Dismiss popup"), MASK_DISMISS, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActL, 3,
	0, LPGENT("Open event"), MASK_OPEN, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActL, 3,
	0, LPGENT("Dismiss event"), MASK_REMOVE, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActL, 3,

	0, LPGENT("Dismiss popup"), MASK_DISMISS, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActR, 4,
	0, LPGENT("Open event"), MASK_OPEN, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActR, 4,
	0, LPGENT("Dismiss event"), MASK_REMOVE, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActR, 4,

	0, LPGENT("Dismiss popup"), MASK_DISMISS, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActTE, 5,
	0, LPGENT("Open event"), MASK_OPEN, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActTE, 5,

	0, LPGENT("Disable event notifications for instant messages"), IDC_CHKWINDOWCHECK, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.iDisable, 0,
	0, LPGENT("Disable event notifications for group chats"), IDC_CHKWINDOWCHECK, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.iMUCDisable, 0,
	0, LPGENT("Disable notifications for non-message events"), IDC_CHKWINDOWCHECK, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bDisableNonMessage, 0,

	0, LPGENT("Remove popups for a contact when the message window is focused"), PU_REMOVE_ON_FOCUS, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.dwRemoveMask, 7,
	0, LPGENT("Remove popups for a contact when I start typing a reply"), PU_REMOVE_ON_TYPE, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.dwRemoveMask, 7,
	0, LPGENT("Remove popups for a contact when I send a reply"), PU_REMOVE_ON_SEND, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.dwRemoveMask, 7,

	0, NULL, 0, 0, 0, 0
};

TOptionListGroup CTranslator::m_lvGroupsNEN[] = {
	0, LPGENT("Disable notifications"),
	0, LPGENT("General options"),
	0, LPGENT("System tray icon"),
	0, LPGENT("Left click actions (popups only)"),
	0, LPGENT("Right click actions (popups only)"),
	0, LPGENT("Timeout actions (popups only)"),
	0, LPGENT("Combine notifications for the same contact"),
	0, LPGENT("Remove popups under following conditions"),
	0, NULL
};

TOptionListGroup CTranslator::m_lvGroupsMsg[] = {
	0, LPGENT("Message window behaviour"),
	0, LPGENT("Sending messages"),
	0, LPGENT("Other options"),
	0, NULL
};

TOptionListItem CTranslator::m_lvItemsMsg[] = {
	0, LPGENT("Send on SHIFT - Enter"), 0, LOI_TYPE_SETTING, (UINT_PTR)"sendonshiftenter", 1,
	0, LPGENT("Send message on 'Enter'"), SRMSGDEFSET_SENDONENTER, LOI_TYPE_SETTING, (UINT_PTR)SRMSGSET_SENDONENTER, 1,
	0, LPGENT("Send message on double 'Enter'"), 0, LOI_TYPE_SETTING, (UINT_PTR)"SendOnDblEnter", 1,
	0, LPGENT("Minimize the message window on send"), SRMSGDEFSET_AUTOMIN, LOI_TYPE_SETTING, (UINT_PTR)SRMSGSET_AUTOMIN, 1,
	//Mad
	0, LPGENT("Close the message window on send"), 0, LOI_TYPE_SETTING, (UINT_PTR)"AutoClose", 1,
	//mad_
	0, LPGENT("Always flash contact list and tray icon for new messages"), 0, LOI_TYPE_SETTING, (UINT_PTR)"flashcl", 0,
	0, LPGENT("Delete temporary contacts on close"), 0, LOI_TYPE_SETTING, (UINT_PTR)"deletetemp", 0,
	0, LPGENT("Enable \"Paste and send\" feature"), 0, LOI_TYPE_SETTING, (UINT_PTR)"pasteandsend", 1,
	0, LPGENT("Allow BBCode formatting in outgoing messages"), 0, LOI_TYPE_SETTING, (UINT_PTR)"sendformat", 1,
	0, LPGENT("Automatically split long messages (experimental, use with care)"), 0, LOI_TYPE_SETTING, (UINT_PTR)"autosplit", 2,
	0, NULL, 0, 0, 0, 0
};

TOptionListGroup CTranslator::m_lvGroupsLog[] = {
	0, LPGENT("Message log appearance"),
	0, LPGENT("Support for external plugins"),
	0, LPGENT("Other options"),
	0, LPGENT("Additional events to show"),
	0, LPGENT("Timestamp settings (note: timstamps also depend on your templates)"),
	0, LPGENT("Message log icons"),
	0, NULL
};

TOptionListItem CTranslator::m_lvItemsLog[] = {
	0, LPGENT("Show file events"), 1, LOI_TYPE_SETTING, (UINT_PTR)SRMSGSET_SHOWFILES, 3,
	0, LPGENT("Show timestamps"), 1, LOI_TYPE_FLAG, (UINT_PTR)MWF_LOG_SHOWTIME, 4,
	0, LPGENT("Show dates in timestamps"), 1, LOI_TYPE_FLAG, (UINT_PTR)MWF_LOG_SHOWDATES, 4,
	0, LPGENT("Show seconds in timestamps"), 1, LOI_TYPE_FLAG, (UINT_PTR)MWF_LOG_SHOWSECONDS, 4,
	0, LPGENT("Use contacts local time (if timezone info available)"), 0, LOI_TYPE_FLAG, (UINT_PTR)MWF_LOG_LOCALTIME, 4,
	0, LPGENT("Draw grid lines"), 1, LOI_TYPE_FLAG,  MWF_LOG_GRID, 0,
	0, LPGENT("Event type icons in the message log"), 1, LOI_TYPE_FLAG, MWF_LOG_SHOWICONS, 5,
	0, LPGENT("Text symbols as event markers"), 0, LOI_TYPE_FLAG, MWF_LOG_SYMBOLS, 5,
	0, LPGENT("Use Incoming/Outgoing Icons"), 1, LOI_TYPE_FLAG, MWF_LOG_INOUTICONS, 5,
	0, LPGENT("Use Message Grouping"), 1, LOI_TYPE_FLAG, MWF_LOG_GROUPMODE, 0,
	0, LPGENT("Indent message body"), 1, LOI_TYPE_FLAG, MWF_LOG_INDENT, 0,
	0, LPGENT("Simple text formatting (*bold* etc.)"), 0, LOI_TYPE_FLAG, MWF_LOG_TEXTFORMAT, 0,
	0, LPGENT("Support BBCode formatting"), 1, LOI_TYPE_FLAG, MWF_LOG_BBCODE, 0,
	0, LPGENT("Place a separator in the log after a window lost its foreground status"), 0, LOI_TYPE_SETTING, (UINT_PTR)"usedividers", 0,
	0, LPGENT("Only place a separator when an incoming event is announced with a popup"), 0, LOI_TYPE_SETTING, (UINT_PTR)"div_popupconfig", 0,
	0, LPGENT("RTL is default text direction"), 0, LOI_TYPE_FLAG, MWF_LOG_RTL, 0,
	//0, LPGENT("Support Math Module plugin"), 1, LOI_TYPE_SETTING, (UINT_PTR)"wantmathmod", 1,
//MAD:
	0, LPGENT("Show events at the new line (IEView Compatibility Mode)"), 1, LOI_TYPE_FLAG, MWF_LOG_NEWLINE, 1,
	0, LPGENT("Underline timestamp/nickname (IEView Compatibility Mode)"), 0, LOI_TYPE_FLAG, MWF_LOG_UNDERLINE, 1,
	0, LPGENT("Show timestamp after nickname (IEView Compatibility Mode)"), 0, LOI_TYPE_FLAG, MWF_LOG_SWAPNICK, 1,
//
	0, LPGENT("Log status changes"), 1, LOI_TYPE_FLAG, MWF_LOG_STATUSCHANGES, 2,
	0, LPGENT("Automatically copy selected text"), 0, LOI_TYPE_SETTING, (UINT_PTR)"autocopy", 2,
	0, LPGENT("Use normal templates (uncheck to use simple templates if your template set supports them)"), 1, LOI_TYPE_FLAG, MWF_LOG_NORMALTEMPLATES, 0,
	0, NULL, 0, 0, 0, 0
};

TOptionListGroup CTranslator::m_lvGroupsTab[] = {
	0, LPGENT("Tab options"),
	0, LPGENT("How to create tabs and windows for incoming messages"),
	0, LPGENT("Miscellaneous options"),
	0, NULL
};

TOptionListItem CTranslator::m_lvItemsTab[] = {
	0, LPGENT("Show status text on tabs"), 1, LOI_TYPE_SETTING, (UINT_PTR)"tabstatus", 0,
	0, LPGENT("Prefer xStatus icons when available"), 1, LOI_TYPE_SETTING, (UINT_PTR)"use_xicons", 0,
	0, LPGENT("Detailed tooltip on tabs (requires mToolTip or Tipper plugin)"), 0, LOI_TYPE_SETTING, (UINT_PTR)"d_tooltips", 0,
	0, LPGENT("ALWAYS activate new message sessions (has PRIORITY over the options below)"), SRMSGDEFSET_AUTOPOPUP, LOI_TYPE_SETTING, (UINT_PTR)SRMSGSET_AUTOPOPUP, 1,
	0, LPGENT("Automatically create new message sessions without activating them"), 1, LOI_TYPE_SETTING, (UINT_PTR)"autotabs", 1,
	0, LPGENT("New windows are minimized (the option above MUST be active)"), 1, LOI_TYPE_SETTING, (UINT_PTR)"autocontainer", 1,
	0, LPGENT("Activate a minimized window when a new tab is created inside it"), 0, LOI_TYPE_SETTING, (UINT_PTR)"cpopup", 1,
	0, LPGENT("Automatically switch existing tabs in minimized windows on incoming messages (ignored when using Aero Peek task bar features)"), 1, LOI_TYPE_SETTING, (UINT_PTR)"autoswitchtabs", 1,
	0, LPGENT("Remember and set keyboard layout per contact"), 1, LOI_TYPE_SETTING, (UINT_PTR)"al", 2,
	0, LPGENT("Close button only hides message windows"), 0, LOI_TYPE_SETTING, (UINT_PTR)"hideonclose", 2,
	0, LPGENT("Allow TAB key in typing area (this will disable focus selection by TAB key)"), 0, LOI_TYPE_SETTING, (UINT_PTR)"tabmode", 2,
	0, LPGENT("Add offline contacts to multisend list"),0,LOI_TYPE_SETTING,(UINT_PTR) "AllowOfflineMultisend", 2,
	0, NULL, 0, 0, 0, 0
};


TOptionListItem* CTranslator::getTree(UINT id)
{
	switch(id) {
		case TREE_MODPLUS:
			return(m_lvItemsModPlus);
		case TREE_NEN:
			return(m_lvItemsNEN);
		case TREE_MSG:
			return(m_lvItemsMsg);
		case TREE_LOG:
			return(m_lvItemsLog);
		case TREE_TAB:
			return(m_lvItemsTab);
		default:
			return(0);
	}
}

TOptionListGroup* CTranslator::getGroupTree(UINT id)
{
	switch(id) {
		case TREE_MODPLUS:
			return(m_lvGroupsModPlus);
		case TREE_NEN:
			return(m_lvGroupsNEN);
		case TREE_MSG:
			return(m_lvGroupsMsg);
		case TREE_LOG:
			return(m_lvGroupsLog);
		case TREE_TAB:
			return(m_lvGroupsTab);
		default:
			return(0);
	}
}

void CTranslator::translateGroupTree(TOptionListGroup *lvGroup)
{
	UINT	i = 0;

	while(lvGroup[i].szName) {
		lvGroup[i].szName = TranslateTS(lvGroup[i].szName);
		i++;
	}
}

void CTranslator::translateOptionTree(TOptionListItem *lvItems)
{
	UINT	i = 0;

	while(lvItems[i].szName) {
		lvItems[i].szName = TranslateTS(lvItems[i].szName);
		i++;
	}
}
