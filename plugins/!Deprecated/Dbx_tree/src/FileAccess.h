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
#include <time.h>
#include "stdint.h"
#include "sigslot.h"

class CFileAccess
{
public:
	static const uint8_t cJournalSignature[20];

	CFileAccess(const TCHAR* FileName);
	virtual ~CFileAccess();


	uint32_t Read(void* Buf, uint32_t Source, uint32_t Size)
		{
			return _Read(Buf, Source, Size);
		};

	bool Write(void* Buf, uint32_t Dest, uint32_t Size)
		{
			if (m_Journal.Use)
			{
				DWORD written;

				TJournalEntry * data = reinterpret_cast<TJournalEntry*>(m_Journal.Buffer + m_Journal.BufUse);
				data->Signature = 'writ';
				data->Address = Dest;
				data->Size = Size;
				m_Journal.BufUse += 12;
				if (Size + m_Journal.BufUse < sizeof(m_Journal.Buffer) - 12) // one journal header has always to fit in
				{
					memcpy(m_Journal.Buffer + m_Journal.BufUse, Buf, Size);
					m_Journal.BufUse += Size;
				} else {
					WriteFile(m_Journal.hFile, m_Journal.Buffer, m_Journal.BufUse, &written, NULL);
					WriteFile(m_Journal.hFile, Buf, Size, &written, NULL);

					m_Journal.BufUse = 0;
				}
			} else {
				_Write(Buf, Dest, Size);
			}

			return true;
		};

	void Invalidate(uint32_t Dest, uint32_t Size)
		{
			if (m_Journal.Use)
			{
				DWORD written;

				TJournalEntry * data = reinterpret_cast<TJournalEntry*>(m_Journal.Buffer + m_Journal.BufUse);
				data->Signature = 'inva';
				data->Address = Dest;
				data->Size = Size;
				m_Journal.BufUse += 12;
				if (m_Journal.BufUse > sizeof(m_Journal.Buffer) - 12)
				{
					WriteFile(m_Journal.hFile, m_Journal.Buffer, m_Journal.BufUse, &written, NULL);
					m_Journal.BufUse = 0;
				}
			} else {
				_Invalidate(Dest, Size);
			}
		};

	void Flush()
		{
			if (m_Journal.Use)
			{
				if (m_Journal.BufUse)
				{
					DWORD written;
					WriteFile(m_Journal.hFile, m_Journal.Buffer, m_Journal.BufUse, &written, NULL);
					m_Journal.BufUse = 0;
				}
				FlushFileBuffers(m_Journal.hFile);
			} else {
				_Flush();
			}
		};

	void UseJournal(bool UseIt)
		{
			m_Journal.Use = UseIt;
		};

	void CompleteTransaction()
		{
			if (m_Size != m_LastSize)
			{
				m_sigFileSizeChanged.emit(this, m_Size);
				m_LastSize = m_Size;
			}
		};
	void CloseTransaction()
		{
			if (m_Journal.Use)
			{
				DWORD written;

				TJournalEntry * data = reinterpret_cast<TJournalEntry*>(m_Journal.Buffer + m_Journal.BufUse);
				data->Signature = 'fini';
				data->Address = 0;
				data->Size = m_Size;
				
				WriteFile(m_Journal.hFile, m_Journal.Buffer, m_Journal.BufUse + 12, &written, NULL);
				m_Journal.BufUse = 0;
			}
		};

	void CleanJournal();

	uint32_t Size(uint32_t NewSize);
	uint32_t Size()
		{	return m_Size; };
	void ReadOnly(bool ReadOnly)
		{	m_ReadOnly = ReadOnly; };
	bool ReadOnly()
		{	return m_ReadOnly; };

	typedef sigslot::signal2<CFileAccess *, uint32_t> TOnFileSizeChanged;

	TOnFileSizeChanged & sigFileSizeChanged()
	{	return m_sigFileSizeChanged; };

protected:
	TCHAR* m_FileName;
	struct {
		bool Use;
		TCHAR* FileName;
		HANDLE hFile;

		uint8_t Buffer[4096];
		uint32_t BufUse;
	} m_Journal;

	uint32_t m_Size;
	uint32_t m_AllocSize;
	uint32_t m_AllocGranularity;
	uint32_t m_MinAllocGranularity;
	uint32_t m_MaxAllocGranularity;
	uint32_t m_LastAllocTime;
	bool     m_ReadOnly;
	uint32_t m_LastSize;

	TOnFileSizeChanged m_sigFileSizeChanged;
	virtual uint32_t _Read(void* Buf, uint32_t Source, uint32_t Size) = 0;
  virtual uint32_t _Write(void* Buf, uint32_t Dest, uint32_t Size) = 0;
	virtual void     _Invalidate(uint32_t Dest, uint32_t Size) = 0;
	virtual uint32_t _SetSize(uint32_t Size) = 0;
	virtual void     _Flush() = 0;

#pragma pack (push, 1)
	typedef struct TJournalEntry
	{
		uint32_t Signature;
		uint32_t Address;
		uint32_t Size;
	} TJournalEntry;
#pragma pack (pop)

	void InitJournal();
	void ProcessJournal();
};
