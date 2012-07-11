
#define WINVER 0x0700
#define _WIN32_WINNT 0x0700


#include <io.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>

// #include <m_core.h>

// mir_core hack for MD5
#define MIR_CORE_DLL(T)	T __cdecl


/* Define the state of the MD5 Algorithm. */
typedef unsigned char mir_md5_byte_t; /* 8-bit byte */
typedef unsigned int mir_md5_word_t; /* 32-bit word */

typedef struct mir_md5_state_s {
	mir_md5_word_t count[2];  /* message length in bits, lsw first */
	mir_md5_word_t abcd[4];    /* digest buffer */
	mir_md5_byte_t buf[64];    /* accumulate block */
} mir_md5_state_t;

MIR_CORE_DLL(void) mir_md5_init(mir_md5_state_t *pms);
MIR_CORE_DLL(void) mir_md5_append(mir_md5_state_t *pms, const mir_md5_byte_t *data, int nbytes);
MIR_CORE_DLL(void) mir_md5_finish(mir_md5_state_t *pms, mir_md5_byte_t digest[16]);
MIR_CORE_DLL(void) mir_md5_hash(const mir_md5_byte_t *data, int len, mir_md5_byte_t digest[16]);
