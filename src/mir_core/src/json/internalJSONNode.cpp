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

#include "internalJSONNode.h"
#include "NumberToString.h"  //So that I can convert numbers into strings
#include "JSONNode.h"  //To fill in the foreward declaration
#include "JSONWorker.h"  //For fetching and parsing and such

/*
	The point of these constants is for faster assigning, if I
	were to constantly assign to a string literal, there would be
	lots of copies, but since strings are copy on write, this assignment
	is much faster
*/
static const json_string CONST_TRUE(JSON_TEXT("true"));
static const json_string CONST_FALSE(JSON_TEXT("false"));
static const json_string CONST_NULL(JSON_TEXT("null"));

#ifdef JSON_UNIT_TEST
	void internalJSONNode::incinternalAllocCount(void){ JSONNode::incinternalAllocCount(); }
	void internalJSONNode::decinternalAllocCount(void){ JSONNode::decinternalAllocCount(); }
#endif

internalJSONNode::internalJSONNode(const internalJSONNode & orig) :
	_type(orig._type), _name(orig._name), _name_encoded(orig._name_encoded), Children(),
	_string(orig._string), _string_encoded(orig._string_encoded), _value(orig._value)
	initializeRefCount(1)
	initializeFetch(orig.fetched)
	initializeMutex(0)
	initializeComment(orig._comment)
	initializeValid(orig.isValid){

	incinternalAllocCount();
	#ifdef JSON_MUTEX_CALLBACKS
		_set_mutex(orig.mylock, false);
	#endif
	if (!orig.Children.empty()) {
		Children.reserve(orig.Children.size());
		json_foreach(orig.Children, myrunner){
			Children.push_back(JSONNode::newJSONNode((*myrunner) -> duplicate()));
		}
	}
}

#ifdef JSON_PREPARSE
	#define SetFetchedFalseOrDo(code) code
#else
	#define SetFetchedFalseOrDo(code) SetFetched(false)
#endif

#ifdef JSON_VALIDATE
	#define NOTVALID false
#else
	#define NOTVALID
#endif

//this one is specialized because the root can only be array or node
internalJSONNode::internalJSONNode(const json_string & unparsed) : _type(), _name(),_name_encoded(false), _string(unparsed), _string_encoded(), _value(), Children()
	initializeMutex(0)
	initializeRefCount(1)
	initializeFetch(false)
	initializeComment(0)
	initializeValid(0){

	incinternalAllocCount();
	switch (unparsed[0]) {
		case JSON_TEXT('{'):  //node
			_type = JSON_NODE;
			#ifdef JSON_PREPARSE
				FetchNode();
			#endif
			break;
		case JSON_TEXT('['):  //array
			_type = JSON_ARRAY;
			#ifdef JSON_PREPARSE
				FetchArray();
			#endif
			break;
		default:
			JSON_FAIL_SAFE(JSON_TEXT("root not starting with either { or ["), Nullify(NOTVALID););
			break;
	}
}

