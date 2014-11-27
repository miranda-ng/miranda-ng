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

#include "tlen.h"
#include <ctype.h>

static BOOL TlenXmlProcessElem(XmlState *xmlState, XmlElemType elemType, char *elemText, char *elemAttr);
static void TlenXmlRemoveChild(XmlNode *node, XmlNode *child);

void TlenXmlInitState(XmlState *xmlState)
{
	if (xmlState == NULL) return;
	xmlState->root.name = NULL;
	xmlState->root.depth = 0;
	xmlState->root.numAttr = 0;
	xmlState->root.maxNumAttr = 0;
	xmlState->root.attr = NULL;
	xmlState->root.numChild = 0;
	xmlState->root.maxNumChild = 0;
	xmlState->root.child = NULL;
	xmlState->root.text = NULL;
	xmlState->root.state = NODE_OPEN;
	xmlState->callback1_open = NULL;
	xmlState->callback1_close = NULL;
	xmlState->callback2_open = NULL;
	xmlState->callback2_close = NULL;
	xmlState->userdata1_open = NULL;
	xmlState->userdata1_close = NULL;
	xmlState->userdata2_open = NULL;
	xmlState->userdata2_close = NULL;
}

void TlenXmlDestroyState(XmlState *xmlState)
{
	int i;
	XmlNode *node;

	if (xmlState == NULL)
		return;
	// Note: cannot use TlenXmlFreeNode() to free xmlState->root
	// because it will do mir_free(xmlState->root) which is not freeable.
	node = &(xmlState->root);
	// Free all children first
	for (i=0; i<node->numChild; i++)
		TlenXmlFreeNode(node->child[i]);
	mir_free(node->child);
	// Free all attributes
	for (i=0; i<node->numAttr; i++) {
		mir_free(node->attr[i]->name);
		mir_free(node->attr[i]->value);
		mir_free(node->attr[i]);
	}
	mir_free(node->attr);
	// Free string field
	mir_free(node->text);
	mir_free(node->name);
	/* mir_free(xmlState) - no need, work with static. */
}

BOOL TlenXmlSetCallback(XmlState *xmlState, int depth, XmlElemType type, void (*callback)(XmlNode*, void*), void *userdata)
{
	if (depth == 1 && type == ELEM_OPEN) {
		xmlState->callback1_open = callback;
		xmlState->userdata1_open = userdata;
	}
	else if (depth == 1 && type == ELEM_CLOSE) {
		xmlState->callback1_close = callback;
		xmlState->userdata1_close = userdata;
	}
	else if (depth == 2 && type == ELEM_OPEN) {
		xmlState->callback2_open = callback;
		xmlState->userdata2_open = userdata;
	}
	else if (depth == 2 && type == ELEM_CLOSE) {
		xmlState->callback2_close = callback;
		xmlState->userdata2_close = userdata;
	}
	else
		return FALSE;

	return TRUE;
}

#define TAG_MAX_LEN 50
#define ATTR_MAX_LEN 1024
int TlenXmlParse(XmlState *xmlState, char *buffer, int datalen)
{
	char *p, *q, *r, *eob;
	char *str;
	int num;
	char tag[TAG_MAX_LEN];
	char attr[ATTR_MAX_LEN];
	XmlElemType elemType = ELEM_OPEN;

	eob = buffer + datalen;
	num = 0;
	// Skip leading whitespaces
	for (p=buffer; p<eob && isspace(*p); p++,num++);
	while (num < datalen) {
		if (*p == '<') {	// found starting bracket
			for (q=p+1; q<eob && *q != '>'; q++);
			if (q < eob) {	// found closing bracket
				for (r=p+1; *r != '>' && *r != ' ' && *r != '\t'; r++);
				if (r-(p+1) > TAG_MAX_LEN) {
//					TlenLog("TAG_MAX_LEN too small, ignore current tag");
				}
				else {
					if (*(p+1) == '/') {	// closing tag
						strncpy(tag, p+2, r-(p+2));
						tag[r-(p+2)] = '\0';
						elemType = ELEM_CLOSE;
					}
					else {
						if (*(r-1) == '/') {	// single open/close tag
							strncpy(tag, p+1, r-(p+1)-1);
							tag[r-(p+1)-1] = '\0';
							elemType = ELEM_OPENCLOSE;
						}
						else {
							strncpy(tag, p+1, r-(p+1));
							tag[r-(p+1)] = '\0';
							elemType = ELEM_OPEN;
						}
					}
					for (;r<q && (*r == ' ' || *r == '\t'); r++);
					if (q-r > ATTR_MAX_LEN) {
//						TlenLog("ATTR_MAX_LEN too small, ignore current tag");
					}
					else {
						strncpy(attr, r, q-r);
						if ((q-r)>0 && attr[q-r-1] == '/') {
							attr[q-r-1] = '\0';
							elemType = ELEM_OPENCLOSE;
						}
						else
							attr[q-r] = '\0';
						TlenXmlProcessElem(xmlState, elemType, tag, attr);
					}
				}
				num += (q-p+1);
				p = q + 1;
				if (elemType == ELEM_CLOSE || elemType == ELEM_OPENCLOSE) {
					// Skip whitespaces after end tags
					for (; p<eob && isspace(*p); p++,num++);
				}
			}
			else
				break;
		}
		else {	// found inner text
			for (q=p+1; q<eob && *q != '<'; q++);
			if (q < eob) {	// found starting bracket of the next element
				str = (char *) mir_alloc(q-p+1);
				strncpy(str, p, q-p);
				str[q-p] = '\0';
				TlenXmlProcessElem(xmlState, ELEM_TEXT, str, NULL);
				mir_free(str);
				num += (q-p);
				p = q;
			}
			else
				break;
		}
	}

	return num;
}

