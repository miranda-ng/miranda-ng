/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_image_array.h"
#include "hdr/modern_commonprototypes.h"

// To use this code in other places, replace the body of this func by the body of ske_CreateDIB32
static HBITMAP ImageArray_CreateBitmapPoint(int cx, int cy, void ** pt)
{
	return ske_CreateDIB32Point(cx, cy, pt);
}


// Initialize data
static BOOL ImageArray_Alloc(LP_IMAGE_ARRAY_DATA iad, int size)
{
	int size_grow = size;

	if (size_grow > iad->nodes_allocated_size)
	{
		size_grow += iad->grow_step - (size_grow % iad->grow_step);

		if (iad->nodes != NULL)
		{
			IMAGE_ARRAY_DATA_NODE *tmp = (IMAGE_ARRAY_DATA_NODE *)realloc((void *)iad->nodes,
				sizeof(IMAGE_ARRAY_DATA_NODE) * size_grow);

			if (tmp == NULL)
			{
				TRACE("Out of memory: realloc returned NULL (ImageArray_Alloc)");
				ImageArray_Free(iad, FALSE);
				return FALSE;
			}

			iad->nodes = tmp;
			memset(&iad->nodes[iad->nodes_allocated_size], 0, (size_grow - iad->nodes_allocated_size) * sizeof(IMAGE_ARRAY_DATA_NODE));
		}
		else
		{
			iad->nodes = (IMAGE_ARRAY_DATA_NODE *)malloc(sizeof(IMAGE_ARRAY_DATA_NODE) * size_grow);

			if (iad->nodes == NULL)
			{
				TRACE("Out of memory: alloc returned NULL (ImageArray_Alloc)");

				ImageArray_Free(iad, FALSE);

				return FALSE;
			}

			memset(iad->nodes, 0, (sizeof(IMAGE_ARRAY_DATA_NODE) * size_grow));
		}

		iad->nodes_allocated_size = size_grow;
	}
	else if (size < iad->nodes_allocated_size)
	{
		// Give some more space to try to avoid a free
		if ((iad->nodes_allocated_size - size) / iad->grow_step >= 2)
		{
			IMAGE_ARRAY_DATA_NODE *tmp;

			size_grow += iad->grow_step - (size_grow % iad->grow_step);

			tmp = (IMAGE_ARRAY_DATA_NODE *)realloc((void *)iad->nodes, sizeof(IMAGE_ARRAY_DATA_NODE) * size_grow);

			if (tmp == NULL)
			{
				TRACE("Out of memory: realloc returned NULL when reducing size! (ImageArray_Alloc)");

				ImageArray_Free(iad, FALSE);

				return FALSE;
			}

			iad->nodes = tmp;
		}
	}

	iad->nodes_size = size;

	return TRUE;
}


// Initialize data
void ImageArray_Initialize(LP_IMAGE_ARRAY_DATA iad, BOOL width_based, int grow_step)
{
	iad->width_based = width_based;
	iad->grow_step = grow_step;
	if (iad->grow_step <= 0)
	{
		iad->grow_step = 1;
	}
	iad->hdc = CreateCompatibleDC(NULL);
	iad->img = NULL;

	iad->width = 0;
	iad->height = 0;

	InitializeCriticalSection(&iad->cs);

	iad->nodes = NULL;
	iad->nodes_allocated_size = 0;
	iad->nodes_size = 0;
}


// Free data
// If keep_bitmap is TRUE, doesn't delete de bitmap and return its handle. Else, return NULL
HBITMAP ImageArray_Free(LP_IMAGE_ARRAY_DATA iad, BOOL keep_bitmap)
{
	DeleteDC(iad->hdc);

	if (iad->img != NULL && !keep_bitmap)
	{
		DeleteObject(iad->img);
		iad->img = NULL;
		iad->width = 0;
		iad->height = 0;
	}

	if (iad->nodes != NULL)
	{
		free(iad->nodes);
		iad->nodes = NULL;
		iad->nodes_allocated_size = 0;
		iad->nodes_size = 0;
	}

	DeleteCriticalSection(&iad->cs);

	return iad->img;
}

// Free data but keep config
void ImageArray_Clear(LP_IMAGE_ARRAY_DATA iad)
{
	HDC tmpdc = CreateCompatibleDC(iad->hdc);
	if (iad->hdc) DeleteDC(iad->hdc);
	iad->hdc = tmpdc;

	if (iad->img != NULL)
	{
		DeleteObject(iad->img);
		iad->img = NULL;
		iad->width = 0;
		iad->height = 0;
	}

	if (iad->nodes != NULL)
	{
		free(iad->nodes);
		iad->nodes = NULL;
		iad->nodes_allocated_size = 0;
		iad->nodes_size = 0;
	}
}


