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

#include "proto.h"

HANDLE TwitterProto::AddToListByEvent(int flags,int iContact,HANDLE hDbEvent)
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

int TwitterProto::AuthRecv(HCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int TwitterProto::AuthRequest(HCONTACT hContact,const TCHAR *message)
{
	return 1;
}

HANDLE TwitterProto::ChangeInfo(int type,void *info_data)
{
	MessageBoxA(0,"ChangeInfo","",0);
	return NULL;
}

HANDLE TwitterProto::FileAllow(HCONTACT hContact,HANDLE hTransfer,const TCHAR *path)
{
	return NULL;
}

int TwitterProto::FileCancel(HCONTACT hContact,HANDLE hTransfer)
{
	return 1;
}

int TwitterProto::FileDeny(HCONTACT hContact,HANDLE hTransfer,const TCHAR *reason)
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

int TwitterProto::RecvContacts(HCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int TwitterProto::RecvFile(HCONTACT hContact,PROTORECVFILET *)
{
	return 1;
}

int TwitterProto::RecvUrl(HCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int TwitterProto::SendContacts(HCONTACT hContact, int flags, int nContacts, HCONTACT *hContactsList)
{
	return 1;
}

HANDLE TwitterProto::SendFile(HCONTACT hContact,const TCHAR *desc, TCHAR **files)
{
	return NULL;
}

int TwitterProto::SendUrl(HCONTACT hContact,int flags,const char *url)
{
	return 1;
}

int TwitterProto::SetApparentMode(HCONTACT hContact,int mode)
{
	return 1;
}

int TwitterProto::RecvAwayMsg(HCONTACT hContact,int mode,PROTORECVEVENT *evt)
{
	return 1;
}

int TwitterProto::SetAwayMsg(int status,const TCHAR *msg)
{
	return 1;
}

int TwitterProto::UserIsTyping(HCONTACT hContact,int type)
{
	return 1;
}