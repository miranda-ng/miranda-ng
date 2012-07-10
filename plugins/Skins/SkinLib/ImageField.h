#ifndef __IMAGE_FIELD_H__
# define __IMAGE_FIELD_H__

#include "Field.h"

class ImageField : public Field
{
public:
	ImageField(Dialog *dlg, const char *name);
	virtual ~ImageField();

	virtual FieldType getType() const;

	virtual HBITMAP getImage() const;
	virtual void setImage(HBITMAP hBmp);

	virtual FieldState * createState(DialogState *dialogState);

private:
	HBITMAP hBmp;

};



#endif // __IMAGE_FIELD_H__