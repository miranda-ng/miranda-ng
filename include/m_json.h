/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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

#define IsJSONServiceAvailable() (ServiceExists(MS_JSON_FREE))
#define json_getJSI(a) CallService(MS_JSON_GETINTERFACE,(WPARAM)a,0)

#ifndef LIBJSON_H
    #define JSONNODE void  //so that JSONNODE* is void*
    typedef JSONNODE** JSONNODE_ITERATOR;
#endif

// This function get the service interface for direct calling
// wParam=(WPARAM)(LPJSONSERVICEINTERFACE)
// lParam=0
// Return value ALWAYS 0

typedef struct {
		  /*
			 stuff that's in namespace libJSON
		  */
		  void (*free)(void * str);
		  void (*delete_)(JSONNODE * node);
		  // #ifdef JSON_MEMORY_MANAGE
			 void (*free_all)(void);
			 void (*delete_all)(void);
		  // #endif
		  JSONNODE * (*parse)(const char * json);
		  char * (*strip_white_space)(const char * json);
		  // #ifdef JSON_VALIDATE
			 JSONNODE * (*validate)(const char * json);
		  // #endif

		  /*
			 stuff that's in class JSONNode
		   */
		  //ctors
		  JSONNODE * (*new_a)(const char * name, const char * value);
		  JSONNODE * (*new_i)(const char * name, long value);
		  JSONNODE * (*new_f)(const char * name, double value);
		  JSONNODE * (*new_b)(const char * name, int value);  //because C bools are ints and C++ will implicitly cast it
		  JSONNODE * (*new_)(char type);
		  JSONNODE * (*copy)(const JSONNODE * orig);
		  JSONNODE * (*duplicate)(const JSONNODE * orig);

		  //assignment
		  void (*set_a)(JSONNODE * node, const char * value);
		  void (*set_i)(JSONNODE * node, long value);
		  void (*set_f)(JSONNODE * node, double value);
		  void (*set_b)(JSONNODE * node, int value);  //because C bools are ints ane C++ will implicit
		  void (*set_n)(JSONNODE * node, const JSONNODE * orig);

		  //inspectors
		  char (*type)(const JSONNODE * node);
		  unsigned int (*size)(const JSONNODE * node);
		  int (*empty)(const JSONNODE * node);
		  char * (*name)(const JSONNODE * node);
		  // #ifdef JSON_COMMENTS
			 char * (*get_comment)(const JSONNODE * node);
		  // #endif
		  char * (*as_string)(const JSONNODE * node);
		  long (*as_int)(const JSONNODE * node);
		  double (*as_float)(const JSONNODE * node);
		  int (*as_bool)(const JSONNODE * node);
		  JSONNODE * (*as_node)(const JSONNODE * node);
		  JSONNODE * (*as_array)(const JSONNODE * node);
		  // #ifdef JSON_BINARY
			 void * (*as_binary)(const JSONNODE * node, unsigned long * size);
		  // #endif
		  // #ifdef JSON_WRITER
			 char * (*write)(const JSONNODE * node);
			 char * (*write_formatted)(const JSONNODE * node);
		  // #endif

		  //modifiers
		  void (*set_name)(JSONNODE * node, const char * name);
		  // #ifdef JSON_COMMENTS
			 void (*set_comment)(JSONNODE * node, const char * comment);
		  // #endif
		  void (*clear)(JSONNODE * node);
		  void (*nullify)(JSONNODE * node);
		  void (*swap)(JSONNODE * node, JSONNODE * node2);
		  void (*merge)(JSONNODE * node, JSONNODE * node2);
		  // #ifndef JSON_PREPARSE
			 void (*preparse)(JSONNODE * node);
		  // #endif
		  // #ifdef JSON_BINARY
			 void (*set_binary)(JSONNODE * node, const void * data, unsigned long length);
		  // #endif
		  void (*cast)(JSONNODE * node, char type);

		  //children access
		  void (*reserve)(JSONNODE * node, unsigned int siz);
		  JSONNODE * (*at)(JSONNODE * node, unsigned int pos);
		  JSONNODE * (*get)(JSONNODE * node, const char * name);
		  // #ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
			 JSONNODE * (*get_nocase)(JSONNODE * node, const char * name);
			 JSONNODE * (*pop_back_nocase)(JSONNODE * node, const char * name);
		  // #endif
		  void (*push_back)(JSONNODE * node, JSONNODE * node2);
		  JSONNODE * (*pop_back_at)(JSONNODE * node, unsigned int pos);
		  JSONNODE * (*pop_back)(JSONNODE * node, const char * name);
		  // #ifdef JSON_ITERATORS
			 JSONNODE_ITERATOR (*find)(JSONNODE * node, const char * name);
			 // #ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
				JSONNODE_ITERATOR (*find_nocase)(JSONNODE * node, const char * name);
			 // #endif
			 JSONNODE_ITERATOR (*erase)(JSONNODE * node, JSONNODE_ITERATOR it);
			 JSONNODE_ITERATOR (*erase_multi)(JSONNODE * node, JSONNODE_ITERATOR start, JSONNODE_ITERATOR end);
			 JSONNODE_ITERATOR (*insert)(JSONNODE * node, JSONNODE_ITERATOR it, JSONNODE * node2);
			 JSONNODE_ITERATOR (*insert_multi)(JSONNODE * node, JSONNODE_ITERATOR it, JSONNODE_ITERATOR start, JSONNODE_ITERATOR end);

			 //iterator functions
			 JSONNODE_ITERATOR (*begin)(JSONNODE * node);
			 JSONNODE_ITERATOR (*end)(JSONNODE * node);
		  // #endif

		  //comparison
		  int (*equal)(JSONNODE * node, JSONNODE * node2);
} JSONSERVICEINTERFACE, *PJSONSERVICEINTERFACE, *LPJSONSERVICEINTERFACE;


