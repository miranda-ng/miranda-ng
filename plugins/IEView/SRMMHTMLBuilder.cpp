/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2008  Piotr Piastucki

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
#include "SRMMHTMLBuilder.h"

#include "Options.h"
#include "Utils.h"

// srmm stuff
#define SMF_LOG_SHOWNICK 1
#define SMF_LOG_SHOWTIME 2
#define SMF_LOG_SHOWDATES 4
#define SMF_LOG_SHOWICONS 8
#define SMF_LOG_SHOWSTATUSCHANGES 16
#define SRMMMOD "SRMM"

#define SRMSGSET_SHOWLOGICONS      "ShowLogIcon"
#define SRMSGSET_HIDENAMES         "HideNames"
#define SRMSGSET_SHOWTIME          "ShowTime"
#define SRMSGSET_SHOWDATE          "ShowDate"
#define SRMSGSET_SHOWSTATUSCHANGES "ShowStatusChanges"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE 4

#define FONT_NUM 10

static const char *classNames[] = {
	".messageOut", ".messageIn", ".nameOut", ".timeOut", ".colonOut", ".nameIn", ".timeIn", ".colonIn",
	".inputArea", ".notices"
};

bool SRMMHTMLBuilder::isDbEventShown(DBEVENTINFO * dbei)
{
	switch (dbei->eventType) {
		case EVENTTYPE_MESSAGE:
			return 1;
		case EVENTTYPE_STATUSCHANGE:
			if (dbei->flags & DBEF_READ) return 0;
			return 1;
	}
	return 0;
}

void SRMMHTMLBuilder::loadMsgDlgFont(int i, LOGFONTA * lf, COLORREF * colour) {
	char str[32];
	int style;
	DBVARIANT dbv;
	if (colour) {
		wsprintfA(str, "SRMFont%dCol", i);
		*colour = DBGetContactSettingDword(NULL, SRMMMOD, str, 0x000000);
	}
	if (lf) {
		wsprintfA(str, "SRMFont%dSize", i);
		lf->lfHeight = (char) DBGetContactSettingByte(NULL, SRMMMOD, str, 10);
		lf->lfHeight = abs(lf->lfHeight);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		wsprintfA(str, "SRMFont%dSty", i);
		style = DBGetContactSettingByte(NULL, SRMMMOD, str, 0);
		lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
		lf->lfStrikeOut = 0;
		wsprintfA(str, "SRMFont%dSet", i);
		lf->lfCharSet = DBGetContactSettingByte(NULL, SRMMMOD, str, DEFAULT_CHARSET);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wsprintfA(str, "SRMFont%d", i);
		if (DBGetContactSetting(NULL, SRMMMOD, str, &dbv))
			lstrcpyA(lf->lfFaceName, "Verdana");
		else {
			lstrcpynA(lf->lfFaceName, dbv.pszVal, sizeof(lf->lfFaceName));
			DBFreeVariant(&dbv);
		}
	}
}

char *SRMMHTMLBuilder::timestampToString(DWORD dwFlags, time_t check) {
	static char szResult[512];
	char str[80];
	DBTIMETOSTRING dbtts;
	dbtts.cbDest = 70;;
	dbtts.szDest = str;
	szResult[0] = '\0';
	if(!(dwFlags & SMF_LOG_SHOWDATES)) {
		dbtts.szFormat = (char *)"s";
	}
	else {
		dbtts.szFormat = (char *)"d t";
	}
	CallService(MS_DB_TIME_TIMESTAMPTOSTRING, check, (LPARAM) & dbtts);
	strncat(szResult, str, 500);
	Utils::UTF8Encode(szResult, szResult, 500);
	return szResult;
}


