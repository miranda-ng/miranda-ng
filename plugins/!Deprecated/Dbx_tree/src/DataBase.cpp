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
#include "DataBase.h"
#include "newpluginapi.h"
#include "Logger.h"

CDataBase *gDataBase = NULL;

CDataBase::CDataBase(const TCHAR *FileName)
{
	InitDbInstance(this);
	//RegisterServices();

	size_t len = _tcslen(FileName);
	m_FileName[0] = new TCHAR[len + 1];
	_tcsncpy_s(m_FileName[0], len + 1, FileName, len);
	m_FileName[0][len] = 0;

	TCHAR * tmp = _tcsrchr(m_FileName[0], '.');
	if (tmp)
	{
		m_FileName[1] = new TCHAR[len + 1];
		_tcsncpy_s(m_FileName[1], len + 1, m_FileName[0], tmp - m_FileName[0]);
		_tcscat_s(m_FileName[1], len + 1, _T(".pri"));
	} else {
		m_FileName[1] = new TCHAR[len + 5];
		_tcscpy_s(m_FileName[1], len + 5, m_FileName[0]);
		_tcscat_s(m_FileName[1], len + 5, _T(".pri"));
	}

	m_Opened = false;

	for (int i = 0; i < DBFileMax; ++i)
	{
		m_BlockManager[i] = NULL;
		m_FileAccess[i] = NULL;
		m_EncryptionManager[i] = NULL;
		m_HeaderBlock[i] = 0;
	}

	m_Entities = NULL;
	m_Settings = NULL;
	m_Events   = NULL;

}
CDataBase::~CDataBase()
{
	delete m_Events;
	delete m_Settings;
	delete m_Entities;

	m_Entities = NULL;
	m_Settings = NULL;
	m_Events   = NULL;

	for (int i = DBFileMax - 1; i >= 0; --i)
	{
		delete m_BlockManager[i];
		delete m_FileAccess[i];
		delete m_EncryptionManager[i];

		m_BlockManager[i]      = NULL;
		m_FileAccess[i]        = NULL;
		m_EncryptionManager[i] = NULL;

		delete [] (m_FileName[i]);
	}
	DestroyDbInstance(this);
}

int CDataBase::CreateDB()
{
	/// TODO: create and show wizard
	if (!CreateNewFile(DBFileSetting) ||
		  !CreateNewFile(DBFilePrivate))
		return EMKPRF_CREATEFAILED;

	return 0;
}