// Add image to the list (return the index of the image or -1 on error)
// If pos == -1, add to the end of the list
int ImageArray_AddImage(LP_IMAGE_ARRAY_DATA iad, HBITMAP hBmp, int pos)
{
	BITMAP bm;
	int new_width, new_height;
	HBITMAP hNewBmp, old_bmp;
	HDC hdc_old;
	BOOL last_one;

	int i;

	if (hBmp == NULL)
		return -1;

	EnterCriticalSection(&iad->cs);

	if (pos < 0)
		pos = iad->nodes_size;

	// Add to end?
	if (pos >= iad->nodes_size)
	{
		pos = iad->nodes_size;
		last_one = TRUE;
	}
	else
	{
		last_one = FALSE;
	}

	// Get bounds
	if (!GetObject(hBmp, sizeof(BITMAP), &bm))
	{
		LeaveCriticalSection(&iad->cs);
		return -1;
	}

	if (iad->width_based)
	{
		new_width = max(bm.bmWidth, iad->width);
		new_height = iad->height + bm.bmHeight;
	}
	else
	{
		new_width = bm.bmWidth + iad->width;
		new_height = max(iad->height, bm.bmHeight);
	}

	// Alloc image
	hNewBmp = ImageArray_CreateBitmapPoint(new_width, new_height, &(iad->lpBits));
	if (hNewBmp == NULL)
	{
		LeaveCriticalSection(&iad->cs);
		return -1;
	}

	// Alloc array
	if (!ImageArray_Alloc(iad, iad->nodes_size + 1))
	{
		DeleteObject(hNewBmp);
		LeaveCriticalSection(&iad->cs);
		return -1;
	}

	// Move image...

	// Set some draw states
	SelectObject(iad->hdc, hNewBmp);
	hdc_old = CreateCompatibleDC(iad->hdc);
	old_bmp = (HBITMAP)GetCurrentObject(hdc_old, OBJ_BITMAP);

	SetBkMode(iad->hdc, TRANSPARENT);
	{
		POINT org;
		GetBrushOrgEx(iad->hdc, &org);
		SetStretchBltMode(iad->hdc, HALFTONE);
		SetBrushOrgEx(iad->hdc, org.x, org.y, NULL);
	}

	{
		int x = 0, y = 0, w = 0, h = 0;

		// 1- old data
		if (pos > 0)
		{
			SelectObject(hdc_old, iad->img);

			if (iad->width_based)
			{
				w = iad->width;
				h = 0;
				for (i = 0; i < pos; i++)
				{
					h += iad->nodes[i].height;
				}
			}
			else
			{
				h = iad->height;
				w = 0;
				for (i = 0; i < pos; i++)
				{
					w += iad->nodes[i].width;
				}
			}
			BitBlt(iad->hdc, 0, 0, w, h, hdc_old, 0, 0, SRCCOPY);
		}

		// 2- new image
		if (iad->width_based)
		{
			x = 0;
			y = h;
		}
		else
		{
			x = w;
			y = 0;
		}
		SelectObject(hdc_old, hBmp);
		BitBlt(iad->hdc, x, y, bm.bmWidth, bm.bmHeight, hdc_old, 0, 0, SRCCOPY);

		// 3- old data
		if (!last_one)
		{
			int ox, oy;

			SelectObject(hdc_old, iad->img);

			if (iad->width_based)
			{
				ox = 0;
				oy = y;

				x = 0;
				y += bm.bmHeight;

				w = iad->width;
				h = iad->height - h;
			}
			else
			{
				ox = x;
				oy = 0;

				x += bm.bmWidth;
				y = 0;

				w = iad->width - w;
				h = iad->height;
			}
			BitBlt(iad->hdc, x, y, w, h, hdc_old, ox, oy, SRCCOPY);
		}
	}

	// restore things
	SelectObject(hdc_old, old_bmp);
	DeleteDC(hdc_old);
	if (iad->img != NULL) DeleteObject(iad->img);
	iad->img = hNewBmp;

	// Move array
	if (!last_one && iad->nodes_size > 1)
	{
		memmove(&iad->nodes[pos + 1], &iad->nodes[pos], (iad->nodes_size - pos) * sizeof(IMAGE_ARRAY_DATA_NODE));
	}
	iad->nodes[pos].width = bm.bmWidth;
	iad->nodes[pos].height = bm.bmHeight;

	iad->width = new_width;
	iad->height = new_height;

	// Finished it!
	LeaveCriticalSection(&iad->cs);

	return pos;
}



