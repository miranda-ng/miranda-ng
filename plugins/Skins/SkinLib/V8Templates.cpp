#include "globals.h"
#include <v8.h>
#include "V8Templates.h"
#include "DialogState_v8_wrapper.h"
#include "FieldState_v8_wrapper.h"
#include "ControlFieldState_v8_wrapper.h"
#include "TextFieldState_v8_wrapper.h"
#include "FontState_v8_wrapper.h"
#include "BorderState_v8_wrapper.h"
#include "SkinOption_v8_wrapper.h"

using namespace v8;


V8Templates::V8Templates()
{
}

V8Templates::~V8Templates()
{
	dialogStateTemplate.Dispose();
	fieldStateTemplate.Dispose();
	controlFieldStateTemplate.Dispose();
	textFieldStateTemplate.Dispose();
	fontStateTemplate.Dispose();
	borderStateTemplate.Dispose();
	skinOptionTemplate.Dispose();
}


Handle<ObjectTemplate> V8Templates::getGlobalTemplate()
{
	HandleScope scope;
	
	if (!globalTemplate.IsEmpty())
		return globalTemplate; 
	
	Handle<ObjectTemplate> templ = ObjectTemplate::New();
	templ->Set(String::New("HORIZONTAL_ALIGN_LEFT"), String::New("HORIZONTAL_ALIGN_LEFT"));
	templ->Set(String::New("LEFT"), String::New("HORIZONTAL_ALIGN_LEFT"));
	templ->Set(String::New("HORIZONTAL_ALIGN_CENTER"), String::New("HORIZONTAL_ALIGN_CENTER"));
	templ->Set(String::New("CENTER"), String::New("HORIZONTAL_ALIGN_CENTER"));
	templ->Set(String::New("HORIZONTAL_ALIGN_RIGHT"), String::New("HORIZONTAL_ALIGN_RIGHT"));
	templ->Set(String::New("RIGHT"), String::New("HORIZONTAL_ALIGN_RIGHT"));
	templ->Set(String::New("VERTICAL_ALIGN_TOP"), String::New("VERTICAL_ALIGN_TOP"));
	templ->Set(String::New("TOP"), String::New("VERTICAL_ALIGN_TOP"));
	templ->Set(String::New("VERTICAL_ALIGN_CENTER"), String::New("VERTICAL_ALIGN_CENTER"));
	templ->Set(String::New("CENTER"), String::New("VERTICAL_ALIGN_CENTER"));
	templ->Set(String::New("VERTICAL_ALIGN_BOTTOM"), String::New("VERTICAL_ALIGN_BOTTOM"));
	templ->Set(String::New("BOTTOM"), String::New("VERTICAL_ALIGN_BOTTOM"));
	templ->Set(String::New("CHECKBOX"), String::New("CHECKBOX"));
	templ->Set(String::New("NUMBER"), String::New("NUMBER"));
	templ->Set(String::New("TEXT"), String::New("TEXT"));
	addGlobalTemplateFields(templ);
	
	globalTemplate = Persistent<ObjectTemplate>::New(templ);
	
	return globalTemplate;
}


void V8Templates::addGlobalTemplateFields(Handle<ObjectTemplate> &templ)
{
}


int V8Templates::numOfDialogStateInternalFields()
{
	return 1;
}

Handle<ObjectTemplate> V8Templates::getDialogStateTemplate()
{
	HandleScope scope;
	
	if (!dialogStateTemplate.IsEmpty())
		return dialogStateTemplate; 
	
	Handle<ObjectTemplate> templ = ObjectTemplate::New();
	templ->SetInternalFieldCount(numOfDialogStateInternalFields());
	AddDialogStateAcessors(templ);
	addDialogStateTemplateFields(templ);
	
	dialogStateTemplate = Persistent<ObjectTemplate>::New(templ);
	
	return dialogStateTemplate;
}

void V8Templates::addDialogStateTemplateFields(Handle<ObjectTemplate> &templ)
{
}

Handle<Object> V8Templates::newDialogState()
{
	HandleScope scope;
	
	Handle<Object> obj = getDialogStateTemplate()->NewInstance();
	obj->Set(String::New("bordersRaw"), newBorderState(), ReadOnly);
	
	return scope.Close(obj);
}

void V8Templates::fillDialogState(Handle<Object> v8Obj, DialogState *obj)
{
	HandleScope scope;
	
	_ASSERT(!v8Obj.IsEmpty());

	v8Obj->SetInternalField(0, External::New(obj));
	fillBorderState(Handle<Object>::Cast(v8Obj->Get(String::New("bordersRaw"))), obj->getBorders());
}

	
int V8Templates::numOfFieldStateInternalFields()
{
	return 1;
}

Handle<ObjectTemplate> V8Templates::getFieldStateTemplate()
{
	HandleScope scope;
	
	if (!fieldStateTemplate.IsEmpty())
		return fieldStateTemplate; 
	
	Handle<ObjectTemplate> templ = ObjectTemplate::New();
	templ->SetInternalFieldCount(numOfFieldStateInternalFields());
	AddFieldStateAcessors(templ);
	addFieldStateTemplateFields(templ);
	
	fieldStateTemplate = Persistent<ObjectTemplate>::New(templ);
	
	return fieldStateTemplate;
}

