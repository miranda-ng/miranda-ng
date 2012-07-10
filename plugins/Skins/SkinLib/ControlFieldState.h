#ifndef __LABEL_CONTROL_FIELD_STATE_H__
# define __LABEL_CONTROL_FIELD_STATE_H__

#include "ControlField.h"
#include "FieldState.h"
#include "FontState.h"


class ControlFieldState : public FieldState
{
public:
	virtual ~ControlFieldState();

	virtual ControlField * getField() const;

	virtual Size getPreferedSize() const = 0;

	virtual const TCHAR * getText() const;
	virtual void setText(const TCHAR *text);

	virtual FontState * getFont();
	virtual const FontState * getFont() const;

protected:
	ControlFieldState(DialogState *dialog, ControlField *field);

	virtual Size getTextPreferedSize(unsigned int format) const;

private:
	FontState font;

	bool textSet;
	std::tstring text;

	friend class ControlField;
};


#endif // __LABEL_CONTROL_FIELD_STATE_H__