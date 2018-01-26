/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-12 Miranda ICQ/IM project,
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

*/

#ifndef __M_IMGSRVC_H
#define __M_IMGSRVC_H

#include "../libs/freeimage/src/FreeImage.h"

// load an image from disk
// wParam = full path and filename to the image
// lParam = IMGL_* flags
// returns a valid HBITMAP or 0 if image cannot be loaded
// if IMGL_RETURNDIB is set, it returns a pointer to a freeimage bitmap (FIBITMAP *)

#define IMGL_RETURNDIB 1        // will NOT return a HBITMAP but a FIBITMAP * instead (useful, if you
                                // want to do further image manipulations before converting to a Win32 bitmap
                                // caller MUST then free the FIBITMAP * using fii->FI_Unload() or MS_IMG_UNLOAD (see below)

#define IMGL_WCHAR     2        // filename is wchar_t

#define MS_IMG_LOAD "IMG/Load"

/*
 * control structure for loading images from memory buffers (e.g. network buffers, memory mapped files).
 */

struct IMGSRVC_MEMIO
{
    long iLen;					// length of the buffer
    void *pBuf;					// the buffer itself (you are responsible for allocating and free'ing it)
    FREE_IMAGE_FORMAT fif;		// -1 to detect the format or one of the FIF_* image format constants
    UINT flags;                 // flags to pass to FreeImage_LoadFromMemory()  (see freeimage docs)
};

// load an image from a memory buffer
// wParam = IMGSRVC_MEMIO *
// lParam = flags (see IMG/Load), valid are IMGL_RETURNDIB
// you must popupate iLen (buffer length) and pBuf (pointer to memory buffer)
// you must also specify the format in IMGSRVC_MEMIO.fif using one of the FIF_* constants defined in m_freeimage.h

#define MS_IMG_LOADFROMMEM "IMG/LoadFromMem"

// flags for IMGSRVC_INFO.dwMask

#define IMGI_FBITMAP  1  // the dib member is valid
#define IMGI_HBITMAP  2  // the hbm member is valid

/*
 * generic structure for various img functions
 * you must populate the fields as required, set the mask bits to indicate which member is valid
 */

struct IMGSRVC_INFO
{
    DWORD cbSize;
    union {
        char *szName;
        wchar_t *wszName;
        wchar_t *tszName;
    };
    HBITMAP hbm;
    FIBITMAP *dib;
    DWORD    dwMask;
    FREE_IMAGE_FORMAT fif;
};

// save image to disk
// wParam = IMGSRVC_INFO *  (szName/wszName, hbm OR dib, cbSize, dwMask mandatory. fif optional, if FIF_UNKNOWN is given
//                           it will be determined from the filename).
// lParam = low word: IMG_* flags     (IMGL_WCHAR is the only valid - filename will be assumed to be wchar_t and wszName must be used)
//          high word: FreeImage_Save flags
// set IMGSRVC_INFO.dwMask to indicate whether the HBITMAP of FIBITMAP member is valid

#define MS_IMG_SAVE  "IMG/Save"

// unload a FIFBITMAP
// wParam = FIFBITMAP *
// lParam = 0;
// this service is useful when you have loaded a bitmap with IMGL_RETURNDIB in which case you do not get
// a HBITMAP but instead a FBITMAP * which describes the freeimage-internal representation of a bitmap.

#define MS_IMG_UNLOAD "IMG/Unload"

/*
 * resizer from loadavatars moved to image service plugin
*/

#define RESIZEBITMAP_STRETCH 0				// Distort bitmap to size in (max_width, max_height)
#define RESIZEBITMAP_KEEP_PROPORTIONS 1		// Keep bitmap proportions (probabily only one of the
											// max_width/max_height will be respected, and the other will be
											// smaller)
#define RESIZEBITMAP_CROP 2					// Keep bitmap proportions but crop it to fix exactly in (max_width, max_height)
											// Some image info outside will be lost
#define RESIZEBITMAP_MAKE_SQUARE 3			// Image will be allways square. Image will be croped and the size
											// returned will be min(max_width, max_height)

#define RESIZEBITMAP_FLAG_DONT_GROW	0x1000	// If set, the image will not grow. Else, it will grow to fit the max width/height

typedef struct {
	size_t size; // sizeof(ResizeBitmap);

	HBITMAP hBmp;

	int max_width;
	int max_height;

	int fit; // One of RESIZEBITMAP_* with an OR with RESIZEBITMAP_FLAG_DONT_GROW if needed
} ResizeBitmap;

// Returns a copy of the bitmap with the size especified or the original bitmap if nothing has to be changed
// wParam = ResizeBitmap *
// lParam = NULL
// return NULL on error, ResizeBitmap->hBmp if don't need to resize or a new HBITMAP if resized
// You are responsible for calling DestroyObject() on the original HBITMAP

#define MS_IMG_RESIZE "IMG/ResizeBitmap"


/*
 * format conversion helpers
 *
 * these helper macros allow converting HBITMAP to FIBITMAP * format and vice vera. In any case,
 * the caller is responsible for freeing or deleting the original object.
 * These macros wrap around the FI_CreateHBITMAPFromDib() and FI_CreateDIBFromHBITMAP() interface
 * functions.
 */

//#define FI_HBM2DIB(x) (FI_CreateDIBFromHBITMAP((x)))
//#define FI_DIB2HBM(x) (FI_CreateHBITMAPFromDIB((x)))

#endif // __M_IMGSRVC_H
