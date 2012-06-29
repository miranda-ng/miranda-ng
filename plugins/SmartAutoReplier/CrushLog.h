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

/// crash item that is written in file...
typedef struct _CRUSHLOG_ITEM
{
	CHAR*	CompilationDate;		/// compilation date of the source file
	CHAR*	FileName;				/// file name where exception has occured
	CHAR*	FunctionName;			/// function name where exception has occured
	INT		LineNumber;				/// line number
	INT		LastError;				/// last error
}CRUSHLOG_ITEM, *PCRUSHLOG_ITEM;

#define LOG_FILENAME		TEXT("sarcrushes.log")

/// class that handles all crushes
/// occured between BEGIN_LOG_AND_PRO.. and END_LOG..
class CCrushLog
{
public:	/// ctors, dtors section...
	CCrushLog(void);	
	~CCrushLog(void);
public:
	static void Init(void);		/// inits internal data..
	static void DeInit(void);	/// deinits internal data..
	static void SaveToLog(CRUSHLOG_ITEM & item);	/// saves data to log file
	static void InternalProtect(CHAR*	FunctionName, CHAR* FileName, 
								CHAR*	CompilationDate, INT LineNumber,
								INT		LastError);
public:
	static TCHAR m_szLogPath[MAX_PATH];		/// path to log file
private:	
	static CRITICAL_SECTION m_critSect;		/// sync object...
};
