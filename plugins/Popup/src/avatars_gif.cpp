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

GifAvatar::GifAvatar(MCONTACT hContact) : PopupAvatar()
{
	av = (avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
	bIsAnimated = true;
	bIsValid = true;
	GDIPlus_GetGIFSize(av->szFilename, &this->width, &this->height);

	hBitmap = NULL;
	frameDelays = NULL;
	frameCount = 0;
	frameSize.cx = frameSize.cy = 0;

	cachedHeight = cachedWidth = -1;

	activeFrame = 0;
}

GifAvatar::~GifAvatar()
{
	if (frameDelays) {
		mir_free(frameDelays);
		frameDelays = NULL;
	}
	if (hBitmap) DeleteObject(hBitmap);
}

int GifAvatar::activeFrameDelay()
{
	if (!av || !frameCount || !frameDelays || !hBitmap || (activeFrame < 0) || (activeFrame >= frameCount))
		return -1;
	return frameDelays[activeFrame];
}

void GifAvatar::draw(MyBitmap *bmp, int x, int y, int w, int h, POPUPOPTIONS *options)
{
	if (!av || (w <= 0) || (h <= 0)) return;

	if (!frameCount || !frameDelays || !hBitmap || (cachedWidth != w) || (cachedHeight != h))
	{
		cachedWidth = w;
		cachedHeight = h;
		if (frameDelays) {
			mir_free(frameDelays);
			frameDelays = NULL;
		}
		if (hBitmap) DeleteObject(hBitmap);
		GDIPlus_ExtractAnimatedGIF(av->szFilename, w, h, &hBitmap, &frameDelays, &frameCount, &frameSize);
	}

	if (!frameCount) return;

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
	SelectObject(hdcTmp, hBitmap);
	SetStretchBltMode(bmp->getDC(), HALFTONE);


	if (av->dwFlags & AVS_PREMULTIPLIED)
	{
		BLENDFUNCTION bf;
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = 255;
		bf.AlphaFormat = AC_SRC_ALPHA;
		AlphaBlend(bmp->getDC(), x, y, w, h, hdcTmp, frameSize.cx*activeFrame, 0, frameSize.cx, frameSize.cy, bf);

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
		StretchBlt(bmp->getDC(), x, y, w, h, hdcTmp, frameSize.cx*activeFrame, 0, frameSize.cx, frameSize.cy, SRCCOPY);
		if (options->avatarBorders) {
			HBRUSH hbr = CreateSolidBrush(fonts.clAvatarBorder);
			FrameRgn(bmp->getDC(), rgn, hbr, 1, 1);
			DeleteObject(hbr);
		}
		bmp->restoreAlpha(x, y, w, h);
	}
	DeleteObject(rgn);
	SelectClipRgn(bmp->getDC(), NULL);
	DeleteDC(hdcTmp);

	activeFrame = (activeFrame + 1) % frameCount;
}
