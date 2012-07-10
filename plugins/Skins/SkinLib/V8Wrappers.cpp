#include "globals.h"
#include "V8Wrappers.h"

#include <utf8_helpers.h>


using namespace v8;


#ifdef UNICODE
# define V8_TCHAR uint16_t
#else
# define V8_TCHAR char
#endif



static Handle<Value> IsEmptyCallback(const Arguments& args)
{
	HandleScope scope;
	
	if (args.Length() < 1) 
		return scope.Close( Undefined() );

	for(int i = 0; i < args.Length(); i++)
	{
		Local<Value> arg = args[0];

		if (arg.IsEmpty() || arg->IsNull() || arg->IsUndefined())
		{
			return scope.Close( Boolean::New(true) );
		}
		else if (arg->IsObject())
		{
			Local<Object> self = Local<Object>::Cast(arg);
			if (self->InternalFieldCount() < 1)
				continue;

			Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
			FieldState *field = (FieldState *) wrap->Value();
			if (field == NULL)
				continue;

			if (field->isEmpty())
				return scope.Close( Boolean::New(true) );
		}
		else if (arg->IsString())
		{
			Local<String> str = Local<String>::Cast(arg);
			if (str->Length() <= 0)
				return scope.Close( Boolean::New(true) );
		}
	}

	return scope.Close( Boolean::New(false) );
}

static Handle<Value> RGBCallback(const Arguments& args)
{
	HandleScope scope;
	
	if (args.Length() != 3) 
		return scope.Close( Undefined() );

	COLORREF color = RGB(args[0]->Int32Value(), args[1]->Int32Value(), args[2]->Int32Value());
	return scope.Close( Int32::New(color) );
}

static Handle<Value> AlertCallback(const Arguments& args)
{
	HandleScope scope;

	if (args.Length() < 1) 
		return scope.Close( Boolean::New(false) );

	Local<Value> arg = args[0];
	String::Utf8Value utf8_value(arg->ToDetailString());

	char *tmp = *utf8_value;
// TODO 	MessageBox(NULL, Utf8ToTchar(*utf8_value), _T("Skin alert"), MB_OK);

	return scope.Close( Boolean::New(true) );
}

void V8Wrappers::addGlobalTemplateFields(Handle<ObjectTemplate> &templ)
{
	HandleScope scope;

	templ->Set(String::New("IsEmpty"), FunctionTemplate::New(&IsEmptyCallback));
	templ->Set(String::New("RGB"), FunctionTemplate::New(&RGBCallback));
	templ->Set(String::New("alert"), FunctionTemplate::New(&AlertCallback));
}


Handle<Object> V8Wrappers::newState(FieldType type)
{
	switch(type)
	{
		case SIMPLE_TEXT:
			return newTextFieldState();
		case SIMPLE_IMAGE:
			return newImageFieldState();
		case SIMPLE_ICON:
			return newIconFieldState();
		case CONTROL_LABEL:
			return newLabelFieldState();
		case CONTROL_BUTTON:
			return newButtonFieldState();
		case CONTROL_EDIT:
			return newEditFieldState();
	}
	throw "Unknown type";
}

void V8Wrappers::fillState(Handle<Object> obj, FieldState *state)
{
	switch(state->getField()->getType())
	{
		case SIMPLE_TEXT:
			fillTextFieldState(obj, (TextFieldState *) state);
			break;
		case SIMPLE_IMAGE:
			fillImageFieldState(obj, (ImageFieldState *) state);
			break;
		case SIMPLE_ICON:
			fillIconFieldState(obj, (IconFieldState *) state);
			break;
		case CONTROL_LABEL:
			fillLabelFieldState(obj, (LabelFieldState *) state);
			break;
		case CONTROL_BUTTON:
			fillButtonFieldState(obj, (ButtonFieldState *) state);
			break;
		case CONTROL_EDIT:
			fillEditFieldState(obj, (EditFieldState *) state);
			break;
		default:
			throw "Unknown type";
	}
}

static Handle<Value> Get_SkinOption_value(SkinOption *opt) 
{
	HandleScope scope;
	
	if (opt == NULL)
		return scope.Close( Undefined() );

	switch (opt->getType())
	{
		case CHECKBOX:	return scope.Close( Boolean::New(opt->getValueCheckbox()) );
		case NUMBER:	return scope.Close( Int32::New(opt->getValueNumber()) );
		case TEXT:		return scope.Close( String::New((const V8_TCHAR *) opt->getValueText()) );
	}

	return scope.Close( Undefined() );
}

static Handle<Value> Get_Options_Fields(Local<String> aName, const AccessorInfo &info)
{
	HandleScope scope;

	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	SkinOptions *opts = (SkinOptions *) wrap->Value();
	if (opts == NULL)
		return scope.Close( Undefined() );

	String::AsciiValue name(aName);
	if (name.length() <= 0)
		return scope.Close( Undefined() );

	bool configure = self->GetInternalField(1)->BooleanValue();
	if (configure)
	{
		SkinOption * opt = opts->getOption(*name);

		if (opt == NULL)
		{
			opt = new SkinOption(*name);
			opts->addOption(opt);
		}

		wrap = Local<External>::Cast(info.Data());
		if (wrap.IsEmpty())
			return scope.Close( Undefined() );

		V8Wrappers *wrappers = (V8Wrappers *) wrap->Value();
		if (wrappers == NULL)
			return scope.Close( Undefined() );

		Handle<Object> ret = wrappers->newSkinOption();
		wrappers->fillSkinOption(ret, opt);
		return scope.Close( ret );
	}
	else
	{
		SkinOption * opt = opts->getOption(*name);
		return scope.Close( Get_SkinOption_value(opt) );
	}
}

