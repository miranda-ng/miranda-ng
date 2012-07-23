/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "commonheaders.h"

//the cache has not been loaded when these functions are used

extern DBSignature dbSignature;

int CDb3Base::CreateDbHeaders()
{
	DBContact user;
	DWORD bytesWritten;

	CopyMemory(m_dbHeader.signature, &dbSignature,sizeof(m_dbHeader.signature));
	m_dbHeader.version = DB_THIS_VERSION;
	m_dbHeader.ofsFileEnd = sizeof(struct DBHeader);
	m_dbHeader.slackSpace = 0;
	m_dbHeader.contactCount = 0;
	m_dbHeader.ofsFirstContact = 0;
	m_dbHeader.ofsFirstModuleName = 0;
	m_dbHeader.ofsUser = 0;
	//create user
	m_dbHeader.ofsUser = m_dbHeader.ofsFileEnd;
	m_dbHeader.ofsFileEnd += sizeof(DBContact);
	SetFilePointer(m_hDbFile,0,NULL,FILE_BEGIN);
	WriteFile(m_hDbFile,&m_dbHeader,sizeof(m_dbHeader),&bytesWritten,NULL);
	user.signature = DBCONTACT_SIGNATURE;
	user.ofsNext = 0;
	user.ofsFirstSettings = 0;
	user.eventCount = 0;
	user.ofsFirstEvent = user.ofsLastEvent = 0;
	SetFilePointer(m_hDbFile,m_dbHeader.ofsUser,NULL,FILE_BEGIN);
	WriteFile(m_hDbFile,&user,sizeof(DBContact),&bytesWritten,NULL);
	FlushFileBuffers(m_hDbFile);
	return 0;
}

int CDb3Base::CheckDbHeaders()
{
	if (memcmp(m_dbHeader.signature, &dbSignature, sizeof(m_dbHeader.signature))) return 1;
	if (m_dbHeader.version != DB_THIS_VERSION) return 2;
	if (m_dbHeader.ofsUser == 0) return 3;
	return 0;
}
