
/*
  Meanwhile - Unofficial Lotus Sametime Community Client Library
  Copyright (C) 2004  Christopher (siege) O'Brien
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
  
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _MW_DEBUG_H
#define _MW_DEBUG_H


#include <stdarg.h>
#include <glib.h>

#include "mw_common.h"


/** replaces NULL strings with "(null)". useful for printf where
    you're unsure that the %s will be non-NULL. Note that while the
    linux printf will do this automatically, not all will. The others
    will instead segfault */
#define NSTR(str) ((str)? (str): "(null)")


// Miranda NG adaptation, MSVC
// #ifndef g_debug
// #define g_debug(format...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, format)
// #endif


// Miranda NG adaptation, MSVC
// #ifndef g_info
// #define g_info(format...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, format)
// #endif


#ifndef MW_MAILME_ADDRESS
/** email address used in mw_debug_mailme. */
#define MW_MAILME_ADDRESS  "meanwhile-devel@lists.sourceforge.net"
#endif


#ifndef MW_MAILME_CUT_START
#define MW_MAILME_CUT_START  "-------- begin copy --------"
#endif


#ifndef MW_MAILME_CUT_STOP
#define MW_MAILME_CUT_STOP   "--------- end copy ---------"
#endif


#ifndef MW_MAILME_MESSAGE
/** message used in mw_debug_mailme instructing user on what to do
    with the debugging output produced from that function */
#define MW_MAILME_MESSAGE "\n" \
 "  Greetings! It seems that you've run across protocol data that the\n" \
 "Meanwhile library does not yet know about. As such, there may be\n"    \
 "some unexpected behavior in this session. If you'd like to help\n"     \
 "resolve this issue, please copy and paste the following block into\n"  \
 "an email to the address listed below with a brief explanation of\n"    \
 "what you were doing at the time of this message. Thanks a lot!"
#endif


void mw_debug_datav(const guchar *buf, gsize len,
		    const char *info, va_list args);


void mw_debug_data(const guchar *buf, gsize len,
		   const char *info, ...);


void mw_debug_opaquev(struct mwOpaque *o, const char *info, va_list args);


void mw_debug_opaque(struct mwOpaque *o, const char *info, ...);


void mw_mailme_datav(const guchar *buf, gsize len,
		     const char *info, va_list args);

void mw_mailme_data(const guchar *buf, gsize len,
		    const char *info, ...);


/** Outputs a hex dump of a mwOpaque with debugging info and a
    pre-defined message. Identical to mw_mailme_opaque, but taking a
    va_list argument */
void mw_mailme_opaquev(struct mwOpaque *o, const char *info, va_list args);



/** Outputs a hex dump of a mwOpaque with debugging info and a
    pre-defined message.

    if MW_MAILME is undefined or false, this function acts the same as
    mw_mailme_opaque.

    @arg block  data to be printed in a hex block
    @arg info   a printf-style format string

    The resulting message is in the following format:
    @code
    MW_MAILME_MESSAGE
    " Please send mail to: " MW_MAILME_ADDRESS
    MW_MAILME_CUT_START
    info
    block
    MW_MAILME_CUT_STOP
    @endcode
 */
void mw_mailme_opaque(struct mwOpaque *o, const char *info, ...);


#endif