static void TlenXmlParseAttr(XmlNode *node, char *text)
{
	char *kstart, *vstart;
	int klen, vlen;
	char *p;
	XmlAttr *a;

	if (node == NULL || text == NULL || strlen(text) <= 0)
		return;

	for (p=text;;) {

		// Skip leading whitespaces
		for (;*p != '\0' && (*p == ' ' || *p == '\t'); p++);
		if (*p == '\0')
			break;

		// Fetch key
		kstart = p;
		for (;*p != '\0' && *p != '=' && *p != ' ' && *p != '\t'; p++);
		klen = p-kstart;

		if (node->numAttr >= node->maxNumAttr) {
			node->maxNumAttr = node->numAttr + 20;
			node->attr = (XmlAttr **) mir_realloc(node->attr, node->maxNumAttr*sizeof(XmlAttr *));
		}
		a = node->attr[node->numAttr] = (XmlAttr *) mir_alloc(sizeof(XmlAttr));
		node->numAttr++;

		// Skip possible whitespaces between key and '='
		for (;*p != '\0' && (*p == ' ' || *p == '\t'); p++);

		if (*p == '\0') {
			a->name = (char *) mir_alloc(klen+1);
			strncpy(a->name, kstart, klen);
			a->name[klen] = '\0';
			a->value = mir_strdup("");
			break;
		}

		if (*p != '=') {
			a->name = (char *) mir_alloc(klen+1);
			strncpy(a->name, kstart, klen);
			a->name[klen] = '\0';
			a->value = mir_strdup("");
			continue;
		}

		// Found '='
		p++;

		// Skip possible whitespaces between '=' and value
		for (;*p != '\0' && (*p == ' ' || *p == '\t'); p++);

		if (*p == '\0') {
			a->name = (char *) mir_alloc(klen+1);
			strncpy(a->name, kstart, klen);
			a->name[klen] = '\0';
			a->value = mir_strdup("");
			break;
		}

		// Fetch value
		if (*p == '\'' || *p == '"') {
			p++;
			vstart = p;
			for (;*p != '\0' && *p != *(vstart-1); p++);
			vlen = p-vstart;
			if (*p != '\0') p++;
		}
		else {
			vstart = p;
			for (;*p != '\0' && *p != ' ' && *p != '\t'; p++);
			vlen = p-vstart;
		}

		a->name = (char *) mir_alloc(klen+1);
		strncpy(a->name, kstart, klen);
		a->name[klen] = '\0';
		a->value = (char *) mir_alloc(vlen+1);
		strncpy(a->value, vstart, vlen);
		a->value[vlen] = '\0';
	}
}

