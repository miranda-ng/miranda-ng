#ifndef __V8_TEMPLATES_H__
# define __V8_TEMPLATES_H__

#include <v8.h>
#include "DialogState.h"
#include "FieldState.h"
#include "ControlFieldState.h"
#include "LabelFieldState.h"
#include "ButtonFieldState.h"
#include "EditFieldState.h"
#include "IconFieldState.h"
#include "ImageFieldState.h"
#include "TextFieldState.h"
#include "FontState.h"
#include "BorderState.h"
#include "SkinOption.h"


class V8Templates
{
public:
	V8Templates();
	virtual ~V8Templates();
	
	virtual v8::Handle<v8::ObjectTemplate> getGlobalTemplate();
	
	virtual v8::Handle<v8::ObjectTemplate> getDialogStateTemplate();
	virtual v8::Handle<v8::Object> newDialogState();
	virtual void fillDialogState(v8::Handle<v8::Object> v8Obj, DialogState *obj);
	
	virtual v8::Handle<v8::ObjectTemplate> getFieldStateTemplate();
	virtual v8::Handle<v8::Object> newFieldState();
	virtual void fillFieldState(v8::Handle<v8::Object> v8Obj, FieldState *obj);
	
	virtual v8::Handle<v8::ObjectTemplate> getControlFieldStateTemplate();
	virtual v8::Handle<v8::Object> newControlFieldState();
	virtual void fillControlFieldState(v8::Handle<v8::Object> v8Obj, ControlFieldState *obj);
	
	virtual v8::Handle<v8::ObjectTemplate> getLabelFieldStateTemplate();
	virtual v8::Handle<v8::Object> newLabelFieldState();
	virtual void fillLabelFieldState(v8::Handle<v8::Object> v8Obj, LabelFieldState *obj);
	
	virtual v8::Handle<v8::ObjectTemplate> getButtonFieldStateTemplate();
	virtual v8::Handle<v8::Object> newButtonFieldState();
	virtual void fillButtonFieldState(v8::Handle<v8::Object> v8Obj, ButtonFieldState *obj);
	
	virtual v8::Handle<v8::ObjectTemplate> getEditFieldStateTemplate();
	virtual v8::Handle<v8::Object> newEditFieldState();
	virtual void fillEditFieldState(v8::Handle<v8::Object> v8Obj, EditFieldState *obj);
	
	virtual v8::Handle<v8::ObjectTemplate> getIconFieldStateTemplate();
	virtual v8::Handle<v8::Object> newIconFieldState();
	virtual void fillIconFieldState(v8::Handle<v8::Object> v8Obj, IconFieldState *obj);
	
	virtual v8::Handle<v8::ObjectTemplate> getImageFieldStateTemplate();
	virtual v8::Handle<v8::Object> newImageFieldState();
	virtual void fillImageFieldState(v8::Handle<v8::Object> v8Obj, ImageFieldState *obj);
	
	virtual v8::Handle<v8::ObjectTemplate> getTextFieldStateTemplate();
	virtual v8::Handle<v8::Object> newTextFieldState();
	virtual void fillTextFieldState(v8::Handle<v8::Object> v8Obj, TextFieldState *obj);
	
	virtual v8::Handle<v8::ObjectTemplate> getFontStateTemplate();
	virtual v8::Handle<v8::Object> newFontState();
	virtual void fillFontState(v8::Handle<v8::Object> v8Obj, FontState *obj);
	
	virtual v8::Handle<v8::ObjectTemplate> getBorderStateTemplate();
	virtual v8::Handle<v8::Object> newBorderState();
	virtual void fillBorderState(v8::Handle<v8::Object> v8Obj, BorderState *obj);
	
	virtual v8::Handle<v8::ObjectTemplate> getSkinOptionTemplate();
	virtual v8::Handle<v8::Object> newSkinOption();
	virtual void fillSkinOption(v8::Handle<v8::Object> v8Obj, SkinOption *obj);
	

private:
	v8::Persistent<v8::ObjectTemplate> globalTemplate;
	v8::Persistent<v8::ObjectTemplate> dialogStateTemplate;
	v8::Persistent<v8::ObjectTemplate> fieldStateTemplate;
	v8::Persistent<v8::ObjectTemplate> controlFieldStateTemplate;
	v8::Persistent<v8::ObjectTemplate> textFieldStateTemplate;
	v8::Persistent<v8::ObjectTemplate> fontStateTemplate;
	v8::Persistent<v8::ObjectTemplate> borderStateTemplate;
	v8::Persistent<v8::ObjectTemplate> skinOptionTemplate;

protected:
	virtual void addGlobalTemplateFields(v8::Handle<v8::ObjectTemplate> &templ);

	virtual int numOfDialogStateInternalFields();
	virtual void addDialogStateTemplateFields(v8::Handle<v8::ObjectTemplate> &templ);
	
	virtual int numOfFieldStateInternalFields();
	virtual void addFieldStateTemplateFields(v8::Handle<v8::ObjectTemplate> &templ);
	
	virtual int numOfControlFieldStateInternalFields();
	virtual void addControlFieldStateTemplateFields(v8::Handle<v8::ObjectTemplate> &templ);
	
	virtual int numOfTextFieldStateInternalFields();
	virtual void addTextFieldStateTemplateFields(v8::Handle<v8::ObjectTemplate> &templ);
	
	virtual int numOfFontStateInternalFields();
	virtual void addFontStateTemplateFields(v8::Handle<v8::ObjectTemplate> &templ);
	
	virtual int numOfBorderStateInternalFields();
	virtual void addBorderStateTemplateFields(v8::Handle<v8::ObjectTemplate> &templ);
	
	virtual int numOfSkinOptionInternalFields();
	virtual void addSkinOptionTemplateFields(v8::Handle<v8::ObjectTemplate> &templ);
	
};



#endif // __V8_TEMPLATES_H__
