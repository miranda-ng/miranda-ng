#include "globals.h"
#include "EditField.h"
#include "EditFieldState.h"


EditField::EditField(Dialog *dlg, const char *name, HWND hwnd) 
		: ControlField(dlg, name, hwnd)
{
}

EditField::~EditField()
{
}

FieldType EditField::getType() const
{
	return CONTROL_EDIT;
}

FieldState * EditField::createState(DialogState *dialogState)
{
	return new EditFieldState(dialogState, this);
}