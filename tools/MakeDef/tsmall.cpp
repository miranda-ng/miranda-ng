#include <stdio.h>
#include <string.h>

#include "h_object.h"
#include "h_util.h"

HStaticSmallAllocator::HStaticSmallAllocator(size_t aPageSize) :
	HSmallAllocator(aPageSize)
{
	char* tData = new char[sizeof(HDataPage) + aPageSize];

	first = (HDataPage*)tData;
	first->free = aPageSize;
	first->next = NULL;
}

void HStaticSmallAllocator::tide()
{
	HDataPage* p1;

	for (HDataPage* p = first->next; p != NULL; p = p1) {
		p1 = p->next;
		delete p;
	}

	memset(&first->next, 0, pageSize + sizeof(void*));

	first->free = pageSize;
	first->next = NULL;
}

//====[ HSmallAllocator ]======================================================

HSmallAllocator::HSmallAllocator(size_t aPageSize) :
	first(NULL),
	pageSize(aPageSize)
{}

HSmallAllocator::~HSmallAllocator()
{
	tide();
}

char* HSmallAllocator::placeStr(const char* str)
{
	if (str == NULL)
		str = "";

	size_t len = strlen(str) + 1;

	void* p = allocateSpace(len);
	if (p == NULL)
		return NULL;

	memcpy(p, str, len);
	return (char*)p;
}

void* HSmallAllocator::placeData(const void* data, size_t len)
{
	void* p = allocateSpace(len);
	if (p == NULL)
		return NULL;

	if (data != NULL)
		memcpy(p, data, len);

	return p;
}

void* HSmallAllocator::allocateSpace(size_t bytes)
{
	HDataPage* p;

	if (bytes > pageSize) {
		size_t dataLen = sizeof(HDataPage) + bytes;
		char* data = new char[dataLen];
		memset(data, 0, dataLen);

		if (first == NULL)
			first = (HDataPage*)data;
		else {
			for (p = first; p->next != NULL; p = p->next)
				;

			p->next = (HDataPage*)data;
		}

		return data + sizeof(HDataPage);
	}

	for (p = first; p != NULL; p = p->next)
		if (p->free >= bytes)
			break;

	if (p == NULL || p->free < bytes) {
		size_t dataLen = sizeof(HDataPage) + pageSize;
		char* data = new char[dataLen];
		memset(data, 0, dataLen);

		p = (HDataPage*)data;
		p->free = pageSize;

		p->next = first;
		first = p;
	}

	char* res = (char*)(p + 1) + pageSize - p->free;   p->free -= bytes;
	return res;
}

void HSmallAllocator::tide()
{
	HDataPage* p1;

	for (HDataPage* p = first; p != 0; p = p1) {
		p1 = p->next;
		delete p;
	}

	first = NULL;
}

//=============================================================================

void* operator new(size_t bytes, HSmallAllocator& owner)
{
	return owner.allocateSpace(bytes);
}

//=============================================================================

void* HSmallAllocatorObject::operator new(size_t bytes, HSmallAllocator& owner)
{
	return owner.allocateSpace(bytes);
}

void* HSmallAllocatorObject::operator new[](size_t bytes, HSmallAllocator& owner) {
	return owner.allocateSpace(bytes);
}

//=============================================================================

void* HSmartSmallAllocatorObject::operator new(size_t bytes, HSmallAllocator& owner)
{
	char* result = (char*)owner.allocateSpace(bytes + sizeof(void*));
	*(HSmallAllocator**)result = &owner;
	return result + sizeof(void*);
}

void* HSmartSmallAllocatorObject::operator new[](size_t bytes, HSmallAllocator& owner) {
	char* result = (char*)owner.allocateSpace(bytes + sizeof(void*));
	*(HSmallAllocator**)result = &owner;
	return result + sizeof(void*);
}
