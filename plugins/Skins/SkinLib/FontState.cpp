#include "globals.h"
#include "FontState.h"


FontState::FontState(HFONT hFont, COLORREF aColor) : hFont(NULL), externalFont(false), color(aColor)
{
	setHFONT(hFont);
}

FontState::~FontState()
{
	releaseHFONT();
}

void FontState::rebuildHFONT()
{
	releaseHFONT();
	buildHFONT();
}

void FontState::buildAttribs()
{
	LOGFONT lf = {0};
	if (hFont == NULL || GetObject(hFont, sizeof(lf), &lf) == 0)
	{
		face = _T("Tahoma");
		size = 9;
		italic = false;
		bold = false;
		underline = false;
		strikeout = false;

		rebuildHFONT();

		return;
	}

	face = lf.lfFaceName;
	italic = (lf.lfItalic != 0);
	bold = (lf.lfWeight > FW_NORMAL);
	underline = (lf.lfUnderline != 0);
	strikeout = (lf.lfStrikeOut != 0);

	HDC hdc = GetDC(NULL);
	size = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(hdc, LOGPIXELSY));
	ReleaseDC(NULL, hdc);
}

void FontState::buildHFONT()
{
	if (hFont != NULL)
		return;

	LOGFONT lf;
	
	_tcscpy(lf.lfFaceName, getFace());

	lf.lfWidth = lf.lfEscapement = lf.lfOrientation = 0;
	lf.lfWeight = isBold() ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = isItalic() ? 1 : 0;
	lf.lfUnderline = isUnderline() ? 1 : 0;
	lf.lfStrikeOut = isStrikeOut() ? 1 : 0;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	HDC hdc = GetDC(NULL);
	lf.lfHeight = -MulDiv(getSize(), GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(NULL, hdc);

	hFont = CreateFontIndirect(&lf);
	externalFont = false;
}

void FontState::releaseHFONT()
{
	if (hFont == NULL)
		return;

	if (!externalFont)
		DeleteObject(hFont);

	hFont = NULL;
}


HFONT FontState::getHFONT() const
{
	return hFont;
}


HFONT FontState::createHFONT() const
{
	LOGFONT lf;
	if (hFont == NULL || GetObject(hFont, sizeof(lf), &lf) == 0)
		return NULL;
	else
		return CreateFontIndirect(&lf);
}

void FontState::setHFONT(HFONT hFont)
{
	releaseHFONT();
	this->hFont = hFont;
	externalFont = true;
	buildAttribs();
}

const TCHAR * FontState::getFace() const
{
	return face.c_str();
}

void FontState::setFace(const TCHAR * face)
{
	this->face = face;
	rebuildHFONT();
}

int FontState::getSize() const
{
	return size;
}

void FontState::setSize(int size)
{
	this->size = size;
	rebuildHFONT();
}

COLORREF FontState::getColor() const
{
	return color;
}

void FontState::setColor(COLORREF color)
{
	this->color = color;
}

bool FontState::isItalic() const
{
	return italic;
}

void FontState::setItalic(bool italic)
{
	this->italic = italic;
	rebuildHFONT();
}

bool FontState::isBold() const
{
	return bold;
}

void FontState::setBold(bool bold)
{
	this->bold = bold;
	rebuildHFONT();
}

bool FontState::isUnderline() const
{
	return underline;
}

void FontState::setUnderline(bool underline)
{
	this->underline = underline;
	rebuildHFONT();
}

bool FontState::isStrikeOut() const
{
	return strikeout;
}

void FontState::setStrikeOut(bool strikeout)
{
	this->strikeout = strikeout;
	rebuildHFONT();
}
