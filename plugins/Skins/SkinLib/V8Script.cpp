#include "globals.h"
#include "V8Script.h"
#include "V8Wrappers.h"

#include <utf8_helpers.h>

using namespace v8;

#ifdef UNICODE
# define V8_TCHAR uint16_t
#else
# define V8_TCHAR char
#endif


V8Wrappers *wrappers = NULL;


void V8Script::initializeEngine()
{
	if (wrappers != NULL)
		return;

	wrappers = new V8Wrappers();
}


V8Script::V8Script() : exceptionCallback(NULL), exceptionCallbackParam(NULL)
{
}

V8Script::~V8Script()
{
	dispose();
}

bool V8Script::compile(const TCHAR *source, Dialog *dlg)
{
	dispose();

	HandleScope handle_scope;

	context = Context::New(NULL, wrappers->getGlobalTemplate());

	Context::Scope context_scope(context);

	context->Global()->Set(String::New("window"), wrappers->newDialogState(), ReadOnly);
	context->Global()->Set(String::New("opts"), wrappers->newOptions(), ReadOnly);
	context->Global()->Set(String::New("info"), wrappers->newDialogInfo(), ReadOnly);
	for(unsigned int i = 0; i < dlg->getFieldCount(); i++)
	{
		Field *field = dlg->getField(i);
		context->Global()->Set(String::New(field->getName()), wrappers->newState(field->getType()), ReadOnly);
	}

	TryCatch try_catch;
	Local<Script> script = Script::Compile(String::New((const V8_TCHAR *) source), String::New(dlg->getName()));
	if (script.IsEmpty()) 
	{
		reportException(&try_catch);
		dispose();
		return false;
	}

	// Run once to get the functions
	Handle<Value> result = script->Run();
	if (script.IsEmpty()) 
	{
		reportException(&try_catch);
		dispose();
		return false;
	}

	Handle<Value> configureFunction = context->Global()->Get(String::New("configure"));
	if (!configureFunction.IsEmpty() && !configureFunction->IsFunction()) 
		configureFunction.Clear();

	Handle<Value> drawFunction = context->Global()->Get(String::New("draw"));
	if (drawFunction.IsEmpty() || !drawFunction->IsFunction()) 
	{
		dispose();
		return false;
	}

	this->configureFunction = Persistent<Function>::New(Handle<Function>::Cast(configureFunction));
	this->drawFunction = Persistent<Function>::New(Handle<Function>::Cast(drawFunction));

	return true;
}

void V8Script::dispose()
{
	context.Dispose();
	configureFunction.Dispose();
	drawFunction.Dispose();

	context.Clear();
	configureFunction.Clear();
	drawFunction.Clear();
}

bool V8Script::isValid()
{
	return !context.IsEmpty() && !drawFunction.IsEmpty();
}

static Handle<Object> get(Handle<Object> obj, const char *field)
{
	HandleScope scope;
	
	Local<Value> v = obj->Get(String::New(field));
	_ASSERT(!v.IsEmpty());
	_ASSERT(!v->IsUndefined());
	_ASSERT(v->IsObject());

	return scope.Close( Handle<Object>::Cast(v) );
}

void V8Script::fillWrappers(DialogState *state, SkinOptions *opts, DialogInfo *info, bool configure)
{
	Local<Object> global = context->Global();
	wrappers->fillOptions(get(global, "opts"), opts, configure);
	wrappers->fillDialogState(get(global, "window"), state);
	wrappers->fillDialogInfo(get(global, "info"), info);
	for(unsigned int i = 0; i < state->fields.size(); i++)
	{
		FieldState *field = state->fields[i];
		wrappers->fillState(get(global, field->getField()->getName()), field);
	}
}

std::pair<SkinOptions *,DialogState *> V8Script::configure(Dialog *dlg)
{
	if (!isValid())
		return std::pair<SkinOptions *,DialogState *>(NULL, NULL);

	SkinOptions *opts = new SkinOptions();
	DialogState *state = dlg->createState();

	if (!configureFunction.IsEmpty())
	{
		HandleScope handle_scope;

		Context::Scope context_scope(context);

		fillWrappers(state, opts, dlg->getInfo(), true);

		TryCatch try_catch;
		Handle<Value> result = configureFunction->Call(context->Global(), 0, NULL);
		if (result.IsEmpty()) 
		{
			reportException(&try_catch);
			delete opts;
			delete state;
			return std::pair<SkinOptions *,DialogState *>(NULL, NULL);;
		}
	}

	return std::pair<SkinOptions *,DialogState *>(opts, state);
}

bool V8Script::run(DialogState * state, SkinOptions *opts, DialogInfo *info)
{
	if (!isValid())
		return false;

	HandleScope handle_scope;

	Context::Scope context_scope(context);

	fillWrappers(state, opts, info, false);

	TryCatch try_catch;
	Handle<Value> result = drawFunction->Call(context->Global(), 0, NULL);
	if (result.IsEmpty()) 
	{
		reportException(&try_catch);
		return false;
	}

	return true;
}

void V8Script::setExceptionCallback(ExceptionCallback cb, void *param)
{
	exceptionCallback = cb;
	exceptionCallbackParam = param;
}

void V8Script::reportException(v8::TryCatch *try_catch)
{
	std::string err;
	char tmp[1024];

	HandleScope handle_scope;
	String::Utf8Value exception(try_catch->Exception());
	Handle<Message> message = try_catch->Message();
	if (message.IsEmpty()) 
	{
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		_snprintf(tmp, 1024, "%s\n", *exception);
		err += tmp;
	}
	else 
	{
		// Print (filename):(line number): (message).
		String::Utf8Value filename(message->GetScriptResourceName());
		int linenum = message->GetLineNumber();
		_snprintf(tmp, 1024, "%s:%i: %s\n", *filename, linenum, *exception);
		err += tmp;

		// Print line of source code.
		String::Utf8Value sourceline(message->GetSourceLine());
		_snprintf(tmp, 1024, "%s\n", *sourceline);
		err += tmp;

		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn();
		for (int i = 0; i < start; i++) {
			err += " ";
		}
		int end = message->GetEndColumn();
		for (int i = start; i < end; i++) {
			err += "^";
		}
		err += "\n";
	}

	Utf8ToTchar tcharErr(err.c_str());

	OutputDebugString(tcharErr);

	if (exceptionCallback != NULL)
		exceptionCallback(exceptionCallbackParam, tcharErr);
}
