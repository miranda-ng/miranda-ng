/* 
Copyright (C) 2007-2009 Ricardo Pescuma Domenecci

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


#ifndef __M_VOICESERVICE_H__
# define __M_VOICESERVICE_H__


/*
This services are a mirror of the services/notifications in m_voice.h,
with the difference that that ones are to be used by protocols, and this ones
are to be used by plugins that can make calls to contacts in multiple protocols.

To get the devices for input/output and some options, query the db directly:
 VoiceService/EchoCancelation	BYTE		default: TRUE
 VoiceService/MicBoost			BYTE		default: TRUE
 VoiceService/Input				TString		default: windows default
 VoiceService/Output			TString		default: windows default
*/


struct VOICE_MODULE
{
	int cbSize;           // sizeof(VOICE_MODULE)
	TCHAR *description;   // The description of the voice provider. This is the name that will be shown to the user
	const char *name;     // The internal name of the voice provider. All PS_* serivces
		                   // defined in m_voice.h need to be created based in this name. For example, 
                         // PS_VOICE_CALL (/Voice/Call) need to be created as <name>/Voice/Call
	HANDLE icon;          // Icon to identify provider (from icolib)
	int flags;            // or of VOICE_CAPS_*. You don't need to send VOICE_CAPS_VOICE.
};
/*
Register a new plugin that can make/receive voice calls (a voice provider).

wParam: const VOICE_MODULE *
lParam: ignored
return: 0 on success
*/
#define MS_VOICESERVICE_REGISTER		"VoiceService/Register"

/*
Unregister a plugin that can make/receive voice calls (a voice provider).

wParam: (const char *) Provider name
lParam: ignored
return: 0 on success
*/
#define MS_VOICESERVICE_UNREGISTER		"VoiceService/Unregister"

/*
Request a voice call to hContact.

wParam: (HANDLE) hContact
lParam: ignored
return: the number of option calls for a contact. If > 0, it can be called
*/
#define MS_VOICESERVICE_CAN_CALL			"VoiceService/CanCall"

/*
Request a voice call to hContact.

wParam: (HANDLE) hContact
lParam: (char *) Voice provider or NULL to use any provider avaiable
return: 0 on success
*/
#define MS_VOICESERVICE_CALL				"VoiceService/Call"



#endif // __M_VOICESERVICE_H__
