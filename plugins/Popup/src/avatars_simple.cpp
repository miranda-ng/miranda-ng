/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

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
*/

#include "headers.h"

SimpleAvatar::SimpleAvatar(HANDLE h, bool bUseBitmap) :
PopupAvatar()
{
	bIsAnimated = false;
	bIsValid = true;

	if (bUseBitmap)
	{
		BITMAP bmp;
		GetObject((HBITMAP)h, sizeof(bmp), &bmp);
		width = abs(bmp.bmWidth);
		height = abs(bmp.bmHeight);

		avNeedFree = true;
		av = new avatarCacheEntry;
		av->bmHeight = abs(bmp.bmHeight);
		av->bmWidth = abs(bmp.bmWidth);
		av->hbmPic = (HBITMAP)h;
		av->dwFlags = AVS_BITMAP_VALID;
		return;
	}

	if (h && ServiceExists(MS_AV_GETAVATARBITMAP))
	{
		avNeedFree = false;
		av = (avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)h, 0);
		if (av)
		{
			if (av->hbmPic && (av->dwFlags&AVS_BITMAP_VALID) && !(av->dwFlags&AVS_HIDEONCLIST) && !(av->dwFlags&AVS_NOTREADY))
			{
				width = av->bmWidth;
				height = av->bmHeight;
				return;
			}

			if (av->dwFlags&AVS_NOTREADY)
				bIsValid = false;
		}
	}

	width = height = 0;
	av = NULL;
	avNeedFree = false;
}

SimpleAvatar::~SimpleAvatar()
{
	if (avNeedFree) delete av;
}

int SimpleAvatar::activeFrameDelay()
{
	return -1;
}

void SimpleAvatar::draw(MyBitmap *bmp, int x, int y, int w, int h, POPUPOPTIONS *options)
{
	if (!av) return;

	HRGN rgn;
	if (options->avatarRadius)
	{
		rgn = CreateRoundRectRgn(x, y, x + w, y + h, 2 * options->avatarRadius, 2 * options->avatarRadius);
		SelectClipRgn(bmp->getDC(), rgn);
	}
	else
	{
		rgn = CreateRectRgn(x, y, x + w, y + h);
	}

	HDC hdcTmp = CreateCompatibleDC(bmp->getDC());
	SelectObject(hdcTmp, av->hbmPic);
	SetStretchBltMode(bmp->getDC(), HALFTONE);


	if (av->dwFlags & AVS_HASTRANSPARENCY)
	{
		BLENDFUNCTION bf;
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = 255;
		bf.AlphaFormat = AC_SRC_ALPHA;
		AlphaBlend(bmp->getDC(), x, y, w, h, hdcTmp, 0, 0, av->bmWidth, av->bmHeight, bf);

		if (options->avatarBorders && options->avatarPNGBorders)
		{
			HBRUSH hbr = CreateSolidBrush(fonts.clAvatarBorder);
			bmp->saveAlpha(x, y, w, h);
			FrameRgn(bmp->getDC(), rgn, hbr, 1, 1);
			DeleteObject(hbr);
			bmp->restoreAlpha(x, y, w, h);
		}
	}
	else {
		bmp->saveAlpha(x, y, w, h);
		StretchBlt(bmp->getDC(), x, y, w, h, hdcTmp, 0, 0, av->bmWidth, av->bmHeight, SRCCOPY);
		if (options->avatarBorders){
			HBRUSH hbr = CreateSolidBrush(fonts.clAvatarBorder);
			FrameRgn(bmp->getDC(), rgn, hbr, 1, 1);
			DeleteObject(hbr);
		}
		bmp->restoreAlpha(x, y, w, h);
	}
	DeleteObject(rgn);
	SelectClipRgn(bmp->getDC(), NULL);
	DeleteDC(hdcTmp);
}
