
#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(P128)
#endif

/*********************************************************************/
/*                                                                   */
/* Licensed Materials - Property of IBM                              */
/*                                                                   */
/* L-GHUS-5VMPGW, L-GHUS-5S3PEE                                      */
/* (C) Copyright IBM Corp. 1991, 2004  All Rights Reserved           */
/*                                                                   */
/* US Government Users Restricted Rights - Use, duplication or       */
/* disclosure restricted by GSA ADP Schedule Contract with           */
/* IBM Corp.                                                         */
/*                                                                   */
/*********************************************************************/


#ifndef ENV_DEFS
#define ENV_DEFS

#ifdef __cplusplus
extern "C" {
#endif

/*	Size of the buffer used to hold the environment variable values (i.e., it
	excludes the variable name) but including the trailing null terminator.

	NOTE:	The largest known example of an environment variable value is a
			max'ed out COMx=...  (the modem init strings can be large, and
			there's plenty of them).
*/
#define	MAXENVVALUE 256

/* Used to preface ini variables that are different between OSs which may */
/* share the same INI file. */

#if defined(PM)
#define OS_PREFIX			"PM"
#elif defined(W)
#define OS_PREFIX				"WIN"
#else
#define OS_PREFIX				""		/* Only necessary to distinguish */
									/* between entries used in a multi */
									/* OS environment with a single NOTES.INI */
#endif
/*	Environment variable package */
#define NATIVE "Native"
/*  System temp directory constants */
#define TEMP_DIR_PREFIX				"notes"
#define TEMP_DIR_SUFFIX_LEN			6	  /* If changing, must change TEMP_DIR_SUFFIX_FORMATSPEC to match */
#define TEMP_DIR_SUFFIX_FORMATSPEC	"%06x"
#define TEMP_DIR_DEFAULT_SUFFIX		"G00000"
#define TEMP_DIR_SUFFIX_MAX_VALUE	0x00FFFFFF

BOOL LNPUBLIC		OSGetEnvironmentString (const char far *VariableName, char far *retValueBuffer, WORD BufferLength);
long LNPUBLIC		OSGetEnvironmentLong (const char far *VariableName);
#define OSGetEnvironmentInt(s) ((int) OSGetEnvironmentLong(s))

void LNPUBLIC		OSSetEnvironmentVariable (const char far *VariableName, const char far *Value);
void LNPUBLIC		OSSetEnvironmentInt (const char far *VariableName, int Value);

#ifdef __cplusplus
}
#endif


#endif



#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif

