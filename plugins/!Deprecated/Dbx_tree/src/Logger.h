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

#pragma once

#include <vector>
#include <time.h>
#include "Interface.h"

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define __WFUNCTION__ WIDEN(__FUNCTION__)


#define LOG(Level, Message, ...) CLogger::Instance().Append(__WFILE__, __WFUNCTION__, __LINE__, 0, CLogger:: ## Level, Message, __VA_ARGS__)
#define LOGSYS(Level, Message, ...) CLogger::Instance().Append(__WFILE__, __WFUNCTION__, __LINE__, GetLastError(), CLogger:: ## Level, Message, __VA_ARGS__)


#define CHECK(Assertion, Level, Message, ...) if (!(Assertion)) LOG(Level, Message, __VA_ARGS__)
#define CHECKSYS(Assertion, Level, Message, ...) if (!(Assertion)) LOGSYS(Level, Message, __VA_ARGS__)

class CLogger
{
	public:
		enum TLevel
		{
			logNOTICE,
			logWARNING,
			logERROR,
			logCRITICAL
		};

		CLogger();
		~CLogger();
		
		void Append(const TCHAR * File, const TCHAR * Function, const int Line, DWORD SysState, TLevel Level, const TCHAR * Message, ...);
		TLevel ShowMessage(TLevel CanAsyncTill = logERROR);

		static CLogger & Instance()
			{	return _Instance; };

		TLevel Level()
			{	return m_Level; };
	protected:
		std::vector<TCHAR *> m_Messages;
		size_t m_Length;
		TLevel m_Level;

		static void MessageBoxAsync(void * MsgBoxParams);
	private:
		static CLogger _Instance;

};
