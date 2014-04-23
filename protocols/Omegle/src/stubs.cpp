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

MCONTACT OmegleProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	return NULL;
};

MCONTACT OmegleProto::AddToListByEvent(int flags,int iContact,HANDLE hDbEvent)
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

int OmegleProto::AuthRecv(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int OmegleProto::AuthRequest(MCONTACT hContact,const PROTOCHAR *message)
{
	return 1;
}

HANDLE OmegleProto::FileAllow(MCONTACT hContact,HANDLE hTransfer,const PROTOCHAR *path)
{
	return NULL;
}

int OmegleProto::FileCancel(MCONTACT hContact,HANDLE hTransfer)
{
	return 1;
}

int OmegleProto::FileDeny(MCONTACT hContact,HANDLE hTransfer,const PROTOCHAR *reason)
{
	return 1;
}

int OmegleProto::FileResume(HANDLE hTransfer,int *action,const PROTOCHAR **filename)
{
	return 1;
}

int OmegleProto::GetInfo( MCONTACT hContact, int infoType )
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

int OmegleProto::RecvContacts(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int OmegleProto::RecvFile(MCONTACT hContact,PROTORECVFILET *)
{
	return 1;
}

int OmegleProto::RecvUrl(MCONTACT hContact,PROTORECVEVENT *)
{
	return 1;
}

int OmegleProto::SendContacts(MCONTACT hContact,int flags,int nContacts,MCONTACT *hContactsList)
{
	return 1;
}

HANDLE OmegleProto::SendFile(MCONTACT hContact,const PROTOCHAR *desc, PROTOCHAR **files)
{
	return NULL;
}

int OmegleProto::SendUrl(MCONTACT hContact,int flags,const char *url)
{
	return 1;
}

int OmegleProto::SetApparentMode(MCONTACT hContact,int mode)
{
	return 1;
}

int OmegleProto::RecvAwayMsg(MCONTACT hContact,int mode,PROTORECVEVENT *evt)
{
	return 1;
}

int OmegleProto::SetAwayMsg(int iStatus, const PROTOCHAR* msg)
{
	return 1;
}

HANDLE OmegleProto::GetAwayMsg( MCONTACT hContact )
{
	return NULL;
}

int OmegleProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	return 1;
}
