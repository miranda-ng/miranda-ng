#ifndef __SKINNED_DIALOG_H__
# define __SKINNED_DIALOG_H__

#include <windows.h>
#include <tchar.h>
#include "tstring.h"

#include "Dialog.h"
#include "ButtonField.h"
#include "EditField.h"
#include "IconField.h"
#include "ImageField.h"
#include "LabelField.h"
#include "TextField.h"

#include "DialogState.h"
#include "BorderState.h"
#include "ButtonFieldState.h"
#include "EditFieldState.h"
#include "FontState.h"
#include "IconFieldState.h"
#include "ImageFieldState.h"
#include "LabelFieldState.h"
#include "TextFieldState.h"

#include "SkinOptions.h"

class V8Script;


typedef void (*MessageCallback)(void *param, const TCHAR *err);



class SkinnedDialog : public Dialog
{
public:
	SkinnedDialog(const char *name);
	virtual ~SkinnedDialog();

	virtual const TCHAR * getFilename() const;
	virtual void setFilename(const TCHAR *filename);

	virtual bool addField(Field *field);

	virtual void setSize(const Size &size);

	/// Return the cached state. Do not free the result. 
	/// Each call to this method can potentially create the state, so don't cache it.
	virtual DialogState * getState();

	virtual void setErrorCallback(MessageCallback cb, void *param = NULL);
	virtual void setTraceCallback(MessageCallback cb, void *param = NULL);

	virtual SkinOptions * getOpts();
	virtual DialogState * getDefaultState();

protected:
	virtual bool fileChanged();
	virtual int compile(); /// @return 0 error, 1 no change, 2 compiled

	virtual void trace(TCHAR *msg, ...);

	virtual void onFieldChange(const Field *field);

private:
	std::tstring filename;
	__time64_t fileChangedTime;
	V8Script *script;
	DialogState *state;
	SkinOptions *opts;
	DialogState *defaultState;

	MessageCallback errorCallback;
	void *errorCallbackParam;
	MessageCallback traceCallback;
	void *traceCallbackParam;

	void releaseCompiledScript();
	void releaseState();
	void readFile(std::tstring &ret);

	static void staticOnFieldChange(void *param, const Field *field);
};




#endif // __SKINNED_DIALOG_H__
