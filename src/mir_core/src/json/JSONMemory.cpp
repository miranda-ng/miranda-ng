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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "..\commonheaders.h"

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
			return mymalloc((unsigned long)siz);
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
			return myrealloc(ptr, (unsigned long)siz);
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
