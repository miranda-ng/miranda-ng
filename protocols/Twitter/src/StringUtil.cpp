/*
Copyright © 2012-22 Miranda NG team
Copyright © 2009 Jim Porter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void Split(const CMStringA &str, OBJLIST<CMStringA> &out, char sep, bool includeEmpty)
{
	int start = 0;
	int end = 0;

	while (true) {
		if (end == str.GetLength() || str[end] == sep) {
			if (end > start || includeEmpty)
				out.insert(new CMStringA(str.Mid(start, end - start)));

			if (end == str.GetLength())
				break;

			++end;
			start = end;
		}
		else ++end;
	}
}

StringPairs ParseQueryString(const CMStringA &query)
{
	StringPairs ret;

	OBJLIST<CMStringA> queryParams(10);
	Split(query, queryParams, '&', false);

	for (auto &it : queryParams) {
		OBJLIST<CMStringA> paramElements(2);
		Split(*it, paramElements, '=', true);
		if (paramElements.getCount() == 2)
			ret[paramElements[0]] = paramElements[1];
	}
	return ret;
}

void htmlEntitiesDecode(CMStringA &context)
{
	context.Replace("&amp;", "&");
	context.Replace("&quot;", "\"");
	context.Replace("&lt;", "<");
	context.Replace("&gt;", ">");
}
