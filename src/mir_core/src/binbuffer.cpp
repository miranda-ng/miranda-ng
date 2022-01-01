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

MBinBuffer::MBinBuffer() :
	m_buf(nullptr),
	m_len(0)
{
}

MBinBuffer::~MBinBuffer()
{
	mir_free(m_buf);
}

void MBinBuffer::append(const void *pBuf, size_t bufLen)
{
	if (pBuf == nullptr || bufLen == 0)
		return;

	m_buf = (char*)mir_realloc(m_buf, bufLen + m_len);
	if (m_buf) {
		memcpy(m_buf + m_len, pBuf, bufLen);
		m_len += bufLen;
	}
	else m_len = 0;
}

void MBinBuffer::appendBefore(const void *pBuf, size_t bufLen)
{
	if (pBuf == nullptr || bufLen == 0)
		return;

	m_buf = (char*)mir_realloc(m_buf, bufLen + m_len);
	if (m_buf) {
		memmove(m_buf + bufLen, m_buf, m_len);
		memcpy(m_buf, pBuf, bufLen);
		m_len += bufLen;
	}
	else m_len = 0;
}

void MBinBuffer::assign(const void *pBuf, size_t bufLen)
{
	if (pBuf == nullptr || bufLen == 0)
		return;

	m_buf = (char *)mir_realloc(m_buf, bufLen);
	if (m_buf) {
		memcpy(m_buf, pBuf, bufLen);
		m_len = bufLen;
	}
	else m_len = 0;
}

void MBinBuffer::remove(size_t sz)
{
	if (sz > m_len)
		m_len = sz;

	if (m_len == sz) {
		m_len = 0;
		mir_free(m_buf); m_buf = nullptr;
	}
	else {
		memmove(m_buf, m_buf + sz, m_len - sz);
		m_len -= sz;
	}
}
