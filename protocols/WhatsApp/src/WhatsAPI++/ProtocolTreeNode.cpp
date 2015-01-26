/*
 * ProtocolTreeNode.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include "../common.h" // #TODO Remove Miranda-dependency

#include "WAException.h"
#include "ProtocolTreeNode.h"

static std::string nilstr;

ProtocolTreeNode::ProtocolTreeNode(const string &_tag, vector<unsigned char>* _data, vector<ProtocolTreeNode*> *_children) :
	tag(_tag)
{
	data = _data;
	attributes = NULL;
	children = _children;
}

ProtocolTreeNode::ProtocolTreeNode(const string &_tag, ProtocolTreeNode *_child) :
	tag(_tag)
{
	this->data = NULL;
	this->attributes = NULL;
	this->children = new std::vector<ProtocolTreeNode*>();
	children->push_back(_child);
}

ProtocolTreeNode::~ProtocolTreeNode()
{
	delete this->attributes;

	if (this->children != NULL) {
		for (size_t i = 0; i < this->children->size(); i++)
			if (this->children->at(i) != NULL)
				delete this->children->at(i);
		delete this->children;
	}

	delete data;
}


string ProtocolTreeNode::toString() const
{
	string out;
	out += "<" + this->tag;
	if (this->attributes != NULL) {
		map<string, string>::iterator ii;
		for (ii = attributes->begin(); ii != attributes->end(); ii++)
			out += " " + ii->first + "=\"" + ii->second + "\"";
	}
	out += ">\n";
	out += getDataAsString();

	if (this->children != NULL) {
		vector<ProtocolTreeNode*>::iterator ii;
		for (ii = children->begin(); ii != children->end(); ii++)
			out += (*ii)->toString();
	}

	out += "</" + this->tag + ">\n";

	return out;
}

ProtocolTreeNode* ProtocolTreeNode::getChild(const string& id)
{
	if (this->children == NULL || this->children->size() == 0)
		return NULL;

	for (std::size_t i = 0; i < this->children->size(); i++)
		if (id.compare((*children)[i]->tag) == 0)
			return (*children)[i];

	return NULL;
}

ProtocolTreeNode* ProtocolTreeNode::getChild(size_t id)
{
	if (this->children == NULL || this->children->size() == 0)
		return NULL;

	if (children->size() > id)
		return (*children)[id];

	return NULL;
}

const string& ProtocolTreeNode::getAttributeValue(const string& attribute)
{
	if (this->attributes == NULL)
		return nilstr;

	map<string, string>::iterator it = attributes->find(attribute);
	if (it == attributes->end())
		return nilstr;

	return it->second;
}

vector<ProtocolTreeNode*> ProtocolTreeNode::getAllChildren()
{
	if (this->children == NULL)
		return vector<ProtocolTreeNode*>();

	return *this->children;
}

std::string ProtocolTreeNode::getDataAsString() const
{
	if (this->data == NULL)
		return nilstr;
	return std::string(this->data->begin(), this->data->end());
}

vector<ProtocolTreeNode*> ProtocolTreeNode::getAllChildren(const string &tag)
{
	vector<ProtocolTreeNode*> ret;

	if (this->children != NULL)
		for (size_t i = 0; i < this->children->size(); i++)
			if (tag.compare((*children)[i]->tag) == 0)
				ret.push_back((*children)[i]);

	return ret;
}

bool ProtocolTreeNode::tagEquals(ProtocolTreeNode *node, const string& tag)
{
	return (node != NULL && node->tag.compare(tag) == 0);
}

void ProtocolTreeNode::require(ProtocolTreeNode *node, const string& tag)
{
	if (!tagEquals(node, tag))
		throw WAException("failed require. node:" + node->toString() + "tag: " + tag, WAException::CORRUPT_STREAM_EX, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

ProtocolTreeNode& operator<<(ProtocolTreeNode &node, const XATTR &attr)
{
	if (node.attributes == NULL)
		node.attributes = new map<string, string>;

	(*node.attributes)[attr.name] = attr.value;
	return node;
}

ProtocolTreeNode* operator<<(ProtocolTreeNode *node, const XATTR &attr)
{
	if (node->attributes == NULL)
		node->attributes = new map<string, string>;

	(*node->attributes)[attr.name] = attr.value;
	return node;
}

ProtocolTreeNode& operator<<(ProtocolTreeNode &node, const XATTRI &attr)
{
	if (node.attributes == NULL)
		node.attributes = new map<string, string>;

	char szValue[100];
	_itoa_s(attr.value, szValue, 10);
	(*node.attributes)[attr.name] = szValue;
	return node;
}

ProtocolTreeNode* operator<<(ProtocolTreeNode *node, const XATTRI &attr)
{
	if (node->attributes == NULL)
		node->attributes = new map<string, string>;

	char szValue[100];
	_itoa_s(attr.value, szValue, 10);
	(*node->attributes)[attr.name] = szValue;
	return node;
}