Handle<ObjectTemplate> V8Wrappers::getOptionsTemplate()
{
	HandleScope scope;
	
	if (!optionsTemplate.IsEmpty())
		return optionsTemplate; 
	
	Handle<ObjectTemplate> templ = ObjectTemplate::New();
	templ->SetInternalFieldCount(2);
	templ->SetNamedPropertyHandler(&Get_Options_Fields, 0, 0, 0, 0, External::New(this));
	
	optionsTemplate = Persistent<ObjectTemplate>::New(templ);
	
	return optionsTemplate;
}

Handle<Object> V8Wrappers::newOptions()
{
	HandleScope scope;
	
	Handle<Object> obj = getOptionsTemplate()->NewInstance();
	
	return scope.Close(obj);
}

void V8Wrappers::fillOptions(Handle<Object> v8Obj, SkinOptions *obj, bool configure)
{
	HandleScope scope;
	
	_ASSERT(!v8Obj.IsEmpty());

	v8Obj->SetInternalField(0, External::New(obj));
	v8Obj->SetInternalField(1, Boolean::New(configure));
}


static Handle<Value> Get_SkinOption_value(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	return scope.Close( Get_SkinOption_value((SkinOption *) wrap->Value()) );
}

static void Set_SkinOption_value(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	SkinOption *opt = (SkinOption *) wrap->Value();
	if (opt == NULL)
		return;

	switch (opt->getType())
	{
		case CHECKBOX:	
			if (!value.IsEmpty() && value->IsBoolean())
				opt->setValueCheckbox(value->BooleanValue());
			break;
		case NUMBER:
			if (!value.IsEmpty() && value->IsNumber())
				opt->setValueNumber(value->Int32Value());
			break;
		case TEXT:		
			if (!value.IsEmpty() && value->IsString())
				opt->setValueText(Utf8ToTchar(*String::Utf8Value(value)));
			break;
	}
}

void V8Wrappers::addSkinOptionTemplateFields(Handle<ObjectTemplate> &templ)
{
	HandleScope scope;
	
	templ->SetAccessor(String::New("value"), Get_SkinOption_value, Set_SkinOption_value);
}


static Handle<Value> Get_DialogInfo_Fields(Local<String> aName, const AccessorInfo &info)
{
	HandleScope scope;

	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	DialogInfo *dialogInfo = (DialogInfo *) wrap->Value();
	if (dialogInfo == NULL)
		return scope.Close( Undefined() );

	String::AsciiValue name(aName);
	if (name.length() <= 0)
		return scope.Close( Undefined() );

	wrap = Local<External>::Cast(info.Data());
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	V8Wrappers *wrappers = (V8Wrappers *) wrap->Value();
	if (wrappers == NULL)
		return scope.Close( Undefined() );

	Local<String> aPrefix = Local<String>::Cast(self->GetInternalField(1));
	if (aPrefix.IsEmpty())
		return scope.Close( Undefined() );

	String::AsciiValue prefix(aPrefix);
	
	std::string var;
	var += *prefix;
	var += *name;

	switch(dialogInfo->getType(var.c_str()))
	{
		case TYPE_VARIABLE:
		{
			var += '.';

			Handle<Object> ret = wrappers->newDialogInfo();
			wrappers->fillDialogInfo(ret, dialogInfo, var.c_str());
			return scope.Close( ret );
		}
		case TYPE_INT:
			return scope.Close( Int32::New(dialogInfo->getAsInt(var.c_str())) );
		case TYPE_DOUBLE:
			return scope.Close( Number::New(dialogInfo->getAsDouble(var.c_str())) );
		case TYPE_BOOL:
			return scope.Close( Boolean::New(dialogInfo->getAsBool(var.c_str())) );
		case TYPE_STRING:
			return scope.Close( String::New((const V8_TCHAR *) dialogInfo->getAsString(var.c_str())) );
		case UNKNOWN:
		default:
			return scope.Close( Undefined() );
	}
}

Handle<ObjectTemplate> V8Wrappers::getDialogInfoTemplate()
{
	HandleScope scope;
	
	if (!dialogInfoTemplate.IsEmpty())
		return dialogInfoTemplate; 
	
	Handle<ObjectTemplate> templ = ObjectTemplate::New();
	templ->SetInternalFieldCount(2);
	templ->SetNamedPropertyHandler(&Get_DialogInfo_Fields, 0, 0, 0, 0, External::New(this));
	
	dialogInfoTemplate = Persistent<ObjectTemplate>::New(templ);
	
	return dialogInfoTemplate;
}

Handle<Object> V8Wrappers::newDialogInfo()
{
	HandleScope scope;
	
	Handle<Object> obj = getDialogInfoTemplate()->NewInstance();
	
	return scope.Close(obj);
}

void V8Wrappers::fillDialogInfo(Handle<Object> v8Obj, DialogInfo *obj, const char *prefix)
{
	HandleScope scope;
	
	_ASSERT(!v8Obj.IsEmpty());

	v8Obj->SetInternalField(0, External::New(obj));

	v8Obj->SetInternalField(1, String::New(prefix != NULL ? prefix : ""));
}
