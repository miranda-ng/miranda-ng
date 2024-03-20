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

/////////////////////////////////////////////////////////////////////////////////////////
// Litehtml interface

CRITICAL_SECTION cairo_font::m_sync;

cairo_surface_t* NSWebPage::get_image(const std::string &)
{
	return 0;
}

void NSWebPage::get_client_rect(litehtml::position &pos) const
{
	pos = litehtml::size(ctrl.cachedWindowWidth, ctrl.cachedWindowHeight);

}

void NSWebPage::import_css(litehtml::string &, const litehtml::string &, litehtml::string &)
{
}

void NSWebPage::load_image(const char *, const char *, bool)
{
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