internalJSONNode::internalJSONNode(const json_string & name_t, const json_string & value_t) : _type(), _name_encoded(), _name(JSONWorker::FixString(name_t, NAME_ENCODED)), _string(), _string_encoded(), _value(), Children()
	initializeMutex(0)
	initializeRefCount(1)
	initializeFetch(0)
	initializeComment(0)
	initializeValid(0){

	incinternalAllocCount();

	if (value_t.empty()) {
		_type = JSON_NULL;
		#ifdef JSON_VALIDATE
			isValid = true;
		#endif
		SetFetched(true);
		return;
	}

	_string = value_t;
	const json_char firstchar = value_t[0];
	#if defined JSON_DEBUG || defined JSON_SAFE
		const json_char lastchar = value_t[value_t.length() - 1];
	#endif

	switch (firstchar){
		case JSON_TEXT('\"'):  //a json_string literal, still escaped and with leading and trailing quotes
			JSON_ASSERT_SAFE(lastchar == JSON_TEXT('\"'), JSON_TEXT("Unterminated quote"), Nullify(NOTVALID); return;);
			_type = JSON_STRING;
			SetFetchedFalseOrDo(FetchString());
			break;
		case JSON_TEXT('{'):  //a child node, or set of children
			JSON_ASSERT_SAFE(lastchar == JSON_TEXT('}'), JSON_TEXT("Missing }"), Nullify(NOTVALID); return;);
			_type = JSON_NODE;
			SetFetchedFalseOrDo(FetchNode());
			break;
		case JSON_TEXT('['):  //an array
			JSON_ASSERT_SAFE(lastchar == JSON_TEXT(']'), JSON_TEXT("Missing ]"), Nullify(NOTVALID); return;);
			_type = JSON_ARRAY;
			SetFetchedFalseOrDo(FetchArray());
			break;
		case JSON_TEXT('t'):
			JSON_ASSERT_SAFE(value_t == JSON_TEXT("true"), json_string(json_string(JSON_TEXT("unknown JSON literal: ")) + value_t).c_str(), Nullify(NOTVALID); return;);
			_value._bool = true;
			_type = JSON_BOOL;
			SetFetched(true);
			break;
		case JSON_TEXT('f'):
			JSON_ASSERT_SAFE(value_t == JSON_TEXT("false"), json_string(json_string(JSON_TEXT("unknown JSON literal: ")) + value_t).c_str(), Nullify(NOTVALID); return;);
			_value._bool = false;
			_type = JSON_BOOL;
			SetFetched(true);
			break;
		case JSON_TEXT('n'):
			JSON_ASSERT_SAFE(value_t == JSON_TEXT("null"), json_string(json_string(JSON_TEXT("unknown JSON literal: ")) + value_t).c_str(), Nullify(NOTVALID); return;);
			_type = JSON_NULL;
			SetFetched(true);
			#ifdef JSON_VALIDATE
				isValid = true;
			#endif
			break;
		default:
			JSON_ASSERT_SAFE(value_t.find_first_not_of(JSON_TEXT("0123456789.e+-")) == json_string::npos, json_string(json_string(JSON_TEXT("unknown JSON literal: ")) + value_t).c_str(), Nullify(NOTVALID); return;);
			_type = JSON_NUMBER;
			SetFetchedFalseOrDo(FetchNumber());
			break;
	}
}

internalJSONNode::~internalJSONNode(void){
	decinternalAllocCount();
	#ifdef JSON_MUTEX_CALLBACKS
		_unset_mutex();
	#endif
	//DO NOT delete the children!  It automatically gets removed
}

void internalJSONNode::FetchString(void) const {
	JSON_ASSERT_SAFE(!_string.empty(), JSON_TEXT("JSON json_string type is empty?"), Nullify(NOTVALID); return;);
	JSON_ASSERT_SAFE(_string[0] == JSON_TEXT('\"'), JSON_TEXT("JSON json_string type doesn't start with a quotation?"), Nullify(NOTVALID); return;);
	JSON_ASSERT_SAFE(_string[_string.length() - 1] == JSON_TEXT('\"'), JSON_TEXT("JSON json_string type doesn't end with a quotation?"), Nullify(NOTVALID); return;);
	_string = JSONWorker::FixString(_string.substr(1, _string.length() - 2), STRING_ENCODED);
}

void internalJSONNode::FetchNode(void) const {
	JSON_ASSERT_SAFE(!_string.empty(), JSON_TEXT("JSON node type is empty?"), Nullify(NOTVALID); return;);
	JSON_ASSERT_SAFE(_string[0] == JSON_TEXT('{'), JSON_TEXT("JSON node type doesn't start with a bracket?"), Nullify(NOTVALID); return;);
	JSON_ASSERT_SAFE(_string[_string.length() - 1] == JSON_TEXT('}'), JSON_TEXT("JSON node type doesn't end with a bracket?"), Nullify(NOTVALID); return;);
	JSONWorker::DoNode(this, _string);
	clearString(_string);
}

void internalJSONNode::FetchArray(void) const {
	JSON_ASSERT_SAFE(!_string.empty(), JSON_TEXT("JSON node type is empty?"), Nullify(NOTVALID); return;);
	JSON_ASSERT_SAFE(_string[0] == JSON_TEXT('['), JSON_TEXT("JSON node type doesn't start with a square bracket?"), Nullify(NOTVALID); return;);
	JSON_ASSERT_SAFE(_string[_string.length() - 1] == JSON_TEXT(']'), JSON_TEXT("JSON node type doesn't end with a square bracket?"), Nullify(NOTVALID); return;);
	JSONWorker::DoArray(this, _string);
	clearString(_string);
}

