/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-13 Robert Pösel

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

MCONTACT FacebookProto::AddToListByEvent(int flags,int iContact,HANDLE hDbEvent)
{
	return NULL;
}

int FacebookProto::AuthRecv(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

HANDLE FacebookProto::FileAllow(MCONTACT hContact,HANDLE hTransfer,const PROTOCHAR *path)
{
	return NULL;
}

int FacebookProto::FileCancel(MCONTACT hContact,HANDLE hTransfer)
{
	return 1;
}

int FacebookProto::FileDeny(MCONTACT hContact,HANDLE hTransfer,const PROTOCHAR *reason)
{
	return 1;
}

int FacebookProto::FileResume(HANDLE hTransfer,int *action,const PROTOCHAR **filename)
{
	return 1;
}

HWND FacebookProto::SearchAdvanced(HWND owner)
{
	return NULL;
}

HWND FacebookProto::CreateExtendedSearchUI(HWND owner)
{
	return NULL;
}

int FacebookProto::RecvContacts(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int FacebookProto::RecvFile(MCONTACT hContact,PROTORECVFILET *)
{
	return 1;
}

int FacebookProto::RecvUrl(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int FacebookProto::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList)
{
	return 1;
}

HANDLE FacebookProto::SendFile(MCONTACT hContact,const PROTOCHAR *desc, PROTOCHAR **files)
{
	return NULL;
}

int FacebookProto::SendUrl(MCONTACT hContact,int flags,const char *url)
{
	return 1;
}

int FacebookProto::SetApparentMode(MCONTACT hContact,int mode)
{
	return 1;
}

int FacebookProto::RecvAwayMsg(MCONTACT hContact,int mode,PROTORECVEVENT *evt)
{
	return 1;
}
