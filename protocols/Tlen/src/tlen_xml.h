/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
Copyright (C) 2004-2007  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _TLEN_XML_H_
#define _TLEN_XML_H_

typedef enum { ELEM_OPEN, ELEM_CLOSE, ELEM_OPENCLOSE, ELEM_TEXT } XmlElemType;
typedef enum { NODE_OPEN, NODE_CLOSE } XmlNodeType;

typedef struct tagXmlAttr {
	char *name;
	char *value;
} XmlAttr;

typedef struct tagXmlNode {
	int depth;									// depth of the current node (1=root)
	char *name;									// tag name of the current node
	int numAttr;								// number of attributes
	int maxNumAttr;								// internal use (num of slots currently allocated to attr)
	XmlAttr **attr;								// attribute list
	int numChild;								// number of direct child nodes
	int maxNumChild;							// internal use (num of slots currently allocated to child)
	struct tagXmlNode **child;					// child node list
	char *text;
	XmlNodeType state;							// internal use by parser
} XmlNode;


typedef struct tagXmlState {
	XmlNode root;			// root is the document (depth = 0);
	// callback for depth=n element on opening/closing
	void (*callback1_open)(XmlNode *,void *);
	void (*callback1_close)(XmlNode *,void *);
	void (*callback2_open)(XmlNode *,void *);
	void (*callback2_close)(XmlNode *,void *);
	void *userdata1_open;
	void *userdata1_close;
	void *userdata2_open;
	void *userdata2_close;
} XmlState;

void TlenXmlInitState(XmlState *xmlState);
void TlenXmlDestroyState(XmlState *xmlState);
BOOL TlenXmlSetCallback(XmlState *xmlState, int depth, XmlElemType type, void (*callback)(XmlNode*, void*), void *userdata);
int TlenXmlParse(XmlState *xmlState, char *buffer, int datalen);
char *TlenXmlGetAttrValue(XmlNode *node, char *key);
XmlNode *TlenXmlGetChild(XmlNode *node, char *tag);
XmlNode *TlenXmlGetNthChild(XmlNode *node, char *tag, int nth);
XmlNode *TlenXmlGetChildWithGivenAttrValue(XmlNode *node, char *tag, char *attrKey, char *attrValue);
void TlenXmlFreeNode(XmlNode *node);
XmlNode *TlenXmlCopyNode(XmlNode *node);

XmlNode *TlenXmlCreateNode(char *name);
void TlenXmlAddAttr(XmlNode *n, char *name, char *value);
XmlNode *TlenXmlAddChild(XmlNode *n, char *name);
void TlenXmlAddText(XmlNode *n, char *text);

#endif