static BOOL TlenXmlProcessElem(XmlState *xmlState, XmlElemType elemType, char *elemText, char *elemAttr)
{
	XmlNode *node, *parentNode, *n;
	//BOOL activateCallback = FALSE;
	char *text, *attr;

	if (elemText == NULL) return FALSE;

	if (elemType == ELEM_OPEN && !strcmp(elemText, "?xml")) {
//		TlenLog("XML: skip <?xml> tag");
		return TRUE;
	}

	// Find active node
	node = &(xmlState->root);
	parentNode = NULL;
	while (node->numChild>0 && node->child[node->numChild-1]->state == NODE_OPEN) {
		parentNode = node;
		node = node->child[node->numChild-1];
	}

	if (node->state != NODE_OPEN) return FALSE;

	text = mir_strdup(elemText);

	if (elemAttr)
		attr = mir_strdup(elemAttr);
	else
		attr = NULL;

	switch (elemType) {
	case ELEM_OPEN:
		if (node->numChild >= node->maxNumChild) {
			node->maxNumChild = node->numChild + 20;
			node->child = (XmlNode **) mir_realloc(node->child, node->maxNumChild*sizeof(XmlNode *));
		}
		n = node->child[node->numChild] = (XmlNode *) mir_alloc(sizeof(XmlNode));
		node->numChild++;
		n->name = text;
		n->depth = node->depth + 1;
		n->state = NODE_OPEN;
		n->numChild = n->maxNumChild = 0;
		n->child = NULL;
		n->numAttr = n->maxNumAttr = 0;
		n->attr = NULL;
		TlenXmlParseAttr(n, attr);
		n->text = NULL;
		if (n->depth == 1 && xmlState->callback1_open != NULL)
			(*(xmlState->callback1_open))(n, xmlState->userdata1_open);
		if (n->depth == 2 && xmlState->callback2_open != NULL)
			(*xmlState->callback2_open)(n, xmlState->userdata2_open);
		break;
	case ELEM_OPENCLOSE:
		if (node->numChild >= node->maxNumChild) {
			node->maxNumChild = node->numChild + 20;
			node->child = (XmlNode **) mir_realloc(node->child, node->maxNumChild*sizeof(XmlNode *));
		}
		n = node->child[node->numChild] = (XmlNode *) mir_alloc(sizeof(XmlNode));
		node->numChild++;
		n->name = text;
		n->depth = node->depth + 1;
		n->state = NODE_CLOSE;
		n->numChild = n->maxNumAttr = 0;
		n->child = NULL;
		n->numAttr = n->maxNumAttr = 0;
		n->attr = NULL;
		TlenXmlParseAttr(n, attr);
		n->text = NULL;
		if (n->depth == 1 && xmlState->callback1_close != NULL) {
			(*(xmlState->callback1_close))(n, xmlState->userdata1_close);
			TlenXmlRemoveChild(node, n);
		}
		if (n->depth == 2 && xmlState->callback2_close != NULL) {
			(*xmlState->callback2_close)(n, xmlState->userdata2_close);
			TlenXmlRemoveChild(node, n);
		}
		break;
	case ELEM_CLOSE:
		if (node->name != NULL && !strcmp(node->name, text)) {
			node->state = NODE_CLOSE;
			int nodeDepth = node->depth;
			if (nodeDepth == 1 && xmlState->callback1_close != NULL) {
				(*(xmlState->callback1_close))(node, xmlState->userdata1_close);
				TlenXmlRemoveChild(parentNode, node);
			}
			if (nodeDepth == 2 && xmlState->callback2_close != NULL) {
				(*xmlState->callback2_close)(node, xmlState->userdata2_close);
				TlenXmlRemoveChild(parentNode, node);
			}
			mir_free(text);
		}
		else {
//			TlenLog("XML: Closing </%s> without opening tag", text);
			mir_free(text);
			if (attr) mir_free(attr);
			return FALSE;
		}
		break;
	case ELEM_TEXT:
		node->text = text;
		break;
	default:
		mir_free(text);
		if (attr) mir_free(attr);
		return FALSE;
	}

	if (attr) mir_free(attr);

	return TRUE;
}

char *TlenXmlGetAttrValue(XmlNode *node, char *key)
{
	int i;

	if (node == NULL || node->numAttr <= 0 || key == NULL || strlen(key) <= 0)
		return NULL;
	for (i=0; i<node->numAttr; i++) {
		if (node->attr[i]->name && !strcmp(key, node->attr[i]->name))
			return node->attr[i]->value;
	}
	return NULL;
}

XmlNode *TlenXmlGetChild(XmlNode *node, char *tag)
{
	return TlenXmlGetNthChild(node, tag, 1);
}

XmlNode *TlenXmlGetNthChild(XmlNode *node, char *tag, int nth)
{
	int i, num;

	if (node == NULL || node->numChild <= 0 || tag == NULL || strlen(tag) <= 0 || nth < 1)
		return NULL;
	num = 1;
	for (i=0; i<node->numChild; i++) {
		if (node->child[i]->name && !strcmp(tag, node->child[i]->name)) {
			if (num == nth) {
				return node->child[i];
			}
			num++;
		}
	}
	return NULL;
}

