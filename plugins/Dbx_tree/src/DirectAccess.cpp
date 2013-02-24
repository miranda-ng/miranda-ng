/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#include "Interface.h"
#include "DirectAccess.h"
#include "Logger.h"

CDirectAccess::CDirectAccess(const TCHAR* FileName)
: CFileAccess(FileName)
{
	m_File = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
	CHECKSYS(m_File != INVALID_HANDLE_VALUE,
		logCRITICAL, _T("CreateFile failed"));

	m_MinAllocGranularity = 0x00001000;  // 4kb   to avoid heavy fragmentation
	m_AllocGranularity    = 0x00008000;  // 32kb
	m_MaxAllocGranularity = 0x00100000;  // 1mb   for fast increasing

	uint32_t size = GetFileSize(m_File, NULL);
	size = (size + m_AllocGranularity - 1) & ~(m_AllocGranularity - 1);

	if (size == 0)
		size = m_AllocGranularity;

	m_AllocSize = size;

	InitJournal();
}

CDirectAccess::~CDirectAccess()
{
	if (m_File)
	{
		if (INVALID_SET_FILE_POINTER != SetFilePointer(m_File, m_Size, NULL, FILE_BEGIN))
			SetEndOfFile(m_File);

		CloseHandle(m_File);
	}
}

uint32_t CDirectAccess::_Read(void* Buf, uint32_t Source, uint32_t Size)
{
	DWORD read = 0;

	CHECKSYS(INVALID_SET_FILE_POINTER != SetFilePointer(m_File, Source, NULL, FILE_BEGIN),
		logERROR, _T("SetFilePointer failed"));

	CHECKSYS(ReadFile(m_File, Buf, Size, &read, NULL),
		logERROR, _T("ReadFile failed"));

	return read;
}
uint32_t CDirectAccess::_Write(void* Buf, uint32_t Dest, uint32_t Size)
{
	DWORD written = 0;

	CHECKSYS(INVALID_SET_FILE_POINTER != SetFilePointer(m_File, Dest, NULL, FILE_BEGIN),
		logERROR, _T("SetFilePointer failed"));

	CHECKSYS(WriteFile(m_File, Buf, Size, &written, NULL),
		logERROR, _T("WriteFile failed"));

	return written;
}

uint32_t CDirectAccess::_SetSize(uint32_t Size)
{
	CHECKSYS(INVALID_SET_FILE_POINTER != SetFilePointer(m_File, Size, NULL, FILE_BEGIN),
		logERROR, _T("SetFilePointer failed"));

	CHECKSYS(SetEndOfFile(m_File),
		logERROR, _T("SetEndOfFile failed"));

	return Size;		
}

void CDirectAccess::_Invalidate(uint32_t Dest, uint32_t Size)
{
	DWORD written;
	uint8_t buf[4096];
	memset(buf, 0, sizeof(buf));

	CHECKSYS(INVALID_SET_FILE_POINTER != SetFilePointer(m_File, Dest, NULL, FILE_BEGIN),
		logERROR, _T("SetFilePointer failed"));

	while (Size > sizeof(buf))
	{
		Size -= sizeof(buf);
		CHECKSYS(WriteFile(m_File, buf, sizeof(buf), &written, NULL),
			logERROR, _T("WriteFile failed"));
	}
	CHECKSYS(WriteFile(m_File, buf, Size, &written, NULL),
		logERROR, _T("WriteFile failed"));
}

void CDirectAccess::_Flush()
{
	CHECKSYS(FlushFileBuffers(m_File),
		logERROR, _T("FlushFileBuffers failed"));
}
