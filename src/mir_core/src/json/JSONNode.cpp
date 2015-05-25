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

#include "JSONNode.h"

#ifdef JSON_UNIT_TEST
	int allocCount = 0;
	int deallocCount = 0;
	int internalAllocCount = 0;
	int internalDeallocCount = 0;
	int JSONNode::getNodeAllocationCount(void){ return allocCount; }
	int JSONNode::getNodeDeallocationCount(void){ return deallocCount; }
	int JSONNode::getInternalAllocationCount(void){ return internalAllocCount; }
	int JSONNode::getInternalDeallocationCount(void){ return internalDeallocCount; }
	void JSONNode::incAllocCount(void){ ++allocCount; }
	void JSONNode::decAllocCount(void){ ++deallocCount; }
	void JSONNode::incinternalAllocCount(void){ ++internalAllocCount; }
	void JSONNode::decinternalAllocCount(void){ ++internalDeallocCount; }
#endif

JSONNode nullNode(JSON_NULL);

#define IMPLEMENT_CTOR(type)\
	JSONNode::JSONNode(const json_string & name_t, type value_t) : internal(internalJSONNode::newInternal()) {\
		internal -> Set(value_t);\
		internal -> setname(name_t);\
		incAllocCount();\
	}
IMPLEMENT_FOR_ALL_TYPES(IMPLEMENT_CTOR)

#ifndef JSON_LIBRARY
	JSONNode::JSONNode(const json_string & name_t, const json_char * value_t) : internal(internalJSONNode::newInternal()) {
		internal -> Set(json_string(value_t));
		internal -> setname(name_t);
		incAllocCount();
	}
#endif

JSONNode JSONNode::as_node(void) const {
	JSON_CHECK_INTERNAL();
	if (type() == JSON_NODE){
		return *this;
	} else if (type() == JSON_ARRAY){
		JSONNode res = duplicate();
		res.internal -> _type = JSON_NODE;
		return res;
	}
	#ifdef JSON_MUTEX_CALLBACKS
		if (internal -> mylock){
			JSONNode res = JSONNode(JSON_NODE);
			res.set_mutex(internal -> mylock);
			return res;
		}
	#endif
	return JSONNode(JSON_NODE);
}

JSONNode JSONNode::as_array(void) const {
	JSON_CHECK_INTERNAL();
	if (type() == JSON_ARRAY){
		return *this;
	} else if (type() == JSON_NODE){
		JSONNode res = duplicate();
		res.internal -> _type = JSON_ARRAY;
		json_foreach(res.internal -> Children, runner){
			(*runner) -> set_name(JSON_TEXT(""));
		}
		return res;
	}
	#ifdef JSON_MUTEX_CALLBACKS
		if (internal -> mylock){
			JSONNode res = JSONNode(JSON_ARRAY);
			res.set_mutex(internal -> mylock);
			return res;
		}
	#endif
	return JSONNode(JSON_ARRAY);
}

void JSONNode::cast(char newtype){
	JSON_CHECK_INTERNAL();
	if (newtype == type()) return;

	switch(newtype){
		case JSON_NULL:
			nullify();
			return;
		case JSON_STRING:
			*this = as_string();
			return;
		case JSON_NUMBER:
			*this = as_float();
			return;
		case JSON_BOOL:
			*this = as_bool();
			return;
		case JSON_ARRAY:
			*this = as_array();
			return;
		case JSON_NODE:
			*this = as_node();
			return;
	}
	JSON_FAIL(JSON_TEXT("cast to unknown type"));
}

