/*
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

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

/////////////////////////////////////////////////////////////////////////////////////////

MFilePath::MFileIterator::iterator MFilePath::MFileIterator::iterator::operator++()
{
	if (ptr != nullptr) {
		if (::FindNextFileW(ptr->m_hFind, &ptr->m_data) == 0) {
			::FindClose(ptr->m_hFind); ptr->m_hFind = INVALID_HANDLE_VALUE;
			ptr = nullptr;
		}
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////

MFilePath::MFileIterator::MFileIterator(const wchar_t *pwszPath)
{
	if (pwszPath != nullptr)
		m_hFind = ::FindFirstFileW(pwszPath, &m_data);
}

MFilePath::MFileIterator::~MFileIterator()
{
	if (m_hFind != INVALID_HANDLE_VALUE)
		::FindClose(m_hFind);
}

MFilePath::MFileIterator::iterator MFilePath::MFileIterator::begin()
{
	if (m_hFind == INVALID_HANDLE_VALUE)
		return MFilePath::MFileIterator::iterator(nullptr);

	return MFilePath::MFileIterator::iterator(this);
}

bool MFilePath::MFileIterator::isDir() const
{
	if (m_hFind == INVALID_HANDLE_VALUE)
		return false;

	return (m_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringW MFilePath::getExtension() const
{
	int idx = ReverseFind('.');
	return (idx == -1) ? L"" : Right(GetLength() - idx - 1);
}

bool MFilePath::isExecutable() const
{
	CMStringW wszExt = getExtension(), wszEnv;
	wszExt.MakeUpper();
	wszEnv.GetEnvironmentVariableW(L"PATHEXT");

	for (auto *p = wcstok(wszEnv.GetBuffer(), L";"); p; p = wcstok(0, L";"))
		if (wszExt == p + 1)
			return true;

	return false;
}

bool MFilePath::isExist() const
{
	return _waccess(c_str(), 0) == 0;
}

bool MFilePath::move(const wchar_t *pwszDest)
{
	return MoveFileW(c_str(), pwszDest) != 0;
}

MFilePath::MFileIterator MFilePath::search()
{
	DWORD dwAttr = GetFileAttributesW(c_str());
	if (dwAttr != -1 && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		return MFileIterator(*this + L"\\*");

	return MFileIterator(c_str());
}
