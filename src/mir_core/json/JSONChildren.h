#ifndef JSONCHILDREN_H
#define JSONCHILDREN_H

#include "JSONMemory.h"
#include "JSONDebug.h"  //for JSON_ASSERT macro

#define json_foreach(children, iterator)\
	JSONNode ** iterator = children.begin();\
	for(JSONNode ** iterator##_end = children.end(); iterator != iterator##_end; ++iterator)

/*
 This class is essentially a vector that has been heavily optimized for the specific purpose
 of holding JSONNode children.  It acts the same way as a vector, it has a automatically
 expanding array.  On destruction, this container automatically destroys everything contained
 in it as well, so that you libJSON doesn't have to do that.
 
 T is JSONNode*, I can't define it that way directly because JSONNode uses this container, and because
 the container deletes the children automatically, forward declaration can't be used
 */

class JSONNode;  //forward declaration

class jsonChildren {
public:
	//starts completely empty and the array is not allocated
	jsonChildren(void) : array(0), mysize(0), mycapacity(0) { }

	//deletes the array and everything that is contained within it (using delete)
	~jsonChildren(void){
		if (array){  //the following function calls are safe, but take more time than a check here
			deleteAll();
			libjson_free<JSONNode*>(array);
		}
	}

	//increase the size of the array
	void inc(json_index_t amount);
	void inc(void);

	//Adds something to the vector, doubling the array if necessary
	void push_back(JSONNode * item){
		inc();
		array[mysize++] = item;
	}

	//Adds something to the front of the vector, doubling the array if necessary
	void push_front(JSONNode * item){
		inc();
		memmove(array + 1, array, mysize++ * sizeof(JSONNode *));
		array[0] = item;
	}

	//gets an item out of the vector by it's position
	inline JSONNode * operator[] (json_index_t position) const {
		JSON_ASSERT(position < mysize, JSON_TEXT("Using [] out of bounds"));
		JSON_ASSERT(position < mycapacity, JSON_TEXT("Using [] out of bounds"));
		JSON_ASSERT(array, JSON_TEXT("Array is null"));
		return array[position];
	}

	//returns the allocated capacity, but keep in mind that some might not be valid
	inline json_index_t capacity() const {
		return mycapacity;
	}

	//returns the number of valid objects within the vector
	inline json_index_t size() const {
		return mysize;
	}

	//tests whether or not the vector is empty
	inline bool empty() const {
		return mysize == 0;
	}

	//clears (and deletes) everything from the vector and sets it's size to 0
	inline void clear() {
		if (array){  //don't bother clearing anything if there is nothing in it
			JSON_ASSERT(mycapacity != 0, JSON_TEXT("mycapacity is not zero, but array is null"));
			deleteAll();
			mysize = 0;
		}
		JSON_ASSERT(mysize == 0, JSON_TEXT("mysize is not zero after clear"));
	}

	//returns the beginning of the array
	inline JSONNode ** begin(void) const {
		return array;
	}

	//returns the end of the array
	inline JSONNode ** end(void) const {
		return array + mysize;
	}

	//makes sure that even after shirnking and expanding, the iterator is in same relative position
	struct iteratorKeeper {
	public:
		#ifdef JSON_LIBRARY
			iteratorKeeper(jsonChildren * pthis, JSONNode ** & position) : 
				myRelativeOffset((json_index_t)(position - pthis -> array)),
		#else
			iteratorKeeper(jsonChildren * pthis, JSONNode ** & position, bool reverse = false) : 
				myRelativeOffset(reverse ? (json_index_t)(pthis -> array + (size_t)pthis -> mysize - position) : (json_index_t)(position - pthis -> array)),
				myReverse(reverse),
		#endif
		myChildren(pthis), 
		myPos(position){}

		~iteratorKeeper(void){
			#ifdef JSON_LIBRARY
				myPos = myChildren -> array + myRelativeOffset;
			#else
				if (myReverse){
					myPos = myChildren -> array + myChildren -> mysize - myRelativeOffset;
				} else {
					myPos = myChildren -> array + myRelativeOffset;
				}
			#endif
		}
	private:
		iteratorKeeper(const iteratorKeeper &);
		iteratorKeeper & operator = (const iteratorKeeper &);

		jsonChildren * myChildren;
		JSONNode ** & myPos;
		json_index_t myRelativeOffset;
		#ifndef JSON_LIBRARY
			bool myReverse BITS(1);
		#endif
	};

	//This function DOES NOT delete the item it points to
	inline void erase(JSONNode ** & position){
		JSON_ASSERT(array, JSON_TEXT("erasing something from a null array 1"));
		JSON_ASSERT(position >= array, JSON_TEXT("position is beneath the start of the array 1"));
		JSON_ASSERT(position <= array + mysize, JSON_TEXT("erasing out of bounds 1"));
		memmove(position, position + 1, (mysize-- - (position - array) - 1) * sizeof(JSONNode *));
		iteratorKeeper ik(this, position);
		shrink();
	}

	//This function DOES NOT delete the item it points to
	inline void erase(JSONNode ** & position, json_index_t number){
		doerase(position, number);
		iteratorKeeper ik(this, position);
		shrink();
	}

	//This function DOES NOT delete the item it points to
	inline void erase(JSONNode ** position, json_index_t number, JSONNode ** & starter){
		doerase(position, number);
		iteratorKeeper ik(this, starter);
		shrink();
	}

	#ifdef JSON_LIBRARY
		void insert(JSONNode ** & position, JSONNode * item){
	#else
		void insert(JSONNode ** & position, JSONNode * item, bool reverse = false){
	#endif
		//position isnt relative to array because of realloc
		JSON_ASSERT(position >= array, JSON_TEXT("position is beneath the start of the array insert 1"));
		JSON_ASSERT(position <= array + mysize, JSON_TEXT("position is above the end of the array insert 1"));
		{
			#ifdef JSON_LIBRARY
				iteratorKeeper ik(this, position);
			#else
				iteratorKeeper ik(this, position, reverse);
			#endif
			inc();
		}
		memmove(position + 1, position, (mysize++ - (position - array)) * sizeof(JSONNode *));
		*position = item;
	}

	void insert(JSONNode ** & position, JSONNode ** items, json_index_t num){
		JSON_ASSERT(position >= array, JSON_TEXT("position is beneath the start of the array insert 2"));
		JSON_ASSERT(position <= array + mysize, JSON_TEXT("position is above the end of the array insert 2"));
		{
			iteratorKeeper ik(this, position);
			inc(num);
		}
		const size_t ptrs = ((JSONNode **)(array + mysize)) - position;
		memmove(position + num, position, ptrs * sizeof(JSONNode *));
		memcpy(position, items, num * sizeof(JSONNode *));
		mysize += num;
	}

	inline void reserve(json_index_t amount){
		JSON_ASSERT(!array, JSON_TEXT("reserve is not meant to expand a preexisting array"));
		JSON_ASSERT(!mycapacity, JSON_TEXT("reservec is not meant to expand a preexisting array"));
		JSON_ASSERT(!mysize, JSON_TEXT("reserves is not meant to expand a preexisting array"));
		array = json_malloc<JSONNode*>(mycapacity = amount);
	}

	inline void reserve2(json_index_t amount){
		if (array){
			if (mycapacity < amount) inc(amount - mycapacity);
		} else {
			reserve(amount);
		}
	}

	//shrinks the array to only as large as it needs to be to hold everything within it
	inline void shrink() {
		if (mysize == 0){  //size is zero, we should completely free the array
			libjson_free<JSONNode*>(array);  //free does checks for a null pointer, so don't bother checking
			array = 0;
			#ifdef JSON_LESS_MEMORY
				} else {  //need to shrink it, using realloc
					JSON_ASSERT(array, JSON_TEXT("shrinking a null array that is not size 0"));
					array = json_realloc<JSONNode*>(array, mysize);
			#endif
		}
		mycapacity = mysize;
	}
JSON_PRIVATE
	//to make sure it's not copyable
	jsonChildren(const jsonChildren &);
	jsonChildren & operator = (const jsonChildren &);

	void deleteAll(void);  //implemented in JSONNode.cpp
	void doerase(JSONNode ** position, json_index_t number);

	JSONNode ** array;  //the expandable array

	json_index_t mysize;	     //the number of valid items
	json_index_t mycapacity;   //the number of possible items
};

#endif
