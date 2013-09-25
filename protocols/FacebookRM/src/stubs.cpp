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

HANDLE FacebookProto::AddToListByEvent(int flags,int iContact,HANDLE hDbEvent)
{
	return NULL;
}

int FacebookProto::AuthRecv(HANDLE hContact,PROTORECVEVENT *)
{
	return 1;
}

HANDLE FacebookProto::ChangeInfo(int type,void *info_data)
{
	MessageBoxA(0,"ChangeInfo","",0);
	return NULL;
}

HANDLE FacebookProto::FileAllow(HANDLE hContact,HANDLE hTransfer,const PROTOCHAR *path)
{
	return NULL;
}

int FacebookProto::FileCancel(HANDLE hContact,HANDLE hTransfer)
{
	return 1;
}

int FacebookProto::FileDeny(HANDLE hContact,HANDLE hTransfer,const PROTOCHAR *reason)
{
	return 1;
}

int FacebookProto::FileResume(HANDLE hTransfer,int *action,const PROTOCHAR **filename)
{
	return 1;
}

int FacebookProto::GetInfo(HANDLE hContact, int infoType)
{
	// TODO: Most probably some ProtoAck should be here instead
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

int FacebookProto::RecvContacts(HANDLE hContact,PROTORECVEVENT *)
{
	return 1;
}

int FacebookProto::RecvFile(HANDLE hContact,PROTORECVFILET *)
{
	return 1;
}

int FacebookProto::RecvUrl(HANDLE hContact,PROTORECVEVENT *)
{
	return 1;
}

int FacebookProto::SendContacts(HANDLE hContact,int flags,int nContacts,HANDLE *hContactsList)
{
	return 1;
}

HANDLE FacebookProto::SendFile(HANDLE hContact,const PROTOCHAR *desc, PROTOCHAR **files)
{
	return NULL;
}

int FacebookProto::SendUrl(HANDLE hContact,int flags,const char *url)
{
	return 1;
}

int FacebookProto::SetApparentMode(HANDLE hContact,int mode)
{
	return 1;
}

int FacebookProto::RecvAwayMsg(HANDLE hContact,int mode,PROTORECVEVENT *evt)
{
	return 1;
}
