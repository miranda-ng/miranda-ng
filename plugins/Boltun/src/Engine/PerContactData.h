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

static std::map<unsigned, void*> perContactDataObjects;

template <class Source, class Data, class ContactHandle>
class PerContactData
{
	PerContactData& operator=(const PerContactData&);

	template <class Source, class Data>
	struct InternalData
	{
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
	mir_cs mapLock;
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
	mir_cslock mlck(mapLock);
	if (timerID)
	{
		KillTimer(NULL, timerID);
		perContactDataObjects.erase(timerID);
	}
	while (!datas.empty())
	{
		delete (*datas.begin()).second;
		datas.erase(datas.begin());
	}
}

template <class Source, class Data, class ContactHandle>
Data* PerContactData<Source, Data, ContactHandle>::GetData(ContactHandle Contact)
{
	mir_cslock mlck(mapLock);
	mapIt it;
	if ((it = datas.find(Contact)) == datas.end())
		it = datas.insert(make_pair(Contact, new InternalData<Source, Data>(source))).first;
	(*it).second->time = 0;
	return (*it).second->data;
}

template <class Source, class Data, class ContactHandle>
void PerContactData<Source, Data, ContactHandle>::PutData(ContactHandle Contact)
{
	mir_cslock mlck(mapLock);
	::time(&(datas[Contact]->time));
	if (!timerID)
	{
		timerID = SetTimer(NULL, 0, 30000, RunTimerProc<Source, Data, ContactHandle>);
		assert(timerID);
		perContactDataObjects[timerID] = this;
	}
}

template <class Source, class Data, class ContactHandle>
void PerContactData<Source, Data, ContactHandle>::CleanupData()
{
	mir_cslock mlck(mapLock);
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
}

template <class Source, class Data, class ContactHandle>
VOID CALLBACK RunTimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	PerContactData<Source, Data, ContactHandle>* val = (PerContactData<Source, Data, ContactHandle>*)perContactDataObjects[idEvent];
	val->CleanupData();
}

#endif /* PerContactDataH */