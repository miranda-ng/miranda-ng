/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010 Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#include "ieview_common.h"

TemplateHTMLBuilder::TemplateHTMLBuilder()
{
	iLastEventType = -1;
	startedTime = time(NULL);
	lastEventTime = time(NULL);
	groupTemplate = NULL;
	flashAvatarsTime[0] = time(NULL);
	flashAvatarsTime[1] = time(NULL);
	flashAvatars[0] = NULL;
	flashAvatars[1] = NULL;
}

TemplateHTMLBuilder::~TemplateHTMLBuilder()
{
	for (int i = 0; i < 2; i++)
		if (flashAvatars[i] != NULL)
			mir_free(flashAvatars[i]);
}

char* TemplateHTMLBuilder::getAvatar(MCONTACT hContact, const char *szProto)
{
	DBVARIANT dbv;
	TCHAR tmpPath[MAX_PATH];
	TCHAR *result = NULL;

	if (Options::getAvatarServiceFlags() == Options::AVATARSERVICE_PRESENT) {
		avatarCacheEntry *ace;
		if (hContact == NULL)
			ace = (avatarCacheEntry *)CallService(MS_AV_GETMYAVATAR, 0, (LPARAM)szProto);
		else
			ace = (avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);

		if (ace != NULL) {
			if (ace->cbSize == sizeof(avatarCacheEntry))
				result = ace->szFilename;
			else // compatibility: in M0.9 it will always be char*
				MultiByteToWideChar(CP_ACP, 0, (char*)ace->szFilename, -1, tmpPath, SIZEOF(tmpPath));
		}
	}
	if (!db_get_ts(hContact, "ContactPhoto", "File", &dbv)) {
		if (mir_tstrlen(dbv.ptszVal) > 0) {
			TCHAR *ext = _tcsrchr(dbv.ptszVal, '.');
			if (ext && mir_tstrcmpi(ext, _T(".xml")) == 0)
				result = (TCHAR*)getFlashAvatar(dbv.ptszVal, (hContact == NULL) ? 1 : 0);
			else {
				if (result == NULL) {
					/* relative -> absolute */
					mir_tstrcpy(tmpPath, dbv.ptszVal);
					if (_tcsncmp(tmpPath, _T("http://"), 7))
						PathToAbsoluteT(dbv.ptszVal, tmpPath);

					result = tmpPath;
				}
			}
		}
		db_free(&dbv);
	}
	char* res = mir_utf8encodeT(result);
	Utils::convertPath(res);
	return res;
}

const char *TemplateHTMLBuilder::getFlashAvatar(const TCHAR *file, int index)
{
	if (time(NULL) - flashAvatarsTime[index] > 600 || flashAvatars[index] == NULL) {
		if (flashAvatars[index] != NULL) {
			mir_free(flashAvatars[index]);
			flashAvatars[index] = NULL;
		}
		flashAvatarsTime[index] = time(NULL);
		int src = _topen(file, _O_BINARY | _O_RDONLY);
		if (src != -1) {
			char pBuf[2048];
			char *urlBuf;
			_read(src, pBuf, 2048);
			_close(src);
			urlBuf = strstr(pBuf, "<URL>");
			if (urlBuf) {
				flashAvatars[index] = mir_strdup(strtok(urlBuf + 5, "<\t\n\r"));
			}
		}
	}
	return flashAvatars[index];
}

TemplateMap *TemplateHTMLBuilder::getTemplateMap(ProtocolSettings * protoSettings)
{
	return TemplateMap::getTemplateMap(protoSettings->getSRMMTemplateFilename());
}

int TemplateHTMLBuilder::getFlags(ProtocolSettings * protoSettings)
{
	return protoSettings->getSRMMFlags();
}

