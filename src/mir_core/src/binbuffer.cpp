/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

struct BufImpl
{
	uint32_t size, lockCount;

	BufImpl* alloc(size_t newSize)
	{
		bool bEmpty = (this == nullptr);
		auto *res = (BufImpl *)mir_realloc(this, newSize + sizeof(BufImpl));
		if (bEmpty) {
			res->lockCount = 1;
			res->size = 0;
		}
		return res;
	}

	BufImpl* realloc(size_t newSize)
	{
		bool bEmpty;
		newSize += sizeof(BufImpl);
		if (this != nullptr) {
			newSize += size;
			bEmpty = false;
		}
		else bEmpty = true;

		auto *res = (BufImpl *)mir_realloc(this, newSize);
		if (bEmpty) {
			res->lockCount = 1;
			res->size = 0;
		}
		return res;
	}

	void free()
	{
		if (this == nullptr)
			return;

		if (lockCount == 1)
			mir_free(this);
		else
			lockCount--;
	}
};

__forceinline BufImpl* ptr2buf(char *p)
{
	return (p == nullptr) ? nullptr : (BufImpl*)p-1;
}

/////////////////////////////////////////////////////////////////////////////////////////

MBinBuffer::MBinBuffer()
{}

MBinBuffer::MBinBuffer(const MBinBuffer &orig)
{
	ptr2buf(m_buf)->free();

	BufImpl *p = ptr2buf(m_buf = orig.m_buf);
	if (p)
		p->lockCount++;
}

MBinBuffer::MBinBuffer(size_t preAlloc)
{
	BufImpl *p = (BufImpl *)mir_alloc(sizeof(BufImpl) + preAlloc);
	p->lockCount = 1;
	p->size = (unsigned)preAlloc;
	m_buf = (char*)(p + 1);
}

MBinBuffer::~MBinBuffer()
{
	ptr2buf(m_buf)->free();
}

void MBinBuffer::append(const void *pBuf, size_t bufLen)
{
	if (pBuf == nullptr || bufLen == 0)
		return;

	BufImpl *p = ptr2buf(m_buf)->realloc(bufLen);
	if (p) {
		m_buf = (char*)(p + 1);
		memcpy(m_buf + p->size, pBuf, bufLen);
		p->size += (unsigned)bufLen;
	}
	else m_buf = nullptr;
}

void MBinBuffer::appendBefore(const void *pBuf, size_t bufLen)
{
	if (pBuf == nullptr || bufLen == 0)
		return;

	BufImpl *p = ptr2buf(m_buf)->realloc(bufLen);
	if (p) {
		m_buf = (char *)(p + 1);
		memmove(m_buf + bufLen, m_buf, p->size);
		memcpy(m_buf, pBuf, bufLen);
		p->size += (unsigned)bufLen;
	}
	else m_buf = nullptr;
}

void MBinBuffer::assign(const void *pBuf, size_t bufLen)
{
	if (pBuf == nullptr || bufLen == 0)
		return;

	BufImpl *p = ptr2buf(m_buf)->alloc(bufLen);
	if (p) {
		p->size = (unsigned)bufLen;
		m_buf = (char *)(p + 1);
		memcpy(m_buf, pBuf, bufLen);
	}
	else m_buf = nullptr;
}

size_t MBinBuffer::length() const
{
	BufImpl *p = ptr2buf(m_buf);
	return (p) ? p->size : 0;
}

void MBinBuffer::remove(size_t sz)
{
	BufImpl *p = ptr2buf(m_buf);
	if (!p)
		return;

	if (sz > p->size)
		sz = p->size;

	if (p->size == sz) {
		p->free();
		m_buf = nullptr;
	}
	else {
		memmove(m_buf, m_buf + sz, p->size - sz);
		p->size -= (unsigned)sz;
	}
}
