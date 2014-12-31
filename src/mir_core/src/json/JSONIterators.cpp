/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (�) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "JSONNode.h"

#ifdef JSON_ITERATORS
	#ifdef JSON_REF_COUNT
		#define JSON_ASSERT_UNIQUE(x) JSON_ASSERT(internal -> refcount == 1, json_string(JSON_TEXT(x)) + JSON_TEXT(" in non single reference"))
	#else
		#define JSON_ASSERT_UNIQUE(x) (void)0
	#endif

	#ifdef JSON_MUTEX_CALLBACKS
		#define JSON_MUTEX_COPY2 ,internal -> mylock
	#else
		#define JSON_MUTEX_COPY2
	#endif

JSONNode::json_iterator JSONNode::find(const json_string & name_t){
	JSON_CHECK_INTERNAL();
	JSON_ASSERT(type() == JSON_NODE, JSON_TEXT("finding a non-iteratable node"));
	makeUniqueInternal();
	if (JSONNode ** res = internal -> at(name_t)) {
		return ptr_to_json_iterator(res);
	}
	return end();
}

#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	JSONNode::json_iterator JSONNode::find_nocase(const json_string & name_t){
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE, JSON_TEXT("finding a non-iteratable node"));
		makeUniqueInternal();
		if (JSONNode ** res = internal -> at_nocase(name_t)) {
			return ptr_to_json_iterator(res);
		}
		return end();
	}
#endif

JSONNode::json_iterator JSONNode::erase(json_iterator pos){
	JSON_CHECK_INTERNAL();
	JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("erasing a non-iteratable node"));
	JSON_ASSERT_UNIQUE("erase 1");
	JSON_ASSERT_SAFE(pos < end(), JSON_TEXT("erase out of range"), return end(););
	JSON_ASSERT_SAFE(pos >= begin(), JSON_TEXT("erase out of range"), return begin(););
	deleteJSONNode(*(json_iterator_ptr(pos)));
	internal -> Children.erase(json_iterator_ptr(pos));
	return (empty()) ? end() : pos;
}

JSONNode::json_iterator JSONNode::erase(json_iterator _start, const json_iterator & _end){
	if (_start == _end) return _start;
	JSON_CHECK_INTERNAL();
	JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("erasing a non-iteratable node"));
	JSON_ASSERT_UNIQUE("erase 3");
	JSON_ASSERT_SAFE(_start <= end(), JSON_TEXT("erase out of lo range"), return end(););
	JSON_ASSERT_SAFE(_end <= end(), JSON_TEXT("erase out of hi range"), return end(););
	JSON_ASSERT_SAFE(_start >= begin(), JSON_TEXT("erase out of lo range"), return begin(););
	JSON_ASSERT_SAFE(_end >= begin(), JSON_TEXT("erase out of hi range"), return begin(););
	for (JSONNode ** pos = json_iterator_ptr(_start); pos < json_iterator_ptr(_end); ++pos){
		deleteJSONNode(*pos);
	}

	internal -> Children.erase(json_iterator_ptr(_start), json_iterator_ptr(_end) - json_iterator_ptr(_start));
	return (empty()) ? end() : _start;
}

#ifdef JSON_LIBRARY
JSONNode::json_iterator JSONNode::insert(json_iterator pos, JSONNode * x){
#else
JSONNode::json_iterator JSONNode::insert(json_iterator pos, const JSONNode & x){
#endif
	JSON_CHECK_INTERNAL();
	JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("erasing a non-iteratable node"));
	JSON_ASSERT_UNIQUE("insert 1");
	if (json_iterator_ptr(pos) >= internal -> Children.end()) {
		internal -> push_back(x);
		return end() - 1;
	}
	JSON_ASSERT_SAFE(pos >= begin(), JSON_TEXT("insert out of lo range"), return begin(););
	#ifdef JSON_LIBRARY
		internal -> Children.insert(json_iterator_ptr(pos), x);
	#else
		internal -> Children.insert(json_iterator_ptr(pos), newJSONNode(x));
	#endif
	return pos;
}

JSONNode::json_iterator JSONNode::insertFFF(json_iterator pos, JSONNode ** const _start, JSONNode ** const _end){
	JSON_CHECK_INTERNAL();
	JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("erasing a non-iteratable node"));
	JSON_ASSERT_UNIQUE("insertFFF");
	JSON_ASSERT_SAFE(pos <= end(), JSON_TEXT("insert out of high range"), return end(););
	JSON_ASSERT_SAFE(pos >= begin(), JSON_TEXT("insert out of low range"), return begin(););
	const size_t num = _end - _start;
	json_auto<JSONNode *> mem(num);
	JSONNode ** runner = mem.ptr;
	for (JSONNode ** po = _start; po < _end; ++po){
		*runner++=newJSONNode(*(*po)  JSON_MUTEX_COPY2);
	}
	internal -> Children.insert(json_iterator_ptr(pos), mem.ptr, num);
	return pos;
}

