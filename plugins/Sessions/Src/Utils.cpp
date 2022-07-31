/*
Sessions Management plugin for Miranda IM

Copyright (C) 2007-2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void CSession::fromString(const char *str)
{
	JSONNode root = JSONNode::parse(str);
	if (!root)
		return;

	wszName = root["n"].as_mstring();
	bIsUser = root["u"].as_int() != 0;
	bIsFavorite = root["f"].as_int() != 0;

	for (auto &mm : root["m"])
		contacts.push_back(mm.as_int());
}

CMStringA CSession::getSetting() const
{
	if (bIsUser)
		return CMStringA(FORMAT, "SessionUser_%d", id);
	
	return CMStringA(FORMAT, "SessionDate_%d", id);
}

void CSession::remove()
{
	g_plugin.delSetting(getSetting());

	if (bIsUser)
		g_arUserSessions.remove(this);
	else
		g_arDateSessions.remove(this);
}

void CSession::save()
{
	g_plugin.setUString(getSetting(), toString().c_str());
}

std::string CSession::toString() const
{
	JSONNode members(JSON_ARRAY); members.set_name("m");
	for (auto &cc : contacts)
		members.push_back(JSONNode("", int(cc)));

	JSONNode root;
	root << WCHAR_PARAM("n", wszName) << INT_PARAM("f", bIsFavorite) << INT_PARAM("u", bIsUser) << members;
	return root.write();
}

/////////////////////////////////////////////////////////////////////////////////////////

void DeleteAutoSession(CSession *pSession)
{
	char szSessionName[256];
	mir_snprintf(szSessionName, "%s_%u", "SessionDate", pSession->id);
	g_plugin.delSetting(szSessionName);

	g_arDateSessions.remove(pSession);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CheckForDuplicate(MCONTACT *contact_list, MCONTACT hContact)
{
	for (int i = 0;; i++) {
		if (contact_list[i] == hContact)
			return i;
		if (contact_list[i] == 0)
			return -1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void LoadSessionToCombobox(CCtrlCombo &combo, bool bUser)
{
	if (session_list_recovered[0] && !bUser)
		combo.AddString(TranslateT("Session Before Last Crash"), 0);

	auto &pList = (bUser) ? g_arUserSessions : g_arDateSessions;
	for (auto &it : pList) {
		if (it->bIsFavorite || bUser)
			combo.AddString(it->wszName, LPARAM(it));
		else
			combo.InsertString(it->wszName, 0, LPARAM(it));
	}
}

void OffsetWindow(HWND parent, HWND hwnd, int dx, int dy)
{
	POINT pt;
	RECT rc;
	GetWindowRect(hwnd, &rc);
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(parent, &pt);
	rc.left = pt.x;
	rc.top = pt.y;

	OffsetRect(&rc, dx, dy);
	SetWindowPos(hwnd, nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

int CheckContactVisibility(MCONTACT hContact)
{
	return db_mc_isSub(hContact) || !Contact::IsHidden(hContact);
}

void RenameUserDefSession(int ses_count, wchar_t* ptszNewName)
{
	char szSession[256];
	mir_snprintf(szSession, "%s_%u", "SessionUser", ses_count);
	g_plugin.setWString(szSession, ptszNewName);
}

void SavePosition(HWND hwnd, char *wndName)
{
	RECT rc;
	GetWindowRect(hwnd, &rc);
	char buffer[512];
	mir_snprintf(buffer, "%sPosX", wndName);
	g_plugin.setDword(buffer, rc.left);
	mir_snprintf(buffer, "%sPosY", wndName);
	g_plugin.setDword(buffer, rc.top);
}

void LoadPosition(HWND hWnd, char *wndName)
{
	char buffer[512];
	mir_snprintf(buffer, "%sPosX", wndName);
	int x = g_plugin.getDword(buffer, ((GetSystemMetrics(SM_CXSCREEN)) / 2) - 130);
	mir_snprintf(buffer, "%sPosY", wndName);
	int y = g_plugin.getDword(buffer, ((GetSystemMetrics(SM_CYSCREEN)) / 2) - 80);
	SetWindowPos(hWnd, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE);
}
