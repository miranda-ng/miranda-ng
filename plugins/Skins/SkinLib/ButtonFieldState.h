#ifndef __BUTTON_FIELD_STATE_H__
# define __BUTTON_FIELD_STATE_H__

#include "ControlFieldState.h"


class ButtonFieldState : public ControlFieldState
{
public:
	virtual ~ButtonFieldState();

	virtual Size getPreferedSize() const;

	virtual bool isEmpty() const;

private:
	ButtonFieldState(DialogState *dialog, ControlField *field);

	friend class ButtonField;
};


#endif // __BUTTON_FIELD_STATE_H__