int CDataBase::CheckFile(TDBFileType Index)
{
	TGenericFileHeader h;
	memset(&h, 0, sizeof(h));
	DWORD r = 0;
	HANDLE htmp = CreateFile(m_FileName[Index], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
	if (htmp != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(htmp, 0, NULL, FILE_BEGIN);
		if (ReadFile(htmp, &h, sizeof(h), &r, NULL))
		{
			if (0 != memcmp(h.Gen.Signature, cFileSignature[Index], sizeof(cFileSignature[Index])))
			{
				CloseHandle(htmp);
				return EGROKPRF_UNKHEADER;
			}

			if (cDBVersion < h.Gen.Version)
			{
				CloseHandle(htmp);
				return EGROKPRF_VERNEWER;
			}

			CloseHandle(htmp);
			return EGROKPRF_NOERROR;
		}
		CloseHandle(htmp);
	}

	return EGROKPRF_CANTREAD;
}

int CDataBase::CheckDB()
{
	int res = CheckFile(DBFileSetting);

	if (res != EGROKPRF_NOERROR)
		return res;

	if (PrivateFileExists())
		res = CheckFile(DBFilePrivate);

	return res;
}

int CDataBase::LoadFile(TDBFileType Index)
{
	TGenericFileHeader h;
	m_EncryptionManager[Index] = new CEncryptionManager;

	m_FileAccess[Index] = new CMappedMemory(m_FileName[Index]);

	m_FileAccess[Index]->Read(&h, 0, sizeof(h));
	m_EncryptionManager[Index]->InitEncryption(h.Gen.FileEncryption);

	m_FileAccess[Index]->Size(h.Gen.FileSize);
	m_FileAccess[Index]->sigFileSizeChanged().connect(this, &CDataBase::onFileSizeChanged);

	m_BlockManager[Index] = new CBlockManager(*m_FileAccess[Index], *m_EncryptionManager[Index]);
	
	CBlockManager::WriteTransaction trans(*m_BlockManager[Index]); // don't fire size event until header is loaded

	m_HeaderBlock[Index] = m_BlockManager[Index]->ScanFile(sizeof(h), cHeaderBlockSignature, h.Gen.FileSize);

	if (m_HeaderBlock[Index] == 0)
	{
		LOG(logCRITICAL, _T("Header Block not found! File damaged: \"%s\""), m_FileName[Index]);
		return -1;
	}

	uint32_t size = sizeof(h);
	uint32_t sig = -1;
	m_Header[Index] = m_BlockManager[Index]->ReadBlock<TGenericFileHeader>(0, size, sig);

	sig = cHeaderBlockSignature;
	TGenericFileHeader * buf = m_BlockManager[Index]->ReadBlock<TGenericFileHeader>(m_HeaderBlock[Index], size, sig);
	if (!buf)
	{
		LOG(logCRITICAL, _T("Header Block cannot be read! File damaged: \"%s\""), m_FileName[Index]);
		return -1;
	}

	buf->Gen.Obscure = 0;

	if (memcmp(m_Header[Index], buf, size))
	{
		LOG(logCRITICAL, _T("Header Block in \"%s\" damaged!"), m_FileName[Index]);
		return -1;
	}

	return 0;
}

int CDataBase::OpenDB()
{
  if (!PrivateFileExists())
	{
		// TODO WIZARD
		if (!CreateNewFile(DBFilePrivate))
			return -1;
	}

	int res = LoadFile(DBFileSetting);
	if ((res != 0) && (CLogger::logERROR <= CLogger::Instance().ShowMessage()))
	{
		return res;
	}

	res = LoadFile(DBFilePrivate);

	if ((res != 0) && (CLogger::logERROR <= CLogger::Instance().ShowMessage()))
	{
		return res;
	}
	if (CLogger::logERROR <= CLogger::Instance().ShowMessage())
		return -1;

	m_Entities = new CEntities(*m_BlockManager[DBFilePrivate],
													 m_Header[DBFilePrivate]->Pri.RootEntity,
													 m_Header[DBFilePrivate]->Pri.Entities,
													 m_Header[DBFilePrivate]->Pri.Virtuals);

	m_Entities->sigRootChanged().connect(this, &CDataBase::onEntitiesRootChanged);
	m_Entities->sigVirtualRootChanged().connect(this, &CDataBase::onVirtualsRootChanged);

	if (m_Entities->getRootEntity() != m_Header[DBFilePrivate]->Pri.RootEntity)
	{
		m_Header[DBFilePrivate]->Pri.RootEntity = m_Entities->getRootEntity();
		ReWriteHeader(DBFilePrivate);
	}

	m_Settings = new CSettings(*m_BlockManager[DBFileSetting],
		                         *m_BlockManager[DBFilePrivate],
														  m_Header[DBFileSetting]->Set.Settings,
														 *m_Entities);

	m_Settings->sigRootChanged().connect(this, &CDataBase::onSettingsRootChanged);

	m_Events = new CEvents(*m_BlockManager[DBFilePrivate],
		                     *m_EncryptionManager[DBFilePrivate],
												 *m_Entities,
												 *m_Settings);

	for (MCONTACT id = m_Entities->compFirstContact(); id != 0; id = m_Entities->compNextContact(id)) {
		DBCachedContact *cc = m_cache->AddContactToCache(id);
		CheckProto(cc, "");
	}

	return 0;
}

bool CDataBase::PrivateFileExists()
{
	HANDLE htmp = CreateFile(m_FileName[DBFilePrivate], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
	if (htmp != INVALID_HANDLE_VALUE)
	{
		CloseHandle(htmp);
		return true;
	}

	return false;
}


bool CDataBase::CreateNewFile(TDBFileType File)
{
	CEncryptionManager enc;
	CDirectAccess fa(m_FileName[File]);
	fa.Size(sizeof(TGenericFileHeader));
	CBlockManager bm(fa, enc);
	bm.ScanFile(sizeof(TGenericFileHeader), 0, sizeof(TGenericFileHeader));

	CBlockManager::WriteTransaction trans(bm);

	uint32_t block;
	TGenericFileHeader * buf = bm.CreateBlock<TGenericFileHeader>(block, cHeaderBlockSignature);
	uint32_t size = 0;
	uint32_t sig = -1;
	TGenericFileHeader * h = bm.ReadBlock<TGenericFileHeader>(0, size, sig);

	memset(h, 0, sizeof(TGenericFileHeader));
	memcpy(&h->Gen.Signature, &cFileSignature[File], sizeof(h->Gen.Signature));
	h->Gen.Version = cDBVersion;
	h->Gen.FileSize = fa.Size();

	memcpy(buf, h, sizeof(TGenericFileHeader));
	bm.UpdateBlock(block, 0);
	bm.UpdateBlock(0, -1);
	
	return true;
}

inline void CDataBase::ReWriteHeader(TDBFileType Index)
{
	m_BlockManager[Index]->UpdateBlock(0, -1);
	uint32_t size = 0, sig = 0;
	TGenericFileHeader * h = m_BlockManager[Index]->ReadBlock<TGenericFileHeader>(m_HeaderBlock[Index], size, sig);
	
	*h = *m_Header[Index];
	h->Gen.Obscure = GetTickCount();
	m_BlockManager[Index]->UpdateBlock(m_HeaderBlock[Index], 0);
}


void CDataBase::onSettingsRootChanged(CSettings* Settings, CSettingsTree::TNodeRef NewRoot)
{
	m_Header[DBFileSetting]->Set.Settings = NewRoot;
	ReWriteHeader(DBFileSetting);
}
void CDataBase::onVirtualsRootChanged(void* Virtuals, CVirtuals::TNodeRef NewRoot)
{
	m_Header[DBFilePrivate]->Pri.Virtuals = NewRoot;
	ReWriteHeader(DBFilePrivate);
}
void CDataBase::onEntitiesRootChanged(void* Entities, CEntities::TNodeRef NewRoot)
{
	m_Header[DBFilePrivate]->Pri.Entities = NewRoot;
	ReWriteHeader(DBFilePrivate);
}
void CDataBase::onFileSizeChanged(CFileAccess * File, uint32_t Size)
{
	if (File == m_FileAccess[DBFileSetting])
	{
		m_Header[DBFileSetting]->Gen.FileSize = Size;
		ReWriteHeader(DBFileSetting);
	} else {
		m_Header[DBFilePrivate]->Gen.FileSize = Size;
		ReWriteHeader(DBFilePrivate);
	}
}

int CDataBase::getProfileName(int BufferSize, char * Buffer)
{
	TCHAR * slash = _tcsrchr(m_FileName[DBFileSetting], '\\');
	if (slash)
		slash++;
	else
		slash = m_FileName[DBFileSetting];

	int l = static_cast<int>(_tcslen(slash));
	if (BufferSize < l + 1)
		return -1;
	
	char * tmp = mir_t2a(slash);
	strcpy_s(Buffer, BufferSize, tmp);
	mir_free(tmp);

	return 0;
}
int CDataBase::getProfilePath(int BufferSize, char * Buffer)
{
	TCHAR * slash = _tcsrchr(m_FileName[DBFileSetting], '\\');
	if (!slash)
		return -1;

	int l = slash - m_FileName[DBFileSetting];

	if (BufferSize < l + 1)
	{
		return -1;
	}

	*slash = 0;
	char * tmp = mir_t2a(m_FileName[DBFileSetting]);
	strcpy_s(Buffer, BufferSize, tmp);
	mir_free(tmp);
	*slash = '\\';

	return 0;
}
