#ifndef _IOLAYER_H_
#define _IOLAYER_H_

struct _tagIOLAYER;
typedef struct _tagIOLAYER IOLAYER;

struct _tagIOLAYER
{
	void (*Exit) (IOLAYER *hIO);

	char *(*Post) (IOLAYER *hIO, char *pszURL, char *pszPostFields, unsigned int cbPostFields, unsigned int *pdwLength);
	char *(*Get) (IOLAYER *hIO, char *pszURL, unsigned int *pdwLength);
	void (*Cancel) (IOLAYER *hIO);
	char *(*GetLastError) (IOLAYER *hIO);
	char *(*EscapeString) (IOLAYER *hIO, char *pszData);
	void (*FreeEscapeString) (char *pszData);
};

#ifdef WIN32
IOLAYER *IoLayerW32_Init(void);
IOLAYER *IoLayerNETLIB_Init(void);
#endif
IOLAYER *IoLayerCURL_Init(void);

#endif
