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

#include "stdafx.h"

// srmm stuff
#define SMF_LOG_SHOWNICK 1
#define SMF_LOG_SHOWTIME 2
#define SMF_LOG_SHOWDATE 4
#define SMF_LOG_SHOWICONS 8
#define SMF_LOG_SHOWSTATUSCHANGES 16
#define SMF_LOG_SHOWSECONDS 32
#define SMF_LOG_USERELATIVEDATE 64
#define SMF_LOG_USELONGDATE 128
#define SMF_LOG_GROUPMESSAGES	256
#define SMF_LOG_MARKFOLLOWUPS	512
#define SMF_LOG_MSGONNEWLINE 	1024
#define SMF_LOG_DRAWLINES	   2048

#define SRMMMOD "SRMM"

#define SRMSGSET_SHOWLOGICONS      "ShowLogIcon"
#define SRMSGSET_HIDENAMES         "HideNames"
#define SRMSGSET_SHOWTIME          "ShowTime"
#define SRMSGSET_SHOWDATE          "ShowDate"
#define SRMSGSET_SHOWSTATUSCHANGES "ShowStatusChanges"
#define SRMSGSET_SHOWSECONDS       "ShowSeconds"
#define SRMSGSET_USERELATIVEDATE   "UseRelativeDate"
#define SRMSGSET_USELONGDATE  	   "UseLongDate"
#define SRMSGSET_GROUPMESSAGES     "GroupMessages"
#define SRMSGSET_MARKFOLLOWUPS	   "MarkFollowUps"
#define SRMSGSET_MESSAGEONNEWLINE  "MessageOnNewLine"
#define SRMSGSET_DRAWLINES		   "DrawLines"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE 4

#define FONT_NUM 10

static const char *classNames[] = {
	".messageOut", ".messageIn", ".nameOut", ".timeOut", ".colonOut", ".nameIn", ".timeIn", ".colonIn",
	".inputArea", ".notices"
};

ScriverHTMLBuilder::ScriverHTMLBuilder()
{
	setLastEventType(-1);
	setLastEventTime(time(0));
	startedTime = time(0);
}

bool ScriverHTMLBuilder::isDbEventShown(DBEVENTINFO * dbei)
{
	switch (dbei->eventType) {
	case EVENTTYPE_MESSAGE:
		return 1;
	case EVENTTYPE_FILE:
		return 1;
	default:
		return Utils::DbEventIsForMsgWindow(dbei);
	}
}

void ScriverHTMLBuilder::loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour)
{
	char str[32];
	int style;
	DBVARIANT dbv;
	if (colour) {
		mir_snprintf(str, "SRMFont%dCol", i);
		*colour = db_get_dw(0, SRMMMOD, str, 0x000000);
	}
	if (lf) {
		mir_snprintf(str, "SRMFont%dSize", i);
		lf->lfHeight = (char)db_get_b(0, SRMMMOD, str, 10);
		lf->lfHeight = abs(lf->lfHeight);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, "SRMFont%dSty", i);
		style = db_get_b(0, SRMMMOD, str, 0);
		lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
		lf->lfStrikeOut = 0;
		mir_snprintf(str, "SRMFont%dSet", i);
		lf->lfCharSet = db_get_b(0, SRMMMOD, str, DEFAULT_CHARSET);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, "SRMFont%d", i);
		if (db_get_s(0, SRMMMOD, str, &dbv))
			strncpy_s(lf->lfFaceName, "Verdana", _TRUNCATE);
		else {
			strncpy_s(lf->lfFaceName, dbv.pszVal, _TRUNCATE);
			db_free(&dbv);
		}
	}
}

char* ScriverHTMLBuilder::timestampToString(uint32_t dwFlags, time_t check, int mode)
{
	static char szResult[512]; szResult[0] = '\0';
	char str[80];
	char format[20]; format[0] = '\0';

	if ((mode == 0 || mode == 1) && (dwFlags & SMF_LOG_SHOWDATE)) {
		struct tm tm_now, tm_today;
		time_t now = time(0);
		time_t today;
		tm_now = *localtime(&now);
		tm_today = tm_now;
		tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
		today = mktime(&tm_today);

		if (dwFlags & SMF_LOG_USERELATIVEDATE && check >= today) {
			strncpy(szResult, Translate("Today"), _countof(szResult)-1);
			if (mode == 0) {
				mir_strcat(szResult, ",");
			}
		}
		else if (dwFlags & SMF_LOG_USERELATIVEDATE && check > (today - 86400)) {
			strncpy(szResult, Translate("Yesterday"), _countof(szResult)-1);
			if (mode == 0) {
				mir_strcat(szResult, ",");
			}
		}
		else {
			if (dwFlags & SMF_LOG_USELONGDATE)
				strncpy_s(format, "D", _TRUNCATE);
			else
				strncpy_s(format, "d", _TRUNCATE);
		}
	}
	if (mode == 0 || mode == 2) {
		if (mode == 0 && (dwFlags & SMF_LOG_SHOWDATE))
			mir_strcat(format, " ");

		mir_strcat(format, (dwFlags & SMF_LOG_SHOWSECONDS) ? "s" : "t");
	}
	if (format[0] != '\0') {
		TimeZone_ToString(check, format, str, _countof(str));
		mir_strncat(szResult, str, _countof(szResult) - mir_strlen(szResult));
	}
	mir_strncpy(szResult, ptrA(mir_utf8encode(szResult)), 500);
	return szResult;
}

void ScriverHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event)
{
	LOGFONTA lf;
	COLORREF color;

	ProtocolSettings *protoSettings = getSRMMProtocolSettings(event->hContact);
	if (protoSettings == nullptr)
		return;

	if (protoSettings->getSRMMMode() == Options::MODE_TEMPLATE) {
		buildHeadTemplate(view, event, protoSettings);
		return;
	}
	
	CMStringA str;
	if (protoSettings->getSRMMMode() == Options::MODE_CSS) {
		const char *externalCSS = protoSettings->getSRMMCssFilename();
		if (strncmp(externalCSS, "http://", 7))
			str.AppendFormat("<html><head><link rel=\"stylesheet\" href=\"file://%s\"/></head><body class=\"body\">\n", externalCSS);
		else
			str.AppendFormat("<html><head><link rel=\"stylesheet\" href=\"%s\"/></head><body class=\"body\">\n", externalCSS);
	}
	else {
		HDC hdc = GetDC(nullptr);
		int logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(nullptr, hdc);
		str.Append("<html><head>");
		str.Append("<style type=\"text/css\">\n");
		COLORREF bkgColor = db_get_dw(0, SRMMMOD, "BkgColour", 0xFFFFFF);
		COLORREF inColor = db_get_dw(0, SRMMMOD, "IncomingBkgColour", 0xFFFFFF);
		COLORREF outColor = db_get_dw(0, SRMMMOD, "OutgoingBkgColour", 0xFFFFFF);
		COLORREF lineColor = db_get_dw(0, SRMMMOD, "LineColour", 0xFFFFFF);
		bkgColor = (((bkgColor & 0xFF) << 16) | (bkgColor & 0xFF00) | ((bkgColor & 0xFF0000) >> 16));
		inColor = (((inColor & 0xFF) << 16) | (inColor & 0xFF00) | ((inColor & 0xFF0000) >> 16));
		outColor = (((outColor & 0xFF) << 16) | (outColor & 0xFF00) | ((outColor & 0xFF0000) >> 16));
		lineColor = (((lineColor & 0xFF) << 16) | (lineColor & 0xFF00) | ((lineColor & 0xFF0000) >> 16));
		if (protoSettings->getSRMMFlags() & Options::LOG_IMAGE_ENABLED)
			str.AppendFormat(".body {padding: 2px; text-align: left; background-attachment: %s; background-color: #%06X;  background-image: url('%s'); overflow: auto;}\n",
				protoSettings->getSRMMFlags() & Options::LOG_IMAGE_SCROLL ? "scroll" : "fixed", bkgColor, protoSettings->getSRMMBackgroundFilename());
		else
			str.AppendFormat(".body {margin: 0px; text-align: left; background-color: #%06X; overflow: auto;}\n", bkgColor);

		str.Append(".link {color: #0000FF; text-decoration: underline;}\n");
		str.Append(".img {}\n");
		if (protoSettings->getSRMMFlags() & Options::LOG_IMAGE_ENABLED) {
			str.Append(".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			str.Append(".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			str.AppendFormat(".divInGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", lineColor);
			str.AppendFormat(".divOutGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", lineColor);
			str.Append(".divInRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			str.Append(".divOutRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			str.AppendFormat(".divInGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int)lineColor);
			str.AppendFormat(".divOutGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int)lineColor);
		}
		else {
			str.AppendFormat(".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", inColor);
			str.AppendFormat(".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", outColor);
			str.AppendFormat(".divInGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
				lineColor, inColor);
			str.AppendFormat(".divOutGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
				lineColor, outColor);
			str.AppendFormat(".divInRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)inColor);
			str.AppendFormat(".divOutRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)outColor);
			str.AppendFormat(".divInGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
				lineColor, inColor);
			str.AppendFormat(".divOutGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
				lineColor, outColor);
		}
		str.Append(".divNotice {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
		str.AppendFormat(".divNoticeGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", lineColor);
		str.Append(".divNoticeRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
		str.AppendFormat(".divNoticeGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int)lineColor);
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

void ScriverHTMLBuilder::appendEventNonTemplate(IEView *view, IEVIEWEVENT *event)
{
	bool showColon;
	uint32_t dwFlags = db_get_b(0, SRMMMOD, SRMSGSET_SHOWTIME, 0) ? SMF_LOG_SHOWTIME : 0;
	dwFlags |= !db_get_b(0, SRMMMOD, SRMSGSET_HIDENAMES, 0) ? SMF_LOG_SHOWNICK : 0;
	dwFlags |= db_get_b(0, SRMMMOD, SRMSGSET_SHOWDATE, 0) ? SMF_LOG_SHOWDATE : 0;
	dwFlags |= db_get_b(0, SRMMMOD, SRMSGSET_SHOWLOGICONS, 0) ? SMF_LOG_SHOWICONS : 0;
	dwFlags |= db_get_b(0, SRMMMOD, SRMSGSET_SHOWSTATUSCHANGES, 0) ? SMF_LOG_SHOWSTATUSCHANGES : 0;
	dwFlags |= db_get_b(0, SRMMMOD, SRMSGSET_SHOWSECONDS, 0) ? SMF_LOG_SHOWSECONDS : 0;
	dwFlags |= db_get_b(0, SRMMMOD, SRMSGSET_USERELATIVEDATE, 0) ? SMF_LOG_USERELATIVEDATE : 0;
	dwFlags |= db_get_b(0, SRMMMOD, SRMSGSET_USELONGDATE, 0) ? SMF_LOG_USELONGDATE : 0;
	dwFlags |= db_get_b(0, SRMMMOD, SRMSGSET_GROUPMESSAGES, 0) ? SMF_LOG_GROUPMESSAGES : 0;
	dwFlags |= db_get_b(0, SRMMMOD, SRMSGSET_MARKFOLLOWUPS, 0) ? SMF_LOG_MARKFOLLOWUPS : 0;
	dwFlags |= db_get_b(0, SRMMMOD, SRMSGSET_MESSAGEONNEWLINE, 0) ? SMF_LOG_MSGONNEWLINE : 0;
	dwFlags |= db_get_b(0, SRMMMOD, SRMSGSET_DRAWLINES, 0) ? SMF_LOG_DRAWLINES : 0;

	IEVIEWEVENTDATA* eventData = event->eventData;
	for (int eventIdx = 0; eventData != nullptr && (eventIdx < event->count || event->count == -1); eventData = eventData->next, eventIdx++) {
		const char *className = "";
		bool isSent = (eventData->dwFlags & IEEDF_SENT) != 0;
		bool isRTL = (eventData->dwFlags & IEEDF_RTL) != 0;
		showColon = false;
		if (eventData->iType == IEED_EVENT_MESSAGE || eventData->iType == IEED_EVENT_STATUSCHANGE || eventData->iType == IEED_EVENT_FILE) {
			int isGroupBreak = TRUE;
			if ((dwFlags & SMF_LOG_GROUPMESSAGES) && eventData->dwFlags == LOWORD(getLastEventType()) &&
				eventData->iType == IEED_EVENT_MESSAGE && HIWORD(getLastEventType()) == IEED_EVENT_MESSAGE &&
				(isSameDate(eventData->time, getLastEventTime())) &&
				(((eventData->time < startedTime) == (getLastEventTime() < startedTime)) || !(eventData->dwFlags & IEEDF_READ))) {
				isGroupBreak = FALSE;
			}
			ptrA szName, szText;
			if (eventData->dwFlags & IEEDF_UNICODE_NICK)
				szName = encodeUTF8(event->hContact, eventData->szNick.w, ENF_NAMESMILEYS, true);
			else
				szName = encodeUTF8(event->hContact, eventData->szNick.a, ENF_NAMESMILEYS, true);

			if (eventData->dwFlags & IEEDF_UNICODE_TEXT)
				szText = encodeUTF8(event->hContact, eventData->szText.w, ENF_ALL, isSent);
			else
				szText = encodeUTF8(event->hContact, eventData->szText.a, event->codepage, ENF_ALL, isSent);

			/* Scriver-specific formatting */
			if ((dwFlags & SMF_LOG_DRAWLINES) && isGroupBreak && getLastEventType() != -1) {
				if (eventData->iType == IEED_EVENT_MESSAGE)
					className = isRTL ? (isSent ? "divOutGridRTL" : "divInGridRTL") : (isSent ? "divOutGrid" : "divInGrid");
				else
					className = isRTL ? "divNoticeGridRTL" : "divNoticeGrid";
			}
			else {
				if (eventData->iType == IEED_EVENT_MESSAGE)
					className = isRTL ? (isSent ? "divOutRTL" : "divInRTL") : (isSent ? "divOut" : "divIn");
				else
					className = isRTL ? "divNoticeRTL" : "divNotice";
			}

			CMStringA str;
			str.AppendFormat("<div class=\"%s\">", className);
			if ((dwFlags & SMF_LOG_SHOWICONS) && isGroupBreak) {
				const char *iconFile = "";
				if (eventData->iType == IEED_EVENT_MESSAGE)
					iconFile = isSent ? "message_out.gif" : "message_in.gif";
				else if (eventData->iType == IEED_EVENT_FILE)
					iconFile = "file.gif";
				else if (eventData->iType == IEED_EVENT_STATUSCHANGE)
					iconFile = "status.gif";

				Utils::appendIcon(str, iconFile);
			}

			if (dwFlags & SMF_LOG_SHOWTIME &&
				(eventData->iType != IEED_EVENT_MESSAGE ||
				(dwFlags & SMF_LOG_MARKFOLLOWUPS || isGroupBreak || !(dwFlags & SMF_LOG_GROUPMESSAGES)))) {
				char* timestampString = nullptr;
				if (dwFlags & SMF_LOG_GROUPMESSAGES) {
					if (isGroupBreak) {
						if (!(dwFlags & SMF_LOG_MARKFOLLOWUPS))
							timestampString = timestampToString(dwFlags, eventData->time, 0);
						else if (dwFlags & SMF_LOG_SHOWDATE)
							timestampString = timestampToString(dwFlags, eventData->time, 1);
					}
					else if (dwFlags & SMF_LOG_MARKFOLLOWUPS)
						timestampString = timestampToString(dwFlags, eventData->time, 2);
				}
				else timestampString = timestampToString(dwFlags, eventData->time, 0);

				if (timestampString != nullptr)
					str.AppendFormat("<span class=\"%s\">%s</span>", isSent ? "timeOut" : "timeIn", timestampString);

				if (eventData->iType != IEED_EVENT_MESSAGE)
					str.AppendFormat("<span class=\"%s\">: </span>", isSent ? "colonOut" : "colonIn");

				showColon = true;
			}
			if ((dwFlags & SMF_LOG_SHOWNICK && eventData->iType == IEED_EVENT_MESSAGE && isGroupBreak) || eventData->iType == IEED_EVENT_STATUSCHANGE) {
				if (eventData->iType == IEED_EVENT_MESSAGE) {
					if (showColon)
						str.AppendFormat("<span class=\"%s\"> %s</span>", isSent ? "nameOut" : "nameIn", szName.get());
					else
						str.AppendFormat("<span class=\"%s\">%s</span>", isSent ? "nameOut" : "nameIn", szName.get());

					showColon = true;
					if (dwFlags & SMF_LOG_GROUPMESSAGES) {
						str.Append("<br>");
						showColon = false;
					}
				}
				else str.AppendFormat("<span class=\"notices\">%s </span>", szName.get());
			}
			if (dwFlags & SMF_LOG_SHOWTIME && dwFlags & SMF_LOG_GROUPMESSAGES && dwFlags & SMF_LOG_MARKFOLLOWUPS
				&& eventData->iType == IEED_EVENT_MESSAGE && isGroupBreak) {
				str.AppendFormat("<span class=\"%s\">%s</span>", isSent ? "timeOut" : "timeIn", timestampToString(dwFlags, eventData->time, 2));
				showColon = true;
			}
			if (showColon && eventData->iType == IEED_EVENT_MESSAGE)
				str.AppendFormat("<span class=\"%s\">: </span>", isSent ? "colonOut" : "colonIn");

			if (eventData->iType == IEED_EVENT_MESSAGE) {
				if (dwFlags & SMF_LOG_MSGONNEWLINE && showColon)
					str.Append("<br>");

				className = isSent ? "messageOut" : "messageIn";
			}
			else className = "notices";

			if (eventData->iType == IEED_EVENT_FILE) {
				str.AppendFormat("<span class=\"%s\">%s: %s</span>", className, 
					(isSent) ? Translate("File sent") : Translate("File received"), szText.get());
			}
			else str.AppendFormat("<span class=\"%s\">%s</span>", className, szText.get());

			str.Append("</div>\n");
			setLastEventType(MAKELONG(eventData->dwFlags, eventData->iType));
			setLastEventTime(eventData->time);

			view->write(str);
		}
	}
	view->documentClose();
}

void ScriverHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event)
{
	ProtocolSettings *protoSettings = getSRMMProtocolSettings(event->hContact);
	if (protoSettings == nullptr)
		return;

	if (protoSettings->getSRMMMode() == Options::MODE_TEMPLATE)
		appendEventTemplate(view, event, protoSettings);
	else
		appendEventNonTemplate(view, event);
}
