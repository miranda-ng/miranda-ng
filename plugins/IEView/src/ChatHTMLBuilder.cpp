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

char *ChatHTMLBuilder::timestampToString(time_t time)
{
	static char szResult[512];
	static char str[80];
	char *pszStamp = "[%H:%M]";
	//InitSetting( &g_Settings.pszTimeStamp, "HeaderTime", _T("[%H:%M]"));
	strftime(str, 79, pszStamp, localtime(&time));
	mir_strncpy(szResult, ptrA(mir_utf8encode(str)), 500);
	return szResult;
}

void ChatHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event)
{
	LOGFONTA lf;
	COLORREF color;
	char *output = NULL;
	int outputSize;
	ProtocolSettings *protoSettings = getChatProtocolSettings(event->pszProto);
	if (protoSettings == NULL) {
		return;
	}
	if (protoSettings->getChatMode() == Options::MODE_TEMPLATE) {
		//		buildHeadTemplate(view, event);
		return;
	}
	if (protoSettings->getChatMode() == Options::MODE_CSS) {
		const char *externalCSS = protoSettings->getChatCssFilename();
		Utils::appendText(&output, &outputSize, "<html><head><link rel=\"stylesheet\" href=\"%s\"/></head><body class=\"body\">\n", externalCSS);
	}
	else {
		HDC hdc = GetDC(NULL);
		int logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
		Utils::appendText(&output, &outputSize, "<html><head>");
		Utils::appendText(&output, &outputSize, "<style type=\"text/css\">\n");
		COLORREF bkgColor = db_get_dw(NULL, CHATMOD, "BackgroundLog", 0xFFFFFF);
		COLORREF inColor, outColor;
		bkgColor = (((bkgColor & 0xFF) << 16) | (bkgColor & 0xFF00) | ((bkgColor & 0xFF0000) >> 16));
		inColor = outColor = bkgColor;
		if (protoSettings->getChatFlags() & Options::LOG_IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".body {padding: 2px; text-align: left; background-attachment: %s; background-color: #%06X;  background-image: url('%s'); overflow: auto;}\n",
				protoSettings->getChatFlags() & Options::LOG_IMAGE_SCROLL ? "scroll" : "fixed", (int)bkgColor, protoSettings->getChatBackgroundFilename());
		}
		else {
			Utils::appendText(&output, &outputSize, ".body {margin: 0px; text-align: left; background-color: #%06X; overflow: auto;}\n",
				(int)bkgColor);
		}
		Utils::appendText(&output, &outputSize, ".link {color: #0000FF; text-decoration: underline;}\n");
		Utils::appendText(&output, &outputSize, ".img {vertical-align: middle;}\n");
		if (protoSettings->getChatFlags() & Options::LOG_IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
		}
		else {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)inColor);
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)outColor);
		}
		for (int i = 0; i < FONT_NUM; i++) {
			loadMsgDlgFont(i, &lf, &color);
			Utils::appendText(&output, &outputSize, "%s {font-family: %s; font-size: %dpt; font-weight: %s; color: #%06X; %s }\n",
				classNames[i],
				lf.lfFaceName,
				abs((signed char)lf.lfHeight) * 74 / logPixelSY,
				lf.lfWeight >= FW_BOLD ? "bold" : "normal",
				(int)(((color & 0xFF) << 16) | (color & 0xFF00) | ((color & 0xFF0000) >> 16)),
				lf.lfItalic ? "font-style: italic;" : "");
		}
		Utils::appendText(&output, &outputSize, "</style></head><body class=\"body\">\n");
	}
	if (output != NULL) {
		view->write(output);
		free(output);
	}
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
		//DWORD dwFlags = eventData->dwFlags;
		const char *iconFile = "";
		DWORD dwData = eventData->dwData;
		bool isSent = eventData->bIsMe != 0;
		int outputSize = 0;
		char *output = NULL;
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
		Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divOut" : "divIn");
		if (dwData & IEEDD_GC_SHOW_ICON) {
			Utils::appendIcon(&output, &outputSize, iconFile);
		}
		if (dwData & IEEDD_GC_SHOW_TIME) {
			Utils::appendText(&output, &outputSize, "<span class=\"timestamp\">%s </span>", timestampToString(eventData->time));
		}
		if ((dwData & IEEDD_GC_SHOW_NICK) && eventData->iType == IEED_GC_EVENT_MESSAGE) {
			Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: </span>",
				isSent ? "nameOut" : "nameIn", szName);
		}
		if (dwData & IEEDD_GC_MSG_ON_NEW_LINE) {
			Utils::appendText(&output, &outputSize, "<br>");
		}
		Utils::appendText(&output, &outputSize, "<span class=\"%s\">", className);
		Utils::appendText(&output, &outputSize, "%s", szText);
		Utils::appendText(&output, &outputSize, "</span></div>\n");
		if (output != NULL) {
			view->write(output);
			free(output);
		}
	}
}

void ChatHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event)
{
	ProtocolSettings *protoSettings = getChatProtocolSettings(event->pszProto);
	if (protoSettings == NULL) {
		return;
	}
	// 	if (protoSettings->getSRMMMode() == Options::MODE_TEMPLATE) {
	//	appendEventTemplate(view, event);
	//	} else {
	appendEventNonTemplate(view, event);
	//	}
}

bool ChatHTMLBuilder::isDbEventShown(DBEVENTINFO *)
{
	return true;
}
