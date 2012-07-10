#include "globals.h"
#include "ImageField.h"
#include "ImageFieldState.h"


ImageField::ImageField(Dialog *dlg, const char *name) 
		: Field(dlg, name), hBmp(NULL)
{

}

ImageField::~ImageField()
{
}

FieldType ImageField::getType() const
{
	return SIMPLE_IMAGE;
}

HBITMAP ImageField::getImage() const
{
	return hBmp;
}

void ImageField::setImage(HBITMAP hBmp)
{
	if (this->hBmp == hBmp)
		return;

	this->hBmp = hBmp;
	fireOnChange();
}

FieldState * ImageField::createState(DialogState *dialogState)
{
	return new ImageFieldState(dialogState, this);
}