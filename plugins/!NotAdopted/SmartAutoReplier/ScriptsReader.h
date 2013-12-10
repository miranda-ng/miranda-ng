/*
 *  Smart Auto Replier (SAR) - auto replier plugin for Miranda IM
 *
 *  Copyright (C) 2004 - 2012 by Volodymyr M. Shcherbyna <volodymyr@shcherbyna.com>
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

#include <list>

/// class that converts all scripts
/// into an actions or raw text

/// replacing commands
#define SETTINGS_SCRIPT_USERNAME			"%username%"
#define SETTINGS_SCRIPT_CURRDATA			"%currdate%"
#define SETTINGS_SCRIPT_MODEMESSAGE			"%modemessage%"
#define SETTINGS_SCRIPT_INCOMMINGMESSAGE	"%incomingmessage%"
#define SETTINGS_SCRIPT_OWNNICKNAME			"%ownnickname%"

/// actios commands

typedef struct _ACTION
{
	LPTSTR ActionCode;
}ACTION, *PACTION;

typedef std::list<ACTION> ACTIONS;

class CScriptsReader
{
public:
	CScriptsReader(void);
	~CScriptsReader(void);
public:
	ACTIONS ParceString(LPTSTR & strMessage);
	bool GetReturnMessage(LPTSTR lpContactName, LPTSTR & lpMsg, LPTSTR & lpIncomingMsg);
	static bool Replace(LPSTR & lpString, LPSTR strReplWhat, LPSTR & strReplWith);
private:
	void InternalInit(void);
private:
};