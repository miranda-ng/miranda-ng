/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
//this module was created in v0.1.1.0

//this header file is for the use of protocol modules only. Other users should
//use the functions exposed in m_protocols.h and m_protosvc.h

#ifndef M_PROTOMOD_H__
#define M_PROTOMOD_H__ 1

#include <stdio.h>

#include "m_protocols.h"

//notify the protocol manager that you're around
//wParam=0
//lParam=(PROTOCOLDESCRIPTOR*)&descriptor
//returns 0 on success, nonzero on failure
//This service must be called in your module's Load() routine.
//descriptor.type can be a value other than the PROTOTYPE_ constants specified
//above to provide more precise positioning information for the contact
//protocol lists. It is strongly recommended that you give values relative to
//the constants, however, by adding or subtracting small integers (<=100).
//PROTOTYPE_PROTOCOL modules must not do this. The value must be exact.
//See MS_PROTO_ENUMPROTOCOLS for more notes.
#define MS_PROTO_REGISTERMODULE    "Proto/RegisterModule"

//adds the specified protocol module to the chain for a contact
//wParam=(WPARAM)(HANDLE)hContact
//lParam=(LPARAM)(const char*)szName
//returns 0 on success, nonzero on failure
//The module is added in the correct position according to the type given when
//it was registered.
#define MS_PROTO_ADDTOCONTACT      "Proto/AddToContact"

//removes the specified protocol module from the chain for a contact
//wParam=(WPARAM)(HANDLE)hContact
//lParam=(LPARAM)(const char*)szName
//returns 0 on success, nonzero on failure
#define MS_PROTO_REMOVEFROMCONTACT      "Proto/RemoveFromContact"

//Create a protocol service
//Protocol services are called with wParam and lParam as standard if they are
//to be called with CallProtoService() (as PS_ services are)
//If they are called with CallContactService() (PSS_ and PSR_ services) then
//they are called with lParam=(CCSDATA*)&ccs and wParam an opaque internal
//reference that should be passed unchanged to MS_PROTO_CHAIN*.
__inline static HANDLE CreateProtoServiceFunction(const char *szModule,const char *szService,MIRANDASERVICE serviceProc)
{
	char str[MAXMODULELABELLENGTH];
	_snprintf(str, sizeof(str), "%s%s", szModule, szService);
    str[MAXMODULELABELLENGTH-1] = 0;
	return CreateServiceFunction(str,serviceProc);
}

//Call the next service in the chain for this send operation
//wParam=wParam
//lParam=lParam
//The return value should be returned immediately
//wParam and lParam should be passed as the parameters that your service was
//called with. wParam must remain untouched but lParam is a CCSDATA structure
//that can be copied and modified if needed.
//Typically, the last line of any chaining protocol function is
//return CallService(MS_PROTO_CHAINSEND,wParam,lParam);
#define MS_PROTO_CHAINSEND       "Proto/ChainSend"

//Call the next service in the chain for this receive operation
//wParam=wParam
//lParam=lParam
//The return value should be returned immediately
//wParam and lParam should be passed as the parameters that your service was
//called with. wParam must remain untouched but lParam is a CCSDATA structure
//that can be copied and modified if needed.
//When being initiated by the network-access protocol module, wParam should be
//zero.
//Thread safety: ms_proto_chainrecv is completely thread safe since 0.1.2.0
//Calls to it are translated to the main thread and passed on from there. The
//function will not return until all callees have returned, irrepective of
//differences between threads the functions are in.
#define MS_PROTO_CHAINRECV       "Proto/ChainRecv"

//Broadcast a ME_PROTO_ACK event
//wParam=0
//lParam=(LPARAM)(ACKDATA*)&ack
//returns the return value of the notifyeventhooks() call
//Thread safety: me_proto_ack is completely thread safe since 0.1.2.0
//See the notes in core/modules.h under NotifyEventHooks()
#define MS_PROTO_BROADCASTACK    "Proto/BroadcastAck"
__inline static INT_PTR ProtoBroadcastAck(const char *szModule,HANDLE hContact,int type,int result,HANDLE hProcess,LPARAM lParam)
{
	ACKDATA ack={0};
	ack.cbSize=sizeof(ACKDATA);
	ack.szModule=szModule; ack.hContact=hContact;
	ack.type=type; ack.result=result;
	ack.hProcess=hProcess; ack.lParam=lParam;
	return CallService(MS_PROTO_BROADCASTACK,0,(LPARAM)&ack);
}

/* -- Added during 0.3.4 (2004/09/27) development! -----
*/

/*
	wParam: (HANDLE)hContact
	lParam: 0
	Affect: Given a hContact, return the protocol that is registered for it, or NULL if no such protocol exists,
			the returned string does not have to be freed and is valid even for multiple threads.
	Note:	Prior to 2004/09/28 this service WAS NOT THREAD SAFE and was slower
	Note:	Prior to 2004/09/28 this service would return NULL for a hContact if the protocol module
			associated with the hContact was not currently loaded, no such check is performed now.
	Version: 0.3.4 (2004/09/28)
*/
#define MS_PROTODIR_PROTOFROMCONTACT MS_PROTO_GETCONTACTBASEPROTO

#endif // M_PROTOMOD_H__


