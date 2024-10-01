/*

Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "chat.h"

/////////////////////////////////////////////////////////////////////////////////////////
//	LOGINFO members

LOGINFO::LOGINFO(const GCEVENT *gce) :
	ptszNick(mir_wstrdup(gce->pszNick.w)),
	ptszText(mir_wstrdup(gce->pszText.w)),
	ptszStatus(mir_wstrdup(gce->pszStatus.w)),
	ptszUserInfo(mir_wstrdup(gce->pszUserInfo.w))
{
	time = gce->time;
	iType = gce->iType;
	bIsMe = gce->bIsMe;
}

LOGINFO::~LOGINFO()
{}

int LOGINFO::getIndex() const
{
	switch (iType) {
	case GC_EVENT_MESSAGE:
		if (bIsMe)
			return 10;
		else
			return 9;

	case GC_EVENT_JOIN: return 3;
	case GC_EVENT_PART: return 4;
	case GC_EVENT_QUIT: return 5;
	case GC_EVENT_NICK: return 7;
	case GC_EVENT_KICK: return 6;
	case GC_EVENT_NOTICE: return 8;
	case GC_EVENT_TOPIC: return 11;
	case GC_EVENT_INFORMATION:return 12;
	case GC_EVENT_ADDSTATUS: return 13;
	case GC_EVENT_REMOVESTATUS: return 14;
	case GC_EVENT_ACTION: return 15;
	}
	return 0;
}

int LOGINFO::getIcon() const
{
	switch (iType) {
	case GC_EVENT_MESSAGE:
		if (bIsMe)
			return ICON_MESSAGEOUT;
		else
			return ICON_MESSAGE;

	case GC_EVENT_JOIN: return ICON_JOIN;
	case GC_EVENT_PART: return ICON_PART;
	case GC_EVENT_QUIT: return ICON_QUIT;
	case GC_EVENT_NICK: return ICON_NICK;
	case GC_EVENT_KICK: return ICON_KICK;
	case GC_EVENT_NOTICE: return ICON_NOTICE;
	case GC_EVENT_TOPIC: return ICON_TOPIC;
	case GC_EVENT_INFORMATION:return ICON_INFO;
	case GC_EVENT_ADDSTATUS: return ICON_ADDSTATUS;
	case GC_EVENT_REMOVESTATUS: return ICON_REMSTATUS;
	case GC_EVENT_ACTION: return ICON_ACTION;
	}
	return 0;
}

void LOGINFO::write(RtfChatLogStreamData *streamData, bool /*simpleMode*/, CMStringA &str, const wchar_t *line) const
{
	CMStringA buf;

	for (; *line; line++) {
		if (*line == '\r' && line[1] == '\n') {
			buf.Append("\\par ");
			line++;
		}
		else if (*line == '\n') {
			buf.Append("\\line ");
		}
		/*
		else if (*line == '%' && !simpleMode) {
			switch (*++line) {
			case 'c':
			case 'f':
				if (g_Settings->bStripFormat || streamData->bStripFormat)
					line += 2;

				else if (line[1] != '\0' && line[2] != '\0') {
					wchar_t szTemp3[3], c = *line;
					int col;
					szTemp3[0] = line[1];
					szTemp3[1] = line[2];
					szTemp3[2] = '\0';
					line += 2;

					col = _wtoi(szTemp3);
					col += (OPTIONS_FONTCOUNT + 1);
					buf.AppendFormat((c == 'c') ? "\\cf%u " : "\\highlight%u ", col);
				}
				break;
			case 'C':
			case 'F':
				if (!g_Settings->bStripFormat && !streamData->bStripFormat) {
					int j = bIsHighlighted ? 16 : getIndex();
					if (*line == 'C')
						buf.AppendFormat("\\cf%u ", j + 1);
					else
						buf.AppendFormat("\\highlight0 ");
				}
				break;
			case 'b':
			case 'u':
			case 'i':
				if (!streamData->bStripFormat)
					buf.AppendFormat((*line == 'u') ? "\\%cl " : "\\%c ", *line);
				break;

			case 'B':
			case 'U':
			case 'I':
				if (!streamData->bStripFormat)
					buf.AppendFormat((*line == 'U') ? "\\%cl0 " : "\\%c0 ", tolower(*line));
				break;

			case 'r':
				if (!streamData->bStripFormat)
					buf.AppendFormat("%s ", Log_SetStyle(getIndex()));
				break;

			default:
				buf.AppendChar('%');
				break;
			}
		}
		*/
		else if (*line == '\t' && !streamData->bStripFormat) {
			buf.Append("\\tab ");
		}
		else if ((*line == '\\' || *line == '{' || *line == '}') && !streamData->bStripFormat) {
			buf.AppendChar('\\');
			buf.AppendChar(*line);
		}
		else if (*line > 0 && *line < 128) {
			buf.AppendChar(*line);
		}
		else buf.AppendFormat("\\u%u ?", (uint16_t)*line);
	}

	str += buf;
}
