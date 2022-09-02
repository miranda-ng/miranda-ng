/* 
Copyright (C) 2006-2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __M_VOICE_H__
# define __M_VOICE_H__


#define EVENTTYPE_VOICE_CALL 8739


#define VOICE_CAPS_VOICE			(1<<0)  // Voice is supported for this protocol. You need to set this one.
#define VOICE_CAPS_CALL_CONTACT		(1<<1)	// Set if a call can be made to a hContact (PS_VOICE_CALL_CONTACT_VALID is used to validate the string)
#define VOICE_CAPS_CALL_STRING		(1<<3)	// Set if a call can be made to some string (PS_VOICE_CALL_STRING_VALID is used to validate the string)
/*
Request to the protocol capabilities relative to voice.

wParam: 0
lParam: 0
return: VOICE_CAPS_*
*/
#define PS_VOICE_CAPS "/Voice/Caps"

#define VOICE_SECURE  0x00000001
#define VOICE_UNICODE 0x80000000

#define VOICE_STATE_TALKING 0
#define VOICE_STATE_RINGING 1
#define VOICE_STATE_CALLING 2
#define VOICE_STATE_ON_HOLD 3
#define VOICE_STATE_ENDED   4
#define VOICE_STATE_BUSY    5
#define VOICE_STATE_READY	6
#define VOICE_STATE_SETSTATUS	7

struct VOICE_CALL
{
	int cbSize;              // Struct size
	const char *moduleName;  // The name of the module (the same as VOICE_MODULE.name or the protocol szModule)
	const char *id;          // Protocol specific ID for this call
	int flags;               // VOICE_UNICODE to say the string is unicode or 0. VOICE_SECURE to say this is an encrypted call

	// Either contact or number must be != NULL
	// If both are != NULL the call will be made to the number and will be associated with the contact
	// This fields are only needed in first notification for a call id
	MCONTACT hContact;       // Contact associated with the call (can be NULL)
	MAllCStrings szNumber;   // Number to call (can be NULL)
	MAllCStrings szName;     // Name of the caller. This makes sense only on incoming calls,
		                      // where no contact is associated and the caller has a name and a number.

	int state;               // VOICE_STATE_*
	const wchar_t *szStatus;
};

/*
Notifies that a voice call changed state

wParam: const VOICE_CALL *
lParam: ignored
return: 0 on success
*/
#define PE_VOICE_CALL_STATE  "/Voice/State"

/*
Request the protocol to make a voice call

wParam: (HANDLE) hContact
lParam: (const TCHAR *) number
return: 0 on success
Or the contact or the number must be != NULL. If both are != NULL the call will be 
made to the number and will be associated with the contact.
*/
#define PS_VOICE_CALL  "/Voice/Call"

/*
Service called to make the protocol answer a call or restore a hold call.
It is an async call. If the call was answered, the PE_VOICE_CALL_STATE
notification will be fired.

wParam: (const char *) id
lParam: ignored
return: 0 on success
*/
#define PS_VOICE_ANSWERCALL  "/Voice/AnswerCall"

/*
Service called to make the protocol answer a call. This can be called if the 
call is ringing or has started. If called any other time it should be ignored.
It is an async call. If the call was droped, the PE_VOICE_CALL_STATE
notification will be fired.

wParam: (const char *) id
lParam: ignored
return: 0 on success
*/
#define PS_VOICE_DROPCALL  "/Voice/DropCall"

/*
Service called to make the protocol hold a call. This means that the call should not
be droped, but it should be muted and put in a hold, to allow other call to be answered.
If the protocol can't hold a cal, it should be droped.

This can be called if the call has started. If called any other time it should be ignored.
It is an async call. If the call was droped, the PE_VOICE_CALL_STATE
notification will be fired.

wParam: (const char *) id
lParam: ignored
return: 0 on success
*/
#define PS_VOICE_HOLDCALL  "/Voice/HoldCall"

/*
Send a DTMF (one digit text) to a talking call.

wParam: (const char *) id
lParam: (TCHAR) dtmf
return: 0 on success
*/
#define PS_VOICE_SEND_DTMF  "/Voice/SendDTMF"

/*
Used if protocol support VOICE_CALL_STRING. The call string is passed as
wParam and the proto should validate it. If this service does not exist all numbers can be called.

wParam: (const TCHAR *) call string
lParam: ignored
return: 0 if wrong, 1 if correct
*/
#define PS_VOICE_CALL_STRING_VALID "/Voice/CallStringValid"

/*
Used if protocol support VOICE_CALL_CONTACT. 
The hContact is passed as wParam and the proto should tell if this contact can be 
called. If this service does not exist all contacts can be called (or, if it is a protocol,
all contacts from the protocol can be called).

wParam: (HANDLE) hContact
lParam: (BOOL) TRUE if it is a test for 'can call now?', FALSE if is a test for 'will be possible to call someday?'
return: 0 if can't be called, 1 if can
*/
#define PS_VOICE_CALL_CONTACT_VALID	"/Voice/CallContactValid"

#endif // __M_VOICE_H__