void internalJSONNode::FetchNumber(void) const {
	#ifdef JSON_UNICODE
		{
			const size_t len = _string.length();
			json_auto<char> temp(len + 1);
			wcstombs(temp.ptr, _string.c_str(), len);
			temp.ptr[len] = '\0';
			_value._number = (json_number)atof(temp.ptr);
		}
	#else
		_value._number = (json_number)atof(_string.c_str());
	#endif
}

#ifndef JSON_PREPARSE
	void internalJSONNode::Fetch(void) const {
		if (fetched) return;
		switch (type()) {
			case JSON_STRING:
				FetchString();
				break;
			case JSON_NODE:
				FetchNode();
				break;
			case JSON_ARRAY:
				FetchArray();
				break;
			case JSON_NUMBER:
				FetchNumber();
				break;
			#if defined JSON_DEBUG || defined JSON_SAFE
				default:
					JSON_FAIL(JSON_TEXT("Fetching an unknown type"));
					Nullify(NOTVALID);
			#endif
		}
		fetched = true;
	}
#endif

void internalJSONNode::Set(const json_string & val){
	_type = JSON_STRING;
	_string = val;
	_string_encoded = true;
	SetFetched(true);
}

#ifdef JSON_LIBRARY
	void internalJSONNode::Set(long val){
		_type = JSON_NUMBER;
		_value._number = (json_number)val;
		_string = NumberToString::_itoa<long>(val, sizeof(long));
		SetFetched(true);
	}

	void internalJSONNode::Set(json_number val){
		_type = JSON_NUMBER;
		_value._number = val;
		_string = NumberToString::_ftoa<json_number>(val);
		SetFetched(true);
	}
#else
	#define SET(converter, type)\
		void internalJSONNode::Set(type val){\
			_type = JSON_NUMBER;\
			_value._number = (json_number)val;\
			_string = NumberToString::converter<type>(val, sizeof(type));\
			SetFetched(true);\
		}
	#define SET_INTEGER(type) SET(_itoa, type) SET(_uitoa, unsigned type)
	#define SET_FLOAT(type) \
		void internalJSONNode::Set(type val){\
			_type = JSON_NUMBER;\
			_value._number = (json_number)val;\
			_string = NumberToString::_ftoa<type>(val);\
			SetFetched(true);\
		}

	SET_INTEGER(char)
	SET_INTEGER(short)
	SET_INTEGER(int)
	SET_INTEGER(long)
	#ifndef JSON_ISO_STRICT
		SET_INTEGER(long long)
	#endif

	SET_FLOAT(float)
	SET_FLOAT(double)
#endif

void internalJSONNode::Set(bool val){
	_type = JSON_BOOL;
	_value._bool = val;
	_string = val ? CONST_TRUE : CONST_FALSE;
	SetFetched(true);
}

bool internalJSONNode::IsEqualTo(const internalJSONNode * val) const {
	#ifdef JSON_REF_COUNT
		if (this == val) return true;  //reference counting the same internal object, so they must be equal
	#endif
	if (type() != val -> type()) return false;  //aren't even same type
	if (_name != val -> _name) return false;  //names aren't the same
	if (type() == JSON_NULL) return true;  //both null, can't be different
	#ifndef JSON_PREPARSE
		Fetch();
		val -> Fetch();
	#endif
	switch (type()) {
		case JSON_STRING:
			return val -> _string == _string;
		case JSON_NUMBER:
			return NumberToString::areEqual(val -> _value._number, _value._number);
		case JSON_BOOL:
			return val -> _value._bool == _value._bool;
	};

	JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("Checking for equality, not sure what type"));
	if (Children.size() != val -> Children.size()) return false;  //if they arne't he same size then they certainly aren't equal

	//make sure each children is the same
	JSONNode ** valrunner = val -> Children.begin();
	json_foreach(Children, myrunner){
		JSON_ASSERT(*myrunner, JSON_TEXT("a null pointer within the children"));
		JSON_ASSERT(*valrunner, JSON_TEXT("a null pointer within the children"));
		JSON_ASSERT(valrunner != val -> Children.end(), JSON_TEXT("at the end of other one's children, but they're the same size?"));
		if (**myrunner != **valrunner) return false;
		++valrunner;
	}
	return true;
}

