/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-13 Miranda NG project,
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

#include "commonheaders.h"

#include <m_protomod.h>

static HANDLE hAckEvent;

void InitProtocols()
{
	hAckEvent = CreateHookableEvent(ME_PROTO_ACK);
}

void UninitProtocols()
{
	if (hAckEvent) {
		DestroyHookableEvent(hAckEvent);
		hAckEvent = NULL;
	}
}

MIR_CORE_DLL(INT_PTR) ProtoBroadcastAck(const char *szModule, HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	if (type == ACKTYPE_AVATAR && hProcess) {
		PROTO_AVATAR_INFORMATION* ai = (PROTO_AVATAR_INFORMATION*)hProcess;
		if (ai->cbSize == sizeof(PROTO_AVATAR_INFORMATION)) {
			PROTO_AVATAR_INFORMATIONW aiw = { sizeof(aiw), ai->hContact, ai->format };
			MultiByteToWideChar(CP_ACP, 0, ai->filename, -1, aiw.filename, SIZEOF(aiw.filename));

			hProcess = &aiw;
		}
	}

	ACKDATA ack = { sizeof(ACKDATA), szModule, hContact, type, result, hProcess, lParam };
	return NotifyEventHooks(hAckEvent, 0, (LPARAM)&ack);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(INT_PTR) ProtoCallService(const char *szModule, const char *szService, WPARAM wParam, LPARAM lParam)
{
	if (szModule == NULL || szService == NULL)
		return false;

	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str,szModule,strlen(szModule));
	strncat_s(str,szService,strlen(szService));
	return CallService(str, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) ProtoServiceExists(const char *szModule, const char *szService)
{
	if (szModule == NULL || szService == NULL)
		return false;

	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str,szModule,strlen(szModule));
	strncat_s(str,szService,strlen(szService));
	return ServiceExists(str);
}
