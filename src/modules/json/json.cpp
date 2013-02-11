/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-12 Miranda IM, 2012-13 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "..\..\core\commonheaders.h"
#include <m_json.h>

#include "libJSON.h"

///////////////////////////////////////////////////////////////////////////////
// Interface function

INT_PTR GetInterface(WPARAM wParam, LPARAM lParam)
{
	LPJSONSERVICEINTERFACE lpJSI = (LPJSONSERVICEINTERFACE)wParam;
	memset(lpJSI,0,sizeof(JSONSERVICEINTERFACE));

#define SETJSI(x) lpJSI->##x = json_##x
	SETJSI(free);
	lpJSI->delete_ = json_delete;
#ifdef JSON_MEMORY_MANAGE
	SETJSI(free_all);
	SETJSI(delete_all);
#endif
	SETJSI(parse);
	SETJSI(strip_white_space);
#ifdef JSON_VALIDATE
	SETJSI(validate);
#endif
	SETJSI(new_a);
	SETJSI(new_i);
	SETJSI(new_f);
	SETJSI(new_b);
	lpJSI->new_ = json_new;
	SETJSI(copy);
	SETJSI(duplicate);
	SETJSI(set_a);
	SETJSI(set_i);
	SETJSI(set_f);
	SETJSI(set_b);
	SETJSI(set_n);
	SETJSI(type);
	SETJSI(size);
	SETJSI(empty);
	SETJSI(name);
#ifdef JSON_COMMENTS
	SETJSI(get_comment);
#endif
	SETJSI(as_string);
	SETJSI(as_int);
	SETJSI(as_float);
	SETJSI(as_bool);
	SETJSI(as_node);
	SETJSI(as_array);
	#ifdef JSON_BINARY
	SETJSI(as_binary);
	#endif
	#ifdef JSON_WRITER
	SETJSI(write);
	SETJSI(write_formatted);
	#endif
	SETJSI(set_name);
	#ifdef JSON_COMMENTS
	SETJSI(set_comment);
	#endif
	SETJSI(clear);
	SETJSI(nullify);
	SETJSI(swap);
	SETJSI(merge);
	#ifndef JSON_PREPARSE
	SETJSI(preparse);
	#endif
	#ifdef JSON_BINARY
	SETJSI(set_binary);
	#endif
	SETJSI(cast);

	//children access
	SETJSI(reserve);
	SETJSI(at);
	SETJSI(get);
	#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	SETJSI(get_nocase);
	SETJSI(pop_back_nocase);
	#endif
	SETJSI(push_back);
	SETJSI(pop_back_at);
	SETJSI(pop_back);
	#ifdef JSON_ITERATORS
	SETJSI(find);
	#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	SETJSI(find_nocase);
	#endif
	SETJSI(erase);
	SETJSI(erase_multi);
	SETJSI(insert);
	SETJSI(insert_multi);

	//iterator functions
	SETJSI(begin);
	SETJSI(end);
	#endif

	SETJSI(equal);

	return 0;
}

int InitJson()
{
	CreateServiceFunction(MS_JSON_GETINTERFACE,GetInterface);

	json_register_memory_callbacks((json_malloc_t)mir_alloc,(json_realloc_t)mir_realloc,(json_free_t)mir_free);
	return 0;
}
