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

#include "stdafx.h"

// create an object for sending
IcqFileTransfer::IcqFileTransfer(MCONTACT hContact, const wchar_t *pwszFileName) :
	m_wszFileName(pwszFileName)
{
	pfts.flags = PFTS_UNICODE | PFTS_SENDING;
	pfts.hContact = hContact;
	pfts.szCurrentFile.w = m_wszFileName.GetBuffer();

	const wchar_t *p = wcsrchr(pfts.szCurrentFile.w, '\\');
	if (pwszFileName != nullptr)
		p++;
	else
		p = pfts.szCurrentFile.w;
	m_wszShortName = p;
}

IcqFileTransfer::~IcqFileTransfer()
{
	if (m_fileId >= 0)
		_close(m_fileId);
}

void IcqFileTransfer::FillHeaders(AsyncHttpRequest *pReq)
{
	pReq->AddHeader("Content-Type", "application/octet-stream");
	pReq->AddHeader("Content-Disposition", CMStringA(FORMAT, "attachment; filename=\"%s\"", T2Utf(m_wszShortName).get()));

	uint32_t dwPortion = pfts.currentFileSize - pfts.currentFileProgress;
	if (dwPortion > 1000000)
		dwPortion = 1000000;

	pReq->AddHeader("Content-Range", CMStringA(FORMAT, "bytes %lld-%lld/%lld", pfts.currentFileProgress, pfts.currentFileProgress + dwPortion - 1, pfts.currentFileSize));
	pReq->AddHeader("Content-Length", CMStringA(FORMAT, "%d", dwPortion));

	pReq->dataLength = dwPortion;
	pReq->pData = (char *)mir_alloc(dwPortion);
	_lseek(m_fileId, pfts.currentFileProgress, SEEK_SET);
	_read(m_fileId, pReq->pData, dwPortion);

	pfts.currentFileProgress += dwPortion;
	pfts.totalProgress += dwPortion;
}
