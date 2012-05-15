#ifdef _ALPHA_FUSE_
#ifndef _M_FUSE_
#define _M_FUSE_

#define CALLSERVICE_NOTFOUND      ((int)0x80000000)
#define MAXMODULELABELLENGTH 64
typedef int (*MIRANDAHOOK)(WPARAM,LPARAM);
typedef INT_PTR (*MIRANDASERVICE)(WPARAM,LPARAM);

#define FUSE_INIT	0			// core started, Param=**FUSE_LINK
#define FUSE_DEINIT 1			// core stopped
#define FUSE_DEFMOD	3			// LoadDefaultModules() return code, Param=*int
#define FUSE_DEATH  4			// DestroyingModularEngine() just got called

typedef struct {
	int cbSize;
	HANDLE (*CreateHookableEvent)(const char *);
	int (*DestroyHookableEvent)(HANDLE);
	int (*NotifyEventHooks)(HANDLE,WPARAM,LPARAM);
	HANDLE (*HookEvent)(const char *,MIRANDAHOOK);
	HANDLE (*HookEventMessage)(const char *,HWND,UINT);
	int (*UnhookEvent)(HANDLE);
	HANDLE (*CreateServiceFunction)(const char *,MIRANDASERVICE);
	HANDLE (*CreateTransientServiceFunction)(const char *,MIRANDASERVICE);
	int (*DestroyServiceFunction)(HANDLE);
	int (*CallService)(const char *,WPARAM,LPARAM);
	int (*ServiceExists)(const char *);		  //v0.1.0.1+
	int (*CallServiceSync)(const char*,WPARAM,LPARAM); //v0.1.2.2+
} FUSE_LINK;

#endif
#endif