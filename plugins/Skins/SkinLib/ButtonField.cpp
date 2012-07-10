#include "globals.h"
#include "ButtonField.h"
#include "ButtonFieldState.h"


ButtonField::ButtonField(Dialog *dlg, const char *name, HWND hwnd) : ControlField(dlg, name, hwnd)
{
}

ButtonField::~ButtonField()
{
}

FieldType ButtonField::getType() const
{
	return CONTROL_BUTTON;
}

FieldState * ButtonField::createState(DialogState *dialogState)
{
	return new ButtonFieldState(dialogState, this);
}