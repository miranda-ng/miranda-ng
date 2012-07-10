#ifndef __LABEL_FIELD_STATE_H__
# define __LABEL_FIELD_STATE_H__

#include "ControlFieldState.h"
#include "LabelField.h"


class LabelFieldState : public ControlFieldState
{
public:
	virtual ~LabelFieldState();

	virtual Size getPreferedSize() const;

	virtual bool isEmpty() const;

private:
	LabelFieldState(DialogState *dialog, LabelField *field);

	friend class LabelField;
};



#endif // __LABEL_FIELD_STATE_H__
