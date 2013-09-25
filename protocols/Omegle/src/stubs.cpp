/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-13 Robert Pösel

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

HANDLE OmegleProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	return NULL;
};

HANDLE OmegleProto::AddToListByEvent(int flags,int iContact,HANDLE hDbEvent)
{
	return NULL;
};

int OmegleProto::Authorize(HANDLE hDbEvent)
{
	return 1;
}

int OmegleProto::AuthDeny(HANDLE hDbEvent,const PROTOCHAR *reason)
{
	return 1;
}

int OmegleProto::AuthRecv(HANDLE hContact,PROTORECVEVENT *)
{
	return 1;
}

int OmegleProto::AuthRequest(HANDLE hContact,const PROTOCHAR *message)
{
	return 1;
}

HANDLE OmegleProto::ChangeInfo(int type,void *info_data)
{
	return NULL;
}

HANDLE OmegleProto::FileAllow(HANDLE hContact,HANDLE hTransfer,const PROTOCHAR *path)
{
	return NULL;
}

int OmegleProto::FileCancel(HANDLE hContact,HANDLE hTransfer)
{
	return 1;
}

int OmegleProto::FileDeny(HANDLE hContact,HANDLE hTransfer,const PROTOCHAR *reason)
{
	return 1;
}

int OmegleProto::FileResume(HANDLE hTransfer,int *action,const PROTOCHAR **filename)
{
	return 1;
}

int OmegleProto::GetInfo( HANDLE hContact, int infoType )
{
	return 1;
}

HANDLE OmegleProto::SearchBasic( const PROTOCHAR* id )
{
	return NULL;
}

HANDLE OmegleProto::SearchByEmail( const PROTOCHAR* email )
{
	return NULL;
}

HANDLE OmegleProto::SearchByName( const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName )
{
	return NULL;
}

HWND OmegleProto::SearchAdvanced(HWND owner)
{
	return NULL;
}

HWND OmegleProto::CreateExtendedSearchUI(HWND owner)
{
	return NULL;
}

int OmegleProto::RecvContacts(HANDLE hContact,PROTORECVEVENT *)
{
	return 1;
}

int OmegleProto::RecvFile(HANDLE hContact,PROTORECVFILET *)
{
	return 1;
}

int OmegleProto::RecvUrl(HANDLE hContact,PROTORECVEVENT *)
{
	return 1;
}

int OmegleProto::SendContacts(HANDLE hContact,int flags,int nContacts,HANDLE *hContactsList)
{
	return 1;
}

HANDLE OmegleProto::SendFile(HANDLE hContact,const PROTOCHAR *desc, PROTOCHAR **files)
{
	return NULL;
}

int OmegleProto::SendUrl(HANDLE hContact,int flags,const char *url)
{
	return 1;
}

int OmegleProto::SetApparentMode(HANDLE hContact,int mode)
{
	return 1;
}

int OmegleProto::RecvAwayMsg(HANDLE hContact,int mode,PROTORECVEVENT *evt)
{
	return 1;
}

int OmegleProto::UserIsTyping(HANDLE hContact, int type)
{
	return 1;
}

int OmegleProto::SetAwayMsg(int iStatus, const PROTOCHAR* msg)
{
	return 1;
}

HANDLE OmegleProto::GetAwayMsg( HANDLE hContact )
{
	return NULL;
}

int OmegleProto::RecvMsg(HANDLE hContact, PROTORECVEVENT *pre)
{
	return 1;
}
