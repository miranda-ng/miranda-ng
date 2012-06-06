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
	return 0;
}

int TwitterProto::Authorize(HANDLE hContact)
{
	return 0;
}

int TwitterProto::AuthDeny(HANDLE hContact,const char *reason)
{
	return 0;
}

int TwitterProto::AuthRecv(HANDLE hContact,PROTORECVEVENT *)
{
	return 0;
}

int TwitterProto::AuthRequest(HANDLE hContact,const char *message)
{
	return 0;
}

HANDLE TwitterProto::ChangeInfo(int type,void *info_data)
{
	MessageBoxA(0,"ChangeInfo","",0);
	return 0;
}

HANDLE TwitterProto::FileAllow(HANDLE hContact,HANDLE hTransfer,const char *path)
{
	return 0;
}

int TwitterProto::FileCancel(HANDLE hContact,HANDLE hTransfer)
{
	return 0;
}

int TwitterProto::FileDeny(HANDLE hContact,HANDLE hTransfer,const char *reason)
{
	return 0;
}

int TwitterProto::FileResume(HANDLE hTransfer,int *action,const char **filename)
{
	return 0;
}

HANDLE TwitterProto::SearchByName(const char *nick,const char *first_name,
                                  const char *last_name)
{
	return 0;
}

HWND TwitterProto::SearchAdvanced(HWND owner)
{
	return 0;
}

HWND TwitterProto::CreateExtendedSearchUI(HWND owner)
{
	return 0;
}

int TwitterProto::RecvContacts(HANDLE hContact,PROTORECVEVENT *)
{
	return 0;
}

int TwitterProto::RecvFile(HANDLE hContact,PROTORECVFILE *)
{
	return 0;
}

int TwitterProto::RecvUrl(HANDLE hContact,PROTORECVEVENT *)
{
	return 0;
}

int TwitterProto::SendContacts(HANDLE hContact,int flags,int nContacts,HANDLE *hContactsList)
{
	return 0;
}

HANDLE TwitterProto::SendFile(HANDLE hContact,const char *desc, char **files)
{
	return 0;
}

int TwitterProto::SendUrl(HANDLE hContact,int flags,const char *url)
{
	return 0;
}

int TwitterProto::SetApparentMode(HANDLE hContact,int mode)
{
	return 0;
}

int TwitterProto::RecvAwayMsg(HANDLE hContact,int mode,PROTORECVEVENT *evt)
{
	return 0;
}

int TwitterProto::SendAwayMsg(HANDLE hContact,HANDLE hProcess,const char *msg)
{
	return 0;
}

int TwitterProto::SetAwayMsg(int status,const char *msg)
{
	return 0;
}

int TwitterProto::UserIsTyping(HANDLE hContact,int type)
{ 
	return 0;
}