/*
 *  Smart Auto Replier (SAR) - auto replier plugin for Miranda IM
 *
 *  Copyright (C) 2004 - 2012 by Volodymyr M. Shcherbyna <volodymyr@shcherbyna.com>
 *
 *  This code is inspired by article of RichardS at http://www.codeproject.com/Articles/11508/Integrating-Lua-into-C
 *
 *      This file is part of SAR.
 *
 *  SAR is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SAR is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SAR.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "LuaBridge.h"
#include "LuaScript.h"

#include <map>
#include <string>

using namespace std;

class CSarLuaScript : public CLuaScript
{
public:
	CSarLuaScript(CLuaBridge & luaBridge);
	~CSarLuaScript(void);

private:
	int SendMessage(CLuaBridge & luaBridge);
	int GetMyStatus(CLuaBridge & luaBridge);
	int SetMyStatus(CLuaBridge & luaBridge);
	int Wait(CLuaBridge & luaBridge);
	int FindUser(CLuaBridge & luaBridge);
	int SetVariable(CLuaBridge & luaBridge);
	int GetVariable(CLuaBridge & luaBridge);

protected:
	int ScriptCalling(CLuaBridge & luaBridge, int nFncNumber);	

	void HandleReturns(CLuaBridge & luaBridge, const char *szFunc);

protected:
   int m_nFuncBaseIndex;
};

