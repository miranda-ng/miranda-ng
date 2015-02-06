/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-15 Robert Pösel

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

#include "common.h"

MCONTACT FacebookProto::AddToListByEvent(int, int, MEVENT)
{
	return NULL;
}

int FacebookProto::AuthRecv(MCONTACT, PROTORECVEVENT *)
{
	return 1;
}

HANDLE FacebookProto::FileAllow(MCONTACT, HANDLE, const PROTOCHAR *)
{
	return NULL;
}

int FacebookProto::FileCancel(MCONTACT, HANDLE)
{
	return 1;
}

int FacebookProto::FileDeny(MCONTACT, HANDLE, const PROTOCHAR *)
{
	return 1;
}

int FacebookProto::FileResume(HANDLE, int *, const PROTOCHAR **)
{
	return 1;
}

HWND FacebookProto::SearchAdvanced(HWND)
{
	return NULL;
}

HWND FacebookProto::CreateExtendedSearchUI(HWND)
{
	return NULL;
}

int FacebookProto::RecvContacts(MCONTACT, PROTORECVEVENT *)
{
	return 1;
}

int FacebookProto::RecvFile(MCONTACT, PROTORECVFILET *)
{
	return 1;
}

int FacebookProto::RecvUrl(MCONTACT, PROTORECVEVENT *)
{
	return 1;
}

int FacebookProto::SendContacts(MCONTACT, int, int, MCONTACT *)
{
	return 1;
}

HANDLE FacebookProto::SendFile(MCONTACT, const PROTOCHAR *, PROTOCHAR **)
{
	return NULL;
}

int FacebookProto::SendUrl(MCONTACT, int, const char *)
{
	return 1;
}

int FacebookProto::SetApparentMode(MCONTACT, int)
{
	return 1;
}

int FacebookProto::RecvAwayMsg(MCONTACT, int, PROTORECVEVENT *)
{
	return 1;
}
