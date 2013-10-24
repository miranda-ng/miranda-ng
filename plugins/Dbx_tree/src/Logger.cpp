/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#include "Interface.h"
#include "Logger.h"
#include <process.h>

CLogger CLogger::_Instance;

CLogger::CLogger()
	:	m_Length(0),
		m_Level(logNOTICE)
{

}

CLogger::~CLogger()
{
	for (std::vector<TCHAR *>::iterator it = m_Messages.begin(); it != m_Messages.end(); ++it)
		delete [] *it;
}

void CLogger::Append(const TCHAR * File, const TCHAR * Function, const int Line, DWORD SysState, TLevel Level, const TCHAR * Message, ...)
{
	if (m_Level < Level)
		m_Level = Level;

	time_t rawtime = time(NULL);
	tm timeinfo;
	TCHAR timebuf[80];
	localtime_s(&timeinfo, &rawtime);
	size_t len = _tcsftime(timebuf, sizeof(timebuf) / sizeof(*timebuf), _T("%c"), &timeinfo);

	TCHAR msgbuf[4096];
	va_list va;
	va_start(va, Message);
	len += mir_vsntprintf(msgbuf, SIZEOF(msgbuf), Message, va);
	va_end(va);

	TCHAR * message;
	if (SysState)
	{
		TCHAR syserror[2048];
		len += FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, SysState, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), syserror, 2048, NULL);
		
		len += /*_tcslen(File) + 10 +*/ _tcslen(Function) + 32 + 12 + 1;

		message = new TCHAR[len];
		m_Length += mir_sntprintf(message, len, _T("[%s - %s]\n%s\n\nSystem Error Code: %d\n%s\n\n"), timebuf, /*File, Line, */ Function, msgbuf, SysState, syserror) + 1;
	} else {
		len += /*_tcslen(File) + 10 +*/ _tcslen(Function) + 12 + 1;

		message = new TCHAR[len];
		m_Length += mir_sntprintf(message, len, _T("[%s - %s]\n%s\n\n"), timebuf, /*File, Line, */Function, msgbuf) + 1;
	}
	m_Messages.push_back(message);
}

CLogger::TLevel CLogger::ShowMessage(TLevel CanAsyncTill)
{
	if (m_Messages.size() == 0)
		return logNOTICE;

	TCHAR * msg = new TCHAR[m_Length];
	*msg = 0;

	for (std::vector<TCHAR *>::iterator it = m_Messages.begin(); it != m_Messages.end(); ++it)
	{
		_tcscat_s(msg, m_Length, *it);
		delete [] *it;
	}
	m_Messages.clear();

	if (m_Level <= CanAsyncTill)
	{
		MSGBOXPARAMS * p = new MSGBOXPARAMS;
		p->cbSize = sizeof(*p);
		p->hwndOwner = 0;
		p->hInstance = NULL;
		p->lpszText = msg;
		p->lpszCaption = _T(__PLUGIN_NAME);
		p->dwStyle = MB_OK | (m_Level >= logERROR)?MB_ICONHAND:((m_Level == logWARNING)?MB_ICONWARNING:MB_ICONINFORMATION);
		p->lpszIcon = NULL;
		p->dwContextHelpId = 0;
		p->lpfnMsgBoxCallback = NULL;
		p->dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

		_beginthread(&CLogger::MessageBoxAsync, 0, p);
	} else {
		MessageBox(0, msg, _T(__PLUGIN_NAME), MB_OK | (m_Level >= logERROR)?MB_ICONHAND:((m_Level == logWARNING)?MB_ICONWARNING:MB_ICONINFORMATION));
		delete [] msg;
	}

	TLevel tmp = m_Level;
	m_Level = logNOTICE;
	return tmp;
}

void CLogger::MessageBoxAsync(void * MsgBoxParams)
{
	MSGBOXPARAMS* p = reinterpret_cast<MSGBOXPARAMS*>(MsgBoxParams);
	MessageBoxIndirect(p);
	delete [] p->lpszText;
	delete p;
}