void SRMMHTMLBuilder::buildHead(IEView *view, IEVIEWEVENT *event) {
	LOGFONTA lf;
	COLORREF color;
	char *output = NULL;
	int outputSize;
	ProtocolSettings *protoSettings = getSRMMProtocolSettings(event->hContact);
	if (protoSettings == NULL) {
		return;
	}
 	if (protoSettings->getSRMMMode() == Options::MODE_TEMPLATE) {
		buildHeadTemplate(view, event);
		return;
	}
 	if (protoSettings->getSRMMMode() == Options::MODE_CSS) {
	 	const char *externalCSS = (event->dwFlags & IEEF_RTL) ? protoSettings->getSRMMCssFilenameRtl() : protoSettings->getSRMMCssFilename();
		Utils::appendText(&output, &outputSize, "<html><head><link rel=\"stylesheet\" href=\"%s\"/></head><body class=\"body\">\n",externalCSS);
	} else {
		HDC hdc = GetDC(NULL);
		int logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
		Utils::appendText(&output, &outputSize, "<html><head><style type=\"text/css\">\n");
		COLORREF bkgColor = DBGetContactSettingDword(NULL, SRMMMOD, "BkgColour", 0xFFFFFF);
		COLORREF inColor, outColor;
		bkgColor= (((bkgColor & 0xFF) << 16) | (bkgColor & 0xFF00) | ((bkgColor & 0xFF0000) >> 16));
		inColor = outColor = bkgColor;
		if (protoSettings->getSRMMFlags() & Options::LOG_IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".body {margin: 0px; text-align: left; background-attachment: %s; background-color: #%06X;  background-image: url('%s'); overflow: auto;}\n",
			protoSettings->getSRMMFlags() & Options::LOG_IMAGE_SCROLL ? "scroll" : "fixed", (int) bkgColor, protoSettings->getSRMMBackgroundFilename());
		} else {
			Utils::appendText(&output, &outputSize, ".body {margin: 0px; text-align: left; background-color: #%06X; overflow: auto;}\n",
				 		 (int) bkgColor);
		}
		Utils::appendText(&output, &outputSize, ".link {color: #0000FF; text-decoration: underline;}\n");
		Utils::appendText(&output, &outputSize, ".img {vertical-align: middle;}\n");
		if (protoSettings->getSRMMFlags() & Options::LOG_IMAGE_ENABLED) {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word;}\n");
		} else {
			Utils::appendText(&output, &outputSize, ".divIn {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) inColor);
			Utils::appendText(&output, &outputSize, ".divOut {padding-left: 2px; padding-right: 2px; word-wrap: break-word; background-color: #%06X;}\n", (int) outColor);
		}
	 	for(int i = 0; i < FONT_NUM; i++) {
			loadMsgDlgFont(i, &lf, &color);
			Utils::appendText(&output, &outputSize, "%s {font-family: %s; font-size: %dpt; font-weight: %s; color: #%06X; %s }\n",
			classNames[i],
			lf.lfFaceName,
			abs((signed char)lf.lfHeight) *  74 /logPixelSY ,
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

}

void SRMMHTMLBuilder::appendEventNonTemplate(IEView *view, IEVIEWEVENT *event) {

	DWORD dwFlags = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTIME, 0) ? SMF_LOG_SHOWTIME : 0;
	dwFlags |= !DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_HIDENAMES, 0) ? SMF_LOG_SHOWNICK : 0;
	dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWDATE, 0) ? SMF_LOG_SHOWDATES : 0;
	dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWLOGICONS, 0) ? SMF_LOG_SHOWICONS : 0;
	dwFlags |= DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWSTATUSCHANGES, 0) ? SMF_LOG_SHOWSTATUSCHANGES : 0;

	char *szRealProto = getRealProto(event->hContact);
	IEVIEWEVENTDATA* eventData = event->eventData;
	for (int eventIdx = 0; eventData!=NULL && (eventIdx < event->count || event->count==-1); eventData = eventData->next, eventIdx++) {
		int outputSize;
		char *output;
		output = NULL;
		int isSent = eventData->dwFlags & IEEDF_SENT;
		int isRTL = eventData->dwFlags & IEEDF_RTL;
		showColon = false;

		if (eventData->iType == IEED_EVENT_MESSAGE || eventData->iType == IEED_EVENT_STATUSCHANGE) {
			char *szName = NULL;
			char *szText = NULL;
			if (eventData->dwFlags & IEEDF_UNICODE_NICK) {
				szName = encodeUTF8(eventData->pszNickW, szRealProto, ENF_NAMESMILEYS);
			} else {
				szName = encodeUTF8(eventData->pszNick, szRealProto, ENF_NAMESMILEYS);
			}
			if (eventData->dwFlags & IEEDF_UNICODE_TEXT) {
				szText = encodeUTF8(eventData->pszTextW, szRealProto, ENF_ALL);
			} else {
				szText = encodeUTF8(eventData->pszText, event->codepage, szRealProto, ENF_ALL);
			}

			/* SRMM-specific formatting */
			Utils::appendText(&output, &outputSize, "<div class=\"%s\">", isSent ? "divOut" : "divIn");
			if (dwFlags & SMF_LOG_SHOWICONS) {
				const char *iconFile = "";
				if (dbei.eventType == EVENTTYPE_MESSAGE) {
					iconFile = isSent ? "message_out.gif" : "message_in.gif";
				} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
					iconFile = "status.gif";
				}
				Utils::appendText(&output, &outputSize, "<img class=\"img\" src=\"%s/plugins/ieview/%s\"/>",
								workingDir, iconFile);
			}
			if (dwFlags & SMF_LOG_SHOWTIME) {
				const char *className = "";
				className = isSent ? "timeOut" : "timeIn";
				if (!(dwFlags & SMF_LOG_SHOWNICK) || (dbei.eventType == EVENTTYPE_STATUSCHANGE)) {
					const char *className2 = "";
					className2 = isSent ? "colonOut" : "colonIn";
					Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span><span class=\"%s\">: </span>",
							className, timestampToString(dwFlags, dbei.timestamp), className2);
				} else {
					Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s </span>",
							className, timestampToString(dwFlags, dbei.timestamp));
				}
			}
			if (dwFlags & SMF_LOG_SHOWNICK) {
				if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
					Utils::appendText(&output, &outputSize, "<span class=\"notices\">%s </span>", szName);
				} else {
					Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span><span class=\"%s\">: </span>",
								isSent ? "nameOut" : "nameIn", szName, isSent ? "colonOut" : "colonIn");
				}
			}
			const char *className = "";
			if (dbei.eventType == EVENTTYPE_MESSAGE) {
				className = isSent ? "messageOut" : "messageIn";
			} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
				className = "notices";
			}
			Utils::appendText(&output, &outputSize, "<span class=\"%s\">%s</span>", className, szText);
			Utils::appendText(&output, &outputSize, "</div>\n");
			event->hDbEventFirst = hCurDbEvent;
			if (szName!=NULL) delete szName;
			if (szText!=NULL) delete szText;
		}
		if (output != NULL) {
			view->write(output);
			free(output);
		}
	}
	if (szRealProto!=NULL) delete szRealProto;
}

void SRMMHTMLBuilder::appendEvent(IEView *view, IEVIEWEVENT *event) {
	ProtocolSettings *protoSettings = getSRMMProtocolSettings(event->hContact);
	if (protoSettings == NULL) {
		return;
	}
 	if (protoSettings->getSRMMMode() == Options::MODE_TEMPLATE) {
		appendEventTemplate(view, event, protoSettings);
	} else {
		appendEventNonTemplate(view, event);
	}
}
