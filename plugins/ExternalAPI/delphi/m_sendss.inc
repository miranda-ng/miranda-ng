//This file is part of Send Screenshot a Miranda IM plugin
//Copyright (c) 2004-2006 Sérgio Vieira Rolanski
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

{$IFNDEF M_SENDSS}
{$DEFINE M_SENDSS}

const
//////////////////////////////////////////////////////////////////////////////
// Open the main capture dialog, but does not capture anything
// wParam = contact handle
// lParam = 0
// Returns zero on success
//   -1 if was not possibly to create capture window
  MS_SENDSS_OPENDIALOG:PAnsiChar = 'SendSS/OpenCaptureDialog';

//////////////////////////////////////////////////////////////////////////////
// Send an image to imageshack.us
// wParam = (char*)filename
// lParam = (HANDLE)contact (can be null)
// v0.8.0.0+
// Returns always 0 (if lParam != 0)
// Returns (char*)URL or error message if lParam = 0. caller must mir_free the pointer
  MS_SENDSS_SEND2IMAGESHACK:PAnsiChar = 'SendSS/Send2ImageShack';

//////////////////////////////////////////////////////////////////////////////
// 1. Send a screenshot of the desktop to the selected contact
// wParam = contact handle
// lParam = 0
// 2. Open the capture dialog in take screenshot only mode (it will not be sent)
// wParam = 0
// lParam = anything but 0
// Returns:
//   zero on success
//   -1 if was not possibly to create capture window
  MS_SENDSS_SENDDESKTOP:PAnsiChar = 'SendSS/SendDesktop';

//////////////////////////////////////////////////////////////////////////////
// Edit a in-memory bitmap on the edit window
// wParam = (SENDSSCB) callback function address to call when editing is done
// lParam = (HBITMAP) bitmap handle, a copy is made so the calling function can free this handle after the service function returns
// Returns:
//   Zero on success
//   -1 if could not create the edit window
//   -2 if was not possible to alocate needed memory
  MS_SENDSS_EDITBITMAP:PAnsiChar = 'SendSS/EditBitmap';

// The parameter passed is the bitmap handle
// the called function must free this handle when it's not needed anymore
// Don't do much processing on the called function or you will lock miranda up
//typedef void (__stdcall *SENDSSCB)(LPVOID);

{$ENDIF}
