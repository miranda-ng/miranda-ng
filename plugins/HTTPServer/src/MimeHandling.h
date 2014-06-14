#define LINE_MAX_SIZE 512

#ifdef __cplusplus
extern "C" {
#endif

#define szMimeTypeConfigFile _T("HTTPMimeTypes")

	/* MIME DB Data structure

	         ----------                ----------
	        | mimeType |              | mimeType |
	        |----------|              |----------|
	    ----|   next -----------------|   next   |
	        |----------|              |----------|
	        | extList ---             | extList ---
	         ----------  |             ----------  |
	                   --|---                    --|---
	                  | ext  |                  | ext  |
	                  |------|                  |------|
	                  | next |                  | next |
	                   --|---                    ------
	                   --|---
	                  | ext  |
	                  |------|
	                  | next |
	                   ------
	*/

	typedef struct _ExtensionListCell {
		TCHAR *ext;
		struct _ExtensionListCell *next;
	} ExtensionListCell;


	typedef struct _ContentType {
		TCHAR *mimeType;
		ExtensionListCell *extList;
		struct _ContentType *next;
	} ContentType;

	typedef ContentType *ContentTypeDB;
	typedef ExtensionListCell *ExtensionList;



	extern int bInitMimeHandling();
	extern const TCHAR *pszGetMimeType(const TCHAR *pszFileName);

#ifdef __cplusplus
}
#endif