char *TemplateHTMLBuilder::timestampToString(DWORD dwFlags, time_t check, int mode)
{
	static char szResult[512];
	TCHAR str[300];
	DBTIMETOSTRINGT dbtts;
	dbtts.cbDest = 70;
	dbtts.szDest = str;
	szResult[0] = '\0';
	if (mode) { //time
		dbtts.szFormat = (dwFlags & Options::LOG_SHOW_SECONDS) ? _T("s") : _T("t");
		CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, check, (LPARAM)&dbtts);
	}
	else {//date
		struct tm tm_now, tm_today;
		time_t now = time(NULL);
		time_t today;
		tm_now = *localtime(&now);
		tm_today = tm_now;
		tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
		today = mktime(&tm_today);
		if (dwFlags & Options::LOG_RELATIVE_DATE && check >= today)
			mir_tstrncpy(str, TranslateT("Today"), SIZEOF(str));
		else if (dwFlags & Options::LOG_RELATIVE_DATE && check > (today - 86400))
			mir_tstrncpy(str, TranslateT("Yesterday"), SIZEOF(str));
		else {
			dbtts.szFormat = (dwFlags & Options::LOG_LONG_DATE) ? _T("D") : _T("d");
			CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, check, (LPARAM)& dbtts);
		}
	}

	mir_strncpy(szResult, T2Utf(str), 500);
	return szResult;
}

void TemplateHTMLBuilder::buildHeadTemplate(IEView *view, IEVIEWEVENT *event, ProtocolSettings *protoSettings)
{
	if (protoSettings == NULL)
		return;

	DBVARIANT dbv;
	char tempBase[1024];
	char tempStr[1024];
	char *szNameIn = NULL;
	char *szNameOut = NULL;
	char *szUINIn = NULL;
	char *szUINOut = NULL;
	char *szNickIn = NULL;
	char *szNickOut = NULL;
	char *szStatusMsg = NULL;

	MCONTACT hRealContact = getRealContact(event->hContact);
	char *szRealProto = getProto(hRealContact);
	char *szProto = getProto(event->pszProto, event->hContact);
	tempBase[0] = '\0';

	TemplateMap *tmpm = getTemplateMap(protoSettings);
	if (tmpm == NULL)
		return;

	mir_strcpy(tempBase, "file://");
	mir_strncat(tempBase, tmpm->getFilename(), SIZEOF(tempBase) - mir_strlen(tempBase));
	char *pathrun = tempBase + mir_strlen(tempBase);
	while ((*pathrun != '\\' && *pathrun != '/') && (pathrun > tempBase))
		pathrun--;
	pathrun++;
	*pathrun = '\0';

	char *szBase = mir_utf8encode(tempBase);
	getUINs(event->hContact, szUINIn, szUINOut);
	if (getFlags(protoSettings) & Options::LOG_SHOW_NICKNAMES) {
		szNameOut = getEncodedContactName(NULL, szProto, szRealProto);
		szNameIn = getEncodedContactName(event->hContact, szProto, szRealProto);
	}
	else {
		szNameOut = mir_strdup("&nbsp;");
		szNameIn = mir_strdup("&nbsp;");
	}
	mir_snprintf(tempStr, SIZEOF(tempStr), "%snoavatar.png", tempBase);
	TCHAR szNoAvatarPath[MAX_PATH];
	_tcsncpy_s(szNoAvatarPath, _A2T(protoSettings->getSRMMTemplateFilename()), _TRUNCATE);
	TCHAR *szNoAvatarPathTmp = _tcsrchr(szNoAvatarPath, '\\');
	if (szNoAvatarPathTmp != NULL)
		*szNoAvatarPathTmp = 0;
	mir_tstrcat(szNoAvatarPath, _T("\\noavatar.png"));
	if (_taccess(szNoAvatarPath, 0) == -1)
		mir_snprintf(tempStr, SIZEOF(tempStr), "%snoavatar.jpg", tempBase);
	else
		mir_snprintf(tempStr, SIZEOF(tempStr), "%snoavatar.png", tempBase);
	char *szNoAvatar = mir_utf8encode(tempStr);

	char *szAvatarIn = getAvatar(event->hContact, szRealProto);
	if (szAvatarIn == NULL)
		szAvatarIn = mir_strdup(szNoAvatar);

	char *szAvatarOut = getAvatar(NULL, szRealProto);
	if (szAvatarOut == NULL)
		szAvatarOut = mir_strdup(szNoAvatar);

	if (!db_get(event->hContact, "CList", "StatusMsg", &dbv)) {
		if (mir_strlen(dbv.pszVal) > 0)
			szStatusMsg = mir_utf8encode(dbv.pszVal);
		db_free(&dbv);
	}

	CONTACTINFO ci;
	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = event->hContact;
	ci.szProto = szProto;
	ci.dwFlag = CNF_NICK | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)& ci))
		szNickIn = encodeUTF8(event->hContact, szRealProto, ci.pszVal, ENF_NAMESMILEYS, true);

	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = NULL;
	ci.szProto = szProto;
	ci.dwFlag = CNF_NICK | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)& ci))
		szNickOut = encodeUTF8(event->hContact, szRealProto, ci.pszVal, ENF_NAMESMILEYS, true);

	Template *tmplt = NULL;
	if (tmpm) {
		tmplt = tmpm->getTemplate(((event->dwFlags & IEEF_RTL) && tmpm->isRTL()) ? "HTMLStartRTL" : "HTMLStart");
		if (tmplt == NULL)
			tmplt = tmpm->getTemplate("HTMLStart");
	}

	CMStringA str;

	if (tmplt != NULL) {
		for (Token *token = tmplt->getTokens(); token != NULL; token = token->getNext()) {
			const char *tokenVal;
			tokenVal = NULL;
			switch (token->getType()) {
			case Token::PLAIN:
				tokenVal = token->getText();
				break;
			case Token::BASE:
				tokenVal = szBase;
				break;
			case Token::NAMEIN:
				tokenVal = szNameIn;
				break;
			case Token::NAMEOUT:
				tokenVal = szNameOut;
				break;
			case Token::AVATARIN:
				tokenVal = szAvatarIn;
				break;
			case Token::AVATAROUT:
				tokenVal = szAvatarOut;
				break;
			case Token::PROTO:
				tokenVal = szRealProto;
				break;
			case Token::UININ:
				tokenVal = szUINIn;
				break;
			case Token::UINOUT:
				tokenVal = szUINOut;
				break;
			case Token::STATUSMSG:
				tokenVal = szStatusMsg;
				break;
			case Token::NICKIN:
				tokenVal = szNickIn;
				break;
			case Token::NICKOUT:
				tokenVal = szNickOut;
				break;
			}
			if (tokenVal != NULL) {
				if (token->getEscape())
					str.Append(ptrA(Utils::escapeString(tokenVal)));
				else
					str.Append(tokenVal);
			}
		}
	}

	if (!str.IsEmpty())
		view->write(str);

	mir_free(szBase);
	mir_free(szRealProto);
	mir_free(szProto);
	mir_free(szUINIn);
	mir_free(szUINOut);
	mir_free(szNoAvatar);
	mir_free(szAvatarIn);
	mir_free(szAvatarOut);
	mir_free(szNameIn);
	mir_free(szNameOut);
	mir_free(szNickIn);
	mir_free(szNickOut);
	mir_free(szStatusMsg);

	groupTemplate = NULL;
	iLastEventType = -1;
}

