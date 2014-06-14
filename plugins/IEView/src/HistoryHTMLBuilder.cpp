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

// srmm stuff
#define SMF_LOG_SHOWNICK 1
#define SMF_LOG_SHOWTIME 2
#define SMF_LOG_SHOWDATES 4
#define SMF_LOG_SHOWICONS 8
#define HPPMOD "HistoryPlusPlus"

#define SRMSGSET_SHOWICONS      "ShowIcons"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE 4

#define DIV_FONT_NUM 7

static const char *divClassNames[] = {
	".divMessageOut", ".divMessageIn",
	".divFileOut", ".divFileIn",
	".divUrlOut", ".divUrlIn",
	".divSystem"
};

static const char *dbDivSettingNames[] = {
	"OutMes", "IncMes",
	"OutFil", "IncFil",
	"OutUrl", "IncUrl",
	"Added"
};

#define SPAN_FONT_NUM 4

static const char *spanClassNames[] = {
	".nameOut", ".nameIn",
	".timeOut", ".timeIn",
};

static const char *dbSpanSettingNames[] = {
	"Profile", "Contact",
	"ProfileDate", "ContactDate",
};

HistoryHTMLBuilder::HistoryHTMLBuilder()
{
	setLastEventType(-1);
	setLastEventTime(time(NULL));
	startedTime = time(NULL);
}

bool HistoryHTMLBuilder::isDbEventShown(DBEVENTINFO * dbei)
{
	switch (dbei->eventType) {
	case EVENTTYPE_MESSAGE:
		return 1;
	default:
		return Utils::DbEventIsForHistory(dbei);
	}
}

char *HistoryHTMLBuilder::timestampToString(DWORD dwFlags, time_t check)
{
	static char szResult[512];
	char str[80];
	DBTIMETOSTRING dbtts;
	dbtts.cbDest = 70;
	dbtts.szDest = str;
	szResult[0] = '\0';
	dbtts.szFormat = (char *)"d t";
	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM) & dbtts);
	strncat(szResult, str, 500);
	lstrcpynA(szResult, ptrA(mir_utf8encode(szResult)), 500);
	return szResult;
}

void HistoryHTMLBuilder::loadMsgDlgFont(const char *dbSetting, LOGFONTA * lf, COLORREF * colour, COLORREF * bkgColour)
{
	char str[128];
	int style;
	DBVARIANT dbv;
	if (bkgColour) {
		mir_snprintf(str, SIZEOF(str), "Back.%s", dbSetting);
		*bkgColour = db_get_dw(NULL, HPPMOD, str, 0xFFFFFF);
	}
	if (colour) {
		mir_snprintf(str, SIZEOF(str), "Font.%s.Color", dbSetting);
		*colour = db_get_dw(NULL, HPPMOD, str, 0x000000);
	}
	if (lf) {
		mir_snprintf(str, SIZEOF(str), "Font.%s.Size", dbSetting);
		lf->lfHeight = (char) db_get_b(NULL, HPPMOD, str, 10);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, SIZEOF(str), "Font.%s.Style.Bold", dbSetting);
		style = db_get_b(NULL, HPPMOD, str, 0);
		lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
		mir_snprintf(str, SIZEOF(str), "Font.%s.Style.Italic", dbSetting);
		style = db_get_b(NULL, HPPMOD, str, 0) << 1;
		lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
		lf->lfStrikeOut = 0;
		mir_snprintf(str, SIZEOF(str), "Font.%s.Charset", dbSetting);
		lf->lfCharSet = db_get_b(NULL, HPPMOD, str, DEFAULT_CHARSET);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, SIZEOF(str), "Font.%s.Name", dbSetting);
		if (db_get(NULL, HPPMOD, str, &dbv))
			lstrcpyA(lf->lfFaceName, "Verdana");
		else {
			lstrcpynA(lf->lfFaceName, dbv.pszVal, sizeof(lf->lfFaceName));
			db_free(&dbv);
		}
	}
}

const char *HistoryHTMLBuilder::getTemplateFilename(ProtocolSettings * protoSettings)
{
	return protoSettings->getHistoryTemplateFilename();
}

