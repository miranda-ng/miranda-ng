#ifndef INTERNAL_JSONNODE_H
#define INTERNAL_JSONNODE_H

#include "JSONDebug.h"
#include "JSONChildren.h"
#include "JSONMemory.h"
#ifdef JSON_DEBUG
	#include <climits>  //to check int value
#endif

/*
	This class is the work horse of libJSON, it handles all of the
	functinality of JSONNode. This object is reference counted for
	speed and memory reasons.

	If JSON_REF_COUNT is not on, this internal structure still has an important
	purpose, as it can be passed around by JSONNoders that are flagged as temporary
*/

class JSONNode;  //forward declaration

#ifndef JSON_LIBRARY
	#define DECL_SET_INTEGER(type) void Set(type); void Set(unsigned type);
#endif

#ifdef JSON_MUTEX_CALLBACKS
	#define initializeMutex(x) ,mylock(x)
#else
	#define initializeMutex(x)
#endif

#ifdef JSON_PREPARSE
	#define SetFetched(b) (void)0
	#define Fetch() (void)0
#define initializeFetch(x)
#else
	#define initializeFetch(x) ,fetched(x)
#endif

#ifdef JSON_REF_COUNT
	#define initializeRefCount(x) ,refcount(x)
#else
	#define initializeRefCount(x)
#endif

#ifdef JSON_COMMENTS
	#define initializeComment(x) ,_comment(x)
#else
	#define initializeComment(x)
#endif

#ifndef JSON_UNIT_TEST
	#define incAllocCount() (void)0
	#define decAllocCount() (void)0
	#define incinternalAllocCount() (void)0
	#define decinternalAllocCount() (void)0
#endif

#ifdef JSON_VALIDATE
	#define initializeValid(x) ,isValid(x)
#else
	#define initializeValid(x)
#endif

class internalJSONNode {
public:
	internalJSONNode(char mytype = JSON_NULL);
	internalJSONNode(const json_string & unparsed);
	internalJSONNode(const json_string & name_t, const json_string & value_t);
	internalJSONNode(const internalJSONNode & orig);  
	internalJSONNode & operator = (const internalJSONNode &);
	~internalJSONNode(void);

	static internalJSONNode * newInternal(char mytype = JSON_NULL);
	static internalJSONNode * newInternal(const json_string & unparsed);
	static internalJSONNode * newInternal(const json_string & name_t, const json_string & value_t);
	static internalJSONNode * newInternal(const internalJSONNode & orig);  //not copyable, only by this class
	static void deleteInternal(internalJSONNode * ptr);

	json_index_t size(void) const;
	bool empty(void) const;
	unsigned char type(void) const;

	const json_char* name(void) const;
	void setname(const json_string & newname);
	#ifdef JSON_COMMENTS
		void setcomment(const json_string & comment);
		json_string getcomment(void) const;
	#endif
	json_string as_string(void) const;
	#if defined(M_STRING_H__)
		CMString as_mstring(void) const;
	#endif
	long as_int(void) const;
	json_number as_float(void) const;
	bool as_bool(void) const;

	#ifndef JSON_PREPARSE
		void preparse(void);
	#endif

	#ifdef JSON_LIBRARY
		void push_back(JSONNode *node);
	#else
		void push_back(const JSONNode & node);
	#endif
	void reserve(json_index_t size);
	void push_front(const JSONNode & node);
	JSONNode * pop_back(json_index_t pos);
	JSONNode * pop_back(const json_string & name_t);
	#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
		JSONNode * pop_back_nocase(const json_string & name_t);
	#endif

	JSONNode * at(json_index_t pos);
	//These return ** because pop_back needs them
	JSONNode ** at(const json_string & name_t);
	#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
		JSONNode ** at_nocase(const json_string & name_t);
	#endif

	void Set(const json_string & val);
	#ifdef JSON_LIBRARY
		void Set(json_number val);
		void Set(long val);
	#else
		DECL_SET_INTEGER(char)
		DECL_SET_INTEGER(short)
		DECL_SET_INTEGER(int)
		DECL_SET_INTEGER(long)
		#ifndef JSON_ISO_STRICT
			DECL_SET_INTEGER(long long)
		#endif

		void Set(float val);
		void Set(double val);
	#endif
	void Set(bool val);

	bool IsEqualTo(const json_string & val)const ;
	bool IsEqualTo(bool val) const;
	bool IsEqualTo(const internalJSONNode * val) const;  

	template<typename T>
	bool IsEqualToNum(T val) const;

	internalJSONNode * incRef(void);
	#ifdef JSON_REF_COUNT
		void decRef(void);
		bool hasNoReferences(void);
	#endif
	internalJSONNode * makeUnique(void);

