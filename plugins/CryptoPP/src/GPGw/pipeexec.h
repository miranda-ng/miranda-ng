#ifndef __PIPEEXEC_H__
#define __PIPEEXEC_H__

#include <windows.h>

// typen
typedef enum
{
  pxSuccess,
  pxSuccessExitCodeInvalid,
  pxCreatePipeFailed,
  pxDuplicateHandleFailed,
  pxCloseHandleFailed,
  pxCreateProcessFailed,
  pxThreadWaitFailed,
  pxReadFileFailed,
  pxBufferOverflow
}
pxResult;

pxResult pxExecute(char *acommandline, char *ainput, char **aoutput, LPDWORD aexitcode);

#endif // __PIPEEXEC_H__

