#include "globals.h"
#include "LabelField.h"
#include "LabelFieldState.h"


LabelField::LabelField(Dialog *dlg, const char *name, HWND hwnd) 
		: ControlField(dlg, name, hwnd)
{
}

LabelField::~LabelField()
{
}

FieldType LabelField::getType() const
{
	return CONTROL_LABEL;
}

FieldState * LabelField::createState(DialogState *dialogState)
{
	return new LabelFieldState(dialogState, this);
}