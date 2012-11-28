#ifndef __85dbfa97_919b_4776_919c_7410a1c3d787_HTMLEngine_h__
#define __85dbfa97_919b_4776_919c_7410a1c3d787_HTMLEngine_h__

class IHTMLParser;

class IHTMLEngine
{
public:
	typedef boost::shared_ptr<IHTMLParser> THTMLParserPtr;

public:
	IHTMLEngine(void){}
	virtual ~IHTMLEngine() {}

	virtual THTMLParserPtr GetParserPtr()const = 0;
};

#endif //__85dbfa97_919b_4776_919c_7410a1c3d787_HTMLEngine_h__
