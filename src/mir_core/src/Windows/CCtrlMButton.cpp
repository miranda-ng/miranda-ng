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
// CCtrlMButton

CCtrlMButton::CCtrlMButton(CDlgBase *dlg, int ctrlId, HICON hIcon, const char* tooltip) 
	: CCtrlButton(dlg, ctrlId),
	m_hIcon(hIcon),
	m_toolTip(tooltip)
{}

CCtrlMButton::CCtrlMButton(CDlgBase *dlg, int ctrlId, int iCoreIcon, const char* tooltip)
	: CCtrlButton(dlg, ctrlId),
	m_hIcon(::Skin_LoadIcon(iCoreIcon)),
	m_toolTip(tooltip)
{}

CCtrlMButton::~CCtrlMButton()
{
	::IcoLib_ReleaseIcon(m_hIcon);
}

void CCtrlMButton::OnInit()
{
	CCtrlButton::OnInit();

	SendMessage(m_hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hIcon);
	SendMessage(m_hwnd, BUTTONADDTOOLTIP, (WPARAM)m_toolTip, 0);
	SendMessage(m_hwnd, BUTTONSETASFLATBTN, (WPARAM)m_toolTip, 0);
}

void CCtrlMButton::MakeFlat()
{
	SendMessage(m_hwnd, BUTTONSETASFLATBTN, TRUE, 0);
}

void CCtrlMButton::MakePush()
{
	SendMessage(m_hwnd, BUTTONSETASPUSHBTN, TRUE, 0);
}
