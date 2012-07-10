#ifndef __LABEL_FIELD_H__
# define __LABEL_FIELD_H__

#include "ControlField.h"


class LabelField : public ControlField
{
public:
	LabelField(Dialog *dlg, const char *name, HWND hwnd);
	virtual ~LabelField();

	virtual FieldType getType() const;

	virtual FieldState * createState(DialogState *dialogState);
};



#endif // __LABEL_FIELD_H__
