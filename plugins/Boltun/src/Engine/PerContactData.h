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

#ifndef PerContactDataH
#define PerContactDataH

#include "CriticalSection.h"

static std::map<unsigned, void*> perContactDataObjects;

template <class Source, class Data, class ContactHandle>
class PerContactData
{
	template <class Source, class Data>
	struct InternalData
	{
		CriticalSection lock;
		Data *data;
		time_t time;
		inline InternalData(const Source& src)
			:time(0)
		{
			data = new Data(src);
		}

		inline InternalData()
			: data(NULL)
		{
			assert(false);
		}

		inline ~InternalData()
		{
			delete data;
		}
	};
	CriticalSection mapLock;
	unsigned timerID;
	std::map<ContactHandle, InternalData<Source, Data>* > datas;
	typedef typename std::map<ContactHandle, InternalData<Source, Data>* >::iterator mapIt;
	const Source& source;
	void CleanupData();
	template <class Source, class Data, class ContactHandle>
	friend VOID CALLBACK RunTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
public:
	PerContactData(const Source& src);
	~PerContactData();
	Data* GetData(ContactHandle Contact);
	void PutData(ContactHandle Contact);
};

template <class Source, class Data, class ContactHandle>
PerContactData<Source, Data, ContactHandle>::PerContactData(const Source& src)
	:source(src), timerID(0)
{
}

template <class Source, class Data, class ContactHandle>
PerContactData<Source, Data, ContactHandle>::~PerContactData()
{
	mapLock.Enter();
	if (timerID)
	{
		KillTimer(NULL, timerID);
		perContactDataObjects.erase(timerID);
	}
	while (!datas.empty())
	{
		while (!(*datas.begin()).second->lock.TryEnter())
		{
			mapLock.Leave();
			Sleep(10);
			mapLock.Enter();
		}
		//Now we know exactly that no-one onws a contact lock
		InternalData<Source, Data>* data = (*datas.begin()).second;
		data->lock.Leave();
		delete data;
		datas.erase(datas.begin());
	}
	mapLock.Leave();
}

template <class Source, class Data, class ContactHandle>
Data* PerContactData<Source, Data, ContactHandle>::GetData(ContactHandle Contact)
{
	mapLock.Enter();
	mapIt it;
	if ((it = datas.find(Contact)) == datas.end())
		it = datas.insert(make_pair(Contact, new InternalData<Source, Data>(source))).first;
	(*it).second->lock.Enter();
	(*it).second->time = 0;
	Data* data = (*it).second->data;
	mapLock.Leave();
	return data;
}

template <class Source, class Data, class ContactHandle>
void PerContactData<Source, Data, ContactHandle>::PutData(ContactHandle Contact)
{
	mapLock.Enter();
	datas[Contact]->lock.Leave();
	::time(&(datas[Contact]->time));
	if (!timerID)
	{
		timerID = SetTimer(NULL, 0, 30000, RunTimerProc<Source, Data, ContactHandle>);
		assert(timerID);
		perContactDataObjects[timerID] = this;
	}
	mapLock.Leave();
}

template <class Source, class Data, class ContactHandle>
void PerContactData<Source, Data, ContactHandle>::CleanupData()
{
	mapLock.Enter();
	time_t now;
	time(&now);
	for (mapIt it = datas.begin(); it != datas.end();)
	{
		if ((*it).second->time) //it's being in use
		{
			int diff = (int)difftime(now, (*it).second->time);
			if (diff >= 30 * 60) //half of an hour
			{
				mapIt tmp = it;
				it++;
				delete (*tmp).second;
				datas.erase(tmp);
			}
			else
				it++;
		}
		else
			it++;
	}
	if (timerID && datas.empty()) //timerID may become NULL before locking, so should check
	{
		KillTimer(NULL, timerID);
		perContactDataObjects.erase(timerID);
	}
	mapLock.Leave();
}

template <class Source, class Data, class ContactHandle>
VOID CALLBACK RunTimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	PerContactData<Source, Data, ContactHandle>* val = (PerContactData<Source, Data, ContactHandle>*)perContactDataObjects[idEvent];
	val->CleanupData();
}

#endif /* PerContactDataH */