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
#include "TemplateHTMLBuilder.h"

#define EVENTTYPE_STATUSCHANGE 25368

TemplateHTMLBuilder::TemplateHTMLBuilder() {
	iLastEventType = -1;
	startedTime = time(NULL);
	lastEventTime = time(NULL);
	groupTemplate = NULL;
	flashAvatarsTime[0] = time(NULL);
	flashAvatarsTime[1] = time(NULL);
	flashAvatars[0] = NULL;
	flashAvatars[1] = NULL;
}

TemplateHTMLBuilder::~TemplateHTMLBuilder() {
	for (int i = 0; i < 2; i++) {
		if (flashAvatars[i] != NULL) {
				delete flashAvatars[i];
		}
	}
}

char *TemplateHTMLBuilder::getAvatar(HANDLE hContact, const char * szProto) {
	DBVARIANT dbv;
	TCHAR tmpPath[MAX_PATH];
	TCHAR *result = NULL;

	if (Options::getAvatarServiceFlags() == Options::AVATARSERVICE_PRESENT) {
		struct avatarCacheEntry *ace = NULL;
		if (hContact == NULL) {
			ace = (struct avatarCacheEntry *)CallService(MS_AV_GETMYAVATAR, 0, (LPARAM)szProto);
		} else {
			ace = (struct avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)hContact, 0);
		}
		if (ace!=NULL) {
			if ( ace->cbSize == sizeof(avatarCacheEntry))
				result = ace->szFilename;
			else {
				// compatibility: in M0.9 it will always be char*
				MultiByteToWideChar( CP_ACP, 0, (char*)ace->szFilename, -1, tmpPath, SIZEOF(tmpPath));
				
			}
		}
	}
	if (!DBGetContactSettingTString(hContact, "ContactPhoto", "File", &dbv)) {
		if (_tcslen(dbv.ptszVal) > 0) {
			TCHAR* ext = _tcsrchr(dbv.ptszVal, '.');
			if (ext && lstrcmpi(ext, _T(".xml")) == 0) {
				result = ( TCHAR* )getFlashAvatar(dbv.ptszVal, (hContact == NULL) ? 1 : 0);
			} else {
				if (result == NULL) {
					/* relative -> absolute */
					_tcscpy (tmpPath, dbv.ptszVal);
					if (ServiceExists(MS_UTILS_PATHTOABSOLUTET) && _tcsncmp(tmpPath, _T("http://"), 7))
						CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)dbv.ptszVal, (LPARAM)tmpPath);

					result = tmpPath;
				}
			}
		}
		DBFreeVariant(&dbv);
	}
	char* res = Utils::UTF8Encode(result);
	Utils::convertPath(res);
	return res;
}

const char *TemplateHTMLBuilder::getFlashAvatar(const TCHAR *file, int index) {
	if (time(NULL) - flashAvatarsTime[index] > 600 || flashAvatars[index] == NULL) {
		if (flashAvatars[index] != NULL) {
			delete flashAvatars[index];
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
			if(urlBuf) {
				flashAvatars[index] = Utils::dupString(strtok(urlBuf + 5, "<\t\n\r"));
			}
		}
	}
	return flashAvatars[index];
}

TemplateMap *TemplateHTMLBuilder::getTemplateMap(ProtocolSettings * protoSettings) {
	return TemplateMap::getTemplateMap(protoSettings->getSRMMTemplateFilename());
}

int TemplateHTMLBuilder::getFlags(ProtocolSettings * protoSettings) {
	return protoSettings->getSRMMFlags();
}

char *TemplateHTMLBuilder::timestampToString(DWORD dwFlags, time_t check, int mode) {
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
			_tcsncpy(str, TranslateT("Today"), SIZEOF(str));
		else if(dwFlags & Options::LOG_RELATIVE_DATE && check > (today - 86400))
			_tcsncpy(str, TranslateT("Yesterday"), SIZEOF(str));
		else {
			dbtts.szFormat = (dwFlags & Options::LOG_LONG_DATE) ? _T("D") : _T("d");
			CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, check, (LPARAM) & dbtts);
		}
	}
	Utils::UTF8Encode(str, szResult, 500);
	return szResult;
}

