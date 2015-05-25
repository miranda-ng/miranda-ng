/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#ifndef MIM_LIBJSON_H
#define MIM_LIBJSON_H

#include <m_core.h>

#pragma warning(disable:4190)

#ifdef __cplusplus

#include "../src/mir_core/src/json/JSONNode.h"
typedef JSONNode JSONNODE;

extern "C"
{
#else
typedef void JSONNODE;
#endif

MIR_CORE_DLL(void) json_free(void *str);
MIR_CORE_DLL(void) json_delete(JSONNODE *node);

MIR_CORE_DLL(JSONNODE*) json_parse(const char *json);
MIR_CORE_DLL(TCHAR*) json_strip_white_space(const char *json);

#ifdef JSON_VALIDATE
	MIR_CORE_DLL(JSONNODE*) json_validate(const char *json);
#endif

/*
	stuff that's in class JSONNode
*/
//ctors
MIR_CORE_DLL(JSONNODE*) json_new_a(const char *name, const char *value);
MIR_CORE_DLL(JSONNODE*) json_new_i(const char *name, long value);
MIR_CORE_DLL(JSONNODE*) json_new_f(const char *name, double value);
MIR_CORE_DLL(JSONNODE*) json_new_b(const char *name, int value);  //because C bools are ints and C++ will implicitly cast it
MIR_CORE_DLL(JSONNODE*) json_new(char type);
MIR_CORE_DLL(JSONNODE*) json_copy(const JSONNODE *orig);
MIR_CORE_DLL(JSONNODE*) json_duplicate(const JSONNODE *orig);

//assignment
MIR_CORE_DLL(void) json_set_a(JSONNODE *node, const char *value);
MIR_CORE_DLL(void) json_set_i(JSONNODE *node, long value);
MIR_CORE_DLL(void) json_set_f(JSONNODE *node, double value);
MIR_CORE_DLL(void) json_set_b(JSONNODE *node, int value);  //because C bools are ints ane C++ will implicit
MIR_CORE_DLL(void) json_set_n(JSONNODE *node, const JSONNODE *orig);

//inspectors
MIR_CORE_DLL(char) json_type(const JSONNODE *node);
MIR_CORE_DLL(size_t) json_size(const JSONNODE *node);
MIR_CORE_DLL(int) json_empty(const JSONNODE *node);
MIR_CORE_DLL(const char*) json_name(const JSONNODE *node);
#ifdef JSON_COMMENTS
	MIR_CORE_DLL(char * json_get_comment(const JSONNODE *node);
#endif
MIR_CORE_DLL(TCHAR*) json_as_string(const JSONNODE *node);
MIR_CORE_DLL(long) json_as_int(const JSONNODE *node);
MIR_CORE_DLL(double) json_as_float(const JSONNODE *node);
MIR_CORE_DLL(int) json_as_bool(const JSONNODE *node);
MIR_CORE_DLL(JSONNODE*) json_as_node(const JSONNODE *node);
MIR_CORE_DLL(JSONNODE*) json_as_array(const JSONNODE *node);

MIR_CORE_DLL(TCHAR*) json_write(const JSONNODE *node);
MIR_CORE_DLL(TCHAR*) json_write_formatted(const JSONNODE *node);

//modifiers
MIR_CORE_DLL(void) json_set_name(JSONNODE *node, const char *name);
#ifdef JSON_COMMENTS
	MIR_CORE_DLL(void) json_set_comment(JSONNODE *node, const char * comment);
#endif
MIR_CORE_DLL(void) json_clear(JSONNODE *node);
MIR_CORE_DLL(void) json_nullify(JSONNODE *node);
MIR_CORE_DLL(void) json_swap(JSONNODE *node, JSONNODE *node2);
MIR_CORE_DLL(void) json_merge(JSONNODE *node, JSONNODE *node2);
#ifndef JSON_PREPARSE
	MIR_CORE_DLL(void) json_preparse(JSONNODE *node);
#endif
#ifdef JSON_BINARY
	MIR_CORE_DLL(void) json_set_binary(JSONNODE *node, const void * data, unsigned long length);
#endif
MIR_CORE_DLL(void) json_cast(JSONNODE *node, char type);

//children access
MIR_CORE_DLL(void) json_reserve(JSONNODE *node, size_t siz);
MIR_CORE_DLL(JSONNODE*) json_at(JSONNODE *node, size_t pos);
MIR_CORE_DLL(JSONNODE*) json_get(JSONNODE *node, const char *name);
#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	MIR_CORE_DLL(JSONNODE*) json_get_nocase(JSONNODE *node, const char *name);
	MIR_CORE_DLL(JSONNODE*) json_pop_back_nocase(JSONNODE *node, const char *name);
#endif
MIR_CORE_DLL(void) json_push_back(JSONNODE *node, JSONNODE *node2);
MIR_CORE_DLL(JSONNODE*) json_pop_back_at(JSONNODE *node, size_t pos);
MIR_CORE_DLL(JSONNODE*) json_pop_back(JSONNODE *node, const char *name);

//comparison
MIR_CORE_DLL(int) json_equal(JSONNODE *node, JSONNODE *node2);

#ifdef __cplusplus

class JSONROOT
{
	JSONNODE *m_node;

public:
	__forceinline JSONROOT() { m_node = NULL; }
	__forceinline JSONROOT(LPCSTR text) { Parse(text); }
	__forceinline ~JSONROOT() { json_delete(m_node); }

	__forceinline operator JSONNODE*() const { return m_node; }

	__forceinline JSONNODE* Parse(LPCSTR text) { return (m_node = json_parse(text)); }
};

}

#ifdef _XSTRING_
MIR_C_CORE_DLL(std::string) json_as_pstring(const JSONNODE *node);
#endif

#endif

#endif // MIM_LIBJSON_H
