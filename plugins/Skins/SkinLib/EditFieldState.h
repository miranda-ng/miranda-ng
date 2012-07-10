#ifndef __EDIT_FIELD_STATE_H__
# define __EDIT_FIELD_STATE_H__

#include "ControlFieldState.h"
#include "EditField.h"


class EditFieldState : public ControlFieldState
{
public:
	virtual ~EditFieldState();

	virtual Size getPreferedSize() const;

	virtual bool isEmpty() const;

private:
	EditFieldState(DialogState *dialog, EditField *field);

	friend class EditField;
};


#endif // __EDIT_FIELD_STATE_H__
