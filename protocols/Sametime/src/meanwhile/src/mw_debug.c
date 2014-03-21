
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


#include <glib/gstring.h>

#include "mw_debug.h"



#define FRMT1            "%02x"
#define FRMT2            FRMT1 FRMT1 " "
#define FRMT4            FRMT2 FRMT2
#define FRMT8            FRMT4 FRMT4
#define FRMT16           FRMT8 FRMT8

#define ADVANCE(b, n, c)  {b += c; n -= c;}



/** writes hex pairs of buf to str */
static void pretty_print(GString *str, const guchar *buf, gsize len) {
  while(len >= 16) {
    /* write a complete line */
    g_string_append_printf(str, FRMT16,
			   buf[0],  buf[1],  buf[2],  buf[3],
			   buf[4],  buf[5],  buf[6],  buf[7],
			   buf[8],  buf[9],  buf[10], buf[11],
			   buf[12], buf[13], buf[14], buf[15]);
    ADVANCE(buf, len, 16);
    
    /* append \n to each line but the last */
    if(len) g_string_append(str, "\n");
  }

  /* write an incomplete line */
  if(len >= 8) {
    g_string_append_printf(str, FRMT8,
			   buf[0], buf[1], buf[2], buf[3],
			   buf[4], buf[5], buf[6], buf[7]);
    ADVANCE(buf, len, 8);
  }
  
  if(len >= 4) {
    g_string_append_printf(str, FRMT4,
			   buf[0], buf[1], buf[2], buf[3]);
    ADVANCE(buf, len, 4);
  }

  if(len >= 2) {
    g_string_append_printf(str, FRMT2, buf[0], buf[1]);
    ADVANCE(buf, len, 2);
  }

  if(len >= 1) {
    g_string_append_printf(str, FRMT1, buf[0]);
    ADVANCE(buf, len, 1);
  }
}



void mw_debug_datav(const guchar *buf, gsize len,
		    const char *msg, va_list args) {
  GString *str;

  g_return_if_fail(buf != NULL || len == 0);

  str = g_string_new(NULL);

  if(msg) {
    char *txt = g_strdup_vprintf(msg, args);
    g_string_append_printf(str, "%s\n", txt);
    g_free(txt);
  }
  pretty_print(str, buf, len);

  g_debug(str->str);
  g_string_free(str, TRUE);
}



void mw_debug_data(const guchar *buf, gsize len,
		   const char *msg, ...) {
  va_list args;
  
  g_return_if_fail(buf != NULL || len == 0);

  va_start(args, msg);
  mw_debug_datav(buf, len, msg, args);
  va_end(args);
}



void mw_debug_opaquev(struct mwOpaque *o, const char *txt, va_list args) {
  g_return_if_fail(o != NULL);
  mw_debug_datav(o->data, o->len, txt, args);
}



void mw_debug_opaque(struct mwOpaque *o, const char *txt, ...) {
  va_list args;

  g_return_if_fail(o != NULL);

  va_start(args, txt);
  mw_debug_opaquev(o, txt, args);
  va_end(args);
}


void mw_mailme_datav(const guchar *buf, gsize len,
		     const char *info, va_list args) {

#if MW_MAILME
  GString *str;
  char *txt;

  str = g_string_new(MW_MAILME_MESSAGE "\n"
		     "  Please send mail to: " MW_MAILME_ADDRESS "\n"
		     MW_MAILME_CUT_START "\n");
  str = g_string_new(NULL);

  txt = g_strdup_vprintf(info, args);
  g_string_append_printf(str, "%s\n", txt);
  g_free(txt);

  if(buf && len) pretty_print(str, buf, len);

  g_string_append(str, MW_MAILME_CUT_STOP);

  g_debug(str->str);
  g_string_free(str, TRUE);

#else
  mw_debug_datav(buf, len, info, args);

#endif
}



void mw_mailme_data(const guchar *buf, gsize len,
		    const char *info, ...) {
  va_list args;
  va_start(args, info);
  mw_mailme_datav(buf, len, info, args);
  va_end(args);
}



void mw_mailme_opaquev(struct mwOpaque *o, const char *info, va_list args) {
  mw_mailme_datav(o->data, o->len, info, args);
}



void mw_mailme_opaque(struct mwOpaque *o, const char *info, ...) {
  va_list args;
  va_start(args, info);
  mw_mailme_opaquev(o, info, args);
  va_end(args);
}
