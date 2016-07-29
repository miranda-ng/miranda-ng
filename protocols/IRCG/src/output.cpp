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

static CMStringW FormatOutput(const CIrcMessage* pmsg)
{
	CMStringW sMessage;

	if (pmsg->m_bIncoming) { // Is it an incoming message?
		if (pmsg->sCommand == L"WALLOPS" && pmsg->parameters.getCount() > 0) {
			wchar_t temp[200]; *temp = '\0';
			mir_snwprintf(temp, TranslateT("WallOps from %s: "), pmsg->prefix.sNick.c_str());
			sMessage = temp;
			for (int i = 0; i < (int)pmsg->parameters.getCount(); i++) {
				sMessage += pmsg->parameters[i];
				if (i != pmsg->parameters.getCount() - 1)
					sMessage += L" ";
			}
			goto THE_END;
		}

		if (pmsg->sCommand == L"INVITE" && pmsg->parameters.getCount() > 1) {
			wchar_t temp[256]; *temp = '\0';
			mir_snwprintf(temp, TranslateT("%s invites you to %s"), pmsg->prefix.sNick.c_str(), pmsg->parameters[1].c_str());
			sMessage = temp;
			for (int i = 2; i < (int)pmsg->parameters.getCount(); i++) {
				sMessage += L": " + pmsg->parameters[i];
				if (i != pmsg->parameters.getCount() - 1)
					sMessage += L" ";
			}
			goto THE_END;
		}

		int index = _wtoi(pmsg->sCommand.c_str());
		if (index == 301 && pmsg->parameters.getCount() > 0) {
			wchar_t temp[500]; *temp = '\0';
			mir_snwprintf(temp, TranslateT("%s is away"), pmsg->parameters[1].c_str());
			sMessage = temp;
			for (int i = 2; i < (int)pmsg->parameters.getCount(); i++) {
				sMessage += L": " + pmsg->parameters[i];
				if (i != pmsg->parameters.getCount() - 1)
					sMessage += L" ";
			}
			goto THE_END;
		}

		if ((index == 443 || index == 441) && pmsg->parameters.getCount() > 3)
			return pmsg->parameters[1] + L" " + pmsg->parameters[3] + L": " + pmsg->parameters[2];

		if (index == 303) {  // ISON command
			sMessage = TranslateT("These are online: ");
			for (int i = 1; i < (int)pmsg->parameters.getCount(); i++) {
				sMessage += pmsg->parameters[i];
				if (i != pmsg->parameters.getCount() - 1)
					sMessage += L", ";
			}
			goto THE_END;
		}

		if ((index > 400 || index < 500) && pmsg->parameters.getCount() > 2 && pmsg->sCommand[0] == '4') //all error messages
			return pmsg->parameters[2] + L": " + pmsg->parameters[1];
	}
	else if (pmsg->sCommand == L"NOTICE" && pmsg->parameters.getCount() > 1) {
		wchar_t temp[500]; *temp = '\0';

		int l = pmsg->parameters[1].GetLength();
		if (l > 3 && pmsg->parameters[1][0] == 1 && pmsg->parameters[1][l - 1] == 1) {
			// CTCP reply
			CMStringW tempstr = pmsg->parameters[1];
			tempstr.Delete(0, 1);
			tempstr.Delete(tempstr.GetLength() - 1, 1);
			CMStringW type = GetWord(tempstr.c_str(), 0);
			if (mir_wstrcmpi(type.c_str(), L"ping") == 0)
				mir_snwprintf(temp, TranslateT("CTCP %s reply sent to %s"), type.c_str(), pmsg->parameters[0].c_str());
			else
				mir_snwprintf(temp, TranslateT("CTCP %s reply sent to %s: %s"), type.c_str(), pmsg->parameters[0].c_str(), GetWordAddress(tempstr.c_str(), 1));
			sMessage = temp;
		}
		else {
			mir_snwprintf(temp, TranslateT("Notice to %s: "), pmsg->parameters[0].c_str());
			sMessage = temp;
			for (int i = 1; i < (int)pmsg->parameters.getCount(); i++) {
				sMessage += pmsg->parameters[i];
				if (i != pmsg->parameters.getCount() - 1)
					sMessage += L" ";
			}
		}
		goto THE_END;
	}

	// Default Message handler.	

	if (pmsg->m_bIncoming) {
		if (pmsg->parameters.getCount() < 2 && pmsg->parameters.getCount() > 0)
			return pmsg->sCommand + L" : " + pmsg->parameters[0];

		if (pmsg->parameters.getCount() > 1)
		for (int i = 1; i < (int)pmsg->parameters.getCount(); i++)
			sMessage += pmsg->parameters[i] + L" ";
	}
	else {
		if (pmsg->prefix.sNick.GetLength())
			sMessage = pmsg->prefix.sNick + L" ";
		sMessage += pmsg->sCommand + L" ";
		for (int i = 0; i < (int)pmsg->parameters.getCount(); i++)
			sMessage += pmsg->parameters[i] + L" ";
	}

THE_END:
	return sMessage;
}

BOOL CIrcProto::ShowMessage(const CIrcMessage* pmsg)
{
	CMStringW mess = FormatOutput(pmsg);

	if (!pmsg->m_bIncoming)
		mess.Replace(L"%%", L"%");

	int iTemp = _wtoi(pmsg->sCommand.c_str());

	//To active window
	if ((iTemp > 400 || iTemp < 500) && pmsg->sCommand[0] == '4' //all error messages	
		|| pmsg->sCommand == L"303"		//ISON command
		|| pmsg->sCommand == L"INVITE"
		|| ((pmsg->sCommand == L"NOTICE") && ((pmsg->parameters.getCount() > 2) ? (wcsstr(pmsg->parameters[1].c_str(), L"\001") == NULL) : false)) // CTCP answers should go to m_network Log window!
		|| pmsg->sCommand == L"515")		//chanserv error
	{
		DoEvent(GC_EVENT_INFORMATION, NULL, pmsg->m_bIncoming ? pmsg->prefix.sNick.c_str() : m_info.sNick.c_str(), mess.c_str(), NULL, NULL, NULL, true, pmsg->m_bIncoming ? false : true);
		return TRUE;
	}

	if (m_useServer) {
		DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW,
			(pmsg->m_bIncoming) ? pmsg->prefix.sNick.c_str() : m_info.sNick.c_str(),
			mess.c_str(), NULL, NULL, NULL, true, pmsg->m_bIncoming ? false : true);
		return true;
	}
	return false;
}
