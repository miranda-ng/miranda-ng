#ifndef JSON_MEMORY_H
#define JSON_MEMORY_H

#include <cstdlib> //for malloc, realloc, and free
#include <cstring> //for memmove
#include "JSONOptions.h"
#include "JSONDebug.h"

#if defined(JSON_DEBUG) || defined(JSON_SAFE)
	#define JSON_FREE_PASSTYPE &
#else
	#define JSON_FREE_PASSTYPE
#endif

#ifdef JSON_MEMORY_CALLBACKS
	class JSONMemory {
	public:
		static void * json_malloc(size_t siz);
		static void * json_realloc(void * ptr, size_t siz);
		static void json_free(void * ptr);
		static void registerMemoryCallbacks(json_malloc_t mal, json_realloc_t real, json_free_t fre);
	};

	template <typename T> static inline T * json_malloc(size_t count){
		return (T *)JSONMemory::json_malloc(sizeof(T) * count);
	}

	template <typename T> static inline T * json_realloc(T * ptr, size_t count){
		return (T *)JSONMemory::json_realloc(ptr, sizeof(T) * count);
	}

	template <typename T> static inline void libjson_free(T * JSON_FREE_PASSTYPE ptr){
		JSONMemory::json_free(ptr);
		#if defined(JSON_DEBUG) || defined(JSON_SAFE)  //in debug or safe mode, set the pointer to 0 so that it can't be used again
			ptr = 0;
		#endif
	}
#else
	template <typename T>
	static inline T * json_malloc(size_t count){
		#ifdef JSON_DEBUG  //in debug mode, see if the malloc was successful
			void * result = malloc(count * sizeof(T));
			JSON_ASSERT(result, JSON_TEXT("out of memory"));
			#ifdef JSON_NULL_MEMORY
				memset(result, '\0', count  * sizeof(T));
			#endif
			return (T *)result;
		#else
			return (T *)malloc(count * sizeof(T));
		#endif
	}

	template <typename T>
	static inline void libjson_free(T * JSON_FREE_PASSTYPE ptr){
		free(ptr);
		#if defined(JSON_DEBUG) || defined(JSON_SAFE)  //in debug or safe mode, set the pointer to 0 so that it can't be used again
			ptr = 0;
		#endif
	}

	template <typename T>
	static inline T * json_realloc(T * ptr, size_t count){
		#ifdef JSON_DEBUG  //in debug mode, check the results of realloc to be sure it was successful
			void * result = realloc(ptr, count * sizeof(T));
			JSON_ASSERT(result, JSON_TEXT("out of memory"));
			#ifdef JSON_NULL_MEMORY
				memset(result, '\0', count  * sizeof(T));
			#endif
			return (T *)result;
		#else
			return (T *)realloc(ptr, count * sizeof(T));
		#endif
	}
#endif

#ifdef JSON_MEMORY_MANAGE
	#include <map>
	class JSONNode;
	struct auto_expand {
		auto_expand(void) : mymap() {}
		~auto_expand(void){ purge(); }
		void purge(void);
		inline void clear(void){ purge(); mymap.clear(); }
		inline void * insert(void * ptr){ mymap[ptr] = ptr; return ptr; }
		inline void remove(void * ptr){ 
			std::map<void *, void *>::iterator i = mymap.find(ptr);
			JSON_ASSERT(i != mymap.end(), JSON_TEXT("Removing a non-managed item"));
			mymap.erase(i);
		}
		std::map<void *, void *> mymap;
	};

	struct auto_expand_node {
		auto_expand_node(void) : mymap() {}
		~auto_expand_node(void){ purge(); }
		void purge(void);
		inline void clear(void){ purge(); mymap.clear(); }
		inline JSONNode * insert(JSONNode * ptr){ mymap[ptr] = ptr; return ptr; }
		inline void remove(void * ptr){ 
			std::map<void *, JSONNode *>::iterator i = mymap.find(ptr);
			if(i != mymap.end()) mymap.erase(i);
		}
		std::map<void *, JSONNode *> mymap;
	};
#endif

//The C++ way, use an self-deleting pointer and let the optimizer decide when it gets destroyed
template <typename T>
class json_auto {
	public:
		json_auto(void) : ptr(0){}
		json_auto(size_t count) : ptr(json_malloc<T>(count)) {}
		~json_auto(void){
			libjson_free<T>(ptr);
		}
		void set(T * p){
			ptr = p; 
		}
		T * ptr;
	private:
		json_auto(const json_auto &);
		json_auto & operator = (const json_auto &);
};

//Clears a string, if required, frees the memory
static inline void clearString(json_string & str){
	#ifdef JSON_LESS_MEMORY
		json_string().swap(str);
	#else
		str.clear();
	#endif
}

#endif