#ifdef JSON_VALIDATE
void internalJSONNode::Nullify(bool validation) const {
	isValid = validation;
#else
void internalJSONNode::Nullify(void) const {
#endif
	_type = JSON_NULL;
	_string = CONST_NULL;
	SetFetched(true);
}

#ifdef JSON_MUTEX_CALLBACKS
	#define JSON_MUTEX_COPY ,mylock
#else
	#define JSON_MUTEX_COPY
#endif

#ifdef JSON_LIBRARY
void internalJSONNode::push_back(JSONNode *node){
#else
void internalJSONNode::push_back(const JSONNode & node){
#endif
	JSON_ASSERT(type() == JSON_ARRAY || type() == JSON_NODE, JSON_TEXT("pushing back to something that is not an array or object"));
	#ifdef JSON_LIBRARY
		#ifdef JSON_MUTEX_CALLBACKS
			if (mylock) node -> set_mutex(mylock);
		#endif
		Children.push_back(node);
	#else
		Children.push_back(JSONNode::newJSONNode(node   JSON_MUTEX_COPY));
	#endif
}

void internalJSONNode::push_front(const JSONNode & node){
	JSON_ASSERT(type() == JSON_ARRAY || type() == JSON_NODE, JSON_TEXT("pushing front to something that is not an array or object"));
	Children.push_front(JSONNode::newJSONNode(node   JSON_MUTEX_COPY));
}

JSONNode * internalJSONNode::pop_back(json_index_t pos){
	JSONNode * result = Children[pos];
	JSONNode ** temp = Children.begin() + pos;
	Children.erase(temp);
	return result;
}

JSONNode * internalJSONNode::pop_back(const json_string & name_t){
	if (JSONNode ** res = at(name_t)) {
		JSONNode * result = *res;
		Children.erase(res);
		return result;
	}
	return 0;
}

#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	JSONNode * internalJSONNode::pop_back_nocase(const json_string & name_t){
		if (JSONNode ** res = at_nocase(name_t)) {
			JSONNode * result = *res;
			Children.erase(res);
			return result;
		}
		return 0;
	}
#endif

JSONNode ** internalJSONNode::at(const json_string & name_t){
	Fetch();
	json_foreach(Children, myrunner){
		JSON_ASSERT(*myrunner, JSON_TEXT("a null pointer within the children"));
		if ((*myrunner) -> name() == name_t) return myrunner;
	}
	return 0;
}

#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	bool internalJSONNode::AreEqualNoCase(const json_char * ch_one, const json_char * ch_two){
		while (*ch_one){  //only need to check one, if the other one terminates early, the check will cause it to fail
			const json_char c_one = *ch_one;
			const json_char c_two = *ch_two;
			if (c_one != c_two){
				if ((c_two > 64) && (c_two < 91)) {  //A - Z
					if (c_one != (json_char)(c_two + 32)) return false;
				} else if ((c_two > 96) && (c_two < 123)) {  //a - z
					if (c_one != (json_char)(c_two - 32)) return false;
				} else { //not a letter, so return false
					return false;
				}
			}
			++ch_one;
			++ch_two;

		}
		return *ch_two == '\0';  //this one has to be null terminated too, or else json_string two is longer, hence, not equal
	}

	JSONNode ** internalJSONNode::at_nocase(const json_string & name_t){
		Fetch();
		json_foreach(Children, myrunner){
			JSON_ASSERT(*myrunner, JSON_TEXT("a null pointer within the children"));
			if (AreEqualNoCase((*myrunner) -> name().c_str(), name_t.c_str())) return myrunner;
		}
		return 0;
	}
#endif

#ifndef JSON_PREPARSE
	void internalJSONNode::preparse(void){
		Fetch();
		json_foreach(Children, myrunner){
			(*myrunner) -> preparse();
		}
	}
#endif

#ifdef JSON_VALIDATE
	bool internalJSONNode::validate(void){
		json_foreach(Children, myrunner){
			if ((*myrunner) -> type() != JSON_NULL){
				#ifndef JSON_PREPARSE
					(*myrunner) -> internal -> Fetch();
				#endif
				if ((*myrunner) -> type() == JSON_NULL) return false;
			} else if (!((*myrunner) -> internal -> isValid)) {
				JSON_FAIL(_name + JSON_TEXT(" is null and not valid"));
				return false;
			}
		}
		json_foreach(Children, runner){
			if (!((*runner) -> internal -> validate())) return false;
		}
		return true;
	}
