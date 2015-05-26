#ifndef JSONNODE_H
#define JSONNODE_H

#include "JSONDefs.h"   //for string type
#include "JSONDebug.h"

#ifdef JSON_BINARY
	#include "JSON_Base64.h"
#endif

#ifndef JSON_REF_COUNT
	#define makeUniqueInternal() (void)0
#endif

#define JSON_CHECK_INTERNAL() JSON_ASSERT(internal, JSON_TEXT("no internal"))

#ifdef JSON_MUTEX_CALLBACKS
	#define JSON_MUTEX_COPY_DECL ,void * parentMutex
	#define JSON_MUTEX_COPY_DECL2 ,void * parentMutex = 0
#else
	#define JSON_MUTEX_COPY_DECL
	#define JSON_MUTEX_COPY_DECL2
#endif

#ifdef JSON_LIBRARY
	#define JSON_PTR_LIB *
	#define JSON_NEW(x) JSONNode::newJSONNode_Shallow(x)
	#define DECLARE_FOR_ALL_TYPES(foo)\
		foo(long);\
		foo(double);\
		foo(bool);\
		foo(const json_string &);
	#define DECLARE_FOR_ALL_TYPES_CONST(foo)\
		foo(long) const;\
		foo(double) const;\
		foo(bool) const;\
		foo(const json_string &) const;\
		foo(const JSONNode &) const;
	#define IMPLEMENT_FOR_ALL_NUMBERS(foo)\
		foo(long)\
		foo(double)

#else
	#define JSON_PTR_LIB
	#define JSON_NEW(x) x
	#define DECLARE_FOR_ALL_TYPES(foo)\
		foo(char); foo(unsigned char);\
		foo(short); foo(unsigned short);\
		foo(int); foo(unsigned int);\
		foo(long); foo(unsigned long);\
		foo(float); foo(double);\
		foo(bool);\
		foo(const json_string &);\
		foo(const json_char *);
	#define DECLARE_FOR_ALL_TYPES_CONST(foo)\
		foo(char) const; foo(unsigned char) const;\
		foo(short) const; foo(unsigned short) const;\
		foo(int) const; foo(unsigned int) const;\
		foo(long) const; foo(unsigned long) const;\
		foo(float) const; foo(double) const;\
		foo(bool) const;\
		foo(const json_string &) const;\
		foo(const JSONNode &) const;\
		foo(const json_char *) const;
	#define IMPLEMENT_FOR_ALL_NUMBERS(foo)\
		foo(char) foo(unsigned char)\
		foo(short) foo(unsigned short)\
		foo(int) foo(unsigned int)\
		foo(long) foo(unsigned long)\
		foo(float) foo(double)
#endif
#define IMPLEMENT_FOR_ALL_TYPES(foo)\
	IMPLEMENT_FOR_ALL_NUMBERS(foo)\
	foo(const json_string &)\
	foo(bool)
/*
	This class is mostly just a wrapper class around internalJSONNode, this class keeps
	the reference count and handles copy on write and such.  This class is also responsible
	for argument checking and throwing exceptions if needed.
*/

class MIR_CORE_EXPORT JSONNode
{
	friend class JSONWorker;
	friend class internalJSONNode;

public:
	explicit JSONNode(char mytype = JSON_NODE);
	#define DECLARE_CTOR(type) JSONNode(const json_string & name_t, type value_t)
	DECLARE_FOR_ALL_TYPES(DECLARE_CTOR)

	JSONNode(const JSONNode & orig);
	~JSONNode(void);

	static JSONNode parse(const json_char *str);

	json_index_t size(void) const;
	bool empty(void) const;
	bool isnull(void) const;
	void clear(void);
	unsigned char type(void) const;

	const json_char* name(void) const;
	void set_name(const json_string & newname);
	#ifdef JSON_COMMENTS
		void set_comment(const json_string & comment);
		json_string get_comment(void) const;
	#endif
	#ifndef JSON_PREPARSE
		void preparse(void);
	#endif
	#ifdef JSON_VALIDATE
		#ifndef JSON_SAFE
			#error JSON_VALIDATE also requires JSON_SAFE
		#endif
		bool validate(void);
	#endif

	json_string as_string(void) const;
#if defined(M_STRING_H__)
	CMString as_mstring(void) const;
#endif
	long as_int(void) const;
	double as_float(void) const;
	bool as_bool(void) const;
	JSONNode as_node(void) const;
	JSONNode as_array(void) const;

	#ifdef JSON_BINARY
		std::string as_binary(void) const;
		void set_binary(const unsigned char * bin, json_index_t bytes);
	#endif

