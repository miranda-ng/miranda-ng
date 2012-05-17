#ifndef __ITaskbarList3_FWD_DEFINED__
#define __ITaskbarList3_FWD_DEFINED__
typedef interface ITaskbarList3 ITaskbarList3;
#endif 	/* __ITaskbarList3_FWD_DEFINED__ */

/* interface ITaskbarList3 */
/* [object][uuid] */ 
   
MIDL_INTERFACE("ea1afb91-9e28-4b86-90e9-9e9f8a5eefaf")
ITaskbarList3 : public ITaskbarList2
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetProgressValue( 
        /* [in] */ __RPC__in HWND hwnd,
        /* [in] */ ULONGLONG ullCompleted,
        /* [in] */ ULONGLONG ullTotal) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetProgressState( 
        /* [in] */ __RPC__in HWND hwnd,
        /* [in] */ TBPFLAG tbpFlags) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE RegisterTab( 
        /* [in] */ __RPC__in HWND hwndTab,
        /* [in] */ __RPC__in HWND hwndMDI) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE UnregisterTab( 
        /* [in] */ __RPC__in HWND hwndTab) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetTabOrder( 
        /* [in] */ __RPC__in HWND hwndTab,
        /* [in] */ __RPC__in HWND hwndInsertBefore) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetTabActive( 
        /* [in] */ __RPC__in HWND hwndTab,
        /* [in] */ __RPC__in HWND hwndMDI,
        /* [in] */ TBATFLAG tbatFlags) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE ThumbBarAddButtons( 
        /* [in] */ __RPC__in HWND hwnd,
        /* [in] */ UINT cButtons,
        /* [size_is][in] */ __RPC__in_ecount_full(cButtons) LPTHUMBBUTTON pButton) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE ThumbBarUpdateButtons( 
        /* [in] */ __RPC__in HWND hwnd,
        /* [in] */ UINT cButtons,
        /* [size_is][in] */ __RPC__in_ecount_full(cButtons) LPTHUMBBUTTON pButton) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE ThumbBarSetImageList( 
        /* [in] */ __RPC__in HWND hwnd,
        /* [in] */ __RPC__in_opt HIMAGELIST himl) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetOverlayIcon( 
        /* [in] */ __RPC__in HWND hwnd,
        /* [in] */ __RPC__in HICON hIcon,
        /* [string][in] */ __RPC__in_string LPCWSTR pszDescription) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetThumbnailTooltip( 
        /* [in] */ __RPC__in HWND hwnd,
        /* [string][in] */ __RPC__in_string LPCWSTR pszTip) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetThumbnailClip( 
        /* [in] */ __RPC__in HWND hwnd,
        /* [in] */ __RPC__in RECT *prcClip) = 0;

};
