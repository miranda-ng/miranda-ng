/*
Copyright (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "TxDIB.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Litehtml interface

CRITICAL_SECTION cairo_font::m_sync;

cairo_surface_t *dib_to_surface(CTxDIB &img)
{
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, img.getWidth(), img.getHeight());
	unsigned char *dst = cairo_image_surface_get_data(surface);
	unsigned char *src = (unsigned char *)img.getBits();
	int line_size = img.getWidth() * 4;
	int dst_offset = img.getWidth() * (img.getHeight() - 1) * 4;
	int src_offset = 0;
	for (int i = 0; i < img.getHeight(); i++, src_offset += line_size, dst_offset -= line_size) {
		memcpy(dst + dst_offset, src + src_offset, line_size);
	}
	cairo_surface_mark_dirty(surface);
	return surface;
}

void NSWebPage::on_image_loaded(const wchar_t *file, const wchar_t *url, bool redraw_only)
{
	if (!mir_wstrncmp(file, L"file://", 7))
		file += 7;

	CTxDIB img;
	if (img.load(file)) {
		cairo_surface_t *surface = dib_to_surface(img);
		m_images.add_image(T2Utf(url).get(), surface);

		PostMessage(ctrl.m_hwnd, NSM_IMAGE_LOADED, redraw_only, 0);
	}
}

////////////////////////////////////////////////////////////////////////////////

cairo_surface_t* NSWebPage::get_image(const std::string &url)
{
	return m_images.get_image(url);
}

void NSWebPage::get_client_rect(litehtml::position &pos) const
{
	pos = litehtml::size(ctrl.cachedWindowWidth, ctrl.cachedWindowHeight);

}

void NSWebPage::import_css(litehtml::string &, const litehtml::string &, litehtml::string &)
{
}

void NSWebPage::load_image(const char *src, const char */*baseUrl*/, bool redraw_on_ready)
{
	Utf2T wszUrl(src);
	if (m_images.reserve(src))
		on_image_loaded(wszUrl, wszUrl, redraw_on_ready);
}

void NSWebPage::on_anchor_click(const char *pszUtl, const litehtml::element::ptr &)
{
	Utils_OpenUrl(pszUtl);
}

void NSWebPage::set_base_url(const char *)
{
}

void NSWebPage::set_caption(const char *)
{
}

void NSWebPage::set_clip(const litehtml::position &, const litehtml::border_radiuses &)
{
}

void NSWebPage::set_cursor(const char *pszCursor)
{
	if (!mir_strcmp(pszCursor, "pointer"))
		SetCursor(LoadCursor(NULL, IDC_HAND));
	else
		SetCursor(LoadCursor(NULL, IDC_ARROW));
}
