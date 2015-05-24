//***********************************************************
//	Copyright © 2003-2008 Alexander S. Kiselev, Valentin Pavlyuchenko
//
//	This file is part of Boltun.
//
//    Boltun is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    Boltun is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//	  along with Boltun. If not, see <http://www.gnu.org/licenses/>.
//
//***********************************************************

#include "stdafx.h"

//Database keys
#define DB_EVERYBODY "Everybody"
#define DB_NOT_IN_LIST "NotInList"
#define DB_AUTOAWAY "AutoAway"
#define DB_WARN "Warn"
#define DB_MARK_READ "MarkRead"
#define DB_WAIT_TIME "WaitTime"
#define DB_THINK_TIME "ThinkTime"
#define DB_PAUSE_DEPENDS "PauseDepends"
#define DB_PAUSE_RANDOM "PauseRandom"
#define DB_WARN_TEXT "WarnText"
#define DB_MIND_FILE_NAME "MindFileName"
#define DB_ENGINE_SILENT "SilentEngine"
#define DB_ENGINE_LOWERCASE "MakeLowerCase"
#define DB_ENGINE_UNDERSTAND_ALWAYS "UnderstandAlways"

inline TCHAR* GetString(char* key, const TCHAR* def)
{
	DBVARIANT dbv;
	TCHAR* val;
	if (!db_get_ts(NULL, BOLTUN_KEY, key, &dbv))
	{
		size_t len = mir_wstrlen(dbv.ptszVal) + 1;
		val = new TCHAR[len];
		_tcscpy_s(val, len, dbv.ptszVal);
		db_free(&dbv);
	}
	else
	{
		size_t len = mir_wstrlen(def) + 1;
		val = new TCHAR[len];
		_tcscpy_s(val, len, def);
	}
	return val;
}

inline const TCHAR* SetString(char* key, const TCHAR* value)
{
	size_t len = mir_tstrlen(value) + 1;
	TCHAR* val = new TCHAR[len];
	_tcscpy_s(val, len, value);
	db_set_ts(NULL, BOLTUN_KEY, key, val);
	return val;
}

#define BUILDETTERS(x, str, def) \
	const bool BoltunConfig::Get##x() { \
	return db_get_dw(NULL, BOLTUN_KEY, str, def) != 0; } \
	const bool BoltunConfig::Set##x(const bool value) { \
	db_set_dw(NULL, BOLTUN_KEY, str, value); \
	return value; }

#define BUILDINTETTERS(x, str, def) \
	const int BoltunConfig::Get##x() { \
	return db_get_dw(NULL, BOLTUN_KEY, str, def); } \
	const int BoltunConfig::Set##x(const int value) { \
	db_set_dw(NULL, BOLTUN_KEY, str, value); \
	return value; }

#define BUILDSTRETTERS(x, str, def) \
	const TCHAR* BoltunConfig::Get##x() { \
	return GetString(str, def); } \
	const TCHAR* BoltunConfig::Set##x(const TCHAR* value) { \
	return SetString(str, value); }

#define BUILDINIT(x) \
	x(&BoltunConfig::Get##x, &BoltunConfig::Set##x)

BUILDETTERS(TalkWithNotInList, DB_NOT_IN_LIST, false);
BUILDETTERS(TalkWithEverybody, DB_EVERYBODY, false);
BUILDETTERS(TalkWarnContacts, DB_WARN, false);
BUILDETTERS(TalkEveryoneWhileAway, DB_AUTOAWAY, false);
BUILDETTERS(MarkAsRead, DB_MARK_READ, true);
BUILDINTETTERS(AnswerPauseTime, DB_WAIT_TIME, 2);
BUILDINTETTERS(AnswerThinkTime, DB_THINK_TIME, 4);
BUILDETTERS(PauseDepends, DB_PAUSE_DEPENDS, TRUE);
BUILDETTERS(PauseRandom, DB_PAUSE_RANDOM, TRUE);
BUILDSTRETTERS(WarnText, DB_WARN_TEXT, TranslateTS(DEFAULT_WARN_TEXT));
BUILDSTRETTERS(MindFileName, DB_MIND_FILE_NAME, DEFAULT_MIND_FILE);
BUILDETTERS(EngineStaySilent, DB_ENGINE_SILENT, FALSE);
BUILDETTERS(EngineMakeLowerCase, DB_ENGINE_LOWERCASE, FALSE);
BUILDETTERS(EngineUnderstandAlways, DB_ENGINE_UNDERSTAND_ALWAYS, FALSE);

BoltunConfig::BoltunConfig()
	:BUILDINIT(TalkWithEverybody),
	BUILDINIT(TalkWithNotInList),
	BUILDINIT(TalkWarnContacts),
	BUILDINIT(TalkEveryoneWhileAway),
	BUILDINIT(MarkAsRead),
	BUILDINIT(AnswerPauseTime),
	BUILDINIT(AnswerThinkTime),
	BUILDINIT(PauseDepends),
	BUILDINIT(PauseRandom),
	BUILDINIT(WarnText),
	BUILDINIT(MindFileName),
	BUILDINIT(EngineStaySilent),
	BUILDINIT(EngineMakeLowerCase),
	BUILDINIT(EngineUnderstandAlways)
{
	TalkWithEverybody.SetOwner(this);
	TalkWithNotInList.SetOwner(this);
	TalkWarnContacts.SetOwner(this);
	TalkEveryoneWhileAway.SetOwner(this);
	MarkAsRead.SetOwner(this);
	AnswerPauseTime.SetOwner(this);
	AnswerThinkTime.SetOwner(this);
	PauseDepends.SetOwner(this);
	PauseRandom.SetOwner(this);
	WarnText.SetOwner(this);
	MindFileName.SetOwner(this);
	EngineStaySilent.SetOwner(this);
	EngineMakeLowerCase.SetOwner(this);
	EngineUnderstandAlways.SetOwner(this);
}

BoltunConfig::~BoltunConfig()
{
}

class _BoltunConfigInit
{
public:
	BoltunConfig cfg;
};

_BoltunConfigInit inst;

BoltunConfig &Config = inst.cfg;