// Change an image in the list (return TRUE on success)
BOOL ImageArray_ChangeImage(LP_IMAGE_ARRAY_DATA iad, HBITMAP hBmp, int pos)
{
	BITMAP bm;
	int new_width, new_height;
	HBITMAP hNewBmp;
	HDC hdc_old;
	int i;

	if (hBmp == NULL)
		return FALSE;

	if (pos < 0)
		return FALSE;

	if (pos >= iad->nodes_size)
		return FALSE;

	EnterCriticalSection(&iad->cs);

	// Get bounds
	if (!GetObject(hBmp, sizeof(BITMAP), &bm))
	{
		LeaveCriticalSection(&iad->cs);
		return FALSE;
	}

	if (iad->width_based)
	{
		new_width = max(bm.bmWidth, iad->width);
		new_height = iad->height + bm.bmHeight - iad->nodes[pos].height;
	}
	else
	{
		new_width = bm.bmWidth + iad->width - iad->nodes[pos].width;
		new_height = max(iad->height, bm.bmHeight);
	}

	// Alloc image
	hNewBmp = ImageArray_CreateBitmapPoint(new_width, new_height, &(iad->lpBits));
	if (hNewBmp == NULL)
	{
		LeaveCriticalSection(&iad->cs);
		return FALSE;
	}

	// Move image...

	// Set some draw states
	SelectObject(iad->hdc, hNewBmp);
	hdc_old = CreateCompatibleDC(iad->hdc);

	SetBkMode(iad->hdc, TRANSPARENT);
	{
		POINT org;
		GetBrushOrgEx(iad->hdc, &org);
		SetStretchBltMode(iad->hdc, HALFTONE);
		SetBrushOrgEx(iad->hdc, org.x, org.y, NULL);
	}

	{
		int x = 0, y = 0, w = 0, h = 0;

		// 1- old data
		if (pos > 0)
		{
			SelectObject(hdc_old, iad->img);

			if (iad->width_based)
			{
				w = iad->width;
				h = 0;
				for (i = 0; i < pos; i++)
				{
					h += iad->nodes[i].height;
				}
			}
			else
			{
				h = iad->height;
				w = 0;
				for (i = 0; i < pos; i++)
				{
					w += iad->nodes[i].width;
				}
			}
			BitBlt(iad->hdc, 0, 0, w, h, hdc_old, 0, 0, SRCCOPY);
		}

		// 2- new image
		if (iad->width_based)
		{
			x = 0;
			y = h;
		}
		else
		{
			x = w;
			y = 0;
		}
		SelectObject(hdc_old, hBmp);
		BitBlt(iad->hdc, x, y, bm.bmWidth, bm.bmHeight, hdc_old, 0, 0, SRCCOPY);

		// 3- old data
		if (pos < iad->nodes_size - 1)
		{
			int ox, oy;

			SelectObject(hdc_old, iad->img);

			if (iad->width_based)
			{
				ox = 0;
				oy = y + iad->nodes[pos].height;

				x = 0;
				y += bm.bmHeight;

				w = iad->width;
				h = iad->height - h - iad->nodes[pos].height;
			}
			else
			{
				ox = x + iad->nodes[pos].width;
				oy = 0;

				x += bm.bmWidth;
				y = 0;

				w = iad->width - w - iad->nodes[pos].width;
				h = iad->height;
			}
			BitBlt(iad->hdc, x, y, w, h, hdc_old, ox, oy, SRCCOPY);
		}
	}

	// restore things
	DeleteDC(hdc_old);
	if (iad->img != NULL) DeleteObject(iad->img);
	iad->img = hNewBmp;

	// Move array
	iad->nodes[pos].width = bm.bmWidth;
	iad->nodes[pos].height = bm.bmHeight;

	iad->width = new_width;
	iad->height = new_height;

	// Finished it!
	LeaveCriticalSection(&iad->cs);

	return pos;
}


