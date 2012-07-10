#include "globals.h"
#include "TextFieldState.h"


TextFieldState::TextFieldState(DialogState *dialog, TextField *field) 
		: FieldState(dialog, field), font(field->getFont(), field->getFontColor()), textSet(false)
{
}

TextFieldState::~TextFieldState()
{
}

TextField * TextFieldState::getField() const
{
	return (TextField *) FieldState::getField();
}


Size TextFieldState::getPreferedSize() const
{
	HDC hdc = CreateCompatibleDC(NULL);

	HFONT newFont = getFont()->getHFONT();
	HFONT oldFont = (HFONT) SelectObject(hdc, newFont);

	RECT rc = {0};
	const TCHAR *text = getText();
	DrawText(hdc, text, -1, &rc, DT_CALCRECT | DT_NOPREFIX | DT_EXPANDTABS | DT_SINGLELINE);

	SelectObject(hdc, oldFont);

	DeleteDC(hdc);

	return Size(rc.right - rc.left, rc.bottom - rc.top);
}

const TCHAR * TextFieldState::getText() const
{
	if (textSet)
		return text.c_str();

	return getField()->getText();
}

void TextFieldState::setText(const TCHAR *text)
{
	this->text = text;
	textSet = true;
}

FontState * TextFieldState::getFont()
{
	return &font;
}

const FontState * TextFieldState::getFont() const
{
	return &font;
}

bool TextFieldState::isEmpty() const
{
	return lstrlen(getText()) <= 0;
}
