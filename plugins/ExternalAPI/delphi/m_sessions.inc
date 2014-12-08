{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2006 Miranda ICQ/IM project, 
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
}

{$IFNDEF M_SESSION}
{$DEFINE M_SESSION}

// Pipe Messages

{
	szEntity=szUI, szProto (always != NULL)
	hSession=yes
	
	A new entity handle is about to be created to bind .szUI for usage by .szProto,
	note that this handle is not yet in the handle list and the .szProto will
	get the message first, then the .szUI will.
	
	.szUI must set up any instance data within the given hSession,
	this is done with Sion_EntityCookieSet(hSession,SDR_UI,data).
	
	.szProto may also set up data to associate with the given hSession,
	this is done with Sion_EntityCookieSet(hSession,SDR_PROTO,data);
	
	This message is always sent from the main thread (a thread context switch
	occurs if needed).
}
const
  ENTITY_CREATE  = 1;
  ENTITY_DESTROY = 2;

// Services/Hooks

{ either .dwTo, dwFrom may have an SDR_* type, and SDR_* flag }
   SDR_ALL   = 1;
   SDR_SION  = 2;
   SDR_PROTO = 3;
   SDR_UI    = 4;

{ you may extend this structure but .cbSize must stay and the reserved data
at the end of the structure must not be rewritten into }
type
  PIPE_DATA = record
    cbSize  :int;
    szEntity:PAnsiChar;   // can be NULL
    hSession:THANDLE; // ""
    dwMsg   :dword;
    dwTo    :dword;
    dwFrom  :dword;   // SDR_*, SDR_ALL is not a good thing
    wParam  :WPARAM;
    lParam  :LPARAM;
    reserved:array [0..1] of dword; // is actually apart of the structure and is used internally
  end;

{
wParam=0
lParam=(LPARAM)&PIPE_DATA

Issue a call to an entity by name, type or to everyone, if you send
a message to an entity by name, e.g. "ICQ" it will only goto "ICQ" and not
to anyone else.
}
const
  MS_SION_PIPE:PAnsiChar = 'Sion/PipeCall';

{
wParam=0
lParam=(LPARAM)&PIPE_DATA

Begin your lovely relationship with everyone else who began a relationship
before you, uh.. fill a pipe data structure and call this service :

struct PIPE_DATA pd;
pd.cbSize=sizeof(pd);
pd.dwTo=SDR_PROTO;
pd.szEntity="ICQ";
pd.lParam=(MIRANDASERVICE)MyCallback;
CallService(MS_SION_PIPEHOOK,0,(LPARAM)&pd);

The service returns 0 on success and non zero on failure, once you have registered either as a UI or a protocol
your MIRANDASERVICE will be called on the event on a pipe message that is
either directed to your entity name (.szEntity!=NULL) or by SDR_* type.

Note that the entity name may not be yours, but the pipe system may of been
instructed to issue the call with the caller's entity name given to you, because
you know what your entity name is already, you should not rely on .szEntity
being anything, but if it is there, the message is for you only and no one else
will get it and the value of .szEntity is dependant on the message.
}
  MS_SION_PIPEHOOK:PAnsiChar = 'Sion/PipeHook';

{
wParam=0
lParam=(LPARAM)&SION_ENTITY_DESCRIPTOR

Create an entity handle binded to .szUI and for .szProto, this service will
switch threads if it needs to to send ENTITY_CREATE to both .szUI and .szProto
}
type
  SION_ENTITY_DESCRIPTOR = record
    cbSize  :int;
    szUI    :PAnsiChar;
    szProto :PAnsiChar;
    hSession:THANDLE;	// returned if successful
  end;

const
  MS_SION_ENTITY_CREATE:PAnsiChar = 'Sion/EntityCreate';

