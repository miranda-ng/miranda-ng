#include "internalJSONNode.h"  //internal structure for json value

#ifdef JSON_BINARY
	#include "JSON_Base64.h"
#endif

/*
	Implementations are here to keep the class declaration cleaner.  They can't be placed in a different
	file because they are inlined.
*/
inline JSONNode::JSONNode(char mytype) : internal(internalJSONNode::newInternal(mytype)) {
	JSON_ASSERT((mytype == JSON_NULL) ||
				(mytype == JSON_STRING) ||
				(mytype == JSON_NUMBER) ||
				(mytype == JSON_BOOL) ||
				(mytype == JSON_ARRAY) ||
				(mytype == JSON_NODE), JSON_TEXT("Not a proper JSON type"));
	incAllocCount();
}
inline JSONNode::JSONNode(const json_string & unparsed) :
	internal(internalJSONNode::newInternal(unparsed))
{ //root, specialized because it can only be array or node
	incAllocCount();
}
inline JSONNode::JSONNode(internalJSONNode * internal_t) :
	internal(internal_t)
{ //do not increment anything, this is only used in one case and it's already taken care of 
	incAllocCount();
}
inline JSONNode::JSONNode(const JSONNode & orig): internal(orig.internal -> incRef())
{
	incAllocCount();
}
//this allows a temp node to simply transfer its contents, even with ref counting off
inline JSONNode::JSONNode(bool, JSONNode & orig): internal(orig.internal)
{
	orig.internal = 0;
	incAllocCount();
}
inline JSONNode::~JSONNode(void)
{
	if (internal) decRef();
	decAllocCount();
}
inline json_index_t JSONNode::size(void) const
{
	JSON_CHECK_INTERNAL();
	return internal -> size();
}
inline bool JSONNode::empty(void) const {
	JSON_CHECK_INTERNAL();
	return internal -> empty();
}
inline void JSONNode::clear(void){
	JSON_CHECK_INTERNAL();
	if (!empty()) {
		makeUniqueInternal();
		internal -> Children.clear();
	}
}
inline unsigned char JSONNode::type(void) const
{
	JSON_CHECK_INTERNAL();
	return internal->type();
}
inline bool JSONNode::isnull(void) const
{
	JSON_CHECK_INTERNAL();
	return internal->type() == JSON_NULL;
}
inline JSONNode::operator bool() const
{
	JSON_CHECK_INTERNAL();
	return internal->type() != JSON_NULL;
}
inline const json_char* JSONNode::name(void) const
{
	JSON_CHECK_INTERNAL();
	return internal -> name();
}
inline void JSONNode::set_name(const json_string & newname)
{
	JSON_CHECK_INTERNAL();
	makeUniqueInternal();
	internal -> setname(newname);
}
#ifdef JSON_COMMENTS
	inline void JSONNode::set_comment(const json_string & newname)
	{
		JSON_CHECK_INTERNAL();
		makeUniqueInternal();
		internal -> setcomment(newname);
	}
	inline json_string JSONNode::get_comment(void) const { 
		JSON_CHECK_INTERNAL();
		return internal -> getcomment();
	}
#endif
inline json_string JSONNode::as_string(void) const
{
	JSON_CHECK_INTERNAL();
	return internal -> as_string();
}
#if defined(M_STRING_H__)
inline CMString JSONNode::as_mstring(void) const
{
	JSON_CHECK_INTERNAL();
	return internal->as_mstring();
}
#endif
inline long JSONNode::as_int(void) const
{
	JSON_CHECK_INTERNAL();
	return internal -> as_int();
}
inline double JSONNode::as_float(void) const {
	JSON_CHECK_INTERNAL();
	return internal -> as_float();
}
inline bool JSONNode::as_bool(void) const {
	JSON_CHECK_INTERNAL();
	return internal -> as_bool();
}
#ifdef JSON_BINARY
	inline void JSONNode::set_binary(const unsigned char * bin, json_index_t bytes){
		JSON_CHECK_INTERNAL();
		*this = JSONBase64::json_encode64(bin, bytes);
	}
	inline std::string JSONNode::as_binary(void) const {
		JSON_ASSERT_SAFE(type() == JSON_STRING, JSON_TEXT("using as_binary for a non-string type"), return EMPTY_STRING2;);
		JSON_CHECK_INTERNAL();
		return JSONBase64::json_decode64(as_string());
	}
#endif
inline JSONNode & JSONNode::operator[](const json_char *name_t)
{
	return at(name_t);
}
inline const JSONNode & JSONNode::operator[](const json_char *name_t) const
{
	return at(name_t);
}
#ifdef JSON_LIBRARY
inline void JSONNode::push_back(JSONNode * child){
#else
inline void JSONNode::push_back(const JSONNode & child){
#endif
	JSON_CHECK_INTERNAL();
	makeUniqueInternal();
	internal -> push_back(child);
}

inline void JSONNode::reserve(json_index_t size){
	makeUniqueInternal();
	internal -> reserve(size);
}
inline JSONNode & JSONNode::operator = (const JSONNode & orig){
	JSON_CHECK_INTERNAL();
	#ifdef JSON_REF_COUNT
		if (internal == orig.internal) return *this;  //don't want it accidentally deleting itself
	#endif
	decRef();  //dereference my current one
	internal = orig.internal -> incRef();  //increase reference of original
	return *this;
}
#ifndef JSON_LIBRARY
	inline JSONNode & JSONNode::operator = (const json_char * val){
		JSON_CHECK_INTERNAL();
		*this = json_string(val);
		return *this;
	}
#endif

#define NODE_SET_TYPED(type)\
	inline JSONNode & JSONNode::operator = (type val){\
		JSON_CHECK_INTERNAL();\
		makeUniqueInternal();\
		internal -> Set(val);\
		return *this;\
	}
IMPLEMENT_FOR_ALL_TYPES(NODE_SET_TYPED)

/*
	This section is the equality operators
*/
#define NODE_CHECK_EQUALITY(type)\
	inline bool JSONNode::operator == (type val) const {\
		JSON_CHECK_INTERNAL();\
		return internal -> IsEqualToNum<type>(val);\
	}
IMPLEMENT_FOR_ALL_NUMBERS(NODE_CHECK_EQUALITY)
inline bool JSONNode::operator == (const json_string & val) const {
	JSON_CHECK_INTERNAL();
	return internal -> IsEqualTo(val);
}
#ifndef JSON_LIBRARY
	inline bool JSONNode::operator == (const json_char * val) const {
		JSON_CHECK_INTERNAL();
		return *this == json_string(val);
	}
#endif
inline bool JSONNode::operator == (bool val) const {
	JSON_CHECK_INTERNAL();
	return internal -> IsEqualTo(val);
}
inline bool JSONNode::operator == (const JSONNode & val) const {
	JSON_CHECK_INTERNAL();
	return internal -> IsEqualTo(val.internal);
}

/*
	This section is the inequality operators
*/

#define NODE_CHECK_INEQUALITY(type)\
	inline bool JSONNode::operator != (type val) const {\
		JSON_CHECK_INTERNAL();\
		return !(*this == val);\
	}
IMPLEMENT_FOR_ALL_TYPES(NODE_CHECK_INEQUALITY)
NODE_CHECK_INEQUALITY(const JSONNode &)
#ifndef JSON_LIBRARY
	NODE_CHECK_INEQUALITY(const json_char * )
#endif

inline void JSONNode::nullify(void){
	JSON_CHECK_INTERNAL();
	makeUniqueInternal();
	internal -> Nullify();
}
inline void JSONNode::swap(JSONNode & other){
	JSON_CHECK_INTERNAL();
	internalJSONNode * temp = other.internal;
	other.internal = internal;
	internal = temp;
	JSON_CHECK_INTERNAL();
}
inline void JSONNode::decRef(void){ //decrements internal's counter, deletes it if needed
	JSON_CHECK_INTERNAL();
	#ifdef JSON_REF_COUNT
		internal -> decRef();
		if (internal -> hasNoReferences()) {
			internalJSONNode::deleteInternal(internal);
		}
	#else
		internalJSONNode::deleteInternal(internal);
	#endif
}
#ifdef JSON_REF_COUNT
	inline void JSONNode::makeUniqueInternal() { //makes internal it's own
		JSON_CHECK_INTERNAL();
		internal = internal -> makeUnique();  //might return itself or a new one that's exactly the same
	}
#endif
#ifdef JSON_ITERATORS
	inline JSONNode::json_iterator JSONNode::begin(void){
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("iterating a non-iteratable node"));
		makeUniqueInternal();
		return json_iterator(internal -> begin());
	}
	inline JSONNode::json_iterator JSONNode::end(void){
		JSON_CHECK_INTERNAL();
		JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("iterating a non-iteratable node"));
		makeUniqueInternal();
		return json_iterator(internal -> end());
	}

	#ifndef JSON_LIBRARY
		inline JSONNode::const_iterator JSONNode::begin(void) const {
			JSON_CHECK_INTERNAL();
			JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("iterating a non-iteratable node"));
			return JSONNode::const_iterator(internal -> begin());
		}
		inline JSONNode::const_iterator JSONNode::end(void) const {
			JSON_CHECK_INTERNAL();
			JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("iterating a non-iteratable node"));
			return JSONNode::const_iterator(internal -> end());
		}
		inline JSONNode::reverse_iterator JSONNode::rbegin(void){
			JSON_CHECK_INTERNAL();
			JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("iterating a non-iteratable node"));
			makeUniqueInternal();
			return JSONNode::reverse_iterator(internal -> end() - 1);
		}
		inline JSONNode::reverse_iterator JSONNode::rend(void){
			JSON_CHECK_INTERNAL();
			JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("iterating a non-iteratable node"));
			makeUniqueInternal();
			return JSONNode::reverse_iterator(internal -> begin() - 1);
		}
		inline JSONNode::reverse_const_iterator JSONNode::rbegin(void) const {
			JSON_CHECK_INTERNAL();
			JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("iterating a non-iteratable node"));
			return JSONNode::reverse_const_iterator(internal -> end() - 1);
		}
		inline JSONNode::reverse_const_iterator JSONNode::rend(void) const {
			JSON_CHECK_INTERNAL();
			JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("iterating a non-iteratable node"));
			return JSONNode::reverse_const_iterator(internal -> begin() - 1);
		}

		inline JSONNode::iterator JSONNode::insert(json_iterator pos, const const_iterator & _start, const const_iterator & _end){
			return insertFFF(pos, _start.it, _end.it);
		}

		inline JSONNode::reverse_iterator JSONNode::insert(reverse_iterator pos, const const_iterator & _start, const const_iterator & _end){
			return insertRFF(pos, _start.it, _end.it);
		}

		inline JSONNode::reverse_iterator JSONNode::insert(reverse_iterator pos, const iterator & _start, const iterator & _end){
			return insertRFF(pos, _start.it, _end.it);
		}

		inline JSONNode::reverse_iterator JSONNode::insert(reverse_iterator pos, const reverse_const_iterator & _start, const reverse_const_iterator & _end){
			return insertRRR(pos, _start.it, _end.it);
		}

		inline JSONNode::reverse_iterator JSONNode::insert(reverse_iterator pos, const reverse_iterator & _start, const reverse_iterator & _end){
			return insertRRR(pos, _start.it, _end.it);
		}

		inline JSONNode::iterator JSONNode::insert(json_iterator pos, const reverse_const_iterator & _start, const reverse_const_iterator & _end){
			return insertFRR(pos, _start.it, _end.it); 
		}

		inline JSONNode::iterator JSONNode::insert(iterator pos, const reverse_iterator & _start, const reverse_iterator & _end){
			return insertFRR(pos, _start.it, _end.it); 
		}
	#endif

	inline JSONNode::json_iterator JSONNode::insert(json_iterator pos, const json_iterator & _start, const json_iterator & _end){
		return insertFFF(pos, json_iterator_ptr(_start), json_iterator_ptr(_end));
	}
