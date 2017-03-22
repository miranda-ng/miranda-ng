/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// handles pretranslated strings

#include "stdafx.h"

TOptionListGroup m_lvGroupsModPlus[] =
{
	{ 0, LPGENW("Message window tweaks") },
	{ 0, LPGENW("Display metacontact icons") },
	{ 0, LPGENW("Error feedback") },
	{ 0, nullptr }
};

TOptionListItem m_lvItemsModPlus[] =
{
	{ 0, LPGENW("Enable typing sounds"), 0, LOI_TYPE_SETTING, (UINT_PTR)"adv_soundontyping", 0 },
	{ 0, LPGENW("Close current tab on send"), 0, LOI_TYPE_SETTING, (UINT_PTR)"adv_AutoClose_2", 0 },
	{ 0, LPGENW("Automatic keyboard layout: Do not load the system default for new contacts"), 1, LOI_TYPE_SETTING, (UINT_PTR)"adv_leaveKeyboardAlone", 0 },
	{ 0, LPGENW("Enable unattended send (experimental feature, required for multisend and send later) (*)"), 0, LOI_TYPE_SETTING, (UINT_PTR)"sendLaterAvail", 0 },
	{ 0, LPGENW("Show client description in info panel"), 1, LOI_TYPE_SETTING, (UINT_PTR)"ShowClientDescription", 0 },
	{ 0, LPGENW("On tab control"), 1, LOI_TYPE_SETTING, (UINT_PTR)"MetaiconTab", 1 },
	{ 0, LPGENW("On the button bar"), 0, LOI_TYPE_SETTING, (UINT_PTR)"MetaiconBar", 1 },
	{ 0, LPGENW("Disable error popups on sending failures"), 0, LOI_TYPE_SETTING, (UINT_PTR)"adv_noErrorPopups", 2 },
	{ 0, nullptr, 0, 0, 0, 0 }
};

TOptionListItem m_lvItemsNEN[] =
{
	{ 0, LPGENW("Show a preview of the event"), IDC_CHKPREVIEW, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bPreview, 1 },
	{ 0, LPGENW("Don't announce event when message dialog is open"), IDC_CHKWINDOWCHECK, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bWindowCheck, 1 },
	{ 0, LPGENW("Don't announce events from RSS protocols"), IDC_NORSS, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bNoRSS, 1 },
	{ 0, LPGENW("Enable the system tray icon"), IDC_ENABLETRAYSUPPORT, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bTraySupport, 2 },
	{ 0, LPGENW("Merge new events for the same contact into existing popup"), 1, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bMergePopup, 6 },
	{ 0, LPGENW("Show headers"), IDC_CHKSHOWHEADERS, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bShowHeaders, 6 },
	{ 0, LPGENW("Dismiss popup"), MASK_DISMISS, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActL, 3 },
	{ 0, LPGENW("Open event"), MASK_OPEN, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActL, 3 },
	{ 0, LPGENW("Dismiss event"), MASK_REMOVE, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActL, 3 },

	{ 0, LPGENW("Dismiss popup"), MASK_DISMISS, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActR, 4 },
	{ 0, LPGENW("Open event"), MASK_OPEN, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActR, 4 },
	{ 0, LPGENW("Dismiss event"), MASK_REMOVE, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActR, 4 },

	{ 0, LPGENW("Dismiss popup"), MASK_DISMISS, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActTE, 5 },
	{ 0, LPGENW("Open event"), MASK_OPEN, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.maskActTE, 5 },

	{ 0, LPGENW("Disable event notifications for instant messages"), IDC_CHKWINDOWCHECK, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.iDisable, 0 },
	{ 0, LPGENW("Disable event notifications for group chats"), IDC_CHKWINDOWCHECK, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.iMUCDisable, 0 },
	{ 0, LPGENW("Disable notifications for non-message events"), IDC_CHKWINDOWCHECK, LOI_TYPE_SETTING, (UINT_PTR)&nen_options.bDisableNonMessage, 0 },

	{ 0, LPGENW("Remove popups for a contact when the message window is focused"), PU_REMOVE_ON_FOCUS, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.dwRemoveMask, 7 },
	{ 0, LPGENW("Remove popups for a contact when I start typing a reply"), PU_REMOVE_ON_TYPE, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.dwRemoveMask, 7 },
	{ 0, LPGENW("Remove popups for a contact when I send a reply"), PU_REMOVE_ON_SEND, LOI_TYPE_FLAG, (UINT_PTR)&nen_options.dwRemoveMask, 7 },

	{ 0, nullptr, 0, 0, 0, 0 }
};

TOptionListGroup m_lvGroupsNEN[] =
{
	{ 0, LPGENW("Disable notifications") },
	{ 0, LPGENW("General options") },
	{ 0, LPGENW("System tray icon") },
	{ 0, LPGENW("Left click actions (popups only)") },
	{ 0, LPGENW("Right click actions (popups only)") },
	{ 0, LPGENW("Timeout actions (popups only)") },
	{ 0, LPGENW("Combine notifications for the same contact") },
	{ 0, LPGENW("Remove popups under following conditions") },
	{ 0, nullptr }
};

