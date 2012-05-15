/*
 * xsltutils.h: interfaces for the utilities module of the XSLT engine.
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */

#ifndef __XML_XSLTUTILS_H__
#define __XML_XSLTUTILS_H__

#if defined(WIN32) && defined(_MSC_VER)
#include <libxslt/xsltwin32config.h>
#else
#include <libxslt/xsltconfig.h>
#endif

#include <libxml/xpath.h>
#include <libxml/xmlerror.h>
#include "xsltexports.h"
#include "xsltInternals.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * XSLT_TODO:
 *
 * Macro to flag unimplemented blocks.
 */
#define XSLT_TODO 							\
    xsltGenericError(xsltGenericErrorContext,				\
	    "Unimplemented block at %s:%d\n",				\
            __FILE__, __LINE__);

/**
 * XSLT_STRANGE:
 *
 * Macro to flag that a problem was detected internally.
 */
#define XSLT_STRANGE 							\
    xsltGenericError(xsltGenericErrorContext,				\
	    "Internal error at %s:%d\n",				\
            __FILE__, __LINE__);

/**
 * IS_XSLT_ELEM:
 *
 * Checks that the element pertains to XSLT namespace.
 */
#define IS_XSLT_ELEM(n)							\
    (((n) != NULL) && ((n)->ns != NULL) &&				\
     (xmlStrEqual((n)->ns->href, XSLT_NAMESPACE)))

/**
 * IS_XSLT_NAME:
 *
 * Checks the value of an element in XSLT namespace.
 */
#define IS_XSLT_NAME(n, val)						\
    (xmlStrEqual((n)->name, (const xmlChar *) (val)))

/**
 * IS_XSLT_REAL_NODE:
 *
 * Check that a node is a 'real' one: document, element, text or attribute.
 */
#define IS_XSLT_REAL_NODE(n)						\
    (((n) != NULL) &&							\
     (((n)->type == XML_ELEMENT_NODE) ||				\
      ((n)->type == XML_TEXT_NODE) ||					\
      ((n)->type == XML_ATTRIBUTE_NODE) ||				\
      ((n)->type == XML_DOCUMENT_NODE) ||				\
      ((n)->type == XML_HTML_DOCUMENT_NODE) ||				\
      ((n)->type == XML_PI_NODE)))

/*
 * Our own version of namespaced atributes lookup.
 */
XSLTPUBFUN xmlChar * XSLTCALL	xsltGetNsProp			(xmlNodePtr node,
						 const xmlChar *name,
						 const xmlChar *nameSpace);
XSLTPUBFUN int XSLTCALL		xsltGetUTF8Char			(const unsigned char *utf,
						 int *len);

/*
 * XSLT specific error and debug reporting functions.
 */
XSLTPUBVAR xmlGenericErrorFunc xsltGenericError;
XSLTPUBVAR void *xsltGenericErrorContext;
XSLTPUBVAR xmlGenericErrorFunc xsltGenericDebug;
XSLTPUBVAR void *xsltGenericDebugContext;

XSLTPUBFUN void XSLTCALL		
		xsltPrintErrorContext		(xsltTransformContextPtr ctxt,
	                                         xsltStylesheetPtr style,
						 xmlNodePtr node);
XSLTPUBFUN void XSLTCALL		
    		xsltMessage			(xsltTransformContextPtr ctxt,
						 xmlNodePtr node,
						 xmlNodePtr inst);
XSLTPUBFUN void XSLTCALL		
    		xsltSetGenericErrorFunc		(void *ctx,
						 xmlGenericErrorFunc handler);
XSLTPUBFUN void XSLTCALL		
    		xsltSetGenericDebugFunc		(void *ctx,
						 xmlGenericErrorFunc handler);
XSLTPUBFUN void XSLTCALL		
    		xsltSetTransformErrorFunc	(xsltTransformContextPtr ctxt,
						 void *ctx,
						 xmlGenericErrorFunc handler);
XSLTPUBFUN void XSLTCALL		
    		xsltTransformError		(xsltTransformContextPtr ctxt,
						 xsltStylesheetPtr style,
						 xmlNodePtr node,
						 const char *msg,
						 ...);

/*
 * Sorting.
 */

XSLTPUBFUN void XSLTCALL		
    		xsltDocumentSortFunction	(xmlNodeSetPtr list);
XSLTPUBFUN void XSLTCALL		
    		xsltSetSortFunc			(xsltSortFunc handler);
