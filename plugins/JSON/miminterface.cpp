#define WIN32_LEAN_AND_MEAN
#define MIID_LIBJSON { 0x2004b4be, 0xfbb9, 0x4c74, { 0x8c, 0xb6, 0x26, 0x86, 0x7f, 0xca, 0x33, 0x4e } }

#include <windows.h>
#include "newpluginapi.h"
#include "m_plugins.h"
#include "m_system.h"
#include "libJSON.h"
#include "m_libJSON.h"

MM_INTERFACE   mmi;
PLUGINLINK* pluginLink;				// Struct of functions pointers for service calls

static const MUUID interfaces[] = {MIID_LIBJSON,MIID_LAST};

static HANDLE s_services[53];

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"JSON Service",
	PLUGIN_MAKE_VERSION(0,0,0,2),
	"JSON Library for Miranda IM / DLLExport interface (" __DATE__ " " __TIME__" libJSON 0.6)",
	"Stark Wong",
	"starkwong@hotmail.com",
	"(C)2011 Stark Wong",
	"http://www.studiokuma.com",
	UNICODE_AWARE,
	0,
	// {2004B4BE-FBB9-4c74-8CB6-26867FCA334E}
	MIID_LIBJSON
};

extern "C" {
	BOOL WINAPI DllMain(HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved) {
		return TRUE;
	}

	__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) 
	{
		return &pluginInfo;
	}

	__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void) {
		return interfaces;
	}

	__declspec(dllexport) int Unload(void) {
		for (HANDLE* service=s_services; *service; service++) {
			DestroyServiceFunction(*service);
		}
		return 0;
	}

	INT_PTR GetInterface(WPARAM wParam, LPARAM lParam) {
		LPJSONSERVICEINTERFACE lpJSI=(LPJSONSERVICEINTERFACE)wParam;
		memset(lpJSI,0,sizeof(JSONSERVICEINTERFACE));

#define SETJSI(x) lpJSI->##x=json_##x
		SETJSI(free);
		lpJSI->delete_=json_delete;
#ifdef JSON_MEMORY_MANAGE
		SETJSI(free_all);
		SETJSI(delete_all);
#endif
		SETJSI(parse);
		SETJSI(strip_white_space);
#ifdef JSON_VALIDATE
		SETJSI(validate);
#endif
		SETJSI(new_a);
		SETJSI(new_i);
		SETJSI(new_f);
		SETJSI(new_b);
		lpJSI->new_=json_new;
		SETJSI(copy);
		SETJSI(duplicate);
		SETJSI(set_a);
		SETJSI(set_i);
		SETJSI(set_f);
		SETJSI(set_b);
		SETJSI(set_n);
		SETJSI(type);
		SETJSI(size);
		SETJSI(empty);
		SETJSI(name);
#ifdef JSON_COMMENTS
		SETJSI(get_comment);
#endif
		SETJSI(as_string);
		SETJSI(as_int);
		SETJSI(as_float);
		SETJSI(as_bool);
		SETJSI(as_node);
		SETJSI(as_array);
		#ifdef JSON_BINARY
		SETJSI(as_binary);
		#endif
		#ifdef JSON_WRITER
		SETJSI(write);
		SETJSI(write_formatted);
		#endif
		SETJSI(set_name);
		#ifdef JSON_COMMENTS
		SETJSI(set_comment);
		#endif
		SETJSI(clear);
		SETJSI(nullify);
		SETJSI(swap);
		SETJSI(merge);
		#ifndef JSON_PREPARSE
		SETJSI(preparse);
		#endif
		#ifdef JSON_BINARY
		SETJSI(set_binary);
		#endif
		SETJSI(cast);

		//children access
		SETJSI(reserve);
		SETJSI(at);
		SETJSI(get);
		#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
		SETJSI(get_nocase);
		SETJSI(pop_back_nocase);
		#endif
		SETJSI(push_back);
		SETJSI(pop_back_at);
		SETJSI(pop_back);
		#ifdef JSON_ITERATORS
		SETJSI(find);
		#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
		SETJSI(find_nocase);
		#endif
		SETJSI(erase);
		SETJSI(erase_multi);
		SETJSI(insert);
		SETJSI(insert_multi);

		//iterator functions
		SETJSI(begin);
		SETJSI(end);
		#endif

		SETJSI(equal);

		return 0;
	}