// Remove an image
BOOL ImageArray_RemoveImage(LP_IMAGE_ARRAY_DATA iad, int pos)
{
	int new_width, new_height;
	HBITMAP hNewBmp;
	HDC hdc_old;
	int i;

	if (pos < 0)
		return FALSE;

	if (pos >= iad->nodes_size)
		return FALSE;

	EnterCriticalSection(&iad->cs);

	// Get bounds
	if (iad->width_based)
	{
		new_width = iad->width;
		new_height = iad->height - iad->nodes[pos].height;
	}
	else
	{
		new_width = iad->width - iad->nodes[pos].width;
		new_height = iad->height;
	}

	// Alloc image
	hNewBmp = ImageArray_CreateBitmapPoint(new_width, new_height, &(iad->lpBits));
	if (hNewBmp == NULL)
	{
		LeaveCriticalSection(&iad->cs);
		return FALSE;
	}

	// Move image...

	// Set some draw states
	SelectObject(iad->hdc, hNewBmp);
	hdc_old = CreateCompatibleDC(iad->hdc);

	SetBkMode(iad->hdc, TRANSPARENT);
	{
		POINT org;
		GetBrushOrgEx(iad->hdc, &org);
		SetStretchBltMode(iad->hdc, HALFTONE);
		SetBrushOrgEx(iad->hdc, org.x, org.y, NULL);
	}

	{
		int x = 0, y = 0, w = 0, h = 0;

		if (pos > 0)
		{
			SelectObject(hdc_old, iad->img);

			if (iad->width_based)
			{
				w = iad->width;
				h = 0;
				for (i = 0; i < pos; i++)
				{
					h += iad->nodes[i].height;
				}
			}
			else
			{
				h = iad->height;
				w = 0;
				for (i = 0; i < pos; i++)
				{
					w += iad->nodes[i].width;
				}
			}
			BitBlt(iad->hdc, 0, 0, w, h, hdc_old, 0, 0, SRCCOPY);
		}

		if (pos < iad->nodes_size - 1)
		{
			int ox, oy;

			SelectObject(hdc_old, iad->img);

			if (iad->width_based)
			{
				ox = 0;
				oy = h + iad->nodes[pos].height;

				x = 0;
				y = h;

				w = iad->width;
				h = iad->height - h - iad->nodes[pos].height;
			}
			else
			{
				ox = w + iad->nodes[pos].width;
				oy = 0;

				x = w;
				y = 0;

				w = iad->width - w - iad->nodes[pos].width;
				h = iad->height;
			}
			BitBlt(iad->hdc, x, y, w, h, hdc_old, ox, oy, SRCCOPY);
		}
	}

	// restore things
	DeleteDC(hdc_old);
	if (iad->img != NULL) DeleteObject(iad->img);
	iad->img = hNewBmp;

	// Move array
	if (pos < iad->nodes_size - 1)
	{
		memmove(&iad->nodes[pos], &iad->nodes[pos + 1], (iad->nodes_size - pos - 1) * sizeof(IMAGE_ARRAY_DATA_NODE));
	}

	iad->nodes_size--;

	iad->width = new_width;
	iad->height = new_height;

	// Free array
	ImageArray_Alloc(iad, iad->nodes_size);

	// Finished it!
	LeaveCriticalSection(&iad->cs);

	return pos;
}



BOOL ImageArray_DrawImage(LP_IMAGE_ARRAY_DATA iad, int pos, HDC hdcDest, int nXDest, int nYDest, BYTE Alpha)
{
	if (hdcDest == NULL || pos < 0 || pos >= iad->nodes_size)
		return FALSE;

	EnterCriticalSection(&iad->cs);
	{
		int w, h, i;

		if (iad->width_based)
		{
			w = 0;
			h = 0;
			for (i = 0; i < pos; i++)
			{
				h += iad->nodes[i].height;
			}
		}
		else
		{
			h = 0;
			w = 0;
			for (i = 0; i < pos; i++)
			{
				w += iad->nodes[i].width;
			}
		}
		{
			BLENDFUNCTION bf = { AC_SRC_OVER, 0, Alpha, AC_SRC_ALPHA };
			/*ske_*/AlphaBlend(hdcDest, nXDest, nYDest, iad->nodes[pos].width, iad->nodes[pos].height, iad->hdc, w, h, iad->nodes[pos].width, iad->nodes[pos].height, bf);
		}
	}


	LeaveCriticalSection(&iad->cs);
	return FALSE;
}

BOOL ImageArray_GetImageSize(LP_IMAGE_ARRAY_DATA iad, int pos, SIZE * lpSize)
{
	EnterCriticalSection(&iad->cs);
	if (lpSize)
	{
		lpSize->cx = iad->nodes[pos].width;
		lpSize->cy = iad->nodes[pos].height;
	}
	LeaveCriticalSection(&iad->cs);
	return TRUE;
}