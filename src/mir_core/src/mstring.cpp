/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

/////////////////////////////////////////////////////////////////////////////////////////
// CMBaseString

class CNilMStringData : public CMStringData
{
public:
	CNilMStringData();

public:
	wchar_t achNil[2];
};

CNilMStringData::CNilMStringData()
{
	nRefs = 2;  // Never gets freed
	nDataLength = 0;
	nAllocLength = 0;
	achNil[0] = 0;
	achNil[1] = 0;
}

static CNilMStringData m_nil;

/////////////////////////////////////////////////////////////////////////////////////////
// CMBaseString

MIR_CORE_DLL(CMStringData*) mirstr_allocate(int nChars, int nCharSize)
{
	nChars++; // nil char
	size_t nDataBytes = nCharSize * nChars;
	size_t nTotalSize = nDataBytes + sizeof(CMStringData);

	CMStringData *pData = static_cast<CMStringData*>(malloc(nTotalSize));
	if (pData == NULL)
		return NULL;

	pData->nRefs = 1;
	pData->nAllocLength = nChars - 1;
	pData->nDataLength = 0;
	return pData;
}

MIR_CORE_DLL(void) mirstr_free(CMStringData *pData)
{
	free(pData);
}

MIR_CORE_DLL(CMStringData*) mirstr_realloc(CMStringData* pData, int nChars, int nCharSize)
{
	nChars++; // nil char
	ULONG nDataBytes = nCharSize * nChars;
	ULONG nTotalSize = nDataBytes + sizeof(CMStringData);

	CMStringData *pNewData = static_cast<CMStringData*>(realloc(pData, nTotalSize));
	if (pNewData == NULL)
		return NULL;

	pNewData->nAllocLength = nChars - 1;
	return pNewData;
}

MIR_CORE_DLL(CMStringData*) mirstr_getNil()
{
	m_nil.AddRef();
	return &m_nil;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CMStringData

MIR_CORE_DLL(void) mirstr_lock(CMStringData* pThis)
{
	pThis->nRefs--;  // Locked buffers can't be shared, so no interlocked operation necessary
	if (pThis->nRefs == 0)
		pThis->nRefs = -1;
}

MIR_CORE_DLL(void) mirstr_release(CMStringData* pThis)
{
	if (InterlockedDecrement(&pThis->nRefs) <= 0)
		mirstr_free(pThis);
}

MIR_CORE_DLL(void) mirstr_unlock(CMStringData* pThis)
{
	if (pThis->IsLocked())
	{
		pThis->nRefs++;  // Locked buffers can't be shared, so no interlocked operation necessary
		if (pThis->nRefs == 0)
			pThis->nRefs = 1;
	}
}
