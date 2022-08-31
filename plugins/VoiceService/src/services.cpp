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

static INT_PTR CALLBACK DlgProcNewCall(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	VoiceCall *call = (VoiceCall *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			call = (VoiceCall *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			wchar_t text[1024];

			VoiceCall *currentCall = GetTalkingCall();
			if (currentCall == NULL) {
				mir_snwprintf(text, TranslateT("%s wants to start a voice call with you. What you want to do?"),
					call->displayName);
			}
			else if (currentCall->CanHold()) {
				mir_snwprintf(text, TranslateT("%s wants to start a voice call with you. What you want to do?\n\nIf you answer the call, the current call will be put on hold."),
					call->displayName);
			}
			else {
				mir_snwprintf(text, TranslateT("%s wants to start a voice call with you. What you want to do?\n\nIf you answer the call, the current call will be dropped."),
					call->displayName);
			}

			SendMessage(GetDlgItem(hwndDlg, IDC_TEXT), WM_SETTEXT, 0, (LPARAM)text);

			HICON hIcon = g_plugin.getIcon(IDI_RINGING);
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			IcoLib_ReleaseIcon(hIcon);

			if (call->hContact == NULL)
				ShowWindow(GetDlgItem(hwndDlg, IDC_AUTO), SW_HIDE);
		}
		return TRUE;

	case WM_COMMAND:
		switch (wParam) {
		case ID_ANSWER:
			if (call->hContact != NULL && IsDlgButtonChecked(hwndDlg, IDC_AUTO))
				g_plugin.setWord(call->hContact, "AutoAccept", AUTO_ACCEPT);

			Answer(call);

			DestroyWindow(hwndDlg);
			break;

		case ID_DROP:
			if (call->hContact != NULL && IsDlgButtonChecked(hwndDlg, IDC_AUTO))
				g_plugin.setWord(call->hContact, "AutoAccept", AUTO_DROP);

			call->Drop();

			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_CLOSE:
		call->Notify(false, false, true);
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		call->SetNewCallHWND(NULL);
		break;
	}

	return FALSE;
}

static INT_PTR CListDblClick(WPARAM, LPARAM lParam)
{
	return 0;
	CLISTEVENT *ce = (CLISTEVENT *)lParam;

	VoiceCall *call = (VoiceCall *)ce->lParam;

	HWND hwnd = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_NEW_CALL), NULL, DlgProcNewCall, (LPARAM)call);

	ShowWindow(hwnd, SW_SHOWNORMAL);

	call->SetNewCallHWND(hwnd);
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

			calls.remove(calls.indexOf(&call));
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
