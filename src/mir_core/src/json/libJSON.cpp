/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "..\commonheaders.h"

#include <m_json.h>

/*
	This is the implementation of the C interface to libJSON
	This file may be included in any C++ application, but it will
	be completely ignored if JSON_LIBRARY isn't defined.  The
	only reason JSON_LIBRARY should be defined is when compiling libJSON
	as a library
*/

#include "JSONNode.h"
#include "JSONNode.inl"

#include "JSONWorker.h"

#ifdef JSON_MEMORY_MANAGE
	auto_expand StringHandler;
	auto_expand_node NodeHandler;
	#define MANAGER_INSERT(x) NodeHandler.insert(x)
#else
	#define MANAGER_INSERT(x) x
#endif

#ifdef JSON_SAFE
	static const json_char * EMPTY_CSTRING = JSON_TEXT("");
#endif

extern JSONNode nullNode;

inline TCHAR* toCString(const json_string & str)
{
	return mir_utf8decodeT( str.c_str());
}

/*
	stuff that's in namespace libJSON
*/

MIR_CORE_DLL(void) json_free(void *str){
	JSON_ASSERT_SAFE(str, JSON_TEXT("freeing null ptr"), return;);
	#ifdef JSON_MEMORY_MANAGE
		StringHandler.remove(str);
	#endif
	libjson_free<void>(str);
}

MIR_CORE_DLL(void) json_delete(JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("deleting null ptr"), return;);
	#ifdef JSON_MEMORY_MANAGE
		NodeHandler.remove(node);
	#endif
	JSONNode::deleteJSONNode((JSONNode *)node);
}

#ifdef JSON_MEMORY_MANAGE
	MIR_CORE_DLL(void) json_free_all(void){
		StringHandler.clear();
	}

	MIR_CORE_DLL(void) json_delete_all(void){
		NodeHandler.clear();
	}
#endif

JSONNode JSONNode::parse(const json_char *str)
{
	return JSONWorker::parse(str);
}

MIR_CORE_DLL(JSONNODE*) json_parse(const json_char *json){
	JSON_ASSERT_SAFE(json, JSON_TEXT("null ptr to json_parse"), return 0;);
	try {
		//use this constructor to simply copy reference instead of copying the temp
		return MANAGER_INSERT(JSONNode::newJSONNode_Shallow(JSONWorker::parse(json)));
	} catch (std::invalid_argument){}
	return 0;
}

MIR_CORE_DLL(TCHAR*) json_strip_white_space(const json_char *json){
	JSON_ASSERT_SAFE(json, JSON_TEXT("null ptr to json_strip_white_space"), return 0;);
	return toCString(JSONWorker::RemoveWhiteSpaceAndComments(json));
}

#ifdef JSON_VALIDATE
	MIR_CORE_DLL(JSONNODE*) json_validate(const json_char *json){
		JSON_ASSERT_SAFE(json, JSON_TEXT("null ptr to json_validate"), return 0;);
		try {
			//use this constructor to simply copy reference instead of copying the temp
			return MANAGER_INSERT(JSONNode::newJSONNode_Shallow(JSONWorker::validate(json)));
		} catch (std::invalid_argument){}
		return 0;
	}
#endif

#if defined JSON_DEBUG && !defined JSON_STDERROR
	//When libjson errors, a callback allows the user to know what went wrong
	MIR_CORE_DLL(void) json_register_debug_callback(json_error_callback_t callback){
		JSONDebug::register_callback(callback);
	}
#endif

#ifdef JSON_MUTEX_CALLBACKS
	#ifdef JSON_MUTEX_MANAGE
		MIR_CORE_DLL(void) json_register_mutex_callbacks(json_mutex_callback_t lock, json_mutex_callback_t unlock, json_mutex_callback_t destroy, void * manager_lock){
			JSONNode::register_mutex_callbacks(lock, unlock, manager_lock);
			JSONNode::register_mutex_destructor(destroy);
		}

	#else
		MIR_CORE_DLL(void) json_register_mutex_callbacks(json_mutex_callback_t lock, json_mutex_callback_t unlock, void * manager_lock){
			JSONNode::register_mutex_callbacks(lock, unlock, manager_lock);
		}
	#endif

	MIR_CORE_DLL(void) json_set_global_mutex(void * mutex){
		JSONNode::set_global_mutex(mutex);
	}

	MIR_CORE_DLL(void) json_set_mutex(JSONNODE *node, void * mutex){
		JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_mutex"), return;);
		((JSONNode*)node) -> set_mutex(mutex);
	}

	MIR_CORE_DLL(void) json_lock(JSONNODE *node, int threadid){
		JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_lock"), return;);
		((JSONNode*)node) -> lock(threadid);
	}

	MIR_CORE_DLL(void) json_unlock(JSONNODE *node, int threadid){
		JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_unlock"), return;);
		((JSONNode*)node) -> unlock(threadid);
	}
