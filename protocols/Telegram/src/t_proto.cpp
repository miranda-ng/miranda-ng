/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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

CTelegramProto::CTelegramProto(const char* protoName, const TCHAR* userName) : PROTO<CTelegramProto>(protoName, userName)
{
	tgl_register_app_id(&tgl, TELEGRAM_APP_ID, TELEGRAM_APP_HASH);
	char version[64];
	mir_snprintf(version, "Miranda Telegram %d.%d.%d.%d", __MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM);
	tgl_set_app_version(&tgl, version);
	tgl_init(&tgl);

	tgl_update_callback tgucb = {};


}

CTelegramProto::~CTelegramProto()
{
}

DWORD_PTR CTelegramProto::GetCaps(int type, MCONTACT)
{
	return 0;
}

MCONTACT CTelegramProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	return 0;
}

MCONTACT CTelegramProto::AddToListByEvent(int, int, MEVENT hDbEvent)
{
	return 0;
}

int CTelegramProto::Authorize(MEVENT hDbEvent)
{
	return 0;
}

int CTelegramProto::AuthDeny(MEVENT hDbEvent, const TCHAR*)
{
	return 0;

}

int CTelegramProto::AuthRecv(MCONTACT, PROTORECVEVENT* pre)
{
	return 0;
}

int CTelegramProto::AuthRequest(MCONTACT hContact, const TCHAR *szMessage)
{
	return 0;
}

int CTelegramProto::GetInfo(MCONTACT hContact, int)
{
	return 0;
}

int CTelegramProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	return 0;
}

int CTelegramProto::SetStatus(int iNewStatus)
{
	return 0;
}

int CTelegramProto::UserIsTyping(MCONTACT hContact, int type)
{
	return 0;
}

int CTelegramProto::OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int CTelegramProto::OnPreShutdown(WPARAM, LPARAM)
{
	return 0;
}
