#include "globals.h"
#include "LabelFieldState_v8_wrapper.h"
#include <v8.h>
#include "LabelFieldState.h"

using namespace v8;


#ifdef UNICODE
# define V8_TCHAR uint16_t
#else
# define V8_TCHAR char
#endif


void AddLabelFieldStateAcessors(Handle<ObjectTemplate> &templ)
{
	HandleScope scope;
	
}
