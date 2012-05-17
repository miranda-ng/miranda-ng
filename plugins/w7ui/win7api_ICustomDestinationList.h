#ifndef __ICustomDestinationList_INTERFACE_DEFINED__
#define __ICustomDestinationList_INTERFACE_DEFINED__

typedef /* [v1_enum] */ enum tagKNOWNDESTCATEGORY {
  KDC_FREQUENT = 1,
  KDC_RECENT = (KDC_FREQUENT + 1)
} KNOWNDESTCATEGORY;

MIDL_INTERFACE("6332debf-87b5-4670-90c0-5e57b408a49e")
ICustomDestinationList : public IUnknown {
 public:
  virtual HRESULT STDMETHODCALLTYPE SetAppID(
      /* [string][in] */__RPC__in_string LPCWSTR pszAppID) = 0;
  virtual HRESULT STDMETHODCALLTYPE BeginList(
      /* [out] */ __RPC__out UINT *pcMaxSlots,
      /* [in] */ __RPC__in REFIID riid,
      /* [iid_is][out] */ __RPC__deref_out_opt void **ppv) = 0;
  virtual HRESULT STDMETHODCALLTYPE AppendCategory(
      /* [string][in] */ __RPC__in_string LPCWSTR pszCategory,
      /* [in] */ __RPC__in_opt IObjectArray *poa) = 0;
  virtual HRESULT STDMETHODCALLTYPE AppendKnownCategory(
      /* [in] */ KNOWNDESTCATEGORY category) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddUserTasks(
      /* [in] */ __RPC__in_opt IObjectArray *poa) = 0;
  virtual HRESULT STDMETHODCALLTYPE CommitList(void) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetRemovedDestinations(
      /* [in] */ __RPC__in REFIID riid,
      /* [iid_is][out] */ __RPC__deref_out_opt void **ppv) = 0;
  virtual HRESULT STDMETHODCALLTYPE DeleteList(
      /* [string][in] */ __RPC__in_string LPCWSTR pszAppID) = 0;
  virtual HRESULT STDMETHODCALLTYPE AbortList(void) = 0;
};

#endif  // __ICustomDestinationList_INTERFACE_DEFINED__