XmlNode *TlenXmlGetChildWithGivenAttrValue(XmlNode *node, char *tag, char *attrKey, char *attrValue)
{
	int i;
	char *str;

	if (node == NULL || node->numChild <= 0 || tag == NULL || strlen(tag) <= 0 || attrKey == NULL || strlen(attrKey) <= 0 || attrValue == NULL || strlen(attrValue) <= 0)
		return NULL;
	for (i=0; i<node->numChild; i++) {
		if (node->child[i]->name && !strcmp(tag, node->child[i]->name)) {
			if ((str=TlenXmlGetAttrValue(node->child[i], attrKey)) != NULL)
				if (!strcmp(str, attrValue))
					return node->child[i];
		}
	}
	return NULL;
}

static void TlenXmlRemoveChild(XmlNode *node, XmlNode *child)
{
	int i;

	if (node == NULL || child == NULL || node->numChild <= 0) return;
	for (i=0; i<node->numChild; i++) {
		if (node->child[i] == child)
			break;
	}
	if (i < node->numChild) {
		for (++i; i<node->numChild; i++)
			node->child[i-1] = node->child[i];
		node->numChild--;
		TlenXmlFreeNode(child);
	}
}

void TlenXmlFreeNode(XmlNode *node)
{
	int i;

	if (node == NULL)
		return;
	// Free all children first
	for (i=0; i<node->numChild; i++)
		TlenXmlFreeNode(node->child[i]);
	mir_free(node->child);
	// Free all attributes
	for (i=0; i<node->numAttr; i++) {
		mir_free(node->attr[i]->name);
		mir_free(node->attr[i]->value);
		mir_free(node->attr[i]);
	}
	mir_free(node->attr);
	// Free string field
	mir_free(node->text);
	mir_free(node->name);
	// Free the node itself
	mir_free(node);
}

XmlNode *TlenXmlCopyNode(XmlNode *node)
{
	XmlNode *n;
	int i;

	if (node == NULL) return NULL;
	n = (XmlNode *) mir_alloc(sizeof(XmlNode));
	// Copy attributes
	if (node->numAttr > 0) {
		n->attr = (XmlAttr **) mir_alloc(node->numAttr*sizeof(XmlAttr *));
		for (i=0; i<node->numAttr; i++) {
			n->attr[i] = (XmlAttr *) mir_alloc(sizeof(XmlAttr));
			if (node->attr[i]->name) n->attr[i]->name = mir_strdup(node->attr[i]->name);
			else n->attr[i]->name = NULL;
			if (node->attr[i]->value) n->attr[i]->value = mir_strdup(node->attr[i]->value);
			else n->attr[i]->value = NULL;
		}
	}
	else
		n->attr = NULL;
	// Recursively copy children
	if (node->numChild > 0) {
		n->child = (XmlNode **) mir_alloc(node->numChild*sizeof(XmlNode *));
		for (i=0; i<node->numChild; i++)
			n->child[i] = TlenXmlCopyNode(node->child[i]);
	}
	else
		n->child = NULL;
	// Copy other fields
	n->numAttr = node->numAttr;
	n->maxNumAttr = node->numAttr;
	n->numChild = node->numChild;
	n->maxNumChild = node->numChild;
	n->depth = node->depth;
	n->state = node->state;
	n->name = (node->name)?mir_strdup(node->name):NULL;
	n->text = (node->text)?mir_strdup(node->text):NULL;

	return n;
}

XmlNode *TlenXmlCreateNode(char *name)
{
	XmlNode *n;

	if (name == NULL)
		return NULL;

	n = (XmlNode *) mir_alloc(sizeof(XmlNode));
	memset(n, 0, sizeof(XmlNode));
	n->name = mir_strdup(name);
	return n;
}

void TlenXmlAddAttr(XmlNode *n, char *name, char *value)
{
	int i;

	if (n == NULL || name == NULL || value == NULL)
		return;

	i = n->numAttr;
	(n->numAttr)++;
	n->attr = (XmlAttr **) mir_realloc(n->attr, sizeof(XmlAttr *) * n->numAttr);
	n->attr[i] = (XmlAttr *) mir_alloc(sizeof(XmlAttr));
	n->attr[i]->name = mir_strdup(name);
	n->attr[i]->value = mir_strdup(value);
}

XmlNode *TlenXmlAddChild(XmlNode *n, char *name)
{
	int i;

	if (n == NULL || name == NULL)
		return NULL;

	i = n->numChild;
	n->numChild++;
	n->child = (XmlNode **) mir_realloc(n->child, sizeof(XmlNode *) * n->numChild);
	n->child[i] = (XmlNode *) mir_alloc(sizeof(XmlNode));
	memset(n->child[i], 0, sizeof(XmlNode));
	n->child[i]->name = mir_strdup(name);
	return n->child[i];
}

void TlenXmlAddText(XmlNode *n, char *text)
{
	if (n != NULL && text != NULL) {
		if (n->text) mir_free(n->text);
		n->text = mir_strdup(text);
	}
}

