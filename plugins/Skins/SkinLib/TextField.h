#ifndef __TEXT_FIELD_H__
# define __TEXT_FIELD_H__

#include "Field.h"


class TextField : public Field
{
public:
	TextField(Dialog *dlg, const char *name);
	virtual ~TextField();

	virtual FieldType getType() const;

	virtual const TCHAR * getText() const;
	virtual void setText(const TCHAR *text);

	virtual HFONT getFont() const;
	virtual void setFont(HFONT hFont);

	virtual COLORREF getFontColor() const;
	virtual void setFontColor(COLORREF color);

	virtual FieldState * createState(DialogState *dialogState);

private:
	std::tstring text;
	HFONT hFont;
	COLORREF fontColor;

};



#endif // __TEXT_FIELD_H__
