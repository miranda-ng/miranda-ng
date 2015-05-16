
//This file is part of Msg_Export a Miranda IM plugin
//Copyright (C)2002 Kennet Nielsen ( http://sourceforge.net/projects/msg-export/ )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef MSG_EXP_FILE_VIEWER
#define MSG_EXP_FILE_VIEWER

void UpdateFileViews(const TCHAR *pszFile);

bool bOpenExternaly(MCONTACT hContact);
bool bShowFileViewer(MCONTACT hContact);

bool bUseInternalViewer(bool bNew);
bool bUseInternalViewer();

extern tstring sFileViewerPrg;

#endif

