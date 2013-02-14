/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is mir_free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "variables.h"
#include "parse_xml.h"

#include <string.h>
#include "libxml/xmlmemory.h"
#include "libxml/debugXML.h"
#include "libxml/HTMLtree.h"
#include "libxml/xmlIO.h"
#include "libxml/DOCBparser.h"
#include "libxml/xinclude.h"
#include "libxml/catalog.h"
#include "libxslt/xslt.h"
#include "libxslt/xsltInternals.h"
#include "libxslt/transform.h"
#include "libxslt/xsltutils.h"

xsltStylesheetPtr (*pXsltParseStylesheetDoc)(xmlDocPtr doc);
xmlDocPtr (*pXmlParseMemory)(const char * buffer, int size);

void (*pXmlFree)(void *mem);
xmlDocPtr (*pXmlParseFile)(const char * filename);
void (*pXmlFreeDoc)(xmlDocPtr cur);
void (*pXmlInitParser)(void);
void (*pXmlCleanupParser)(void);
int (*pXmlSubstituteEntitiesDefault)(int val);

xsltStylesheetPtr (*pXsltParseStylesheetFile)(const xmlChar * filename);
int (*pXsltSaveResultToString)(xmlChar ** doc_txt_ptr, int * doc_txt_len, xmlDocPtr result, xsltStylesheetPtr style);
void (*pXsltFreeStylesheet)(xsltStylesheetPtr sheet);
xmlDocPtr (*pXsltApplyStylesheet)(xsltStylesheetPtr style, xmlDocPtr doc, const char ** params);
void (*pXsltCleanupGlobals)(void);

/*
	pattern, subject
*/
static TCHAR *parseXslts(ARGUMENTSINFO *ai) {
	xmlDocPtr res;
	xmlChar *xmlChRes = NULL;
	int resLen;
	
	if (ai->argc != 3) {
		return NULL;
	}

	char *szStyleSheet = mir_t2a(ai->targv[1]);
	char *szDoc = mir_t2a(ai->targv[2]);


	log_debugA("calling xsltParseMemory");
	xmlDocPtr sdoc = pXmlParseMemory(szStyleSheet, strlen(szStyleSheet));
	if (sdoc == NULL) {
		return NULL;
	}
	
	log_debugA("calling xsltParseStylesheetDoc");
	xsltStylesheetPtr cur = pXsltParseStylesheetDoc(sdoc);
	if (cur == NULL) {
		log_debugA("calling xsltFreeDoc");
		pXmlFreeDoc(sdoc);
	
		return NULL;
	}
	
	log_debugA("calling xsltParseMemory");
	xmlDocPtr doc = pXmlParseMemory(szDoc, strlen(szDoc));
	if (doc == NULL) {
		log_debugA("calling xsltFreeDoc");
		pXmlFreeDoc(sdoc);
		log_debugA("calling xsltFreeStylesheet");
		pXsltFreeStylesheet(cur);
	
		return NULL;
	}

	log_debugA("calling xsltApplyStylesheet");
	res = pXsltApplyStylesheet(cur, doc, NULL);
	if (res == NULL) {
		log_debugA("calling xsltFreeStylesheet");
		pXsltFreeStylesheet(cur);
		log_debugA("calling xsltFreeDoc");
		pXmlFreeDoc(doc);
		log_debugA("calling xsltFreeDoc");
		pXmlFreeDoc(sdoc);

		return NULL;
	}

	log_debugA("calling xsltSaveResultToString");
	pXsltSaveResultToString(&xmlChRes, &resLen, res, cur);
	log_debugA("calling xsltFreeStylesheet");
	pXsltFreeStylesheet(cur);
	log_debugA("calling xsltFreeDoc");
	pXmlFreeDoc(res);
	log_debugA("calling xsltFreeDoc");
	pXmlFreeDoc(doc);
	log_debugA("calling mir_free");
	mir_free(szStyleSheet);
	log_debugA("calling mir_free");
	mir_free(szDoc);

	TCHAR *tszRes = mir_a2t((char *)xmlChRes);

	log_debugA("calling xmlFree");
	pXmlFree(xmlChRes);
	
	return tszRes;
}