TOptionListGroup m_lvGroupsMsg[] =
{
	{ 0, LPGENW("Message window behavior") },
	{ 0, LPGENW("Sending messages") },
	{ 0, LPGENW("Other options") },
	{ 0, nullptr }
};

TOptionListItem m_lvItemsMsg[] =
{
	{ 0, LPGENW("Send on Shift+Enter"), 0, LOI_TYPE_SETTING, (UINT_PTR)"sendonshiftenter", 1 },
	{ 0, LPGENW("Send message on 'Enter'"), SRMSGDEFSET_SENDONENTER, LOI_TYPE_SETTING, (UINT_PTR)SRMSGSET_SENDONENTER, 1 },
	{ 0, LPGENW("Send message on double 'Enter'"), 0, LOI_TYPE_SETTING, (UINT_PTR)"SendOnDblEnter", 1 },
	{ 0, LPGENW("Minimize the message window on send"), SRMSGDEFSET_AUTOMIN, LOI_TYPE_SETTING, (UINT_PTR)SRMSGSET_AUTOMIN, 1 },
	{ 0, LPGENW("Close the message window on send"), 0, LOI_TYPE_SETTING, (UINT_PTR)"AutoClose", 1 },
	{ 0, LPGENW("Always flash contact list and tray icon for new messages"), 0, LOI_TYPE_SETTING, (UINT_PTR)"flashcl", 0 },
	{ 0, LPGENW("Delete temporary contacts on close"), 0, LOI_TYPE_SETTING, (UINT_PTR)"deletetemp", 0 },
	{ 0, LPGENW("Enable \"Paste and send\" feature"), 0, LOI_TYPE_SETTING, (UINT_PTR)"pasteandsend", 1 },
	{ 0, LPGENW("Allow BBCode formatting in outgoing messages"), 0, LOI_TYPE_SETTING, (UINT_PTR)"sendformat", 1 },
	{ 0, LPGENW("Automatically split long messages (experimental, use with care)"), 0, LOI_TYPE_SETTING, (UINT_PTR)"autosplit", 2 },
	{ 0, LPGENW("Log status changes"), 0, LOI_TYPE_SETTING, (UINT_PTR)"logstatuschanges", 2 },
	{ 0, LPGENW("Automatically copy selected text"), 1, LOI_TYPE_SETTING, (UINT_PTR)"autocopy", 2 },
	{ 0, nullptr, 0, 0, 0, 0 }
};

TOptionListGroup m_lvGroupsLog[] =
{
	{ 0, LPGENW("Message log appearance") },
	{ 0, LPGENW("Support for external plugins") },
	{ 0, LPGENW("Timestamp settings (note: timestamps also depend on your templates)") },
	{ 0, LPGENW("Message log icons") },
	{ 0, nullptr }
};

TOptionListItem m_lvItemsLog[] =
{
	{ 0, LPGENW("Show timestamps"), 1, LOI_TYPE_FLAG, (UINT_PTR)MWF_LOG_SHOWTIME, 2 },
	{ 0, LPGENW("Show dates in timestamps"), 1, LOI_TYPE_FLAG, (UINT_PTR)MWF_LOG_SHOWDATES, 2 },
	{ 0, LPGENW("Show seconds in timestamps"), 1, LOI_TYPE_FLAG, (UINT_PTR)MWF_LOG_SHOWSECONDS, 2 },
	{ 0, LPGENW("Use contacts local time (if timezone info available)"), 0, LOI_TYPE_FLAG, (UINT_PTR)MWF_LOG_LOCALTIME, 2 },
	{ 0, LPGENW("Draw grid lines"), 1, LOI_TYPE_FLAG, MWF_LOG_GRID, 0 },
	{ 0, LPGENW("Event type icons in the message log"), 1, LOI_TYPE_FLAG, MWF_LOG_SHOWICONS, 3 },
	{ 0, LPGENW("Text symbols as event markers"), 0, LOI_TYPE_FLAG, MWF_LOG_SYMBOLS, 3 },
	{ 0, LPGENW("Use incoming/outgoing icons"), 1, LOI_TYPE_FLAG, MWF_LOG_INOUTICONS, 3 },
	{ 0, LPGENW("Use message grouping"), 1, LOI_TYPE_FLAG, MWF_LOG_GROUPMODE, 0 },
	{ 0, LPGENW("Indent message body"), 1, LOI_TYPE_FLAG, MWF_LOG_INDENT, 0 },
	{ 0, LPGENW("Simple text formatting (*bold*, etc.)"), 0, LOI_TYPE_FLAG, MWF_LOG_TEXTFORMAT, 0 },
	{ 0, LPGENW("Support BBCode formatting"), 1, LOI_TYPE_FLAG, MWF_LOG_BBCODE, 0 },
	{ 0, LPGENW("Place a separator in the log after a window lost its foreground status"), 0, LOI_TYPE_SETTING, (UINT_PTR)"usedividers", 0 },
	{ 0, LPGENW("Only place a separator when an incoming event is announced with a popup"), 0, LOI_TYPE_SETTING, (UINT_PTR)"div_popupconfig", 0 },
	{ 0, LPGENW("RTL is default text direction"), 0, LOI_TYPE_FLAG, MWF_LOG_RTL, 0 },
	{ 0, LPGENW("Show events at the new line (IEView Compatibility Mode)"), 1, LOI_TYPE_FLAG, MWF_LOG_NEWLINE, 1 },
	{ 0, LPGENW("Underline timestamp/nickname (IEView Compatibility Mode)"), 0, LOI_TYPE_FLAG, MWF_LOG_UNDERLINE, 1 },
	{ 0, LPGENW("Show timestamp after nickname (IEView Compatibility Mode)"), 0, LOI_TYPE_FLAG, MWF_LOG_SWAPNICK, 1 },
	{ 0, LPGENW("Use normal templates (uncheck to use simple templates if your template set supports them)"), 1, LOI_TYPE_FLAG, MWF_LOG_NORMALTEMPLATES, 0 },
	{ 0, nullptr, 0, 0, 0, 0 }
};

