#include "hdr/modern_commonheaders.h"
#include "hdr/modern_callproc.h"

//////////////////////////////////////////////////////////////////////////
// standard callback proc
int CALLBACK __DoCallBack( LPARAM pvStorage)
{
    call::__baseCall * storage = (call::__baseCall *) pvStorage;
    int res=storage->__DoCallStorageProc();
    delete storage;
    return res;
}

//////////////////////////////////////////////////////////////////////////
// To do: Replace __ProcessCall to switching threads etc 
// and specify _DoCallback as callback
int call::__ProcessCall( __baseCall * pStorage, ASYNC_T async )
{
    return __DoCallBack( (LPARAM)pStorage);
}