// WRAPPER_[in][out]
// (ext,in)
// (ext,in,param)
// (ext,in,param1,param2)
#define WRAPPER_00(x,y) static INT_PTR JSON_##x(WPARAM wParam, LPARAM lParam) { y(); return 0; }
#define WRAPPER_10(x,y,z) static INT_PTR JSON_##x(WPARAM wParam, LPARAM lParam) { y((z)wParam); return 0; }
#define WRAPPER_01(x,y) static INT_PTR JSON_##x(WPARAM wParam, LPARAM lParam) { return (INT_PTR)y(); }
#define WRAPPER_11(x,y,z) static INT_PTR JSON_##x(WPARAM wParam, LPARAM lParam) { return (INT_PTR)y((z)wParam); }
#define WRAPPER_20(w,x,y,z) static INT_PTR JSON_##w(WPARAM wParam, LPARAM lParam) { x((y)wParam,(z)lParam); return 0; }
#define WRAPPER_21(w,x,y,z) static INT_PTR JSON_##w(WPARAM wParam, LPARAM lParam) { return (INT_PTR)x((y)wParam,(z)lParam); }

WRAPPER_10(FREE,json_free,LPVOID)
WRAPPER_10(DELETE,json_delete,JSONNODE*)
#ifdef JSON_MEMORY_MANAGE
WRAPPER_00(FREE_ALL,json_free_all)
WRAPPER_00(DELETE_ALL,json_delete_all)
#endif // JSON_MEMORY_MANAGE
WRAPPER_11(PARSE,json_parse,LPCSTR);
WRAPPER_11(STRIP_WHITE_SPACE,json_strip_white_space,LPCSTR);
#ifdef JSON_VALIDATE
WRAPPER_11(VALIDATE,json_validate,LPCSTR);
#endif // JSON_VALIDATE
WRAPPER_21(NEW_STRING,json_new_a,LPCSTR,LPCSTR);
WRAPPER_21(NEW_INTEGER,json_new_i,LPCSTR,long);
WRAPPER_21(NEW_FLOAT,json_new_f,LPCSTR,float);
WRAPPER_21(NEW_BOOLEAN,json_new_b,LPCSTR,int);
WRAPPER_11(NEW,json_new,char);
WRAPPER_11(COPY,json_copy,JSONNODE*);
WRAPPER_11(DUPLICATE,json_duplicate,JSONNODE*);
WRAPPER_20(SET_STRING,json_set_a,JSONNODE*,LPCSTR);
WRAPPER_20(SET_INTEGER,json_set_i,JSONNODE*,long);
WRAPPER_20(SET_FLOAT,json_set_f,JSONNODE*,float);
WRAPPER_20(SET_BOOLEAN,json_set_b,JSONNODE*,int);
WRAPPER_20(SET_NODE,json_set_n,JSONNODE*,JSONNODE*);
WRAPPER_11(TYPE,json_type,JSONNODE*);
WRAPPER_11(SIZE,json_size,JSONNODE*);
WRAPPER_11(EMPTY,json_empty,JSONNODE*);
WRAPPER_11(NAME,json_name,JSONNODE*);
#ifdef JSON_COMMENTS
WRAPPER_11(COMMENT,json_comment,JSONNODE*);
WRAPPER_20(SET_COMMENT,json_set_comment,JSONNODE*,LPCSTR);
#endif // JSON_COMMENTS
WRAPPER_11(AS_STRING,json_as_string,JSONNODE*);
WRAPPER_11(AS_INTEGER,json_as_int,JSONNODE*);
WRAPPER_11(AS_FLOAT,json_as_float,JSONNODE*);
WRAPPER_11(AS_BOOLEAN,json_as_bool,JSONNODE*);
WRAPPER_11(AS_NODE,json_as_node,JSONNODE*);
WRAPPER_11(AS_ARRAY,json_as_array,JSONNODE*);
#if JSON_BINARY
WRAPPER_21(AS_BINARY,json_as_binary,JSONNODE*,LPDWORD);
#endif // JSON_BINARY
WRAPPER_11(WRITE,json_write,JSONNODE*);
WRAPPER_11(WRITE_FORMATTED,json_write_formatted,JSONNODE*);
WRAPPER_20(SET_NAME,json_set_name,JSONNODE*,LPCSTR);
WRAPPER_10(CLEAR,json_clear,JSONNODE*);
WRAPPER_10(NULLIFY,json_nullify,JSONNODE*);
WRAPPER_20(SWAP,json_swap,JSONNODE*,JSONNODE*);
WRAPPER_20(MERGE,json_merge,JSONNODE*,JSONNODE*);
WRAPPER_10(PREPARSE,json_preparse,JSONNODE*);
// LIBJSON_DLL void json_set_binary(JSONNODE * node, const void * data, unsigned long length);
WRAPPER_20(CAST,json_cast,JSONNODE*,char);
WRAPPER_20(RESERVE,json_reserve,JSONNODE*,int);
WRAPPER_21(AT,json_at,JSONNODE*,int);
WRAPPER_21(GET,json_get,JSONNODE*,LPSTR);
#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
WRAPPER_21(GET_NOCASE,json_get_nocase,JSONNODE*,LPCSTR);
WRAPPER_21(POP_BACK_NOCASE,json_pop_back_nocase,JSONNODE*,LPCSTR);
#endif // JSON_CASE_INSENSITIVE_FUNCTIONS
WRAPPER_20(PUSH_BACK,json_push_back,JSONNODE*,JSONNODE*);
WRAPPER_21(POP_BACK_AT,json_pop_back_at,JSONNODE*,int);
WRAPPER_21(POP_BACK,json_pop_back,JSONNODE*,LPCSTR);
/*
#ifdef JSON_ITERATORS
 LIBJSON_DLL JSONNODE_ITERATOR json_find(JSONNODE * node, const json_char * name);
 #ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	LIBJSON_DLL JSONNODE_ITERATOR json_find_nocase(JSONNODE * node, const json_char * name);
 #endif
 LIBJSON_DLL JSONNODE_ITERATOR json_erase(JSONNODE * node, JSONNODE_ITERATOR it);
 LIBJSON_DLL JSONNODE_ITERATOR json_erase_multi(JSONNODE * node, JSONNODE_ITERATOR start, JSONNODE_ITERATOR end);
 LIBJSON_DLL JSONNODE_ITERATOR json_insert(JSONNODE * node, JSONNODE_ITERATOR it, JSONNODE * node2);
 LIBJSON_DLL JSONNODE_ITERATOR json_insert_multi(JSONNODE * node, JSONNODE_ITERATOR it, JSONNODE_ITERATOR start, JSONNODE_ITERATOR end);

 //iterator functions
 LIBJSON_DLL JSONNODE_ITERATOR json_begin(JSONNODE * node);
 LIBJSON_DLL JSONNODE_ITERATOR json_end(JSONNODE * node);
#endif
*/
WRAPPER_21(EQUAL,json_equal,JSONNODE*,JSONNODE*);