#ifndef JSON_LIBRARY
	JSONNode::const_iterator JSONNode::find(const json_string & name_t) const {
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE, JSON_TEXT("finding a non-iteratable node"));
		if (JSONNode ** res = internal -> at(name_t)) {
			return JSONNode::const_iterator(res);
		}
		return JSONNode::const_iterator(internal -> end());
	}

	#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
		JSONNode::const_iterator JSONNode::find_nocase(const json_string & name_t) const {
			JSON_CHECK_INTERNAL();
			JSON_ASSERT(type() == JSON_NODE, JSON_TEXT("finding a non-iteratable node"));
			if (JSONNode ** res = internal -> at_nocase(name_t)) {
				return JSONNode::const_iterator(res);
			}
			return JSONNode::const_iterator(internal -> end());
		}
	#endif

	JSONNode::reverse_iterator JSONNode::erase(reverse_iterator pos){
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("erasing a non-iteratable node"));
		JSON_ASSERT_UNIQUE("erase 2");
		JSON_ASSERT_SAFE(pos < rend(), JSON_TEXT("erase out of range"), return rend(););
		JSON_ASSERT_SAFE(pos >= rbegin(), JSON_TEXT("erase out of range"), return rbegin(););
		deleteJSONNode(*(pos.it));
		internal -> Children.erase(pos.it);
		return (empty()) ? rend() : pos + 1;
	}

	JSONNode::reverse_iterator JSONNode::erase(reverse_iterator _start, const reverse_iterator & _end){
		if (_start == _end) return _start;
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("erasing a non-iteratable node"));
		JSON_ASSERT_UNIQUE("erase 4");
		JSON_ASSERT_SAFE(_start <= rend(), JSON_TEXT("erase out of lo range"), return rend(););
		JSON_ASSERT_SAFE(_end <= rend(), JSON_TEXT("erase out of hi range"), return rend(););
		JSON_ASSERT_SAFE(_start >= rbegin(), JSON_TEXT("erase out of lo range"), return rbegin(););
		JSON_ASSERT_SAFE(_end >= rbegin(), JSON_TEXT("erase out of hi range"), return rbegin(););
		for (JSONNode ** pos = _start.it; pos > _end.it; --pos){
			deleteJSONNode(*pos);
		}
		const size_t num = _start.it - _end.it;
		internal -> Children.erase(_end.it + 1, num, _start.it);
		return (empty()) ? rend() : _start + num;
	}

	JSONNode::reverse_iterator JSONNode::insert(reverse_iterator pos, const JSONNode & x){
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("erasing a non-iteratable node"));
		JSON_ASSERT_UNIQUE("insert 1");
		if (pos.it < internal -> Children.begin()) {
			internal -> push_front(x);
			return rend() - 1;
		}
		JSON_ASSERT_SAFE(pos >= rbegin(), JSON_TEXT("insert out of range"), return rbegin(););
		internal -> Children.insert(++pos.it, newJSONNode(x), true);
		return pos;
	}

	JSONNode::reverse_iterator JSONNode::insertRFF(reverse_iterator pos, JSONNode ** const _start, JSONNode ** const _end){
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("erasing a non-iteratable node"));
		JSON_ASSERT_UNIQUE("insert RFF");
		JSON_ASSERT_SAFE(pos <= rend(), JSON_TEXT("insert out of range"), return rend(););
		JSON_ASSERT_SAFE(pos >= rbegin(), JSON_TEXT("insert out of range"), return rbegin(););
		const size_t num = _end - _start;
		json_auto<JSONNode *> mem(num);
		JSONNode ** runner = mem.ptr + num;
		for (JSONNode ** po = _start; po < _end; ++po){  //fill it backwards
			*(--runner) = newJSONNode(*(*po)    JSON_MUTEX_COPY2);
		}
		internal -> Children.insert(++pos.it, mem.ptr, num);
		return pos - num + 1;
	}

	JSONNode::iterator JSONNode::insertFRR(json_iterator pos, JSONNode ** const _start, JSONNode ** const _end){
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("erasing a non-iteratable node"));
		JSON_ASSERT_UNIQUE("insert FRR");
		JSON_ASSERT_SAFE(pos <= end(), JSON_TEXT("insert out of range"), return end(););
		JSON_ASSERT_SAFE(pos >= begin(), JSON_TEXT("insert out of range"), return begin(););
		const size_t num = _start - _end;
		json_auto<JSONNode *> mem(num);
		JSONNode ** runner = mem.ptr;
		for (JSONNode ** po = _start; po > _end; --po){
			*runner++=newJSONNode(*(*po)    JSON_MUTEX_COPY2);
		}
		internal -> Children.insert(pos.it, mem.ptr, num);
		return pos;
	}

	JSONNode::reverse_iterator JSONNode::insertRRR(reverse_iterator pos, JSONNode ** const _start, JSONNode ** const _end){
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("erasing a non-iteratable node"));
		JSON_ASSERT_UNIQUE("insert RRR");
		JSON_ASSERT_SAFE(pos <= rend(), JSON_TEXT("insert out of range"), return rend(););
		JSON_ASSERT_SAFE(pos >= rbegin(), JSON_TEXT("insert out of range"), return rbegin(););
		const size_t num = _start - _end;
		json_auto<JSONNode *> mem(num);
		JSONNode ** runner = mem.ptr;
		for (JSONNode ** po = _start; po > _end; --po){
			*runner++=newJSONNode(*(*po)    JSON_MUTEX_COPY2);
		}
		internal -> Children.insert(++pos.it, mem.ptr, num);
		return pos - num + 1;
	}
#endif

#endif
