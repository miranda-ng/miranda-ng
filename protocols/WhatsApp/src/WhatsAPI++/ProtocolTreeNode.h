/*
* ProtocolTreeNode.h
*
* Created on: 26/06/2012
* Author: Antonio
*/

#if !defined(PROTOCOLNODE_H)
#define PROTOCOLNODE_H

#include <string>
#include <vector>
#include <map>

using namespace std;

struct XATTR
{
	__forceinline XATTR(const char *_name, const char *_value) :
		name(_name), value(_value)
	{}

	__forceinline XATTR(const char *_name, const std::string &_value) :
		name(_name), value(_value.c_str())
	{}

	__forceinline XATTR(const std::string &_name, const std::string &_value) :
		name(_name.c_str()), value(_value.c_str())
	{}

	const char *name, *value;
};

struct XATTRI
{
	__forceinline XATTRI(const char *_name, int _value) :
		name(_name), value(_value)
	{}

	__forceinline XATTRI(const std::string &_name, int _value) :
		name(_name.c_str()), value(_value)
	{}

	const char *name;
	int value;
};

class ProtocolTreeNode
{
	ProtocolTreeNode(const ProtocolTreeNode&); // to prevent copying

public:
	vector<unsigned char>* data;
   string tag;
   map<string, string> *attributes;
   vector<ProtocolTreeNode*> *children;

   ProtocolTreeNode(const string& tag, ProtocolTreeNode* child);
   ProtocolTreeNode(const string& tag, vector<unsigned char>* data = NULL, vector<ProtocolTreeNode*> *children = NULL);
	~ProtocolTreeNode();

   string toString() const; 
   ProtocolTreeNode* getChild(const string& id);
   ProtocolTreeNode* getChild(size_t id);
   const string& getAttributeValue(const string& attribute);

   vector<ProtocolTreeNode*> getAllChildren();
   vector<ProtocolTreeNode*> getAllChildren(const string& tag);
	std::string getDataAsString() const;

   static bool tagEquals(ProtocolTreeNode *node, const string& tag);
   static void require(ProtocolTreeNode *node, const string& tag);
};

ProtocolTreeNode& operator<<(ProtocolTreeNode&, const XATTR&);
ProtocolTreeNode* operator<<(ProtocolTreeNode*, const XATTR&);

ProtocolTreeNode& operator<<(ProtocolTreeNode&, const XATTRI&);
ProtocolTreeNode* operator<<(ProtocolTreeNode*, const XATTRI&);

#endif /* PROTOCOLNODE_H_ */