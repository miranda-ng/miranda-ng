#ifndef __f88e20d7_5e65_40fb_a7b5_7c7af1ee1c78_IXMLEngine_h__
#define __f88e20d7_5e65_40fb_a7b5_7c7af1ee1c78_IXMLEngine_h__

class IXMLNode
{
public:
	typedef boost::shared_ptr<IXMLNode> TXMLNodePtr;

public:
	IXMLNode() {}
	virtual ~IXMLNode() {}

	virtual size_t GetChildCount()const = 0;
	virtual TXMLNodePtr GetChildNode(size_t nIndex)const = 0;

	virtual tstring GetText()const = 0;
	virtual tstring GetName()const = 0;

	virtual bool AddChild(const TXMLNodePtr& pNode) = 0;
	virtual bool AddAttribute(const tstring& rsName, const tstring& rsValue) = 0;
	virtual tstring GetAttributeValue(const tstring& rsAttrName) = 0;
	virtual void Write(tostream& o)const = 0;
};

inline tostream& operator<<(tostream& o, const IXMLNode& node)
{
	node.Write(o);
	return o;
}

class IXMLEngine
{
public:
	IXMLEngine() {}

	virtual ~IXMLEngine() {}

	virtual IXMLNode::TXMLNodePtr LoadFile(const tstring& rsFileName)const = 0;
	virtual bool SaveFile(const tstring& rsFileName, const IXMLNode::TXMLNodePtr& pNode)const = 0;
	virtual IXMLNode::TXMLNodePtr CreateNode(const tstring& rsName, const tstring& rsText)const = 0;
};

#endif //__f88e20d7_5e65_40fb_a7b5_7c7af1ee1c78_IXMLEngine_h__
