#include "globals.h"
#include "EditFieldState_v8_wrapper.h"
#include <v8.h>
#include "EditFieldState.h"

using namespace v8;


#ifdef UNICODE
# define V8_TCHAR uint16_t
#else
# define V8_TCHAR char
#endif


void AddEditFieldStateAcessors(Handle<ObjectTemplate> &templ)
{
	HandleScope scope;
	
}