#endif

#ifdef JSON_DEBUG
	#ifndef JSON_LIBRARY
		JSONNode internalJSONNode::Dump(size_t & totalbytes) const {
			JSONNode dumpage(JSON_NODE);
			dumpage.set_name(JSON_TEXT("internalJSONNode"));
			dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("this"), (long)this)));

			size_t memory = sizeof(internalJSONNode);
			memory += _name.capacity() * sizeof(json_char);
			memory += _string.capacity() * sizeof(json_char);
			memory += Children.capacity() * sizeof(JSONNode*);
			#ifdef JSON_COMMENTS
				memory += _comment.capacity() * sizeof(json_char);
			#endif
			totalbytes += memory;
			dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("bytes used"), memory)));

			#ifdef JSON_REF_COUNT
				dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("refcount"), refcount)));
			#endif
			#ifdef JSON_MUTEX_CALLBACKS
				dumpage.push_back(JSON_NEW(DumpMutex()));
			#endif

			#define DUMPCASE(ty)\
				case ty:\
					dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("_type"), JSON_TEXT(#ty))));\
					break;

			switch(type()) {
				DUMPCASE(JSON_NULL)
				DUMPCASE(JSON_STRING)
				DUMPCASE(JSON_NUMBER)
				DUMPCASE(JSON_BOOL)
				DUMPCASE(JSON_ARRAY)
				DUMPCASE(JSON_NODE)
				default:
					dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("_type"), JSON_TEXT("Unknown"))));
			}

			JSONNode str(JSON_NODE);
			str.set_name(JSON_TEXT("_name"));
			str.push_back(JSON_NEW(JSONNode(json_string(JSON_TEXT("value")), _name)));
			str.push_back(JSON_NEW(JSONNode(JSON_TEXT("length"), _name.length())));
			str.push_back(JSON_NEW(JSONNode(JSON_TEXT("capactiy"), _name.capacity())));

			#ifdef JSON_LESS_MEMORY
				dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("_name_encoded"), _type & 0x10)));
			#else
				dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("_name_encoded"), _name_encoded)));
			#endif
			dumpage.push_back(JSON_NEW(str));
			dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("_string_encoded"), _string_encoded)));
			str.clear();
			str.set_name(JSON_TEXT("_string"));
			str.push_back(JSON_NEW(JSONNode(json_string(JSON_TEXT("value")), _string)));
			str.push_back(JSON_NEW(JSONNode(JSON_TEXT("length"), _string.length())));
			str.push_back(JSON_NEW(JSONNode(JSON_TEXT("capactiy"), _string.capacity())));
			dumpage.push_back(JSON_NEW(str));

			JSONNode unio(JSON_NODE);
			unio.set_name(JSON_TEXT("_value"));
			unio.push_back(JSON_NEW(JSONNode(JSON_TEXT("_bool"), _value._bool)));
			unio.push_back(JSON_NEW(JSONNode(JSON_TEXT("_number"), _value._number)));
			dumpage.push_back(JSON_NEW(unio));

			#ifndef JSON_PREPARSE
				dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("fetched"), fetched)));
			#endif

			#ifdef JSON_COMMENTS
				str.clear();
				str.set_name(JSON_TEXT("_comment"));
				str.push_back(JSON_NEW(JSONNode(JSON_TEXT("value"), _comment)));
				str.push_back(JSON_NEW(JSONNode(JSON_TEXT("length"), _comment.length())));
				str.push_back(JSON_NEW(JSONNode(JSON_TEXT("capactiy"), _comment.capacity())));
				dumpage.push_back(JSON_NEW(str));
			#endif

			JSONNode arra(JSON_NODE);
			arra.set_name(JSON_TEXT("Children"));
			arra.push_back(JSON_NEW(JSONNode(JSON_TEXT("size"), Children.size())));
			arra.push_back(JSON_NEW(JSONNode(JSON_TEXT("capacity"), Children.capacity())));
			JSONNode chil(JSON_ARRAY);
			chil.set_name(JSON_TEXT("array"));
			json_foreach(Children, it){
				chil.push_back(JSON_NEW((*it) -> dump(totalbytes)));
			}
			arra.push_back(JSON_NEW(chil));
			dumpage.push_back(JSON_NEW(arra));

			return dumpage;
		}
	#endif
#endif
