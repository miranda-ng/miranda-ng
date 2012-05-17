/* 
Copyright (C) 2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/
#ifndef __DBUTILS_H__
# define __DBUTILS_H__


class DBTString
{
	DBVARIANT dbv;
	bool exists;

public:
	DBTString(HANDLE hContact, char *module, char *setting)
	{
		ZeroMemory(&dbv, sizeof(dbv));
		exists = (DBGetContactSettingTString(hContact, module, setting, &dbv) == 0);
	}

	~DBTString()
	{
		if (exists)
			DBFreeVariant(&dbv);
	}

	const TCHAR * get() const
	{
		if (!exists)
			return NULL;
		else
			return dbv.ptszVal;
	}

	const bool operator==(const TCHAR *other)
	{
		return get() == other;
	}

	const bool operator!=(const TCHAR *other)
	{
		return get() != other;
	}

	operator const TCHAR *() const
	{
		return get();
	}
};






#endif // __DBUTILS_H__