//different just to supress the warning
#ifdef JSON_REF_COUNT
void JSONNode::merge(JSONNode & other){
#else
void JSONNode::merge(JSONNode &) {
#endif
	JSON_CHECK_INTERNAL();
	#ifdef JSON_REF_COUNT
		if (internal == other.internal) return;
		JSON_ASSERT(*this == other, JSON_TEXT("merging two nodes that aren't equal"));
		if (internal -> refcount < other.internal -> refcount){
			*this = other;
		} else {
			other = *this;
		}
	#endif
}

#ifdef JSON_REF_COUNT
	void JSONNode::merge(JSONNode * other){
		JSON_CHECK_INTERNAL();
		if (internal == other -> internal) return;
		*other = *this;
	}

	//different just to supress the warning
	void JSONNode::merge(unsigned int num, ...) {
#else
	void JSONNode::merge(unsigned int, ...) {
#endif
	JSON_CHECK_INTERNAL();
	#ifdef JSON_REF_COUNT
		va_list args;
		va_start(args, num);
		for(unsigned int i=0; i < num; ++i){
			merge(va_arg(args, JSONNode*));
		}
		va_end(args);
	#endif
}

JSONNode JSONNode::duplicate(void) const {
	JSON_CHECK_INTERNAL();
	JSONNode mycopy(*this);
	#ifdef JSON_REF_COUNT
		JSON_ASSERT(internal == mycopy.internal, JSON_TEXT("copy ctor failed to ref count correctly"));
		mycopy.makeUniqueInternal();
	#endif
	JSON_ASSERT(internal != mycopy.internal, JSON_TEXT("makeUniqueInternal failed"));
	return mycopy;
}

JSONNode & JSONNode::at(json_index_t pos){
	JSON_CHECK_INTERNAL();
	if (pos >= internal -> size()) {
		JSON_FAIL(JSON_TEXT("at() out of bounds"));
		return nullNode;
	}
	return (*this)[pos];
}

const JSONNode & JSONNode::at(json_index_t pos) const {
	JSON_CHECK_INTERNAL();
	if (pos >= internal -> size()) {
		JSON_FAIL(JSON_TEXT("at() const out of bounds"));
		return nullNode;
	}
	return (*this)[pos];
}

JSONNode & JSONNode::operator[](json_index_t pos){
	JSON_CHECK_INTERNAL();
	JSON_ASSERT(pos < internal -> size(), JSON_TEXT("[] out of bounds"));
	makeUniqueInternal();
	return *(internal -> at(pos));
}

const JSONNode & JSONNode::operator[](json_index_t pos) const {
	JSON_CHECK_INTERNAL();
	JSON_ASSERT(pos < internal -> size(), JSON_TEXT("[] const out of bounds"));
	return *(internal -> at(pos));
}

JSONNode & JSONNode::at(const json_char *name_t){
	JSON_CHECK_INTERNAL();
	JSON_ASSERT(type() == JSON_NODE, JSON_TEXT("at a non-iteratable node"));
	makeUniqueInternal();
	if (JSONNode ** res = internal -> at(name_t)) {
		return *(*res);
	}
	JSON_FAIL(json_string(JSON_TEXT("at could not find child by name: ")) + name_t);
	return nullNode;
}

const JSONNode & JSONNode::at(const json_char *name_t) const {
	JSON_CHECK_INTERNAL();
	JSON_ASSERT(type() == JSON_NODE, JSON_TEXT("at a non-iteratable node"));
	if (JSONNode ** res = internal -> at(name_t)) {
		return *(*res);
	}
	JSON_FAIL(json_string(JSON_TEXT("at const could not find child by name: ")) + name_t);
	return nullNode;
}

#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	JSONNode & JSONNode::at_nocase(const json_string & name_t){
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE, JSON_TEXT("at a non-iteratable node"));
		makeUniqueInternal();
		if (JSONNode ** res = internal -> at_nocase(name_t)) {
			return *(*res);
		}
		JSON_FAIL(json_string(JSON_TEXT("at_nocase could not find child by name: ")) + name_t);
		return nullNode;
	}

	const JSONNode & JSONNode::at_nocase(const json_string & name_t) const {
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE, JSON_TEXT("at a non-iteratable node"));
		if (JSONNode ** res = internal -> at_nocase(name_t)) {
			return *(*res);
		}
		JSON_FAIL(json_string(JSON_TEXT("at_nocase const could not find child by name: ")) + name_t);
		return nullNode;
	}
#endif

#ifndef JSON_LIBRARY
	struct auto_delete {
		public:
			auto_delete(JSONNode *node) : mynode(node){};
			~auto_delete(void){ JSONNode::deleteJSONNode(mynode); };
			JSONNode * mynode;
		private:
			auto_delete(const auto_delete &);
			auto_delete & operator = (const auto_delete &);
	};
#endif

JSONNode JSON_PTR_LIB JSONNode::pop_back(json_index_t pos){
	JSON_CHECK_INTERNAL();
	if (pos >= internal -> size()) {
		JSON_FAIL(JSON_TEXT("pop_back out of bounds"));
		return nullNode;
	}
	makeUniqueInternal();
	#ifdef JSON_LIBRARY
		return internal -> pop_back(pos);
	#else
		auto_delete temp(internal -> pop_back(pos));
		return *temp.mynode;
	#endif
}

JSONNode JSON_PTR_LIB JSONNode::pop_back(const json_string & name_t){
	JSON_CHECK_INTERNAL();
	JSON_ASSERT(type() == JSON_NODE, JSON_TEXT("popping a non-iteratable node"));
	#ifdef JSON_LIBRARY
		return internal -> pop_back(name_t);
	#else
		if (JSONNode * res = internal -> pop_back(name_t)) {
			auto_delete temp(res);
			return *(temp.mynode);
		}
		JSON_FAIL(json_string(JSON_TEXT("pop_back const could not find child by name: ")) + name_t);
		return nullNode;
	#endif
}

#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	JSONNode JSON_PTR_LIB JSONNode::pop_back_nocase(const json_string & name_t){
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE, JSON_TEXT("popping a non-iteratable node"));
		#ifdef JSON_LIBRARY
			return internal -> pop_back_nocase(name_t);
		#else
			if (JSONNode * res = internal -> pop_back_nocase(name_t)) {
				auto_delete temp(res);
				return *(temp.mynode);
			}
			JSON_FAIL(json_string(JSON_TEXT("pop_back_nocase could not find child by name: ")) + name_t);
			return nullNode;
		#endif
	}
#endif