void TemplateHTMLBuilder::buildHeadTemplate(IEView *view, IEVIEWEVENT *event, ProtocolSettings *protoSettings) {
	DBVARIANT dbv;
	CONTACTINFO ci;
	char tempBase[1024];
	char tempStr[1024];
	HANDLE hRealContact;
	char *szRealProto = NULL;
	char *szBase=NULL;
	char *szNoAvatar=NULL;
	char *szProto = NULL;
	char *szNameIn = NULL;
	char *szNameOut = NULL;
	char *szAvatarIn = NULL;
	char *szAvatarOut = NULL;
	char *szUINIn = NULL;
	char *szUINOut = NULL;
	char *szNickIn = NULL;
	char *szNickOut = NULL;
	char *szStatusMsg = NULL;
	int outputSize;
	char *output;

	output = NULL;
	hRealContact = getRealContact(event->hContact);
	szRealProto = getProto(hRealContact);
	szProto = getProto(event->pszProto, event->hContact);
	tempBase[0]='\0';
	if (protoSettings == NULL)
		return;

	TemplateMap *tmpm = getTemplateMap(protoSettings);
	if (tmpm==NULL)
		return;

	strcpy(tempBase, "file://");
	strcat(tempBase, tmpm->getFilename());
	char* pathrun = tempBase + strlen(tempBase);
	while ((*pathrun != '\\' && *pathrun != '/') && (pathrun > tempBase)) pathrun--;
	pathrun++;
	*pathrun = '\0';

	szBase = Utils::UTF8Encode(tempBase);
	getUINs(event->hContact, szUINIn, szUINOut);
	if (getFlags(protoSettings) & Options::LOG_SHOW_NICKNAMES) {
		szNameOut = getEncodedContactName(NULL, szProto, szRealProto);
		szNameIn = getEncodedContactName(event->hContact, szProto, szRealProto);
	} else {
		szNameOut = Utils::dupString("&nbsp;");
		szNameIn = Utils::dupString("&nbsp;");
	}
	sprintf(tempStr, "%snoavatar.png", tempBase);
		szNoAvatar = Utils::UTF8Encode(tempStr);
		if (access(szNoAvatar, 0) != -1)
		{
			sprintf(tempStr, "%snoavatar.jpg", tempBase);
			szNoAvatar = Utils::UTF8Encode(tempStr);
		}
	szAvatarIn = getAvatar(event->hContact, szRealProto);
	if (szAvatarIn == NULL) {
		szAvatarIn = Utils::dupString(szNoAvatar);
	}
	szAvatarOut = getAvatar(NULL, szRealProto);
	if (szAvatarOut == NULL) {
		szAvatarOut = Utils::dupString(szNoAvatar);
	}
	if (!DBGetContactSetting(event->hContact, "CList", "StatusMsg",&dbv)) {
		if (strlen(dbv.pszVal) > 0) {
			szStatusMsg = Utils::UTF8Encode(dbv.pszVal);
		}
		DBFreeVariant(&dbv);
	}
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = event->hContact;
	ci.szProto = szProto;
	ci.dwFlag = CNF_NICK | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		szNickIn = encodeUTF8(event->hContact, szRealProto, ci.pszVal, ENF_NAMESMILEYS, true);
	}
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = NULL;
	ci.szProto = szProto;
	ci.dwFlag = CNF_NICK | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		szNickOut = encodeUTF8(event->hContact, szRealProto, ci.pszVal, ENF_NAMESMILEYS, true);
	}

	Template *tmplt = tmpm->getTemplate(((event->dwFlags & IEEF_RTL) && tmpm->isRTL()) ? "HTMLStartRTL" : "HTMLStart");
	if (tmplt == NULL) {
		tmplt = tmpm->getTemplate("HTMLStart");
	}
	if (tmplt!=NULL) {
		for (Token *token = tmplt->getTokens();token!=NULL;token=token->getNext()) {
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
				if (token->getEscape()) {
					char *escapedToken = Utils::escapeString(tokenVal);
					Utils::appendText(&output, &outputSize, "%s", escapedToken);
					delete escapedToken;
				} else {
					Utils::appendText(&output, &outputSize, "%s", tokenVal);
				}
			}
		}
	}
	if (output != NULL) {
		view->write(output);
		free(output);
	}
	if (szBase!=NULL) delete szBase;
	if (szRealProto!=NULL) delete szRealProto;
	if (szProto!=NULL) delete szProto;
	if (szUINIn!=NULL) delete szUINIn;
	if (szUINOut!=NULL) delete szUINOut;
	if (szNoAvatar!=NULL) delete szNoAvatar;
	if (szAvatarIn!=NULL) delete szAvatarIn;
	if (szAvatarOut!=NULL) delete szAvatarOut;
	if (szNameIn!=NULL) delete szNameIn;
	if (szNameOut!=NULL) delete szNameOut;
	if (szNickIn!=NULL) delete szNickIn;
	if (szNickOut!=NULL) delete szNickOut;
	if (szStatusMsg!=NULL) delete szStatusMsg;
	//view->scrollToBottom();
	groupTemplate = NULL;
	iLastEventType = -1;
}

