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

#define MUCCMOD 			"MUCC"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE 4

#define FONT_NUM 9

static const char *classNames[] = {
	".timestamp", ".nameIn", ".nameOut", ".messageIn", ".messageOut", ".userJoined", ".userLeft", ".topicChange",
	".error"
};

MUCCHTMLBuilder::MUCCHTMLBuilder()
{
	setLastEventType(-1);
	setLastEventTime(time(NULL));
}

void MUCCHTMLBuilder::loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour)
{
	char str[32];
	int style;
	DBVARIANT dbv;
	if (colour) {
		mir_snprintf(str, SIZEOF(str), "Font%dCol", i);
		*colour = db_get_dw(NULL, MUCCMOD, str, 0x000000);
	}
	if (lf) {
		mir_snprintf(str, SIZEOF(str), "Font%dSize", i);
		lf->lfHeight = db_get_b(NULL, MUCCMOD, str, 10);
		lf->lfHeight = abs(lf->lfHeight);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, SIZEOF(str), "Font%dStyle", i);
		style = db_get_b(NULL, MUCCMOD, str, 0);
		lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
		lf->lfStrikeOut = 0;
		mir_snprintf(str, SIZEOF(str), "Font%dSet", i);
		lf->lfCharSet = db_get_b(NULL, MUCCMOD, str, DEFAULT_CHARSET);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, SIZEOF(str), "Font%dFace", i);
		if (db_get(NULL, MUCCMOD, str, &dbv))
			mir_strcpy(lf->lfFaceName, "Verdana");
		else {
			mir_strncpy(lf->lfFaceName, dbv.pszVal, sizeof(lf->lfFaceName));
			db_free(&dbv);
		}
	}
}

char *MUCCHTMLBuilder::timestampToString(DWORD dwData, time_t check)
{
	static char szResult[512];
	char str[80];

	DBTIMETOSTRING dbtts;

	dbtts.cbDest = 70;
	dbtts.szDest = str;

	szResult[0] = '\0';
	struct tm tm_now, tm_today;
	time_t now = time(NULL);
	time_t today;
	tm_now = *localtime(&now);
	tm_today = tm_now;
	tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
	today = mktime(&tm_today);
	if (dwData&IEEDD_MUCC_SHOW_DATE && dwData&IEEDD_MUCC_SHOW_TIME) {
		if (dwData&IEEDD_MUCC_LONG_DATE) {
			dbtts.szFormat = dwData&IEEDD_MUCC_SECONDS ? (char *)"D s" : (char *)"D t";
		}
		else {
			dbtts.szFormat = dwData&IEEDD_MUCC_SECONDS ? (char *)"d s" : (char *)"d t";
		}
	}
	else if (dwData&IEEDD_MUCC_SHOW_DATE) {
		dbtts.szFormat = dwData&IEEDD_MUCC_LONG_DATE ? (char *)"D" : (char *)"d";
	}
	else if (dwData&IEEDD_MUCC_SHOW_TIME) {
		dbtts.szFormat = dwData&IEEDD_MUCC_SECONDS ? (char *)"s" : (char *)"t";
	}
	else {
		dbtts.szFormat = (char *)"";
	}
	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM)& dbtts);
	strncat(szResult, str, 500);
	mir_strncpy(szResult, ptrA(mir_utf8encode(szResult)), 500);
	return szResult;
}

void MUCCHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event)
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
		COLORREF bkgColor = db_get_dw(NULL, MUCCMOD, "BackgroundLog", 0xFFFFFF);
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
			Utils::appendText(&output, &outputSize, ".divUserJoined {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divUserLeft {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divTopicChange {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
		}
		else {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)inColor);
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)outColor);
			Utils::appendText(&output, &outputSize, ".divUserJoined {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)inColor);
			Utils::appendText(&output, &outputSize, ".divUserLeft {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)inColor);
			Utils::appendText(&output, &outputSize, ".divTopicChange {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)inColor);
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

void MUCCHTMLBuilder::appendEventNonTemplate(IEView *view, IEVIEWEVENT *event)
{
	IEVIEWEVENTDATA* eventData = event->eventData;
	for (int eventIdx = 0; eventData != NULL && (eventIdx < event->count || event->count == -1); eventData = eventData->next, eventIdx++) {
		DWORD dwData = eventData->dwData;
		char *style = NULL;
		int  styleSize;
		bool isSent = eventData->bIsMe != 0;
		int  outputSize;
		char *output = NULL;
		ptrA szName, szText;
		if (eventData->iType == IEED_MUCC_EVENT_MESSAGE) {
			if (eventData->dwFlags & IEEDF_UNICODE_TEXT)
				szText = encodeUTF8(NULL, event->pszProto, eventData->pszTextW, ENF_ALL, isSent);
			else
				szText = encodeUTF8(NULL, event->pszProto, eventData->pszText, ENF_ALL, isSent);

			if (eventData->dwFlags & IEEDF_UNICODE_NICK)
				szName = encodeUTF8(NULL, event->pszProto, eventData->pszNickW, ENF_NAMESMILEYS, true);
			else
				szName = encodeUTF8(NULL, event->pszProto, eventData->pszNick, ENF_NAMESMILEYS, true);

			Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divOut" : "divIn");
			if (dwData & IEEDD_MUCC_SHOW_TIME || dwData & IEEDD_MUCC_SHOW_DATE)
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s </span>",
				isSent ? "timestamp" : "timestamp", timestampToString(dwData, eventData->time));

			if (dwData & IEEDD_MUCC_SHOW_NICK)
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: </span>",
				isSent ? "nameOut" : "nameIn", szName);

			if (dwData & IEEDD_MUCC_MSG_ON_NEW_LINE)
				Utils::appendText(&output, &outputSize, "<br>");

			const char *className = isSent ? "messageOut" : "messageIn";
			if (eventData->dwFlags & IEEDF_FORMAT_SIZE && eventData->fontSize > 0)
				Utils::appendText(&style, &styleSize, "font-size:%dpt;", eventData->fontSize);

			if (eventData->dwFlags & IEEDF_FORMAT_COLOR && eventData->color != 0xFFFFFFFF)
				Utils::appendText(&style, &styleSize, "color:#%06X;", ((eventData->color & 0xFF) << 16) | (eventData->color & 0xFF00) | ((eventData->color & 0xFF0000) >> 16));

			if (eventData->dwFlags & IEEDF_FORMAT_FONT)
				Utils::appendText(&style, &styleSize, "font-family:%s;", eventData->fontName);

			if (eventData->dwFlags & IEEDF_FORMAT_STYLE) {
				Utils::appendText(&style, &styleSize, "font-weight: %s;", eventData->fontStyle & IE_FONT_BOLD ? "bold" : "normal");
				Utils::appendText(&style, &styleSize, "font-style: %s;", eventData->fontStyle & IE_FONT_ITALIC ? "italic" : "normal");
				Utils::appendText(&style, &styleSize, "text-decoration: %s;", eventData->fontStyle & IE_FONT_UNDERLINE ? "underline" : "none");
			}
			Utils::appendText(&output, &outputSize, "<span class=\"%s\"><span style=\"%s\">%s</span></span>", className, style != NULL ? style : "", szText);
			Utils::appendText(&output, &outputSize, "</div>\n");
			if (style != NULL)
				free(style);
		}
		else if (eventData->iType == IEED_MUCC_EVENT_JOINED || eventData->iType == IEED_MUCC_EVENT_LEFT || eventData->iType == IEED_MUCC_EVENT_TOPIC) {
			const char *className, *divName, *eventText;
			if (eventData->iType == IEED_MUCC_EVENT_JOINED) {
				className = "userJoined";
				divName = "divUserJoined";
				eventText = LPGEN("%s has joined.");
				szText = encodeUTF8(NULL, event->pszProto, eventData->pszNick, ENF_NONE, isSent);
			}
			else if (eventData->iType == IEED_MUCC_EVENT_LEFT) {
				className = "userLeft";
				divName = "divUserJoined";
				eventText = LPGEN("%s has left.");
				szText = encodeUTF8(NULL, event->pszProto, eventData->pszNick, ENF_NONE, isSent);
			}
			else {
				className = "topicChange";
				divName = "divTopicChange";
				eventText = LPGEN("The topic is %s.");
				szText = encodeUTF8(NULL, event->pszProto, eventData->pszText, ENF_ALL, isSent);
			}
			Utils::appendText(&output, &outputSize, "<div class=\"%s\">", divName);
			if (dwData & IEEDD_MUCC_SHOW_TIME || dwData & IEEDD_MUCC_SHOW_DATE)
				Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s </span>",
				isSent ? "timestamp" : "timestamp", timestampToString(dwData, eventData->time));

			Utils::appendText(&output, &outputSize, "<span class=\"%s\">", className);
			Utils::appendText(&output, &outputSize, Translate(eventText), szText);
			Utils::appendText(&output, &outputSize, "</span>");
			Utils::appendText(&output, &outputSize, "</div>\n");
		}
		else if (eventData->iType == IEED_MUCC_EVENT_ERROR) {
			const char *className = "error";
			szText = encodeUTF8(NULL, event->pszProto, eventData->pszText, ENF_NONE, isSent);
			Utils::appendText(&output, &outputSize, "<div class=\"%s\">", "divError");
			Utils::appendText(&output, &outputSize, "<span class=\"%s\"> %s: %s</span>", className, Translate("Error"), szText);
			Utils::appendText(&output, &outputSize, "</div>\n");
		}

		if (output != NULL) {
			view->write(output);
			free(output);
		}
	}
}

void MUCCHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event)
{
	ProtocolSettings *protoSettings = getChatProtocolSettings(event->pszProto);
	if (protoSettings != NULL)
		appendEventNonTemplate(view, event);
}

bool MUCCHTMLBuilder::isDbEventShown(DBEVENTINFO *)
{
	return true;
}
