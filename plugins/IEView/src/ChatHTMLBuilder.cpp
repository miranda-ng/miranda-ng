/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "ieview_common.h"

#define CHATMOD 			"Chat"
#define CHATFONTMOD 		"ChatFonts"
#define CHAT_ICON_FLAGS		"IconFlags"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE 4

#define FONT_NUM 17

static const char *classNames[] = {
	".timestamp", ".nameIn", ".nameOut", ".userJoined", ".userLeft", ".userDisconnected",
	".userKicked", ".nickChange", ".notice",
	".messageIn", ".messageOut", ".topicChange", ".information", ".statusEnable", ".statusDisable",
	".action", ".highlight"
};

ChatHTMLBuilder::ChatHTMLBuilder()
{
	setLastEventType(-1);
	setLastEventTime(time(NULL));
}

void ChatHTMLBuilder::loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour)
{
	char str[32];
	int style;
	DBVARIANT dbv;
	if (colour) {
		mir_snprintf(str, SIZEOF(str), "Font%dCol", i);
		*colour = db_get_dw(NULL, CHATFONTMOD, str, 0x000000);
	}
	if (lf) {
		mir_snprintf(str, SIZEOF(str), "Font%dSize", i);
		lf->lfHeight = (char)db_get_b(NULL, CHATFONTMOD, str, 10);
		lf->lfHeight = abs(lf->lfHeight);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, SIZEOF(str), "Font%dSty", i);
		style = db_get_b(NULL, CHATFONTMOD, str, 0);
		lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
		lf->lfStrikeOut = 0;
		mir_snprintf(str, SIZEOF(str), "Font%dSet", i);
		lf->lfCharSet = db_get_b(NULL, CHATFONTMOD, str, DEFAULT_CHARSET);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, SIZEOF(str), "Font%d", i);
		if (db_get(NULL, CHATFONTMOD, str, &dbv))
			mir_strcpy(lf->lfFaceName, "Verdana");
		else {
			mir_strncpy(lf->lfFaceName, dbv.pszVal, sizeof(lf->lfFaceName));
			db_free(&dbv);
		}
	}
}

char* ChatHTMLBuilder::timestampToString(time_t time)
{
	static char szResult[512];
	static char str[80];
	char *pszStamp = "[%H:%M]";
	strftime(str, 79, pszStamp, localtime(&time));
	mir_strncpy(szResult, ptrA(mir_utf8encode(str)), 500);
	return szResult;
}

void ChatHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event)
{
	LOGFONTA lf;
	COLORREF color;

	ProtocolSettings *protoSettings = getChatProtocolSettings(event->pszProto);
	if (protoSettings == NULL)
		return;

	if (protoSettings->getChatMode() == Options::MODE_TEMPLATE)
		return;

	CMStringA str;
	if (protoSettings->getChatMode() == Options::MODE_CSS) {
		const char *externalCSS = protoSettings->getChatCssFilename();
		str.AppendFormat("<html><head><link rel=\"stylesheet\" href=\"%s\"/></head><body class=\"body\">\n", externalCSS);
	}
	else {
		HDC hdc = GetDC(NULL);
		int logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
		str.Append("<html><head>");
		str.Append("<style type=\"text/css\">\n");
		COLORREF bkgColor = db_get_dw(NULL, CHATMOD, "BackgroundLog", 0xFFFFFF);
		bkgColor = (((bkgColor & 0xFF) << 16) | (bkgColor & 0xFF00) | ((bkgColor & 0xFF0000) >> 16));
		COLORREF inColor = bkgColor, outColor = bkgColor;
		if (protoSettings->getChatFlags() & Options::LOG_IMAGE_ENABLED) {
			str.AppendFormat(".body {padding: 2px; text-align: left; background-attachment: %s; background-color: #%06X;  background-image: url('%s'); overflow: auto;}\n",
				protoSettings->getChatFlags() & Options::LOG_IMAGE_SCROLL ? "scroll" : "fixed", bkgColor, protoSettings->getChatBackgroundFilename());
		}
		else str.AppendFormat(".body {margin: 0px; text-align: left; background-color: #%06X; overflow: auto;}\n", bkgColor);

		str.Append(".link {color: #0000FF; text-decoration: underline;}\n");
		str.Append(".img {vertical-align: middle;}\n");
		if (protoSettings->getChatFlags() & Options::LOG_IMAGE_ENABLED) {
			str.Append(".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			str.Append(".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
		}
		else {
			str.AppendFormat(".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", inColor);
			str.AppendFormat(".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", outColor);
		}
		for (int i = 0; i < FONT_NUM; i++) {
			loadMsgDlgFont(i, &lf, &color);
			str.AppendFormat("%s {font-family: %s; font-size: %dpt; font-weight: %s; color: #%06X; %s }\n",
				classNames[i],
				lf.lfFaceName,
				abs((signed char)lf.lfHeight) * 74 / logPixelSY,
				lf.lfWeight >= FW_BOLD ? "bold" : "normal",
				(int)(((color & 0xFF) << 16) | (color & 0xFF00) | ((color & 0xFF0000) >> 16)),
				lf.lfItalic ? "font-style: italic;" : "");
		}
		str.Append("</style></head><body class=\"body\">\n");
	}

	if (!str.IsEmpty())
		view->write(str);

	setLastEventType(-1);
}

/* WORK IN PROGRESS:
 * The following method is going to be completely rewritten soon. Do not modify or complain for the time being...
 */

void ChatHTMLBuilder::appendEventNonTemplate(IEView *view, IEVIEWEVENT *event)
{
	DWORD iconFlags = db_get_dw(NULL, CHATMOD, CHAT_ICON_FLAGS, 0);
	IEVIEWEVENTDATA* eventData = event->eventData;
	for (int eventIdx = 0; eventData != NULL && (eventIdx < event->count || event->count == -1); eventData = eventData->next, eventIdx++) {
		const char *iconFile = "";
		DWORD dwData = eventData->dwData;
		bool isSent = eventData->bIsMe != 0;
		const char *className = "";
		bool showIcon = false;

		ptrA szName, szText;
		if (eventData->dwFlags & IEEDF_UNICODE_TEXT)
			szText = encodeUTF8(NULL, event->pszProto, eventData->pszTextW, ENF_ALL | ENF_CHAT_FORMATTING, isSent);
		else
			szText = encodeUTF8(NULL, event->pszProto, (char *)eventData->pszText, ENF_ALL | ENF_CHAT_FORMATTING, isSent);

		if (eventData->dwFlags & IEEDF_UNICODE_NICK)
			szName = encodeUTF8(NULL, event->pszProto, eventData->pszNickW, ENF_NAMESMILEYS, true);
		else
			szName = encodeUTF8(NULL, event->pszProto, (char *)eventData->pszNick, ENF_NAMESMILEYS, true);

		if (eventData->iType == IEED_GC_EVENT_MESSAGE) {
			iconFile = isSent ? "message_out_chat.gif" : "message_in_chat.gif";
			showIcon = (iconFlags & (isSent ? GC_EVENT_MESSAGE : GC_EVENT_MESSAGE)) != 0;
			className = isSent ? "messageOut" : "messageIn";
		}
		else {
			if (eventData->iType == IEED_GC_EVENT_ACTION) {
				iconFile = "action.gif";
				className = "action";
			}
			else if (eventData->iType == IEED_GC_EVENT_JOIN) {
				iconFile = "join.gif";
				className = "userJoined";
			}
			else if (eventData->iType == IEED_GC_EVENT_PART) {
				iconFile = "part.gif";
				className = "userLeft";
			}
			else if (eventData->iType == IEED_GC_EVENT_QUIT) {
				iconFile = "quit.gif";
				className = "userDisconnected";
			}
			else if (eventData->iType == IEED_GC_EVENT_NICK) {
				iconFile = "nick.gif";
				className = "nickChange";
			}
			else if (eventData->iType == IEED_GC_EVENT_KICK) {
				iconFile = "kick.gif";
				className = "userKicked";
			}
			else if (eventData->iType == IEED_GC_EVENT_NOTICE) {
				iconFile = "notice.gif";
				className = "notice";
			}
			else if (eventData->iType == IEED_GC_EVENT_TOPIC) {
				iconFile = "topic.gif";
				className = "topicChange";
			}
			else if (eventData->iType == IEED_GC_EVENT_ADDSTATUS) {
				iconFile = "addstatus.gif";
				className = "statusEnable";
			}
			else if (eventData->iType == IEED_GC_EVENT_REMOVESTATUS) {
				iconFile = "removestatus.gif";
				className = "statusDisable";
			}
			else if (eventData->iType == IEED_GC_EVENT_INFORMATION) {
				iconFile = "info.gif";
				className = "information";
			}
		}
		
		CMStringA str(FORMAT, "<div class=\"%s\">", isSent ? "divOut" : "divIn");
		if (dwData & IEEDD_GC_SHOW_ICON)
			Utils::appendIcon(str, iconFile);

		if (dwData & IEEDD_GC_SHOW_TIME)
			str.AppendFormat("<span class=\"timestamp\">%s </span>", timestampToString(eventData->time));

		if ((dwData & IEEDD_GC_SHOW_NICK) && eventData->iType == IEED_GC_EVENT_MESSAGE)
			str.AppendFormat("<span class=\"%s\">%s: </span>", isSent ? "nameOut" : "nameIn", szName);

		if (dwData & IEEDD_GC_MSG_ON_NEW_LINE)
			str.Append("<br>");

		str.AppendFormat("<span class=\"%s\">", className);
		str.Append(szText);
		str.Append("</span></div>\n");

		if (!str.IsEmpty())
			view->write(str);
	}
}

void ChatHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event)
{
	ProtocolSettings *protoSettings = getChatProtocolSettings(event->pszProto);
	if (protoSettings != NULL)
		appendEventNonTemplate(view, event);
}

bool ChatHTMLBuilder::isDbEventShown(DBEVENTINFO*)
{
	return true;
}