#endif

/*
stuff that's in class JSONNode
*/
//ctors
MIR_CORE_DLL(JSONNODE*) json_new_a(const json_char *name, const json_char *value){
	JSON_ASSERT_SAFE(name, JSON_TEXT("null name to json_new_a"), name = EMPTY_CSTRING;);
	JSON_ASSERT_SAFE(value, JSON_TEXT("null value to json_new_a"), value = EMPTY_CSTRING;);
	#ifdef JSON_MEMORY_CALLBACKS
		return MANAGER_INSERT(new(json_malloc<JSONNode>(1)) JSONNode(name, json_string(value)));
	#else
		return MANAGER_INSERT(new JSONNode(name, json_string(value)));
	#endif
}

MIR_CORE_DLL(JSONNODE*) json_new_i(const json_char *name, long value){
	JSON_ASSERT_SAFE(name, JSON_TEXT("null name to json_new_i"), name = EMPTY_CSTRING;);
	#ifdef JSON_MEMORY_CALLBACKS
		return MANAGER_INSERT(new(json_malloc<JSONNode>(1)) JSONNode(name, value));
	#else
		return MANAGER_INSERT(new JSONNode(name, value));
	#endif
}

MIR_CORE_DLL(JSONNODE*) json_new_f(const json_char *name, double value){
	JSON_ASSERT_SAFE(name, JSON_TEXT("null name to json_new_f"), name = EMPTY_CSTRING;);
	#ifdef JSON_MEMORY_CALLBACKS
		return MANAGER_INSERT(new(json_malloc<JSONNode>(1)) JSONNode(name, value));
	#else
		return MANAGER_INSERT(new JSONNode(name, value));
	#endif
}

MIR_CORE_DLL(JSONNODE*) json_new_b(const json_char *name, int value){
	JSON_ASSERT_SAFE(name, JSON_TEXT("null name to json_new_b"), name = EMPTY_CSTRING;);
	#ifdef JSON_MEMORY_CALLBACKS
		return MANAGER_INSERT(new(json_malloc<JSONNode>(1)) JSONNode(name, value != 0 ));
	#else
		return MANAGER_INSERT(new JSONNode(name, value != 0));
	#endif
}

MIR_CORE_DLL(JSONNODE*) json_new(char type){
	#ifdef JSON_MEMORY_CALLBACKS
		return MANAGER_INSERT(new(json_malloc<JSONNode>(1)) JSONNode(type));
	#else
		return MANAGER_INSERT(new JSONNode(type));
	#endif
}

MIR_CORE_DLL(JSONNODE*) json_copy(const JSONNODE *orig){
	JSON_ASSERT_SAFE(orig, JSON_TEXT("null orig to json_copy"), return 0;);
	#ifdef JSON_MEMORY_CALLBACKS
		return MANAGER_INSERT(new(json_malloc<JSONNode>(1)) JSONNode(*((JSONNode*)orig)));
	#else
		return MANAGER_INSERT(new JSONNode(*((JSONNode*)orig)));
	#endif
}

MIR_CORE_DLL(JSONNODE*) json_duplicate(const JSONNODE *orig){
	JSON_ASSERT_SAFE(orig, JSON_TEXT("null orig to json_duplicate"), return 0;);
	return MANAGER_INSERT(JSONNode::newJSONNode_Shallow(((JSONNode*)orig) -> duplicate()));
}

//assignment
MIR_CORE_DLL(void) json_set_a(JSONNODE *node, const json_char *value){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_a"), return;);
	JSON_ASSERT_SAFE(value, JSON_TEXT("null value to json_set_a"), value = EMPTY_CSTRING;);
	*((JSONNode*)node) = json_string(value);
}

MIR_CORE_DLL(void) json_set_i(JSONNODE *node, long value){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_i"), return;);
	*((JSONNode*)node) = value;
}

MIR_CORE_DLL(void) json_set_f(JSONNODE *node, double value){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_f"), return;);
	*((JSONNode*)node) = value;
}

MIR_CORE_DLL(void) json_set_b(JSONNODE *node, int value){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_b"), return;);
	*((JSONNode*)node) = value != 0;
}

