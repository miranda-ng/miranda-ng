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
  MS_IMG_LOAD:PAnsiChar = 'IMG/Load';

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

type
  TIMGSRVC_MEMIO = record
    iLen:cardinal; // length of the buffer
    pBuf:pointer;  // the buffer itself (you are responsible for allocating and free'ing it)
    fif:integer;   // FREE_IMAGE_FORMAT,-1 to detect the format or one of the FIF_* image
                   // format constant. Make sure to provide the right one.
    flags:dword;   // flags to pass to FreeImage_LoadFromMem()  (see freeimage docs)
  end;
  _tagIMGSRVC_MEMIO = TIMGSRVC_MEMIO;

const
{
  load an image from a memory buffer
  wParam = IMGSRVC_MEMIO *
  lParam = flags (see IMG/Load), valid are IMGL_RETURNDIB
  you must popupate iLen (buffer length) and pBuf (pointer to memory buffer)
  you must also specify the format in IMGSRVC_MEMIO.fif using one of the FIF_* constants
}
  MS_IMG_LOADFROMMEM:PAnsiChar = 'IMG/LoadFromMem';

// flags for IMGSRVC_INFO.dwMask

  IMGI_FBITMAP = 1;  // the dib member is valid
  IMGI_HBITMAP = 2;  // the hbm member is valid

{
  generic structure for various img functions
  you must populate the fields as required, set the mask bits to indicate which member is valid
}

type
//!!
{
  FIBITMAP = record
    data : pointer;
  end;
  PFIBITMAP = ^FIBITMAP;
}
  TIMGSRVC_INFO = record
    cbSize:dword;
    szName:TChar;
    hbm:HBITMAP;
    dib:pointer; // PFIBITMAP
    dwMask:dword;
    fif:integer; // FREE_IMAGE_FORMAT
  end;
  _tagIMGSRVC_INFO = TIMGSRVC_INFO;

const
{
  save image to disk
  wParam = pointer to IMGSRVC_INFO (szName/wszName, hbm OR dib, cbSize, dwMask
           mandatory. fif optional, if FIF_UNKNOWN is given it will be determined
           from the filename).
  lParam = low word: IMG_* flags (IMGL_WCHAR is the only valid - filename will be assumed
           to be wchar_t and wszName must be used)
           high word: FreeImage_Save flags
  set IMGSRVC_INFO.dwMask to indicate whether the HBITMAP of FIBITMAP member is valid
}
  MS_IMG_SAVE:PAnsiChar = 'IMG/Save';

{
  unload a FIFBITMAP
  wParam = FIFBITMAP *
  lParam = 0;
  this service is useful when you have loaded a bitmap with IMGL_RETURNDIB in
  which case you do not get a HBITMAP but instead a FBITMAP * which describes
  the freeimage-internal representation of a bitmap.
}
  MS_IMG_UNLOAD:PAnsiChar = 'IMG/Unload';

{
 resizer from loadavatars moved to image service plugin
}

  RESIZEBITMAP_STRETCH          = 0;   // Distort bitmap to size in (max_width, max_height)
  RESIZEBITMAP_KEEP_PROPORTIONS = 1;   // Keep bitmap proportions (probabily only
                                       // one of the max_width/max_height will be
                                       // respected, and the other will be smaller)
  RESIZEBITMAP_CROP             = 2;   // Keep bitmap proportions but crop it to
                                       // fix exactly in (max_width, max_height)
                                       // Some image info outside will be lost
  RESIZEBITMAP_MAKE_SQUARE      = 3;   // Image will be allways square. Image will
                                       // be croped and the size returned will be
                                       // min(max_width, max_height)
  RESIZEBITMAP_FLAG_DONT_GROW  = $1000; // If set, the image will not grow. Else,
                                       // it will grow to fit the max width/height

type
  TResizeBitmap = record
    size      :size_t;   // sizeof(ResizeBitmap);
    hBmp      :HBITMAP;
    max_width :int;
    max_height:int;
    fit       :int;      // One of RESIZEBITMAP_* with an OR with RESIZEBITMAP_FLAG_DONT_GROW if needed
  end;

const
{
  Returns a copy of the bitmap with the size especified or the original bitmap
  if nothing has to be changed
  wParam = ResizeBitmap *
  lParam = NULL
  return NULL on error, ResizeBitmap->hBmp if don't need to resize or a new HBITMAP if resized
  You are responsible for calling DestroyObject() on the original HBITMAP
}
  MS_IMG_RESIZE:PAnsiChar = 'IMG/ResizeBitmap';

{
 * format conversion helpers
 *
 * these helper macros allow converting HBITMAP to FIBITMAP * format and vice vera. In any case,
 * the caller is responsible for freeing or deleting the original object.
 * These macros wrap around the FI_CreateHBITMAPFromDib() and FI_CreateDIBFromHBITMAP() interface
 * functions.
}

//#define FI_HBM2DIB(x) (FI_CreateDIBFromHBITMAP((x)))
//#define FI_DIB2HBM(x) (FI_CreateHBITMAPFromDIB((x)))

{$ENDIF}
