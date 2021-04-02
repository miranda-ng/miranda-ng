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

INT_PTR __cdecl CIrcProto::Scripting_InsertRawIn(WPARAM, LPARAM lParam)
{
	char* pszRaw = (char*)lParam;

	if (m_scriptingEnabled && pszRaw && IsConnected()) {
		wchar_t* p = mir_a2u(pszRaw);
		InsertIncomingEvent(p);
		mir_free(p);
		return 0;
	}

	return 1;
}
 
INT_PTR __cdecl CIrcProto::Scripting_InsertRawOut( WPARAM, LPARAM lParam )
{
	char* pszRaw = (char*)lParam;
	if (m_scriptingEnabled && pszRaw && IsConnected()) {
		CMStringA S = pszRaw;
		S.Replace("%", "%%%%");
		NLSendNoScript((const unsigned char *)S.c_str(), (int)mir_strlen(S.c_str()));
		return 0;
	}

	return 1;
}

INT_PTR __cdecl CIrcProto::Scripting_InsertGuiIn(WPARAM, LPARAM)
{
	return 1;
}

//helper functions
static void __stdcall OnHook(void * pi)
{
	GCHOOK* gch = (GCHOOK*)pi;
	free(gch->ptszUID);
	free(gch->ptszText);
	delete gch;
}

static void __cdecl GuiOutThread(GCHOOK *gch)
{
	Thread_SetName("IRC: GuiOutThread");
	CallFunctionAsync(OnHook, gch);
}

INT_PTR __cdecl CIrcProto::Scripting_InsertGuiOut(WPARAM, LPARAM lParam)
{
	GCHOOK* gch = (GCHOOK*)lParam;

	if (m_scriptingEnabled && gch) {
		GCHOOK* gchook = new GCHOOK;
		gchook->dwData = gch->dwData;
		gchook->iType = gch->iType;
		if (gch->ptszText)
			gchook->ptszText = wcsdup(gch->ptszText);
		else
			gchook->ptszText = nullptr;

		if (gch->ptszUID)
			gchook->ptszUID = wcsdup(gch->ptszUID);
		else
			gchook->ptszUID = nullptr;

		mir_forkThread<GCHOOK>(GuiOutThread, gchook);
		return 0;
	}

	return 1;
}

INT_PTR __cdecl CIrcProto::Scripting_GetIrcData(WPARAM, LPARAM lparam)
{
	if (m_scriptingEnabled && lparam) {
		CMStringA sString = (char*)lparam, sRequest;
		CMStringW sOutput, sChannel;

		int i = sString.Find("|");
		if (i != -1) {
			sRequest = sString.Mid(0, i);
			wchar_t* p = mir_a2u(sString.Mid(i + 1));
			sChannel = p;
			mir_free(p);
		}
		else sRequest = sString;

		sRequest.MakeLower();

		if (sRequest == "ownnick" && IsConnected())
			sOutput = m_info.sNick;

		else if (sRequest == "primarynick")
			sOutput = m_nick;

		else if (sRequest == "secondarynick")
			sOutput = m_alternativeNick;

		else if (sRequest == "myip")
			return (INT_PTR)mir_strdup(m_manualHost ? m_mySpecifiedHostIP :
			(m_IPFromServer) ? m_myHost : m_myLocalHost);

		else if (sRequest == "usercount" && !sChannel.IsEmpty()) {
			GC_INFO gci = { 0 };
			gci.Flags = GCF_BYID | GCF_COUNT;
			gci.pszModule = m_szModuleName;
			gci.pszID = sChannel.c_str();
			if (!Chat_GetInfo(&gci)) {
				wchar_t szTemp[40];
				mir_snwprintf(szTemp, L"%u", gci.iCount);
				sOutput = szTemp;
			}
		}
		else if (sRequest == "userlist" && !sChannel.IsEmpty()) {
			GC_INFO gci = { 0 };
			gci.Flags = GCF_BYID | GCF_USERS;
			gci.pszModule = m_szModuleName;
			gci.pszID = sChannel.c_str();
			if (!Chat_GetInfo(&gci))
				return (INT_PTR)mir_strdup(gci.pszUsers);
		}
		else if (sRequest == "channellist") {
			CMStringW S = L"";
			int n = g_chatApi.SM_GetCount(m_szModuleName);
			if (n >= 0) {
				int j = 0;
				while (j < n) {
					GC_INFO gci = { 0 };
					gci.Flags = GCF_BYINDEX | GCF_ID;
					gci.pszModule = m_szModuleName;
					gci.iItem = j;
					if (!Chat_GetInfo(&gci)) {
						if (mir_wstrcmpi(gci.pszID, SERVERWINDOW)) {
							CMStringW S1 = gci.pszID;
							int k = S1.Find(L" ");
							if (k != -1)
								S1 = S1.Mid(0, k);
							S += S1 + L" ";
						}
					}
					j++;
				}
			}

			if (!S.IsEmpty())
				sOutput = (wchar_t*)S.c_str();
		}
		// send it to mbot
		if (!sOutput.IsEmpty())
			return (INT_PTR)mir_u2a(sOutput.c_str());
	}
	return 0;
}
