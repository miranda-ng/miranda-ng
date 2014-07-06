/*
Copyright © 2009 Jim Porter

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
#include "proto.h"

MCONTACT TwitterProto::AddToListByEvent(int flags,int iContact,HANDLE hDbEvent)
{
	return NULL;
}

int TwitterProto::Authorize(HANDLE hDbEvent)
{
	return 1;
}

int TwitterProto::AuthDeny(HANDLE hDbEvent,const TCHAR *reason)
{
	return 1;
}

int TwitterProto::AuthRecv(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int TwitterProto::AuthRequest(MCONTACT hContact,const TCHAR *message)
{
	return 1;
}

HANDLE TwitterProto::FileAllow(MCONTACT hContact,HANDLE hTransfer,const TCHAR *path)
{
	return NULL;
}

int TwitterProto::FileCancel(MCONTACT hContact,HANDLE hTransfer)
{
	return 1;
}

int TwitterProto::FileDeny(MCONTACT hContact,HANDLE hTransfer,const TCHAR *reason)
{
	return 1;
}

int TwitterProto::FileResume(HANDLE hTransfer,int *action,const TCHAR **filename)
{
	return 1;
}

HANDLE TwitterProto::SearchByName(const TCHAR *nick,const TCHAR *first_name, const TCHAR *last_name)
{
	return NULL;
}

HWND TwitterProto::SearchAdvanced(HWND owner)
{
	return NULL;
}

HWND TwitterProto::CreateExtendedSearchUI(HWND owner)
{
	return NULL;
}

int TwitterProto::RecvContacts(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int TwitterProto::RecvFile(MCONTACT hContact,PROTORECVFILET *)
{
	return 1;
}

int TwitterProto::RecvUrl(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int TwitterProto::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList)
{
	return 1;
}

HANDLE TwitterProto::SendFile(MCONTACT hContact,const TCHAR *desc, TCHAR **files)
{
	return NULL;
}

int TwitterProto::SendUrl(MCONTACT hContact,int flags,const char *url)
{
	return 1;
}

int TwitterProto::SetApparentMode(MCONTACT hContact,int mode)
{
	return 1;
}

int TwitterProto::RecvAwayMsg(MCONTACT hContact,int mode,PROTORECVEVENT *evt)
{
	return 1;
}

int TwitterProto::SetAwayMsg(int status,const TCHAR *msg)
{
	return 1;
}

int TwitterProto::UserIsTyping(MCONTACT hContact,int type)
{
	return 1;
}