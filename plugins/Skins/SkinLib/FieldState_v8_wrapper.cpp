#include "globals.h"
#include "FieldState_v8_wrapper.h"
#include <v8.h>
#include "FieldState.h"
#include <utf8_helpers.h>

using namespace v8;


#ifdef UNICODE
# define V8_TCHAR uint16_t
#else
# define V8_TCHAR char
#endif


static Handle<Value> Get_FieldState_x(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getX()) );
}

static void Set_FieldState_x(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsNumber())
		tmp->setX(value->Int32Value());
}


static Handle<Value> Get_FieldState_y(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getY()) );
}

static void Set_FieldState_y(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsNumber())
		tmp->setY(value->Int32Value());
}


static Handle<Value> Get_FieldState_width(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getWidth()) );
}

static void Set_FieldState_width(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsNumber())
		tmp->setWidth(value->Int32Value());
}


static Handle<Value> Get_FieldState_height(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getHeight()) );
}

static void Set_FieldState_height(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsNumber())
		tmp->setHeight(value->Int32Value());
}


static Handle<Value> Get_FieldState_left(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getLeft()) );
}

static void Set_FieldState_left(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsNumber())
		tmp->setLeft(value->Int32Value());
}


static Handle<Value> Get_FieldState_top(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getTop()) );
}

static void Set_FieldState_top(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsNumber())
		tmp->setTop(value->Int32Value());
}


static Handle<Value> Get_FieldState_right(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getRight()) );
}

static void Set_FieldState_right(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsNumber())
		tmp->setRight(value->Int32Value());
}


static Handle<Value> Get_FieldState_bottom(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Int32::New(tmp->getBottom()) );
}

static void Set_FieldState_bottom(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsNumber())
		tmp->setBottom(value->Int32Value());
}


static Handle<Value> Get_FieldState_visible(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Boolean::New(tmp->isVisible()) );
}

static void Set_FieldState_visible(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsBoolean())
		tmp->setVisible(value->BooleanValue());
}


static Handle<Value> Get_FieldState_enabled(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( Boolean::New(tmp->isEnabled()) );
}


static Handle<Value> Get_FieldState_toolTip(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	return scope.Close( String::New((const V8_TCHAR *) tmp->getToolTip()) );
}

static void Set_FieldState_toolTip(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsString())
	{
		String::Utf8Value utf8_value(value);
		tmp->setToolTip(Utf8ToTchar(*utf8_value));
	}
}


static Handle<Value> Get_FieldState_hAlign(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	switch(tmp->getHAlign())
	{
		case HORIZONTAL_ALIGN_LEFT: return scope.Close( String::New((const V8_TCHAR *) _T("LEFT")) );
		case HORIZONTAL_ALIGN_CENTER: return scope.Close( String::New((const V8_TCHAR *) _T("CENTER")) );
		case HORIZONTAL_ALIGN_RIGHT: return scope.Close( String::New((const V8_TCHAR *) _T("RIGHT")) );
	}
	return scope.Close( Undefined() );
}

static void Set_FieldState_hAlign(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsString())
	{
		String::Utf8Value utf8_value(value);
		Utf8ToTchar tval(*utf8_value);
		if ( lstrcmpi(_T("HORIZONTAL_ALIGN_LEFT"), tval) == 0 )
			tmp->setHAlign(HORIZONTAL_ALIGN_LEFT);
		else if ( lstrcmpi(_T("LEFT"), tval) == 0 )
			tmp->setHAlign(HORIZONTAL_ALIGN_LEFT);
		else if ( lstrcmpi(_T("HORIZONTAL_ALIGN_CENTER"), tval) == 0 )
			tmp->setHAlign(HORIZONTAL_ALIGN_CENTER);
		else if ( lstrcmpi(_T("CENTER"), tval) == 0 )
			tmp->setHAlign(HORIZONTAL_ALIGN_CENTER);
		else if ( lstrcmpi(_T("HORIZONTAL_ALIGN_RIGHT"), tval) == 0 )
			tmp->setHAlign(HORIZONTAL_ALIGN_RIGHT);
		else if ( lstrcmpi(_T("RIGHT"), tval) == 0 )
			tmp->setHAlign(HORIZONTAL_ALIGN_RIGHT);
	}
}