/*
	files
*/
static TCHAR *parseXsltf(ARGUMENTSINFO *ai) {
	TCHAR *tszRes;
	xmlDocPtr doc, res;
	xmlChar *xmlChRes = NULL;
	int resLen;
	
	if (ai->argc != 3) {
		return NULL;
	}

	char *szStyleSheet = mir_t2a(ai->targv[1]);
	char *szDoc = mir_t2a(ai->targv[2]);


	log_debugA("xslt with %s and %s", szStyleSheet, szDoc);

	log_debugA("calling xsltParseStylesheetFile");
	xsltStylesheetPtr cur = pXsltParseStylesheetFile((const xmlChar *)szStyleSheet);
	if (cur == NULL) {
	
		return NULL;
	}
	log_debugA("result: %x", cur);

	log_debugA("calling xmlParseFile");
	doc = pXmlParseFile(szDoc);
	if (doc == NULL) {
		pXsltFreeStylesheet(cur);
	
		return NULL;
	}
	log_debugA("result: %x", doc);

	log_debugA("calling xsltApplyStylesheet");
	res = pXsltApplyStylesheet(cur, doc, NULL);
	if (res == NULL) {
		pXsltFreeStylesheet(cur);
		pXmlFreeDoc(doc);

		return NULL;
	}
	log_debugA("result: %x", res);

	log_debugA("calling xsltSaveResultToString");
	pXsltSaveResultToString(&xmlChRes, &resLen, res, cur);

	log_debugA("calling xsltFreeStylesheet(cur)");
	pXsltFreeStylesheet(cur);

	log_debugA("calling xmlFreeDoc(res)");
	pXmlFreeDoc(res);

	log_debugA("calling xmlFreeDoc(doc)");
	pXmlFreeDoc(doc);

	//log_debug("calling xsltCleanupGlobals");
    //pXsltCleanupGlobals();

	//log_debug("calling xmlCleanupParser");
    //pXmlCleanupParser();

	mir_free(szStyleSheet);
	mir_free(szDoc);

	tszRes = mir_a2t((char *)xmlChRes);

	log_debugA("calling xmlFree");
	pXmlFree(xmlChRes);
	
	return tszRes;
}

int initXslt() {
	HMODULE hModule = LoadLibrary(_T("libxml2.dll"));
	if (hModule == NULL) {
		TCHAR path[MAX_PATH];
		GetModuleFileName(NULL, path, MAX_PATH);
		TCHAR *cur = _tcsrchr(path, '\\');
		if (cur != NULL)
			_tcscpy(cur+1, _T("libxml2.dll"));
		else
			_tcscpy(path, _T("libxml2.dll"));
		hModule = LoadLibrary(path);
	}
	if (hModule == NULL) {
		return -1;
	}
	
	pXmlParseFile = (struct _xmlDoc *(__cdecl *)(const char *))GetProcAddress(hModule, "xmlParseFile");
	pXmlFreeDoc = (void (__cdecl *)(struct _xmlDoc *))GetProcAddress(hModule, "xmlFreeDoc");
	pXmlCleanupParser = (void (__cdecl *)(void ))GetProcAddress(hModule, "xmlCleanupParser");
	pXmlInitParser = (void (__cdecl *)(void ))GetProcAddress(hModule, "xmlInitParser");
	pXmlParseMemory = (struct _xmlDoc *(__cdecl *)(const char *,int ))GetProcAddress(hModule, "xmlParseMemory");
	pXmlFree = (void (__cdecl *)(void *))*((void (__cdecl **)(void *))GetProcAddress(hModule, "xmlFree"));
	pXmlSubstituteEntitiesDefault = (int (__cdecl *)(int ))GetProcAddress(hModule, "xmlSubstituteEntitiesDefault");

	hModule = LoadLibraryA("libxslt.dll");
	if (hModule == NULL) {
		char path[MAX_PATH];
		char *cur;

		GetModuleFileNameA(NULL, path, sizeof(path));
		cur = strrchr(path, '\\');
		if (cur != NULL)
			strcpy(cur+1, "libxslt.dll");
		else
			strcpy(path, "libxslt.dll");
		hModule = LoadLibraryA(path);
	}
	if (hModule == NULL) {
		return -1;
	}

	/* xmlFree !!! pthreads? */

	pXsltParseStylesheetFile = (struct _xsltStylesheet *(__cdecl *)(const unsigned char *))GetProcAddress(hModule, "xsltParseStylesheetFile");
	pXsltFreeStylesheet = (void (__cdecl *)(struct _xsltStylesheet *))GetProcAddress(hModule, "xsltFreeStylesheet");
	pXsltApplyStylesheet = (struct _xmlDoc *(__cdecl *)(struct _xsltStylesheet *,struct _xmlDoc *,const char ** ))GetProcAddress(hModule, "xsltApplyStylesheet");
	pXsltSaveResultToString = (int (__cdecl *)(unsigned char ** ,int *,struct _xmlDoc *,struct _xsltStylesheet *))GetProcAddress(hModule, "xsltSaveResultToString");
	pXsltCleanupGlobals = (void (__cdecl *)(void ))GetProcAddress(hModule, "xsltCleanupGlobals");
	pXsltParseStylesheetDoc = (struct _xsltStylesheet *(__cdecl *)(struct _xmlDoc *))GetProcAddress(hModule, "xsltParseStylesheetDoc");

	/* init */
	pXmlInitParser();
	pXmlSubstituteEntitiesDefault(1);

	return 0;
}

int registerXsltTokens() {

	if (initXslt() != 0) {
		log_infoA("Variables: XSLT library not initialized");
		return -1;
	}

	registerIntToken(_T(XSLTF), parseXsltf, TRF_FUNCTION, LPGEN("XML")"\t(x,y)\t"LPGEN("apply stylesheet file x to document file y"));
	registerIntToken(_T(XSLTS), parseXslts, TRF_FUNCTION, LPGEN("XML")"\t(x,y)\t"LPGEN("apply stylesheet x to document y"));

	return 0;
}