	JSONNode & at(json_index_t pos);
	const JSONNode & at(json_index_t pos) const;
	JSONNode & operator[](json_index_t pos);
	const JSONNode & operator[](json_index_t pos) const;

	JSONNode & at(const json_char *name_t);
	const JSONNode & at(const json_char *name_t) const;
	#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
		JSONNode & at_nocase(const json_string & name_t);
		const JSONNode & at_nocase(const json_string & name_t) const;
	#endif
	operator bool() const;
	JSONNode & operator[](const json_char *name_t);
	const JSONNode & operator[](const json_char *name_t) const;
	#ifdef JSON_LIBRARY
		void push_back(JSONNode *node);
	#else
		void push_back(const JSONNode & node);
	#endif
	void reserve(json_index_t size);
	JSONNode JSON_PTR_LIB pop_back(json_index_t pos);
	JSONNode JSON_PTR_LIB pop_back(const json_string & name_t);
	#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
		JSONNode JSON_PTR_LIB pop_back_nocase(const json_string & name_t);
	#endif

	DECLARE_FOR_ALL_TYPES(JSONNode & operator = )
	JSONNode & operator = (const JSONNode &);
	DECLARE_FOR_ALL_TYPES_CONST(bool operator ==)
	DECLARE_FOR_ALL_TYPES_CONST(bool operator !=)

	void nullify(void);
	void swap(JSONNode & other);
	void merge(JSONNode & other);
	void merge(unsigned int num, ...);
	JSONNode duplicate(void) const;
	void cast(char newtype);

	//iterator
	#ifdef JSON_ITERATORS
		#ifndef JSON_LIBRARY
			#define json_iterator_ptr(iter) iter.it
			#define ptr_to_json_iterator(iter) json_iterator(iter)
			struct iterator {
				inline iterator& operator ++(void){ ++it; return *this; }
				inline iterator& operator --(void){ --it; return *this; }
				inline iterator& operator +=(long i){ it += i; return *this; }
				inline iterator& operator -=(long i){ it -= i; return *this; }
				inline iterator operator ++(int){
					iterator result(*this);
					++it;
					return result;
				}
				inline iterator operator --(int){
					iterator result(*this);
					--it;
					return result;
				}
				inline iterator operator +(long i) const {
					iterator result(*this);
					result.it += i;
					return result;
				}
				inline iterator operator -(long i) const {
					iterator result(*this);
					result.it -= i;
					return result;
				}
				inline JSONNode& operator [](size_t pos) const { return *it[pos]; };
				inline JSONNode& operator *(void) const { return *(*it); }
				inline bool operator == (const iterator & other) const { return it == other.it; }
				inline bool operator != (const iterator & other) const { return it != other.it; }
				inline bool operator > (const iterator & other) const { return it > other.it; }
				inline bool operator >= (const iterator & other) const { return it >= other.it; }
				inline bool operator < (const iterator & other) const { return it < other.it; }
				inline bool operator <= (const iterator & other) const { return it <= other.it; }
				inline iterator & operator = (const iterator & orig){ it = orig.it; return *this; }
				iterator (const iterator & orig) : it(orig.it) {}
			private:
				JSONNode ** it;
				iterator(JSONNode ** starter) : it(starter) {}
				friend class JSONNode;
			};
			typedef iterator json_iterator;

			struct const_iterator {
				inline const_iterator& operator ++(void){ ++it; return *this; }
				inline const_iterator& operator --(void){ --it; return *this; }
				inline const_iterator& operator +=(size_t i) { it += i; return *this; }
				inline const_iterator& operator -=(size_t i) { it -= i; return *this; }
				inline const_iterator operator ++(int){
					const_iterator result(*this);
					++it;
					return result;
				}
				inline const_iterator operator --(int){
					const_iterator result(*this);
					--it;
					return result;
				}
				inline const_iterator operator +(size_t i) const {
					const_iterator result(*this);
					result.it += i;
					return result;
				}
				inline const_iterator operator -(size_t i) const {
					const_iterator result(*this);
					result.it -= i;
					return result;
				}
				inline const JSONNode& operator [](size_t pos) const { return const_cast<const JSONNode&>(*it[pos]); };
				inline const JSONNode& operator *(void) const { return const_cast<const JSONNode&>(*(*it)); }
				inline bool operator == (const const_iterator & other) const { return it == other.it; }
				inline bool operator != (const const_iterator & other) const { return it != other.it; }
				inline bool operator > (const const_iterator & other) const { return it > other.it; }
				inline bool operator >= (const const_iterator & other) const { return it >= other.it; }
				inline bool operator < (const const_iterator & other) const { return it < other.it; }
				inline bool operator <= (const const_iterator & other) const { return it <= other.it; }
				inline const_iterator & operator = (const const_iterator & orig){ it = orig.it; return *this; }
				const_iterator (const const_iterator & orig) : it(orig.it) {}
			private:
				JSONNode ** it;
				const_iterator(JSONNode ** starter) : it(starter) {}
				friend class JSONNode;
			};
			const_iterator begin(void) const;
			const_iterator end(void) const;

