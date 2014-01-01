/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "JSONChildren.h"
#include "JSONNode.h"

void jsonChildren::inc(void){
	if (mysize == mycapacity){  //it's full
		if (!mycapacity){  //the array hasn't been created yet
			JSON_ASSERT(!array, JSON_TEXT("Expanding a 0 capacity array, but not null"));
			#ifdef JSON_LESS_MEMORY
				array = json_malloc<JSONNode*>(1);
				mycapacity = 1;
			#else
				array = json_malloc<JSONNode*>(8);  //8 seems average for JSON, and it's only 64 bytes
				mycapacity = 8;
			#endif
		} else {
			#ifdef JSON_LESS_MEMORY
				mycapacity += 1;  //increment the size of the array
			#else
				mycapacity <<= 1;  //double the size of the array
			#endif
			array = json_realloc<JSONNode*>(array, mycapacity);
		}
	}
}

void jsonChildren::inc(json_index_t amount){
	if (!amount) return;
	if (mysize + amount >= mycapacity){  //it's full
		if (!mycapacity){  //the array hasn't been created yet
			JSON_ASSERT(!array, JSON_TEXT("Expanding a 0 capacity array, but not null"));
			#ifdef JSON_LESS_MEMORY
				array = json_malloc<JSONNode*>(amount);
				mycapacity = amount;
			#else
				array = json_malloc<JSONNode*>(amount > 8 ? amount : 8);  //8 seems average for JSON, and it's only 64 bytes
				mycapacity = amount > 8 ? amount : 8;
			#endif
		} else {
			#ifdef JSON_LESS_MEMORY
				mycapacity = mysize + amount;  //increment the size of the array
			#else
				while(mysize + amount > mycapacity){
					mycapacity <<= 1;  //double the size of the array
				}
			#endif
			array = json_realloc<JSONNode*>(array, mycapacity);
		}
	}
}

//actually deletes everything within the vector, this is safe to do on an empty or even a null array
void jsonChildren::deleteAll(void){
	json_foreach((*this), runner){
		JSON_ASSERT(*runner, JSON_TEXT("a null pointer within the children"));
		JSONNode::deleteJSONNode(*runner);  //this is why I can't do forward declaration
	}
}

void jsonChildren::doerase(JSONNode ** position, json_index_t number){
	JSON_ASSERT(array, JSON_TEXT("erasing something from a null array 2"));
	JSON_ASSERT(position >= array, JSON_TEXT("position is beneath the start of the array 2"));
	JSON_ASSERT(position + number <= array + mysize, JSON_TEXT("erasing out of bounds 2"));
	if (position + number >= array + mysize){
		mysize = (json_index_t)(position - array);
		#ifndef JSON_ISO_STRICT
			JSON_ASSERT((long long)position - (long long)array >= 0, JSON_TEXT("doing negative allocation"));
		#endif
	} else {
		memmove(position, position + number, (mysize - (position - array) - number) * sizeof(JSONNode *));
		mysize -= number;
	}
}