#ifndef _STATIC
extern JSONSERVICEINTERFACE jsi;

#ifndef LIBJSON_H
#define json_free(a) jsi.free(a)
#define json_delete(a) jsi.delete_(a)
// #ifdef JSON_MEMORY_MANAGE
#define json_free_all() jsi.free_all()
#define json_delete_all() jsi.delete_all()
// #endif
#define json_parse(a) jsi.parse(a)
#define json_strip_white_space(a) jsi.strip_white_space(a)
// #ifdef JSON_VALIDATE
#define json_validate(a) jsi.validate(a)
// #endif
#define json_new_a(a,b) jsi.new_a(a,b)
#define json_new_i(a,b) jsi.new_i(a,b)
#define json_new_f(a,b) jsi.new_f(a,b)
#define json_new_b(a,b) jsi.new_b(a,b)
#define json_new(a) jsi.new_(a)
#define json_copy(a) jsi.copy(a)
#define json_duplicate(a) jsi.duplicate(a)
#define json_set_a(a,b) jsi.set_a(a,b)
#define json_set_i(a,b) jsi.set_i(a,b)
#define json_set_f(a,b) jsi.set_f(a,b)
#define json_set_b(a,b) jsi.set_b(a,b)
#define json_set_n(a,b) jsi.set_n(a,b)
#define json_type(a) jsi.type(a)
#define json_size(a) jsi.size(a)
#define json_empty(a) jsi.empty(a)
#define json_name(a) jsi.name(a)
// #ifdef JSON_COMMENTS
#define json_get_comment(a) jsi.get_comment(a)
// #endif
#define json_as_string(a) jsi.as_string(a)
#define json_as_int(a) jsi.as_int(a)
#define json_as_float(a) jsi.as_float(a)
#define json_as_bool(a) jsi.as_bool(a)
#define json_as_node(a) jsi.as_node(a)
#define json_as_array(a) jsi.as_array(a)
// #ifdef JSON_BINARY
#define json_as_binary(a,b) jsi.as_binary(a,b)
// #endif
// #ifdef JSON_WRITER
#define json_write(a) jsi.write(a)
#define json_write_formatted(a) jsi.write_formatted(a)
// #endif
#define json_set_name(a,b) jsi.set_name(a,b)
// #ifdef JSON_COMMENTS
#define json_set_comment(a,b) jsi.set_comment(a,b)
// #endif
#define json_clear(a) jsi.clear(a)
#define json_nullify(a) jsi.nullify(a)
#define json_swap(a,b) jsi.swap(a,b)
#define json_merge(a,b) jsi.merge(a,b)
// #ifndef JSON_PREPARSE
#define json_preparse(a) jsi.preparse(a)
// #endif
// #ifdef JSON_BINARY
#define json_set_binary(a,b,c) jsi.set_binary(a,b,c)
// #endif
#define json_cast(a,b) jsi.cast(a,b)
#define json_reserve(a,b) jsi.reserve(a,b)
#define json_at(a,b) jsi.at(a,b)
#define json_get(a,b) jsi.get(a,b)
// #ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
#define json_get_nocase(a,b) jsi.get_nocase(a,b)
#define json_pop_back_nocase(a,b) jsi.pop_back_nocase(a,b)
// #endif
#define json_push_back(a,b) jsi.push_back(a,b)
#define json_pop_back_at(a,b) jsi.pop_back_at(a,b)
#define json_pop_back(a,b) jsi.pop_back(a,b)
// #ifdef JSON_ITERATORS
#define json_find(a,b) jsi.find(a,b)
// #ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
#define json_find_nocase(a,b) jsi.find_nocase(a,b)
// #endif
#define json_erase(a,b) jsi.erase(a,b)
#define json_erase_multi(a,b,c) jsi.erase_multi(a,b,c)
#define json_insert(a,b,c) jsi.insert(a,b,c)
#define json_insert_multi(a,b,c,d) jsi.insert_multi(a,b,c,d)
#define json_begin(a) jsi.begin(a)
#define json_end(a) jsi.end(a)
#define json_equal(a,b) jsi.equal(a,b)
#endif
#endif // _STATIC

#define MS_JSON_GETINTERFACE "JSON/GetInterface"

#endif // MIM_LIBJSON_H
