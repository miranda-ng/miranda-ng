#include "globals.h"
#include "Field.h"
#include "FieldState.h"


Field::Field(Dialog *aDlg, const char *aName) 
		: dlg(aDlg), name(aName), onChangeCallback(NULL), onChangeCallbackParam(NULL), enabled(true)
{
}

Field::~Field()
{
}

Dialog * Field::getDialog() const
{
	return dlg;
}

const char * Field::getName() const
{
	return name.c_str();
}

bool Field::isEnabled() const
{
	return enabled;
}

void Field::setEnabled(bool enabled)
{
	this->enabled = enabled;
	fireOnChange();
}

const TCHAR * Field::getToolTip() const
{
	return tooltip.c_str();
}

void Field::setToolTip(const TCHAR *tooltip)
{
	this->tooltip = tooltip;
}

void Field::setOnChangeCallback(FieldCallback cb, void *param /*= NULL*/)
{
	onChangeCallback = cb;
	onChangeCallbackParam = param;
}

void Field::fireOnChange() const
{
	if (onChangeCallback != NULL)
		onChangeCallback(onChangeCallbackParam, this);
}
