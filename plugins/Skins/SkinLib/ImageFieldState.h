#ifndef __IMAGE_FIELD_STATE_H__
# define __IMAGE_FIELD_STATE_H__

#include "ImageField.h"
#include "FieldState.h"


class ImageFieldState : public FieldState
{
public:
	virtual ~ImageFieldState();

	virtual ImageField * getField() const;

	virtual Size getPreferedSize() const;

	virtual HBITMAP getImage() const;

	virtual bool isEmpty() const;

private:
	ImageFieldState(DialogState *dialog, ImageField *field);

	friend class ImageField;
};


#endif // __IMAGE_FIELD_STATE_H__