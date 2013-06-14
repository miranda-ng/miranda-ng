#pragma once

#include <algorithm>
#include <vector>

class StringList
{
private:
	std::vector<std::wstring> strings;

public:
	StringList() {}
	StringList(const wchar_t* string, const wchar_t *delimeters = L" ")
	{
		wchar_t *data = ::mir_wstrdup(string);
		if (data)
		{
			wchar_t *p = ::wcstok(data, delimeters);
			if (p)
			{
				this->strings.push_back(p);
				while (p = wcstok(NULL, delimeters))
				{
					this->strings.push_back(p);
				}
			}
			::mir_free(data);
		}
	}
	virtual ~StringList() { }

	__inline const wchar_t *operator[](size_t idx) const 
	{ 
		return (idx >= 0 && idx < this->size()) ? this->strings[idx].c_str() : NULL; 
	}

	__inline void insert(const wchar_t* p)
	{
		this->strings.push_back(::mir_wstrdup(p));
	}

	__inline bool contains(const wchar_t* p) const
	{
		return std::find(this->strings.begin(), this->strings.end(), p) != this->strings.end();
	}

	__inline size_t size() const
	{
		return this->strings.size();
	}

	__inline bool empty() const
	{
		return this->strings.empty();
	}

	__inline void clear()
	{
		this->strings.clear();
	}
};