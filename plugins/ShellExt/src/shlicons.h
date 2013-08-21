
struct TWICBitmap : public IWICBitmapSource
{
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

   virtual HRESULT STDMETHODCALLTYPE GetSize(UINT *puiWidth, UINT *puiHeight);
   virtual HRESULT STDMETHODCALLTYPE GetPixelFormat(WICPixelFormatGUID *pPixelFormat);
   virtual HRESULT STDMETHODCALLTYPE GetResolution(double *pDpiX, double *pDpiY);
   virtual HRESULT STDMETHODCALLTYPE CopyPalette(IWICPalette *pIPalette);
   virtual HRESULT STDMETHODCALLTYPE CopyPixels(const WICRect *prc, UINT cbStride, UINT cbBufferSize, BYTE *pbBuffer);
};

struct TImageFactory : public IImagingFactory
{
};

TImageFactory* ARGB_GetWorker();

HBITMAP ARGB_BitmapFromIcon(TImageFactory* Factory, HDC hdc, HICON hIcon);
