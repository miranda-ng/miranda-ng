/* Module:  main.c
   Purpose: Main commandline dispatcher for imo.im Skypeproxy
   Author:  leecher
   Date:    07.11.2009
*/
#include <stdlib.h>
#include <stdio.h>
#include "imo2sproxy.h"
#include "socksproxy.h"
#ifdef WIN32
#include "w32skypeemu.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#define Daemonize FreeConsole()
#ifdef _DEBUG
#define LOG(_args_) printf _args_
#ifndef _WIN64
#include "crash.c"
#endif
#endif
#else
#define Daemonize daemon(0, 0)
#endif

// -----------------------------------------------------------------------------
// EIP
// -----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int i, j=0, bDaemon = 0;
	char cMode='s';
	IMO2SPROXY_CFG stCfg;
	SOCKSPROXY_CFG stSocksCfg;
#ifdef WIN32
	W32SKYPEEMU_CFG stSypeEmuCfg;
#endif
	IMO2SPROXY *pProxy;

#ifdef _DEBUG
	Crash_Init();
#endif

	Imo2sproxy_Defaults (&stCfg);
	SocksProxy_Defaults (&stSocksCfg);
#ifdef WIN32
	W32SkypeEmu_Defaults(&stSypeEmuCfg);
#endif
	if (argc<3)
	{
		printf ("imo.im Skypeproxy V1.15 - (c) by leecher 2009-2012\n\n"
			"%s [-d] [-v [-l <Logfile>]] [-t] [-i]\n"
			"[-m<s|a>] [-h <Bind to IP>] [-p <Port>] <Username> <Password>\n\n"
			"-d\t- Daemonize (detach from console)\n"
			"-v\t- Verbose mode, log commands to console\n"
			"-l\t- Set logfile to redirect verbose log to.\n"
			"-t\t- Ignore server timestamp and use current time for messages\n"
			"-i\t- Use interactive mode (starts imo.im flash app upon call)\n"
			"-m\t- Specify connection mode to use:\n"
			"\t\ts\tSocket mode (SkypeProxy protocol) [default]\n"
#ifdef WIN32
			"\t\ta\tSkype API Emulation via Window messages (real Skype API)\n"
#endif
			"\tParameters for socket mode:\n"
			"-h\t- Bind to a specific IP, not to all interfaces (default)\n"
			"-p\t- Bind to another port (default: 1401)\n"
			"Default: Bind to any interface, Use Port %d\n", argv[0], stSocksCfg.sPort);
		return EXIT_FAILURE;
	}

	for (i=1; i<argc; i++)
	{
		if (argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
			case 'v':
				stCfg.bVerbose = 1;
				break;
			case 'l':
				if (argc<=i+1)
				{
					fprintf (stderr, "Please specify logfile for -l\n");
					return  EXIT_FAILURE;
				}
				if (!(stCfg.fpLog = fopen(argv[++i], "a")))
				{
					fprintf (stderr, "Cannot open logfile %s\n", argv[i]);
					return EXIT_FAILURE;
				}
				break;
			case 'd':
				bDaemon = 1;
				break;
			case 'h':
				if (argc<=i+1)
				{
					fprintf (stderr, "Please specify bind IP for -h\n");
					return  EXIT_FAILURE;
				}
				stSocksCfg.lAddr = inet_addr(argv[++i]);
				break;
			case 'p':
				if (argc<=i+1)
				{
					fprintf (stderr, "Please specify port for -p\n");
					return EXIT_FAILURE;
				}
				stSocksCfg.sPort = atoi(argv[++i]);
				break;
			case 't':
				stCfg.iFlags |= IMO2S_FLAG_CURRTIMESTAMP;
				break;
			case 'i':
				stCfg.iFlags |= IMO2S_FLAG_ALLOWINTERACT;
				break;
			case 'm':
				switch (argv[i][2])
				{
				case 'a':
#ifndef WIN32
					fprintf (stderr, "This mode is not available on your platform!\n");
					return EXIT_FAILURE;
#endif
				case 's':
					cMode=argv[i][2];
					break;
				default:
					fprintf (stderr, "Unknown mode: %c\n", argv[i][2]);
					return EXIT_FAILURE;
				}
				break;
			default:
				printf ("Unknown parameter: %s\n", argv[i]);
				break;
			}
		}
		else
		{
			switch (j)
			{
			case 0: stCfg.pszUser = argv[i]; break;
			case 1: stCfg.pszPass = argv[i]; break;
			default: printf ("Unknown extra arg: %s\n", argv[i]); break;
			}
			j++;
		}
	}

	if (!stCfg.pszUser)
	{
		fprintf (stderr, "Please specify Username\n");
		return EXIT_FAILURE;
	}
	if (!stCfg.pszPass)
	{
		fprintf (stderr, "Please specify Password\n");
		return EXIT_FAILURE;
	}
	if (bDaemon && stCfg.bVerbose && stCfg.fpLog == stdout)
	{
		printf ("Parameters Verbose mode and daemonize cannot be used together, if you don't\n"
			"specify a logfile, not daemonizing.\n");
		bDaemon = 0;
	}
	if (bDaemon) Daemonize;

	switch (cMode)
	{
	case 's':
		if (!(pProxy = SocksProxy_Init (&stCfg, &stSocksCfg)))
			return EXIT_FAILURE;
		break;
	case 'a':
#ifdef WIN32
		if (!(pProxy = W32SkypeEmu_Init (&stCfg, &stSypeEmuCfg)))
			return EXIT_FAILURE;
#else
		fprintf (stderr, "Skype API EMulator is only available in WIN32\n");
		return EXIT_FAILURE;
#endif
		break;
	}

	if (pProxy->Open(pProxy) == 0)
		pProxy->Loop(pProxy);
	pProxy->Exit(pProxy);

	if (stCfg.fpLog && stCfg.fpLog != stdout && stCfg.fpLog != stderr)
	{
		fclose(stCfg.fpLog);
		stCfg.fpLog = NULL;
	}

	return EXIT_SUCCESS;
}

