/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

#ifdef LIBJSON_EXPORTS
#define LIBJSON_EXPORT __declspec(dllexport)
#else
#define LIBJSON_EXPORT __declspec(dllimport)
#endif

#define LIBJSON_DLL(T) LIBJSON_EXPORT T __stdcall

#ifdef __cplusplus

#include "../libs/libjson/src/JSONNode.h"

extern "C"
{
#else
typedef void JSONNode;
#endif

LIBJSON_DLL(void) json_free(void *str);
LIBJSON_DLL(void) json_delete(JSONNode *node);

LIBJSON_DLL(JSONNode*) json_parse(const char *json);
LIBJSON_DLL(wchar_t*) json_strip_white_space(const char *json);

#ifdef JSON_VALIDATE
	LIBJSON_DLL(JSONNode*) json_validate(const char *json);
#endif

/*
	stuff that's in class JSONNode
*/
//ctors
LIBJSON_DLL(JSONNode*) json_new_a(const char *name, const char *value);
LIBJSON_DLL(JSONNode*) json_new_i(const char *name, long value);
LIBJSON_DLL(JSONNode*) json_new_f(const char *name, double value);
LIBJSON_DLL(JSONNode*) json_new_b(const char *name, int value);  //because C bools are ints and C++ will implicitly cast it
LIBJSON_DLL(JSONNode*) json_new(char type);
LIBJSON_DLL(JSONNode*) json_copy(const JSONNode *orig);
LIBJSON_DLL(JSONNode*) json_duplicate(const JSONNode *orig);

//assignment
LIBJSON_DLL(void) json_set_a(JSONNode *node, const char *value);
LIBJSON_DLL(void) json_set_i(JSONNode *node, long value);
LIBJSON_DLL(void) json_set_f(JSONNode *node, double value);
LIBJSON_DLL(void) json_set_b(JSONNode *node, int value);  //because C bools are ints ane C++ will implicit
LIBJSON_DLL(void) json_set_n(JSONNode *node, const JSONNode *orig);

//inspectors
LIBJSON_DLL(char) json_type(const JSONNode *node);
LIBJSON_DLL(size_t) json_size(const JSONNode *node);
LIBJSON_DLL(int) json_empty(const JSONNode *node);
LIBJSON_DLL(const char*) json_name(const JSONNode *node);
#ifdef JSON_COMMENTS
	LIBJSON_DLL(char * json_get_comment(const JSONNode *node);
#endif
LIBJSON_DLL(wchar_t*) json_as_string(const JSONNode *node);
LIBJSON_DLL(long) json_as_int(const JSONNode *node);
LIBJSON_DLL(double) json_as_float(const JSONNode *node);
LIBJSON_DLL(int) json_as_bool(const JSONNode *node);
LIBJSON_DLL(JSONNode*) json_as_node(const JSONNode *node);
LIBJSON_DLL(JSONNode*) json_as_array(const JSONNode *node);

LIBJSON_DLL(wchar_t*) json_write(const JSONNode *node);
LIBJSON_DLL(wchar_t*) json_write_formatted(const JSONNode *node);

//modifiers
LIBJSON_DLL(void) json_set_name(JSONNode *node, const char *name);
#ifdef JSON_COMMENTS
	LIBJSON_DLL(void) json_set_comment(JSONNode *node, const char * comment);
#endif
LIBJSON_DLL(void) json_clear(JSONNode *node);
LIBJSON_DLL(void) json_nullify(JSONNode *node);
LIBJSON_DLL(void) json_swap(JSONNode *node, JSONNode *node2);
LIBJSON_DLL(void) json_merge(JSONNode *node, JSONNode *node2);
#ifndef JSON_PREPARSE
	LIBJSON_DLL(void) json_preparse(JSONNode *node);
#endif
#ifdef JSON_BINARY
	LIBJSON_DLL(void) json_set_binary(JSONNode *node, const void * data, unsigned long length);
#endif
LIBJSON_DLL(void) json_cast(JSONNode *node, char type);

//children access
LIBJSON_DLL(void) json_reserve(JSONNode *node, size_t siz);
LIBJSON_DLL(JSONNode*) json_at(JSONNode *node, size_t pos);
LIBJSON_DLL(JSONNode*) json_get(JSONNode *node, const char *name);
#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	LIBJSON_DLL(JSONNode*) json_get_nocase(JSONNode *node, const char *name);
	LIBJSON_DLL(JSONNode*) json_pop_back_nocase(JSONNode *node, const char *name);
#endif
LIBJSON_DLL(void) json_push_back(JSONNode *node, JSONNode *node2);
LIBJSON_DLL(JSONNode*) json_pop_back_at(JSONNode *node, size_t pos);
LIBJSON_DLL(JSONNode*) json_pop_back(JSONNode *node, const char *name);

//comparison
LIBJSON_DLL(int) json_equal(JSONNode *node, JSONNode *node2);

}

#ifdef __cplusplus

class JSONROOT
{
	JSONNode *m_node;

public:
	__forceinline JSONROOT() : m_node(nullptr) {}
	__forceinline JSONROOT(LPCSTR text) : m_node(json_parse(text)) {}
	__forceinline ~JSONROOT() { json_delete(m_node); }

	__forceinline operator JSONNode*() const { return m_node; }
};

struct NULL_PARAM : public PARAM
{
	__forceinline NULL_PARAM(LPCSTR _name) : PARAM(_name)
	{}
	;
};

struct JSON_PARAM : public PARAM
{
	JSONNode node;
	__forceinline JSON_PARAM(LPCSTR _name, JSONNode _node) :
		PARAM(_name), node(_node)
	{}
};

LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const INT_PARAM &param);
LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const INT64_PARAM &param);
LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const SINT64_PARAM &param);
LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const BOOL_PARAM &param);
LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const CHAR_PARAM &param);
LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const WCHAR_PARAM &param);
LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const NULL_PARAM &param);
LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const JSON_PARAM &param);

#endif // __cplusplus

#ifndef LIBJSON_EXPORTS
	#pragma comment(lib, "libjson.lib")
#endif

#endif // MIM_LIBJSON_H
