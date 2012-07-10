#ifndef __TEXT_FIELD_STATE_H__
# define __TEXT_FIELD_STATE_H__

#include "TextField.h"
#include "FieldState.h"
#include "FontState.h"


class TextFieldState : public FieldState
{
public:
	virtual ~TextFieldState();

	virtual TextField * getField() const;

	virtual Size getPreferedSize() const;

	virtual const TCHAR * getText() const;
	virtual void setText(const TCHAR *text);

	virtual FontState * getFont();
	virtual const FontState * getFont() const;

	virtual bool isEmpty() const;

private:
	TextFieldState(DialogState *dialog, TextField *field);

	FontState font;
	BOOL textSet;
	std::tstring text;


	friend class TextField;
};


#endif // __TEXT_FIELD_STATE_H__