void TemplateHTMLBuilder::appendEventTemplate(IEView *view, IEVIEWEVENT *event, ProtocolSettings* protoSettings) {
	DBVARIANT dbv;
	CONTACTINFO ci;
	HANDLE hRealContact;
	char *szRealProto = NULL;
	char tempBase[1024];
	char *szBase=NULL;
	char tempStr[1024];
	char *szNoAvatar=NULL;
	char szCID[32];
	char *szName = NULL;
	char *szNameIn = NULL;
	char *szNameOut = NULL;
	char *szUIN = NULL;
	char *szUINIn = NULL;
	char *szUINOut = NULL;
	char *szNickIn = NULL;
	char *szNickOut = NULL;
	char *szStatusMsg = NULL;
	char *szAvatar = NULL;
	char *szAvatarIn = NULL;
	char *szAvatarOut = NULL;
	char *szText = NULL;
	char *szProto = NULL;
	char *szFileDesc = NULL;
	const char *tmpltName[2];
	bool isGrouping = false;
//	DWORD today = (DWORD)time(NULL);
//	today = today - today % 86400;
	if (protoSettings == NULL)
		return;
	
	hRealContact = getRealContact(event->hContact);
	szRealProto = getProto(hRealContact);
	szProto = getProto(event->pszProto, event->hContact);
	tempBase[0]='\0';
	TemplateMap *tmpm = getTemplateMap(protoSettings);
	if (tmpm!=NULL) {
		strcpy(tempBase, "file://");
		strcat(tempBase, tmpm->getFilename());
		char* pathrun = tempBase + strlen(tempBase);
		while ((*pathrun != '\\' && *pathrun != '/') && (pathrun > tempBase)) pathrun--;
		pathrun++;
		*pathrun = '\0';
		isGrouping = tmpm->isGrouping();
	}
	szBase = Utils::UTF8Encode(tempBase);

	if (event->hContact != NULL) {
		getUINs(event->hContact, szUINIn, szUINOut);
	}

	if (event->hContact != NULL) {
		szNameOut = getEncodedContactName(NULL, szProto, szRealProto);
		szNameIn = getEncodedContactName(event->hContact, szProto, szRealProto);
	} else {
		szNameOut = Utils::dupString("&nbsp;");
		szNameIn = Utils::dupString("&nbsp;");
	}
	sprintf(tempStr, "%snoavatar.png", tempBase);
		szNoAvatar = Utils::UTF8Encode(tempStr);
		if (access(szNoAvatar, 0) != -1)
		{
			sprintf(tempStr, "%snoavatar.jpg", tempBase);
			szNoAvatar = Utils::UTF8Encode(tempStr);
		}

	if(event->hContact != NULL) {
		szAvatarIn = getAvatar(event->hContact, szRealProto);
	}
	if (szAvatarIn == NULL) {
		szAvatarIn = Utils::dupString(szNoAvatar);
	}
	szAvatarOut = getAvatar(NULL, szRealProto);
	if (szAvatarOut == NULL) {
		szAvatarOut = Utils::dupString(szNoAvatar);
	}
	if(event->hContact != NULL) {
		if (!DBGetContactSetting(event->hContact, "CList", "StatusMsg",&dbv)) {
			if (strlen(dbv.pszVal) > 0) {
				szStatusMsg = Utils::UTF8Encode(dbv.pszVal);
			}
			DBFreeVariant(&dbv);
		}
	}
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = event->hContact;
	ci.szProto = szProto;
	ci.dwFlag = CNF_NICK | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		szNickIn = encodeUTF8(event->hContact, szRealProto, ci.pszVal, ENF_NAMESMILEYS, true);
	}
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = NULL;
	ci.szProto = szProto;
	ci.dwFlag = CNF_NICK | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		szNickOut = encodeUTF8(event->hContact, szRealProto, ci.pszVal, ENF_NAMESMILEYS, true);
	}
	IEVIEWEVENTDATA* eventData = event->eventData;
	for (int eventIdx = 0; eventData!=NULL && (eventIdx < event->count || event->count==-1); eventData = eventData->next, eventIdx++) {
		int outputSize;
		char *output;
		output = NULL;
		if (eventData->iType == IEED_EVENT_MESSAGE || eventData->iType == IEED_EVENT_STATUSCHANGE || eventData->iType == IEED_EVENT_FILE || eventData->iType == IEED_EVENT_URL || eventData->iType == IEED_EVENT_SYSTEM) {
			bool isSent = (eventData->dwFlags & IEEDF_SENT) != 0;
			bool isRTL = (eventData->dwFlags & IEEDF_RTL) && tmpm->isRTL();
			bool isHistory = (eventData->time < (DWORD)getStartedTime() && (eventData->dwFlags & IEEDF_READ || eventData->dwFlags & IEEDF_SENT));
			bool isGroupBreak = TRUE;
 			if ((getFlags(protoSettings) & Options::LOG_GROUP_MESSAGES) && eventData->dwFlags == LOWORD(getLastEventType())
			  && eventData->iType == IEED_EVENT_MESSAGE && HIWORD(getLastEventType()) == IEED_EVENT_MESSAGE
			  && (isSameDate(eventData->time, getLastEventTime()))
//			  && ((eventData->time < today) == (getLastEventTime() < today))
			  && (((eventData->time < (DWORD)startedTime) == (getLastEventTime() < (DWORD)startedTime)) || !(eventData->dwFlags & IEEDF_READ))) {
				isGroupBreak = FALSE;
			}
			if (isSent) {
				szAvatar = szAvatarOut;
				szUIN = szUINOut;
				sprintf(szCID, "%d", 0);
			} else {
				szAvatar = szAvatarIn;
				szUIN = szUINIn;
				sprintf(szCID, "%d", (int)event->hContact);
			}
			tmpltName[0] = groupTemplate;
			tmpltName[1] = NULL;
			groupTemplate = NULL;
			szName = NULL;
			szText = NULL;
			szFileDesc = NULL;
			if (event->eventData->dwFlags & IEEDF_UNICODE_NICK) {
				szName = encodeUTF8(event->hContact, szRealProto, eventData->pszNickW, ENF_NAMESMILEYS, true);
			} else {
				szName = encodeUTF8(event->hContact, szRealProto, eventData->pszNick, ENF_NAMESMILEYS, true);
			}
			if (eventData->dwFlags & IEEDF_UNICODE_TEXT) {
				szText = encodeUTF8(event->hContact, szRealProto, eventData->pszTextW, eventData->iType == IEED_EVENT_MESSAGE ? ENF_ALL : 0, isSent);
			} else {
				szText = encodeUTF8(event->hContact, szRealProto, eventData->pszText, event->codepage, eventData->iType == IEED_EVENT_MESSAGE ? ENF_ALL : 0, isSent);
			}
			if (eventData->dwFlags & IEEDF_UNICODE_TEXT2) {
				szFileDesc = encodeUTF8(event->hContact, szRealProto, eventData->pszText2W, 0, isSent);
			} else {
				szFileDesc = encodeUTF8(event->hContact, szRealProto, eventData->pszText2, event->codepage, 0, isSent);
			}
			if ((eventData->iType == IEED_EVENT_MESSAGE)) {
				if (!isRTL) {
					if (isGrouping && (getFlags(protoSettings) & Options::LOG_GROUP_MESSAGES)) {
						if (isGroupBreak) {
							tmpltName[1] = isHistory ? isSent ? "hMessageOutGroupStart" : "hMessageInGroupStart" : isSent ? "MessageOutGroupStart" : "MessageInGroupStart";
						} else {
							tmpltName[0] = isHistory ? isSent ? "hMessageOutGroupInner" : "hMessageInGroupInner" : isSent ? "MessageOutGroupInner" : "MessageInGroupInner";
						}
						groupTemplate = isHistory ? isSent ? "hMessageOutGroupEnd" : "hMessageInGroupEnd" : isSent ? "MessageOutGroupEnd" : "MessageInGroupEnd";
					} else {
						tmpltName[1] = isHistory ? isSent ? "hMessageOut" : "hMessageIn" : isSent ? "MessageOut" : "MessageIn";
					}
				} else {
					if (isGrouping && (getFlags(protoSettings) & Options::LOG_GROUP_MESSAGES)) {
						if (isGroupBreak) {
							tmpltName[1] = isHistory ? isSent ? "hMessageOutGroupStartRTL" : "hMessageInGroupStartRTL" : isSent ? "MessageOutGroupStartRTL" : "MessageInGroupStartRTL";
						} else {
							tmpltName[0] = isHistory ? isSent ? "hMessageOutGroupInnerRTL" : "hMessageInGroupInnerRTL" : isSent ? "MessageOutGroupInnerRTL" : "MessageInGroupInnerRTL";
						}
						groupTemplate = isHistory ? isSent ? "hMessageOutGroupEndRTL" : "hMessageInGroupEndRTL" : isSent ? "MessageOutGroupEndRTL" : "MessageInGroupEndRTL";
					} else {
						tmpltName[1] = isHistory ? isSent ? "hMessageOutRTL" : "hMessageInRTL" : isSent ? "MessageOutRTL" : "MessageInRTL";
					}
				}
			} else if (eventData->iType == IEED_EVENT_FILE) {
				tmpltName[1] = isHistory ? isSent ? "hFileOut" : "hFileIn" : isSent ? "FileOut" : "FileIn";
				Template *tmplt = (tmpm == NULL) ? NULL : tmpm->getTemplate(tmpltName[1]);
				if (tmplt == NULL) {
					tmpltName[1] = isHistory ? "hFile" : "File";
				}
			} else if (eventData->iType == IEED_EVENT_URL) {
				tmpltName[1] = isHistory ? isSent ? "hURLOut" : "hURLIn" : isSent ? "URLOut" : "URLIn";
				Template *tmplt = (tmpm == NULL) ? NULL : tmpm->getTemplate(tmpltName[1]);
				if (tmplt == NULL) {
					tmpltName[1] = isHistory ? "hURL" : "URL";
				}
			} else if (eventData->iType == IEED_EVENT_STATUSCHANGE || (eventData->iType == IEED_EVENT_SYSTEM)) {
				tmpltName[1] = isHistory ? "hStatus" : "Status";
			}
			/* template-specific formatting */
			for (int i=0;i<2;i++) {
				if (tmpltName[i] == NULL || tmpm == NULL) continue;
				Template *tmplt = tmpm->getTemplate(tmpltName[i]);
				if (tmplt == NULL) continue;
				for (Token *token = tmplt->getTokens();token!=NULL;token=token->getNext()) {
					const char *tokenVal;
					tokenVal = NULL;
					switch (token->getType()) {
						case Token::PLAIN:
							tokenVal = token->getText();
							break;
						case Token::NAME:
							if (getFlags(protoSettings) & Options::LOG_SHOW_NICKNAMES) {
								tokenVal = szName;
							} else {
								tokenVal = "&nbsp;";
							}
							break;
						case Token::TIME:
							if (getFlags(protoSettings) & Options::LOG_SHOW_TIME) {
								tokenVal = timestampToString(getFlags(protoSettings), eventData->time, 1);
							} else {
								tokenVal = "&nbsp;";
							}
							break;
						case Token::DATE:
							if (getFlags(protoSettings) & Options::LOG_SHOW_DATE) {
								tokenVal = timestampToString(getFlags(protoSettings), eventData->time, 0);
							} else {
								tokenVal = "&nbsp;";
							}
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
								if (event->hContact != NULL) {
									tokenVal = szNameIn;
								} else {
									tokenVal = szName;
								}
							} else {
								tokenVal = "&nbsp;";
							}
							break;
						case Token::NAMEOUT:
							if (getFlags(protoSettings) & Options::LOG_SHOW_NICKNAMES) {
								if (event->hContact != NULL) {
									tokenVal = szNameOut;
								} else {
									tokenVal = szName;
								}
							} else {
								tokenVal = "&nbsp;";
							}
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
						if (token->getEscape()) {
							char *escapedToken = Utils::escapeString(tokenVal);
							Utils::appendText(&output, &outputSize, "%s", escapedToken);
							delete escapedToken;
						} else {
							Utils::appendText(&output, &outputSize, "%s", tokenVal);
						}
					}
				}
			}
			setLastEventType(MAKELONG(eventData->dwFlags, eventData->iType));
			setLastEventTime(eventData->time);
			if (szName!=NULL) delete szName;
			if (szText!=NULL) delete szText;
			if (szFileDesc!=NULL) delete szFileDesc;
		}
		if (output != NULL) {
			view->write(output);
			free(output);
		}
	}
	if (szBase!=NULL) delete szBase;
	if (szRealProto!=NULL) delete szRealProto;
	if (szProto!=NULL) delete szProto;
	if (szUINIn!=NULL) delete szUINIn;
	if (szUINOut!=NULL) delete szUINOut;
	if (szNoAvatar!=NULL) delete szNoAvatar;
	if (szAvatarIn!=NULL) delete szAvatarIn;
	if (szAvatarOut!=NULL) delete szAvatarOut;
	if (szNameIn!=NULL) delete szNameIn;
	if (szNameOut!=NULL) delete szNameOut;
	if (szNickIn!=NULL) delete szNickIn;
	if (szNickOut!=NULL) delete szNickOut;
	if (szStatusMsg!=NULL) delete szStatusMsg;
	view->documentClose();
}

time_t TemplateHTMLBuilder::getStartedTime() {
	return startedTime;
}