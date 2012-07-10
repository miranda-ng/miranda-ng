#include "globals.h"
#include "IconFieldState.h"

#define ICON_SIZE 16


IconFieldState::IconFieldState(DialogState *dialog, IconField *field) 
		: FieldState(dialog, field)
{
}

IconFieldState::~IconFieldState()
{
}

IconField * IconFieldState::getField() const
{
	return (IconField *) FieldState::getField();
}

Size IconFieldState::getPreferedSize() const
{
	if (getIcon() == NULL)
		return Size(0, 0);

	return Size(ICON_SIZE, ICON_SIZE);
}

HICON IconFieldState::getIcon() const
{
	return getField()->getIcon();
}

bool IconFieldState::isEmpty() const
{
	return getIcon() == NULL;
}