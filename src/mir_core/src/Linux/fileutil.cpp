/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

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

#include "../stdafx.h"
#include <unistd.h>

MIR_CORE_DLL(FILE*) _wfopen(const wchar_t *pwszFileName, const wchar_t *pwszMode)
{
	return fopen(T2Utf(pwszFileName), T2Utf(pwszMode));
}

MIR_CORE_DLL(int) _wchdir(const wchar_t *pwszPath)
{
	return chdir(T2Utf(pwszPath));
}

/////////////////////////////////////////////////////////////////////////////////////////

MFilePath::MFileIterator::iterator MFilePath::MFileIterator::iterator::operator++()
{
	// if (ptr != nullptr) {
	// 	if (::FindNextFileW(ptr->m_hFind, &ptr->m_data) == 0) {
	// 		::FindClose(ptr->m_hFind); ptr->m_hFind = INVALID_HANDLE_VALUE;
	// 		ptr = nullptr;
	// 	}
	// }
	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////

MFilePath::MFileIterator::MFileIterator(const wchar_t *pwszPath)
{
	// if (pwszPath != nullptr)
	//	m_hFind = ::FindFirstFileW(pwszPath, &m_data);
}

MFilePath::MFileIterator::~MFileIterator()
{
	// if (m_hFind != INVALID_HANDLE_VALUE)
	//	::FindClose(m_hFind);
}

MFilePath::MFileIterator::iterator MFilePath::MFileIterator::begin()
{
	// if (m_hFind == INVALID_HANDLE_VALUE)
	// 	return MFilePath::MFileIterator::iterator(nullptr);

	return MFilePath::MFileIterator::iterator(this);
}

bool MFilePath::MFileIterator::isDir() const
{
	//if (m_hFind == INVALID_HANDLE_VALUE)
	//	return false;

	return (m_flags & 1) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool MFilePath::isExist() const
{
	return ::access(T2Utf(c_str()), 0) == 0;
}

bool MFilePath::move(const wchar_t *pwszDest)
{
	return ::rename(T2Utf(c_str()), T2Utf(pwszDest)) != 0;
}

MFilePath::MFileIterator MFilePath::search()
{
	return MFileIterator(c_str());
}
