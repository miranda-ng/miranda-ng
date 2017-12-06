/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define ADDALIAS		L"alias"

int isValidTokenChar(wchar_t tc);

struct TArgList : public LIST<wchar_t>
{
	TArgList() :
		LIST<wchar_t>(1)
	{}

	void destroy() 
	{
		for (int i = 0; i < count; i++)
			mir_free(items[i]);
		count = 0;
	}
};

wchar_t* getArguments(wchar_t *string, TArgList &aargv);
