/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-15 Miranda NG project,
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
*/

#include "commonheaders.h"

MIR_CORE_DLL(void) BmpFilterGetStrings(TCHAR *dest, size_t destLen)
{
	if (dest == NULL)
		return;

	CMString filter;
	filter.AppendFormat(_T("%s (*.bmp;*.jpg;*.gif;*.png)%c*.BMP;*.RLE;*.JPG;*.JPEG;*.GIF;*.PNG%c"), TranslateT("All Files"), 0, 0);
	filter.AppendFormat(_T("%s (*.bmp;*.rle)%c*.BMP;*.RLE%c"), TranslateT("Windows bitmaps"), 0, 0);
	filter.AppendFormat(_T("%s (*.jpg;*.jpeg)%c*.JPG;*.JPEG%c"), TranslateT("JPEG bitmaps"), 0, 0);
	filter.AppendFormat(_T("%s (*.gif)%c*.GIF%c"), TranslateT("GIF bitmaps"), 0, 0);
	filter.AppendFormat(_T("%s (*.png)%c*.PNG%c"), TranslateT("PNG bitmaps"), 0, 0);
	filter.AppendChar(0);

	_tcsncpy_s(dest, destLen, filter, filter.GetLength());
}
