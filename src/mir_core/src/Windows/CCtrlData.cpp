/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (C) 2012-21 Miranda NG team

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

#include "../stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlData class

CCtrlData::CCtrlData(CDlgBase *wnd, int idCtrl)
	: CCtrlBase(wnd, idCtrl),
	m_dbLink(nullptr)
{}

CCtrlData::~CCtrlData()
{
	delete m_dbLink;
}

void CCtrlData::OnInit()
{
	CCtrlBase::OnInit();
	OnReset();
}

void CCtrlData::CreateDbLink(const char* szModuleName, const char* szSetting, BYTE type, DWORD iValue)
{
	m_dbLink = new CDbLink(szModuleName, szSetting, type, iValue);
}

void CCtrlData::CreateDbLink(const char* szModuleName, const char* szSetting, wchar_t* szValue)
{
	m_dbLink = new CDbLink(szModuleName, szSetting, DBVT_WCHAR, szValue);
}
