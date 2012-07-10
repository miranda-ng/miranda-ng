#ifndef __V8_SCRIPT_H__
# define __V8_SCRIPT_H__

#include <v8.h>
#include <utility>
#include "V8Wrappers.h"


typedef void (*ExceptionCallback)(void *param, const TCHAR *err);


class V8Script
{
public:
	static void initializeEngine();


	V8Script();
	~V8Script();

	bool compile(const TCHAR *source, Dialog *dlg);
	void dispose();

	bool isValid();

	std::pair<SkinOptions *,DialogState *> configure(Dialog *dlg);

	bool run(DialogState * state, SkinOptions *opts, DialogInfo *info);

	void setExceptionCallback(ExceptionCallback cb, void *param = NULL);

private:
	v8::Persistent<v8::Context> context;
	v8::Persistent<v8::Function> configureFunction;
	v8::Persistent<v8::Function> drawFunction;

	ExceptionCallback exceptionCallback;
	void *exceptionCallbackParam;

	void reportException(v8::TryCatch *try_catch);
	void fillWrappers(DialogState *state, SkinOptions *opts, DialogInfo *info, bool configure);
};



#endif // __V8_SCRIPT_H__