void V8Templates::addFieldStateTemplateFields(Handle<ObjectTemplate> &templ)
{
}

Handle<Object> V8Templates::newFieldState()
{
	HandleScope scope;
	
	Handle<Object> obj = getFieldStateTemplate()->NewInstance();
	obj->Set(String::New("bordersRaw"), newBorderState(), ReadOnly);
	
	return scope.Close(obj);
}

void V8Templates::fillFieldState(Handle<Object> v8Obj, FieldState *obj)
{
	HandleScope scope;
	
	_ASSERT(!v8Obj.IsEmpty());

	v8Obj->SetInternalField(0, External::New(obj));
	fillBorderState(Handle<Object>::Cast(v8Obj->Get(String::New("bordersRaw"))), obj->getBorders());
}

	
int V8Templates::numOfControlFieldStateInternalFields()
{
	return 1;
}

Handle<ObjectTemplate> V8Templates::getControlFieldStateTemplate()
{
	HandleScope scope;
	
	if (!controlFieldStateTemplate.IsEmpty())
		return controlFieldStateTemplate; 
	
	Handle<ObjectTemplate> templ = ObjectTemplate::New();
	templ->SetInternalFieldCount(numOfControlFieldStateInternalFields());
	AddFieldStateAcessors(templ);
	AddControlFieldStateAcessors(templ);
	addControlFieldStateTemplateFields(templ);
	
	controlFieldStateTemplate = Persistent<ObjectTemplate>::New(templ);
	
	return controlFieldStateTemplate;
}

void V8Templates::addControlFieldStateTemplateFields(Handle<ObjectTemplate> &templ)
{
}

Handle<Object> V8Templates::newControlFieldState()
{
	HandleScope scope;
	
	Handle<Object> obj = getControlFieldStateTemplate()->NewInstance();
	obj->Set(String::New("bordersRaw"), newBorderState(), ReadOnly);
	obj->Set(String::New("font"), newFontState(), ReadOnly);
	
	return scope.Close(obj);
}

void V8Templates::fillControlFieldState(Handle<Object> v8Obj, ControlFieldState *obj)
{
	HandleScope scope;
	
	_ASSERT(!v8Obj.IsEmpty());

	v8Obj->SetInternalField(0, External::New(obj));
	fillBorderState(Handle<Object>::Cast(v8Obj->Get(String::New("bordersRaw"))), obj->getBorders());
	fillFontState(Handle<Object>::Cast(v8Obj->Get(String::New("font"))), obj->getFont());
}

	
Handle<ObjectTemplate> V8Templates::getLabelFieldStateTemplate()
{
	return getControlFieldStateTemplate();
}

Handle<Object> V8Templates::newLabelFieldState()
{
	return newControlFieldState();
}

void V8Templates::fillLabelFieldState(Handle<Object> v8Obj, LabelFieldState *obj)
{
	fillControlFieldState(v8Obj, obj);
}

	
Handle<ObjectTemplate> V8Templates::getButtonFieldStateTemplate()
{
	return getControlFieldStateTemplate();
}

Handle<Object> V8Templates::newButtonFieldState()
{
	return newControlFieldState();
}

void V8Templates::fillButtonFieldState(Handle<Object> v8Obj, ButtonFieldState *obj)
{
	fillControlFieldState(v8Obj, obj);
}

	
Handle<ObjectTemplate> V8Templates::getEditFieldStateTemplate()
{
	return getControlFieldStateTemplate();
}

Handle<Object> V8Templates::newEditFieldState()
{
	return newControlFieldState();
}

void V8Templates::fillEditFieldState(Handle<Object> v8Obj, EditFieldState *obj)
{
	fillControlFieldState(v8Obj, obj);
}

	
Handle<ObjectTemplate> V8Templates::getIconFieldStateTemplate()
{
	return getFieldStateTemplate();
}

Handle<Object> V8Templates::newIconFieldState()
{
	return newFieldState();
}

void V8Templates::fillIconFieldState(Handle<Object> v8Obj, IconFieldState *obj)
{
	fillFieldState(v8Obj, obj);
}

	
Handle<ObjectTemplate> V8Templates::getImageFieldStateTemplate()
{
	return getFieldStateTemplate();
}

Handle<Object> V8Templates::newImageFieldState()
{
	return newFieldState();
}

void V8Templates::fillImageFieldState(Handle<Object> v8Obj, ImageFieldState *obj)
{
	fillFieldState(v8Obj, obj);
}

	
int V8Templates::numOfTextFieldStateInternalFields()
{
	return 1;
}