	JSONNode ** begin(void) const;
	JSONNode ** end(void) const;
	#ifdef JSON_REF_COUNT
		size_t refcount BITS(20);
	#endif
	bool Fetched(void) const;
	#ifdef JSON_MUTEX_CALLBACKS
		void * mylock;
		void _set_mutex(void * mutex, bool unset = true);
		void _unset_mutex(void);
	#endif
	#ifdef JSON_UNIT_TEST
		static void incinternalAllocCount(void);
		static void decinternalAllocCount(void);
	#endif

	#ifdef JSON_WRITER
		json_string WriteName(bool formatted, bool arrayChild) const;
		json_string WriteChildren(unsigned int indent);
		json_string WriteComment(unsigned int indent) const;
		json_string Write(unsigned int indent, bool arrayChild);
	#endif
	#ifdef JSON_DEBUG
		#ifndef JSON_LIBRARY
			JSONNode Dump(size_t & totalmemory) const;
			JSONNode DumpMutex(void) const;
		#endif
	#endif

	//json parts
	mutable unsigned char _type BITS(3);
	mutable bool _name_encoded BITS(1);  //must be above name due to initialization list order
	json_string _name;	

	mutable json_string _string;   //these are both mutable because the string can change when it's fetched
	mutable bool _string_encoded BITS(1);

	//the value of the json
	union value_union_t {
		bool _bool;
		json_number _number;
	};
	mutable value_union_t _value; //internal structure changes depending on type

	jsonChildren Children;  //container that holds all of my children

	#ifdef JSON_VALIDATE
		mutable bool isValid BITS(1);  //this does not need to be initialized, it's only used if it's null
		void Nullify(bool validation = true) const;
		bool validate(void);
	#else
		void Nullify(void) const;
	#endif

	//Fetching and such
	#ifndef JSON_PREPARSE
		mutable bool fetched BITS(1);
		void SetFetched(bool val) const;
		void Fetch(void) const;  //it's const because it doesn't change the VALUE of the function
	#endif

	#ifdef JSON_COMMENTS
		json_string _comment;
	#endif

	void FetchString(void) const;
	void FetchNode(void) const;
	void FetchArray(void) const;
	void FetchNumber(void) const;
	#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
		static bool AreEqualNoCase(const json_char * ch_one, const json_char * ch_two);
	#endif
};

inline internalJSONNode::internalJSONNode(char mytype) : _type(mytype), Children(), _name(), _name_encoded(), _string(), _string_encoded(), _value()
	initializeMutex(0)  
	initializeRefCount(1)
	initializeFetch(true)
	initializeComment(0)
	initializeValid(true){

	incinternalAllocCount();
}

inline internalJSONNode * internalJSONNode::incRef(void){
	#ifdef JSON_REF_COUNT
		++refcount;
		return this;
	#else
		return makeUnique();
	#endif
}

inline json_index_t internalJSONNode::size(void) const {
	Fetch();
	return Children.size();
}

inline bool internalJSONNode::empty(void) const {
	if (type() != JSON_NODE && type() != JSON_ARRAY) return true;
	Fetch();
	return Children.empty();
}

inline unsigned char internalJSONNode::type(void) const {
	#ifdef JSON_LESS_MEMORY
		return _type & 0xF;
	#else
		return _type;
	#endif
}

inline const json_char* internalJSONNode::name(void) const {
	return _name.c_str();
}

inline void internalJSONNode::setname(const json_string & newname){
	_name = newname;
	#ifdef JSON_LESS_MEMORY
		_type |= 0x10;
	#else
		_name_encoded = true; 
	#endif
}

#ifdef JSON_COMMENTS
	inline void internalJSONNode::setcomment(const json_string & comment){
		_comment = comment;
	}

	inline json_string internalJSONNode::getcomment(void) const {
		return _comment;
	}
#endif

inline json_string internalJSONNode::as_string(void) const {
	Fetch();
	return _string;
}

#if defined(M_STRING_H__)
	inline CMString internalJSONNode::as_mstring(void) const {
		Fetch();
		return CMString(ptrT(mir_utf8decodeT(_string.c_str())));
	}
#endif

inline long internalJSONNode::as_int(void) const {
	Fetch();
	switch(type()) {
		case JSON_NULL:
			return 0;
		case JSON_BOOL:
			return _value._bool ? 1 : 0;
	}
	JSON_ASSERT(type() == JSON_NUMBER, JSON_TEXT("as_int returning undefined results"));
	JSON_ASSERT(_value._number > LONG_MIN, _string + JSON_TEXT(" is outside the lower range of long"));
	JSON_ASSERT(_value._number < LONG_MAX, _string + JSON_TEXT(" is outside the upper range of long"));
	JSON_ASSERT(_value._number == (json_number)((int)_value._number), json_string(JSON_TEXT("as_int will truncate ")) + _string);
	return (int)_value._number;
}

