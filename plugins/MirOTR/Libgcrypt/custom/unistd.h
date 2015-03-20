#ifndef NG_UNISTD_H
#define NG_UNISTD_H
#include <basetsd.h>
#include <stddef.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef S_IWUSR
#	define S_IWUSR _S_IWRITE
#endif
#ifndef S_IRUSR
#	define S_IRUSR _S_IREAD
#endif
#ifndef S_IXUSR
#	define S_IXUSR _S_IEXEC
#endif
#ifndef S_ISREG
#	define S_ISREG(A) (1)
#endif

#ifndef offsetof
#	define offsetof(type, member) ((size_t)&((type *)0)->member)
#endif

#include <io.h>
#define F_OK 0	/* Check for file existence */
#define X_OK 1	/* Check for execute permission. */
#define W_OK 2	/* Check for write permission */
#define R_OK 4	/* Check for read permission */

#define open _open
#define close _close
#define fileno _fileno
#define setmode _setmode
#define access _access
#define lseek _lseek
#define read _read
#define write _write

#include <process.h>
#define getpid _getpid

#endif // NG_UNISTD_H
