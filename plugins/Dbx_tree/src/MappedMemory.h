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

#pragma once

#include <windows.h>
#include "FileAccess.h"

class CMappedMemory : public CFileAccess
{
private:
	uint8_t* m_Base;

	HANDLE m_DirectFile;
	HANDLE m_FileMapping;
protected:
	
	uint32_t _Read(void* Buf, uint32_t Source, uint32_t Size);
  uint32_t _Write(void* Buf, uint32_t Dest, uint32_t Size);
	void     _Invalidate(uint32_t Dest, uint32_t Size);
	uint32_t _SetSize(uint32_t Size);
	void     _Flush();
public:
	CMappedMemory(const TCHAR* FileName);
	virtual ~CMappedMemory();
};
