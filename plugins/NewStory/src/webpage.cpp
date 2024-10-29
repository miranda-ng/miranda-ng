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

struct NSWebCache
{
	NSWebCache(NSWebPage *_1, const wchar_t *_2, ItemData *_3) :
		pPage(_1),
		pItem(_3),
		wszPath(_2)
	{}

	ItemData *pItem;
	NSWebPage *pPage;
	CMStringW  wszPath;
};

static int CompareFiles(const NSWebCache *p1, const NSWebCache *p2)
{
	return mir_wstrcmp(p1->wszPath, p2->wszPath);
}

static mir_cs g_csMissingFiles;
static OBJLIST<NSWebCache> g_arMissingFiles(10, CompareFiles);

INT_PTR SvcFileReady(WPARAM wParam, LPARAM)
{
	NSWebCache tmp(0, (const wchar_t *)wParam, 0);

	mir_cslock lck(g_csMissingFiles);
	if (auto *pCache = g_arMissingFiles.find(&tmp)) {
		pCache->pPage->load_image(tmp.wszPath, pCache->pItem);
		pCache->pItem->m_doc = 0;
		pCache->pItem->savedHeight = -1;
		pCache->pItem->setText();
		g_arMissingFiles.remove(pCache);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static std::set<std::wstring> g_installed_fonts;

int CALLBACK EnumFontsProc(const LOGFONTW *lplf, const TEXTMETRIC *, DWORD, LPARAM)
{
	g_installed_fonts.insert(lplf->lfFaceName);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Litehtml interface

struct
{
	const char *name;
	int color_index;
}
static colors[] = {
	{ "ActiveBorder", COLOR_ACTIVEBORDER },
	{ "ActiveCaption", COLOR_ACTIVECAPTION },
	{ "AppWorkspace", COLOR_APPWORKSPACE },
	{ "Background", COLOR_BACKGROUND },
	{ "ButtonFace", COLOR_BTNFACE },
	{ "ButtonHighlight", COLOR_BTNHIGHLIGHT },
	{ "ButtonShadow", COLOR_BTNSHADOW },
	{ "ButtonText", COLOR_BTNTEXT },
	{ "CaptionText", COLOR_CAPTIONTEXT },
	{ "GrayText", COLOR_GRAYTEXT },
	{ "Highlight", COLOR_HIGHLIGHT },
	{ "HighlightText", COLOR_HIGHLIGHTTEXT },
	{ "InactiveBorder", COLOR_INACTIVEBORDER },
	{ "InactiveCaption", COLOR_INACTIVECAPTION },
	{ "InactiveCaptionText", COLOR_INACTIVECAPTIONTEXT },
	{ "InfoBackground", COLOR_INFOBK },
	{ "InfoText", COLOR_INFOTEXT },
	{ "Menu", COLOR_MENU },
	{ "MenuText", COLOR_MENUTEXT },
	{ "Scrollbar", COLOR_SCROLLBAR },
	{ "ThreeDDarkShadow", COLOR_3DDKSHADOW },
	{ "ThreeDFace", COLOR_3DFACE },
	{ "ThreeDHighlight", COLOR_3DHILIGHT },
	{ "ThreeDLightShadow", COLOR_3DLIGHT },
	{ "ThreeDShadow", COLOR_3DSHADOW },
	{ "Window", COLOR_WINDOW },
	{ "WindowFrame", COLOR_WINDOWFRAME },
	{ "WindowText", COLOR_WINDOWTEXT }
};

std::string NSWebPage::resolve_color(const std::string &color) const
{
	char buf[20];

	if (color == "NSText") {
		mir_snprintf(buf, "#%02X%02X%02X", GetRValue(clText), GetGValue(clText), GetBValue(clText));
		return buf;
	}

	for (auto &clr : colors) {
		if (!t_strcasecmp(color.c_str(), clr.name)) {
			char  str_clr[20];
			DWORD rgb_color = GetSysColor(clr.color_index);
			t_snprintf(str_clr, 20, "#%02X%02X%02X", GetRValue(rgb_color), GetGValue(rgb_color), GetBValue(rgb_color));
			return std::move(std::string(str_clr));
		}
	}
	return std::move(std::string());
}

/////////////////////////////////////////////////////////////////////////////////////////

NSWebPage::NSWebPage(NewstoryListData &_1) :
	ctrl(_1)
{
	m_hClipRgn = NULL;
	m_tmp_hdc = GetDC(NULL);

	if (g_installed_fonts.empty()) {
		EnumFonts(m_tmp_hdc, NULL, EnumFontsProc, 0);
		g_installed_fonts.insert(L"monospace");
		g_installed_fonts.insert(L"serif");
		g_installed_fonts.insert(L"sans-serif");
		g_installed_fonts.insert(L"fantasy");
		g_installed_fonts.insert(L"cursive");
	}
}

NSWebPage::~NSWebPage()
{
	clear_images();
	{
		mir_cslock lck(g_csMissingFiles);
		for (auto &it : g_arMissingFiles.rev_iter())
			if (it->pPage == this)
				g_arMissingFiles.remove(g_arMissingFiles.indexOf(&it));
	}

	for (auto &it : m_fonts)
		delete_font(it.second.font);

	if (m_hClipRgn)
		DeleteObject(m_hClipRgn);

	ReleaseDC(NULL, m_tmp_hdc);
}

void NSWebPage::draw()
{
	ctrl.ScheduleDraw();
}

/////////////////////////////////////////////////////////////////////////////////////////
// former win32_container

static LPCWSTR get_exact_font_name(LPCWSTR facename)
{
	if (!lstrcmpi(facename, L"monospace"))
		return L"Courier New";
	if (!lstrcmpi(facename, L"serif"))
		return L"Times New Roman";
	if (!lstrcmpi(facename, L"sans-serif"))
		return L"Arial";
	if (!lstrcmpi(facename, L"fantasy"))
		return L"Impact";
	if (!lstrcmpi(facename, L"cursive"))
		return L"Comic Sans MS";
	return facename;
}

static void trim_quotes(std::string &str)
{
	if (str.front() == '"' || str.front() == '\'')
		str.erase(0, 1);

	if (str.back() == '"' || str.back() == '\'')
		str.erase(str.length() - 1, 1);
}

uint_ptr NSWebPage::create_font(const char *font_list, int size, int weight, font_style italic, unsigned int decoration, font_metrics *fm)
{
	std::wstring font_name;
	string_vector fonts;
	split_string(font_list, fonts, ",");
	bool found = false;
	for (auto &name : fonts) {
		trim(name);
		trim_quotes(name);
		Utf2T wname(name.c_str());
		if (g_installed_fonts.count(wname.get())) {
			font_name = wname;
			found = true;
			break;
		}
	}
	if (!found) font_name = Utf2T(get_default_font_name());
	font_name = get_exact_font_name(font_name.c_str());

	LOGFONT lf = {};
	wcscpy_s(lf.lfFaceName, LF_FACESIZE, font_name.c_str());

	lf.lfHeight = -size;
	lf.lfWeight = weight;
	lf.lfItalic = (italic == font_style_italic) ? TRUE : FALSE;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfStrikeOut = (decoration & font_decoration_linethrough) ? TRUE : FALSE;
	lf.lfUnderline = (decoration & font_decoration_underline) ? TRUE : FALSE;
	HFONT hFont = CreateFontIndirect(&lf);

	if (fm) {
		SelectObject(m_tmp_hdc, hFont);
		TEXTMETRIC tm = {};
		GetTextMetrics(m_tmp_hdc, &tm);
		fm->ascent = tm.tmAscent;
		fm->descent = tm.tmDescent;
		fm->height = tm.tmHeight;
		fm->x_height = tm.tmHeight / 2;   // this is an estimate; call GetGlyphOutline to get the real value
		fm->draw_spaces = italic || decoration;
	}

	return (uint_ptr)hFont;
}

void NSWebPage::delete_font(uint_ptr hFont)
{
	DeleteObject((HFONT)hFont);
}

const char *NSWebPage::get_default_font_name() const
{
	return "Times New Roman";
}

int NSWebPage::get_default_font_size() const
{
	return 16;
}

int NSWebPage::text_width(const char *text, uint_ptr hFont)
{
	SIZE size = {};
	SelectObject(m_tmp_hdc, (HFONT)hFont);
	Utf2T wtext(text);
	GetTextExtentPoint32(m_tmp_hdc, wtext, (int)mir_wstrlen(wtext), &size);
	return size.cx;
}

void NSWebPage::draw_text(uint_ptr hdc, const char *text, uint_ptr hFont, web_color color, const position &pos)
{
	apply_clip((HDC)hdc);

	HFONT oldFont = (HFONT)SelectObject((HDC)hdc, (HFONT)hFont);

	SetBkMode((HDC)hdc, TRANSPARENT);

	SetTextColor((HDC)hdc, RGB(color.red, color.green, color.blue));

	RECT rcText = { pos.left(), pos.top(), pos.right(), pos.bottom() };
	DrawText((HDC)hdc, Utf2T(text), -1, &rcText, DT_SINGLELINE | DT_NOPREFIX | DT_BOTTOM | DT_NOCLIP);

	SelectObject((HDC)hdc, oldFont);

	release_clip((HDC)hdc);
}

int NSWebPage::pt_to_px(int pt) const
{
	return MulDiv(pt, GetDeviceCaps(m_tmp_hdc, LOGPIXELSY), 72);
}

void NSWebPage::draw_solid_fill(uint_ptr _hdc, const background_layer &bg, const web_color &color)
{
	HDC hdc = (HDC)_hdc;
	apply_clip(hdc);

	fill_rect(hdc, bg.border_box.x, bg.border_box.y, bg.border_box.width, bg.border_box.height, color);

	release_clip(hdc);
}

void NSWebPage::draw_linear_gradient(uint_ptr, const background_layer &, const background_layer::linear_gradient &)
{}

void NSWebPage::draw_radial_gradient(uint_ptr, const background_layer &, const background_layer::radial_gradient &)
{}

void NSWebPage::draw_conic_gradient(uint_ptr, const background_layer &, const background_layer::conic_gradient &)
{}

void NSWebPage::draw_list_marker(uint_ptr hdc, const list_marker &marker)
{
	apply_clip((HDC)hdc);

	int top_margin = marker.pos.height / 3;
	if (top_margin < 4)
		top_margin = 0;

	int draw_x = marker.pos.x;
	int draw_y = marker.pos.y + top_margin;
	int draw_width = marker.pos.height - top_margin * 2;
	int draw_height = marker.pos.height - top_margin * 2;

	switch (marker.marker_type) {
	case list_style_type_circle:
		{
			draw_ellipse((HDC)hdc, draw_x, draw_y, draw_width, draw_height, marker.color, 1);
		}
		break;
	case list_style_type_disc:
		{
			fill_ellipse((HDC)hdc, draw_x, draw_y, draw_width, draw_height, marker.color);
		}
		break;
	case list_style_type_square:
		{
			fill_rect((HDC)hdc, draw_x, draw_y, draw_width, draw_height, marker.color);
		}
		break;
	}
	release_clip((HDC)hdc);
}

FIBITMAP* NSWebPage::load_image(const wchar_t *pwszUrl, ItemData *pItem)
{
	mir_cslockfull lck(m_csImages);
	auto img = m_images.find(pwszUrl);
	if (img != m_images.end() && img->second)
		return (FIBITMAP *)img->second;

	if (uint_ptr newImg = get_image(pwszUrl, false)) {
		add_image(pwszUrl, newImg);
		return (FIBITMAP *)newImg;
	}

	NSWebCache tmp(this, pwszUrl, pItem);
	mir_cslock lck2(g_csMissingFiles);
	if (!g_arMissingFiles.find(&tmp))
		g_arMissingFiles.insert(new NSWebCache(tmp));

	return g_plugin.m_pNoImage;
}

void NSWebPage::load_image(const char *src, const char */*baseurl*/, bool redraw_on_ready)
{
	std::wstring url = Utf2T(src);

	mir_cslockfull lck(m_csImages);
	if (m_images.count(url) == 0) {
		lck.unlock();

		if (uint_ptr img = get_image(url.c_str(), redraw_on_ready))
			add_image(url.c_str(), img);
	}
}

void NSWebPage::add_image(LPCWSTR url, uint_ptr img)
{
	mir_cslock lck(m_csImages);
	m_images[url] = img;
}

FIBITMAP* NSWebPage::find_image(const wchar_t *pwszUrl)
{
	mir_cslock lck(m_csImages);
	auto img = m_images.find(pwszUrl);
	if (img != m_images.end() && img->second)
		return (FIBITMAP *)img->second;

	return nullptr;
}

void NSWebPage::get_image_size(const char *src, const char * /*baseurl*/, size &sz)
{
	std::wstring url = Utf2T(src);

	mir_cslock lck(m_csImages);
	images_map::iterator img = m_images.find(url);
	if (img != m_images.end() && img->second)
		get_img_size(img->second, sz);
	else
		sz.width = sz.height = 48;
}

void NSWebPage::clear_images()
{
	mir_cslock lck(m_csImages);
	for (auto &img : m_images)
		if (img.second)
			delete (FIBITMAP *)img.second;

	m_images.clear();
}

void NSWebPage::set_clip(const position &pos, const border_radiuses &)
{
	m_clips.push_back(pos);
}

void NSWebPage::del_clip()
{
	if (!m_clips.empty())
		m_clips.pop_back();
}

void NSWebPage::apply_clip(HDC hdc)
{
	if (m_hClipRgn) {
		DeleteObject(m_hClipRgn);
		m_hClipRgn = NULL;
	}

	if (!m_clips.empty()) {
		POINT ptView = { 0, 0 };
		GetWindowOrgEx(hdc, &ptView);

		position clip_pos = m_clips.back();
		m_hClipRgn = CreateRectRgn(clip_pos.left() - ptView.x, clip_pos.top() - ptView.y, clip_pos.right() - ptView.x, clip_pos.bottom() - ptView.y);
		SelectClipRgn(hdc, m_hClipRgn);
	}
}

void NSWebPage::release_clip(HDC hdc)
{
	SelectClipRgn(hdc, NULL);

	if (m_hClipRgn) {
		DeleteObject(m_hClipRgn);
		m_hClipRgn = NULL;
	}
}

element::ptr NSWebPage::create_element(const char *, const string_map &, const document::ptr &)
{
	return 0;
}

void NSWebPage::get_media_features(media_features &media)  const
{
	position client;
	get_client_rect(client);

	media.type = media_type_screen;
	media.width = client.width;
	media.height = client.height;
	media.color = 8;
	media.monochrome = 0;
	media.color_index = 256;
	media.resolution = GetDeviceCaps(m_tmp_hdc, LOGPIXELSX);
	media.device_width = GetDeviceCaps(m_tmp_hdc, HORZRES);
	media.device_height = GetDeviceCaps(m_tmp_hdc, VERTRES);
}

void NSWebPage::get_language(std::string &language, std::string &culture) const
{
	language = "en";
	culture = "";
}

void NSWebPage::transform_text(std::string &text, text_transform tt)
{
	if (text.empty()) return;

	LPWSTR txt = _wcsdup(Utf2T(text.c_str()));
	switch (tt) {
	case text_transform_capitalize:
		CharUpperBuff(txt, 1);
		break;
	case text_transform_uppercase:
		CharUpperBuff(txt, lstrlen(txt));
		break;
	case text_transform_lowercase:
		CharLowerBuff(txt, lstrlen(txt));
		break;
	}
	text = T2Utf(txt);
	free(txt);
}

void NSWebPage::link(const document::ptr &, const element::ptr &)
{}

/////////////////////////////////////////////////////////////////////////////////////////
// GDI+ part (former gdiplus_container)

static void FreeImage_Draw(HDC hdc, FIBITMAP *pdib, int x, int y, int w, int h)
{
	BITMAPINFO *pbmpi = FreeImage_GetInfo(pdib);
	BYTE *pDibBits = FreeImage_GetBits(pdib);
	::SetDIBitsToDevice(hdc, x, y, w, h, 0, 0, 0, h, pDibBits, pbmpi, DIB_RGB_COLORS);
}

static Color gdiplus_color(web_color color)
{
	return Color(color.alpha, color.red, color.green, color.blue);
}

void NSWebPage::draw_ellipse(HDC hdc, int x, int y, int width, int height, web_color color, int)
{
	Graphics graphics(hdc);

	graphics.SetCompositingQuality(CompositingQualityHighQuality);
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);

	Pen pen(gdiplus_color(color));
	graphics.DrawEllipse(&pen, x, y, width, height);
}

void NSWebPage::fill_ellipse(HDC hdc, int x, int y, int width, int height, web_color color)
{
	Graphics graphics(hdc);

	graphics.SetCompositingQuality(CompositingQualityHighQuality);
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);

	SolidBrush brush(gdiplus_color(color));
	graphics.FillEllipse(&brush, x, y, width, height);
}

void NSWebPage::fill_rect(HDC hdc, int x, int y, int width, int height, web_color color)
{
	Graphics graphics(hdc);

	SolidBrush brush(gdiplus_color(color));
	graphics.FillRectangle(&brush, x, y, width, height);
}

void NSWebPage::get_img_size(uint_ptr img, size &sz)
{
	FIBITMAP *bmp = (FIBITMAP *)img;
	if (bmp) {
		sz.width = FreeImage_GetWidth(bmp);
		sz.height = FreeImage_GetHeight(bmp);
	}
}

void NSWebPage::draw_image(uint_ptr _hdc, const background_layer &bg, const std::string &src, const std::string& /*base_url*/)
{
	if (src.empty() || (!bg.clip_box.width && !bg.clip_box.height))
		return;

	std::wstring url = Utf2T(src.c_str());
	
	FIBITMAP *bgbmp = find_image(url.c_str());
	if (!bgbmp)
		bgbmp = g_plugin.m_pNoImage;

	Graphics graphics((HDC)_hdc);
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
	graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

	Gdiplus::Region reg(Rect(bg.border_box.left(), bg.border_box.top(), bg.border_box.width, bg.border_box.height));
	graphics.SetClip(&reg);

	FIBITMAP *scaled_img = nullptr;
	int iWidth = FreeImage_GetHeight(bgbmp), iHeight = FreeImage_GetWidth(bgbmp);
	if (bg.origin_box.width != iWidth || bg.origin_box.height != iHeight) {
		bgbmp = scaled_img = FreeImage_Rescale(bgbmp, bg.origin_box.width, bg.origin_box.height);
		iWidth = bg.origin_box.width; iHeight = bg.origin_box.height;
	}

	switch (bg.repeat) {
	case background_repeat_no_repeat:
		FreeImage_Draw((HDC)_hdc, bgbmp, bg.origin_box.x, bg.origin_box.y, iWidth, iHeight);
		break;

	case background_repeat_repeat_x:
		{
			int x = bg.origin_box.x;
			while (x > bg.clip_box.left())
				x -= iWidth;
			for (; x < bg.clip_box.right(); x += iWidth)
				FreeImage_Draw((HDC)_hdc, bgbmp, x, bg.origin_box.y, iWidth, iHeight);
		}
		break;
	case background_repeat_repeat_y:
		{
			int y = bg.origin_box.y;
			while (y > bg.clip_box.top()) y -= iHeight;
			for (; y < bg.clip_box.bottom(); y += iHeight)
				FreeImage_Draw((HDC)_hdc, bgbmp, bg.origin_box.x, y, iWidth, iHeight);
		}
		break;
	case background_repeat_repeat:
		{
			int x = bg.origin_box.x;
			while (x > bg.clip_box.left()) x -= iWidth;
			int y0 = bg.origin_box.y;
			while (y0 > bg.clip_box.top()) y0 -= iHeight;

			for (; x < bg.clip_box.right(); x += iWidth)
				for (int y = y0; y < bg.clip_box.bottom(); y += iHeight)
					FreeImage_Draw((HDC)_hdc, bgbmp, x, y, iWidth, iHeight);
		}
		break;
	}

	if (scaled_img)
		FreeImage_Unload(scaled_img);
}

// length of dash and space for "dashed" style, in multiples of pen width
const float dash = 3;
const float space = 2;

static void draw_horz_border(Graphics &graphics, const border &border, int y, int left, int right)
{
	if (border.style != border_style_double || border.width < 3) {
		if (border.width == 1) right--; // 1px-wide lines are longer by one pixel in GDI+ (the endpoint is also drawn)
		Pen pen(gdiplus_color(border.color), (float)border.width);
		if (border.style == border_style_dotted) {
			float dashValues[2] = { 1, 1 };
			pen.SetDashPattern(dashValues, 2);
		}
		else if (border.style == border_style_dashed) {
			float dashValues[2] = { dash, space };
			pen.SetDashPattern(dashValues, 2);
		}
		graphics.DrawLine(&pen,
			Point(left, y + border.width / 2),
			Point(right, y + border.width / 2));
	}
	else {
		int single_line_width = (int)round(border.width / 3.);
		if (single_line_width == 1) right--;
		Pen pen(gdiplus_color(border.color), (float)single_line_width);
		graphics.DrawLine(&pen,
			Point(left, y + single_line_width / 2),
			Point(right, y + single_line_width / 2));
		graphics.DrawLine(&pen,
			Point(left, y + border.width - 1 - single_line_width / 2),
			Point(right, y + border.width - 1 - single_line_width / 2));
	}
}

static void draw_vert_border(Graphics &graphics, const border &border, int x, int top, int bottom)
{
	if (border.style != border_style_double || border.width < 3) {
		if (border.width == 1) bottom--;
		Pen pen(gdiplus_color(border.color), (float)border.width);
		if (border.style == border_style_dotted) {
			float dashValues[2] = { 1, 1 };
			pen.SetDashPattern(dashValues, 2);
		}
		else if (border.style == border_style_dashed) {
			float dashValues[2] = { dash, space };
			pen.SetDashPattern(dashValues, 2);
		}
		graphics.DrawLine(&pen,
			Point(x + border.width / 2, top),
			Point(x + border.width / 2, bottom));
	}
	else {
		int single_line_width = (int)round(border.width / 3.);
		if (single_line_width == 1) bottom--;
		Pen pen(gdiplus_color(border.color), (float)single_line_width);
		graphics.DrawLine(&pen,
			Point(x + single_line_width / 2, top),
			Point(x + single_line_width / 2, bottom));
		graphics.DrawLine(&pen,
			Point(x + border.width - 1 - single_line_width / 2, top),
			Point(x + border.width - 1 - single_line_width / 2, bottom));
	}
}

void NSWebPage::draw_borders(uint_ptr hdc, const borders &borders, const position &draw_pos, bool)
{
	apply_clip((HDC)hdc);
	Graphics graphics((HDC)hdc);

	if (borders.left.width != 0) {
		draw_vert_border(graphics, borders.left, draw_pos.left(), draw_pos.top(), draw_pos.bottom());
	}
	if (borders.right.width != 0) {
		draw_vert_border(graphics, borders.right, draw_pos.right() - borders.right.width, draw_pos.top(), draw_pos.bottom());
	}
	if (borders.top.width != 0) {
		draw_horz_border(graphics, borders.top, draw_pos.top(), draw_pos.left(), draw_pos.right());
	}
	if (borders.bottom.width != 0) {
		draw_horz_border(graphics, borders.bottom, draw_pos.bottom() - borders.bottom.width, draw_pos.left(), draw_pos.right());
	}

	release_clip((HDC)hdc);
}

/////////////////////////////////////////////////////////////////////////////////////////
// NSWebPage own functions

uint_ptr NSWebPage::get_image(LPCWSTR url_or_path, bool)
{
	if (!mir_wstrncmp(url_or_path, L"file://", 7))
		url_or_path += 7;

	return (uint_ptr)FreeImage_LoadU(FreeImage_GetFIFFromFilenameU(url_or_path), url_or_path);
}

void NSWebPage::get_client_rect(position &pos) const
{
	pos = size(ctrl.cachedWindowWidth, ctrl.cachedWindowHeight);
}

void NSWebPage::import_css(std::string &, const std::string &, std::string &)
{
}

void NSWebPage::on_anchor_click(const char *pszUtl, const element::ptr &)
{
	Utf2T wszUrl(pszUtl);

	DWORD dwType;
	const wchar_t *p = (!mir_wstrncmp(wszUrl, L"file://", 7)) ? wszUrl.get() + 7 : wszUrl.get();
	if (GetBinaryTypeW(p, &dwType)) {
		CMStringW wszText(FORMAT, L"%s\r\n\r\n%s", TranslateT("This url might launch an executable program or virus, are you sure?"), wszUrl.get());
		if (IDYES != MessageBoxW(0, wszText, TranslateT("Potentially dangerous URL"), MB_ICONWARNING | MB_YESNO))
			return;
	}
	Utils_OpenUrlW(wszUrl);
}

void NSWebPage::set_base_url(const char *)
{
}

void NSWebPage::set_caption(const char *)
{
}

void NSWebPage::set_cursor(const char *pszCursor)
{
	if (!mir_strcmp(pszCursor, "pointer"))
		SetCursor(LoadCursor(NULL, IDC_HAND));
	else
		SetCursor(LoadCursor(NULL, IDC_ARROW));
}
