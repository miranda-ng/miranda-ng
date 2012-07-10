#ifndef __LABEL_CONTROL_FIELD_H__
# define __LABEL_CONTROL_FIELD_H__

#include "Field.h"

class ControlField : public Field
{
public:
	ControlField(Dialog *dlg, const char *name, HWND hwnd);
	virtual ~ControlField();

	virtual HWND getHWND();

	virtual void setText(const TCHAR *text);
	virtual const TCHAR * getText();

	virtual void setFont(HFONT hFont);
	virtual HFONT getFont() const;

	virtual COLORREF getFontColor() const;

	virtual int getBorderSize() const;

	virtual bool isScrollVisible(bool horizontal) const;

private:
	HWND hwnd;

	bool textSet;
	std::tstring text;

	HFONT hFont;
};



#endif // __LABEL_CONTROL_FIELD_H__
