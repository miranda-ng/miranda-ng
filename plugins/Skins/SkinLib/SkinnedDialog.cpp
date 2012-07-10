#include "globals.h"
#include "SkinnedDialog.h"

#include <sys/stat.h>
#include "V8Script.h"
#include <utf8_helpers.h>
#include "SkinOptions.h"


SkinnedDialog::SkinnedDialog(const char *name) 
		: Dialog(name), fileChangedTime(0), 
		  script(NULL), state(NULL), opts(NULL), defaultState(NULL),
		  errorCallback(NULL), errorCallbackParam(NULL),
		  traceCallback(NULL), traceCallbackParam(NULL)
{
	V8Script::initializeEngine();
}

SkinnedDialog::~SkinnedDialog()
{
	releaseState();
	releaseCompiledScript();
}

const TCHAR * SkinnedDialog::getFilename() const
{
	return filename.c_str();
}

void SkinnedDialog::setFilename(const TCHAR *filename)
{
	if (this->filename == filename)
		return;

	this->filename = filename;
	releaseState();
	releaseCompiledScript();
}

bool SkinnedDialog::addField(Field *field)
{
	if (Dialog::addField(field))
	{
		releaseCompiledScript();
		releaseState();
		field->setOnChangeCallback(SkinnedDialog::staticOnFieldChange, this);
		return true;
	}
	else
		return false;
}

void SkinnedDialog::setSize(const Size &size)
{
	if (getSize() == size)
		return;

	Dialog::setSize(size);
	releaseState();
}

int SkinnedDialog::compile()
{
	if (!fileChanged())
		return 1;

	releaseCompiledScript();

	struct _stat st = {0};
	if (_tstat(filename.c_str(), &st) != 0)
		return 0;

	std::tstring text;
	readFile(text);
	if (text.size() <= 0)
		return 0;

	script = new V8Script();
	script->setExceptionCallback(errorCallback, errorCallbackParam);

	if (!script->compile(text.c_str(), this))
	{
		releaseCompiledScript();
		return 0;
	}

	std::pair<SkinOptions *,DialogState *> pair = script->configure(this);
	opts = pair.first;
	defaultState = pair.second;
	if (opts == NULL)
	{
		releaseCompiledScript();
		return 0;
	}

	fileChangedTime = st.st_mtime;

	return 2;
}

DialogState * SkinnedDialog::getState()
{	
	if (state != NULL && !fileChanged())
		return state;

	releaseState();

	if (filename.size() <= 0)
		return NULL;

	if (!compile())
		return NULL;

	state = Dialog::createState();
	if (!script->run(state, opts, getInfo()))
	{
		releaseState();
		return NULL;
	}

	return state;
}

void SkinnedDialog::releaseCompiledScript()
{
	delete script;
	script = NULL;
	fileChangedTime = 0;

	delete opts;
	opts = NULL;

	delete defaultState;
	defaultState = NULL;
}

void SkinnedDialog::releaseState()
{
	delete state;
	state = NULL;
}

bool SkinnedDialog::fileChanged()
{
	if (filename.size() <= 0)
		return false;

	struct _stat st = {0};
	if (_tstat(filename.c_str(), &st) != 0)
		return false;

	return st.st_mtime > fileChangedTime;
}

void SkinnedDialog::readFile(std::tstring &ret)
{
	FILE* file = _tfopen(filename.c_str(), _T("rb"));
	if (file == NULL) 
		return;

	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	rewind(file);

	char* chars = new char[size + 1];
	chars[size] = '\0';
	for (int i = 0; i < size;) 
	{
		int read = fread(&chars[i], 1, size - i, file);
		i += read;
	}
	fclose(file);

	ret = Utf8ToTchar(chars);

	delete[] chars;
}

void SkinnedDialog::onFieldChange(const Field *field)
{
	releaseState();
}


void SkinnedDialog::staticOnFieldChange(void *param, const Field *field)
{
	_ASSERT(param != NULL);
	_ASSERT(field != NULL);

	SkinnedDialog *skdlg = (SkinnedDialog *) param;
	skdlg->onFieldChange(field);
}

void SkinnedDialog::setErrorCallback(MessageCallback cb, void *param /*= NULL*/)
{
	errorCallback = cb;
	errorCallbackParam = param;
}

void SkinnedDialog::setTraceCallback(MessageCallback cb, void *param /*= NULL*/)
{
	traceCallback = cb;
	traceCallbackParam = param;
}

void SkinnedDialog::trace(TCHAR *msg, ...)
{
	if (traceCallback == NULL)
		return;

	TCHAR buff[1024];
	memset(buff, 0, sizeof(buff));

	va_list args;
	va_start(args, msg);

	_vsntprintf(buff, MAX_REGS(buff) - 1, msg, args);
	buff[MAX_REGS(buff) - 1] = 0;

	va_end(args);

	traceCallback(traceCallbackParam, buff);
}

SkinOptions * SkinnedDialog::getOpts()
{
	compile();
	return opts;
}

DialogState * SkinnedDialog::getDefaultState()
{
	compile();
	return defaultState;
}