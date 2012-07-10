#include "globals.h"
#include "SkinOption_v8_wrapper.h"
#include <v8.h>
#include "SkinOption.h"
#include <utf8_helpers.h>

using namespace v8;


#ifdef UNICODE
# define V8_TCHAR uint16_t
#else
# define V8_TCHAR char
#endif


static Handle<Value> Get_SkinOption_description(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	SkinOption *tmp = (SkinOption *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( String::New((const V8_TCHAR *) tmp->getDescription()) );
}

static void Set_SkinOption_description(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	SkinOption *tmp = (SkinOption *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsString())
	{
		String::Utf8Value utf8_value(value);
		tmp->setDescription(Utf8ToTchar(*utf8_value));
	}
}


static Handle<Value> Get_SkinOption_min(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	SkinOption *tmp = (SkinOption *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getMin()) );
}

static void Set_SkinOption_min(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	SkinOption *tmp = (SkinOption *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsNumber())
		tmp->setMin(value->Int32Value());
}


static Handle<Value> Get_SkinOption_max(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	SkinOption *tmp = (SkinOption *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getMax()) );
}

static void Set_SkinOption_max(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	SkinOption *tmp = (SkinOption *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsNumber())
		tmp->setMax(value->Int32Value());
}


static Handle<Value> Get_SkinOption_type(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	SkinOption *tmp = (SkinOption *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	switch(tmp->getType())
	{
		case CHECKBOX: return scope.Close( String::New((const V8_TCHAR *) _T("CHECKBOX")) );
		case NUMBER: return scope.Close( String::New((const V8_TCHAR *) _T("NUMBER")) );
		case TEXT: return scope.Close( String::New((const V8_TCHAR *) _T("TEXT")) );
	}
	return scope.Close( Undefined() );
}

static void Set_SkinOption_type(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	SkinOption *tmp = (SkinOption *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsString())
	{
		String::Utf8Value utf8_value(value);
		Utf8ToTchar tval(*utf8_value);
		if ( lstrcmpi(_T("CHECKBOX"), tval) == 0 )
			tmp->setType(CHECKBOX);
		else if ( lstrcmpi(_T("NUMBER"), tval) == 0 )
			tmp->setType(NUMBER);
		else if ( lstrcmpi(_T("TEXT"), tval) == 0 )
			tmp->setType(TEXT);
	}
}


void AddSkinOptionAcessors(Handle<ObjectTemplate> &templ)
{
	HandleScope scope;
	
	templ->SetAccessor(String::New("description"), Get_SkinOption_description, Set_SkinOption_description);
	templ->SetAccessor(String::New("min"), Get_SkinOption_min, Set_SkinOption_min);
	templ->SetAccessor(String::New("max"), Get_SkinOption_max, Set_SkinOption_max);
	templ->SetAccessor(String::New("type"), Get_SkinOption_type, Set_SkinOption_type);
}