#endif
#ifdef JSON_WRITER
	inline json_string JSONNode::write(void){
		JSON_CHECK_INTERNAL();
		JSON_ASSERT_SAFE(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("Writing a non-writable node"), return JSON_TEXT(""););
		return internal -> Write(0xFFFFFFFF, true);
	}
	inline json_string JSONNode::write_formatted(void){
		JSON_CHECK_INTERNAL();
		JSON_ASSERT_SAFE(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("Writing a non-writable node"), return JSON_TEXT(""););
		return internal -> Write(0, true);
	}
#endif
#ifndef JSON_PREPARSE
	inline void JSONNode::preparse(void){
		JSON_CHECK_INTERNAL();
		internal -> preparse();
	}
#endif
#ifdef JSON_VALIDATE
	inline bool JSONNode::validate(void){
		JSON_CHECK_INTERNAL();
		if (type() == JSON_NULL) return false;
		JSON_ASSERT_SAFE(type() == JSON_NODE || type() == JSON_ARRAY, JSON_TEXT("Validating non root node"), return false;);
		#ifndef JSON_PREPARSE
			internal -> Fetch();  //will nullify it if it's bad
		#endif
		if (type() == JSON_NULL) return false;
		return internal -> validate();
	}
#endif
#ifdef JSON_DEBUG
	#ifndef JSON_LIBRARY
		inline JSONNode JSONNode::dump(void) const {
			JSON_CHECK_INTERNAL();
			JSONNode dumpage(JSON_NODE);
			dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("this"), (long)this)));
			size_t total = 0;
			JSONNode node = internal -> Dump(total);
			dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("total bytes used"), total)));
			dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("bytes used"), sizeof(JSONNode))));
			dumpage.push_back(JSON_NEW(node));
			return dumpage;
		}
		inline JSONNode JSONNode::dump(size_t & totalmemory){
			JSON_CHECK_INTERNAL();
			JSONNode dumpage(JSON_NODE);
			dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("this"), (long)this)));
			dumpage.push_back(JSON_NEW(JSONNode(JSON_TEXT("bytes used"), sizeof(JSONNode))));
			dumpage.push_back(JSON_NEW(internal -> Dump(totalmemory)));
			return dumpage;
		}
	#endif
