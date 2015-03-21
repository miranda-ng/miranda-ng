#include <stdio.h>
#include <string.h>

#include "h_collection.h"

HCollection::HCollection(ccIndex aLimit, ccIndex aDelta) :
	count(0),
	limit(0),
	items(NULL),
	delta(aDelta),
	shouldDelete(false),
	lastItem(-1)
{
	setLimit(aLimit);
}

HCollection::~HCollection()
{
	if (shouldDelete)
		freeAll();

	delete items;
}

void* HCollection::at(ccIndex pIndex)
{
	if (pIndex < 0 || pIndex >= count)
		return NULL;

	lastItem = pIndex;
	return items[pIndex];
}

bool HCollection::atFree(ccIndex pIndex)
{
	void* item = at(pIndex);
	if (!atRemove(pIndex))
		return false;

	freeItem(item);
	return true;
}

bool HCollection::atInsert(ccIndex pIndex, void* item)
{
	if (pIndex < 0)
		return false;

	if (count == limit) {
		if (delta == 0)
			return false;

		setLimit(count + delta);
	}

	if (pIndex < count)
		shiftItem(pIndex, 1);

	count++;

	items[pIndex] = (uchar*)item;
	lastItem = pIndex;
	return true;
}

bool HCollection::atPut(ccIndex pIndex, void* item)
{
	if (pIndex >= count) return false;

	items[pIndex] = (uchar*)item;
	return true;
}

bool HCollection::atRemove(ccIndex pIndex)
{
	if (pIndex < 0 || pIndex >= count)
		return false;

	count--;
	if (count > pIndex)
		shiftItem(pIndex, -1);

	lastItem = pIndex;
	return true;
}

void HCollection::remove(void* item)
{
	atRemove(indexOf(item));
}

void HCollection::removeAll()
{
	count = 0;
}

void* HCollection::firstThat(ccTestFunc Test, void* arg)
{
	for (ccIndex i = 0; i < count; i++)
		if (Test(items[i], arg) == true) {
			lastItem = i;
			return items[i];
		}

	return NULL;
}

void* HCollection::lastThat(ccTestFunc Test, void* arg)
{
	for (ccIndex i = count - 1; i >= 0; i--)
		if (Test(items[i], arg) == true) {
			lastItem = i;
			return items[i];
		}

	return NULL;
}

void HCollection::forEach(ccAppFunc action, void* arg)
{
	for (ccIndex i = 0; i < count; i++)
		action(items[i], arg);

	lastItem = count - 1;
}

void HCollection::free(void* item)
{
	remove(item);
	freeItem(item);
}

void HCollection::freeAll()
{
	for (ccIndex i = 0; i < count; i++)
		freeItem(at(i));

	count = 0;
}

void HCollection::freeItem(void* item)
{
	delete item;
}

ccIndex HCollection::indexOf(const void* item)
{
	for (ccIndex i = 0; i < count; i++)
		if (item == items[i])
			return i;

	return -1;
}

bool HCollection::insert(void* item)
{
	return atInsert(count, item);
}

void HCollection::pack()
{
	uchar** curDst = items;
	uchar** curSrc = items;
	uchar** last = items + count;

	while (curSrc < last) {
		if (*curSrc != 0)
			*curDst++ = *curSrc;
		*curSrc++;
	}
}

void HCollection::setLimit(ccIndex aLimit)
{
	if (aLimit < count)
		aLimit = count;

	if (aLimit > maxCollectionSize)
		aLimit = maxCollectionSize;

	if (aLimit != limit) {
		uchar** aItems;

		if (aLimit == 0) aItems = NULL;
		else {
			aItems = new uchar*[aLimit];
			if (count != 0 && aItems != NULL && items != NULL)
				memcpy(aItems, items, count* sizeof(void*));
		}

		delete items;
		items = aItems;
		limit = aLimit;
	}
}

bool HCollection::shiftItem(ccIndex pItemNo, int direction)
{
	if (items == NULL || pItemNo >= limit)
		return false;

	switch (direction) {
	case 1:  // Раздвинуть массив на один элемент
		memmove(items + pItemNo + 1, items + pItemNo, sizeof(void*)*(count - pItemNo));
		items[pItemNo] = NULL;
		return true;

	case -1: // Сдвинуть массив на один элемент
		memmove(items + pItemNo, items + pItemNo + 1, sizeof(void*)*(count - pItemNo));
		items[count] = NULL;
		return true;

	default: return false;
	}
}

void HCollection::tide()
{
	delete items; items = NULL;
	count = 0;
}
