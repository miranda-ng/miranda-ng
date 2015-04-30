#ifndef __3c99e3f7_ecd9_4d9b_8f86_fe293c5fc8e6_HTMLParserMS_h__
#define __3c99e3f7_ecd9_4d9b_8f86_fe293c5fc8e6_HTMLParserMS_h__

class CHTMLParserMS : public IHTMLParser
{
public:
	CHTMLParserMS();
	~CHTMLParserMS();

	virtual THTMLNodePtr ParseString(const tstring& rsHTML);

	static bool IsInstalled();

private:
	bool m_bCallUninit;
	CComPtr<IHTMLDocument2> m_pDoc;
	CComPtr<IMarkupServices>  m_pMS;
	CComPtr<IMarkupPointer>  m_pMkStart;
	CComPtr<IMarkupPointer>  m_pMkFinish;
	mutable mir_cs m_cs;
};

class CHTMLEngineMS : public IHTMLEngine
{
public:
	CHTMLEngineMS();
	~CHTMLEngineMS();

	virtual THTMLParserPtr GetParserPtr()const;
};

#endif //__3c99e3f7_ecd9_4d9b_8f86_fe293c5fc8e6_HTMLParserMS_h__
