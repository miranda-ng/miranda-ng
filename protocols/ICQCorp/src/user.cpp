/*
	 ICQ Corporate protocol plugin for Miranda IM.
	 Copyright (C) 2003-2005 Eugene Tarasenko <zlyden13@inbox.ru>

	 This program is free software; you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation; either version 2 of the License, or
	 (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	 GNU General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program; if not, write to the Free Software
	 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
	 */

#include "stdafx.h"

std::vector <ICQUser *> icqUsers;

///////////////////////////////////////////////////////////////////////////////

ICQUser::ICQUser() :
	socket(WM_NETEVENT_USER)
{
}

///////////////////////////////////////////////////////////////////////////////

void ICQUser::setStatus(unsigned short newStatus)
{
	if (statusVal == newStatus)
		return;

	statusVal = newStatus;
	g_plugin.setWord(hContact, "Status", newStatus);
}

///////////////////////////////////////////////////////////////////////////////

void ICQUser::setInfo(char *name, unsigned int data)
{
	if (data && data != 0xFFFFFFFF)
		g_plugin.setDword(hContact, name, data);
	else
		g_plugin.delSetting(hContact, name);
}

///////////////////////////////////////////////////////////////////////////////

void ICQUser::setInfo(char *name, unsigned short data)
{
	if (data && data != 0xFFFF)
		g_plugin.setWord(hContact, name, data);
	else
		g_plugin.delSetting(hContact, name);
}

///////////////////////////////////////////////////////////////////////////////

void ICQUser::setInfo(char *name, unsigned char data)
{
	if (data && data != 0xFF)
		g_plugin.setByte(hContact, name, data);
	else
		g_plugin.delSetting(hContact, name);
}

///////////////////////////////////////////////////////////////////////////////

void ICQUser::setInfo(char *name, char *data)
{
	if (data[0])
		g_plugin.setString(hContact, name, data);
	else
		g_plugin.delSetting(hContact, name);
}

///////////////////////////////////////////////////////////////////////////////

static char* iptoa(unsigned int ip)
{
	struct in_addr addr;
	addr.S_un.S_addr = htonl(ip);
	return inet_ntoa(addr);
}

///////////////////////////////////////////////////////////////////////////////

static void setTextValue(HWND hWnd, int id, const wchar_t *value)
{
	bool unspecified = value == nullptr;

	EnableWindow(GetDlgItem(hWnd, id), !unspecified);
	SetDlgItemText(hWnd, id, unspecified ? TranslateT("<not specified>") : value);
}

///////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK icqUserInfoDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR hdr;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		return TRUE;

	case WM_NOTIFY:
		hdr = (LPNMHDR)lParam;
		if (hdr->idFrom == 0 && hdr->code == PSN_INFOCHANGED) {
			wchar_t buffer[64];
			unsigned long ip, port;
			MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;

			_itow(g_plugin.getDword(hContact, "UIN", 0), buffer, 10);
			setTextValue(hWnd, IDC_INFO_UIN, buffer);

			ip = g_plugin.getDword(hContact, "IP", 0);
			setTextValue(hWnd, IDC_INFO_IP, ip ? _A2T(iptoa(ip)) : nullptr);

			ip = g_plugin.getDword(hContact, "RealIP", 0);
			setTextValue(hWnd, IDC_INFO_REALIP, ip ? _A2T(iptoa(ip)) : nullptr);

			port = g_plugin.getWord(hContact, "Port", 0);
			_itow(port, buffer, 10);
			setTextValue(hWnd, IDC_INFO_PORT, port ? buffer : nullptr);

			setTextValue(hWnd, IDC_INFO_VERSION, nullptr);
			setTextValue(hWnd, IDC_INFO_MIRVER, nullptr);
			setTextValue(hWnd, IDC_INFO_PING, nullptr);
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL) SendMessage(GetParent(hWnd), msg, wParam, lParam);
		break;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////

int icqUserInfoInitialise(WPARAM wParam, LPARAM lParam)
{
	char *proto = Proto_GetBaseAccountName(lParam);
	if ((proto == nullptr || mir_strcmp(proto, protoName)) && lParam)
		return 0;

	OPTIONSDIALOGPAGE odp = {};
	odp.position = -1900000000;
	odp.szTitle.a = protoName;
	odp.pfnDlgProc = icqUserInfoDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_ICQCORP);
	g_plugin.addUserInfo(wParam, &odp);
	return 0;
}
