// History Linklist Plus
// Copyright (C) 2010 Thomas Wendel, gureedo
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#pragma once

#ifndef _LINKLIST_DLG_H
#define _LINKLIST_DLG_H

INT_PTR CALLBACK MainDlgProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK ProgressBarDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SearchDlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK OptionsDlgProc(HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK SplitterProc(HWND, UINT, WPARAM, LPARAM);

#endif //_LINKLIST_DLG_H