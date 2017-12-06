#ifndef __8C9706FF_6B05_4d0d_85B8_5724E5DC0BA4_HTTPSession_h__
#define __8C9706FF_6B05_4d0d_85B8_5724E5DC0BA4_HTTPSession_h__

class CHTTPSession
{
public:
	CHTTPSession();
	~CHTTPSession();

	static bool Init();

	bool OpenURL(const tstring& rsURL);
	bool ReadResponce(tstring& rsResponce)const;


public:
	class CImpl;
private:
	typedef boost::scoped_ptr<CImpl> TImpl;

private:
	TImpl m_pImpl;
};

#endif //__8C9706FF_6B05_4d0d_85B8_5724E5DC0BA4_HTTPSession_h__