inline json_number internalJSONNode::as_float(void) const {
	Fetch();
	switch(type()) {
		case JSON_NULL:
			return (json_number)0.0;
		case JSON_BOOL:
			return (json_number)(_value._bool ? 1.0 : 0.0);
	}
	JSON_ASSERT(type() == JSON_NUMBER, JSON_TEXT("as_float returning undefined results"));
	return _value._number;   
}

inline bool internalJSONNode::as_bool(void) const {
	Fetch();
	switch(type()) {
		case JSON_NUMBER:
			return _value._number != 0.0f;
		case JSON_NULL:
			return false;
	}
	JSON_ASSERT(type() == JSON_BOOL, JSON_TEXT("as_bool returning undefined results"));
	return _value._bool;
}

inline bool internalJSONNode::IsEqualTo(const json_string & val) const {
	if (type() != JSON_STRING) return false;
	Fetch(); 
	return val == _string;
}

inline bool internalJSONNode::IsEqualTo(bool val) const {
	if (type() != JSON_BOOL) return false;
	Fetch(); 
	return val == _value._bool;
}

template<typename T>
inline bool internalJSONNode::IsEqualToNum(T val) const {
	if (type() != JSON_NUMBER) return false;
	Fetch(); 
	return (json_number)val == _value._number;
}

#ifdef JSON_REF_COUNT
	inline void internalJSONNode::decRef(void){
		JSON_ASSERT(refcount != 0, JSON_TEXT("decRef on a 0 refcount internal"));
		--refcount;
	}

	inline bool internalJSONNode::hasNoReferences(void){
		return refcount == 0;
	}
#endif

inline internalJSONNode * internalJSONNode::makeUnique(void){
	#ifdef JSON_REF_COUNT
		if (refcount > 1){
			decRef();
			return newInternal(*this);
		}
		JSON_ASSERT(refcount == 1, JSON_TEXT("makeUnique on a 0 refcount internal"));
		return this;
	#else
		return newInternal(*this);
	#endif
}

#ifndef JSON_PREPARSE
	inline void internalJSONNode::SetFetched(bool val) const {
		fetched = val;
	}
#endif

inline bool internalJSONNode::Fetched(void) const {
	#ifndef JSON_PREPARSE
		return fetched;
	#else
		return true;
	#endif
}

inline JSONNode ** internalJSONNode::begin(void) const {
	Fetch();
	return Children.begin();
}

inline JSONNode ** internalJSONNode::end(void) const {
	Fetch();
	return Children.end();
}

inline JSONNode * internalJSONNode::at(json_index_t pos){
	Fetch();
	return Children[pos];
}

inline void internalJSONNode::reserve(json_index_t size){
	Fetch();
	Children.reserve2(size);
}

/*
	These functions are to allow allocation to be completely controlled by the callbacks
*/

inline void internalJSONNode::deleteInternal(internalJSONNode * ptr){
	#ifdef JSON_MEMORY_CALLBACKS
		ptr -> ~internalJSONNode();
		libjson_free<internalJSONNode>(ptr);
	#else
		delete ptr;
	#endif
}

inline internalJSONNode * internalJSONNode::newInternal(char mytype){
	#ifdef JSON_MEMORY_CALLBACKS
		return new(json_malloc<internalJSONNode>(1)) internalJSONNode(mytype);
	#else
		return new internalJSONNode(mytype);
	#endif
}

inline internalJSONNode * internalJSONNode::newInternal(const json_string & unparsed){
	#ifdef JSON_MEMORY_CALLBACKS
		return new(json_malloc<internalJSONNode>(1)) internalJSONNode(unparsed);
	#else
		return new internalJSONNode(unparsed);
	#endif
}

inline internalJSONNode * internalJSONNode::newInternal(const json_string & name_t, const json_string & value_t){
	#ifdef JSON_MEMORY_CALLBACKS
		return new(json_malloc<internalJSONNode>(1)) internalJSONNode(name_t, value_t);
	#else
		return new internalJSONNode(name_t, value_t);
	#endif
}

inline internalJSONNode * internalJSONNode::newInternal(const internalJSONNode & orig){
	#ifdef JSON_MEMORY_CALLBACKS
		return new(json_malloc<internalJSONNode>(1)) internalJSONNode(orig);
	#else
		return new internalJSONNode(orig);
	#endif
}

#endif
