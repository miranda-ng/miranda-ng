/*
Copyright (C) 2006 Ricardo Pescuma Domenecci

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

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CListDblClick(WPARAM, LPARAM lParam)
{
	CLISTEVENT *ce = (CLISTEVENT *)lParam;
	ShowCallWindow((VoiceCall *)ce->lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR VoiceRegister(WPARAM wParam, LPARAM)
{
	VOICE_MODULE *in = (VOICE_MODULE *)wParam;
	if (in == NULL || in->cbSize < sizeof(VOICE_MODULE) || in->name == NULL || in->description == NULL)
		return -1;

	if (FindModule(in->name) != NULL)
		return -2;

	if (!ProtoServiceExists(in->name, PS_VOICE_CALL)
		|| !ProtoServiceExists(in->name, PS_VOICE_ANSWERCALL)
		|| !ProtoServiceExists(in->name, PS_VOICE_DROPCALL))
		return -3;

	modules.insert(new VoiceProvider(in->name, in->description, in->flags, in->icon));
	return 0;
}

INT_PTR VoiceUnregister(WPARAM wParam, LPARAM)
{
	char *moduleName = (char *)wParam;
	if (moduleName == NULL || moduleName[0] == 0)
		return -1;

	VoiceProvider *module = FindModule(moduleName);
	if (module == NULL)
		return -2;

	for (auto &call: calls.rev_iter())
		if (call->module == module) {
			call->Drop();
			call->SetState(VOICE_STATE_ENDED);
			calls.removeItem(&call);
		}

	modules.remove(module);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CreateServices()
{
	CreateServiceFunction(MS_VOICESERVICE_CLIST_DBLCLK, CListDblClick);
	CreateServiceFunction(MS_VOICESERVICE_REGISTER, VoiceRegister);
	CreateServiceFunction(MS_VOICESERVICE_UNREGISTER, VoiceUnregister);
}
