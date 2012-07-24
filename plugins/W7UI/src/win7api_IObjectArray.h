#ifndef __IObjectArray_INTERFACE_DEFINED__
#define __IObjectArray_INTERFACE_DEFINED__

MIDL_INTERFACE("92CA9DCD-5622-4bba-A805-5E9F541BD8C9")
IObjectArray : public IUnknown {
 public:
  virtual HRESULT STDMETHODCALLTYPE GetCount(
      /* [out] */ __RPC__out UINT *pcObjects) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetAt(
      /* [in] */ UINT uiIndex,
      /* [in] */ __RPC__in REFIID riid,
      /* [iid_is][out] */ __RPC__deref_out_opt void **ppv) = 0;
};

#endif  // __IObjectArray_INTERFACE_DEFINED__
