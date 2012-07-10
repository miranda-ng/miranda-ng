#include "globals.h"
#include "ButtonFieldState.h"

ButtonFieldState::ButtonFieldState(DialogState *dialog, ControlField *field) 
		: ControlFieldState(dialog, field)
{
}

ButtonFieldState::~ButtonFieldState()
{
}

Size ButtonFieldState::getPreferedSize() const
{
	Size ret = getTextPreferedSize(DT_SINGLELINE);

	int border = getField()->getBorderSize();
	ret.x += 2 * border;
	ret.y += 2 * border;

	ret.x += 12;
	ret.y += 10;

	return ret;
}

bool ButtonFieldState::isEmpty() const
{
	return false;
}