MIR_CORE_DLL(void) json_set_n(JSONNODE *node, const JSONNODE *orig){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_n"), return;);
	JSON_ASSERT_SAFE(orig, JSON_TEXT("null node to json_set_n"), return;);
	*((JSONNode*)node) = *((JSONNode*)orig);
}

//inspectors
MIR_CORE_DLL(char) json_type(const JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_type"), return JSON_NULL;);
	return ((JSONNode*)node) -> type();
}

MIR_CORE_DLL(json_index_t) json_size(const JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_size"), return 0;);
	return ((JSONNode*)node) -> size();
}

MIR_CORE_DLL(int) json_empty(const JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_empty"), return true;);
	return (int)(((JSONNode*)node) -> empty());
}

MIR_CORE_DLL(const json_char*) json_name(const JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_name"), return EMPTY_CSTRING;);
	return ((JSONNode*)node) -> name();
}

#ifdef JSON_COMMENTS
	MIR_CORE_DLL(json_char*) json_get_comment(const JSONNODE *node){
		JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_get_comment"), return toCString(EMPTY_CSTRING););
		return toCString(((JSONNode*)node) -> get_comment());
	}
#endif

MIR_CORE_DLL(TCHAR*) json_as_string(const JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_string"), return toCString(EMPTY_CSTRING););
	return toCString(((JSONNode*)node) -> as_string());
}

MIR_C_CORE_DLL(std::string) json_as_pstring(const JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_string"), return EMPTY_CSTRING;);
	return ((JSONNode*)node) -> as_string();
}

MIR_CORE_DLL(long) json_as_int(const JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_int"), return 0;);
	return ((JSONNode*)node) -> as_int();
}

MIR_CORE_DLL(double) json_as_float(const JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_float"), return 0.0;);
	return ((JSONNode*)node) -> as_float();
}

MIR_CORE_DLL(int) json_as_bool(const JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_bool"), return false;);
	return (int)(((JSONNode*)node) -> as_bool());
}

MIR_CORE_DLL(JSONNODE*) json_as_node(const JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_node"), return 0;);
	return MANAGER_INSERT(JSONNode::newJSONNode_Shallow(((JSONNode*)node) -> as_node()));
}

MIR_CORE_DLL(JSONNODE*) json_as_array(const JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_array"), return 0;);
	return MANAGER_INSERT(JSONNode::newJSONNode_Shallow(((JSONNode*)node) -> as_array()));
}

#ifdef JSON_BINARY
	void * json_as_binary(const JSONNODE *node, unsigned long * size){
		JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_binary"), if (size){*size = 0;} return 0;);
		const std::string result = ((JSONNode*)node) -> as_binary();
		const size_t len = result.length();
		if (size) *size = len;
		#ifdef JSON_SAFE
			if (result.empty()) return 0;
		#endif
		#ifdef JSON_MEMORY_MANAGE
			return StringHandler.insert(memcpy(json_malloc<char>(len), result.data(), len));
		#else
			return memcpy(json_malloc<char>(len), result.data(), len);
		#endif
	}
#endif

#ifdef JSON_WRITER
	MIR_CORE_DLL(TCHAR*) json_write(const JSONNODE *node){
		JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_write"), return toCString(EMPTY_CSTRING););
		return toCString(((JSONNode*)node) -> write());
	}

	MIR_CORE_DLL(TCHAR*) json_write_formatted(const JSONNODE *node){
		JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_write_formatted"), return toCString(EMPTY_CSTRING););
		return toCString(((JSONNode*)node) -> write_formatted());
	}
#endif

//modifiers
MIR_CORE_DLL(void) json_set_name(JSONNODE *node, const json_char *name){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_name"), return;);
	JSON_ASSERT_SAFE(name, JSON_TEXT("null name to json_set_name"), name = EMPTY_CSTRING;);
	((JSONNode*)node) -> set_name(name);
}

#ifdef JSON_COMMENTS
	MIR_CORE_DLL(void) json_set_comment(JSONNODE *node, const json_char * comment){
		JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_comment"), return;);
		JSON_ASSERT_SAFE(comment, JSON_TEXT("null name to json_set_comment"), comment = EMPTY_CSTRING;);
		((JSONNode*)node) -> set_comment(comment);
	}
#endif

MIR_CORE_DLL(void) json_clear(JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_clear"), return;);
	((JSONNode*)node) -> clear();
}

MIR_CORE_DLL(void) json_nullify(JSONNODE *node){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_nullify"), return;);
	((JSONNode*)node) -> nullify();
}