{
wParam=0
lParam=(LPARAM)HANDLE

Decrement the given handle reference count by one, this will cause the
handle to be freed if the reference count reaches zero, if this is the case
there will be a thread switch to the main thread (if not called from the main thread)

During handle shutdown, ENTITY_DESTROY will be sent to the protocol and then the UI,
note that you do not need to give .szUI or .szProto because the handle knows
who it is binded to.
}
  MS_SION_ENTITY_RELEASE:PAnsiChar = 'Sion/EntityRelease';

{
wParam=0
lParam=HANDLE

Add one to the reference count of HANDLE.
}
  MS_SION_ENTITY_CLONE:PAnsiChar = 'Sion/EntityClone';

{
wParam=0
lParam=&SION_ENTITY_COOKIE

Given a .hSession and a .dwSdr (SDR_*) code get/set a cookie pointer,
if you pass data=NULL, then the current data stored for (SDR_*) will be
returned, if you want to wipe that data, set persist=0

Note that this function is now thread safe for SDR_UI, SDR_PROTO, SDR_SION,
also note that UI's must store their instance data using this method.
}
type
  SION_ENTITY_COOKIE = record
    cbSize  :int;
    hSession:THANDLE;
    dwSdr   :dword;   // SDR_* type to store data against, this can be SDR_UI or SDR_PROTO
    data    :pointer; // can be NULL
    persist :int;     // if TRUE and data is NULL then data will not be wiped
  end;
const
  MS_SION_ENTITY_SETCOOKIE:PAnsiChar = 'Sion/EntitySetCookie';

{
wParam=0
lParam=&SION_ENTITY_COOKIE

Given .data and SDR_code, finds the associated .hSession and returns
a reference to it, note that .data can not be NULL, .dwSdr is used
to match the type of cookie.
}
  MS_SION_ENTITY_FINDCOOKIE:PAnsiChar = 'Sion/EntityFindCookie';

(*
// -- Helper functions --

__inline int Sion_PipeRegister(dword dwSdr,AnsiChar *szEntity,MIRANDASERVICE pfnService)
{
	struct PIPE_DATA pd;
	pd.cbSize=sizeof(struct PIPE_DATA);
	pd.dwTo=dwSdr;
	pd.szEntity=szEntity;
	pd.lParam=(LPARAM)pfnService;
	return CallService(MS_SION_PIPEHOOK,0,(LPARAM)&pd);
}

__inline HANDLE Sion_EntityCreate(AnsiChar *szProto, AnsiChar *szUI)
{
	struct SION_ENTITY_DESCRIPTOR sed;
	sed.cbSize=sizeof(sed);
	sed.szProto=szProto;
	sed.szUI=szUI;
	sed.hSession=NULL;
	if (!CallService(MS_SION_ENTITY_CREATE,0,(LPARAM)&sed) && sed.hSession) {
		return sed.hSession;
	}
	return NULL;
}

__inline int Sion_EntityRelease(HANDLE seh)
{
	return CallService(MS_SION_ENTITY_RELEASE,0,(LPARAM)seh);
}

__inline int Sion_EntityClone(HANDLE seh)
{
	return CallService(MS_SION_ENTITY_CLONE,0,(LPARAM)seh);
}

__inline void* Sion_EntityCookieGet(HANDLE seh, dword dwSdr)
{
	struct SION_ENTITY_COOKIE sec;
	sec.cbSize=sizeof(sec);
	sec.hSession=seh;
	sec.dwSdr=dwSdr;
	sec.data=NULL;
	sec.persist=1;
	CallService(MS_SION_ENTITY_SETCOOKIE,0,(LPARAM)&sec);
	return sec.data;
}

__inline int Sion_EntityCookieSet(HANDLE seh, dword dwSdr, void* cookie)
{
	struct SION_ENTITY_COOKIE sec;
	sec.cbSize=sizeof(sec);
	sec.hSession=seh;
	sec.dwSdr=dwSdr;
	sec.data=cookie;
	sec.persist=0;
	return CallService(MS_SION_ENTITY_SETCOOKIE,0,(LPARAM)&sec);
}

__inline HANDLE Sion_EntityCookieFind(dword dwSdr, void* cookie)
{
	struct SION_ENTITY_COOKIE sec;
	sec.cbSize=sizeof(sec);
	sec.hSession=NULL;
	sec.dwSdr=dwSdr;
	sec.data=cookie;
	CallService(MS_SION_ENTITY_FINDCOOKIE,0,(LPARAM)&sec);
	return sec.hSession;
}

__inline int Sion_PipeBroadcast(AnsiChar* szEntity, HANDLE hSession, dword dwMsg, 
	WPARAM wParam, LPARAM lParam, dword dwFrom, dword dwTo) {
	struct PIPE_DATA pd;
	pd.cbSize=sizeof(struct PIPE_DATA);
	pd.szEntity=szEntity;
	pd.hSession=hSession;
	pd.dwMsg=dwMsg;
	pd.dwTo=dwTo;
	pd.dwFrom=dwFrom;
	pd.wParam=wParam;
	pd.lParam=lParam;
	return CallService(MS_SION_PIPE,0,(LPARAM)&pd);
}
*)

