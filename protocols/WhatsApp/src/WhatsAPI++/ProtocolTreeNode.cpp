/*
 * ProtocolTreeNode.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include "WAException.h"
#include "ProtocolTreeNode.h"

ProtocolTreeNode::ProtocolTreeNode(const string& tag, map<string, string> *attributes, vector<unsigned char>* data, vector<ProtocolTreeNode*> *children) {
	this->tag = tag;
	this->data = data;
	this->attributes = attributes;
	this->children = children;
}

ProtocolTreeNode::ProtocolTreeNode(const string& tag, map<string, string> *attributes, ProtocolTreeNode* child) {
	this->tag = tag;
	this->data = NULL;
	this->attributes = attributes;
	this->children = new std::vector<ProtocolTreeNode*>(1);
	(*this->children)[0] = child;
}

ProtocolTreeNode::~ProtocolTreeNode() {
	if (this->attributes != NULL)
		delete this->attributes;
	if (this->children != NULL) {
		for (size_t i = 0; i < this->children->size(); i++)
			if (this->children->at(i) != NULL)
				delete this->children->at(i);
		delete this->children;
	}
	if (this->data != NULL)
		delete data;
}


string ProtocolTreeNode::toString() {
	string out;
	out += "<" + this->tag;
	if (this->attributes != NULL) {
		map<string,string>::iterator ii;
		for (ii = attributes->begin(); ii != attributes->end(); ii++)
			out += "" + ii->first + "=\"" + ii->second + "\"";
	}
	out += ">\n";
	std::string* data = getDataAsString();
	out += (this->data != NULL? *data:"");
	delete data;

	if (this->children != NULL) {
		vector<ProtocolTreeNode*>::iterator ii;

		for (ii = children->begin(); ii != children->end(); ii++)
			out += (*ii)->toString();
	}

	out += "</" + this->tag + ">\n";

	return out;
}

ProtocolTreeNode* ProtocolTreeNode::getChild(const string& id) {
	if (this->children == NULL || this->children->size() == 0)
		return NULL;

	for (std::size_t i = 0; i < this->children->size(); i++)
		if (id.compare((*children)[i]->tag) == 0)
			return (*children)[i];

	return NULL;
}

ProtocolTreeNode* ProtocolTreeNode::getChild(size_t id) {
	if (this->children == NULL || this->children->size() == 0)
		return NULL;

	if (children->size() > id)
		return (*children)[id];

	return NULL;
}

string* ProtocolTreeNode::getAttributeValue(const string& attribute) {
	if (this->attributes == NULL)
		return NULL;

	map<string,string>::iterator it = attributes->find(attribute);
	if (it == attributes->end())
		return NULL;

	return &it->second;
}

vector<ProtocolTreeNode*>* ProtocolTreeNode::getAllChildren() {
	vector<ProtocolTreeNode*>* ret = new vector<ProtocolTreeNode*>();

	if (this->children == NULL)
		return ret;

	return this->children;
}

std::string* ProtocolTreeNode::getDataAsString() {
	if (this->data == NULL)
		return NULL;
	return new std::string(this->data->begin(), this->data->end());
}

vector<ProtocolTreeNode*>* ProtocolTreeNode::getAllChildren(const string& tag) {
	vector<ProtocolTreeNode*>* ret = new vector<ProtocolTreeNode*>();

	if (this->children == NULL)
		return ret;

	for (size_t i = 0; i < this->children->size(); i++)
		if (tag.compare((*children)[i]->tag) == 0)
			ret->push_back((*children)[i]);

	return ret;
}


bool ProtocolTreeNode::tagEquals(ProtocolTreeNode *node, const string& tag) {
	return (node != NULL && node->tag.compare(tag) == 0);
}

void ProtocolTreeNode::require(ProtocolTreeNode *node, const string& tag) {
   if (!tagEquals(node, tag))
	   throw WAException("failed require. node:" + node->toString() + "tag: " + tag, WAException::CORRUPT_STREAM_EX, 0);
}