#endif

inline void JSONNode::deleteJSONNode(JSONNode * ptr){
	#ifdef JSON_MEMORY_CALLBACKS
		ptr -> ~JSONNode();
		libjson_free<JSONNode>(ptr);
	#else
		delete ptr;
	#endif	
}
inline JSONNode * _newJSONNode(const JSONNode & orig){
	#ifdef JSON_MEMORY_CALLBACKS
		return new(json_malloc<JSONNode>(1)) JSONNode(orig);
	#else
		return new JSONNode(orig);
	#endif
}
inline JSONNode * JSONNode::newJSONNode(const JSONNode & orig	JSON_MUTEX_COPY_DECL){
	#ifdef JSON_MUTEX_CALLBACKS
		if (parentMutex){
			JSONNode * temp = _newJSONNode(orig);
			temp -> set_mutex(parentMutex);
			return temp;
		}
	#endif
	return _newJSONNode(orig);
}
inline JSONNode * JSONNode::newJSONNode(internalJSONNode * internal_t){
	#ifdef JSON_MEMORY_CALLBACKS
		return new(json_malloc<JSONNode>(1)) JSONNode(internal_t);
	#else
		return new JSONNode(internal_t);
	#endif 
}

inline JSONNode * JSONNode::newJSONNode_Shallow(const JSONNode & orig){
	#ifdef JSON_MEMORY_CALLBACKS
		return new(json_malloc<JSONNode>(1)) JSONNode(true, const_cast<JSONNode &>(orig));
	#else
		return new JSONNode(true, const_cast<JSONNode &>(orig));
	#endif
}