{
--Pipe Convos--

The following is the planned pathway message, not all may make 
it to the final draft.

Because of the nature of some protocols, there are some
messages that some protocols can ignore since they have no meaning.

SION : sends ENTITY_CREATE after creating a temporary entity handle
PROTO,
UI   : both UI and protocol store structures as cookies within the handle
       which it can later fetch. At this point the UI is assumed single
       contact.
       
SION : sends a message to the protocol to let it know it should allocate transport
PROTO: the proto can assume this from ENTITY_CREATE, but I'm not sure this a good idea.
       
PROTO: Protocol needs to send a message to tell the UI about basic channel stuff
       like if it is multiple contact from the start (IRC style) or open to change
       (MSN style) or if it is private and restricted in all forms of JOIN, INVITE, etc.
UI	 : Can use this message to present information in a 2 person format even if
 	   the protocol level is widly different.
      

SION : ATTACH_CHANNEL or ATTACH_CONTACT
PROTO: will send "JOIN" or "INVITE" to request a join/a contact
       this maybe on the transport just created above.
	   These two messages will be have a HPROCESS code
	   that must be acknowledged later.

	   Note that if the protocol does not require contacts
	   to be attached in this way (invited) then just fake the
	   ATTACHED_* messages.

	   The contacts(s) will be shown in the channel at the UI level
	   even if they are not yet within the channel at the protocol
	   level.

PROTO: sends ATTACHED_CHANNEL or ATTACHED_CONTACT with the HPROCESS
       code given by the ATTACH_* messages, this is to signal
	   that the JOIN was successful or that the invited contact
	   has joined.

	   Note that there maybe more than one ATTACHED_* message.

PROTO: sends a CHANNEL_WHOLIST
UI   : is supposed to listen out for this WHOLIST and present
       a list of people already inside the channel.

	   if for a single user, the channel list maybe hidden.

PROTO: sends a CHANNEL_TOPIC
UI   : displays the topic inside the channel, this message is optional and may not be sent

PROTO: sends a CHANNEL_MODE
UI   : displays the modes that the channel is in, the modes still have to be abstracted
       to Miranda, e.g. IRC mode +M have another Miranda spec flag.

PROTO: sends a CHANNEL_DONE
UI   : the UI is now sure that no more messages are expected.

UI   : sends a UI_TEXT message, with optional source TMCONTACT and of course the message.
PROTO: picks up on this message and transmits it, it must return
       a HPROCESS code that is later acknowledged.

	   It is upto the protocol if it processes this message with the protocol
	   send chain.

PROTO: sends a UI_TEXTED with HPROCESS code given above
UI   : the UI may show the message as 'sent' or show nothing to the user.

UI   : sends UI_IAMTYPING
PROTO: the protocol may or may not send this message to the other parties
       but it must process it, this message is also optional.

PROTO: sends UI_CONTACT_ISTYPING (source TMCONTACT)
UI   : an TMCONTACT within the session is typing, the UI may elect to show this
       message in a status bar, or with a visual effect.	    	
}

{$ENDIF}
