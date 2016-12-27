/*
Copyright © 2016 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

ÑDiscordProto::ÑDiscordProto(const char *proto_name, const wchar_t *username) :
	PROTO<ÑDiscordProto>(proto_name, username)
{
	// Services
	CreateProtoService(PS_GETNAME, &ÑDiscordProto::GetName);
	CreateProtoService(PS_GETSTATUS, &ÑDiscordProto::GetStatus);

	// Events
	HookProtoEvent(ME_OPT_INITIALISE, &ÑDiscordProto::OnOptionsInit);
}

ÑDiscordProto::~ÑDiscordProto()
{
}

DWORD_PTR ÑDiscordProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSGRECV | PF1_SERVERCLIST;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_3:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_AVATARS;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)"E-mail";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)DB_KEY_EMAIL;
	}
	return 0;
}

INT_PTR ÑDiscordProto::GetName(WPARAM wParam, LPARAM lParam)
{
	mir_strncpy((char*)lParam, m_szModuleName, (int)wParam);
	return 0;
}

INT_PTR ÑDiscordProto::GetStatus(WPARAM, LPARAM)
{
	return m_iStatus;
}

/////////////////////////////////////////////////////////////////////////////////////////

int ÑDiscordProto::OnModulesLoaded(WPARAM, LPARAM)
{
	return 0;
}

int ÑDiscordProto::OnPreShutdown(WPARAM, LPARAM)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int ÑDiscordProto::OnEvent(PROTOEVENTTYPE event, WPARAM wParam, LPARAM lParam)
{
	switch (event) {
		case EV_PROTO_ONLOAD:    return OnModulesLoaded(wParam, lParam);
		case EV_PROTO_ONEXIT:    return OnPreShutdown(wParam, lParam);
		case EV_PROTO_ONOPTIONS: return OnOptionsInit(wParam, lParam);
	}

	return 1;
}
