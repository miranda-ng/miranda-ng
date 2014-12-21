#ifndef __98ad6d6d_2a27_43fd_bf3e_c18416a45e54_IHTMLParser_h__
#define __98ad6d6d_2a27_43fd_bf3e_c18416a45e54_IHTMLParser_h__

class IHTMLNode
{
public:
	typedef boost::shared_ptr<IHTMLNode> THTMLNodePtr;

	enum EType
	{
		Table = 1,
		TableRow,
		TableColumn
	};

public:
	IHTMLNode() {}
	virtual ~IHTMLNode() {}

	virtual size_t GetChildCount()const = 0;
	virtual THTMLNodePtr GetChildPtr(size_t nIndex) = 0;
	virtual bool Is(EType nType)const = 0;

	virtual THTMLNodePtr GetElementByID(const tstring& rsID)const = 0;

	virtual tstring GetAttribute(const tstring& rsAttrName)const = 0;
	virtual tstring GetText()const = 0;
};

class IHTMLParser
{
public:
	typedef IHTMLNode::THTMLNodePtr THTMLNodePtr;
public:
	IHTMLParser() {}
	virtual ~IHTMLParser() {}

	virtual THTMLNodePtr ParseString(const tstring& rsHTML) = 0;
};

#endif //__98ad6d6d_2a27_43fd_bf3e_c18416a45e54_IHTMLParser_h__
