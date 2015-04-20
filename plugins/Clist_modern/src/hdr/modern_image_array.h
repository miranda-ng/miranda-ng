/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

Created by Pescuma
*/

#pragma once

#ifndef __IMAGE_ARRAY_H__
# define __IMAGE_ARRAY_H__

#include <windows.h>


struct IMAGE_ARRAY_DATA_NODE
{
	int width;
	int height;
};

struct IMAGE_ARRAY_DATA
{
	// Configuration
	BOOL width_based;
	int grow_step;
	HDC hdc;

	// Img data
	HBITMAP img;
	int width;
	int height;
	void * lpBits;

	// CS
	CRITICAL_SECTION cs;

	// Array
	IMAGE_ARRAY_DATA_NODE *nodes;
	int nodes_size;
	int nodes_allocated_size;
};


// Initialize data
void ImageArray_Initialize(IMAGE_ARRAY_DATA *iad, BOOL width_based, int grow_step);

// Free data but keep config
void ImageArray_Clear(IMAGE_ARRAY_DATA *iad);

// Free data
// If keep_bitmap is TRUE, doesn't delete de bitmap and return its handle. Else, return NULL
HBITMAP ImageArray_Free(IMAGE_ARRAY_DATA *iad, BOOL keep_bitmap);

// Add image to the list (return the index of the image or -1 on error)
// If pos == -1, add to the end of the list
int ImageArray_AddImage(IMAGE_ARRAY_DATA *iad, HBITMAP hBmp, int pos);

// Change an image in the list (return TRUE on success)
BOOL ImageArray_ChangeImage(IMAGE_ARRAY_DATA *iad, HBITMAP hBmp, int pos);

// Remove an image
BOOL ImageArray_RemoveImage(IMAGE_ARRAY_DATA *iad, int pos);

// Draw an image
BOOL ImageArray_DrawImage(IMAGE_ARRAY_DATA *iad, int pos, HDC hdcDest, int nXDest, int nYDest, BYTE Alpha);

#endif // __IMAGE_ARRAY_H__