			struct reverse_iterator {
				inline reverse_iterator& operator ++(void){ --it; return *this; }
				inline reverse_iterator& operator --(void){ ++it; return *this; }
				inline reverse_iterator& operator +=(size_t i){ it -= i; return *this; }
				inline reverse_iterator& operator -=(size_t i) { it += i; return *this; }
				inline reverse_iterator operator ++(int){
					reverse_iterator result(*this);
					--it;
					return result;
				}
				inline reverse_iterator operator --(int){
					reverse_iterator result(*this);
					++it;
					return result;
				}
				inline reverse_iterator operator +(size_t i) const {
					reverse_iterator result(*this);
					result.it -= i;
					return result;
				}
				inline reverse_iterator operator -(size_t i) const {
					reverse_iterator result(*this);
					result.it += i;
					return result;
				}
				inline JSONNode& operator [](size_t pos) const { return *it[pos]; };
				inline JSONNode& operator *(void) const { return *(*it); }
				inline bool operator == (const reverse_iterator & other) const { return it == other.it; }
				inline bool operator != (const reverse_iterator & other) const { return it != other.it; }
				inline bool operator < (const reverse_iterator & other) const { return it > other.it; }
				inline bool operator <= (const reverse_iterator & other) const { return it >= other.it; }
				inline bool operator > (const reverse_iterator & other) const { return it < other.it; }
				inline bool operator >= (const reverse_iterator & other) const { return it <= other.it; }
				inline reverse_iterator & operator = (const reverse_iterator & orig){ it = orig.it; return *this; }
				reverse_iterator (const reverse_iterator & orig) : it(orig.it) {}
			private:
				JSONNode ** it;
				reverse_iterator(JSONNode ** starter) : it(starter) {}
				friend class JSONNode;
			};
			reverse_iterator rbegin(void);
			reverse_iterator rend(void);

			struct reverse_const_iterator {
				inline reverse_const_iterator& operator ++(void){ --it; return *this; }
				inline reverse_const_iterator& operator --(void){ ++it; return *this; }
				inline reverse_const_iterator& operator +=(size_t i){ it -= i; return *this; }
				inline reverse_const_iterator& operator -=(size_t i){ it += i; return *this; }
				inline reverse_const_iterator operator ++(int){
					reverse_const_iterator result(*this);
					--it;
					return result;
				}
				inline reverse_const_iterator operator --(int){
					reverse_const_iterator result(*this);
					++it;
					return result;
				}
				inline reverse_const_iterator operator +(size_t i) const {
					reverse_const_iterator result(*this);
					result.it -= i;
					return result;
				}
				inline reverse_const_iterator operator -(size_t i) const {
					reverse_const_iterator result(*this);
					result.it += i;
					return result;
				}
				inline const JSONNode& operator [](size_t pos) const { return const_cast<const JSONNode&>(*it[pos]); };
				inline const JSONNode& operator *(void) const { return const_cast<const JSONNode&>(*(*it)); }
				inline bool operator == (const reverse_const_iterator & other) const { return it == other.it; }
				inline bool operator != (const reverse_const_iterator & other) const { return it != other.it; }
				inline bool operator < (const reverse_const_iterator & other) const { return it > other.it; }
				inline bool operator <= (const reverse_const_iterator & other) const { return it >= other.it; }
				inline bool operator > (const reverse_const_iterator & other) const { return it < other.it; }
				inline bool operator >= (const reverse_const_iterator & other) const { return it <= other.it; }
				inline reverse_const_iterator & operator = (const reverse_const_iterator & orig){ it = orig.it; return *this; }
				reverse_const_iterator (const reverse_const_iterator & orig) : it(orig.it) {}
			private:
				JSONNode ** it;
				reverse_const_iterator(JSONNode ** starter) : it(starter) {}
				friend class JSONNode;
			};
			reverse_const_iterator rbegin(void) const;
			reverse_const_iterator rend(void) const;

			const_iterator find(const json_string & name_t) const;
			#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
				const_iterator find_nocase(const json_string & name_t) const;
			#endif

			reverse_iterator erase(reverse_iterator pos);
			reverse_iterator erase(reverse_iterator start, const reverse_iterator & end);

