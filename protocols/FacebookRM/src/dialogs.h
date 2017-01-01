/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel

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

#pragma once

INT_PTR CALLBACK FBAccountProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK FBMindProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK FBOptionsProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK FBOptionsMessagingProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK FBOptionsEventsProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK FBOptionsStatusesProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

/////////////////////////////////////////////////////////////////////////////////

typedef CProtoDlgBase<FacebookProto> CFacebookDlgBase;

#define DIALOG_RESULT_OK 1

class CFacebookGuardDialog : public CFacebookDlgBase
{
private:
	const char *m_fb_dtsg;
	char m_code[7];

	CCtrlEdit m_text;
	CCtrlButton m_ok;
	CCtrlButton m_sms;

protected:
	void OnInitDialog();
	void OnOk(CCtrlButton*);
	void OnSms(CCtrlButton*);
	void OnClose();

public:
	CFacebookGuardDialog(FacebookProto *proto, const char *fb_dtsg);

	const char *GetCode();
};
