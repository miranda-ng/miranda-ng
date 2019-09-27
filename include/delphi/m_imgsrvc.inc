{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2007 Miranda ICQ/IM project,
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

-----------------------------------------------------------------------
Miranda Image services plugin / API definitions
Provides various services for image loading, saving and manipulations.

This module is based on the freeimage library, copyrighted by the FreeImage
Project members.

Miranda plugin code (c) 2007 by Nightwish, silvercircle@gmail.com, all else (C)
by the FreeImage project (http://freeimage.sourceforge.net)
}

{$IFNDEF M_IMGSRVC}
{$DEFINE M_IMGSRVC}

const
  IMGL_RETURNDIB = 1; // will NOT return a HBITMAP but a FIBITMAP * instead (useful,
                      // if you want to do further image manipulations before
                      // converting to a Win32 bitmap caller MUST then free the
                      // FIBITMAP * using fii->FI_Unload() or MS_IMG_UNLOAD (see below)
  IMGL_WCHAR     = 2; // filename is Unicode

{
  load an image from disk
  wParam = full path and filename to the image
  lParam = IMGL_* flags
  returns a valid HBITMAP or 0 if image cannot be loaded
  if IMGL_RETURNDIB is set, it returns a pointer to a freeimage bitmap (FIBITMAP *)
}

function Image_Load(fileName:PWideChar; flags:int): HBITMAP; stdcall; external AppDll;

{
  control structure for loading images from memory buffers (e.g. network buffers,
  memory mapped files).
}

//type
//!!
//  FREE_IMAGE_FORMAT = type integer;
const
  FIF_UNKNOWN = -1;
  FIF_BMP     = 0;
  FIF_ICO     = 1;
  FIF_JPEG    = 2;
  FIF_PNG     = 3;
  FIF_GIF     = 4;
  FIF_JNG     = 5;
  FIF_KOALA   = 6;
  FIF_LBM     = 7;
  FIF_IFF     = FIF_LBM;
  FIF_MNG     = 8;
  FIF_PBM     = 9;
  FIF_PBMRAW  = 10;
  FIF_PCD     = 11;
  FIF_PCX     = 12;
  FIF_PGM     = 13;
  FIF_PGMRAW  = 14;
  FIF_PPM     = 15;
  FIF_PPMRAW  = 16;
  FIF_RAS     = 17;
  FIF_TARGA   = 18;
  FIF_TIFF    = 19;
  FIF_WBMP    = 20;
  FIF_PSD     = 21;
  FIF_CUT     = 22;
  FIF_XBM     = 23;
  FIF_XPM     = 24;
  FIF_DDS     = 25;
  FIF_HDR     = 26;
  FIF_FAXG3   = 27;
  FIF_SGI     = 28;
  FIF_EXR     = 29;
  FIF_J2K     = 30;
  FIF_JP2     = 31;
  FIF_PFM     = 32;
  FIF_PICT    = 33;
  FIF_RAW     = 34;

{
  load an image from a memory buffer
  wParam = IMGSRVC_MEMIO *
  lParam = flags (see IMG/Load), valid are IMGL_RETURNDIB
  you must popupate iLen (buffer length) and pBuf (pointer to memory buffer)
  you must also specify the format in IMGSRVC_MEMIO.fif using one of the FIF_* constants
}

function Image_LoadFromMem(pData:pointer; cbLen:cardinal; format:int): HBITMAP; stdcall; external AppDll;

{$ENDIF}
