//***********************************************************
//	Copyright © 2008 Valentin Pavlyuchenko
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

#ifndef CriticalSectionH
#define CriticalSectionH

class CriticalSection
{
	CRITICAL_SECTION csQueue;
public:
	inline CriticalSection()
	{
		InitializeCriticalSection(&csQueue);
	}

	inline ~CriticalSection()
	{
		DeleteCriticalSection(&csQueue);
	}

	inline void Enter()
	{
		EnterCriticalSection(&csQueue);
	}

	inline void Leave()
	{
		LeaveCriticalSection(&csQueue);
	}

	inline bool TryEnter()
	{
		return TryEnterCriticalSection(&csQueue) != 0;
	}
};

#endif /* CriticalSectionH */