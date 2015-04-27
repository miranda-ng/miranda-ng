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

#ifndef _CONFIG_H
#define _CONFIG_H

class BoltunConfig;

template <class T, class BaseClass>
class Property
{
public:
	typedef const T(__thiscall BaseClass::*Getter)();
	typedef const T(__thiscall BaseClass::*Setter)(const T);
private:
	const Getter getter;
	const Setter setter;
	BaseClass* owner;
	bool cacheValid;
	T cached;
public:
	Property(Getter g, Setter s)
		:getter(g), setter(s), cacheValid(false)
	{
	}

	void SetOwner(BaseClass* o)
	{
		owner = o;
	}

	inline operator const T()
	{
		if (cacheValid)
			return cached;
		cached = (owner->*getter)();
		cacheValid = true;
		return cached;
	}

	inline const T operator= (const T& value)
	{
		cacheValid = true;
		return (owner->*setter)(cached = value);
	}
};

template <class T, class BaseClass>
class PtrProperty
{
public:
	typedef const T* (__thiscall BaseClass::*Getter)();
	typedef const T* (__thiscall BaseClass::*Setter)(const T*);
private:
	const Getter getter;
	const Setter setter;
	BaseClass* owner;
	bool cacheValid;
	const T* cached;
public:
	PtrProperty(Getter g, Setter s)
		:getter(g), setter(s), cacheValid(false), cached(NULL)
	{
	}

	~PtrProperty()
	{
		delete cached;
	}

	void SetOwner(BaseClass* o)
	{
		owner = o;
	}

	inline operator const T*()
	{
		if (cacheValid)
			return cached;
		cached = (owner->*getter)();
		cacheValid = true;
		return cached;
	}

	inline const T* operator= (const T* value)
	{
		cacheValid = true;
		delete cached;
		cached = (owner->*setter)(value);
		return cached;
	}
};

class _BoltunConfigInit;

#define BUILDDEFETTERS(x, typ) \
	const typ Get##x(); \
	const typ Set##x(const typ value);

class BoltunConfig
{
	BUILDDEFETTERS(TalkWithEverybody, bool);
	BUILDDEFETTERS(TalkWithNotInList, bool);
	BUILDDEFETTERS(TalkWarnContacts, bool);
	BUILDDEFETTERS(TalkEveryoneWhileAway, bool);
	BUILDDEFETTERS(MarkAsRead, bool);
	BUILDDEFETTERS(AnswerPauseTime, int);
	BUILDDEFETTERS(AnswerThinkTime, int);
	BUILDDEFETTERS(PauseDepends, bool);
	BUILDDEFETTERS(PauseRandom, bool);
	BUILDDEFETTERS(WarnText, TCHAR*);
	BUILDDEFETTERS(MindFileName, TCHAR*);
	BUILDDEFETTERS(EngineStaySilent, bool);
	BUILDDEFETTERS(EngineMakeLowerCase, bool);
	BUILDDEFETTERS(EngineUnderstandAlways, bool);
	BoltunConfig();
	~BoltunConfig();
	friend class _BoltunConfigInit;
public:
	Property<bool, BoltunConfig> TalkWithEverybody;
	Property<bool, BoltunConfig> TalkWithNotInList;
	Property<bool, BoltunConfig> TalkWarnContacts;
	Property<bool, BoltunConfig> TalkEveryoneWhileAway;
	Property<bool, BoltunConfig> MarkAsRead;
	Property<int, BoltunConfig> AnswerPauseTime;
	Property<int, BoltunConfig> AnswerThinkTime;
	Property<bool, BoltunConfig> PauseDepends;
	Property<bool, BoltunConfig> PauseRandom;
	PtrProperty<TCHAR, BoltunConfig> WarnText;
	PtrProperty<TCHAR, BoltunConfig> MindFileName;
	Property<bool, BoltunConfig> EngineStaySilent;
	Property<bool, BoltunConfig> EngineMakeLowerCase;
	Property<bool, BoltunConfig> EngineUnderstandAlways;
};

extern BoltunConfig &Config;

#endif /* _CONFIG_H */