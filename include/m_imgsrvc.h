/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

#include <m_core.h>

// load an image from disk
// returns a valid HBITMAP or 0 if image cannot be loaded
// if IMGL_RETURNDIB is set, it returns a pointer to a freeimage bitmap (FIBITMAP *)

#define IMGL_RETURNDIB 1        // will NOT return a HBITMAP but a FIBITMAP * instead (useful, if you
                                // want to do further image manipulations before converting to a Win32 bitmap
                                // caller MUST then free the FIBITMAP * using fii->FI_Unload() or MS_IMG_UNLOAD (see below)

EXTERN_C MIR_APP_DLL(HBITMAP) Image_Load(const wchar_t *pwszPath, int iFlags = 0 /* one of IMGL_*/ );

/*
 * control structure for loading images from memory buffers (e.g. network buffers, memory mapped files).
 */

// load an image from a memory buffer
// wParam = IMGSRVC_MEMIO *
// lParam = flags (see IMG/Load), valid are IMGL_RETURNDIB
// you must popupate iLen (buffer length) and pBuf (pointer to memory buffer)
// you must also specify the format in IMGSRVC_MEMIO.fif using one of the FIF_* constants defined in m_freeimage.h

EXTERN_C MIR_APP_DLL(HBITMAP) Image_LoadFromMem(const void *pBuf, size_t cbLen, FREE_IMAGE_FORMAT fif);

/*
 * generic structure for various img functions
 * you must populate the fields as required, set the mask bits to indicate which member is valid
 */

// flags for IMGSRVC_INFO::dwMask
#define IMGI_FBITMAP  1  // the dib member is valid
#define IMGI_HBITMAP  2  // the hbm member is valid

struct IMGSRVC_INFO
{
    uint32_t cbSize;
	 const wchar_t *pwszName;
    HBITMAP hbm;
    FIBITMAP *dib;
    uint32_t dwMask;
    FREE_IMAGE_FORMAT fif;
};

EXTERN_C MIR_APP_DLL(int) Image_Save(const IMGSRVC_INFO *pInfo, int iFlags = 0);

/*
 * resizer from loadavatars moved to image service plugin
*/

// Returns a copy of the bitmap with the size especified or the original bitmap if nothing has to be changed
// returns NULL on error, hBmp if don't need to resize or a new HBITMAP if resized
// You are responsible for calling DestroyObject() on the original HBITMAP

#define RESIZEBITMAP_STRETCH 0				// Distort bitmap to size in (max_width, max_height)
#define RESIZEBITMAP_KEEP_PROPORTIONS 1		// Keep bitmap proportions (probabily only one of the
											// max_width/max_height will be respected, and the other will be
											// smaller)
#define RESIZEBITMAP_CROP 2					// Keep bitmap proportions but crop it to fix exactly in (max_width, max_height)
											// Some image info outside will be lost
#define RESIZEBITMAP_MAKE_SQUARE 3			// Image will be allways square. Image will be croped and the size
											// returned will be min(max_width, max_height)

#define RESIZEBITMAP_FLAG_DONT_GROW	0x1000	// If set, the image will not grow. Else, it will grow to fit the max width/height

EXTERN_C MIR_APP_DLL(HBITMAP) Image_Resize(HBITMAP hBmp, int fit /* RESIZEBITMAP_*/, int max_width, int max_height);

#endif // __M_IMGSRVC_H
