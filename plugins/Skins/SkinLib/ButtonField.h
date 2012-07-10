#ifndef __BUTTON_FIELD_H__
# define __BUTTON_FIELD_H__

#include "ControlField.h"

class ButtonField : public ControlField
{
public:
	ButtonField(Dialog *dlg, const char *name, HWND hwnd);
	virtual ~ButtonField();

	virtual FieldType getType() const;

	virtual FieldState * createState(DialogState *dialogState);
};



#endif // __BUTTON_FIELD_H__
