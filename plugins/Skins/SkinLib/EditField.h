#ifndef __EDIT_FIELD_H__
# define __EDIT_FIELD_H__

#include "ControlField.h"


class EditField : public ControlField
{
public:
	EditField(Dialog *dlg, const char *name, HWND hwnd);
	virtual ~EditField();

	virtual FieldType getType() const;

	virtual FieldState * createState(DialogState *dialogState);
};


#endif // __EDIT_FIELD_H__
