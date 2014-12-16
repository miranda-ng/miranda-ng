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

// tabsrmm stuff

#define MWF_LOG_SHOWNICK 512
#define MWF_LOG_SHOWTIME 1024
#define MWF_LOG_SHOWSECONDS 2048
#define MWF_LOG_SHOWDATES 4096
#define MWF_LOG_NEWLINE 8192
#define MWF_LOG_INDENT 16384
#define MWF_LOG_RTL 32768
#define MWF_LOG_UNDERLINE 65536
#define MWF_LOG_SWAPNICK 131072
#define MWF_LOG_SHOWICONS 262144

#define MWF_LOG_INDENTWITHTABS 1048576
#define MWF_LOG_SYMBOLS 0x200000
#define MWF_LOG_TEXTFORMAT 0x2000000
#define MWF_LOG_GRID 0x4000000
#define MWF_LOG_INDIVIDUALBKG 0x8000000

#define MWF_DIVIDERWANTED 0x40000000
#define MWF_LOG_GROUPMODE 0x80000000
#define MWF_LOG_LONGDATES 64
#define MWF_LOG_USERELATIVEDATES 1

#define MWF_SHOW_URLEVENTS 1
#define MWF_SHOW_FILEEVENTS 2
#define MWF_SHOW_INOUTICONS 4
#define MWF_SHOW_EMPTYLINEFIX 8
#define MWF_SHOW_MICROLF 16
#define MWF_SHOW_MARKFOLLOWUPTS 32

#define SRMSGMOD "SRMsg"
#define SRMSGMOD_T "Tab_SRMsg"
#define TABSRMM_FONTMODULE "TabSRMM_Fonts"

#define SRMSGSET_SHOWURLS          "ShowURLs"
#define SRMSGSET_SHOWFILES         "ShowFiles"
#define SRMSGSET_SHOWSTATUSCHANGES "ShowFiles"

#define MWF_LOG_DEFAULT (MWF_LOG_SHOWTIME | MWF_LOG_SHOWNICK | MWF_LOG_SHOWDATES)

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE 4

#define FONT_NUM 19

static const char *classNames[] = {
	".messageOut", ".miscOut", ".messageIn", ".miscIn", ".nameOut", ".timeOut", ".nameIn", ".timeIn",
	".hMessageOut", ".hMiscOut", ".hMessageIn", ".hMiscIn", ".hNameOut", ".hTimeOut", ".hNameIn", ".hTimeIn",
	".inputArea", ".statusChange", ".dividers"
};

TabSRMMHTMLBuilder::TabSRMMHTMLBuilder()
{
	setLastEventType(-1);
	setLastEventTime(time(NULL));
	lastEventTime = time(NULL);
	startedTime = time(NULL);
}

bool TabSRMMHTMLBuilder::isDbEventShown(DWORD dwFlags, DBEVENTINFO *dbei)
{
	switch (dbei->eventType) {
	case EVENTTYPE_MESSAGE:
		return 1;
	case EVENTTYPE_URL:
		if (dwFlags & MWF_SHOW_URLEVENTS) return 1;
		break;
	case EVENTTYPE_FILE:
		if (dwFlags & MWF_SHOW_FILEEVENTS) return 1;
		break;

	case EVENTTYPE_CONTACTS:
	case EVENTTYPE_ADDED:
	case EVENTTYPE_AUTHREQUEST:
		return 0;
	default:
		return Utils::DbEventIsForMsgWindow(dbei);
	}
	return 1;
}

bool TabSRMMHTMLBuilder::isDbEventShown(DBEVENTINFO * dbei)
{
	DWORD dwFlags2 = db_get_b(NULL, SRMSGMOD_T, SRMSGSET_SHOWURLS, 0) ? MWF_SHOW_URLEVENTS : 0;
	dwFlags2 |= db_get_b(NULL, SRMSGMOD_T, SRMSGSET_SHOWFILES, 0) ? MWF_SHOW_FILEEVENTS : 0;
	return isDbEventShown(dwFlags2, dbei);
}