			iterator insert(iterator pos, const JSONNode & x);
			reverse_iterator insert(reverse_iterator pos, const JSONNode & x);
			iterator insert(iterator pos, const reverse_iterator & _start, const reverse_iterator & _end);
			reverse_iterator insert(reverse_iterator pos, const iterator & _start, const iterator & _end);
			reverse_iterator insert(reverse_iterator pos, const reverse_iterator & _start, const reverse_iterator & _end);

			json_iterator insert(json_iterator pos, const const_iterator & _start, const const_iterator & _end);
			reverse_iterator insert(reverse_iterator pos, const const_iterator & _start, const const_iterator & _end);
			json_iterator insert(json_iterator pos, const reverse_const_iterator & _start, const reverse_const_iterator & _end);
			reverse_iterator insert(reverse_iterator pos, const reverse_const_iterator & _start, const reverse_const_iterator & _end);
		#else
			typedef JSONNode** json_iterator;
			#define json_iterator_ptr(iter) iter
			#define ptr_to_json_iterator(iter) iter
			json_iterator insert(json_iterator pos, JSONNode * x);
		#endif

		json_iterator begin(void);
		json_iterator end(void);

		json_iterator find(const json_string & name_t);
		#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
			json_iterator find_nocase(const json_string & name_t);
		#endif
		json_iterator erase(json_iterator pos);
		json_iterator erase(json_iterator start, const json_iterator & end);
		json_iterator insert(json_iterator pos, const json_iterator & _start, const json_iterator & _end);
	#endif

	#ifdef JSON_MUTEX_CALLBACKS
		static void register_mutex_callbacks(json_mutex_callback_t lock, json_mutex_callback_t unlock, void * manager_lock);
		#ifdef JSON_MUTEX_MANAGE
			static void register_mutex_destructor(json_mutex_callback_t destroy);
		#endif
		static void set_global_mutex(void * mutex);
		void set_mutex(void * mutex);
		void lock(int thread);
		void unlock(int thread);
		struct auto_lock {
			public:
				auto_lock(JSONNode & node, int thread) : mynode(&node), mythread(thread){
					mynode -> lock(mythread);
				}
				auto_lock(JSONNode *node, int thread) : mynode(node), mythread(thread){
					mynode -> lock(mythread);
				}
				~auto_lock(void){
					mynode -> unlock(mythread);
				}
			private:
				auto_lock & operator = (const auto_lock &);
				auto_lock(const auto_lock &);
				JSONNode * mynode;
				int mythread;
		};
		static void * getThisLock(JSONNode * pthis);
	#endif

	#ifdef JSON_UNIT_TEST
		static int getNodeAllocationCount(void);
		static int getNodeDeallocationCount(void);
		static int getInternalAllocationCount(void);
		static int getInternalDeallocationCount(void);
		static void incAllocCount(void);
		static void decAllocCount(void);
		static void incinternalAllocCount(void);
		static void decinternalAllocCount(void);
	#endif

	#ifdef JSON_WRITER
		json_string write(void);
		json_string write_formatted(void);
	#endif

	#ifdef JSON_DEBUG
		#ifndef JSON_LIBRARY
			JSONNode dump(void) const;
		#endif
	#endif
	static void deleteJSONNode(JSONNode * ptr);
	static JSONNode * newJSONNode_Shallow(const JSONNode & orig);
JSON_PRIVATE
	static JSONNode * newJSONNode(const JSONNode & orig		JSON_MUTEX_COPY_DECL2);
	static JSONNode * newJSONNode(internalJSONNode * internal_t);
	//used by JSONWorker
	JSONNode(const json_string & unparsed);
	JSONNode(internalJSONNode * internal_t);
	JSONNode(bool, JSONNode & orig);

	void decRef(void);  //decrements internal's counter, deletes it if needed
	#ifdef JSON_REF_COUNT
		void makeUniqueInternal(void); //makes internal it's own
		void merge(JSONNode * other);
	#endif

	#ifdef JSON_DEBUG
		#ifndef JSON_LIBRARY
			JSONNode dump(size_t & totalmemory);
		#endif
	#endif

	#ifdef JSON_ITERATORS
		#ifndef JSON_LIBRARY
			json_iterator insertFRR(json_iterator pos, JSONNode ** const _start, JSONNode ** const _end);
			reverse_iterator insertRRR(reverse_iterator pos, JSONNode ** const _start, JSONNode ** const _end);
			reverse_iterator insertRFF(reverse_iterator pos, JSONNode ** const _start, JSONNode ** const _end);
		#endif
		json_iterator insertFFF(json_iterator pos, JSONNode ** const _start, JSONNode ** const _end);
	#endif

	mutable internalJSONNode * internal;
};

#endif