int HistoryHTMLBuilder::getFlags(ProtocolSettings * protoSettings)
{
	return protoSettings->getHistoryFlags();
}

void HistoryHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event)
{
	LOGFONTA lf;
	int i;
	COLORREF color, bkgColor;
	char *output = NULL;
	int outputSize;
	ProtocolSettings *protoSettings = getHistoryProtocolSettings(event->hContact);
	if (protoSettings == NULL) {
		return;
	}
 	if (protoSettings->getHistoryMode() == Options::MODE_TEMPLATE) {
		buildHeadTemplate(view, event, protoSettings);
		return;
	}
 	if (protoSettings->getHistoryMode() == Options::MODE_CSS) {
		const char *externalCSS = protoSettings->getHistoryCssFilename();
		Utils::appendText(&output, &outputSize, "<html><head><link rel=\"stylesheet\" href=\"%s\"/></head><body class=\"body\">\n", externalCSS);
	} else {
		Utils::appendText(&output, &outputSize, "<html><head>");
		Utils::appendText(&output, &outputSize, "<style type=\"text/css\">\n");
		COLORREF lineColor = db_get_dw(NULL, HPPMOD, "LineColour", 0xFFFFFF);
		lineColor= 0;//(((lineColor & 0xFF) << 16) | (lineColor & 0xFF00) | ((lineColor & 0xFF0000) >> 16));
		bkgColor = 0xFFFFFF;
		if (protoSettings->getHistoryFlags() & Options::LOG_IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".body {padding: 2px; text-align: left; background-attachment: %s; background-color: #%06X;  background-image: url('%s'); overflow: auto;}\n",
			protoSettings->getHistoryFlags() & Options::LOG_IMAGE_SCROLL ? "scroll" : "fixed", (int) bkgColor, protoSettings->getHistoryBackgroundFilename());
		} else {
			Utils::appendText(&output, &outputSize, ".body {margin: 0px; text-align: left; background-color: #%06X; overflow: auto;}\n",
						(int) bkgColor);
		}
		Utils::appendText(&output, &outputSize, ".link {color: #0000FF; text-decoration: underline;}\n");
		Utils::appendText(&output, &outputSize, ".img {float: left; vertical-align: middle;}\n");
	 	for(i = 0; i < DIV_FONT_NUM; i++) {
			loadMsgDlgFont(dbDivSettingNames[i], &lf, &color, &bkgColor);
			if (protoSettings->getHistoryFlags() & Options::LOG_IMAGE_ENABLED) {
				Utils::appendText(&output, &outputSize, "%s {float: left; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; font-family: %s; font-size: %dpt; font-weight: %s; color: #%06X; %s}\n",
					divClassNames[i],
					(int) lineColor,
					lf.lfFaceName,
					lf.lfHeight,
					lf.lfWeight >= FW_BOLD ? "bold" : "normal",
					(int)(((color & 0xFF) << 16) | (color & 0xFF00) | ((color & 0xFF0000) >> 16)),
					lf.lfItalic ? "font-style: italic;" : "");
			} else {
				Utils::appendText(&output, &outputSize, "%s {float: left; padding-left: 2px; padding-right: 2px; word-wrap: break-word; border-top: 1px solid #%06X; background-color: #%06X; font-family: %s; font-size: %dpt; font-weight: %s; color: #%06X; %s}\n",
					divClassNames[i],
					(int) lineColor,
					(int)(((bkgColor & 0xFF) << 16) | (bkgColor & 0xFF00) | ((bkgColor & 0xFF0000) >> 16)),
					lf.lfFaceName,
					lf.lfHeight,
					lf.lfWeight >= FW_BOLD ? "bold" : "normal",
					(int)(((color & 0xFF) << 16) | (color & 0xFF00) | ((color & 0xFF0000) >> 16)),
					lf.lfItalic ? "font-style: italic;" : "");
			}
		}
		for(i = 0; i < SPAN_FONT_NUM; i++) {
			loadMsgDlgFont(dbSpanSettingNames[i], &lf, &color, NULL);
			Utils::appendText(&output, &outputSize, "%s {float: %s; font-family: %s; font-size: %dpt; font-weight: %s; color: #%06X; %s }\n",
			spanClassNames[i],
			i < 2 ? "left" : "right; clear: right;",
			lf.lfFaceName,
			lf.lfHeight,
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

void HistoryHTMLBuilder::appendEventNonTemplate(IEView *view, IEVIEWEVENT *event)
{
	DWORD dwFlags = db_get_b(NULL, HPPMOD, SRMSGSET_SHOWICONS, 0) ? SMF_LOG_SHOWICONS : 0;
	ptrA szRealProto( getRealProto(event->hContact));
	IEVIEWEVENTDATA* eventData = event->eventData;
	for (int eventIdx = 0; eventData!=NULL && (eventIdx < event->count || event->count==-1); eventData = eventData->next, eventIdx++) {
		int outputSize;
		char *output = NULL;
		bool isSent = (eventData->dwFlags & IEEDF_SENT) != 0;
		bool isRTL = (eventData->dwFlags & IEEDF_RTL) != 0;
		if (eventData->iType == IEED_EVENT_MESSAGE || eventData->iType == IEED_EVENT_STATUSCHANGE ||
				eventData->iType == IEED_EVENT_URL || eventData->iType == IEED_EVENT_FILE)
		{
			ptrA szName, szText;
			if (eventData->dwFlags & IEEDF_UNICODE_NICK)
				szName = encodeUTF8(event->hContact, szRealProto, eventData->pszNickW, ENF_NAMESMILEYS, true);
			else
				szName = encodeUTF8(event->hContact, szRealProto, eventData->pszNick, ENF_NAMESMILEYS, true);

			if (eventData->dwFlags & IEEDF_UNICODE_TEXT)
				szText = encodeUTF8(event->hContact, szRealProto, eventData->pszTextW, eventData->iType == IEED_EVENT_MESSAGE ? ENF_ALL : 0, isSent);
			else
				szText = encodeUTF8(event->hContact, szRealProto, eventData->pszText, event->codepage, eventData->iType == IEED_EVENT_MESSAGE ? ENF_ALL : 0, isSent);

			/* History++-specific formatting */
			const char *className = NULL;
			const char *iconFile = NULL;
			switch (eventData->iType) {
			case IEED_EVENT_SYSTEM:
				Utils::appendText(&output, &outputSize, "<div class=\"%s\">", "divSystem");
				break;
			case IEED_EVENT_FILE:
				iconFile = "file.gif";
				Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divFileOut" : "divFileIn");
				break;
			case IEED_EVENT_URL:
				iconFile = "url.gif";
				Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divUrlOut" : "divUrlIn");
				break;
			default:
				iconFile = "message.gif";
				Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divMessageOut" : "divMessageIn");
			}
			if (dwFlags & SMF_LOG_SHOWICONS && iconFile != NULL)
				Utils::appendIcon(&output, &outputSize, iconFile);
			else
				Utils::appendText(&output, &outputSize, " ");

			Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s:</span>", isSent ? "nameOut" : "nameIn", szName);
			Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span><br>", isSent ? "timeOut" : "timeIn", timestampToString(dwFlags, eventData->time));
			if (eventData->iType == IEED_EVENT_FILE)
				Utils::appendText(&output, &outputSize, "%s:<br> %s", isSent ? Translate("Outgoing File Transfer") : Translate("Incoming File Transfer"), szText);
			else if (eventData->iType == IEED_EVENT_URL)
				Utils::appendText(&output, &outputSize, "%s:<br> %s", isSent ? Translate("URL sent") : Translate("URL received"), szText);
			else
				Utils::appendText(&output, &outputSize, "%s", szText);

			Utils::appendText(&output, &outputSize, "</div>\n");
			setLastEventType(MAKELONG(eventData->dwFlags, eventData->iType));
			setLastEventTime(eventData->time);
		}
		if (output != NULL) {
			view->write(output);
			free(output);
		}
	}
	view->documentClose();
}

void HistoryHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event)
{
	ProtocolSettings *protoSettings = getHistoryProtocolSettings(event->hContact);
	if (protoSettings == NULL) {
		return;
	}
	if (protoSettings->getHistoryMode() & Options::MODE_TEMPLATE) {
		appendEventTemplate(view, event, protoSettings);
	} else{
		appendEventNonTemplate(view, event);
	}
}
