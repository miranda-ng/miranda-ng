#include "globals.h"
#include "IconField.h"
#include "IconFieldState.h"


IconField::IconField(Dialog *dlg, const char *name) 
		: Field(dlg, name), hIcon(NULL)
{

}

IconField::~IconField()
{
}

FieldType IconField::getType() const
{
	return SIMPLE_ICON;
}

HICON IconField::getIcon() const
{
	return hIcon;
}

void IconField::setIcon(HICON hIcon)
{
	if (this->hIcon == hIcon)
		return;

	this->hIcon = hIcon;
	fireOnChange();
}

FieldState * IconField::createState(DialogState *dialogState)
{
	return new IconFieldState(dialogState, this);
}