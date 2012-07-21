/* Module:  imo2skypeproxy.c
   Purpose: Implementation of imo2skype API as Skype proxy (for Miranda Skype plugin)
   Author:  leecher
   Date:    01.09.2009

   Changelog:
    1.00  - 01.09.2009 - Initial release
    1.01  - 02.09.2009 - Little bugfix for Onlinestatus, introduced mutex for send thread.
    1.02  - 02.09.2009 - Added support for incoming call notifications and expired sessions.
    1.03  - 03.09.2009 - As WIN32-port of libcurl is a total mess, Win32-version now uses WinINET.
    1.04  - 04.09.2009 - Added support for searching, adding and removing buddies
    1.05  - 06.09.2009 - * Mapped "busy" to DND, not NA, as forum user AL|EN proposed.
                         * Mood text will now also be sent on status change of a contact
                           (doesn't really have an effect though)
                         * New Parameter -d (daemonize) to launch proxy in background
                         * Fixed a bug with a crash when encoding umlauts on Win32
    1.06  - 25.09.2009 - Added parameter -t for local timestamps.
    1.07  - 07.10.2009 - Bugfix: Added parsing of multiple JSON messages
                         Added support for writing to logfile while in daemon mode (-l)
    1.08  - 18.10.2009 - Added support for voice calls via imo.im flash on WIN32 (-i)
                       - Limited memory queues to 50 entries to reduce memory usage.
    1.09  - 07.11.2009 - Split command line main module and proxy module to make proxy
                         accessible by different layers (i.e. new Miranda Plugin layer)
    1.10  - 14.12.2009 - Bug in the cJson library. The authors used the String as second 
                         sprintf-Parameter which contains the format string instead of 
                         ["%s", StringParam] causing problems sending Messages containt a %
                       - Added some space in the dialog for translations and added Translate() 
                         function to some strings in the Wrapper-DLL
                       - More verbose output if you enable logging
                       - Fixes a Bug that caused the plugin to block after going offline 
                         and reconnecting back online
                       - Hopefully fixed a severe threading problem: The Send-Mutex was not
                         covering a full transaction, but only 1 send causing the receiver
                         the receive garbled data causing "An existing connection was forcibly
                         closed by the remote host." error.
                       - Imo2S_Exit freed the temporary buffer too early resulting in a crash
                         on exit.
	1.12  - 19.04.2010 - POST interface changed from www.imo.im/amy to s.imo.im/amy
	1.13  - 19.12.2010 - imo.im modified their interface, so tried to adapt the communication
						 routines at low level to be compatible again. There still may be
						 many bugs. Don't forget to turn off history logging in the options
						 at https://imo.im
						 There also seems to be a nice new feature called "reflection" so that
						 you can login from multiple sessions and messages etc. entered there
						 will be reflected to the current session. This may be addressed in one
						 of the next builds...
	1.14  - 02.03.2011 - Added a Skype WIN32-API Emulation Layer for better compatibility with
						 other clients that don't want to implement the socks-protocol, so that
						 we are compatible out-of-the-box.
					   - Added a few features to be compatible with libpurple. i.e. we now support
					     message numbering, even tough this is only supported in PROTOCOL 4.
					   - Added Keepalive packet sending to circumvent imo.im AutoAway.
	1.15  - 18.07.2012 - Avatar support
					   - Support for new imo.im protocol, can now be compiled to use old or new
					     protocol, in case there are troubles. New protocol enables participation
						 in groupchats generated with native Skype client with at least 3 
						 participants (still a bit experimental)
					   - WININET HTTPS communication layer now uses async calls to try to prevent
					     lockups on connection loss.
*/

#include <memory.h>
#include "imo2sproxy.h"

// -----------------------------------------------------------------------------
// PUBLIC
// -----------------------------------------------------------------------------

void Imo2sproxy_Defaults (IMO2SPROXY_CFG *pCfg)
{
	memset (pCfg, 0, sizeof(IMO2SPROXY_CFG));
	pCfg->fpLog = stdout;
	pCfg->logerror = fprintf;
}

/* Real implementation is now in socksproxy.c for the socket communication method
   and in w32skypeemu.c for the Window messages Skype emulation
 */