MIR_CORE_DLL(void) json_swap(JSONNODE *node, JSONNODE *node2){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_swap"), return;);
	JSON_ASSERT_SAFE(node2, JSON_TEXT("null node to json_swap"), return;);
	((JSONNode*)node) -> swap(*(JSONNode*)node2);
}

MIR_CORE_DLL(void) json_merge(JSONNODE *node, JSONNODE *node2){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_merge"), return;);
	JSON_ASSERT_SAFE(node2, JSON_TEXT("null node to json_merge"), return;);
	((JSONNode*)node) -> merge(*(JSONNode*)node2);
}

#ifndef JSON_PREPARSE
	MIR_CORE_DLL(void) json_preparse(JSONNODE *node){
		JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_preparse"), return;);
		((JSONNode*)node) -> preparse();
	}
#endif

#ifdef JSON_BINARY
	MIR_CORE_DLL(void) json_set_binary(JSONNODE *node, const void * data, unsigned long length){
		JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_swap"), return;);
		JSON_ASSERT_SAFE(data, JSON_TEXT("null data to json_set_binary"), *((JSONNode*)node) = EMPTY_CSTRING; return;);
		((JSONNode*)node) -> set_binary((unsigned char *)data, length);
	}
#endif

MIR_CORE_DLL(void) json_cast(JSONNODE *node, char type){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_cast"), return;);
	((JSONNode*)node) -> cast(type);
}

//children access
MIR_CORE_DLL(void) json_reserve(JSONNODE *node, json_index_t siz){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_reserve"), return;);
	((JSONNode*)node) -> reserve(siz);
}

MIR_CORE_DLL(JSONNODE*) json_at(JSONNODE *node, json_index_t pos){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_at"), return 0;);
	JSONNode &res = ((JSONNode*)node) -> at(pos);
	return (&res == &nullNode) ? NULL : &res;
}

MIR_CORE_DLL(JSONNODE*) json_get(JSONNODE *node, const json_char *name){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_get"), return 0;);
	JSON_ASSERT_SAFE(name, JSON_TEXT("null node to json_get.  Did you mean to use json_at?"), return 0;);
	JSONNode &res = ((JSONNode*)node)->at(name);
	return (&res == &nullNode) ? NULL : &res;
}

#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	MIR_CORE_DLL(JSONNODE*) json_get_nocase(JSONNODE *node, const json_char *name){
		JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_at_nocase"), return 0;);
		JSON_ASSERT_SAFE(name, JSON_TEXT("null name to json_at_nocase"), return 0;);
		try {
			return &((JSONNode*)node) -> at_nocase(name);
		} catch (std::out_of_range){}
		return 0;
	}

	MIR_CORE_DLL(JSONNODE*) json_pop_back_nocase(JSONNODE *node, const json_char *name){
		JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_pop_back_nocase"), return 0;);
		JSON_ASSERT_SAFE(name, JSON_TEXT("null name to json_pop_back_nocase"), return 0;);
		return MANAGER_INSERT(((JSONNode*)node) -> pop_back_nocase(name));
	}
#endif

MIR_CORE_DLL(void) json_push_back(JSONNODE *node, JSONNODE *node2){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_push_back"), return;);
	JSON_ASSERT_SAFE(node2, JSON_TEXT("null node2 to json_push_back"), return;);
	#ifdef JSON_MEMORY_MANAGE
		NodeHandler.remove(node2);
	#endif
	((JSONNode*)node) -> push_back(*(JSONNode*)node2);
}

MIR_CORE_DLL(JSONNODE*) json_pop_back_at(JSONNODE *node, json_index_t pos){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_pop_back_i"), return 0;);
	return MANAGER_INSERT(&((JSONNode*)node) -> pop_back(pos));
}

MIR_CORE_DLL(JSONNODE*) json_pop_back(JSONNODE *node, const json_char *name){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_pop_back"), return 0;);
	JSON_ASSERT_SAFE(name, JSON_TEXT("null name to json_pop_back.  Did you mean to use json_pop_back_at?"), return 0;);
	return MANAGER_INSERT(&((JSONNode*)node) -> pop_back(name));
}

//comparison
MIR_CORE_DLL(int) json_equal(JSONNODE *node, JSONNODE *node2){
	JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_equal"), return false;);
	JSON_ASSERT_SAFE(node2, JSON_TEXT("null node2 to json_equal"), return false;);
	return (int)(*((JSONNode*)node) == *((JSONNode*)node2));
}
