#include "globals.h"
#include "DialogState_v8_wrapper.h"
#include <v8.h>
#include "DialogState.h"

using namespace v8;


#ifdef UNICODE
# define V8_TCHAR uint16_t
#else
# define V8_TCHAR char
#endif


static Handle<Value> Get_DialogState_width(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	DialogState *tmp = (DialogState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getWidth()) );
}

static void Set_DialogState_width(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	DialogState *tmp = (DialogState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsNumber())
		tmp->setWidth(value->Int32Value());
}


static Handle<Value> Get_DialogState_height(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	DialogState *tmp = (DialogState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getHeight()) );
}

static void Set_DialogState_height(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	DialogState *tmp = (DialogState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsNumber())
		tmp->setHeight(value->Int32Value());
}


static Handle<Value> Get_DialogState_x(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	DialogState *tmp = (DialogState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getX()) );
}


static Handle<Value> Get_DialogState_y(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	DialogState *tmp = (DialogState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getY()) );
}


static Handle<Value> Get_DialogState_left(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	DialogState *tmp = (DialogState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getLeft()) );
}


static Handle<Value> Get_DialogState_top(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	DialogState *tmp = (DialogState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getTop()) );
}


static Handle<Value> Get_DialogState_right(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	DialogState *tmp = (DialogState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getRight()) );
}


static Handle<Value> Get_DialogState_bottom(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	DialogState *tmp = (DialogState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getBottom()) );
}


static Handle<Value> Get_DialogState_borders(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	DialogState *tmp = (DialogState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );
	
	return scope.Close( self->Get(String::New("bordersRaw")) );
}

static void Set_DialogState_borders(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	DialogState *tmp = (DialogState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsInt32())
		tmp->getBorders()->setAll(value->Int32Value()); 
}


void AddDialogStateAcessors(Handle<ObjectTemplate> &templ)
{
	HandleScope scope;
	
	templ->SetAccessor(String::New("width"), Get_DialogState_width, Set_DialogState_width);
	templ->SetAccessor(String::New("height"), Get_DialogState_height, Set_DialogState_height);
	templ->SetAccessor(String::New("x"), Get_DialogState_x, NULL, Handle<Value>(), DEFAULT, ReadOnly);
	templ->SetAccessor(String::New("y"), Get_DialogState_y, NULL, Handle<Value>(), DEFAULT, ReadOnly);
	templ->SetAccessor(String::New("left"), Get_DialogState_left, NULL, Handle<Value>(), DEFAULT, ReadOnly);
	templ->SetAccessor(String::New("top"), Get_DialogState_top, NULL, Handle<Value>(), DEFAULT, ReadOnly);
	templ->SetAccessor(String::New("right"), Get_DialogState_right, NULL, Handle<Value>(), DEFAULT, ReadOnly);
	templ->SetAccessor(String::New("bottom"), Get_DialogState_bottom, NULL, Handle<Value>(), DEFAULT, ReadOnly);
	templ->SetAccessor(String::New("borders"), Get_DialogState_borders, Set_DialogState_borders);
}