TOptionListGroup m_lvGroupsTab[] =
{
	{ 0, LPGENW("Tab options") },
	{ 0, LPGENW("How to create tabs and windows for incoming messages") },
	{ 0, LPGENW("Miscellaneous options") },
	{ 0, nullptr }
};

TOptionListItem m_lvItemsTab[] =
{
	{ 0, LPGENW("Show status text on tabs"), 1, LOI_TYPE_SETTING, (UINT_PTR)"tabstatus", 0 },
	{ 0, LPGENW("Prefer xStatus icons when available"), 1, LOI_TYPE_SETTING, (UINT_PTR)"use_xicons", 0 },
	{ 0, LPGENW("Detailed tooltip on tabs (requires Tipper plugin)"), 0, LOI_TYPE_SETTING, (UINT_PTR)"d_tooltips", 0 },
	{ 0, LPGENW("ALWAYS activate new message sessions (has PRIORITY over the options below)"), SRMSGDEFSET_AUTOPOPUP, LOI_TYPE_SETTING, (UINT_PTR)SRMSGSET_AUTOPOPUP, 1 },
	{ 0, LPGENW("Automatically create new message sessions without activating them"), 1, LOI_TYPE_SETTING, (UINT_PTR)"autotabs", 1 },
	{ 0, LPGENW("New windows are minimized (the option above MUST be active)"), 1, LOI_TYPE_SETTING, (UINT_PTR)"autocontainer", 1 },
	{ 0, LPGENW("Activate a minimized window when a new tab is created inside it"), 0, LOI_TYPE_SETTING, (UINT_PTR)"cpopup", 1 },
	{ 0, LPGENW("Automatically switch existing tabs in minimized windows on incoming messages (ignored when using Aero Peek task bar features)"), 1, LOI_TYPE_SETTING, (UINT_PTR)"autoswitchtabs", 1 },
	{ 0, LPGENW("Remember and set keyboard layout per contact"), 1, LOI_TYPE_SETTING, (UINT_PTR)"al", 2 },
	{ 0, LPGENW("Close button only hides message windows"), 0, LOI_TYPE_SETTING, (UINT_PTR)"hideonclose", 2 },
	{ 0, LPGENW("Allow TAB key in typing area (this will disable focus selection by TAB key)"), 0, LOI_TYPE_SETTING, (UINT_PTR)"tabmode", 2 },
	{ 0, LPGENW("Add offline contacts to multisend list"), 0, LOI_TYPE_SETTING, (UINT_PTR) "AllowOfflineMultisend", 2 },
	{ 0, nullptr, 0, 0, 0, 0 }
};

TOptionListItem* CTranslator::getTree(UINT id)
{
	switch (id) {
	case TREE_MODPLUS:   return m_lvItemsModPlus;
	case TREE_NEN:       return m_lvItemsNEN;
	case TREE_MSG:       return m_lvItemsMsg;
	case TREE_LOG:       return m_lvItemsLog;
	case TREE_TAB:       return m_lvItemsTab;
	}
	return 0;
}

TOptionListGroup* CTranslator::getGroupTree(UINT id)
{
	switch (id) {
	case TREE_MODPLUS:   return m_lvGroupsModPlus;
	case TREE_NEN:       return m_lvGroupsNEN;
	case TREE_MSG:       return m_lvGroupsMsg;
	case TREE_LOG:       return m_lvGroupsLog;
	case TREE_TAB:       return m_lvGroupsTab;
	}
	return 0;
}
