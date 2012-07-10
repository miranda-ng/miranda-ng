#include "globals.h"
#include "TextField.h"
#include "TextFieldState.h"


TextField::TextField(Dialog *dlg, const char *name) 
		: Field(dlg, name), hFont(NULL), fontColor(RGB(0,0,0))
{
}


TextField::~TextField()
{
}


FieldType TextField::getType() const
{
	return SIMPLE_TEXT;
}


void TextField::setText(const TCHAR *text)
{
	if (this->text == text)
		return;

	this->text = text;
	fireOnChange();
}


const TCHAR * TextField::getText() const
{
	return text.c_str();
}


void TextField::setFont(HFONT hFont)
{
	if (this->hFont == hFont)
		return;

	this->hFont = hFont;
	fireOnChange();
}


HFONT TextField::getFont() const
{
	if (hFont != NULL)
		return hFont;
	
	// The default is the GUI font
	return (HFONT) GetStockObject(DEFAULT_GUI_FONT);
}


COLORREF TextField::getFontColor() const
{
	return fontColor;
}


void TextField::setFontColor(COLORREF color)
{
	if (fontColor == color)
		return;

	fontColor = color;
	fireOnChange();
}


FieldState * TextField::createState(DialogState *dialogState)
{
	return new TextFieldState(dialogState, this);
}
