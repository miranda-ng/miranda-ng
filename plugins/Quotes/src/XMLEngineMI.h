#ifndef __0c3d1da4_92b7_431c_83e5_f998cd513f0d_XMLEngineMI_h__
#define __0c3d1da4_92b7_431c_83e5_f998cd513f0d_XMLEngineMI_h__

class CXMLEngineMI : public IXMLEngine
{
public:
	CXMLEngineMI();
	~CXMLEngineMI();

	virtual IXMLNode::TXMLNodePtr LoadFile(const tstring& rsFileName)const;
	virtual bool SaveFile(const tstring& rsFileName, const IXMLNode::TXMLNodePtr& pNode)const;
	virtual IXMLNode::TXMLNodePtr CreateNode(const tstring& rsName, const tstring& rsText)const;
};

#endif //__0c3d1da4_92b7_431c_83e5_f998cd513f0d_XMLEngineMI_h__