static Handle<Value> Get_FieldState_vAlign(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );

	switch(tmp->getVAlign())
	{
		case VERTICAL_ALIGN_TOP: return scope.Close( String::New((const V8_TCHAR *) _T("TOP")) );
		case VERTICAL_ALIGN_CENTER: return scope.Close( String::New((const V8_TCHAR *) _T("CENTER")) );
		case VERTICAL_ALIGN_BOTTOM: return scope.Close( String::New((const V8_TCHAR *) _T("BOTTOM")) );
	}
	return scope.Close( Undefined() );
}

static void Set_FieldState_vAlign(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsString())
	{
		String::Utf8Value utf8_value(value);
		Utf8ToTchar tval(*utf8_value);
		if ( lstrcmpi(_T("VERTICAL_ALIGN_TOP"), tval) == 0 )
			tmp->setVAlign(VERTICAL_ALIGN_TOP);
		else if ( lstrcmpi(_T("TOP"), tval) == 0 )
			tmp->setVAlign(VERTICAL_ALIGN_TOP);
		else if ( lstrcmpi(_T("VERTICAL_ALIGN_CENTER"), tval) == 0 )
			tmp->setVAlign(VERTICAL_ALIGN_CENTER);
		else if ( lstrcmpi(_T("CENTER"), tval) == 0 )
			tmp->setVAlign(VERTICAL_ALIGN_CENTER);
		else if ( lstrcmpi(_T("VERTICAL_ALIGN_BOTTOM"), tval) == 0 )
			tmp->setVAlign(VERTICAL_ALIGN_BOTTOM);
		else if ( lstrcmpi(_T("BOTTOM"), tval) == 0 )
			tmp->setVAlign(VERTICAL_ALIGN_BOTTOM);
	}
}


static Handle<Value> Get_FieldState_borders(Local<String> property, const AccessorInfo &info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return scope.Close( Undefined() );

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return scope.Close( Undefined() );
	
	return scope.Close( self->Get(String::New("bordersRaw")) );
}

static void Set_FieldState_borders(Local<String> property, Local<Value> value, const AccessorInfo& info) 
{
	HandleScope scope;
	
	Local<Object> self = info.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	if (wrap.IsEmpty())
		return;

	FieldState *tmp = (FieldState *) wrap->Value();
	if (tmp == NULL)
		return;

	if (!value.IsEmpty() && value->IsInt32())
		tmp->getBorders()->setAll(value->Int32Value()); 
}


void AddFieldStateAcessors(Handle<ObjectTemplate> &templ)
{
	HandleScope scope;
	
	templ->SetAccessor(String::New("x"), Get_FieldState_x, Set_FieldState_x);
	templ->SetAccessor(String::New("y"), Get_FieldState_y, Set_FieldState_y);
	templ->SetAccessor(String::New("width"), Get_FieldState_width, Set_FieldState_width);
	templ->SetAccessor(String::New("height"), Get_FieldState_height, Set_FieldState_height);
	templ->SetAccessor(String::New("left"), Get_FieldState_left, Set_FieldState_left);
	templ->SetAccessor(String::New("top"), Get_FieldState_top, Set_FieldState_top);
	templ->SetAccessor(String::New("right"), Get_FieldState_right, Set_FieldState_right);
	templ->SetAccessor(String::New("bottom"), Get_FieldState_bottom, Set_FieldState_bottom);
	templ->SetAccessor(String::New("visible"), Get_FieldState_visible, Set_FieldState_visible);
	templ->SetAccessor(String::New("enabled"), Get_FieldState_enabled, NULL, Handle<Value>(), DEFAULT, ReadOnly);
	templ->SetAccessor(String::New("toolTip"), Get_FieldState_toolTip, Set_FieldState_toolTip);
	templ->SetAccessor(String::New("hAlign"), Get_FieldState_hAlign, Set_FieldState_hAlign);
	templ->SetAccessor(String::New("vAlign"), Get_FieldState_vAlign, Set_FieldState_vAlign);
	templ->SetAccessor(String::New("borders"), Get_FieldState_borders, Set_FieldState_borders);
}