void TemplateHTMLBuilder::appendEventTemplate(IEView *view, IEVIEWEVENT *event, ProtocolSettings* protoSettings)
{
	if (protoSettings == NULL)
		return;

	DBVARIANT dbv;
	char tempBase[1024];
	char tempStr[1024];
	char szCID[32];
	char *szNameIn = NULL;
	char *szNameOut = NULL;
	char *szUIN = NULL;
	char *szUINIn = NULL;
	char *szUINOut = NULL;
	char *szNickIn = NULL;
	char *szNickOut = NULL;
	char *szStatusMsg = NULL;
	char *szAvatar = NULL;
	const char *tmpltName[2];
	bool isGrouping = false;

	MCONTACT hRealContact = getRealContact(event->hContact);
	char *szRealProto = getProto(hRealContact);
	char *szProto = getProto(event->pszProto, event->hContact);
	tempBase[0] = '\0';

	TemplateMap *tmpm = getTemplateMap(protoSettings);
	if (tmpm != NULL) {
		mir_strcpy(tempBase, "file://");
		mir_strcat(tempBase, tmpm->getFilename());
		char* pathrun = tempBase + mir_strlen(tempBase);
		while ((*pathrun != '\\' && *pathrun != '/') && (pathrun > tempBase)) pathrun--;
		pathrun++;
		*pathrun = '\0';
		isGrouping = tmpm->isGrouping();
	}
	char *szBase = mir_utf8encode(tempBase);

	if (event->hContact != NULL)
		getUINs(event->hContact, szUINIn, szUINOut);

	if (event->hContact != NULL) {
		szNameOut = getEncodedContactName(NULL, szProto, szRealProto);
		szNameIn = getEncodedContactName(event->hContact, szProto, szRealProto);
	}
	else {
		szNameOut = mir_strdup("&nbsp;");
		szNameIn = mir_strdup("&nbsp;");
	}

	TCHAR szNoAvatarPath[MAX_PATH];
	_tcsncpy_s(szNoAvatarPath, _A2T(protoSettings->getSRMMTemplateFilename()), _TRUNCATE);
	TCHAR *szNoAvatarPathTmp = _tcsrchr(szNoAvatarPath, '\\');
	if (szNoAvatarPathTmp != NULL)
		*szNoAvatarPathTmp = 0;
	mir_tstrcat(szNoAvatarPath, _T("\\noavatar.png"));
	if (_taccess(szNoAvatarPath, 0) == -1)
		mir_snprintf(tempStr, SIZEOF(tempStr), "%snoavatar.jpg", tempBase);
	else
		mir_snprintf(tempStr, SIZEOF(tempStr), "%snoavatar.png", tempBase);
	char *szNoAvatar = mir_utf8encode(tempStr);

	char *szAvatarIn = NULL;
	if (event->hContact != NULL)
		szAvatarIn = getAvatar(event->hContact, szRealProto);

	if (szAvatarIn == NULL)
		szAvatarIn = mir_strdup(szNoAvatar);

	char *szAvatarOut = getAvatar(NULL, szRealProto);
	if (szAvatarOut == NULL)
		szAvatarOut = mir_strdup(szNoAvatar);

	if (event->hContact != NULL) {
		if (!db_get(event->hContact, "CList", "StatusMsg", &dbv)) {
			if (mir_strlen(dbv.pszVal) > 0)
				szStatusMsg = mir_utf8encode(dbv.pszVal);
			db_free(&dbv);
		}
	}

	CONTACTINFO ci;
	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = event->hContact;
	ci.szProto = szProto;
	ci.dwFlag = CNF_NICK | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci))
		szNickIn = encodeUTF8(event->hContact, szRealProto, ci.pszVal, ENF_NAMESMILEYS, true);

	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = NULL;
	ci.szProto = szProto;
	ci.dwFlag = CNF_NICK | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)& ci))
		szNickOut = encodeUTF8(event->hContact, szRealProto, ci.pszVal, ENF_NAMESMILEYS, true);

	IEVIEWEVENTDATA* eventData = event->eventData;
	for (int eventIdx = 0; eventData != NULL && (eventIdx < event->count || event->count == -1); eventData = eventData->next, eventIdx++) {
		if (eventData->iType == IEED_EVENT_MESSAGE || eventData->iType == IEED_EVENT_STATUSCHANGE || eventData->iType == IEED_EVENT_FILE || eventData->iType == IEED_EVENT_URL || eventData->iType == IEED_EVENT_SYSTEM) {
			CMStringA str;
			bool isSent = (eventData->dwFlags & IEEDF_SENT) != 0;
			bool isRTL = (eventData->dwFlags & IEEDF_RTL) && tmpm->isRTL();
			bool isHistory = (eventData->time < (DWORD)getStartedTime() && (eventData->dwFlags & IEEDF_READ || eventData->dwFlags & IEEDF_SENT));
			bool isGroupBreak = true;
			if ((getFlags(protoSettings) & Options::LOG_GROUP_MESSAGES) && eventData->dwFlags == LOWORD(getLastEventType())
				&& eventData->iType == IEED_EVENT_MESSAGE && HIWORD(getLastEventType()) == IEED_EVENT_MESSAGE
				&& (isSameDate(eventData->time, getLastEventTime()))
				&& (((eventData->time < (DWORD)startedTime) == (getLastEventTime() < (DWORD)startedTime)) || !(eventData->dwFlags & IEEDF_READ)))
				isGroupBreak = false;

			if (isSent) {
				szAvatar = szAvatarOut;
				szUIN = szUINOut;
				mir_snprintf(szCID, SIZEOF(szCID), "%d", 0);
			}
			else {
				szAvatar = szAvatarIn;
				szUIN = szUINIn;
				mir_snprintf(szCID, SIZEOF(szCID), "%d", (int)event->hContact);
			}
			tmpltName[0] = groupTemplate;
			tmpltName[1] = NULL;
			groupTemplate = NULL;

			char *szName = NULL, *szText = NULL, *szFileDesc = NULL;
			if (event->eventData->dwFlags & IEEDF_UNICODE_NICK)
				szName = encodeUTF8(event->hContact, szRealProto, eventData->pszNickW, ENF_NAMESMILEYS, true);
			else
				szName = encodeUTF8(event->hContact, szRealProto, eventData->pszNick, ENF_NAMESMILEYS, true);

			if (eventData->dwFlags & IEEDF_UNICODE_TEXT)
				szText = encodeUTF8(event->hContact, szRealProto, eventData->pszTextW, eventData->iType == IEED_EVENT_MESSAGE ? ENF_ALL : 0, isSent);
			else
				szText = encodeUTF8(event->hContact, szRealProto, eventData->pszText, event->codepage, eventData->iType == IEED_EVENT_MESSAGE ? ENF_ALL : 0, isSent);

			if (eventData->dwFlags & IEEDF_UNICODE_TEXT2)
				szFileDesc = encodeUTF8(event->hContact, szRealProto, eventData->pszText2W, 0, isSent);
			else
				szFileDesc = encodeUTF8(event->hContact, szRealProto, eventData->pszText2, event->codepage, 0, isSent);

			if ((eventData->iType == IEED_EVENT_MESSAGE)) {
				if (!isRTL) {
					if (isGrouping && (getFlags(protoSettings) & Options::LOG_GROUP_MESSAGES)) {
						if (isGroupBreak)
							tmpltName[1] = isHistory ? isSent ? "hMessageOutGroupStart" : "hMessageInGroupStart" : isSent ? "MessageOutGroupStart" : "MessageInGroupStart";
						else
							tmpltName[0] = isHistory ? isSent ? "hMessageOutGroupInner" : "hMessageInGroupInner" : isSent ? "MessageOutGroupInner" : "MessageInGroupInner";

						groupTemplate = isHistory ? isSent ? "hMessageOutGroupEnd" : "hMessageInGroupEnd" : isSent ? "MessageOutGroupEnd" : "MessageInGroupEnd";
					}
					else tmpltName[1] = isHistory ? isSent ? "hMessageOut" : "hMessageIn" : isSent ? "MessageOut" : "MessageIn";
				}
				else {
					if (isGrouping && (getFlags(protoSettings) & Options::LOG_GROUP_MESSAGES)) {
						if (isGroupBreak)
							tmpltName[1] = isHistory ? isSent ? "hMessageOutGroupStartRTL" : "hMessageInGroupStartRTL" : isSent ? "MessageOutGroupStartRTL" : "MessageInGroupStartRTL";
						else
							tmpltName[0] = isHistory ? isSent ? "hMessageOutGroupInnerRTL" : "hMessageInGroupInnerRTL" : isSent ? "MessageOutGroupInnerRTL" : "MessageInGroupInnerRTL";

						groupTemplate = isHistory ? isSent ? "hMessageOutGroupEndRTL" : "hMessageInGroupEndRTL" : isSent ? "MessageOutGroupEndRTL" : "MessageInGroupEndRTL";
					}
					else tmpltName[1] = isHistory ? isSent ? "hMessageOutRTL" : "hMessageInRTL" : isSent ? "MessageOutRTL" : "MessageInRTL";
				}
			}
			else if (eventData->iType == IEED_EVENT_FILE) {
				tmpltName[1] = isHistory ? isSent ? "hFileOut" : "hFileIn" : isSent ? "FileOut" : "FileIn";
				Template *tmplt = (tmpm == NULL) ? NULL : tmpm->getTemplate(tmpltName[1]);
				if (tmplt == NULL)
					tmpltName[1] = isHistory ? "hFile" : "File";
			}
			else if (eventData->iType == IEED_EVENT_URL) {
				tmpltName[1] = isHistory ? isSent ? "hURLOut" : "hURLIn" : isSent ? "URLOut" : "URLIn";
				Template *tmplt = (tmpm == NULL) ? NULL : tmpm->getTemplate(tmpltName[1]);
				if (tmplt == NULL)
					tmpltName[1] = isHistory ? "hURL" : "URL";
			}
			else if (eventData->iType == IEED_EVENT_STATUSCHANGE || (eventData->iType == IEED_EVENT_SYSTEM))
				tmpltName[1] = isHistory ? "hStatus" : "Status";

			/* template-specific formatting */
			for (int i = 0; i < 2; i++) {
				if (tmpltName[i] == NULL || tmpm == NULL) continue;
				Template *tmplt = tmpm->getTemplate(tmpltName[i]);
				if (tmplt == NULL)
					continue;

				for (Token *token = tmplt->getTokens(); token != NULL; token = token->getNext()) {
					const char *tokenVal;
					tokenVal = NULL;
					switch (token->getType()) {
					case Token::PLAIN:
						tokenVal = token->getText();
						break;
					case Token::INAME:
						if (getFlags(protoSettings) & Options::LOG_SHOW_NICKNAMES)
							tokenVal = szName;
						else
							tokenVal = "&nbsp;";
						break;
					case Token::TIME:
						if (getFlags(protoSettings) & Options::LOG_SHOW_TIME)
							tokenVal = timestampToString(getFlags(protoSettings), eventData->time, 1);
						else
							tokenVal = "&nbsp;";
						break;
					case Token::DATE:
						if (getFlags(protoSettings) & Options::LOG_SHOW_DATE)
							tokenVal = timestampToString(getFlags(protoSettings), eventData->time, 0);
						else
							tokenVal = "&nbsp;";
						break;
					case Token::TEXT:
						tokenVal = szText;
						break;
					case Token::AVATAR:
						tokenVal = szAvatar;
						break;
					case Token::CID:
						tokenVal = szCID;
						break;
					case Token::BASE:
						tokenVal = szBase;
						break;
					case Token::NAMEIN:
						if (getFlags(protoSettings) & Options::LOG_SHOW_NICKNAMES) {
							if (event->hContact != NULL)
								tokenVal = szNameIn;
							else
								tokenVal = szName;
						}
						else tokenVal = "&nbsp;";
						break;
					case Token::NAMEOUT:
						if (getFlags(protoSettings) & Options::LOG_SHOW_NICKNAMES) {
							if (event->hContact != NULL)
								tokenVal = szNameOut;
							else
								tokenVal = szName;
						}
						else tokenVal = "&nbsp;";
						break;
					case Token::AVATARIN:
						tokenVal = szAvatarIn;
						break;
					case Token::AVATAROUT:
						tokenVal = szAvatarOut;
						break;
					case Token::PROTO:
						tokenVal = szRealProto;
						break;
					case Token::UIN:
						tokenVal = szUIN;
						break;
					case Token::UININ:
						tokenVal = szUINIn;
						break;
					case Token::UINOUT:
						tokenVal = szUINOut;
						break;
					case Token::STATUSMSG:
						tokenVal = szStatusMsg;
						break;
					case Token::NICKIN:
						tokenVal = szNickIn;
						break;
					case Token::NICKOUT:
						tokenVal = szNickOut;
						break;
					case Token::FILEDESC:
						tokenVal = szFileDesc;
						break;
					}
					if (tokenVal != NULL) {
						if (token->getEscape())
							str.Append(ptrA(Utils::escapeString(tokenVal)));
						else
							str.Append(tokenVal);
					}
				}
			}
			setLastEventType(MAKELONG(eventData->dwFlags, eventData->iType));
			setLastEventTime(eventData->time);

			mir_free(szName);
			mir_free(szText);
			mir_free(szFileDesc);

			view->write(str);
		}
	}
	mir_free(szBase);
	mir_free(szRealProto);
	mir_free(szProto);
	mir_free(szUINIn);
	mir_free(szUINOut);
	mir_free(szNoAvatar);
	mir_free(szAvatarIn);
	mir_free(szAvatarOut);
	mir_free(szNameIn);
	mir_free(szNameOut);
	mir_free(szNickIn);
	mir_free(szNickOut);
	mir_free(szStatusMsg);
	view->documentClose();
}

time_t TemplateHTMLBuilder::getStartedTime()
{
	return startedTime;
}
