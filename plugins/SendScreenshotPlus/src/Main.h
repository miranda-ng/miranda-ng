/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 Sérgio Vieira Rolanski (portet from Borland C++)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef MainH
#define MainH

//---------------------------------------------------------------------------

extern ATOM g_clsTargetHighlighter;
extern HANDLE			hNetlibUser;

//---------------------------------------------------------------------------

HANDLE	NetlibInit();
void	NetlibClose();

void	IcoLib_LoadModule();
void	AddMenuItems();
int		RegisterServices();
int		UnRegisterServices();

int		hook_ModulesLoaded(WPARAM, LPARAM);
int		hook_SystemPreShutdown(WPARAM wParam, LPARAM lParam);

INT_PTR	service_CaptureAndSendDesktop(WPARAM wParam, LPARAM lParam);
INT_PTR	service_OpenCaptureDialog(WPARAM wParam, LPARAM lParam);
INT_PTR	service_EditBitmap(WPARAM wParam, LPARAM lParam);
INT_PTR	service_Send2ImageShack(WPARAM wParam, LPARAM lParam);

int		OnSendScreenShot(WPARAM wParam, LPARAM lParam);

TCHAR*	GetCustomPath();

//---------------------------------------------------------------------------
#endif
