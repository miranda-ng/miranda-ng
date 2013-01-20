#define LINE_MAX_SIZE 512

#ifdef __cplusplus
extern "C" {
#endif

#define szMimeTypeConfigFile "HTTPMimeTypes"

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
		char* ext;
		struct _ExtensionListCell* next;
	} ExtensionListCell ;


	typedef struct _ContentType {
		char* mimeType;
		ExtensionListCell* extList;
		struct _ContentType* next;
	} ContentType ;

	typedef ContentType* ContentTypeDB;
	typedef ExtensionListCell* ExtensionList;



	extern int bInitMimeHandling();
	extern const char * pszGetMimeType(const char * pszFileName);

#ifdef __cplusplus
}
#endif
