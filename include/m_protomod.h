/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#include <m_protocols.h>
#include <m_protosvc.h>

//notify the protocol manager that you're around
//wParam = 0
//lParam = (PROTOCOLDESCRIPTOR*)&descriptor
//returns 0 on success, nonzero on failure
//This service must be called in your module's Load(void) routine.
//descriptor.type can be a value other than the PROTOTYPE_ constants specified
//above to provide more precise positioning information for the contact
//protocol lists. It is strongly recommended that you give values relative to
//the constants, however, by adding or subtracting small integers ( <= 100).
//PROTOTYPE_PROTOCOL modules must not do this. The value must be exact.
//See MS_PROTO_ENUMPROTOCOLS for more notes.
#define MS_PROTO_REGISTERMODULE    "Proto/RegisterModule"

//adds the specified protocol module to the chain for a contact
//wParam = (MCONTACT)hContact
//lParam = (LPARAM)(const char*)szName
//returns 0 on success, nonzero on failure
//The module is added in the correct position according to the type given when
//it was registered.
#define MS_PROTO_ADDTOCONTACT      "Proto/AddToContact"

//removes the specified protocol module from the chain for a contact
//wParam = (MCONTACT)hContact
//lParam = (LPARAM)(const char*)szName
//returns 0 on success, nonzero on failure
#define MS_PROTO_REMOVEFROMCONTACT      "Proto/RemoveFromContact"

//Call the next service in the chain for this send operation
//wParam = wParam
//lParam = lParam
//The return value should be returned immediately
//wParam and lParam should be passed as the parameters that your service was
//called with. wParam must remain untouched but lParam is a CCSDATA structure
//that can be copied and modified if needed.
//Typically, the last line of any chaining protocol function is
//return CallService(MS_PROTO_CHAINSEND, wParam, lParam);
#define MS_PROTO_CHAINSEND       "Proto/ChainSend"

//Call the next service in the chain for this receive operation
//wParam = wParam
//lParam = lParam
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

__forceinline INT_PTR ProtoChainRecv(MCONTACT hContact, char *szService, WPARAM wParam, LPARAM lParam)
{	CCSDATA ccs = { hContact, szService, wParam, lParam };
	return CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
}

__forceinline INT_PTR ProtoChainRecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{	CCSDATA ccs = { hContact, PSR_MESSAGE, 0, (LPARAM)pre };
	return CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
}

__forceinline INT_PTR ProtoChainRecvFile(MCONTACT hContact, PROTORECVFILET *pre)
{	CCSDATA ccs = { hContact, PSR_FILE, 0, (LPARAM)pre };
	return CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
}

//Broadcast a ME_PROTO_ACK event
//wParam = 0
//lParam = (LPARAM)(ACKDATA*)&ack
//returns the return value of the notifyeventhooks() call
//Thread safety: me_proto_ack is completely thread safe since 0.1.2.0
//See the notes in core/modules.h under NotifyEventHooks()
#define MS_PROTO_BROADCASTACK    "Proto/BroadcastAck"

#endif // M_PROTOMOD_H__
