/* MinGW misses shobjidl.h,extracted from WinAPI documenttion */

#ifndef _SHOBJIDL_H
#define _SHOBJIDL_H

#ifdef __MINGW32__
#include <initguid.h>
#define CMIC_MASK_ASYNCOK SEE_MASK_ASYNCOK

DEFINE_GUID(IID_IDropTargetHelper,0x4657278b,0x411b,0x11d2,0x83,0x9a,0x00,0xc0,0x4f,0xd9,0x18,0xd0);
DEFINE_GUID(IID_IPersistFolder2,0x1ac3d9f0,0x175c,0x11d1,0x95,0xbe,0x00,0x60,0x97,0x97,0xea,0x4f);
DEFINE_GUID(IID_IShellFolder2,0x93f2f68c,0x1d1b,0x11d3,0xa3,0x0e,0x00,0xc0,0x4f,0x79,0xab,0xd1);
DEFINE_GUID(IID_IPersistFolder3,0xcef04fdf,0xfe72,0x11d2,0x87,0xa5,0x00,0xc0,0x4f,0x68,0x37,0xcf);

/*** IUnknown methods ***/
#define IShellExecuteHookW_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IShellExecuteHookW_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IShellExecuteHookW_Release(p) (p)->lpVtbl->Release(p)
/*** IShellExecuteHookW methods ***/
#define IShellExecuteHookW_Execute(p,a) (p)->lpVtbl->Execute(p,a)

#ifdef COBJMACROS
/*** IUnknown methods ***/
#define IShellExtInit_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define IShellExtInit_AddRef(p) (p)->lpVtbl->AddRef(p)
#define IShellExtInit_Release(p) (p)->lpVtbl->Release(p)
/*** IShellExtInit methods ***/
#define IShellExtInit_Initialize(p,a,b,c) (p)->lpVtbl->Initialize(p,a,b,c)
#endif

DEFINE_GUID(CLSID_TaskbarList,0x56fdf344,0xfd6d,0x11d0,0x95,0x8a,0x0,0x60,0x97,0xc9,0xa0,0x90);
DEFINE_GUID(IID_ITaskbarList,0x56fdf342,0xfd6d,0x11d0,0x95,0x8a,0x0,0x60,0x97,0xc9,0xa0,0x90);

DECLARE_INTERFACE_(ITaskbarList,IUnknown)
{
/*** IUnknown methods ***/
STDMETHOD(QueryInterface) (THIS_ REFIID riid,void **ppv) PURE;
STDMETHOD_(ULONG,AddRef) (THIS) PURE;
STDMETHOD_(ULONG,Release) (THIS) PURE;
/*** ITaskbarList specific methods ***/
STDMETHOD(HrInit) (THIS) PURE;
STDMETHOD(AddTab) (THIS_ HWND hwnd) PURE;
STDMETHOD(DeleteTab) (THIS_ HWND hwnd) PURE;
STDMETHOD(ActivateTab) (THIS_ HWND hwnd) PURE;
STDMETHOD(SetActiveAlt) (THIS_ HWND hwnd) PURE;
};
typedef ITaskbarList *LPITaskbarList;

#endif /* __MINGW32__ */
#endif /* _SHOBJIDL_H */