#define CSF(x) s_services[ciServices++]=CreateServiceFunction(MS_JSON_##x,JSON_##x)

	__declspec(dllexport) int Load(PLUGINLINK *link) {
		int ciServices=0;

		pluginLink=link;
		mir_getMMI(&mmi);

		s_services[ciServices++] = CreateServiceFunction(MS_JSON_GETINTERFACE,GetInterface);

		CSF(FREE);
		CSF(DELETE);
#ifdef JSON_MEMORY_MANAGE
		CSF(FREE_ALL);
		CSF(DELETE_ALL);
#endif // JSON_MEMORY_MANAGE
		CSF(PARSE);
		CSF(STRIP_WHITE_SPACE);
#ifdef JSON_VALIDATE
		CSF(VALIDATE);
#endif // JSON_VALIDATE
		CSF(NEW_STRING);
		CSF(NEW_INTEGER);
		CSF(NEW_FLOAT);
		CSF(NEW_BOOLEAN);
		CSF(NEW);
		CSF(COPY);
		CSF(DUPLICATE);
		CSF(SET_STRING);
		CSF(SET_INTEGER);
		CSF(SET_FLOAT);
		CSF(SET_BOOLEAN);
		CSF(SET_NODE);
		CSF(TYPE);
		CSF(SIZE);
		CSF(EMPTY);
		CSF(NAME);
#ifdef JSON_COMMENTS
		CSF(COMMENT);
		CSF(SET_COMMENT);
#endif // JSON_COMMENTS
		CSF(AS_STRING);
		CSF(AS_INTEGER);
		CSF(AS_FLOAT);
		CSF(AS_BOOLEAN);
		CSF(AS_NODE);
		CSF(AS_ARRAY);
#if JSON_BINARY
		CSF(AS_BINARY);
#endif // JSON_BINARY
		CSF(WRITE);
		CSF(WRITE_FORMATTED);
		CSF(SET_NAME);
		CSF(CLEAR);
		CSF(NULLIFY);
		CSF(SWAP);
		CSF(MERGE);
		CSF(PREPARSE);
		// LIBJSON_DLL void json_set_binary(JSONNODE * node, const void * data, unsigned long length);
		CSF(CAST);
		CSF(RESERVE);
		CSF(AT);
		CSF(GET);
#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
		CSF(GET_NOCASE);
		CSF(POP_BACK_NOCASE);
#endif // JSON_CASE_INSENSITIVE_FUNCTIONS
		CSF(PUSH_BACK);
		CSF(POP_BACK_AT);
		CSF(POP_BACK);
		CSF(EQUAL);
		s_services[ciServices]=0;

		json_register_memory_callbacks((json_malloc_t)mmi.mmi_malloc,(json_realloc_t)mmi.mmi_realloc,(json_free_t)mmi.mmi_free);
		return 0;
	}
} // extern "C"