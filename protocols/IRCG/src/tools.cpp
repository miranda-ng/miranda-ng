/*
IRC plugin for Miranda IM

Copyright (C) 2003-05 Jurgen Persson
Copyright (C) 2007-09 George Hazan

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

/////////////////////////////////////////////////////////////////////////////////////////

void CIrcProto::AddToJTemp(wchar_t op, CMStringW& sCommand)
{
	CMStringW res;

	int pos = 0;
	for (;;) {
		CMStringW tmp = sCommand.Tokenize(L",", pos);
		if (pos == -1)
			break;

		tmp = op + tmp;
		if (res.IsEmpty())
			res = tmp;
		else
			res += L" " + tmp;
	}

	DBVARIANT dbv;
	if (!getWString("JTemp", &dbv)) {
		res = CMStringW(dbv.pwszVal) + L" " + res;
		db_free(&dbv);
	}

	setWString("JTemp", res.c_str());
}

CMStringW __stdcall GetWord(const wchar_t* text, int index)
{
	if (text && *text) {
		wchar_t* p1 = (wchar_t*)text;
		wchar_t* p2 = nullptr;

		while (*p1 == ' ')
			p1++;

		if (*p1 != 0) {
			for (int i = 0; i < index; i++) {
				p2 = wcschr(p1, ' ');
				if (!p2)
					p2 = wcschr(p1, 0);
				else
				while (*p2 == ' ')
					p2++;

				p1 = p2;
			}

			p2 = wcschr(p1, ' ');
			if (!p2)
				p2 = wcschr(p1, 0);

			if (p1 != p2)
				return CMStringW(p1, p2 - p1);
		}
	}

	return CMStringW();
}

const wchar_t* __stdcall GetWordAddress(const wchar_t* text, int index)
{
	if (!text || !mir_wstrlen(text))
		return text;

	const wchar_t* temp = text;

	while (*temp == ' ')
		temp++;

	if (index == 0)
		return temp;

	for (int i = 0; i < index; i++) {
		temp = wcschr(temp, ' ');
		if (!temp)
			temp = (wchar_t*)wcschr(text, 0);
		else
		while (*temp == ' ')
			temp++;
		text = temp;
	}

	return temp;
}

void __stdcall RemoveLinebreaks(CMStringW &Message)
{
	while (Message.Find(L"\r\n\r\n", 0) != -1)
		Message.Replace(L"\r\n\r\n", L"\r\n");

	if (Message.Find(L"\r\n", 0) == 0)
		Message.Delete(0, 2);

	if ((Message.GetLength() > 1) && (Message.Find(L"\r\n", Message.GetLength() - 2) == 0))
		Message.Delete(Message.GetLength() - 2, 2);
}

char* __stdcall IrcLoadFile(wchar_t* szPath)
{
	char * szContainer = nullptr;
	uint32_t dwSiz = 0;
	FILE *hFile = _wfopen(szPath, L"rb");
	if (hFile != nullptr) {
		fseek(hFile, 0, SEEK_END); // seek to end
		dwSiz = ftell(hFile); // size
		fseek(hFile, 0, SEEK_SET); // seek back to original pos
		szContainer = new char[dwSiz + 1];
		fread(szContainer, 1, dwSiz, hFile);
		szContainer[dwSiz] = 0;
		fclose(hFile);
		return szContainer;
	}

	return nullptr;
}

int __stdcall WCCmp(const wchar_t* wild, const wchar_t* string)
{
	if (wild == nullptr || !mir_wstrlen(wild) || string == nullptr || !mir_wstrlen(string))
		return 1;

	const wchar_t *cp = nullptr, *mp = nullptr;
	while ((*string) && (*wild != '*')) {
		if ((*wild != *string) && (*wild != '?'))
			return 0;

		wild++;
		string++;
	}

	while (*string) {
		if (*wild == '*') {
			if (!*++wild)
				return 1;

			mp = wild;
			cp = string + 1;
		}
		else if ((*wild == *string) || (*wild == '?')) {
			wild++;
			string++;
		}
		else {
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*')
		wild++;

	return !*wild;
}

bool CIrcProto::IsChannel(const wchar_t* sName)
{
	return (sChannelPrefixes.Find(sName[0]) != -1);
}

CMStringA __stdcall GetWord(const char* text, int index)
{
	if (text && text[0]) {
		char* p1 = (char*)text;
		char* p2 = nullptr;

		while (*p1 == ' ')
			p1++;

		if (*p1 != 0) {
			for (int i = 0; i < index; i++) {
				p2 = strchr(p1, ' ');
				if (!p2)
					p2 = strchr(p1, 0);
				else
				while (*p2 == ' ')
					p2++;

				p1 = p2;
			}

			p2 = strchr(p1, ' ');
			if (!p2)
				p2 = strchr(p1, 0);

			if (p1 != p2)
				return CMStringA(p1, p2 - p1 + 1);
		}
	}

	return CMStringA();
}

bool CIrcProto::IsChannel(const char* sName)
{
	return (sChannelPrefixes.Find(sName[0]) != -1);
}

static int mapIrc2srmm[] = { 15, 0, 1, 4, 14, 6, 3, 5, 13, 12, 2, 10, 9, 11, 7, 8 };

static const wchar_t* DoEnterNumber(const wchar_t *text, int &res)
{
	if (*text >= '0' && *text <= '9') {
		res = text[0] - '0';
		text++;

		if (*text >= '0' && *text <= '9') {
			res *= 10;
			res += text[0] - '0';
			text++;
		}

		res = (res >= 0 && res < _countof(mapIrc2srmm)) ? mapIrc2srmm[res] : -1;
	}
	else res = -1;

	return text;
}

wchar_t* __stdcall DoColorCodes(const wchar_t *text, bool bStrip, bool bReplacePercent)
{
	static wchar_t szTemp[4000]; szTemp[0] = 0;
	wchar_t* p = szTemp;
	bool bBold = false;
	bool bUnderline = false;
	bool bItalics = false;
	int iFG = -1, iBG = -1;

	if (!text)
		return szTemp;

	while (*text != 0) {
		switch (*text) {
		case '%': // escape
			*p++ = '%';
			if (bReplacePercent)
				*p++ = '%';
			text++;
			break;

		case irc::BOLD:
			if (!bStrip) {
				*p++ = '%';
				*p++ = bBold ? 'B' : 'b';
			}
			bBold = !bBold;
			text++;
			break;

		case irc::RESET:
			if (!bStrip) {
				*p++ = '%';
				*p++ = 'r';
			}
			bUnderline = bItalics = bBold = false;
			text++;
			break;

		case irc::ITALICS:
			if (!bStrip) {
				*p++ = '%';
				*p++ = bItalics ? 'I' : 'i';
			}
			bItalics = !bItalics;
			text++;
			break;

		case irc::UNDERLINE:
			if (!bStrip) {
				*p++ = '%';
				*p++ = bUnderline ? 'U' : 'u';
			}
			bUnderline = !bUnderline;
			text++;
			break;

		case irc::COLOR:
			int iOldBG, iOldFG;
			iOldBG = iBG, iOldFG = iFG;

			text = DoEnterNumber(text + 1, iFG);
			if (*text == ',')
				text = DoEnterNumber(text + 1, iBG);
			else
				iBG = -1;

			// create tag for chat.dll
			if (!bStrip) {
				wchar_t buf[10];
				if (iFG != iOldFG) {
					*p++ = '%';
					if (iFG == -1)
						*p++ = 'C';
					else {
						*p++ = 'c';
						mir_snwprintf(buf, L"%02u", iFG);
						*p++ = buf[0];
						*p++ = buf[1];
					}
				}

				if (iBG != iOldBG) {
					*p++ = '%';
					if (iBG == -1)
						*p++ = 'F';
					else {
						*p++ = 'f';
						mir_snwprintf(buf, L"%02u", iBG);
						*p++ = buf[0];
						*p++ = buf[1];
					}
				}
			}
			break;

		default:
			*p++ = *text++;
			break;
		}
	}

	*p = 0;
	return szTemp;
}

INT_PTR CIrcProto::DoEvent(int iEvent, const wchar_t* pszWindow, const wchar_t* pszNick,
	const wchar_t* pszText, const wchar_t* pszStatus, const wchar_t* pszUserInfo,
	DWORD_PTR dwItemData, bool bAddToLog, bool bIsMe, time_t timestamp)
{
	CMStringW sID;
	CMStringW sText;

	if (iEvent == GC_EVENT_INFORMATION && bIsMe && !bEcho)
		return false;

	if (pszText)
		sText = DoColorCodes(pszText, FALSE, TRUE);

	GCEVENT gce = { m_szModuleName, nullptr, iEvent };
	if (pszWindow) {
		sID = pszWindow;
		gce.pszID.w = (wchar_t*)sID.c_str();
	}
	else gce.pszID.w = nullptr;

	gce.pszStatus.w = pszStatus;
	gce.dwFlags = (bAddToLog) ? GCEF_ADDTOLOG : 0;
	gce.pszNick.w = pszNick;
	gce.pszUID.w = pszNick;
	if (iEvent == GC_EVENT_TOPIC)
		gce.pszUserInfo.w = pszUserInfo;
	else
		gce.pszUserInfo.w = m_showAddresses ? pszUserInfo : nullptr;

	if (!sText.IsEmpty())
		gce.pszText.w = sText.c_str();

	gce.dwItemData = dwItemData;
	if (timestamp == 1)
		gce.time = time(0);
	else
		gce.time = timestamp;
	gce.bIsMe = bIsMe;
	return Chat_Event(&gce);
}

CMStringW CIrcProto::ModeToStatus(int sMode)
{
	if (sUserModes.Find(sMode) != -1) {
		switch (sMode) {
		case 'q':
			return TranslateT("Owner");
		case 'o':
			return TranslateT("Op");
		case 'v':
			return TranslateT("Voice");
		case 'h':
			return TranslateT("Halfop");
		case 'a':
			return TranslateT("Admin");
		default:
			return TranslateT("Unknown");
		}
	}

	return TranslateT("Normal");
}

CMStringW CIrcProto::PrefixToStatus(int cPrefix)
{
	const wchar_t* p = wcschr(sUserModePrefixes.c_str(), cPrefix);
	if (p) {
		int index = int(p - sUserModePrefixes.c_str());
		return ModeToStatus(sUserModes[index]);
	}

	return TranslateT("Normal");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Timer functions 

struct TimerPair
{
	TimerPair(CIrcProto* _pro, UINT_PTR _id) :
		ppro(_pro),
		idEvent(_id)
		{}

	UINT_PTR idEvent;
	CIrcProto *ppro;
};

static int CompareTimers(const TimerPair* p1, const TimerPair* p2)
{
	if (p1->idEvent < p2->idEvent)
		return -1;
	return (p1->idEvent == p2->idEvent) ? 0 : 1;
}

static OBJLIST<TimerPair> timers(10, CompareTimers);
static mir_cs timers_cs;

void UninitTimers(void)
{
	mir_cslock lck(timers_cs);
	timers.destroy();
}

CIrcProto* GetTimerOwner(UINT_PTR nIDEvent)
{
	mir_cslock lck(timers_cs);

	TimerPair temp(nullptr, nIDEvent);
	int idx = timers.getIndex(&temp);
	return (idx == -1) ? nullptr : timers[idx].ppro;
}

void CIrcProto::SetChatTimer(UINT_PTR &nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
	if (nIDEvent)
		KillChatTimer(nIDEvent);

	nIDEvent = SetTimer(nullptr, NULL, uElapse, lpTimerFunc);

	mir_cslock lck(timers_cs);
	timers.insert(new TimerPair(this, nIDEvent));
}

void CIrcProto::KillChatTimer(UINT_PTR &nIDEvent)
{
	if (nIDEvent == 0)
		return;
	{
		mir_cslock lck(timers_cs);
		TimerPair temp(this, nIDEvent);
		int idx = timers.getIndex(&temp);
		if (idx != -1)
			timers.remove(idx);
	}

	KillTimer(nullptr, nIDEvent);
	nIDEvent = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CIrcProto::SetChannelSBText(CMStringW sWindow, CHANNELINFO *wi)
{
	CMStringW sTemp = L"";
	if (wi->pszMode) {
		sTemp += L"[";
		sTemp += wi->pszMode;
		sTemp += L"] ";
	}
	if (wi->pszTopic)
		sTemp += wi->pszTopic;
	sTemp = DoColorCodes(sTemp.c_str(), TRUE, FALSE);
	Chat_SetStatusbarText(m_szModuleName, sWindow, sTemp);
	return 0;
}

bool CIrcProto::FreeWindowItemData(CMStringW window, CHANNELINFO *wis)
{
	CHANNELINFO *wi;
	if (!wis)
		wi = (CHANNELINFO*)Chat_GetUserInfo(m_szModuleName, window);
	else
		wi = wis;
	if (wi) {
		delete[] wi->pszLimit;
		delete[] wi->pszMode;
		delete[] wi->pszPassword;
		delete[] wi->pszTopic;
		delete wi;
		return true;
	}
	return false;
}

bool CIrcProto::AddWindowItemData(CMStringW window, const wchar_t* pszLimit, const wchar_t* pszMode, const wchar_t* pszPassword, const wchar_t* pszTopic)
{
	CHANNELINFO *wi = (CHANNELINFO *)Chat_GetUserInfo(m_szModuleName, window);
	if (wi) {
		if (pszLimit) {
			wi->pszLimit = (wchar_t*)realloc(wi->pszLimit, sizeof(wchar_t)*(mir_wstrlen(pszLimit) + 1));
			mir_wstrcpy(wi->pszLimit, pszLimit);
		}
		if (pszMode) {
			wi->pszMode = (wchar_t*)realloc(wi->pszMode, sizeof(wchar_t)*(mir_wstrlen(pszMode) + 1));
			mir_wstrcpy(wi->pszMode, pszMode);
		}
		if (pszPassword) {
			wi->pszPassword = (wchar_t*)realloc(wi->pszPassword, sizeof(wchar_t)*(mir_wstrlen(pszPassword) + 1));
			mir_wstrcpy(wi->pszPassword, pszPassword);
		}
		if (pszTopic) {
			wi->pszTopic = (wchar_t*)realloc(wi->pszTopic, sizeof(wchar_t)*(mir_wstrlen(pszTopic) + 1));
			mir_wstrcpy(wi->pszTopic, pszTopic);
		}

		SetChannelSBText(window, wi);
		return true;
	}
	return false;
}

void CIrcProto::FindLocalIP(HNETLIBCONN hConn) // inspiration from jabber
{
	// Determine local IP
	int socket = Netlib_GetSocket(hConn);
	if (socket != INVALID_SOCKET) {
		struct sockaddr_in saddr;
		int len = sizeof(saddr);
		getsockname(socket, (struct sockaddr *) &saddr, &len);
		mir_strncpy(m_myLocalHost, inet_ntoa(saddr.sin_addr), 49);
		m_myLocalPort = ntohs(saddr.sin_port);
	}
}

void CIrcProto::DoUserhostWithReason(int type, CMStringW reason, bool bSendCommand, const wchar_t *userhostparams, ...)
{
	wchar_t temp[4096];
	CMStringW S = L"";
	switch (type) {
	case 1:
		S = L"USERHOST";
		break;
	case 2:
		S = L"WHO";
		break;
	default:
		S = L"USERHOST";
		break;
	}
	S.AppendChar(' ');
	S.Append(userhostparams);

	va_list ap;
	va_start(ap, userhostparams);
	mir_vsnwprintf(temp, _countof(temp), S.c_str(), ap);
	va_end(ap);

	// Add reason
	if (type == 1)
		vUserhostReasons.insert(new CMStringW(reason));
	else if (type == 2)
		vWhoInProgress.insert(new CMStringW(reason));

	// Do command
	if (IsConnected() && bSendCommand)
		SendIrcMessage(temp, false);
}

CMStringW CIrcProto::GetNextUserhostReason(int type)
{
	CMStringW reason = L"";
	switch (type) {
	case 1:
		if (!vUserhostReasons.getCount())
			return CMStringW();

		// Get reason
		reason = vUserhostReasons[0];
		vUserhostReasons.remove(0);
		break;
	case 2:
		if (!vWhoInProgress.getCount())
			return CMStringW();

		// Get reason
		reason = vWhoInProgress[0];
		vWhoInProgress.remove(0);
		break;
	}

	return reason;
}

CMStringW CIrcProto::PeekAtReasons(int type)
{
	switch (type) {
	case 1:
		if (!vUserhostReasons.getCount())
			return CMStringW();
		return vUserhostReasons[0];

	case 2:
		if (!vWhoInProgress.getCount())
			return CMStringW();
		return vWhoInProgress[0];

	}
	return CMStringW();
}

void CIrcProto::ClearUserhostReasons(int type)
{
	switch (type) {
	case 1:
		vUserhostReasons.destroy();
		break;
	case 2:
		vWhoInProgress.destroy();
		break;
	}
}
