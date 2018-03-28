/*
Copyright (c) 2015-18 Miranda NG team (https://miranda-ng.org)

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

int CSkypeProto::OnAccountLoaded(WPARAM, LPARAM)
{
	setAllContactStatuses(ID_STATUS_OFFLINE, true);

	HookProtoEvent(ME_OPT_INITIALISE, &CSkypeProto::OnOptionsInit);
	HookProtoEvent(ME_MSG_PRECREATEEVENT, &CSkypeProto::OnPreCreateMessage);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &CSkypeProto::OnDbEventRead);

	InitDBEvents();
	InitPopups();
	InitGroupChatModule();

	return 0;
}

INT_PTR CSkypeProto::OnAccountManagerInit(WPARAM, LPARAM lParam)
{
	return (INT_PTR)(CSkypeOptionsMain::CreateAccountManagerPage(this, (HWND)lParam))->GetHwnd();
}