Handle<ObjectTemplate> V8Templates::getTextFieldStateTemplate()
{
	HandleScope scope;
	
	if (!textFieldStateTemplate.IsEmpty())
		return textFieldStateTemplate; 
	
	Handle<ObjectTemplate> templ = ObjectTemplate::New();
	templ->SetInternalFieldCount(numOfTextFieldStateInternalFields());
	AddFieldStateAcessors(templ);
	AddTextFieldStateAcessors(templ);
	addTextFieldStateTemplateFields(templ);
	
	textFieldStateTemplate = Persistent<ObjectTemplate>::New(templ);
	
	return textFieldStateTemplate;
}

void V8Templates::addTextFieldStateTemplateFields(Handle<ObjectTemplate> &templ)
{
}

Handle<Object> V8Templates::newTextFieldState()
{
	HandleScope scope;
	
	Handle<Object> obj = getTextFieldStateTemplate()->NewInstance();
	obj->Set(String::New("bordersRaw"), newBorderState(), ReadOnly);
	obj->Set(String::New("font"), newFontState(), ReadOnly);
	
	return scope.Close(obj);
}

void V8Templates::fillTextFieldState(Handle<Object> v8Obj, TextFieldState *obj)
{
	HandleScope scope;
	
	_ASSERT(!v8Obj.IsEmpty());

	v8Obj->SetInternalField(0, External::New(obj));
	fillBorderState(Handle<Object>::Cast(v8Obj->Get(String::New("bordersRaw"))), obj->getBorders());
	fillFontState(Handle<Object>::Cast(v8Obj->Get(String::New("font"))), obj->getFont());
}

	
int V8Templates::numOfFontStateInternalFields()
{
	return 1;
}

Handle<ObjectTemplate> V8Templates::getFontStateTemplate()
{
	HandleScope scope;
	
	if (!fontStateTemplate.IsEmpty())
		return fontStateTemplate; 
	
	Handle<ObjectTemplate> templ = ObjectTemplate::New();
	templ->SetInternalFieldCount(numOfFontStateInternalFields());
	AddFontStateAcessors(templ);
	addFontStateTemplateFields(templ);
	
	fontStateTemplate = Persistent<ObjectTemplate>::New(templ);
	
	return fontStateTemplate;
}

void V8Templates::addFontStateTemplateFields(Handle<ObjectTemplate> &templ)
{
}

Handle<Object> V8Templates::newFontState()
{
	HandleScope scope;
	
	Handle<Object> obj = getFontStateTemplate()->NewInstance();
	
	return scope.Close(obj);
}

void V8Templates::fillFontState(Handle<Object> v8Obj, FontState *obj)
{
	HandleScope scope;
	
	_ASSERT(!v8Obj.IsEmpty());

	v8Obj->SetInternalField(0, External::New(obj));
}

	
int V8Templates::numOfBorderStateInternalFields()
{
	return 1;
}

Handle<ObjectTemplate> V8Templates::getBorderStateTemplate()
{
	HandleScope scope;
	
	if (!borderStateTemplate.IsEmpty())
		return borderStateTemplate; 
	
	Handle<ObjectTemplate> templ = ObjectTemplate::New();
	templ->SetInternalFieldCount(numOfBorderStateInternalFields());
	AddBorderStateAcessors(templ);
	addBorderStateTemplateFields(templ);
	
	borderStateTemplate = Persistent<ObjectTemplate>::New(templ);
	
	return borderStateTemplate;
}

void V8Templates::addBorderStateTemplateFields(Handle<ObjectTemplate> &templ)
{
}

Handle<Object> V8Templates::newBorderState()
{
	HandleScope scope;
	
	Handle<Object> obj = getBorderStateTemplate()->NewInstance();
	
	return scope.Close(obj);
}

void V8Templates::fillBorderState(Handle<Object> v8Obj, BorderState *obj)
{
	HandleScope scope;
	
	_ASSERT(!v8Obj.IsEmpty());

	v8Obj->SetInternalField(0, External::New(obj));
}

	
int V8Templates::numOfSkinOptionInternalFields()
{
	return 1;
}

Handle<ObjectTemplate> V8Templates::getSkinOptionTemplate()
{
	HandleScope scope;
	
	if (!skinOptionTemplate.IsEmpty())
		return skinOptionTemplate; 
	
	Handle<ObjectTemplate> templ = ObjectTemplate::New();
	templ->SetInternalFieldCount(numOfSkinOptionInternalFields());
	AddSkinOptionAcessors(templ);
	addSkinOptionTemplateFields(templ);
	
	skinOptionTemplate = Persistent<ObjectTemplate>::New(templ);
	
	return skinOptionTemplate;
}

void V8Templates::addSkinOptionTemplateFields(Handle<ObjectTemplate> &templ)
{
}

Handle<Object> V8Templates::newSkinOption()
{
	HandleScope scope;
	
	Handle<Object> obj = getSkinOptionTemplate()->NewInstance();
	
	return scope.Close(obj);
}

void V8Templates::fillSkinOption(Handle<Object> v8Obj, SkinOption *obj)
{
	HandleScope scope;
	
	_ASSERT(!v8Obj.IsEmpty());

	v8Obj->SetInternalField(0, External::New(obj));
}

	
