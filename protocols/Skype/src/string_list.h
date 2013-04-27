#pragma once

#include <m_system_cpp.h>

struct StringList : public LIST<wchar_t>
{
	static int compare(const wchar_t* p1, const wchar_t* p2) { return ::wcsicmp(p1, p2); }

	StringList() : LIST<wchar_t>(2, compare) {}
	StringList(const wchar_t* string, const wchar_t *delimeters = L" ") : LIST<wchar_t>(2, compare)
	{
		wchar_t *data = ::mir_wstrdup(string);
		if (data)
		{
			wchar_t *p = ::wcstok(data, delimeters);
			if (p)
			{
				this->insert(::mir_wstrdup(p));
				while (p = wcstok(NULL, delimeters))
				{
					this->insert(::mir_wstrdup(p));
				}
			}
			::mir_free(data);
		}
	}
	~StringList() { destroy(); }

	void destroy( void )
	{
		for (int i=0; i < count; i++)
			mir_free(items[i]);

		List_Destroy((SortedList*)this);
	}

	int insertn(const wchar_t* p) { return insert(::mir_wstrdup(p)); }

	int remove(int idx)
	{
		mir_free(items[idx]);
		return List_Remove((SortedList*)this, idx);
	}

	int remove(const wchar_t* p)
	{
		int idx;
		return  List_GetIndex((SortedList*)this, (wchar_t*)p, &idx) == 1 ? remove(idx) : -1;
	}

	bool contains(wchar_t* p) const
	{
		int idx;
		return List_GetIndex((SortedList*)this, (wchar_t*)p, &idx) == 1;
	}
};