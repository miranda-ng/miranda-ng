struct ISkinBackend;
struct ISkinElement;

struct SkinRenderParams
{
	HDC hdc;
	RECT rc;
};

struct ISkinDataSource
{
	virtual LPCTSTR GetText(const TCHAR *key) = 0;
	virtual HICON GetIcon(const TCHAR *key) = 0;
	virtual HBITMAP GetBitmap(const TCHAR *key) = 0;
	virtual ISkinBackend *GetObject(const TCHAR *key) = 0;
};

struct ISkinElement
{
	// general manadgement
	virtual void SetParent(ISkinElement *parent) = 0;
	virtual void LoadFromXml(HXML hXml) = 0;
	virtual void SetId(const TCHAR *id) = 0;
	virtual void SetDataSource(ISkinDataSource *ds) = 0;
	virtual void Destroy() = 0;

	// rendering and layouting
	virtual void Measure(SkinRenderParams *params) = 0;
	virtual void Layout(SkinRenderParams *params) = 0;
	virtual void Paint(SkinRenderParams *params) = 0;

	// element tree
	virtual bool IsComplexObject() = 0;
	virtual ISkinElement *GetParent() = 0;
	virtual int GetChildCount() = 0;
	virtual ISkinElement *GetChild(int index) = 0;
	virtual bool AppendChild(ISkinElement *child) = 0;
	virtual bool InsertChild(ISkinElement *child, int index) = 0;
	virtual void RemoveChild(ISkinElement *child) = 0;

	// element properties
	virtual void SetPropText(const TCHAR *key, const TCHAR *value) = 0;
	virtual const TCHAR *GetPropText(const TCHAR *key, const TCHAR *value) = 0;
	virtual void SetPropInt(const TCHAR *key, int value) = 0;
	virtual void SetPropIntText(const TCHAR *key, const TCHAR *value) = 0;
	virtual int GetPropInt(const TCHAR *key) = 0;
};

struct ISkinBackend
{
	virtual LPCTSTR GetText(const TCHAR *key) = 0;
	virtual HICON GetIcon(const TCHAR *key) = 0;
	virtual HBITMAP GetBitmap(const TCHAR *key) = 0;
	virtual ISkinBackend *GetObject(const TCHAR *key) = 0;
};
