#include "JSONMemory.h"
#include "JSONNode.h"

#ifdef JSON_MEMORY_MANAGE
    void auto_expand::purge(void){
	   for(std::map<void *, void *>::iterator i = mymap.begin(), en = mymap.end(); i != en; ++i){
		  #if defined(JSON_DEBUG) || defined(JSON_SAFE)
			 void * temp = (void*)i -> first;  //because its pass by reference
			 libjson_free<void>(temp);
		  #else
			 libjson_free<void>((void*)i -> first);
		  #endif
	   }
    }

    void auto_expand_node::purge(void){
	   for(std::map<void *, JSONNode *>::iterator i = mymap.begin(), en = mymap.end(); i != en; ++i){
		  JSONNode::deleteJSONNode((JSONNode *)i -> second); 
	   }
    }
#endif

#ifdef JSON_MEMORY_CALLBACKS

json_malloc_t mymalloc = 0;
json_realloc_t myrealloc = 0;
json_free_t myfree = 0;

void * JSONMemory::json_malloc(size_t siz){
    if (mymalloc){
	   #ifdef JSON_DEBUG  //in debug mode, see if the malloc was successful
		  void * result = mymalloc(siz);
		  JSON_ASSERT(result, JSON_TEXT("out of memory"));
		  return result;
	   #else
		  return mymalloc(siz);
	   #endif
    }
    #ifdef JSON_DEBUG  //in debug mode, see if the malloc was successful
	   void * result = malloc(siz);
	   JSON_ASSERT(result, JSON_TEXT("out of memory"));
	   return result;
    #else
	   return malloc(siz);
    #endif
}

void * JSONMemory::json_realloc(void * ptr, size_t siz){
    if (myrealloc){
	   #ifdef JSON_DEBUG  //in debug mode, see if the malloc was successful
		  void * result = myrealloc(ptr, siz);
		  JSON_ASSERT(result, JSON_TEXT("out of memory"));
		  return result;
	   #else
		  return myrealloc(ptr, siz);
	   #endif
    }
    #ifdef JSON_DEBUG  //in debug mode, see if the malloc was successful
	   void * result = realloc(ptr, siz);
	   JSON_ASSERT(result, JSON_TEXT("out of memory"));
	   return result;
    #else
	   return realloc(ptr, siz);
    #endif  
}

void JSONMemory::json_free(void * ptr){
    if (myfree){
	   myfree(ptr);
    } else {
	   free(ptr);
    }
}

void JSONMemory::registerMemoryCallbacks(json_malloc_t mal, json_realloc_t real, json_free_t fre){
    mymalloc = mal;
    myrealloc = real;
    myfree = fre;
}

#endif
