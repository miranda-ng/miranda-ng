#ifndef __IObjectCollection_INTERFACE_DEFINED__
#define __IObjectCollection_INTERFACE_DEFINED__

MIDL_INTERFACE("5632b1a4-e38a-400a-928a-d4cd63230295")
IObjectCollection : public IObjectArray {
 public:
  virtual HRESULT STDMETHODCALLTYPE AddObject(
      /* [in] */ __RPC__in_opt IUnknown *punk) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddFromArray(
      /* [in] */ __RPC__in_opt IObjectArray *poaSource) = 0;
  virtual HRESULT STDMETHODCALLTYPE RemoveObjectAt(
      /* [in] */ UINT uiIndex) = 0;
  virtual HRESULT STDMETHODCALLTYPE Clear(void) = 0;
};

#endif  // __IObjectCollection_INTERFACE_DEFINED__
