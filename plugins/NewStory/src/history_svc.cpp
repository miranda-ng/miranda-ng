/*
Copyright (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
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

#include "stdafx.h"

static INT_PTR SvcGetCurrent(WPARAM wParam, LPARAM)
{
	if (auto *pData = (NewstoryListData *)wParam)
		if (auto *p = pData->GetItem(pData->caret))
			return p->dbe.getEvent();

	return 0;
}

static INT_PTR SvcGetSrmm(WPARAM wParam, LPARAM)
{
	auto *pData = (NewstoryListData *)wParam;
	return (pData) ? INT_PTR(pData->pMsgDlg) : 0;
}

static INT_PTR SvcGetSelection(WPARAM wParam, LPARAM lParam)
{
	auto *pData = (NewstoryListData *)wParam;
	auto *pRet = (std::vector<MEVENT>*)lParam;
	if (pData && pRet) {
		for (int i = pData->totalCount; i >= 0; i--)
			if (auto *p = pData->items.get(i))
				if (p->m_bSelected)
					pRet->push_back(p->dbe.getEvent());

		if (pRet->empty() && pData->caret != -1)
			if (auto *p = pData->GetItem(pData->caret))
				pRet->push_back(p->dbe.getEvent());
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

void InitServices()
{
	CreateServiceFunction("NewStory/GetSrmm", &SvcGetSrmm);
	CreateServiceFunction("NewStory/FileReady", &SvcFileReady);
	CreateServiceFunction("NewStory/GetCurrent", &SvcGetCurrent);
	CreateServiceFunction("NewStory/GetSelection", &SvcGetSelection);
}
