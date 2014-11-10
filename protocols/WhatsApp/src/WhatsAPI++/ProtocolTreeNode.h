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

class ProtocolTreeNode {
public:
   vector<unsigned char>* data;
   string tag;
   map<string, string> *attributes;
   vector<ProtocolTreeNode*> *children;

   ProtocolTreeNode(const string& tag, map<string, string> *attributes, ProtocolTreeNode* child);
   ProtocolTreeNode(const string& tag, map<string, string> *attributes, vector<unsigned char>* data = NULL, vector<ProtocolTreeNode*> *children = NULL);
   string toString();
   ProtocolTreeNode* getChild(const string& id);
   ProtocolTreeNode* getChild(size_t id);
   const string& getAttributeValue(const string& attribute);

   vector<ProtocolTreeNode*> getAllChildren();
   vector<ProtocolTreeNode*> getAllChildren(const string& tag);
   std::string getDataAsString();

   static bool tagEquals(ProtocolTreeNode *node, const string& tag);
   static void require(ProtocolTreeNode *node, const string& tag);

   virtual ~ProtocolTreeNode();
};

#endif /* PROTOCOLNODE_H_ */