XSLTPUBFUN void XSLTCALL		
    		xsltSetCtxtSortFunc		(xsltTransformContextPtr ctxt,
						 xsltSortFunc handler);
XSLTPUBFUN void XSLTCALL		
    		xsltDefaultSortFunction		(xsltTransformContextPtr ctxt,
						 xmlNodePtr *sorts,
						 int nbsorts);
XSLTPUBFUN void XSLTCALL		
    		xsltDoSortFunction		(xsltTransformContextPtr ctxt,
						 xmlNodePtr * sorts,
						 int nbsorts);
XSLTPUBFUN xmlXPathObjectPtr * XSLTCALL 
    		xsltComputeSortResult		(xsltTransformContextPtr ctxt,
						 xmlNodePtr sort);

/*
 * QNames handling.
 */

XSLTPUBFUN const xmlChar * XSLTCALL 
    		xsltGetQNameURI			(xmlNodePtr node,
						 xmlChar **name);

/*
 * Output, reuse libxml I/O buffers.
 */
XSLTPUBFUN int XSLTCALL		
    		xsltSaveResultTo		(xmlOutputBufferPtr buf,
						 xmlDocPtr result,
						 xsltStylesheetPtr style);
XSLTPUBFUN int XSLTCALL		
    		xsltSaveResultToFilename	(const char *URI,
						 xmlDocPtr result,
						 xsltStylesheetPtr style,
						 int compression);
XSLTPUBFUN int XSLTCALL		
    		xsltSaveResultToFile		(FILE *file,
						 xmlDocPtr result,
						 xsltStylesheetPtr style);
XSLTPUBFUN int XSLTCALL		
    		xsltSaveResultToFd		(int fd,
						 xmlDocPtr result,
						 xsltStylesheetPtr style);
XSLTPUBFUN int XSLTCALL             
    		xsltSaveResultToString          (xmlChar **doc_txt_ptr, 
                                                 int * doc_txt_len, 
                                                 xmlDocPtr result, 
                                                 xsltStylesheetPtr style);

/*
 * Profiling.
 */
XSLTPUBFUN void XSLTCALL		
    		xsltSaveProfiling		(xsltTransformContextPtr ctxt,
						 FILE *output);
XSLTPUBFUN xmlDocPtr XSLTCALL	
    		xsltGetProfileInformation	(xsltTransformContextPtr ctxt);

XSLTPUBFUN long XSLTCALL		
    		xsltTimestamp			(void);
XSLTPUBFUN void XSLTCALL		
    		xsltCalibrateAdjust		(long delta);

/**
 * XSLT_TIMESTAMP_TICS_PER_SEC:
 *
 * Sampling precision for profiling
 */
#define XSLT_TIMESTAMP_TICS_PER_SEC 100000l

/*
 * Hooks for the debugger.
 */

typedef enum {
    XSLT_DEBUG_NONE = 0, /* no debugging allowed */
    XSLT_DEBUG_INIT,
    XSLT_DEBUG_STEP,
    XSLT_DEBUG_STEPOUT,
    XSLT_DEBUG_NEXT,
    XSLT_DEBUG_STOP,
    XSLT_DEBUG_CONT,
    XSLT_DEBUG_RUN,
    XSLT_DEBUG_RUN_RESTART,
    XSLT_DEBUG_QUIT
} xsltDebugStatusCodes;

XSLTPUBVAR int xslDebugStatus;

typedef void (*xsltHandleDebuggerCallback) (xmlNodePtr cur, xmlNodePtr node,
			xsltTemplatePtr templ, xsltTransformContextPtr ctxt);
typedef int (*xsltAddCallCallback) (xsltTemplatePtr templ, xmlNodePtr source);
typedef void (*xsltDropCallCallback) (void);

XSLTPUBFUN void XSLTCALL
		xsltSetDebuggerStatus		(int value);
XSLTPUBFUN int XSLTCALL
		xsltGetDebuggerStatus		(void);
XSLTPUBFUN int XSLTCALL		
		xsltSetDebuggerCallbacks	(int no, void *block);
XSLTPUBFUN int XSLTCALL		
		xslAddCall			(xsltTemplatePtr templ,
						 xmlNodePtr source);
XSLTPUBFUN void XSLTCALL		
		xslDropCall			(void);

#ifdef __cplusplus
}
#endif

#endif /* __XML_XSLTUTILS_H__ */


