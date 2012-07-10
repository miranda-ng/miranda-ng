#include "globals.h"
#include "ControlFieldState.h"


ControlFieldState::ControlFieldState(DialogState *dialog, ControlField *field) 
		: FieldState(dialog, field), textSet(false), font(field->getFont(), field->getFontColor())
{
}


ControlFieldState::~ControlFieldState()
{
}


ControlField * ControlFieldState::getField() const
{
	return (ControlField *) FieldState::getField();
}


Size ControlFieldState::getTextPreferedSize(unsigned int format) const
{
	HDC hdc = CreateCompatibleDC(NULL);

	HFONT newFont = getFont()->getHFONT();
	HFONT oldFont = (HFONT) SelectObject(hdc, newFont);

	int width = 0;
	int height = 0;

	const TCHAR *text = getText();
	int len = lstrlen(text);
	if (len <= 0)
	{
		TEXTMETRIC tm = {0};
		GetTextMetrics(hdc, &tm);
		height = tm.tmHeight;
	}
	else
	{
		RECT rc = {0};
		if ((format & DT_SINGLELINE) == 0 && size.x >= 0)
		{
			format |= DT_WORDBREAK;
			rc.right = size.x;
		}
		DrawText(hdc, text, len, &rc, DT_CALCRECT | format);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;
	}

	SelectObject(hdc, oldFont);

	DeleteDC(hdc);

	return Size(width, height);
}


const TCHAR * ControlFieldState::getText() const
{
	if (textSet)
		return text.c_str();

	return getField()->getText();
}


void ControlFieldState::setText(const TCHAR *text)
{
	this->text = text;
	textSet = true;
}


FontState * ControlFieldState::getFont()
{
	return &font;
}


const FontState * ControlFieldState::getFont() const
{
	return &font;
}