void TabSRMMHTMLBuilder::loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour)
{
	char str[32];
	int style;
	DBVARIANT dbv;
	if (colour) {
		mir_snprintf(str, SIZEOF(str), "Font%dCol", i);
		*colour = db_get_dw(NULL, TABSRMM_FONTMODULE, str, 0x000000);
	}
	if (lf) {
		HDC hdc = GetDC(NULL);
		mir_snprintf(str, SIZEOF(str), "Font%dSize", i);
		lf->lfHeight = db_get_b(NULL, TABSRMM_FONTMODULE, str, 10);
		ReleaseDC(NULL, hdc);

		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, SIZEOF(str), "Font%dSty", i);
		style = db_get_b(NULL, TABSRMM_FONTMODULE, str, 0);
		lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
		lf->lfStrikeOut = 0;
		mir_snprintf(str, SIZEOF(str), "Font%dSet", i);
		lf->lfCharSet = db_get_b(NULL, TABSRMM_FONTMODULE, str, DEFAULT_CHARSET);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, SIZEOF(str), "Font%d", i);
		if (db_get(NULL, TABSRMM_FONTMODULE, str, &dbv))
			mir_strcpy(lf->lfFaceName, "Verdana");
		else {
			mir_strncpy(lf->lfFaceName, dbv.pszVal, sizeof(lf->lfFaceName));
			db_free(&dbv);
		}
	}
}

char* TabSRMMHTMLBuilder::timestampToString(DWORD dwFlags, time_t check, int isGroupBreak)
{
	static char szResult[512];
	char str[80];

	DBTIMETOSTRING dbtts;

	struct tm tm_now, tm_today;
	time_t now = time(NULL);
	time_t today;

	dbtts.cbDest = 70;
	dbtts.szDest = str;

	if (!isGroupBreak || !(dwFlags & MWF_LOG_SHOWDATES)) {
		dbtts.szFormat = (dwFlags & MWF_LOG_SHOWSECONDS) ? (char *)"s" : (char *)"t";
		szResult[0] = '\0';
	}
	else {
		tm_now = *localtime(&now);
		tm_today = tm_now;
		tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
		today = mktime(&tm_today);

		if (dwFlags & MWF_LOG_USERELATIVEDATES && check >= today) {
			dbtts.szFormat = (dwFlags & MWF_LOG_SHOWSECONDS) ? (char *)"s" : (char *)"t";
			strcpy(szResult, Translate("Today"));
			strcat(szResult, ", ");
		}
		else if (dwFlags & MWF_LOG_USERELATIVEDATES && check > (today - 86400)) {
			dbtts.szFormat = (dwFlags & MWF_LOG_SHOWSECONDS) ? (char *)"s" : (char *)"t";
			strcpy(szResult, Translate("Yesterday"));
			strcat(szResult, ", ");
		}
		else {
			if (dwFlags & MWF_LOG_LONGDATES)
				dbtts.szFormat = (dwFlags & MWF_LOG_SHOWSECONDS) ? (char *)"D s" : (char *)"D t";
			else
				dbtts.szFormat = (dwFlags & MWF_LOG_SHOWSECONDS) ? (char *)"d s" : (char *)"d t";
			szResult[0] = '\0';
		}
	}
	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM)& dbtts);
	strncat(szResult, str, 500);
	mir_strncpy(szResult, ptrA(mir_utf8encode(szResult)), 500);
	return szResult;
}

void TabSRMMHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event)
{
	LOGFONTA lf;
	COLORREF color;
	char *output = NULL;
	int outputSize;

	ProtocolSettings *protoSettings = getSRMMProtocolSettings(event->hContact);
	if (protoSettings == NULL)
		return;

	if (protoSettings->getSRMMMode() == Options::MODE_TEMPLATE) {
		buildHeadTemplate(view, event, protoSettings);
		return;
	}
	if (protoSettings->getSRMMMode() == Options::MODE_CSS) {
		const char *externalCSS = protoSettings->getSRMMCssFilename();
		if (strncmp(externalCSS, "http://", 7))
			Utils::appendText(&output, &outputSize, "<html><head><link rel=\"stylesheet\" href=\"file://%s\"/></head><body class=\"body\">\n", externalCSS);
		else
			Utils::appendText(&output, &outputSize, "<html><head><link rel=\"stylesheet\" href=\"%s\"/></head><body class=\"body\">\n", externalCSS);
	}
	else {
		HDC hdc = GetDC(NULL);
		int logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
		DWORD dwFlags = db_get_dw(NULL, SRMSGMOD_T, "mwflags", MWF_LOG_DEFAULT);
		Utils::appendText(&output, &outputSize, "<html><head><style type=\"text/css\">\n");
		COLORREF inColor, outColor;
		COLORREF bkgColor = db_get_dw(NULL, TABSRMM_FONTMODULE, "BkgColour", 0xFFFFFF);
		bkgColor = (((bkgColor & 0xFF) << 16) | (bkgColor & 0xFF00) | ((bkgColor & 0xFF0000) >> 16));
		COLORREF gridColor = db_get_dw(NULL, TABSRMM_FONTMODULE, "hgrid", 0xFFFFFF);
		gridColor = (((gridColor & 0xFF) << 16) | (gridColor & 0xFF00) | ((gridColor & 0xFF0000) >> 16));
		if (dwFlags & MWF_LOG_INDIVIDUALBKG) {
			inColor = db_get_dw(NULL, TABSRMM_FONTMODULE, "inbg", RGB(224, 224, 224));
			outColor = db_get_dw(NULL, TABSRMM_FONTMODULE, "outbg", RGB(224, 224, 224));
			inColor = (((inColor & 0xFF) << 16) | (inColor & 0xFF00) | ((inColor & 0xFF0000) >> 16));
			outColor = (((outColor & 0xFF) << 16) | (outColor & 0xFF00) | ((outColor & 0xFF0000) >> 16));
		}
		else inColor = outColor = bkgColor;

		if (protoSettings->getSRMMFlags() & Options::LOG_IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".body {margin: 0px; text-align: left; background-attachment: %s; background-color: #%06X;  background-image: url('%s'); overflow: auto;}\n",
				protoSettings->getSRMMFlags() & Options::LOG_IMAGE_SCROLL ? "scroll" : "fixed", (int)bkgColor, protoSettings->getSRMMBackgroundFilename());
		}
		else Utils::appendText(&output, &outputSize, ".body {margin: 0px; text-align: left; background-color: #%06X; overflow: auto;}\n", (int)bkgColor);

		Utils::appendText(&output, &outputSize, ".link {color: #0000FF; text-decoration: underline;}\n");
		Utils::appendText(&output, &outputSize, ".img {vertical-align: middle;}\n");
		if (protoSettings->getSRMMFlags() & Options::LOG_IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divInGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int)gridColor);
			Utils::appendText(&output, &outputSize, ".divOutGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int)gridColor);
			Utils::appendText(&output, &outputSize, ".divInRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divOutRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divInGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int)gridColor);
			Utils::appendText(&output, &outputSize, ".divOutGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X}\n", (int)gridColor);
		}
		else {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)inColor);
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)outColor);
			Utils::appendText(&output, &outputSize, ".divInGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
				(int)gridColor, (int)inColor);
			Utils::appendText(&output, &outputSize, ".divOutGrid {padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
				(int)gridColor, (int)outColor);
			Utils::appendText(&output, &outputSize, ".divInRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)inColor);
			Utils::appendText(&output, &outputSize, ".divOutRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int)outColor);
			Utils::appendText(&output, &outputSize, ".divInGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
				(int)gridColor, (int)inColor);
			Utils::appendText(&output, &outputSize, ".divOutGridRTL {text-align: right; direction:RTL; unicode-bidi:embed; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X;}\n",
				(int)gridColor, (int)outColor);
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

time_t TabSRMMHTMLBuilder::getStartedTime()
{
	return startedTime;
}

void TabSRMMHTMLBuilder::appendEventNonTemplate(IEView *view, IEVIEWEVENT *event)
{
	DWORD today = (DWORD)time(NULL);
	today = today - today % 86400;
	DWORD dwFlags = db_get_dw(NULL, SRMSGMOD_T, "mwflags", MWF_LOG_DEFAULT);
	DWORD dwFlags2 = db_get_b(NULL, SRMSGMOD_T, SRMSGSET_SHOWURLS, 0) ? MWF_SHOW_URLEVENTS : 0;
	dwFlags2 |= db_get_b(NULL, SRMSGMOD_T, SRMSGSET_SHOWFILES, 0) ? MWF_SHOW_FILEEVENTS : 0;
	dwFlags2 |= db_get_b(NULL, SRMSGMOD_T, "in_out_icons", 0) ? MWF_SHOW_INOUTICONS : 0;
	dwFlags2 |= db_get_b(NULL, SRMSGMOD_T, "emptylinefix", 1) ? MWF_SHOW_EMPTYLINEFIX : 0;
	dwFlags2 |= MWF_SHOW_MICROLF;
	dwFlags2 |= db_get_b(NULL, SRMSGMOD_T, "followupts", 1) ? MWF_SHOW_MARKFOLLOWUPTS : 0;

	char *szRealProto = getRealProto(event->hContact);
	IEVIEWEVENTDATA* eventData = event->eventData;
	for (int eventIdx = 0; eventData != NULL && (eventIdx < event->count || event->count == -1); eventData = eventData->next, eventIdx++) {
		int outputSize;
		char *output = NULL;
		if (eventData->iType == IEED_EVENT_MESSAGE || eventData->iType == IEED_EVENT_FILE || eventData->iType == IEED_EVENT_URL || eventData->iType == IEED_EVENT_STATUSCHANGE) {
			bool isGroupBreak = true;
			bool isSent = (eventData->dwFlags & IEEDF_SENT) != 0;
			bool isRTL = (eventData->dwFlags & IEEDF_RTL) != 0;
			int isHistory = (eventData->time < (DWORD)getStartedTime() && (eventData->dwFlags & IEEDF_READ || eventData->dwFlags & IEEDF_SENT));
			if (dwFlags & MWF_LOG_GROUPMODE && eventData->dwFlags == LOWORD(getLastEventType()) &&
				eventData->iType == IEED_EVENT_MESSAGE && HIWORD(getLastEventType()) == IEED_EVENT_MESSAGE &&
				((eventData->time < today) == (getLastEventTime() < today)) &&
				(((eventData->time < (DWORD)startedTime) == (getLastEventTime() < (DWORD)startedTime)) || !(eventData->dwFlags & IEEDF_READ)))
			{
				isGroupBreak = FALSE;
			}

			ptrA szName, szText;
			if (eventData->dwFlags & IEEDF_UNICODE_NICK)
				szName = encodeUTF8(event->hContact, szRealProto, eventData->pszNickW, ENF_NAMESMILEYS, true);
			else
				szName = encodeUTF8(event->hContact, szRealProto, eventData->pszNick, ENF_NAMESMILEYS, true);

			if (eventData->dwFlags & IEEDF_UNICODE_TEXT)
				szText = encodeUTF8(event->hContact, szRealProto, eventData->pszTextW, eventData->iType == IEED_EVENT_MESSAGE ? ENF_ALL : 0, isSent);
			else
				szText = encodeUTF8(event->hContact, szRealProto, eventData->pszText, event->codepage, eventData->iType == IEED_EVENT_MESSAGE ? ENF_ALL : 0, isSent);

			/* TabSRMM-specific formatting */
			if ((dwFlags & MWF_LOG_GRID) && isGroupBreak && getLastEventType() != -1)
				Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isRTL ? isSent ? "divOutGridRTL" : "divInGridRTL" : isSent ? "divOutGrid" : "divInGrid");
			else
				Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isRTL ? isSent ? "divOutRTL" : "divInRTL" : isSent ? "divOut" : "divIn");

			if (dwFlags & MWF_LOG_SHOWICONS && isGroupBreak) {
				const char *iconFile = "";
				if (eventData->iType == IEED_EVENT_MESSAGE) {
					if (dwFlags2 & MWF_SHOW_INOUTICONS) iconFile = isSent ? "message_out.gif" : "message_in.gif";
					else iconFile = "message.gif";
				}
				else if (eventData->iType == IEED_EVENT_FILE)
					iconFile = "file.gif";
				else if (eventData->iType == IEED_EVENT_URL)
					iconFile = "url.gif";
				else if (eventData->iType == IEED_EVENT_STATUSCHANGE)
					iconFile = "status.gif";

				Utils::appendIcon(&output, &outputSize, iconFile);
			}
			if ((dwFlags & MWF_LOG_SWAPNICK) && (dwFlags & MWF_LOG_SHOWNICK) && isGroupBreak && (eventData->iType != IEED_EVENT_STATUSCHANGE)) {
				const char *className = "";
				if (!isHistory)	className = isSent ? "nameOut" : "nameIn";
				else className = isSent ? "hNameOut" : "hNameIn";
				if (dwFlags & MWF_LOG_UNDERLINE)
					Utils::appendText(&output, &outputSize, "<span class=\"%s\"><u>%s%s</span>",
					className, szName, (dwFlags & MWF_LOG_SHOWTIME) ? " </u>" : "</u>: ");
				else
					Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s%s</span>",
					className, szName, (dwFlags & MWF_LOG_SHOWTIME) ? " " : ": ");
			}
			if (dwFlags & MWF_LOG_SHOWTIME && (isGroupBreak || dwFlags2 & MWF_SHOW_MARKFOLLOWUPTS)) {
				const char *className = "";
				if (!isHistory)	className = isSent ? "timeOut" : "timeIn";
				else className = isSent ? "hTimeOut" : "hTimeIn";
				if (dwFlags & MWF_LOG_UNDERLINE)
					Utils::appendText(&output, &outputSize, "<span class=\"%s\"><u>%s%s</span>",
					className, timestampToString(dwFlags, eventData->time, isGroupBreak),
					(!isGroupBreak || (eventData->iType == IEED_EVENT_STATUSCHANGE) || (dwFlags & MWF_LOG_SWAPNICK) || !(dwFlags & MWF_LOG_SHOWNICK)) ? "</u>: " : " </u>");
				else
					Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s%s</span>",
					className, timestampToString(dwFlags, eventData->time, isGroupBreak),
					(!isGroupBreak || (eventData->iType == IEED_EVENT_STATUSCHANGE) || (dwFlags & MWF_LOG_SWAPNICK) || !(dwFlags & MWF_LOG_SHOWNICK)) ? ": " : " ");
			}
			if ((eventData->iType == IEED_EVENT_STATUSCHANGE) || ((dwFlags & MWF_LOG_SHOWNICK) && !(dwFlags & MWF_LOG_SWAPNICK) && isGroupBreak)) {
				if (eventData->iType == IEED_EVENT_STATUSCHANGE)
					Utils::appendText(&output, &outputSize, "<span class=\"statusChange\">%s </span>", szName);
				else {
					const char *className = "";
					if (!isHistory) className = isSent ? "nameOut" : "nameIn";
					else className = isSent ? "hNameOut" : "hNameIn";
					if (dwFlags & MWF_LOG_UNDERLINE)
						Utils::appendText(&output, &outputSize, "<span class=\"%s\"><u>%s</u>: </span>", className, szName);
					else
						Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s: </span>", className, szName);
				}
			}
			if (dwFlags & MWF_LOG_NEWLINE && eventData->iType != IEED_EVENT_STATUSCHANGE && eventData->iType != IEED_EVENT_ERRMSG && isGroupBreak)
				Utils::appendText(&output, &outputSize, "<br>");

			const char *className = "";
			if (eventData->iType == IEED_EVENT_MESSAGE) {
				if (!isHistory) className = isSent ? "messageOut" : "messageIn";
				else className = isSent ? "hMessageOut" : "hMessageIn";
			}
			else if (eventData->iType == IEED_EVENT_FILE)
				className = isHistory ? "hMiscIn" : "miscIn";
			else if (eventData->iType == IEED_EVENT_URL)
				className = isHistory ? "hMiscIn" : "miscIn";
			else if (eventData->iType == IEED_EVENT_STATUSCHANGE)
				className = "statusChange";

			Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>", className, szText);
			Utils::appendText(&output, &outputSize, "</div>\n");
			setLastEventType(MAKELONG(eventData->dwFlags, eventData->iType));
			setLastEventTime(eventData->time);
		}
		if (output != NULL) {
			view->write(output);
			free(output);
		}
	}

	mir_free(szRealProto);
	view->documentClose();
}

void TabSRMMHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event)
{
	ProtocolSettings *protoSettings = getSRMMProtocolSettings(event->hContact);
	if (protoSettings == NULL)
		return;

	if (protoSettings->getSRMMMode() == Options::MODE_TEMPLATE)
		appendEventTemplate(view, event, protoSettings);
	else
		appendEventNonTemplate(view, event);
}
